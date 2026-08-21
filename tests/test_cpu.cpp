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

TEST(Cpu, ExecuteLui) {
  // LUI x1, 0x12345  →  0x123450B7
  // rd gets upper immediate with low 12 bits zero: 0x12345000
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x123450B7u);
  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(regs.read(1), 0x12345000u);
  EXPECT_EQ(cpu.pc(), 4u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteAuipc) {
  // AUIPC x2, 1  →  0x00001117
  // decoded imm = 0x1000; rd = pc + imm
  // Plant at address 0x100 so we prove PC is part of the result.
  Memory mem(512);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0x100, 0x00001117u);
  cpu.reset(0x100);
  cpu.step();

  EXPECT_EQ(regs.read(2), 0x1100u);  // 0x100 + 0x1000
  EXPECT_EQ(cpu.pc(), 0x104u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteBeqTaken) {
  // BEQ x1, x2, +16  →  0x00208863
  // equal regs → PC becomes 0 + 16
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x00208863u);
  regs.write(1, 7u);
  regs.write(2, 7u);

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(cpu.pc(), 16u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteBeqNotTaken) {
  // Same BEQ; unequal regs → fall through to PC + 4
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x00208863u);
  regs.write(1, 7u);
  regs.write(2, 9u);

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(cpu.pc(), 4u);
  EXPECT_FALSE(cpu.halted());
}

// Remaining B-types share rs1=x1, rs2=x2, imm=+16; only funct3 changes.
// Base BEQ encoding 0x00208863; funct3 in bits [14:12].

TEST(Cpu, ExecuteBneTaken) {
  // BNE x1, x2, +16  →  0x00209863
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x00209863u);
  regs.write(1, 1u);
  regs.write(2, 2u);

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(cpu.pc(), 16u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteBneNotTaken) {
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x00209863u);
  regs.write(1, 7u);
  regs.write(2, 7u);

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(cpu.pc(), 4u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteBltTaken) {
  // BLT x1, x2, +16  →  0x0020C863  (signed: -1 < 1)
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x0020C863u);
  regs.write(1, 0xFFFFFFFFu);  // -1
  regs.write(2, 1u);

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(cpu.pc(), 16u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteBltNotTaken) {
  // unsigned-looking bits would be "less", but signed: 1 < -1 is false
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x0020C863u);
  regs.write(1, 1u);
  regs.write(2, 0xFFFFFFFFu);  // -1

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(cpu.pc(), 4u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteBgeTaken) {
  // BGE x1, x2, +16  →  0x0020D863  (signed: -1 >= -1)
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x0020D863u);
  regs.write(1, 0xFFFFFFFFu);
  regs.write(2, 0xFFFFFFFFu);

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(cpu.pc(), 16u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteBgeNotTaken) {
  // signed: -1 >= 1 is false
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x0020D863u);
  regs.write(1, 0xFFFFFFFFu);
  regs.write(2, 1u);

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(cpu.pc(), 4u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteBltuTaken) {
  // BLTU x1, x2, +16  →  0x0020E863  (unsigned: 1 < 0xFFFFFFFF)
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x0020E863u);
  regs.write(1, 1u);
  regs.write(2, 0xFFFFFFFFu);

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(cpu.pc(), 16u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteBltuNotTaken) {
  // unsigned: 0xFFFFFFFF < 1 is false (opposite of signed BLT on same bits)
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x0020E863u);
  regs.write(1, 0xFFFFFFFFu);
  regs.write(2, 1u);

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(cpu.pc(), 4u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteBgeuTaken) {
  // BGEU x1, x2, +16  →  0x0020F863
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x0020F863u);
  regs.write(1, 0xFFFFFFFFu);
  regs.write(2, 1u);

  cpu.reset(0);
  cpu.step();

  EXPECT_EQ(cpu.pc(), 16u);
  EXPECT_FALSE(cpu.halted());
}

TEST(Cpu, ExecuteBgeuNotTaken) {
  Memory mem(256);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  mem.store_word(0, 0x0020F863u);
  regs.write(1, 1u);
  regs.write(2, 0xFFFFFFFFu);

  cpu.reset(0);
  cpu.step();

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

TEST(Cpu, CpuRun) {
    Memory mem(256);
    RegisterFile regs;
    Cpu cpu(mem, regs);
  
    // ADDI x1, x2, 3  →  0x00310093
    regs.write(2, 20u); // give x2 a value of 20
    mem.store_word(0, 0x00410093u); // at index 0  store this instruction; now x1 has 24

    // STORE
    mem.store_word(4, 0x00112423u);       // instruction at PC=4 @ memory address 20+8 store value in x1 (24)

    // Load
    mem.store_word(8, 0x01C02183u);       // instruction at PC=8 @ feed value in mem address 28 (24) into reg x3

    // Ecall
    mem.store_word(12, 0x00000073u);
  
    cpu.reset(0);
    cpu.run();

    EXPECT_TRUE(cpu.halted()); // check if Ecall hit
    EXPECT_EQ(cpu.pc(), 16u);  // check if pc stopped at the right spot
    EXPECT_EQ(regs.read(2), 20u); // check if correct init of value in x2
    EXPECT_EQ(regs.read(1), 24u); // check if addI succeeded
    EXPECT_EQ(mem.load_word(28), 24u); // store: check if mem address 28 contains value in x1 
    EXPECT_EQ(regs.read(3), 24u); // load: check if reg x3 has mem address 28's value
}