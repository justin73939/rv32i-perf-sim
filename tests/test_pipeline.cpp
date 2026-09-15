#include "sim/memory.hpp"
#include "sim/pipeline.hpp"
#include "sim/regfile.hpp"

#include <gtest/gtest.h>

using sim::Memory;
using sim::Pipeline;
using sim::RegisterFile;

TEST(Pipeline, ResetSetsPcAndClearsTrays) {
  Memory mem(256);
  RegisterFile regs;
  Pipeline pipe(mem, regs);

  pipe.reset(0x100);
  EXPECT_EQ(pipe.pc(), 0x100u);
  EXPECT_EQ(pipe.cycles(), 0u);
  EXPECT_FALSE(pipe.halted());
  EXPECT_FALSE(pipe.if_id().valid);
  EXPECT_FALSE(pipe.id_ex().valid);
  EXPECT_FALSE(pipe.ex_mem().valid);
  EXPECT_FALSE(pipe.mem_wb().valid);
}

TEST(Pipeline, OneClockFetchesIntoIfId) {
  // ADDI x1, x2, 3  →  0x00310093  (same encoding as Cpu tests)
  Memory mem(256);
  RegisterFile regs;
  Pipeline pipe(mem, regs);

  mem.store_word(0, 0x00310093u);
  pipe.reset(0);
  pipe.step();

  EXPECT_EQ(pipe.cycles(), 1u);
  EXPECT_EQ(pipe.pc(), 4u);  // fetch PC advanced; instr is not done (not WB)
  EXPECT_TRUE(pipe.if_id().valid);
  EXPECT_EQ(pipe.if_id().pc, 0u);
  EXPECT_EQ(pipe.if_id().inst, 0x00310093u);
  EXPECT_FALSE(pipe.id_ex().valid);
  EXPECT_EQ(regs.read(1), 0u);  // no writeback yet
}

TEST(Pipeline, SecondClockMovesInstrToIdEx) {
  Memory mem(256);
  RegisterFile regs;
  Pipeline pipe(mem, regs);

  mem.store_word(0, 0x00310093u);  // ADDI at 0
  mem.store_word(4, 0x003100B3u);  // ADD  at 4 (just a different word)
  pipe.reset(0);
  pipe.step();
  pipe.step();

  EXPECT_EQ(pipe.cycles(), 2u);
  EXPECT_EQ(pipe.pc(), 8u);

  EXPECT_TRUE(pipe.id_ex().valid);
  EXPECT_EQ(pipe.id_ex().pc, 0u);
  EXPECT_EQ(pipe.id_ex().inst, 0x00310093u);

  EXPECT_TRUE(pipe.if_id().valid);
  EXPECT_EQ(pipe.if_id().pc, 4u);
  EXPECT_EQ(pipe.if_id().inst, 0x003100B3u);

  EXPECT_FALSE(pipe.ex_mem().valid);
}
