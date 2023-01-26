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
                    NOT_IMPLEMENTED;
                    break;
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
            NOT_IMPLEMENTED;
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
        //0xANNN
        case 0xA000: {
            index = NNN;
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
    }

    if(delayTimer > 0){
        delayTimer--;
    }

    if(soundTimer > 0){
        soundTimer--;
    }
}