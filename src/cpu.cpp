#include "sim/cpu.hpp"

#include "sim/alu.hpp"
#include "sim/decoder.hpp"
#include "sim/memory.hpp"
#include "sim/regfile.hpp"

namespace sim {

Cpu::Cpu(Memory& mem, RegisterFile& regs) : mem_(mem), regs_(regs) {}

void Cpu::reset(uint32_t pc) {
  pc_ = pc;
  halted_ = false;
}

uint32_t Cpu::pc() const {
  return pc_;
}

bool Cpu::halted() const {
  return halted_;
}

void Cpu::step() {
  // TODO: implement single-cycle execute
  //
  // If already halted, return immediately.
  //
  // === 1. FETCH ===
  //   inst_word = mem_.load_word(pc_);
  //
  // === 2. DECODE ===
  //   DecodedInstr d = decode(inst_word);
  //
  // === 3. EXECUTE (depends on opcode/format) ===
  //   Read operands from regfile (rs1, rs2 or imm).
  //   Pick AluOp from d.opcode / d.funct3 / d.funct7 (R-type ADD first).
  //   result = alu(a, b, op);
  //
  //   R-type example (ADD x1, x2, x3):
  //     a = regs_.read(d.rs1)
  //     b = regs_.read(d.rs2)
  //     result = alu(a, b, AluOp::Add)
  //     regs_.write(d.rd, result)
  //
  //   I-type (ADDI, etc.): b = d.imm instead of rs2
  //   Load (LW):  addr = regs_.read(rs1) + imm; result = mem_.load_word(addr); write rd
  //   Store (SW): addr = rs1 + imm; mem_.store_word(addr, regs_.read(rs2))
  //   LUI/AUIPC: special — imm or PC+imm into rd (not plain ALU Add)
  //   Branch/Jump: update PC from imm (later slice)
  //   System (0x73): ecall → set halted_ = true; return
  //
  // === 4. PC UPDATE ===
  //   Default for non-branch, non-jump, non-halt: pc_ += 4
  //
  // Start small: R-type ADD only, then grow opcode by opcode.
  if (halted_==true){
    return;
  }
  uint32_t inst_word = mem_.load_word(pc_);
  DecodedInstr x = decode(inst_word);

  if (x.opcode == static_cast<uint8_t>(Opcode::Op)){
    uint32_t a = regs_.read(x.rs1);
    uint32_t b = regs_.read(x.rs2);

    if (x.funct3 == 0b0){
      if (x.funct7 == 0b0){
        uint32_t result = alu(a, b, AluOp::Add);
        regs_.write(x.rd, result);
      }
      else if(x.funct7 == 0b0100000){
        uint32_t result = alu(a, b, AluOp::Sub);
        regs_.write(x.rd, result);
      }
    }
    else if (x.funct3 == 0b001){
      uint32_t result = alu(a, b, AluOp::Sll);
      regs_.write(x.rd, result);
    }
    else if (x.funct3 == 0b010){
      uint32_t result = alu(a, b, AluOp::Slt);
      regs_.write(x.rd, result);
    }
    else if (x.funct3 == 0b011){
      uint32_t result = alu(a, b, AluOp::Sltu);
      regs_.write(x.rd, result);
    }
    else if (x.funct3 == 0b100){
      uint32_t result = alu(a, b, AluOp::Xor);
      regs_.write(x.rd, result);
    }
    else if (x.funct3 == 0b101){
      if (x.funct7 == 0b0){
        uint32_t result = alu(a, b, AluOp::Srl);
        regs_.write(x.rd, result);
      }
      else if(x.funct7 == 0b0100000){
        uint32_t result = alu(a, b, AluOp::Sra);
        regs_.write(x.rd, result);
      }
    }
    else if (x.funct3 == 0b110){
      uint32_t result = alu(a, b, AluOp::Or);
      regs_.write(x.rd, result);
    }
    else if (x.funct3 == 0b111){
      uint32_t result = alu(a, b, AluOp::And);
      regs_.write(x.rd, result);
    }
  }

  if (x.opcode == static_cast<uint8_t>(Opcode::System)){
    halted_=true;
  }

  pc_ +=4;

}

}  // namespace sim
