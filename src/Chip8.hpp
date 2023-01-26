#pragma once

#include <cstdint>
#include <filesystem>
#include <random>

const int DISPLAY_SIZE = 64 * 32;

class Chip8 {
    uint8_t memory[4096]{};   //0x000 - 0x1FF: originally reserved for interpreter
                              //0x050 - 0x0A0: font
                              //0x200 - 0xFFF: ROM loaded from 0x200 onward, anything left
                              //if free for the ROM to use

    uint8_t v[16]{};          //general purpose registers, v[F] is also used as a flag
    uint16_t index{};         //used to store addresses for use in operations
    uint16_t PC{};            //holds address of next instruction to execute
    uint16_t stack[16]{};     //holds addresses during function calls
    uint8_t stackPointer{};   //points to top of stack, where to place next element

    uint8_t delayTimer{};     //If above 0, decrements at 60Hz. Stops at 0
    uint8_t soundTimer{};     //Decrements at 60Hz, tone will buzz when non-zero

    uint16_t opcode{};        //Opcodes are 12-bit


    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t> randByte;

public:
    uint32_t display[DISPLAY_SIZE]{};
    uint8_t keypad[16]{};
    bool drawFlag{};

    bool loadROM(const std::filesystem::path& path);

    Chip8();

    void tick();
};
