#include "sim/pipeline.hpp"

#include "sim/memory.hpp"
#include "sim/regfile.hpp"

namespace sim {

Pipeline::Pipeline(Memory& mem, RegisterFile& regs) : mem_(mem), regs_(regs) {}

void Pipeline::reset(uint32_t pc) {
  pc_ = pc;
  cycles_ = 0;
  halted_ = false;
  if_id_ = IfId{};
  id_ex_ = IdEx{};
  ex_mem_ = ExMem{};
  mem_wb_ = MemWb{};
}

void Pipeline::step() {
  if (halted_) {
    return;
  }

  cycles_ += 1;

  // Shift toward WB first so we still have IF/ID when we overwrite it.
  // Empty stages just copy bubbles (valid == false).
  mem_wb_.valid = ex_mem_.valid;
  mem_wb_.pc = ex_mem_.pc;
  mem_wb_.inst = ex_mem_.inst;

  ex_mem_.valid = id_ex_.valid;
  ex_mem_.pc = id_ex_.pc;
  ex_mem_.inst = id_ex_.inst;

  id_ex_.valid = if_id_.valid;
  id_ex_.pc = if_id_.pc;
  id_ex_.inst = if_id_.inst;

  // IF: fetch at current PC, park the word in IF/ID, then PC += 4.
  if_id_.valid = true;
  if_id_.pc = pc_;
  if_id_.inst = mem_.load_word(pc_);
  pc_ += 4;
}

uint32_t Pipeline::pc() const {
  return pc_;
}

uint32_t Pipeline::cycles() const {
  return cycles_;
}

bool Pipeline::halted() const {
  return halted_;
}

const IfId& Pipeline::if_id() const {
  return if_id_;
}

const IdEx& Pipeline::id_ex() const {
  return id_ex_;
}

const ExMem& Pipeline::ex_mem() const {
  return ex_mem_;
}

const MemWb& Pipeline::mem_wb() const {
  return mem_wb_;
}

}  // namespace sim
