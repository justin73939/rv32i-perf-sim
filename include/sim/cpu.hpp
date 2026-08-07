#pragma once

#include <cstdint>

namespace sim {

class Memory;
class RegisterFile;

// Single-cycle RV32IM CPU — executes one instruction per step().
// Owns PC and halt state; borrows Memory and RegisterFile (does not own them).
class Cpu {
 public:
  Cpu(Memory& mem, RegisterFile& regs);

  // Set PC to `pc`, clear halted flag. Regfile/memory are caller's job to reset separately.
  void reset(uint32_t pc = 0);

  void step();

  uint32_t pc() const;
  bool halted() const;

 private:
  Memory& mem_;
  RegisterFile& regs_;
  uint32_t pc_ = 0;
  bool halted_ = false;
};

}  // namespace sim
