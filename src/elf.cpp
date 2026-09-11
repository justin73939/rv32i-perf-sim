#include "sim/elf.hpp"

#include "sim/memory.hpp"

#include <fstream>

namespace sim {
namespace {

// ---------------------------------------------------------------------------
// ELF is a binary file format. We do NOT #include <elf.h> so this stays
// portable and every constant we care about is visible in this file.
//
// Only what we need for: "is this an RV32 little-endian executable image,
// and which chunks do I copy into guest RAM?"
// ---------------------------------------------------------------------------

// First 4 bytes of every ELF file ("magic number").
constexpr uint8_t kElfMagic0 = 0x7F;
constexpr uint8_t kElfMagic1 = 'E';
constexpr uint8_t kElfMagic2 = 'L';
constexpr uint8_t kElfMagic3 = 'F';

// e_ident[] byte indices inside the 16-byte identity blob at file start.
constexpr std::size_t kEiMag0 = 0;
constexpr std::size_t kEiClass = 4;   // 32-bit vs 64-bit
constexpr std::size_t kEiData = 5;    // endianness
constexpr std::size_t kEiVersion = 6;

constexpr uint8_t kElfClass32 = 1;     // we only support 32-bit ELF
constexpr uint8_t kElfData2Lsb = 1;    // little-endian
constexpr uint8_t kElfVersionCurrent = 1;

// e_machine: RISC-V
constexpr uint16_t kEmRiscv = 243;

// e_type: we accept normal executables and position-independent ones.
constexpr uint16_t kEtExec = 2;
constexpr uint16_t kEtDyn = 3;

// Program header p_type: "load this chunk into memory"
constexpr uint32_t kPtLoad = 1;

// Fixed sizes for the 32-bit ELF structures we parse by hand.
constexpr std::size_t kElf32EhdrSize = 52;
constexpr std::size_t kElf32PhdrSize = 32;

// ---- little-endian readers -------------------------------------------------
// The ELF file is a bag of bytes. Multi-byte fields are little-endian for us,
// same as RISC-V and our Memory class. These helpers pull integers out safely.

uint16_t read_u16_le(const std::vector<uint8_t>& b, std::size_t off) {
  return static_cast<uint16_t>(b[off] | (static_cast<uint16_t>(b[off + 1]) << 8));
}

uint32_t read_u32_le(const std::vector<uint8_t>& b, std::size_t off) {
  return static_cast<uint32_t>(b[off]) |
         (static_cast<uint32_t>(b[off + 1]) << 8) |
         (static_cast<uint32_t>(b[off + 2]) << 16) |
         (static_cast<uint32_t>(b[off + 3]) << 24);
}

LoadElfResult fail(const std::string& msg) {
  LoadElfResult r;
  r.error = msg;
  return r;
}

}  // namespace

LoadElfResult load_elf_bytes(const std::vector<uint8_t>& file, Memory& mem) {
  // Need at least a full ELF header before we can trust any field offsets.
  if (file.size() < kElf32EhdrSize) {
    return fail("ELF too small to contain a 32-bit header");
  }

  // ---- Step 1: identity / "is this even an ELF we support?" ----------------
  if (file[kEiMag0] != kElfMagic0 || file[1] != kElfMagic1 ||
      file[2] != kElfMagic2 || file[3] != kElfMagic3) {
    return fail("not an ELF file (bad magic)");
  }
  if (file[kEiClass] != kElfClass32) {
    return fail("only 32-bit ELF is supported (need ELFCLASS32)");
  }
  if (file[kEiData] != kElfData2Lsb) {
    return fail("only little-endian ELF is supported");
  }
  if (file[kEiVersion] != kElfVersionCurrent) {
    return fail("unsupported ELF identity version");
  }

  const uint16_t e_type = read_u16_le(file, 16);
  const uint16_t e_machine = read_u16_le(file, 18);
  const uint32_t e_version = read_u32_le(file, 20);
  const uint32_t e_entry = read_u32_le(file, 24);   // <-- starting PC after load
  const uint32_t e_phoff = read_u32_le(file, 28);   // file offset of program headers
  const uint16_t e_ehsize = read_u16_le(file, 40);
  const uint16_t e_phentsize = read_u16_le(file, 42);
  const uint16_t e_phnum = read_u16_le(file, 44);

  if (e_machine != kEmRiscv) {
    return fail("ELF machine is not RISC-V");
  }
  if (e_version != 1) {
    return fail("unsupported ELF version field");
  }
  if (e_type != kEtExec && e_type != kEtDyn) {
    return fail("ELF type must be ET_EXEC or ET_DYN");
  }
  if (e_ehsize != kElf32EhdrSize) {
    return fail("unexpected ELF header size");
  }
  if (e_phentsize != kElf32PhdrSize) {
    return fail("unexpected program-header entry size");
  }
  if (e_phnum == 0) {
    return fail("ELF has no program headers to load");
  }

  // Program-header table must lie inside the file.
  const std::size_t ph_table_bytes =
      static_cast<std::size_t>(e_phnum) * kElf32PhdrSize;
  if (static_cast<std::size_t>(e_phoff) + ph_table_bytes > file.size()) {
    return fail("program-header table runs past end of file");
  }

  // ---- Step 2: walk the packing list (program headers) ---------------------
  // Each PT_LOAD entry is one "chunk":
  //   - take p_filesz bytes from the file at p_offset
  //   - write them into guest Memory at p_vaddr
  //   - if p_memsz > p_filesz, zero the remaining bytes (BSS-style tail)
  //
  // Non-PT_LOAD headers (notes, dynamic info, …) are ignored for loading.

  bool saw_load = false;

  for (uint16_t i = 0; i < e_phnum; ++i) {
    const std::size_t pho =
        static_cast<std::size_t>(e_phoff) +
        static_cast<std::size_t>(i) * kElf32PhdrSize;

    const uint32_t p_type = read_u32_le(file, pho + 0);
    const uint32_t p_offset = read_u32_le(file, pho + 4);   // where in the FILE
    const uint32_t p_vaddr = read_u32_le(file, pho + 8);    // where in GUEST RAM
    // p_paddr at +12 is ignored (physical addr; we have flat Memory only)
    const uint32_t p_filesz = read_u32_le(file, pho + 16);  // bytes present in file
    const uint32_t p_memsz = read_u32_le(file, pho + 20);   // bytes needed in RAM
    // p_flags / p_align ignored for now (no MMU / permissions yet)

    if (p_type != kPtLoad) {
      continue;
    }
    saw_load = true;

    if (p_memsz < p_filesz) {
      return fail("PT_LOAD has memsz < filesz");
    }

    // Guest RAM must be large enough for [p_vaddr, p_vaddr + p_memsz).
    // Memory::store8 silently ignores OOB writes — we refuse instead so a bad
    // ELF cannot look "loaded" while dropping bytes on the floor.
    if (static_cast<std::size_t>(p_vaddr) + static_cast<std::size_t>(p_memsz) >
        mem.size()) {
      return fail("PT_LOAD segment does not fit in simulator Memory");
    }

    if (static_cast<std::size_t>(p_offset) + static_cast<std::size_t>(p_filesz) >
        file.size()) {
      return fail("PT_LOAD file range runs past end of ELF");
    }

    // Copy the bytes that actually exist in the file (code + initialized data).
    for (uint32_t j = 0; j < p_filesz; ++j) {
      mem.store8(p_vaddr + j, file[static_cast<std::size_t>(p_offset) + j]);
    }

    // Zero the tail: addresses that need to exist in RAM but were omitted
    // from the file to save space (classic BSS case).
    for (uint32_t j = p_filesz; j < p_memsz; ++j) {
      mem.store8(p_vaddr + j, 0);
    }
  }

  if (!saw_load) {
    return fail("ELF has no PT_LOAD segments");
  }

  // ---- Step 3: hand the caller the starting PC ----------------------------
  // e_entry is a guest address (index into Memory), NOT a file offset.
  LoadElfResult ok;
  ok.entry = e_entry;
  return ok;
}

LoadElfResult load_elf(const std::string& path, Memory& mem) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    return fail("could not open ELF path: " + path);
  }

  // Read the whole file into a byte vector, then reuse the in-memory path.
  std::vector<uint8_t> file((std::istreambuf_iterator<char>(in)),
                            std::istreambuf_iterator<char>());
  if (file.empty()) {
    return fail("ELF file is empty");
  }
  return load_elf_bytes(file, mem);
}

}  // namespace sim
