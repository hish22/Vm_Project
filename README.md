# Vm_Project

This code implements a basic software-based CPU emulator written in C. It simulates a virtual machine architecture that executes a custom bytecode program stored in memory, complete with registers (R0-R3), an instruction pointer, and status flags. The emulator follows a classic **fetch-decode-execute cycle**, where it reads three-byte instructions from a byte array, determines the corresponding operation (such as arithmetic, memory manipulation, or control flow jumps), and processes the logic accordingly until it encounters a stop command.

## How to use it

```bash
gcc vm-08.c -o vm
./vm
```
