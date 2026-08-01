#include "sim/decoder.hpp"

namespace sim {
namespace {

int32_t sign_extend_12(uint32_t imm12) {
  if ((imm12 >> 11) & 1) {
    imm12 |= 0xFFFFF000u;
  }
  return static_cast<int32_t>(imm12);
}
int32_t sign_extend_13(uint32_t imm13) {
  if ((imm13 >> 12) & 1) {
    imm13 |= 0xFFFFE000u;
  }
  return static_cast<int32_t>(imm13);
}
int32_t sign_extend_21(uint32_t imm21) {
  if ((imm21 >> 20) & 1) {
    imm21 |= 0xFFE00000u;
  }
  return static_cast<int32_t>(imm21);
}


void decode_r(uint32_t inst, DecodedInstr& out) {
  out.format = InstFormat::R;
  out.rd = static_cast<uint8_t>((inst >> 7) & 0x1F);
  out.funct3 = static_cast<uint8_t>((inst >> 12) & 0x7);
  out.rs1 = static_cast<uint8_t>((inst >> 15) & 0x1F);
  out.rs2 = static_cast<uint8_t>((inst >> 20) & 0x1F);
  out.funct7 = static_cast<uint8_t>((inst >> 25) & 0x7F);
}

void decode_i(uint32_t inst, DecodedInstr& out) {
  out.format = InstFormat::I;
  out.rd = static_cast<uint8_t>((inst >> 7) & 0x1F);
  out.funct3 = static_cast<uint8_t>((inst >> 12) & 0x7);
  out.rs1 = static_cast<uint8_t>((inst >> 15) & 0x1F);
  const uint32_t imm12 = (inst >> 20) & 0xFFF;
  out.imm = sign_extend_12(imm12);
}

void decode_s(uint32_t inst, DecodedInstr& out) {
  out.format = InstFormat::S;
  out.funct3 = static_cast<uint8_t>((inst >> 12) & 0x7);
  out.rs1 = static_cast<uint8_t>((inst >> 15) & 0x1F);
  out.rs2 = static_cast<uint8_t>((inst >> 20) & 0x1F);
  const uint32_t imm12 =
      ((inst >> 7) & 0x1F) | (((inst >> 25) & 0x7F) << 5);
  out.imm = sign_extend_12(imm12);
}

void decode_b(uint32_t inst, DecodedInstr& out) {
  out.format = InstFormat::B;
  out.funct3 = static_cast<uint8_t>((inst >> 12) & 0x7);
  out.rs1 = static_cast<uint8_t>((inst >> 15) & 0x1F);
  out.rs2 = static_cast<uint8_t>((inst >> 20) & 0x1F);
  const uint32_t imm13 =
      (((inst >> 31) & 0x1) << 12) | (((inst >> 25) & 0x3F) << 5) |
      (((inst >> 8) & 0xF) << 1) | (((inst >> 7) & 0x1) << 11);
  out.imm = sign_extend_13(imm13);
}

void decode_u(uint32_t inst, DecodedInstr& out) {
  out.format = InstFormat::U;
  out.rd = static_cast<uint8_t>((inst >> 7) & 0x1F);
  out.imm = static_cast<int32_t>((inst >> 12) << 12);
}

void decode_j(uint32_t inst, DecodedInstr& out) {
  out.format = InstFormat::J;
  out.rd = static_cast<uint8_t>((inst >> 7) & 0x1F);
  const uint32_t imm21 =
      (((inst >> 31) & 0x1) << 20) | (((inst >> 21) & 0x3FF) << 1) |
      (((inst >> 20) & 0x1) << 11) | (((inst >> 12) & 0xFF) << 12);
  out.imm = sign_extend_21(imm21);
}

}  // namespace

DecodedInstr decode(uint32_t inst) {
  DecodedInstr out;
  out.raw = inst;
  out.opcode = static_cast<uint8_t>(inst & 0x7F);

  if (out.opcode == 0x33) {
    decode_r(inst, out);
  } else if (out.opcode == 0x03 || out.opcode == 0x13 || out.opcode == 0x67) {
    decode_i(inst, out);
  } else if (out.opcode == 0x23) {
    decode_s(inst, out);
  } else if (out.opcode == 0x63) {
    decode_b(inst, out);
  } else if (out.opcode == 0x37 || out.opcode == 0x17) {
    decode_u(inst, out);
  } else if (out.opcode == 0x6F) {
    decode_j(inst, out);
  }

  return out;
}

}  // namespace sim
