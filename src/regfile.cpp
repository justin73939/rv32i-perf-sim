#include "sim/regfile.hpp"

namespace sim {

RegisterFile::RegisterFile() {
  reset();
}

uint32_t RegisterFile::read(uint8_t reg) const {
  if (reg==0){
    return 0;
  }
  else{
    return regs_[reg];
  }
}

void RegisterFile::write(uint8_t reg, uint32_t value) {
  if (reg == 0){
    return;
  }
  else{
    regs_[reg] = value;
  }
}

void RegisterFile::reset() {
  for (int i = 0; i < 32; ++i){
    regs_[i] = 0;
  }
}

}  // namespace sim
