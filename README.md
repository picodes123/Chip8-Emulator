# CHIP-8 Emulator

A CHIP-8 emulator built from scratch in **C++17 using SDL2**. It emulates the CHIP-8 virtual machine and runs any chip-8 supported roms.

## What is CHIP-8?

CHIP-8 is an interpreted programming language and virtual machine developed in the 1970s. It was originally designed to make it easier to develop simple games on early microcomputers.

CHIP-8 has its own memory, registers, instruction set, display, keyboard and timers. An emulator recreates these components in software, allowing CHIP-8 programs to run on modern computers.

### Technical Specifications

| Component | Specification |
|---|---|
| Memory | 4 KB (4096 bytes) |
| Memory Address Range | `0x000` – `0xFFF` |
| General-Purpose Registers | 16 registers (`V0`–`VF`), 8-bit each |
| Index Register | `I` (16-bit) |
| Program Counter | `PC` (16-bit) |
| Stack | 16 levels |
| Stack Pointer | `SP` |
| Delay Timer | 8-bit, 60 Hz |
| Sound Timer | 8-bit, 60 Hz |
| Display | 64 × 32 monochrome pixels |
| Keyboard | 16 hexadecimal keys (`0`–`F`) |
| Instruction Size | 2 bytes |
| Program Start Address | `0x200` |

### How CHIP-8 Works

CHIP-8 follows a simple fetch-decode-execute cycle:

1. **Fetch:** Read the next 2-byte instruction from memory using the Program Counter.
2. **Decode:** Identify the operation represented by the opcode.
3. **Execute:** Perform the operation, such as arithmetic, memory access, jumping or drawing sprites.
4. **Update:** Update registers, memory, timers and display.
5. **Repeat:** Continue executing instructions.

## Demo

- [Pong 2 Gameplay](Demo/PONG2.gif)
- [Tetris Gameplay](Demo/TETRIS.mp4)

## Features

- CHIP-8 CPU, memory and register implementation
- Opcode fetching, decoding and execution
- 64 × 32 monochrome graphics
- Sprite rendering and collision detection
- Keyboard input mapping
- Delay and sound timer logic
- ROM loading
- Adjustable CPU frequency

## Tech Stack

- C++17
- SDL2
- CMake

## Project Structure

```text
Chip8-Emulator/
├── Demo/
├── include/
│   └── chip8.h
├── roms/
│   ├── game roms/
│   └── test roms/
├── src/
│   ├── chip8.cpp
│   └── main.cpp
├── CMakeLists.txt
└── .gitignore
```

## Build and Run (Windows)

**Requirements:** C++17 compiler, CMake, SDL2 and MinGW-w64.

Clone the repository:

```bash
git clone https://github.com/picodes123/Chip8-Emulator.git
cd Chip8-Emulator
```

Configure and build:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER="C:/msys64/ucrt64/bin/g++.exe"
cmake --build build
```

Run a ROM:

```powershell
.\build\chip8.exe ".\roms\game roms\PONG2"
```

Update the SDL2 path in `CMakeLists.txt` if necessary.

## Keyboard Mapping

```text
CHIP-8          Keyboard
1 2 3 C         1 2 3 4
4 5 6 D         Q W E R
7 8 9 E         A S D F
A 0 B F         Z X C V
```

Press `ESC` to exit.

## Future Improvements

- SDL2 audio support

## References

Some resources I used while developing this project:

- [Austin Morlan – Building a CHIP-8 Emulator](https://austinmorlan.com/posts/chip8_emulator/)
- [CHIP-8 – Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)
