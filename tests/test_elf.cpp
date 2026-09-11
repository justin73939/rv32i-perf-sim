#include "sim/elf.hpp"
#include "sim/memory.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

using sim::load_elf_bytes;
using sim::Memory;

namespace {

// Append a little-endian integer to a byte buffer (how ELF stores fields).
void append_u16(std::vector<uint8_t>& out, uint16_t v) {
  out.push_back(static_cast<uint8_t>(v & 0xFF));
  out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
}

void append_u32(std::vector<uint8_t>& out, uint32_t v) {
  out.push_back(static_cast<uint8_t>(v & 0xFF));
  out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
  out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
  out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
}

// Build a tiny handmade RV32 little-endian ELF with ONE PT_LOAD segment.
//
// Layout in the file:
//   [52-byte ELF header]
//   [32-byte program header]
//   [payload bytes: `file_payload`]
//
// That payload is copied to guest address `vaddr`.
// If memsz > file_payload.size(), the loader must zero the extra guest bytes.
std::vector<uint8_t> make_rv32_elf(uint32_t entry, uint32_t vaddr,
                                   const std::vector<uint8_t>& file_payload,
                                   uint32_t memsz) {
  const uint32_t filesz = static_cast<uint32_t>(file_payload.size());
  const uint32_t e_phoff = 52;          // program header starts right after Ehdr
  const uint32_t p_offset = 52 + 32;    // payload starts after Phdr

  std::vector<uint8_t> elf;

  // e_ident[16]
  elf.insert(elf.end(),
             {0x7F, 'E', 'L', 'F', 1 /*CLASS32*/, 1 /*LSB*/, 1 /*version*/, 0,
              0, 0, 0, 0, 0, 0, 0, 0});

  append_u16(elf, 2);     // e_type = ET_EXEC
  append_u16(elf, 243);   // e_machine = EM_RISCV
  append_u32(elf, 1);     // e_version
  append_u32(elf, entry); // e_entry  ← starting PC
  append_u32(elf, e_phoff);
  append_u32(elf, 0);     // e_shoff (no section table needed for loading)
  append_u32(elf, 0);     // e_flags
  append_u16(elf, 52);    // e_ehsize
  append_u16(elf, 32);    // e_phentsize
  append_u16(elf, 1);     // e_phnum = one program header
  append_u16(elf, 0);     // e_shentsize
  append_u16(elf, 0);     // e_shnum
  append_u16(elf, 0);     // e_shstrndx

  // Program header (PT_LOAD)
  append_u32(elf, 1);         // p_type = PT_LOAD
  append_u32(elf, p_offset);  // p_offset in file
  append_u32(elf, vaddr);     // p_vaddr in guest RAM
  append_u32(elf, vaddr);     // p_paddr (unused by us)
  append_u32(elf, filesz);    // p_filesz
  append_u32(elf, memsz);     // p_memsz (>= filesz)
  append_u32(elf, 5);         // p_flags = PF_R|PF_X (informational)
  append_u32(elf, 4);         // p_align

  elf.insert(elf.end(), file_payload.begin(), file_payload.end());
  return elf;
}

}  // namespace

TEST(Elf, RejectsNonElf) {
  Memory mem(256);
  // Long enough to pass the size check so we exercise the magic-number path.
  std::vector<uint8_t> junk(64, 0xAB);
  const auto r = load_elf_bytes(junk, mem);
  EXPECT_FALSE(r.ok());
  EXPECT_NE(r.error.find("magic"), std::string::npos);
}

TEST(Elf, LoadsPayloadAndReturnsEntry) {
  // Plant four recognizable bytes at guest address 16; entry also 16.
  Memory mem(256);
  const std::vector<uint8_t> payload = {0xAA, 0xBB, 0xCC, 0xDD};
  const auto elf = make_rv32_elf(/*entry=*/16, /*vaddr=*/16, payload,
                                 /*memsz=*/4);

  const auto r = load_elf_bytes(elf, mem);
  ASSERT_TRUE(r.ok()) << r.error;
  EXPECT_EQ(r.entry, 16u);
  EXPECT_EQ(mem.load8(16), 0xAAu);
  EXPECT_EQ(mem.load8(17), 0xBBu);
  EXPECT_EQ(mem.load8(18), 0xCCu);
  EXPECT_EQ(mem.load8(19), 0xDDu);
}

TEST(Elf, ZeroFillsBssTail) {
  // File only carries 2 payload bytes, but memsz asks for 6 guest bytes.
  // Addresses vaddr+2 .. vaddr+5 must become 0 even if Memory had garbage.
  Memory mem(256);
  mem.store8(20, 0x11);
  mem.store8(21, 0x22);
  mem.store8(22, 0x33);
  mem.store8(23, 0x44);
  mem.store8(24, 0x55);
  mem.store8(25, 0x66);

  const std::vector<uint8_t> payload = {0xAA, 0xBB};
  const auto elf = make_rv32_elf(/*entry=*/20, /*vaddr=*/20, payload,
                                 /*memsz=*/6);

  const auto r = load_elf_bytes(elf, mem);
  ASSERT_TRUE(r.ok()) << r.error;
  EXPECT_EQ(mem.load8(20), 0xAAu);
  EXPECT_EQ(mem.load8(21), 0xBBu);
  EXPECT_EQ(mem.load8(22), 0u);
  EXPECT_EQ(mem.load8(23), 0u);
  EXPECT_EQ(mem.load8(24), 0u);
  EXPECT_EQ(mem.load8(25), 0u);
}

TEST(Elf, RejectsSegmentThatDoesNotFit) {
  Memory mem(32);  // tiny RAM
  const std::vector<uint8_t> payload = {1, 2, 3, 4};
  // vaddr 30 + memsz 8 would need bytes through address 37 → past size 32
  const auto elf = make_rv32_elf(/*entry=*/30, /*vaddr=*/30, payload,
                                 /*memsz=*/8);

  const auto r = load_elf_bytes(elf, mem);
  EXPECT_FALSE(r.ok());
  EXPECT_NE(r.error.find("fit"), std::string::npos);
}

TEST(Elf, LoadThenCpuCanFetchEntryWord) {
  // Put a real ADDI encoding in the ELF payload so CPU fetch works after load.
  // ADDI x1, x2, 3 → 0x00310093, little-endian bytes: 93 00 31 00
  Memory mem(256);
  const std::vector<uint8_t> payload = {0x93, 0x00, 0x31, 0x00};
  const auto elf = make_rv32_elf(/*entry=*/0, /*vaddr=*/0, payload, /*memsz=*/4);

  const auto r = load_elf_bytes(elf, mem);
  ASSERT_TRUE(r.ok()) << r.error;
  EXPECT_EQ(r.entry, 0u);
  EXPECT_EQ(mem.load_word(0), 0x00310093u);
}
