#include <iostream>
#include "chip8.h"
#define SDL_MAIN_HANDLED
#include "C:/Coding Stuffs/SDL2-2.32.10/x86_64-w64-mingw32/include/SDL2/SDL.h"

using namespace std;

SDL_Keycode keymap[16] = {                                              //physical keyboard key mapped to each chip8 key

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


int main(int argc, char **argv) {


    int w = 1024;
    int h = 512;
    bool running = true;

    Chip8 chip8;

    cout << "Chip-8 Emulator loading...." << endl;

    if (argc != 2){
        cout << "Usage: chip8 <ROM file>" << endl;
        return 1;
    }
    
     if(!chip8.load(argv[1])){
        cout << "failed to load ROM" << endl;
        return 2;
    }

    if (SDL_Init(SDL_INIT_VIDEO)<0){
        cout << "SDL initialization failed: " << SDL_GetError() << endl;
        return 1;
    };

    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        w,h,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL){
        cout << "Window creation failed: " << SDL_GetError() << endl;
        SDL_Quit();
        return 1;
    }
    


    SDL_Event e;                                                

    while (running){

        chip8.emulate_cycle();                                  //fetch and execute instructions
        
        while(SDL_PollEvent (&e)){                              //process SDL events

            if(e.type == SDL_QUIT){                             //event type quit then exit
                running = false;
            }

            if (e.type == SDL_KEYDOWN && !e.key.repeat){        //event type key pressed

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
    }
    

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
