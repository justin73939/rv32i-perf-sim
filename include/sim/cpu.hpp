#pragma once

#include <cstdint>

namespace sim {

class Memory;
class RegisterFile;

enum class FaultCause : uint8_t {
  None,
  InstructionAddressMisaligned,
};

// Single-cycle RV32IM CPU — executes one instruction per step().
// Owns PC and halt state; borrows Memory and RegisterFile (does not own them).
class Cpu {
 public:
  Cpu(Memory& mem, RegisterFile& regs);

  // Set PC to `pc`, clear halted flag. Regfile/memory are caller's job to reset separately.
  void reset(uint32_t pc = 0);

  void step();

  void run();

  uint32_t pc() const;
  bool halted() const;
  bool faulted() const;
  FaultCause fault_cause() const;

 private:
  Memory& mem_;
  RegisterFile& regs_;
  uint32_t pc_ = 0;
  bool halted_ = false;
  FaultCause fault_cause_ = FaultCause::None;
};

}  // namespace sim
