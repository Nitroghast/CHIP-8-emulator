#include <stdbool.h>
#include <stdint.h>
#include "chip8.h"

const uint8_t font[80] = {
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

void resetMachine (CHIP8 *chip) {
    memset(chip, 0, sizeof(*chip));
    memcpy(chip->memory + FONT_START, font, sizeof(font));
    chip->PC = PROG_START;
}

int loadProgram (CHIP8 *chip, const char *path) {
    FILE *f;
    if ((f = fopen(path, "rb")) == NULL) {
        fprintf(stderr, "Can't open %s\n", path);
        return -1;
    }
    size_t n = fread(chip->memory + PROG_START, 1, PROG_MAX, f);
    uint8_t extra;
    if (fread(&extra, 1, 1, f) == 1) {
        fprintf(stderr, "ROM too big\n");
        fclose(f);
        return -1;
    }
    fclose(f);
    return (int) n;
}

fetchError fetch (CHIP8 *chip, uint16_t *instruction) {
    if (chip->PC >= 0xFFF) return programCounterOverflow;
    *instruction = ((uint16_t) chip->memory[chip->PC] << 8) | chip->memory[chip->PC + 1];
    chip->PC += 2;
    return noFetchError;
}

stackError push (CHIP8 *chip, uint16_t val) {
    if (chip->stackPointer >= STACK_SIZE) return stackOverflow;
    chip->stack[(chip->stackPointer)++] = val;
    return noStackError;
}

stackError pop (CHIP8 *chip, uint16_t *returnValue) {
    if (chip->stackPointer == 0) return stackUnderflow;
    *returnValue = chip->stack[--(chip->stackPointer)];
    return noStackError;
}