#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#define PIXEL_CHAR *
#define FONT_START 0x050
#define FONT_SIZE  (sizeof(font))
#define FONT_END   (FONT_START + FONT_SIZE)
#define MEM_SIZE   4096
#define PROG_START 0x200
#define PROG_MAX   (MEM_SIZE - PROG_START)
#define STACK_SIZE 16

extern const uint8_t font[80];

typedef enum {
    noStackError,
    stackOverflow,
    stackUnderflow
} stackError;

typedef enum {
    noFetchError,
    programCounterOverflow
} fetchError;

typedef struct {
    uint16_t stack[STACK_SIZE];
    uint16_t indexRegister;
    uint16_t PC;
    uint8_t memory[4096];
    uint8_t registers[16];
    uint8_t stackPointer;
    uint8_t delayTimer;
    uint8_t soundTimer;
    bool display[32][64];
    bool drawFlag;
} CHIP8;

void resetMachine (CHIP8 *chip);
void drawScreen (CHIP8 *chip);
int loadProgram (CHIP8 *chip, const char *path);
fetchError fetch (CHIP8 *chip, uint16_t *instruction);
stackError push (CHIP8 *chip, const uint16_t val);
stackError pop (CHIP8 *chip, uint16_t *returnValue);