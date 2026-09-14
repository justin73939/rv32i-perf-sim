#include <iostream>
#include <string>
#include "sim/elf.hpp"
#include "sim/cpu.hpp"
#include "sim/memory.hpp"
#include "sim/regfile.hpp"

using sim::LoadElfResult;
using sim::load_elf;
using sim::Cpu;
using sim::FaultCause;
using sim::Memory;
using sim::RegisterFile;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "usage: sim --elf PATH\n";
    return 1;
  }

  if (std::string(argv[1]) != "--elf") {
    std::cerr << "usage: sim --elf PATH\n";
    return 1;
  }

  std::string elf_path = argv[2];

  // Test RAMs were hundreds of bytes. Real ELFs often load well above that.
  Memory mem(1024 * 1024);
  RegisterFile regs;
  Cpu cpu(mem, regs);

  LoadElfResult fileInfo = load_elf(elf_path, mem);
  if (!fileInfo.ok()) {
    std::cerr << fileInfo.error << "\n";
    return 1;
  }

  cpu.reset(fileInfo.entry);
  cpu.run();

  if (cpu.faulted()) {
    std::cerr << "faulted";
    if (cpu.fault_cause() == FaultCause::InstructionAddressMisaligned) {
      std::cerr << ": instruction address misaligned";
    }
    std::cerr << " pc=0x" << std::hex << cpu.pc() << "\n";
    return 1;
  }

  std::cout << "halted pc=0x" << std::hex << cpu.pc() << "\n";
  return 0;
}
