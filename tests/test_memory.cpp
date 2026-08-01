#include "sim/memory.hpp"

#include <gtest/gtest.h>

using sim::Memory;

// Implement Memory until these go green.
// Spec context: RISC-V is little-endian; loads/stores use byte addresses.

TEST(Memory, StartsZero) {
  Memory mem(64);
  EXPECT_EQ(mem.size(), 64u);
  EXPECT_EQ(mem.load_word(0), 0u);
  EXPECT_EQ(mem.load8(0), 0u);
}

TEST(Memory, StoreLoadWord) {
  Memory mem(64);
  mem.store_word(0, 0xA1B2C3D4u);
  EXPECT_EQ(mem.load_word(0), 0xA1B2C3D4u);
}

TEST(Memory, LittleEndianByteOrder) {
  // After storing 0xA1B2C3D4 at address 0:
  //   addr 0 → 0xD4 (least significant byte)
  //   addr 1 → 0xC3
  //   addr 2 → 0xB2
  //   addr 3 → 0xA1 (most significant byte)
  Memory mem(64);
  mem.store_word(0, 0xA1B2C3D4u);
  EXPECT_EQ(mem.load8(0), 0xD4u);
  EXPECT_EQ(mem.load8(1), 0xC3u);
  EXPECT_EQ(mem.load8(2), 0xB2u);
  EXPECT_EQ(mem.load8(3), 0xA1u);
}

TEST(Memory, IndependentAddresses) {
  Memory mem(64);
  mem.store_word(0, 0x11111111u);
  mem.store_word(8, 0x22222222u);
  EXPECT_EQ(mem.load_word(0), 0x11111111u);
  EXPECT_EQ(mem.load_word(8), 0x22222222u);
}

TEST(Memory, Store8LoadWord) {
  Memory mem(64);
  mem.store8(0, 0x11u);
  mem.store8(1, 0x22u);
  mem.store8(2, 0x33u);
  mem.store8(3, 0x44u);
  // little-endian pack: 0x44332211
  EXPECT_EQ(mem.load_word(0), 0x44332211u);
}

TEST(Memory, OutOfRangeLoadIsZero) {
  Memory mem(16);
  EXPECT_EQ(mem.load8(100), 0u);
  EXPECT_EQ(mem.load_word(100), 0u);
}

TEST(Memory, OutOfRangeStoreIsIgnored) {
  Memory mem(16);
  mem.store8(100, 0xFFu);
  mem.store_word(0, 0x12345678u);
  mem.store_word(100, 0xFFFFFFFFu);  // should not crash; in-range word intact
  EXPECT_EQ(mem.load_word(0), 0x12345678u);
}

TEST(Memory, ResetClears) {
  Memory mem(32);
  mem.store_word(4, 0xDEADBEEFu);
  mem.reset();
  EXPECT_EQ(mem.load_word(4), 0u);
}
