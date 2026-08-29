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
  fault_cause_ = FaultCause::None;
}

uint32_t Cpu::pc() const {
  return pc_;
}

bool Cpu::halted() const {
  return halted_;
}

bool Cpu::faulted() const {
  return fault_cause_ != FaultCause::None;
}

FaultCause Cpu::fault_cause() const {
  return fault_cause_;
}

void Cpu::step() {
  // IF (Instruction Fetch) -> ID (Instruction Decode) -> EX (Execute the computation) -> MEM (only for loads and store) -> WB (Write Back into destination register)
  //   Default for non-branch, non-jump, non-halt: pc_ += 4

  if (halted_ || faulted()){
    return;
  }
  uint32_t inst_word = mem_.load_word(pc_);
  DecodedInstr x = decode(inst_word);
  uint32_t next_pc = pc_ + 4;


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


  if (x.opcode == static_cast<uint8_t>(Opcode::Imm)) {
    uint32_t a = regs_.read(x.rs1);
    uint32_t imm = static_cast<uint32_t>(x.imm);

    if (x.funct3 == 0b0) {
      uint32_t result = alu(a, imm, AluOp::Add);
      regs_.write(x.rd, result);
    } 
    else if (x.funct3 == 0b001) {
      uint32_t shamt = imm & 0x1Fu;
      uint32_t result = alu(a, shamt, AluOp::Sll);
      regs_.write(x.rd, result);
    } 
    else if (x.funct3 == 0b010) {
      uint32_t result = alu(a, imm, AluOp::Slt);
      regs_.write(x.rd, result);
    } 
    else if (x.funct3 == 0b011) {
      uint32_t result = alu(a, imm, AluOp::Sltu);
      regs_.write(x.rd, result);
    } 
    else if (x.funct3 == 0b100) {
      uint32_t result = alu(a, imm, AluOp::Xor);
      regs_.write(x.rd, result);
    } 
    else if (x.funct3 == 0b101) {
      uint32_t shamt = imm & 0x1Fu;
      if (x.funct7 == 0b0) {
        uint32_t result = alu(a, shamt, AluOp::Srl);
        regs_.write(x.rd, result);
      } 
      else if (x.funct7 == 0b0100000) {
        uint32_t result = alu(a, shamt, AluOp::Sra);
        regs_.write(x.rd, result);
      }
    } 
    else if (x.funct3 == 0b110) {
      uint32_t result = alu(a, imm, AluOp::Or);
      regs_.write(x.rd, result);
    } 
    else if (x.funct3 == 0b111) {
      uint32_t result = alu(a, imm, AluOp::And);
      regs_.write(x.rd, result);
    }
  }


  if (x.opcode == static_cast<uint8_t>(Opcode::Load)){
    if (x.funct3 == 0b010){  // LW
      uint32_t a = regs_.read(x.rs1);
      uint32_t b = static_cast<uint32_t>(x.imm);
      uint32_t loadedWord = mem_.load_word(a+b);
      regs_.write(x.rd, loadedWord);
    }
    
  }


  if (x.opcode == static_cast<uint8_t>(Opcode::Store)){
    if (x.funct3 == 0b010){  // SW
      uint32_t a = regs_.read(x.rs1);
      uint32_t b = static_cast<uint32_t>(x.imm);
      uint32_t storedWord = regs_.read(x.rs2);
      mem_.store_word(a+b, storedWord);
    }
  }

  if (x.opcode == static_cast<uint8_t>(Opcode::Lui)){
    regs_.write(x.rd, static_cast<uint32_t>(x.imm));
  }

  if (x.opcode == static_cast<uint8_t>(Opcode::Auipc)){
    uint32_t a = pc();
    uint32_t address = a + static_cast<uint32_t>(x.imm);
    regs_.write(x.rd, address);
    
  }

  if (x.opcode == static_cast<uint8_t>(Opcode::Branch)) {
    uint32_t a = regs_.read(x.rs1);
    uint32_t b = regs_.read(x.rs2);
    bool take = false;

    if (x.funct3 == 0b000) {  // BEQ
      take = (a == b);
    } else if (x.funct3 == 0b001) {  // BNE
      take = (a != b);
    } else if (x.funct3 == 0b100) {  // BLT (signed)
      take = (static_cast<int32_t>(a) < static_cast<int32_t>(b));
    } else if (x.funct3 == 0b101) {  // BGE (signed)
      take = (static_cast<int32_t>(a) >= static_cast<int32_t>(b));
    } else if (x.funct3 == 0b110) {  // BLTU (unsigned)
      take = (a < b);
    } else if (x.funct3 == 0b111) {  // BGEU (unsigned)
      take = (a >= b);
    }

    if (take) {
      next_pc = pc_ + static_cast<uint32_t>(x.imm);
    }
  }

  if (x.opcode == static_cast<uint8_t>(Opcode::Jal)) {
    uint32_t target = pc_ + static_cast<uint32_t>(x.imm);
    if ((target & 0x3u) != 0) {
      fault_cause_ = FaultCause::InstructionAddressMisaligned;
      return;
    }

    regs_.write(x.rd, pc_ + 4);
    next_pc = target;
  }

  if (x.opcode == static_cast<uint8_t>(Opcode::Jalr)) {
    if (x.funct3 == 0b000) {  // JALR
      uint32_t target = regs_.read(x.rs1) +
                        static_cast<uint32_t>(x.imm);
      target &= ~1u;  // JALR clears the target address's least-significant bit.

      if ((target & 0x3u) != 0) {
        fault_cause_ = FaultCause::InstructionAddressMisaligned;
        return;
      }

      regs_.write(x.rd, pc_ + 4);
      next_pc = target;
    }
  }

  if (x.opcode == static_cast<uint8_t>(Opcode::System)){
    halted_=true;
  }

  pc_ = next_pc;

}

void Cpu::run(){
  while (!halted_ && !faulted()){
    step();
  }
}

}  // namespace sim
