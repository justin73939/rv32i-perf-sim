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
