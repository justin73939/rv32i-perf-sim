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
  fault_cause_ = FaultCause::None;

  if_id_ = IfId{};
  id_ex_ = IdEx{};
  ex_mem_ = ExMem{};
  mem_wb_ = MemWb{};
}

bool Pipeline::faulted() const {
  return fault_cause_ != FaultCause::None;
}

FaultCause Pipeline::fault_cause() const {
  return fault_cause_;
}

void Pipeline::step() {
  if (halted_) {
    return;
  }

  cycles_ += 1;

  // Shift toward WB first so we still have IF/ID when we overwrite it.
  // Empty stages just copy bubbles (valid == false).
  if (mem_wb_.valid){
    const auto op = mem_wb_.instr.opcode;
    if (op == static_cast<uint8_t>(Opcode::Op) || op == static_cast<uint8_t>(Opcode::Imm) ||
        op == static_cast<uint8_t>(Opcode::Load) || op == static_cast<uint8_t>(Opcode::Lui) ||
        op == static_cast<uint8_t>(Opcode::Auipc) || op == static_cast<uint8_t>(Opcode::Jal) || 
        op == static_cast<uint8_t>(Opcode::Jalr)){ regs_.write(mem_wb_.instr.rd, mem_wb_.result);}
  }

  mem_wb_.valid = ex_mem_.valid;

  if (mem_wb_.valid){
    mem_wb_.instr = ex_mem_.instr;
    mem_wb_.rs1_val = ex_mem_.rs1_val;
    mem_wb_.rs2_val = ex_mem_.rs2_val;
    mem_wb_.result = ex_mem_.result;

    if (mem_wb_.instr.opcode == static_cast<uint8_t>(Opcode::Load)){
      if (mem_wb_.instr.funct3 == 0b010){  // LW
        mem_wb_.result = mem_.load_word(mem_wb_.result);
      }
    }

    if (mem_wb_.instr.opcode == static_cast<uint8_t>(Opcode::Store)){
      if (mem_wb_.instr.funct3 == 0b010){  // SW
        mem_.store_word(mem_wb_.result, mem_wb_.rs2_val);
      }
    }
  }

  else{
    mem_wb_.instr = DecodedInstr{};
    mem_wb_.rs1_val = 0;
    mem_wb_.rs2_val = 0;
    mem_wb_.result = 0;
  }

  ex_mem_.valid = id_ex_.valid;
  if (ex_mem_.valid){
    ex_mem_.instr = id_ex_.instr;
    ex_mem_.pc = id_ex_.pc;
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

    if (ex_mem_.instr.opcode == static_cast<uint8_t>(Opcode::Load)){
      if (ex_mem_.instr.funct3 == 0b010){  // LW
        uint32_t a = ex_mem_.rs1_val;
        uint32_t b = static_cast<uint32_t>(ex_mem_.instr.imm);
        ex_mem_.result = (a+b);
      }
    }

    if (ex_mem_.instr.opcode == static_cast<uint8_t>(Opcode::Store)){
      if (ex_mem_.instr.funct3 == 0b010){  // SW
        uint32_t a = ex_mem_.rs1_val;
        uint32_t b = static_cast<uint32_t>(ex_mem_.instr.imm);
        ex_mem_.result = (a+b);
      }
    }

    if (ex_mem_.instr.opcode == static_cast<uint8_t>(Opcode::Lui)){
      ex_mem_.result = static_cast<uint32_t>(ex_mem_.instr.imm);
    }

    if (ex_mem_.instr.opcode == static_cast<uint8_t>(Opcode::Auipc)){
      uint32_t a = ex_mem_.pc;
      ex_mem_.result = a + static_cast<uint32_t>(ex_mem_.instr.imm);
    }

    if (ex_mem_.instr.opcode == static_cast<uint8_t>(Opcode::Branch)) {
      uint32_t a = ex_mem_.rs1_val;
      uint32_t b = ex_mem_.rs2_val;
      bool take = false;
  
      if (ex_mem_.instr.funct3 == 0b000) {  // BEQ
        take = (a == b);
      } else if (ex_mem_.instr.funct3 == 0b001) {  // BNE
        take = (a != b);
      } else if (ex_mem_.instr.funct3 == 0b100) {  // BLT (signed)
        take = (static_cast<int32_t>(a) < static_cast<int32_t>(b));
      } else if (ex_mem_.instr.funct3 == 0b101) {  // BGE (signed)
        take = (static_cast<int32_t>(a) >= static_cast<int32_t>(b));
      } else if (ex_mem_.instr.funct3 == 0b110) {  // BLTU (unsigned)
        take = (a < b);
      } else if (ex_mem_.instr.funct3 == 0b111) {  // BGEU (unsigned)
        take = (a >= b);
      }
  
      if (take) {
        pc_ = ex_mem_.pc + static_cast<uint32_t>(ex_mem_.instr.imm);

        if_id_.valid = false;
        if_id_.pc = 0;
        if_id_.inst = 0;
      }
    }

    if (ex_mem_.instr.opcode == static_cast<uint8_t>(Opcode::Jal)) {
      ex_mem_.result = ex_mem_.pc + 4;
      if (((ex_mem_.pc + static_cast<uint32_t>(ex_mem_.instr.imm)) & 0x3u) != 0) {
        fault_cause_ = FaultCause::InstructionAddressMisaligned;
        return;
      }
  
      pc_ = ex_mem_.pc + static_cast<uint32_t>(ex_mem_.instr.imm);

      if_id_.valid = false;
      if_id_.pc = 0;
      if_id_.inst = 0;
    }

    if (ex_mem_.instr.opcode == static_cast<uint8_t>(Opcode::Jalr)) {
      if (ex_mem_.instr.funct3 == 0b000) {  // JALR
        ex_mem_.result = ex_mem_.pc + 4;
        uint32_t target = ex_mem_.rs1_val + static_cast<uint32_t>(ex_mem_.instr.imm);
        target &= ~1u;  // JALR clears the target address's least-significant bit.
  
        if ((target & 0x3u) != 0) {
          fault_cause_ = FaultCause::InstructionAddressMisaligned;
          return;
        }
  
        pc_ = target;

        if_id_.valid = false;
        if_id_.pc = 0;
        if_id_.inst = 0;
      }
    }

    if (ex_mem_.instr.opcode == static_cast<uint8_t>(Opcode::System)){
      halted_ = true;
    }
      
  }

  else{
    ex_mem_.instr = DecodedInstr{};
    ex_mem_.pc = 0;
    ex_mem_.rs1_val = 0;
    ex_mem_.rs2_val = 0;
    ex_mem_.result = 0;
  }

    // if IF succeeded, decode and set rs1 and rs2 values for EX
  // else IF is back to invalid, flush all non-exisitng values so not stale
  id_ex_.valid = if_id_.valid;
  if (id_ex_.valid){
    id_ex_.instr = decode(if_id_.inst);
    id_ex_.pc = if_id_.pc;
    id_ex_.rs1_val = regs_.read(id_ex_.instr.rs1);
    id_ex_.rs2_val = regs_.read(id_ex_.instr.rs2);
  }

  else{
    id_ex_.instr = DecodedInstr{};
    id_ex_.pc = 0;
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
