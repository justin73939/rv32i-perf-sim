#include <iostream>

// CLI still a stub. ELF loading lives in sim::load_elf / sim::load_elf_bytes.
// Next slice: wire --elf PATH into main, then reset(entry) + run().
int main() {
  std::cout << "rv32i-perf-sim: stub. Use sim_tests for ELF loader coverage.\n";
  return 0;
}
