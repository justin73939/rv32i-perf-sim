#include "sim/alu.hpp"

#include <gtest/gtest.h>

using sim::AluOp;
using sim::alu;

// Implement alu() until these go green.
// Spec: RISC-V Unprivileged ISA — Chapter 2 (RV32I integer ops), Chapter 13 (M).

TEST(Alu, Add) {
  EXPECT_EQ(alu(2u, 3u, AluOp::Add), 5u);
  EXPECT_EQ(alu(0xFFFFFFFFu, 1u, AluOp::Add), 0u);  // wrap mod 2^32
}

TEST(Alu, Sub) {
  EXPECT_EQ(alu(5u, 3u, AluOp::Sub), 2u);
  EXPECT_EQ(alu(0u, 1u, AluOp::Sub), 0xFFFFFFFFu);  // wrap
}

TEST(Alu, AndOrXor) {
  EXPECT_EQ(alu(0xF0F0F0F0u, 0x0FF00FF0u, AluOp::And), 0x00F000F0u);
  EXPECT_EQ(alu(0xF0F0F0F0u, 0x0FF00FF0u, AluOp::Or), 0xFFF0FFF0u);
  EXPECT_EQ(alu(0xF0F0F0F0u, 0x0FF00FF0u, AluOp::Xor), 0xFF00FF00u);
}

TEST(Alu, Shifts) {
  // Shift amount is low 5 bits of b only.
  EXPECT_EQ(alu(1u, 4u, AluOp::Sll), 16u);
  EXPECT_EQ(alu(0x80000000u, 4u, AluOp::Srl), 0x08000000u);
  EXPECT_EQ(alu(0x80000000u, 4u, AluOp::Sra), 0xF8000000u);  // sign bit fills
  EXPECT_EQ(alu(1u, 0x25u, AluOp::Sll), 32u);  // 0x25 & 0x1F == 5
}

TEST(Alu, SetLessThan) {
  EXPECT_EQ(alu(1u, 2u, AluOp::Slt), 1u);
  EXPECT_EQ(alu(2u, 1u, AluOp::Slt), 0u);
  // Signed: -1 < 1
  EXPECT_EQ(alu(0xFFFFFFFFu, 1u, AluOp::Slt), 1u);
  // Unsigned: 0xFFFFFFFF > 1
  EXPECT_EQ(alu(0xFFFFFFFFu, 1u, AluOp::Sltu), 0u);
}

TEST(Alu, Mul) {
  EXPECT_EQ(alu(6u, 7u, AluOp::Mul), 42u);
  // Low 32 bits of product only
  EXPECT_EQ(alu(0x80000000u, 2u, AluOp::Mul), 0u);
}

// More M-extension ops (MULH*, DIV*, REM*) — add tests as you implement.
// Spec defines divide-by-zero and overflow results; look those up before coding.
