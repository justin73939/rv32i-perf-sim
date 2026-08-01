#include "sim/alu.hpp"

namespace sim {

uint32_t alu(uint32_t a, uint32_t b, AluOp op) {
  // TODO: implement the ALU
  //
  // This is a pure function: inputs a, b, op → one uint32_t out.
  // Prefer if / else if (same style as the decoder).
  //
  // Hints (fill in AFTER source-first gate — don't guess bit tricks yet):
  //   - Add / Sub / And / Or / Xor: C++ +, -, &, |, ^ on uint32_t
  //   - Shifts: only the low 5 bits of b are the shift amount (RV32)
  //   - Slt / Sltu: compare as signed vs unsigned; result is 1 or 0
  //   - Mul family / Div family: see RISC-V M extension; watch divide-by-zero
  //     and signed overflow edge cases (spec defines them — you will look them up)
  //
  // Unknown op: return 0 for now (CPU shouldn't ask for one).
  if (op == AluOp::Add){
    return (a + b); //uint32_t natively takes care of overflow
  }
  else if (op == AluOp::Sub){
    return (a - b);
  }
  else if (op == AluOp::And){
    return (a & b);
  }
  else if (op == AluOp::Or){
    return (a | b);
  }
  else if (op == AluOp::Xor){
    return (a ^ b);
  }
  else if (op == AluOp::Sll){ // think how to only shift by first 5 bits
    return (a << (b & 0x1F));
  }
  else if (op == AluOp::Srl){ // think how to only shift by first 5 bits
    return (a >> (b & 0x1F));
  }
  else if (op == AluOp::Sra){ // if signed then shifted digits replaced by 1, and if not then 0
    return (static_cast<int32_t>(a) >> (b & 0x1F));
  }
  else if (op == AluOp::Slt){ // think how to account for signed-edness
    if (static_cast<int32_t>(a) < static_cast<int32_t>(b)){
      return 1;
    }
    else{
      return 0;
    }
  }
  else if (op == AluOp::Sltu){
    if (a < b){
      return 1;
    }
    else{
      return 0;
    }
  }
  else if (op == AluOp::Mul){
    return (a * b);
  }
  return 0;
}

}  // namespace sim
