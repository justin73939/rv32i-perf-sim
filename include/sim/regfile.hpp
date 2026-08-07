#pragma once

#include <cstdint>

namespace sim {

// 32 general-purpose registers (x0–x31) for the single-cycle CPU.
// Decoder gives register *indices*; the regfile holds the *values*.
// RISC-V rule: x0 is hardwired to zero — reads always return 0, writes are ignored.
class RegisterFile {
 public:
  // All 32 registers start at 0 (matches CPU reset state).
  RegisterFile();

  uint32_t read(uint8_t reg) const;

  void write(uint8_t reg, uint32_t value);

  // Reset every register to 0 (used between test cases / program runs).
  void reset();

 private:
  uint32_t regs_[32] = {};
};

}  // namespace sim
