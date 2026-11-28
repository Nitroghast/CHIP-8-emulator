#include "chip8.h"

int main () {
    srand(time(NULL));
    CHIP8 chip = {0};
    resetMachine(&chip);
    //Recieve program path info and load it into the chip. For testing purposes, it'll be just set in the code
    const char path[] = "IBM_Logo.ch8";
    int bytesRead = loadProgram(&chip, path);
    char b = '\0';
    int r = 0;
    bool invalidOpcode = false;
    printf(
        "Which behavior is the expected for the instructions that change between\n"
        "the original COSMAC VIP and more modern implementations of CHIP-8?\n"
        "Type 'C' for cosmac, 'M' for modern: "
    );
    while ((r = scanf(" %c", &b)) == 1 && toupper(b) != 'C' && toupper(b) != 'M') {
        scanf("%*[^\n]");
        printf("Insert valid value.\n");
    }
    b = toupper(b);
    if (r != 1) {
        return 1;
    }
    scanf("%*[^\n]");
    //Just before main loop
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    double initialTime = t.tv_sec + t.tv_nsec / 1e9;
    fetchError var = noFetchError;
    //Main loop
    while (true) {
        uint16_t instruction = 0;
        int result = 0;
        var = fetch(&chip, &instruction);
        uint8_t opcode = (instruction & 0xF000) >> 12;
        uint8_t x = (instruction & 0x0F00) >> 8;
        uint8_t y = (instruction & 0x00F0) >> 4;
        uint8_t n = instruction & 0x000F;
        uint8_t nn = instruction & 0x00FF;
        uint8_t nnn = instruction & 0x0FFF;
        if (var) {
            fprintf(stderr, "PC overflow\n");
            break;
        }
        stackError stackIssue = noStackError;
        /*
        Those sets of instructions that have fixed parts of the opcode that vary
        among different operations in the set (e.g. set is 0x0, instructions are 0x00E0 or 0x00EE),
        those parts of the opcode that vary will be indicated in comments via the letter M (e.g. 0x00EM)
        */
        switch (opcode) {
            case 0x0: //0x00EM
                if (n == 0) { //0x00E0, clear display
                    memset(&(chip.display), 0, sizeof(chip.display));
                    chip.drawFlag = true;
                } else if (n == 0xE) { //0x00EE, return from subroutine
                    stackIssue = pop(&chip, chip.PC);
                } else {
                    invalidOpcode = true;
                }
                break;
            case 0x1: //0x1NNN, unconditional jump to address NN
                chip.PC = nnn;
                break;
            case 0x2: //0x2NNN, unconditional jump with link to address NN
                stackIssue = push(&chip, chip.PC);
                chip.PC = nnn;
                break;
            case 0x3: //0x3XNN, conditional skip (V[X] == NN)
                if (chip.registers[x] == nn) {
                    chip.PC += 2;
                }
                break;
            case 0x4: //0x4XNN, conditional skip (V[X] != NN)
                if (chip.registers[x] != nn) {
                    chip.PC += 2;
                }
                break;
            case 0x5: //0x5XY0, conditional skip (V[X] == V[Y])
                if (chip.registers[x] == chip.registers[y]) {
                    chip += 2;
                }
                break;
            case 0x6: //0x6XNN, set V[X] to NN
                chip.registers[x] = nn;
                break;
            case 0x7: //0x7XNN, add NN to V[X]
                chip.registers[x] += nn;
                break;
            case 0x8: //0x8XYN, logical and arithmetic operations
                switch (n) {
                    case 0x0: //0x8XY0, set V[X] to V[Y]
                        chip.registers[x] = chip.registers[y];
                        break;
                    case 0x1: //0x8XY1, binary OR between V[X] and V[Y], saved in V[X]
                        chip.registers[x] |= chip.registers[y];
                        break;
                    case 0x2: //0x8XY2, binary AND between V[X] and V[Y], saved in V[X]
                        chip.registers[x] &= chip.registers[y];
                        break;
                    case 0x3: //0x8XY3, binary XOR between V[X] and V[Y], saved in V[X]
                        chip.registers[x] ^= chip.registers[y];
                        break;
                    case 0x4: //0x8XY4, add V[X] and V[Y], saved in V[X], and update V[F]
                        result = chip.registers[x] + chip.registers[y];
                        chip.registers[0xF] = (result > 255) ? 1 : 0;
                        chip.registers[x] = result;
                        break;
                    case 0x5: //0x8XY5, subtract V[Y] from V[X], saved in V[X], and update V[F]
                        result = chip.registers[x] - chip.registers[y];
                        chip.registers[0xF] = (chip.registers[x] >= chip.registers[y]) ? 1 : 0;
                        break;
                    case 0x6: { //0x8XY6, shift V[X] to right; if behavior is set to COSMAC, put V[Y] into V[X] first
                        uint8_t bit;
                        bit = chip.registers[(b == 'C') ? y : x] & 0x01;
                        chip.registers[x] = chip.registers[(b == 'C') ? y : x] >> 1;
                        chip.registers[0xF] = bit;
                        break;
                    }
                    case 0x7: //0x8XY7, subtract V[X] from V[Y], saved in V[Y], and update V[F]
                        result = chip.registers[y] - chip.registers[x];
                        chip.registers[0xF] = (chip.registers[y] >= chip.registers[x]) ? 1 : 0;
                        break;
                    case 0xE: { //0x8XYE
                        uint8_t bit;
                        bit = chip.registers[(b == 'C') ? y : x] >> 7;
                        chip.registers[x] = chip.registers[(b == 'C') ? y : x] << 1;
                        chip.registers[0xF] = bit;
                        break;
                    }
                    default: 
                        invalidOpcode = true;
                        break;
                }
                break;
            case 0x9: //0x9XY0, conditional skip (V[X] != V[Y])
                if (chip.registers[x] != chip.registers[y]) {
                    chip += 2;
                }
                break;
            case 0xA: //0xANNN, set I to NNN
                chip.indexRegister = nnn;
                break;
            case 0xB: //0xBNNN or 0xBXNN depending on implementation
                if (b == 'C') {
                    chip.PC = nnn + chip.registers[0];
                } else {
                    chip.PC = nnn + chip.registers[x];
                }
                break;
            case 0xC: //0xCXNN
                chip.registers[x] = (uint8_t) rand() & nn;
                break;
            case 0xD: //0xDXYN
                uint8_t x_c = chip.registers[x] % 64;
                uint8_t y_c = chip.registers[y] % 32;
                uint8_t n_c = instructionNibbles[0];
                chip.registers[0xF] = 0;
                for (uint8_t i = 0; i < n_c; i++) {
                    if (y_c + i >= 32) break;
                    uint16_t addr = (chip.indexRegister + i) & 0xFFF;
                    bool bits[8] = {0};
                    intToBits(chip.memory[addr], bits);
                    for (size_t j = 0; j < 8; j++) {
                        if (x_c + j >= 64) break;
                        if (chip.display[y_c + i][x_c + j] && bits[j]) chip.registers[0xF] = 1;
                        chip.display[y_c + i][x_c + j] ^= bits[j];
                    }
                }
                render(chip.display);
                break;
            case 0xE: //0xEX9E or 0x
                break;
            default:
                invalidOpcode = true;
        }
        if (stackIssue) {
            fprintf(stderr, "%s.\n", (stackIssue == stackOverflow) ? "Stack overflow error" : "Stack underflow error");
            break;
        }
        if (invalidOpcode) {
            fprintf(stderr, "Invalid opcode 0x%04X at address 0x%03X", (unsigned)instruction, (unsigned)chip.PC - 2);
            break;
        }
    }
return 0;
}