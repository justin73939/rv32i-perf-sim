#pragma once

#include <cstdint>

namespace sim {

// Which operation the ALU should perform.
// The CPU (later) will map DecodedInstr opcode/funct3/funct7 → AluOp.
// The ALU itself does not decode instructions — it only computes.
enum class AluOp {
  Add,
  Sub,
  And,
  Or,
  Xor,
  Sll,   // shift left logical
  Srl,   // shift right logical
  Sra,   // shift right arithmetic (sign-preserving)
  Slt,   // set less than (signed)
  Sltu,  // set less than (unsigned)
  // M extension (multiply / divide)
  Mul,
  Mulh,
  Mulhsu,
  Mulhu,
  Div,
  Divu,
  Rem,
  Remu,
};

// Combinational ALU: given two operands and an op, return the 32-bit result.
//
// Inputs:
//   a   — first operand (usually rs1 value from the regfile)
//   b   — second operand (rs2 value, or an immediate — caller decides)
//   op  — which operation to run
//
// Output:
//   32-bit result (later written to rd via the regfile, or used as address)
//
// Does NOT: touch the regfile, memory, or PC. Pure function.
uint32_t alu(uint32_t a, uint32_t b, AluOp op);

}  // namespace sim
