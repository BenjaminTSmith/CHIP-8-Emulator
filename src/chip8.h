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
} chip8_t;

void clear_screen(chip8_t *);
void initialize(chip8_t *);
void cycle(chip8_t *);
void execute_opcode(chip8_t *);
void load_rom(chip8_t *, char *);
void draw_sprite(chip8_t *);
unsigned char get_keypress(chip8_t *);
void poll_keypress(chip8_t *, SDL_Event);
void draw_screen(chip8_t *, SDL_Window *, SDL_Renderer *);

#endif // !CHIP8_H
