#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sim {

class Memory;

// Result of trying to load an ELF into simulator Memory.
//   ok() == true  → Memory now holds the program; use `entry` with cpu.reset(entry)
//   ok() == false → Memory may be partially written; read `error` for why
struct LoadElfResult {
  uint32_t entry = 0;   // guest RAM address where execution should start (PC)
  std::string error;    // empty on success; human-readable reason on failure

  bool ok() const { return error.empty(); }
};

// Load a little-endian RV32 ELF from a host filesystem path into `mem`.
// Copies PT_LOAD segments; zero-fills BSS-style tails (memsz > filesz).
LoadElfResult load_elf(const std::string& path, Memory& mem);

// Same as load_elf, but the ELF bytes are already in memory (used by unit tests
// so we can build a tiny fake ELF without needing a RISC-V compiler).
LoadElfResult load_elf_bytes(const std::vector<uint8_t>& file, Memory& mem);

}  // namespace sim
