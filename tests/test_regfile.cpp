#include "sim/regfile.hpp"

#include <gtest/gtest.h>

using sim::RegisterFile;

// Implement RegisterFile until these go green.
// Spec: RISC-V Unprivileged ISA — §2.1 "Programmers' Model" (x0 hardwired zero).

TEST(Regfile, StartsZero) {
  RegisterFile rf;
  for (uint8_t i = 0; i < 32; ++i) {
    EXPECT_EQ(rf.read(i), 0u) << "x" << static_cast<int>(i);
  }
}

TEST(Regfile, WriteReadNonZero) {
  RegisterFile rf;
  rf.write(5, 0xDEADBEEFu);
  EXPECT_EQ(rf.read(5), 0xDEADBEEFu);
}

TEST(Regfile, X0AlwaysZero) {
  RegisterFile rf;
  // Attempt to write x0 — must be ignored
  rf.write(0, 0x12345678u);
  EXPECT_EQ(rf.read(0), 0u);
}

TEST(Regfile, WriteX0DoesNotClobberOthers) {
  RegisterFile rf;
  rf.write(1, 42u);
  rf.write(0, 99u);  // should not affect x1
  EXPECT_EQ(rf.read(1), 42u);
  EXPECT_EQ(rf.read(0), 0u);
}

TEST(Regfile, ResetClearsRegisters) {
  RegisterFile rf;
  rf.write(3, 0xFFFFFFFFu);
  rf.write(10, 1u);
  rf.reset();
  EXPECT_EQ(rf.read(3), 0u);
  EXPECT_EQ(rf.read(10), 0u);
  EXPECT_EQ(rf.read(0), 0u);
}

TEST(Regfile, IndependentRegisters) {
  RegisterFile rf;
  rf.write(2, 100u);
  rf.write(3, 200u);
  EXPECT_EQ(rf.read(2), 100u);
  EXPECT_EQ(rf.read(3), 200u);
}
