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
  Sll,   
  Srl,   
  Sra,   
  Slt,   
  Sltu, 
  Mul,
  Mulh,
  Mulhsu,
  Mulhu,
  Div,
  Divu,
  Rem,
  Remu,
};


// Does NOT: touch the regfile, memory, or PC. Pure function.
uint32_t alu(uint32_t a, uint32_t b, AluOp op);

}  // namespace sim
