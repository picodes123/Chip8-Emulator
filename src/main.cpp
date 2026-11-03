#include <iostream>
#include <chrono>
#include <thread>
#include "chip8.h"
#define SDL_MAIN_HANDLED
#include "C:/Coding Stuffs/SDL2-2.32.10/x86_64-w64-mingw32/include/SDL2/SDL.h"

using namespace std;

SDL_Keycode keymap[16] = {                                      //physical keyboard key mapped to each chip8 key

    SDLK_x, //0
    SDLK_1, //1
    SDLK_2, //2
    SDLK_3, //3
    SDLK_q, //4
    SDLK_w, //5
    SDLK_e, //6
    SDLK_a, //7
    SDLK_s, //8
    SDLK_d, //9
    SDLK_z, //A
    SDLK_c, //B
    SDLK_4, //C
    SDLK_r, //D
    SDLK_f, //E
    SDLK_v  //F
};


int main(int argc, char **argv) {                               //argc is argument count, argv is array containing those arguments


    int w = 1024;                                               //width
    int h = 512;                                                //height
    bool running = true;

    auto lastTimerUpdate = std::chrono::steady_clock::now();    //gets the last timer update to get elapsed timer later
    auto lastCpuCycle = std::chrono::steady_clock::now();       //gets the time for the last cpu cycle 

    const int cpuFrequency = 400;                               //500 instruction per sec
    const auto cpuInterval = std::chrono::microseconds(1000000/cpuFrequency);  //2ms

    Chip8 chip8;

    cout << "Chip-8 Emulator loading...." << endl;

    if (argc != 2){                                             //returns error if argc is not 2
        cout << "Usage: chip8 <ROM file>" << endl;              //example: ./chip8 is 1 argc and ./chip8 pong.ch8 is 2 argc
        return 1;
    }
    
     if(!chip8.load(argv[1])){                                  //check rom loaded succesfully, if not print "failed to load ROM"
        cout << "failed to load ROM" << endl;                   //argv[0] = ./chip8 , argv[1] = pong.ch8
        return 2;
    }

    if (SDL_Init(SDL_INIT_VIDEO)<0){                            //initialize SDL
        cout << "SDL initialization failed: " << SDL_GetError() << endl;
        return 1;
    };

    SDL_Window* window = SDL_CreateWindow(                      //create window
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        w,h,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL){                                        //check if window creation failed
        cout << "Window creation failed: " << SDL_GetError() << endl;
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(                //Create renderer
        window, -1, 0);                                         //-1: lets SDL choose rendering driver, 0: doesnt request special renderer flag

    if (renderer == nullptr){                                   //check if renderer creation failed
        cout<< "Renderer creation failed: " << SDL_GetError() <<endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_RenderSetLogicalSize(renderer,w,h);                     //tells SDL what size drawing area to use

    SDL_Texture* screen = SDL_CreateTexture(                    //create 64x32 digital canvas where chip8 can draw pixels
        renderer,
        SDL_PIXELFORMAT_RGBA8888,                               //pixel format- red green blue alpha, 8888 means 8 bit allocated to each channel
        SDL_TEXTUREACCESS_STREAMING, 64, 32                     //streaming means update texture pixel repedeatly
    );
    
    if(screen == nullptr){                                      //print error if texture creation failed
        cout <<"texture creation failed: " << SDL_GetError()<<endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    uint32_t pixels[64 * 32];                                   //new array to give each pixel color

    SDL_Event e;                                                //define SDL events as e 

    while (running){

        auto currentCpuTime = std::chrono::steady_clock::now(); //gets current cpu time

        if(currentCpuTime - lastCpuCycle >= cpuInterval){       //check if enough time has passed
            chip8.emulate_cycle();                              //fetch and execute one instruction
            lastCpuCycle += cpuInterval;                        //update the last cpu cycle time= last time + cpu interval (this reduces timing drift)
        }

        
        auto currentTime = std::chrono::steady_clock::now();                     //gets current time
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(    //find elapsed time = current time - last time
            currentTime - lastTimerUpdate
        );

        if (elapsed >= std::chrono::milliseconds(16)){          //if elapsed >= 16ms
            chip8.update_timers();                              //update delay and sound timer
            lastTimerUpdate = currentTime;
        }
        
        
        while(SDL_PollEvent (&e)){                              //process SDL events

            if(e.type == SDL_QUIT){                             //event type quit then exit
                running = false;
            }

            if (e.type == SDL_KEYDOWN && !e.key.repeat){        //event type key pressed, key.repeat helps key holding down problem

                if (e.key.keysym.sym == SDLK_ESCAPE){           //if escape pressed exit
                    running = false;
                }

                for (int i = 0; i < 16; i++){                   //loop to check every key map we defined above
                    if (e.key.keysym.sym == keymap[i]){         //if pressed key matches with keymap defined above then calls key pressed fn

                        chip8.key_pressed(i);
                    } 
                }
            }
            
            if (e.type == SDL_KEYUP){                           //event type key released

                for (int i = 0; i < 16; i++){                   //loop to check every key map we defined above

                    if (e.key.keysym.sym == keymap[i]){         //if pressed released key matches with keymap defined above then calls key_released fn
                        
                        chip8.key_released(i);
                    }             
                }     
            }        
        }
        if (chip8.drawFlag){                                        //do all these steps only when drawflag is true
            chip8.drawFlag = false;                                 //set drawflag false, so drawflag doesnt keep as true forever
        
            for (int i = 0; i < 64*32; i++){                        //loop for each 2048 pixel to give them color, 0x00FF00FF: bright green ,0x000000FF: black                                               
                pixels[i] = chip8.gfx[i] ? 0x00FF00FF : 0x000000FF; //ternary operator- condition ? value_if_true : value_if_false
            }
            
            SDL_UpdateTexture(screen, nullptr, pixels, 64* sizeof(uint32_t)); //update SDL texture- transfer pi xel buffer into texture

            //clear screen and render
            SDL_RenderClear(renderer);                              //clear prev frame from window
            SDL_RenderCopy(renderer, screen, nullptr, nullptr);     //copy texture(screen) onto renderer
            SDL_RenderPresent(renderer);                            //display the renderer
        }
            //std::this_thread::sleep_for(std::chrono::microseconds(1200));//     //sleep currently commented cause of slow speed of emu
    }
    
    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
