#include <iostream>
#include <chrono>
#include <thread>
#include <cstdint>
#include "SDL.h"

#include "Chip8.hpp"

//keyboard input
uint8_t keymap[16] = {
        SDLK_x,
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_q,
        SDLK_w,
        SDLK_e,
        SDLK_a,
        SDLK_s,
        SDLK_d,
        SDLK_z,
        SDLK_c,
        SDLK_4,
        SDLK_r,
        SDLK_f,
        SDLK_v,
};

int main(int argc, char* argv[]){
    if(argc != 2){
        std::cout << "Usage: Chip8 [ROM]" << std::endl;
        return 1;
    }

    Chip8 Chip8;

    int width = 1024;
    int height = 512;

    // The window we'll be rendering to
    SDL_Window* window = nullptr;

    // Initialize SDL
    if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        exit(1);
    }
    // Create window
    window = SDL_CreateWindow(
            "CHIP-8 Emulator",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height, SDL_WINDOW_SHOWN
    );
    if (window == nullptr){
        printf( "Window could not be created! SDL_Error: %s\n",
                SDL_GetError() );
        exit(2);
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, width, height);

    // Create texture that stores frame buffer
    SDL_Texture* sdlTexture = SDL_CreateTexture(renderer,
                                                SDL_PIXELFORMAT_ARGB8888,
                                                SDL_TEXTUREACCESS_STREAMING,
                                                64, 32);

    // Temporary pixel buffer
    uint32_t pixels[2048];


    load:
    // Attempt to load ROM
    if (!Chip8.loadROM(argv[1]))
        return 2;

    // Emulation loop
    while (true) {
        Chip8.tick();

        // Process SDL events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) exit(0);

            // Process keydown events
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    exit(0);

                if (e.key.keysym.sym == SDLK_F1)
                    goto load;      // *gasp*, a goto statement!
                // Used to reset/reload ROM

                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        Chip8.keypad[i] = 1;
                    }
                }
            }
            // Process keyup events
            if (e.type == SDL_KEYUP) {
                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        Chip8.keypad[i] = 0;
                    }
                }
            }
        }

        // If draw occurred, redraw SDL screen
        if (Chip8.drawFlag) {
            Chip8.drawFlag = false;

            // Store pixels in temporary buffer
            for (int i = 0; i < 2048; ++i) {
                uint8_t pixel = Chip8.display[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }
            // Update SDL texture
            SDL_UpdateTexture(sdlTexture, nullptr, pixels, 64 * sizeof(Uint32));
            // Clear screen and render
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, sdlTexture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }

        // Sleep to slow down emulation speed
        std::this_thread::sleep_for(std::chrono::microseconds(1200));

    }
}