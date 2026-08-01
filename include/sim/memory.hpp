#pragma once

#include <cstdint>
#include <vector>

namespace sim {

// Flat byte-addressable memory for the single-cycle CPU (no cache yet).
//
// Where it sits:
//   loads  → read bytes at address → assemble a register value
//   stores → take a register value → write bytes at address
//   fetch  → later: load_word(pc) to get the next instruction
//
// RISC-V is little-endian: lowest address holds the least-significant byte.
class Memory {
 public:
  // Allocate `size_bytes` of RAM, all zeros.
  explicit Memory(std::size_t size_bytes);

  // How many bytes this memory holds.
  std::size_t size() const;

  // Read one byte at `addr`. Out of range → return 0.
  uint8_t load8(uint32_t addr) const;

  // Write one byte at `addr`. Out of range → no-op.
  void store8(uint32_t addr, uint8_t value);

  // Load a 32-bit word (4 bytes), little-endian.
  // addr, addr+1, addr+2, addr+3 must all be in range or result is 0.
  uint32_t load_word(uint32_t addr) const;

  // Store a 32-bit word (4 bytes), little-endian. Out of range bytes skipped.
  void store_word(uint32_t addr, uint32_t value);

  // Set every byte back to 0.
  void reset();

 private:
  // TODO: backing store — one uint8_t per address
  // Hint: std::vector<uint8_t> data_;
  std::vector<uint8_t> data_;
};

}  // namespace sim
