#include "sim/decoder.hpp"

#include <gtest/gtest.h>

using sim::DecodedInstr;
using sim::InstFormat;
using sim::decode;

// Hand-assembled encodings. Implement decode() until these go green.
// Spec: RISC-V Unprivileged ISA — Chapter 2 (RV32I) + Chapter 13 (M).

TEST(DecoderFormats, RTypeAdd) {
  // ADD x1, x2, x3  →  0x003100B3
  const DecodedInstr d = decode(0x003100B3u);
  EXPECT_EQ(d.format, InstFormat::R);
  EXPECT_EQ(d.opcode, 0x33u);
  EXPECT_EQ(d.rd, 1u);
  EXPECT_EQ(d.funct3, 0u);
  EXPECT_EQ(d.rs1, 2u);
  EXPECT_EQ(d.rs2, 3u);
  EXPECT_EQ(d.funct7, 0u);
  EXPECT_EQ(d.imm, 0);
}

TEST(DecoderFormats, RTypeSub) {
  // SUB x5, x6, x7  →  0x407302B3
  const DecodedInstr d = decode(0x407302B3u);
  EXPECT_EQ(d.format, InstFormat::R);
  EXPECT_EQ(d.opcode, 0x33u);
  EXPECT_EQ(d.rd, 5u);
  EXPECT_EQ(d.funct3, 0u);
  EXPECT_EQ(d.rs1, 6u);
  EXPECT_EQ(d.rs2, 7u);
  EXPECT_EQ(d.funct7, 0x20u);
}

TEST(DecoderFormats, RTypeMulMExtension) {
  // MUL x1, x2, x3  →  0x023100B3  (funct7=1, same R layout as ADD)
  const DecodedInstr d = decode(0x023100B3u);
  EXPECT_EQ(d.format, InstFormat::R);
  EXPECT_EQ(d.opcode, 0x33u);
  EXPECT_EQ(d.rd, 1u);
  EXPECT_EQ(d.rs1, 2u);
  EXPECT_EQ(d.rs2, 3u);
  EXPECT_EQ(d.funct3, 0u);
  EXPECT_EQ(d.funct7, 1u);
}

TEST(DecoderFormats, ITypeAddi) {
  // ADDI x1, x2, -4  →  0xFFC10093
  const DecodedInstr d = decode(0xFFC10093u);
  EXPECT_EQ(d.format, InstFormat::I);
  EXPECT_EQ(d.opcode, 0x13u);
  EXPECT_EQ(d.rd, 1u);
  EXPECT_EQ(d.funct3, 0u);
  EXPECT_EQ(d.rs1, 2u);
  EXPECT_EQ(d.imm, -4);
}

TEST(DecoderFormats, ITypeLw) {
  // LW x10, 12(x8)  →  0x00C42503
  const DecodedInstr d = decode(0x00C42503u);
  EXPECT_EQ(d.format, InstFormat::I);
  EXPECT_EQ(d.opcode, 0x03u);
  EXPECT_EQ(d.rd, 10u);
  EXPECT_EQ(d.funct3, 2u);
  EXPECT_EQ(d.rs1, 8u);
  EXPECT_EQ(d.imm, 12);
}

TEST(DecoderFormats, ITypeJalr) {
  // JALR x1, 4(x2)  →  0x004100E7
  const DecodedInstr d = decode(0x004100E7u);
  EXPECT_EQ(d.format, InstFormat::I);
  EXPECT_EQ(d.opcode, 0x67u);
  EXPECT_EQ(d.rd, 1u);
  EXPECT_EQ(d.rs1, 2u);
  EXPECT_EQ(d.imm, 4);
}

TEST(DecoderFormats, STypeSw) {
  // SW x1, 8(x2)  →  0x00112423
  const DecodedInstr d = decode(0x00112423u);
  EXPECT_EQ(d.format, InstFormat::S);
  EXPECT_EQ(d.opcode, 0x23u);
  EXPECT_EQ(d.funct3, 2u);
  EXPECT_EQ(d.rs1, 2u);
  EXPECT_EQ(d.rs2, 1u);
  EXPECT_EQ(d.imm, 8);
}

TEST(DecoderFormats, BTypeBeqPositive) {
  // BEQ x1, x2, +16  →  0x00208863
  const DecodedInstr d = decode(0x00208863u);
  EXPECT_EQ(d.format, InstFormat::B);
  EXPECT_EQ(d.opcode, 0x63u);
  EXPECT_EQ(d.funct3, 0u);
  EXPECT_EQ(d.rs1, 1u);
  EXPECT_EQ(d.rs2, 2u);
  EXPECT_EQ(d.imm, 16);
}

TEST(DecoderFormats, BTypeBneNegative) {
  // BNE x3, x4, -8  →  0xFE419CE3
  const DecodedInstr d = decode(0xFE419CE3u);
  EXPECT_EQ(d.format, InstFormat::B);
  EXPECT_EQ(d.opcode, 0x63u);
  EXPECT_EQ(d.funct3, 1u);
  EXPECT_EQ(d.rs1, 3u);
  EXPECT_EQ(d.rs2, 4u);
  EXPECT_EQ(d.imm, -8);
}

TEST(DecoderFormats, UTypeLui) {
  // LUI x1, 0x12345  →  0x123450B7
  const DecodedInstr d = decode(0x123450B7u);
  EXPECT_EQ(d.format, InstFormat::U);
  EXPECT_EQ(d.opcode, 0x37u);
  EXPECT_EQ(d.rd, 1u);
  EXPECT_EQ(d.imm, static_cast<int32_t>(0x12345000u));
}

TEST(DecoderFormats, UTypeAuipc) {
  // AUIPC x2, 1  →  0x00001117
  const DecodedInstr d = decode(0x00001117u);
  EXPECT_EQ(d.format, InstFormat::U);
  EXPECT_EQ(d.opcode, 0x17u);
  EXPECT_EQ(d.rd, 2u);
  EXPECT_EQ(d.imm, 0x1000);
}

TEST(DecoderFormats, JTypeJal) {
  // JAL x1, +8  →  0x008000EF
  const DecodedInstr d = decode(0x008000EFu);
  EXPECT_EQ(d.format, InstFormat::J);
  EXPECT_EQ(d.opcode, 0x6Fu);
  EXPECT_EQ(d.rd, 1u);
  EXPECT_EQ(d.imm, 8);
}
