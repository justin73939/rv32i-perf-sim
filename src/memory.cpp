#include "sim/memory.hpp"

namespace sim {

Memory::Memory(std::size_t size_bytes) : data_(size_bytes, 0) {}

std::size_t Memory::size() const {
  return data_.size();
}

uint8_t Memory::load8(uint32_t addr) const {
  // TODO: implement load8
  //
  // Input:  addr — byte address
  // Output: byte at that address, or 0 if addr >= size()
  (void)addr;
  return 0;
}

void Memory::store8(uint32_t addr, uint8_t value) {
  // TODO: implement store8
  //
  // Inputs: addr — byte address; value — byte to write
  // If addr is in range, write it; otherwise do nothing.
  (void)addr;
  (void)value;
}

uint32_t Memory::load_word(uint32_t addr) const {
  // TODO: implement load_word (little-endian)
  //
  // Read 4 bytes at addr .. addr+3 and pack into a uint32_t:
  //   result = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24)
  // where b0 is the byte at the lowest address (little-endian).
  //
  // Hint: you can call load8 four times.
  (void)addr;
  return 0;
}

void Memory::store_word(uint32_t addr, uint32_t value) {
  // TODO: implement store_word (little-endian)
  //
  // Split value into 4 bytes and write them:
  //   byte0 = value & 0xFF          → store at addr
  //   byte1 = (value >> 8) & 0xFF   → store at addr+1
  //   ...
  // Hint: you can call store8 four times.
  (void)addr;
  (void)value;
}

void Memory::reset() {
  // TODO: set every byte in data_ to 0
}

}  // namespace sim
