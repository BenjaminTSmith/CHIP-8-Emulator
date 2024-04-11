#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

typedef struct chip8
{
    unsigned short opcode;
    unsigned char memory[0x1000];
    unsigned char V[0x10];
    unsigned short int I;
    unsigned short int PC;
    unsigned char screen[64 * 32];
    unsigned short stack[0x10];
    unsigned short stack_pointer;
    unsigned char keypad[0x10];
    unsigned char delay_timer;
    unsigned char sound_timer;
} chip8;

void clear_screen(chip8 *);
void initialize(chip8 *);
void cycle(chip8 *);
void execute_opcode(chip8 *);
void load_rom(chip8 *, char *);
void draw_sprite(chip8 *);

#endif // !CHIP8_H
