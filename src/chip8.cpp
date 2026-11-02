#include "chip8.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>



const unsigned char chip8_fontset[80]={     //each hexadeci char take 5byte so 16x5 =80 bytes array
    
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F

};



Chip8::Chip8(){
  
    std::memset(memory,0,sizeof(memory));
    std::memset(V,0,sizeof(V));
    std::memset(stack,0,sizeof(stack));
    std::memset(gfx,0,sizeof(gfx));
    std::memset(key,0,sizeof(key));

    std::memcpy(memory,chip8_fontset,sizeof(chip8_fontset));       //copies fontset into memory starting from 0x000


    pc = 0x200;
    I = 0;
    opcode = 0;
    sp = 0;
    sound_timer = 0;
    delay_timer = 0;
    drawFlag = false;
    waitingForKey = false;
    waitingRegister = 0;

};



bool Chip8::load(const char *file_path){

    std::ifstream rom(file_path, std::ios::binary | std::ios::ate);   //open file in binary from the last posn

    if(!rom.is_open()){                         //tells file opened or not
        return false;
    }

    std::streamsize rom_size = rom.tellg();     //check file posn and tell size

    if (rom_size <0 || rom_size> sizeof(memory)-0x200){     //check if rom size is not more than available memory
        return false;
    }

    rom.seekg(0,std::ios::beg);                //moves file pointer to start

    rom.read(reinterpret_cast<char*>(&memory[0x200]),rom_size);  //read rom_size byte and store them in memory, starting from 0x200
    
    if(rom.gcount() != rom_size){                //verify if rom read was succesful
        return false;
    }

    return true;
}



void Chip8::key_pressed(std::uint8_t key_index){

    if(key_index>=16){                           //prevent invalid chip8 key indexes
        return;
    }

    key[key_index]=1;                            //marks key pressed

    if(waitingForKey){
        V[waitingRegister]=key_index;            //store pressed key in the register
        waitingForKey = false;                   //no more to wait, can execute next instruction
    }
}



void Chip8::key_released(std::uint8_t key_index){  //check key released

    if(key_index>=16){                             //prevent invalid chip8 key indexes
        return;
    }

    key[key_index]=0;                              
}


void Chip8::emulate_cycle(){

    if(waitingForKey){                                    //if waiting for key then return dont fetch another opcode
        return;
    }

    opcode = (memory[pc] << 8) | memory[pc+1];            //left shift first byte then combine with 2nd byte to get opcode

    pc+=2;                                                //increment pc by 2


    switch (opcode & 0xF000){

    case 0x0000:{      

        switch (opcode){
            case 0x00E0:{                                 //its used to clear the screen pixels

                std::memset(gfx,0,sizeof(gfx));           //set gfx 0
                drawFlag = true;                          //tells to draw to SDL
                break;
            }     
            
            case 0x00EE:{                                 //used to return from the subroutine     
                if(sp > 0){
                sp--;                                     //move SP back one position
                pc=stack[sp];                             //PC restores the return address
                }                                         
                break;
            }
            default:
                std::cout << "unknown 0NNN opcode: 0x" << std::hex << opcode <<std::dec << std::endl;

                break;

        }
        break;
    }
    
    case 0x1000:{                                         //jump PC to 0x0NNN

        pc = opcode & 0x0FFF;                  

        break;
    }

    case 0x2000:{                                         //used to store return address and call subroutine                        
        if(sp < 16){
            stack[sp] = pc;                               //store pc address in stack
            sp++;                                         //move to next stack posn
            pc = opcode & 0x0FFF;                         //jump to address 0x0NNN
        }
        break;
    }

    case 0x3000:{                                         // if VX is equal to NN then increment PC by 2

        if (V[(opcode & 0x0F00)>>8] == (opcode & 0x00FF)){
            pc+=2;
        }
        break;
    }

    case 0x4000:{                                         // if VX is not equal to NN then increment PC by 2

        if(V[(opcode & 0x0F00)>>8] != (opcode & 0x00FF)){
            pc+=2;
        }

        break;
    }

    case 0x5000:{                                         // if VX is equal to VY then increment PC by 2

        if((opcode & 0x000F)== 0x0000){                   // confirm N is equal to 0

            if(V[(opcode & 0x0F00)>>8] == V[(opcode & 0x00F0)>>4]){
            pc+=2;
            }
        }

        break;
    }

    case 0x6000:                                          //opcode to set VX = NN
        V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);    //extract VX and NN and equate them
        break;
    
    case 0x7000:                                          //opcode to set VX = VX + NN
        V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF); 
        break;

    case 0x8000:{                                         //opcode for VX and VY
        std::uint8_t x = (opcode & 0x0F00)>>8;            //VX
        std::uint8_t y = (opcode & 0x00F0)>>4;            //VY

        switch (opcode & 0x000F){

            case 0x0000:{                                 // puts value of VY in VX

                V[x]=V[y];                                // sets VX = VY

                break;
            }

            case 0x0001:{                                 //finds "or" of VX and VY and put it in VX

                V[x]|=V[y];                               //VX= VX | VY

                break;
            }

            case 0x0002:{                                 //finds "AND" of VX and VY and put it in VX

                V[x]&=V[y];                               //VX= VX & VY

                break;
            }

            case 0x0003:{                                 //finds "xor" of VX and VY and put it in VX

                V[x]^=V[y];                               //VX= VX ^ VY

                break;
            }

            case 0x0004:{                                 //addition VX=VX+VY, VF = 1 when carry, else 0 when no carry

                std::uint16_t sum = V[x] + V[y];
                if(sum > 255){                            //check carry
                 V[15]=1;                                 //carry
                }
                else {
                 V[15] = 0;                               //no carry
                }

                V[x]=sum;

                break;       
            }

            case 0x0005:{                                   //subs VX=VX-VY, VF = 1 when no borrow, else VF = 0 when borrow

                if (V[x]>=V[y]){                            // checks Vx>=Vy
                 V[15]=1;                                   // no borrow
                }
                else{
                 V[15]=0;                                   // borrow
                }
                V[x]-=V[y];                                 //calculates

                break;
            }

            case 0x0006:{                                   //rightshift the Vx by 1 bit and put that shifted 1bit in VF

                V[15] = V[x] & 0x01;                        //put last Vx bit in VF
                V[x] >>= 1;                                 //right shift Vx by 1 bit

                break;

            }

            case 0x0007:{                                   //8XY7- VX = VY - VX

                if(V[(opcode & 0x00F0)>>4] >= V[x]){        //check VY is greater than or equal to VX
                    
                    V[0xF] = 1;                             //no borrow
                }
                else{
                    V[0xF] = 0;                             //borrow
                }

                V[x] = V[(opcode & 0x00F0)>>4] - V[x];      //VX = VY - VX
                break;
            }

            case 0x000E:{                                   //leftshift the Vx by 1 bit and put that shifted 1bit in VF          

                V[15]= (V[x] & 0x80) >> 7;                  //put first(left) VX bit in VF
                V[x]<<= 1;                                  //left shift VX by 1 bit

                break;
            }

            default:
                std::cout << "unknown 8XYN opcode: 0x" << std::hex << opcode << std::dec << std::endl;

                break;
        }

        break;
    }

    case 0x9000:{                                          // if VX is not equal to VY then increment PC by 2
        if((opcode & 0x000F) == 0x0000){                   // confirm N is equal to 0

            if(V[(opcode & 0x0F00)>>8] != V[(opcode & 0x00F0)>>4]){
            pc+=2;
            }
        }

        break;
    }
    
    case 0xA000:{                                          //set I to address NNN
        I = opcode & 0x0FFF;

        break;
    }

    case 0xB000:{
        pc = (opcode & 0x0FFF) + V[0];                      //jump to the address NNN plus V[0]

        break;
    }

    case 0xC000:{                                           //set VX to rand no. masked by NN

        std::uint8_t x = (opcode & 0x0F00)>>8;              //extract VX
        std::uint8_t nn = opcode & 0x00FF;                  //extract NN

        std::uint8_t random_number = std::rand() % 256;     //extract random no.

        V[x]=random_number & nn;                            //find VX value by 'AND' btween random no. and NN

        break;
    }

        // DXYN: Draws an 8xN sprite at coordinates (VX, VY).
        // Each row consists of 8 bits, read from memory starting at address I.
        // Pixels are drawn using XOR, so existing set pixels are turned off.
        // VF is set to 1 if any pixel is erased (collision), otherwise 0.
        // The value of I remains unchanged.

    case 0xD000:{                                           

        std::uint8_t x = V[(opcode & 0x0F00)>>8];           //extract x,y from XY and height from N
        std::uint8_t y = V[(opcode & 0x00F0)>>4];
        std::uint8_t height = opcode & 0x000F;

        V[0xF] = 0;                                         //set VF or V[15] to 0

        for (int yline = 0; yline < height; yline++){       //loop for every row

            std::uint8_t sprite_byte = memory[I + yline];   //read one row of sprite data from memory

            for(int xline = 0; xline < 8; xline++){         //goes through each bit of xline

                if(sprite_byte & (0x80 >> xline)){          //compare collision using 'AND' with 0x80 shifting each to right everytime till 8, to check if current sprite is ON
                    int screenX = (x+xline) % 64;           
                    int screenY = (y+yline) % 32;

                    int index = screenY * 64 + screenX;     //convert 2d to 1d

                    if (gfx[index]==1){                     //check pixel is already ON
                        V[0xF]=1;                           //if yes then collision
                    }
                    
                    gfx[index] ^= 1;                        //XOR for drawing sprite (or to flip 0 and 1)
                }
            }
        }
        drawFlag = true;                                    //tells SDL to draw
        break;
    }

    case 0xE000:{                                           //keyboard instruction

        std::uint8_t x = (opcode & 0x0F00)>>8;              //x = register in opcode instruction

        switch (opcode & 0x00FF){

            case 0x009E:{                                   //EX9E- skip next instruction if key stored in VX is pressed
                if(V[x] < 16 && key[V[x]]==1){              //1 = key pressed
                    pc += 2;
                }
                break;
            }

            case 0x00A1:{                                   //EXA1- skip next instruction if key stored in VX is not pressed
                if(V[x] < 16 && key[V[x]]==0){              //0 = key not pressed
                    pc += 2;
                }

                break;
            }
        
        default:
            std::cout << "unknown EXNN opcode: 0x" << std::hex << opcode << std::dec << std::endl;    

            break;
        }

        break;
    }

    case 0xF000:{

        std::uint8_t x = (opcode & 0x0F00)>>8;             //extract V register

        switch (opcode & 0x00FF){

            case 0x0007:{                                  //FX07- sets VX to the value of delay timer
                V[x]=delay_timer;
                break;
            }

            case 0x000A:{
                waitingRegister = (opcode & 0x0F00)>>8;    //which register will recieve key number
                waitingForKey = true;                      //wait until further key pressed

                break;
            }

            case 0x0015:{                                  //FX15 - sets delay timer to VX
                delay_timer = V[x];

                break;
            }

            case 0x0018:{                                  //FX18 - sets sound timer to VX
                sound_timer = V[x];

                break;
            }

            case 0x001E:{                                  //FX1E - adds VX to I and store in I
                if(I+V[x]>0x0FFF){                         //as I is 16bit and VX is 8 bit, check overflow 
                    V[0xF]=1;                              //carry 1 to VF
                }

                else{
                    V[0xF]=0;                              //else carry VF = 0
                }

                I+=V[x];                                   //I = I+VX
                break;
            }

            case 0x0029:{                                  //FX29- set I to the sprite address for digit in VX
                I = V[x]*5;                                //each font takes 5byte so multiply VX with 5

                break;
            }

            case 0x0033:{                                  //FX33- BCD conversion, store register value in seperate ones,tens,hundreads in I
                memory[I] = V[x]/100;                      //hundreads
                memory[I+1] = (V[x]/10)%10;                //tens
                memory[I+2] = V[x]%10;                     //ones

                break;
            }

            case 0x0055:{                                  //FX55- stores V0 to VX in memory starting at address I
                for (int i = 0; i <= x; i++){
                    memory[I+i] = V[i];
                    
                }
                I += x + 1;                                 //after putting V0 to VX, point I to next available memory

                break;
            }

             case 0x0065:{                                  //FX65- loads memory starting at address I in V0 to VX
                for (int i = 0; i <= x; i++){
                    V[i] = memory[I+i];
                    
                }
                I += x + 1;                                 //point I to next available memory

                break;
            }
        
        default:
            std::cout << "unknown FXNN opcode: 0x" << std::hex << opcode << std::dec << std::endl;

            break;
        }

        break;
    }

    default: 
        std::cout << "unknown opcode: 0x" << std::hex << opcode << std::dec << std::endl;
        break;
    }
    
}

std::uint8_t Chip8::get_register(std::uint8_t index)const {
    return V[index];
}

void Chip8::set_memory(std::uint16_t address, std::uint8_t value) {
    memory[address] = value;
}