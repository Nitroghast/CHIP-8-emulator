#include "chip8.h"

int main () {

    CHIP8 chip = {0};
    resetMachine(&chip);
    //Recieve program path info and load it into the chip. For testing purposes, it'll be just set in the code
    const char path[] = "IBM_Logo.ch8";
    int bytesRead = loadProgram(&chip, path);
    
    //Just before main loop
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    double initialTime = t.tv_sec + t.tv_nsec / 1e9;
    fetchError var = noFetchError;
    //Main loop
    while (true) {
        uint16_t instruction = 0;
        var = fetch(&chip, &instruction);
        if (var) {
            fprintf(stderr, "PC overflow\n");
            break;
        }
        switch (instruction >> 12) {
            case 0: //Instruction has to be 0x00E0, i.e. clear screen
                memset(&(chip.display), 0, sizeof(chip.display));
                chip.drawFlag = true;
        }
    }
return 0;
}