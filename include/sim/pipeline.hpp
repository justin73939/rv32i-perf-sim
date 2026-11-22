#pragma once

#include <cstdint>
#include <sim/decoder.hpp>

namespace sim {

class Memory;
class RegisterFile;

enum class FaultCause : uint8_t {
  None,
  InstructionAddressMisaligned,
};

// Trays between stages. Week 3 slice 1: only "is this slot real?", the
// instruction's PC, and the raw word. Later slices add rs1/rs2 values,
// ALU result, control bits, etc. — not here.

struct IfId {
  bool valid = false;  // false = bubble (empty tray)
  uint32_t pc = 0;     // PC of this instruction (needed later for AUIPC/branch)
  uint32_t inst = 0;   // 32-bit instruction word from RAM
};

struct IdEx {
  bool valid = false;
  DecodedInstr instr;
  uint32_t pc = 0;
  uint32_t rs1_val = 0;
  uint32_t rs2_val = 0;
};

struct ExMem {
  bool valid = false;
  DecodedInstr instr;
  uint32_t pc = 0;
  uint32_t rs1_val = 0;
  uint32_t rs2_val = 0;
  uint32_t result = 0;
};

struct MemWb {
  bool valid = false;
  DecodedInstr instr;
  uint32_t rs1_val = 0;
  uint32_t rs2_val = 0;
  uint32_t result = 0;
};

// 5-stage pipeline, no hazard hardware (week 3).
// step() = one clock, not one finished instruction (that is still Cpu::step()).
class Pipeline {
 public:
  Pipeline(Memory& mem, RegisterFile& regs);

  void reset(uint32_t pc = 0);

  // One clock: shift trays toward WB, then IF fills IF/ID and PC += 4.
  void step();

  uint32_t pc() const;
  uint32_t cycles() const;
  bool halted() const;

  // Test/debug: look at the trays. Not a RISC-V architectural register.
  const IfId& if_id() const;
  const IdEx& id_ex() const;
  const ExMem& ex_mem() const;
  const MemWb& mem_wb() const;

  FaultCause fault_cause() const;


 private:
  Memory& mem_;
  RegisterFile& regs_;
  uint32_t pc_ = 0;
  uint32_t cycles_ = 0;
  bool halted_ = false;

  IfId if_id_{};
  IdEx id_ex_{};
  ExMem ex_mem_{};
  MemWb mem_wb_{};

  FaultCause fault_cause_ = FaultCause::None;
};

}  // namespace sim
