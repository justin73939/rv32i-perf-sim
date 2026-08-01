#pragma once

#include <cstdint>

namespace sim {

// RISC-V base opcodes (inst[6:0]). See RISC-V unprivileged spec.
enum class Opcode : uint8_t {
  Load = 0x03,
  Imm = 0x13,
  Auipc = 0x17,
  Store = 0x23,
  Op = 0x33,
  Lui = 0x37,
  Branch = 0x63,
  Jalr = 0x67,
  Jal = 0x6F,
  System = 0x73,
};

enum class InstFormat {
  R,
  I,
  S,
  B,
  U,
  J,
  Unknown,
};

// Fields after decode. immediates are sign-extended to 32 bits.
struct DecodedInstr {
  uint32_t raw = 0;
  InstFormat format = InstFormat::Unknown;

  uint8_t opcode = 0;  // inst[6:0]
  uint8_t rd = 0;      // inst[11:7]
  uint8_t funct3 = 0;  // inst[14:12]
  uint8_t rs1 = 0;     // inst[19:15]
  uint8_t rs2 = 0;     // inst[24:20]
  uint8_t funct7 = 0;  // inst[31:25]
  int32_t imm = 0;
};

// Parse one 32-bit little-endian instruction word into fields.
DecodedInstr decode(uint32_t inst);

}  // namespace sim
