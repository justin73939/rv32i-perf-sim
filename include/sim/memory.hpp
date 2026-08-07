#pragma once

#include <cstdint>
#include <vector>

namespace sim {

// Flat byte-addressable memory for the single-cycle CPU (no cache yet).
// RISC-V is little-endian: lowest address holds the least-significant byte.
class Memory {
 public:
  // Allocate `size_bytes` of RAM, all zeros.
  explicit Memory(std::size_t size_bytes);

  // How many bytes this memory holds.
  std::size_t size() const;

  uint8_t load8(uint32_t addr) const;

  void store8(uint32_t addr, uint8_t value);

  uint32_t load_word(uint32_t addr) const;

  void store_word(uint32_t addr, uint32_t value);

  void reset();

 private:
  std::vector<uint8_t> data_;
};

}  // namespace sim
