#ifndef CHIP8_H
#define CHIP8_H
#include <cstdint>

class Chip8 {

    private:

    std::uint8_t memory[4096]; //store 4096 byte of memory
    std::uint8_t V[16];        //registers V0-VF

    std::uint16_t pc;          //program counter, address of next instruction
    std::uint16_t I;           //index register, used for memory addresses 
    std::uint16_t opcode;      //current instruction
    
    std::uint16_t stack[16];   //returns upto 16 return addresses
    std::uint8_t sp;           //tracks current stack position

    std::uint8_t delay_timer;  //decreases at 60hz
    std::uint8_t sound_timer;  //controls the beep

    bool waitingForKey;        //tells us if emulator waiting for key press
    std::uint8_t waitingRegister;  //remember which register stores the key pressed

    public:

    std::uint8_t gfx[64*32];   //graphic buffer 64*32
    std::uint8_t key[16];      //chip-8 has 16 keys
    bool drawFlag;             //indicates a draw has occured  

    Chip8();

    void update_timers();      //for updating timers
    void emulate_cycle();      //emulate one cycle
    bool load(const char *file_path);   //load ROM

    void key_pressed(std::uint8_t key_index);
    void key_released(std::uint8_t key_index);

};


#endif