#include "sim/pipeline.hpp"

#include "sim/memory.hpp"
#include "sim/regfile.hpp"
#include "sim/alu.hpp"

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


  ex_mem_.valid = id_ex_.valid;
  if (ex_mem_.valid){
    ex_mem_.instr = id_ex_.instr;
    ex_mem_.rs1_val = id_ex_.rs1_val;
    ex_mem_.rs2_val = id_ex_.rs2_val;

    if (ex_mem_.instr.opcode == static_cast<uint8_t>(Opcode::Op)){
      uint32_t a = ex_mem_.rs1_val;
      uint32_t b = ex_mem_.rs2_val;

      if (ex_mem_.instr.funct3 == 0b0){

        if (ex_mem_.instr.funct7 == 0b0){
          ex_mem_.result = alu(a, b, AluOp::Add);
        }

        else if(ex_mem_.instr.funct7 == 0b0100000){
          ex_mem_.result = alu(a, b, AluOp::Sub);
        }

      }

      else if (ex_mem_.instr.funct3 == 0b001){
        ex_mem_.result = alu(a, b, AluOp::Sll);
      }

      else if (ex_mem_.instr.funct3 == 0b010){
        ex_mem_.result = alu(a, b, AluOp::Slt);
      }

      else if (ex_mem_.instr.funct3 == 0b011){
        ex_mem_.result = alu(a, b, AluOp::Sltu);
      }

      else if (ex_mem_.instr.funct3 == 0b100){
        ex_mem_.result = alu(a, b, AluOp::Xor);
      }

      else if (ex_mem_.instr.funct3 == 0b101){

        if (ex_mem_.instr.funct7 == 0b0){
          ex_mem_.result = alu(a, b, AluOp::Srl);
        }

        else if(ex_mem_.instr.funct7 == 0b0100000){
          ex_mem_.result = alu(a, b, AluOp::Sra);
        }

      }
      
      else if (ex_mem_.instr.funct3 == 0b110){
        ex_mem_.result = alu(a, b, AluOp::Or);
      }

      else if (ex_mem_.instr.funct3 == 0b111){
        ex_mem_.result = alu(a, b, AluOp::And);
      }
    }

    if (ex_mem_.instr.opcode == static_cast<uint8_t>(Opcode::Imm)) {
      uint32_t a = ex_mem_.rs1_val;
      uint32_t imm = static_cast<uint32_t>(ex_mem_.instr.imm);
      uint32_t shamt = imm & 0x1Fu; // imm[4:0]
      uint32_t funct7 = (imm >> 5) & 0x7Fu; // imm[11:5]
  
      if (ex_mem_.instr.funct3 == 0b0) {
        ex_mem_.result = alu(a, imm, AluOp::Add);
      } 
      else if (ex_mem_.instr.funct3 == 0b001 && funct7 == 0) {
        ex_mem_.result = alu(a, shamt, AluOp::Sll);
      } 
      else if (ex_mem_.instr.funct3 == 0b010) {
        ex_mem_.result = alu(a, imm, AluOp::Slt);
      } 
      else if (ex_mem_.instr.funct3 == 0b011) {
        ex_mem_.result = alu(a, imm, AluOp::Sltu);
      } 
      else if (ex_mem_.instr.funct3 == 0b100) {
        ex_mem_.result = alu(a, imm, AluOp::Xor);
      } 
      else if (ex_mem_.instr.funct3 == 0b101) {
        if (funct7 == 0b0) {
          ex_mem_.result = alu(a, shamt, AluOp::Srl);
        } 
        else if (funct7 == 0b0100000) {
          ex_mem_.result = alu(a, shamt, AluOp::Sra);
        }
      } 
      else if (ex_mem_.instr.funct3 == 0b110) {
        ex_mem_.result = alu(a, imm, AluOp::Or);
      } 
      else if (ex_mem_.instr.funct3 == 0b111) {
        ex_mem_.result = alu(a, imm, AluOp::And);
      }

    }

    
  }

  else{
    ex_mem_.instr = DecodedInstr{};
    ex_mem_.rs1_val = 0;
    ex_mem_.rs2_val = 0;
    ex_mem_.result = 0;
  }


  // if IF succeeded, decode and set rs1 and rs2 values for EX
  // else IF is back to invalid, flush all non-exisitng values so not stale
  id_ex_.valid = if_id_.valid;
  if (id_ex_.valid){
    id_ex_.instr = decode(if_id_.inst);
    id_ex_.rs1_val = regs_.read(id_ex_.instr.rs1);
    id_ex_.rs2_val = regs_.read(id_ex_.instr.rs2);
  }

  else{
    id_ex_.instr = DecodedInstr{};
    id_ex_.rs1_val = 0;
    id_ex_.rs2_val = 0;
  }


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
