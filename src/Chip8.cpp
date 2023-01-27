#include "Chip8.hpp"

#include <fstream>
#include <vector>
#include <iostream>

const int START_ADDRESS = 0x200;
const int FONT_SIZE = 80;
const int FONT_START_ADDRESS = 0x50;

#define NOT_IMPLEMENTED do{ std::cerr << "Unimplemented Opcode" << std::endl; } while(0)


uint8_t fontset[FONT_SIZE] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

bool Chip8::loadROM(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if(file.is_open()){
        if(file.tellg() > 4096){
            std::cerr << "ROM is too large" << std::endl;
        }

        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});

        for(size_t i = 0; i < buffer.size(); i++){
            memory[START_ADDRESS + i] = buffer[i];
        }

        file.close();

        return true;
    }
    else {
        std::cerr << "Unable to open ROM" << std::endl;

        return false;
    }
}

Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
    PC = START_ADDRESS;

    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

    for(int i = 0; i < FONT_SIZE; i++){
        memory[FONT_START_ADDRESS + i] = fontset[i];
    }
}

void Chip8::tick() {
    //Fetch Instruction
    opcode = memory[PC] << 8 | memory[PC + 1];

    uint8_t X    = (opcode & 0x0F00) >> 8;
    uint8_t Y    = (opcode & 0x00F0) >> 4;
    uint8_t N    = opcode & 0x000F;
    uint8_t NN   = opcode & 0x00FF;
    uint16_t NNN = opcode & 0x0FFF;

    PC += 2;

    switch(opcode & 0xF000){
        //0x00E_
        case 0x0000: {
            switch(N){
                //0x00E0
                case 0x0000: {
                    for(unsigned int & i : display){
                        i = 0;
                    }
                    break;
                }
                //0x00EE
                case 0x000E: {
                    stackPointer--;
                    PC = stack[stackPointer];
                }
                default:{
                    std::cout << "Unknown Opcode: " << std::hex << std::uppercase << opcode << std::endl;
                    break;
                }
            }
            break;
        }
        //0x1NNN
        case 0x1000: {
            PC = NNN;
            break;
        }
        //0x2NNN
        case 0x2000: {
            stack[stackPointer] = PC;
            stackPointer++;

            PC = NNN;
            break;
        }
        //0x3XNN
        case 0x3000: {
            if(v[X] == NN){
                PC += 2;
            }
            break;
        }
        //0x4XNN
        case 0x4000: {
            if(v[X] != NN){
                PC += 2;
            }
            break;
        }
        //0x5XY0
        case 0x5000: {
            if(v[X] == v[Y]){
                PC += 2;
            }
            break;
        }
        //0x6XNN
        case 0x6000: {
            v[X] = NN;
            break;
        }
        //0x7XNN
        case 0x7000: {
            v[X] += NN;
            break;
        }
        //0x8XY_
        case 0x8000: {
            switch (N) {
                //0x8XY0
                case 0x0000: {
                    v[X] = v[Y];
                    break;
                }
                //0x8XY1
                case 0x0001: {
                    v[X] |= v[Y];
                    break;
                }
                //0x8XY2
                case 0x0002: {
                    v[X] &= v[Y];
                    break;
                }
                //0x8XY3
                case 0x0003: {
                    v[X] ^= v[Y];
                    break;
                }
                //0x8XY4
                case 0x0004: {
                    uint16_t sum = v[X] + v[Y];

                    if(sum > 255){
                        v[0xF] = 1;
                    }
                    else {
                        v[0xF] = 0;
                    }

                    v[X] = sum & 0xFF;

                    break;
                }
                //0x8XY5
                case 0x0005: {

                    if(v[X] - v[Y] < 0){
                        v[0xF] = 1;
                    }
                    else {
                        v[0xF] = 0;
                    }

                    v[X] -= v[Y];

                    break;
                }
                //0x8XY6
                case 0x0006: {

                    v[0xF] = v[X] & 0x1;

                    v[X] >>= 1;

                    break;
                }
                //0x8XY7
                case 0x0007: {
                    if(v[Y] - v[X] < 0){
                        v[0xF] = 1;
                    }
                    else {
                        v[0xF] = 0;
                    }

                    v[Y] -= v[X];
                    break;
                }
                //0x8XYE
                case 0x000E: {
                    v[0xF] = v[X] & 0x80;

                    v[X] <<= 1;

                    break;
                }
                default: {
                    NOT_IMPLEMENTED;
                    break;
                }
            }
            break;
        }
        //9XY0
        case 0x9000: {
            if(v[X] != v[Y]){
                PC += 2;
            }
            break;
        }
        //0xANNN
        case 0xA000: {
            index = NNN;
            break;
        }
        //0xBNNN
        case 0xB000: {
            PC = NNN + v[0];
            break;
        }
        //0xCXNN
        case 0xC000: {
            v[X] = randByte(randGen) & NN;
            break;
        }
        //0xDXYN
        case 0xD000: {
            uint16_t x = v[X];
            uint16_t y = v[Y];
            uint8_t height = N;
            uint32_t pixel;

            v[0xF] = 0;
            for(int yline = 0; yline < height; yline++){
                pixel = memory[index + yline];
                for(int xline = 0; xline < 8; xline++){
                    if((pixel & (0x80 >> xline)) != 0){
                        if(display[(x + xline + ((y + yline) * 64))] == 1){
                            v[0xF] = 1;
                        }
                        display[(x + xline + ((y + yline) * 64))] ^= 1;
                    }
                }
            }

            drawFlag = true;
            break;
        }
        //0xEX__
        case 0xE000: {
            switch (N) {
                //0xEX9E
                case 0x000E: {
                    if(keypad[v[X]]){
                        PC += 2;
                    }
                    break;
                }
                //0xEXA1
                case 0x0001: {
                    if(!keypad[v[X]]){
                        PC += 2;
                    }
                    break;
                }
                default: {
                    NOT_IMPLEMENTED;
                    break;
                }
            }
            break;
        }
        //0xFX__
        case 0xF000: {
            switch (NN) {
                //0xFX07
                case 0x0007: {
                    v[X] = delayTimer;
                    break;
                }
                //0xFX0A
                case 0x000A: {
                    for(int k = 0; k < 16; k++){
                        if(keypad[k]){
                            v[X] = k;
                            break;
                        }
                    }

                    PC -= 2;
                    break;
                }
                //0xFX15
                case 0x0015: {
                    delayTimer = v[X];
                    break;
                }
                //0xFX18
                case 0x0018: {
                    soundTimer = v[X];
                    break;
                }
                //0xFX1E
                case 0x001E: {
                    index += v[X];
                    break;
                }
                //0xFX29
                case 0x0029: {
                    index = FONT_START_ADDRESS + (v[X] * 5);
                    break;
                }
                //0xFX33
                case 0x0033: {
                    uint8_t value = v[X];

                    for(int i = 2; i >= 0; i--){
                        memory[index + i] = value % 10;
                        value /= 10;
                    }

                    break;
                }
                //0xFX55
                case 0x0055: {
                    for(int i = 0; i < X; i++){
                        memory[index + i] = v[i];
                    }

                    break;
                }
                //0xFX65
                case 0x0065: {
                    for(int i = 0; i < X; i++){
                        v[i] = memory[index + 1];
                    }

                    break;
                }
                default: {
                    NOT_IMPLEMENTED;
                    break;
                }
            }
            break;
        }
    }

    if(delayTimer > 0){
        delayTimer--;
    }

    if(soundTimer > 0){
        soundTimer--;
    }
}