#pragma once

#include <cstdint>

namespace sim {

// 32 general-purpose registers (x0–x31) for the single-cycle CPU.
//
// Where it sits: the execute stage reads rs1/rs2; writeback writes rd.
// Decoder gives you register *indices*; the regfile holds the *values*.
//
// RISC-V rule: x0 is hardwired to zero — reads always return 0, writes are ignored.
class RegisterFile {
 public:
  // All 32 registers start at 0 (matches CPU reset state).
  RegisterFile();

  // Read register `reg` (0–31). x0 always returns 0 regardless of stored state.
  uint32_t read(uint8_t reg) const;

  // Write `value` into register `reg`. No-op when reg == 0.
  void write(uint8_t reg, uint32_t value);

  // Reset every register to 0 (used between test cases / program runs).
  void reset();

 private:
  // TODO: backing storage for x1–x31 (x0 is not stored — it's implicit zero).
  // Hint: a fixed-size array of 32 uint32_t is fine; index 0 slot can stay unused.
  uint32_t regs_[32] = {};
};

}  // namespace sim
