#include "sim/regfile.hpp"

namespace sim {

RegisterFile::RegisterFile() {
  reset();
}

uint32_t RegisterFile::read(uint8_t reg) const {
  // TODO: implement read
  //
  // Inputs:  reg — index from decoded instruction (rs1, rs2, or rd for debug)
  // Output:  32-bit value currently held in that register
  //
  // Rules to enforce:
  //   - If reg == 0 (x0), return 0 always — do not read from storage
  //   - Otherwise return the stored value for that index
  //   - reg is only 5 bits in instructions, but guard anyway if you like
  if (reg==0){
    return 0;
  }
  else{
    return regs_[reg];
  }
}

void RegisterFile::write(uint8_t reg, uint32_t value) {
  // TODO: implement write
  //
  // Inputs:  reg — destination index (rd from decoded instruction)
  //          value — 32-bit result to store (ALU output, load data, etc.)
  //
  // Rules to enforce:
  //   - If reg == 0 (x0), return immediately — writes to x0 are discarded
  //   - Otherwise store value in the backing array at index reg
  if (reg == 0){
    return;
  }
  else{
    regs_[reg] = value;
  }
}

void RegisterFile::reset() {
  // TODO: zero all stored registers (x1–x31; x0 needs no storage)
  //
  // Called from constructor and between tests so each case starts clean.
  for (int i = 0; i < 32; ++i){
    regs_[i] = 0;
  }
}

}  // namespace sim
