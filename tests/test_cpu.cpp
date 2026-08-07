#include "sim/cpu.hpp"
#include "sim/memory.hpp"
#include "sim/regfile.hpp"

#include <gtest/gtest.h>

using sim::Cpu;
using sim::Memory;
using sim::RegisterFile;

// Implement Cpu::step() until these go green.
// Build up: R-type ADD → more opcodes → load/store → ecall halt.

TEST(Cpu, ResetSetsPc) {
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  cpu.reset(0x100);
  EXPECT_EQ(cpu.pc(), 0x100u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteAdd) {
  // ADD x1, x2, x3  →  0x003100B3
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x003100B3u);
  regs.write(2, 5u);
  regs.write(3, 7u);

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(regs.read(1), 12u);
  EXPECT_EQ(cpu.pc(), 4u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteAddi) {
  // ADDI x1, x2, 3  →  0x00310093
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x00310093u);
  regs.write(2, 5u);

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(regs.read(1), 8u);
  EXPECT_EQ(cpu.pc(), 4u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteLw) {
  // LW x10, 12(x8)  →  0x00C42503
  // addr = x8 + 12; load 32-bit word from memory into x10
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x00C42503u);       // instruction at PC=0
  mem.store_word(112, 0xDEADBEEFu);     // data at 100 + 12
  regs.write(8, 100u);                  // base address in x8

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(regs.read(10), 0xDEADBEEFu);
  EXPECT_EQ(cpu.pc(), 4u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteSw) {
  // SW x1, 8(x2)  →  0x00112423
  // addr = x2 + 8; store x1's value into memory as 32-bit word
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x00112423u);       // instruction at PC=0
  regs.write(1, 0xCAFEBABEu);
  regs.write(2, 64u);                   // base address in x2 → store at 72

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(mem.load_word(72), 0xCAFEBABEu);
  EXPECT_EQ(cpu.pc(), 4u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, EcallHalts) {
  // ECALL encoding (SYSTEM opcode, funct3=0, imm=0): 0x00000073
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x00000073u);
  cpu.reset(0);
  cpu.step();

  EXPECT_TRUE(cpu.halted());
  EXPECT_EQ(cpu.pc(), 4u);  // PC advances past ecall before halt (our convention)
}
