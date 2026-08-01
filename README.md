# rv32i-perf-sim

Cycle-accurate RISC-V RV32IM pipeline simulator (in progress).

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Requires CMake ≥ 3.20, a C++17 compiler, and network on first configure (GoogleTest via FetchContent).
