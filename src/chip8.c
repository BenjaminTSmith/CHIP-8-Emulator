#include "chip8.h"
#include <SDL2/SDL_events.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

/*
* ---------Memory Map-------
* 0x000 - 0x1FF : Interpreter (so really nothing)
* 0x050 - 0x0A0 : Font Set
* 0x200 - 0xFFF : RAM
*/

unsigned char chip8_fontset[80] =
    { 
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

void clear_screen(chip8 *chip8)
{
    for (size_t i = 0; i < 2048; i++)
        chip8->screen[i] = 0;
}

void initialize(chip8 *chip8)
{
    // reset everything
    chip8->opcode = 0;
    chip8->PC = 0x200;
    chip8->I = 0;
    chip8->stack_pointer = 0;
    clear_screen(chip8);
    for (size_t i = 0; i < 0x10; i++)
        chip8->stack[i] = 0;
    for (size_t i = 0; i < 0xF; i++)
        chip8->V[i] = 0;
    for (size_t i = 0; i < 0x1000; i++)
        chip8->memory[i] = 0x00;
    for (size_t i = 0; i < 0x10; i++)
        chip8->keypad[i] = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    // load font data
    for (size_t i = 0; i < 80; i++)
        chip8->memory[0x050 + i] = chip8_fontset[i];
}

void cycle(chip8 *chip8)
{
    chip8->opcode = chip8->memory[chip8->PC] << 8 | chip8->memory[chip8->PC + 1];
    execute_opcode(chip8);
    if (chip8->delay_timer > 0)
        chip8->delay_timer--;
    if (chip8->sound_timer > 0)
    {
        printf("EEE!\n");
        chip8->sound_timer--;
    }
}

void execute_opcode(chip8 *chip8)
{
    switch (chip8->opcode & 0xF000)
    {
        case 0x0000:
            switch (chip8->opcode & 0x000F)
            {
                case 0x0000:
                    clear_screen(chip8);
                    break;
                case 0x000E:
                    chip8->PC = chip8->stack[--chip8->stack_pointer];
                    chip8->PC += 2;
                    break;
                default:
                    printf("Unknown opcode: 0x%X", chip8->opcode);
                    break;
            }
            break;
        case 0x1000:
            chip8->PC = chip8->opcode & 0x0FFF;
            break;
        case 0x2000:
            chip8->stack[chip8->stack_pointer++] = chip8->PC;
            chip8->PC = chip8->opcode & 0x0FFF;
            break;
        case 0x3000:
            if (chip8->V[(chip8->opcode & 0x0F00) >> 8] == (chip8->opcode & 0x00FF))
                chip8->PC += 4;
            else
                chip8->PC += 2;
            break;
        case 0x4000:
            if (chip8->V[(chip8->opcode & 0x0F00) >> 8] != (chip8->opcode & 0x00FF))
                chip8->PC += 4;
            else
                chip8->PC += 2;
            break;
        case 0x5000:
            if (chip8->V[(chip8->opcode & 0x0F00) >> 8] == chip8->V[(chip8->opcode & 0x00F0) >> 4])
                chip8->PC += 4;
            else
                chip8->PC += 2;
            break;
        case 0x6000:
            chip8->V[(chip8->opcode & 0x0F00) >> 8] = chip8->opcode & 0x00FF;
            chip8->PC += 2;
            break;
        case 0x7000:
            chip8->V[(chip8->opcode & 0x0F00) >> 8] += chip8->opcode & 0x00FF;
            chip8->PC += 2;
            break;
        case 0x8000:
            switch (chip8->opcode & 0x000F)
            {
                case 0x0000:
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] = chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    break;
                case 0x0001:
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] |= chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    break;
                case 0x0002:
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] &= chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    break;
                case 0x0003:
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] ^= chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    break;
                case 0x0004:
                    if (chip8->V[(chip8->opcode & 0x00F0) >> 4] > (0xFF - chip8->V[(chip8->opcode & 0x0F00) >> 8]))
                        chip8->V[0xF] = 1; // carry
                    else
                        chip8->V[0xF] = 0;
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] += chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    break;
                case 0x0005:
                    if (chip8->V[(chip8->opcode & 0x0F00) >> 8] >= chip8->V[(chip8->opcode & 0x00F0) >> 4])
                        chip8->V[0xF] = 1; // carry
                    else
                        chip8->V[0xF] = 0;
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] -= chip8->V[(chip8->opcode & 0x00F0) >> 4];
                    break;
                case 0x0006:
                    chip8->V[0xF] = chip8->V[(chip8->opcode & 0x0F00) >> 8] & 0x0001;
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] >>= 1;
                    break;
                case 0x0007:
                    if (chip8->V[(chip8->opcode & 0x0F00) >> 8] <= chip8->V[(chip8->opcode & 0x00F0) >> 4])
                        chip8->V[0xF] = 1; // carry
                    else
                        chip8->V[0xF] = 0;
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] = chip8->V[(chip8->opcode & 0x00F0) >> 4] -
                                                              chip8->V[(chip8->opcode & 0x0F00) >> 8];
                    break;
                case 0x000E:
                    chip8->V[0xF] = (chip8->V[(chip8->opcode & 0x0F00) >> 8] & 0x8000) >> 15;
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] <<= 1;
                    break;
                default:
                    printf("Unknown opcode: 0x%X\n", chip8->opcode);
                    break;
            }
            chip8->PC += 2;
            break;
        case 0x9000:
            if (chip8->V[(chip8->opcode & 0x0F00) >> 8] != chip8->V[(chip8->opcode & 0x00F0) >> 4])
                chip8->PC += 4;
            else
                chip8->PC += 2;
            break;
        case 0xA000:
            chip8->I = chip8->opcode & 0x0FFF;
            chip8->PC += 2;
            break;
        case 0xB000:
            chip8->PC = chip8->V[0x0] + (chip8->opcode & 0x0FFF);
            break;
        case 0xC000:
            chip8->V[(chip8->opcode & 0x0F00) >> 8] = rand() % 256 & (chip8->opcode & 0x00FF);
            chip8->PC += 2;
            break;
        case 0xD000:
            draw_sprite(chip8);
            chip8->PC += 2;
            break;
        case 0xE000:
            switch (chip8->opcode & 0x000F)
            {
                case 0x000E:
                    if (chip8->keypad[chip8->V[(chip8->opcode & 0x0F00) >> 8]] == 1)
                        chip8->PC += 4;
                    else
                        chip8->PC += 2;
                    break;
                case 0x0001:
                    if (chip8->keypad[chip8->V[(chip8->opcode & 0x0F00) >> 8]] == 0)
                        chip8->PC += 4;
                    else
                        chip8->PC += 2;
                    break;
                default:
                    printf("Unknown opcode: 0x%X\n", chip8->opcode);
                    break;
            }
            break;
        case 0xF000:
            switch (chip8->opcode & 0x00FF)
            {
                case 0x0007: // set register VX to delay timer
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] = chip8->delay_timer;
                    break;
                case 0x000A: // wait for a keypress
                    chip8->V[(chip8->opcode & 0x0F00) >> 8] = get_keypress(chip8);
                    break;
                case 0x0015:
                    chip8->delay_timer = chip8->V[(chip8->opcode & 0x0F00) >> 8];
                    break;
                case 0x0018:
                    chip8->sound_timer = chip8->V[(chip8->opcode & 0x0F00) >> 8];
                    break;
                case 0x001E:
                    chip8->I += chip8->V[(chip8->opcode & 0x0F00) >> 8];
                    break;
                case 0x0029: // set I to memory address of sprite/font data
                    chip8->I = 0x050 + chip8->V[(chip8->opcode & 0x0F00) >> 8] * 6;
                    break;
                case 0x0033:
                    chip8->memory[chip8->I] = chip8->V[(chip8->opcode & 0x0F00) >> 8] / 100;
                    chip8->memory[chip8->I + 1] = (chip8->V[(chip8->opcode & 0x0F00) >> 8] / 10) % 10;
                    chip8->memory[chip8->I + 2] = (chip8->V[(chip8->opcode & 0x0F00) >> 8] % 100) % 10;
                    break;
                case 0x0055:
                    for (unsigned char i = 0; i <= (chip8->opcode & 0x0F00) >> 8; i++)
                        chip8->memory[chip8->I + i] = chip8->V[i];
                    break;
                case 0x0065:
                    for (unsigned char i = 0; i <= (chip8->opcode & 0x0F00) >> 8; i++)
                        chip8->V[i] = chip8->memory[chip8->I + i];
                    break;
                default:
                    printf("Unknown opcode: 0x%X\n", chip8->opcode);
                    break;
            }
            chip8->PC += 2;
            break;
        default:
            printf("Unknown opcode: 0x%X\n", chip8->opcode);
            break;
    }  
}

void draw_sprite(chip8 *chip8)
{
    // from: https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
    unsigned short x = chip8->V[(chip8->opcode & 0x0F00) >> 8];
    unsigned short y = chip8->V[(chip8->opcode & 0x00F0) >> 4];
    unsigned short height = chip8->opcode & 0x000F;
    unsigned short pixel;

    chip8->V[0xF] = 0;
    for (int yline = 0; yline < height; yline++)
    {
        pixel = chip8->memory[chip8->I + yline];
        for (int xline = 0; xline < 8; xline++)
        {
            if ((pixel & (0x80 >> xline)) != 0)
            {
                if (chip8->screen[(x + xline + ((y + yline) * 64))] == 1)
                    chip8->V[0xF] = 1;                                 
                chip8->screen[x + xline + ((y + yline) * 64)] ^= 1;
            }
        }
    }
}

void load_rom(chip8 *chip8, char *rom)
{
    FILE *file = fopen(rom, "rb");
}

unsigned char get_keypress(chip8 *chip8)
{
    unsigned char key = 0;
    while (!key)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.key.keysym.sym)
            {
                case SDLK_1:
                    key = 0x1;
                    break;
                case SDLK_2:
                    key = 0x2;
                    break;
                case SDLK_3:
                    key = 0x3;
                    break;
                case SDLK_4:
                    key = 0xC;
                    break;
                case SDLK_q:
                    key = 0x4;
                    break;
                case SDLK_w:
                    key = 0x5;
                    break;
                case SDLK_e:
                    key = 0x6;
                    break;
                case SDLK_r:
                    key = 0xD;
                    break;
                case SDLK_a:
                    key = 0x7;
                    break;
                case SDLK_s:
                    key = 0x8;
                    break;
                case SDLK_d:
                    key = 0x9;
                    break;
                case SDLK_f:
                    key = 0xE;
                    break;
                case SDLK_z:
                    key = 0xA;
                    break;
                case SDLK_x:
                    key = 0x0;
                    break;
                case SDLK_c:
                    key = 0xB;
                    break;
                case SDLK_v:
                    key = 0xF;
                    break;
                default:
                    break;
            }
        }
    }
    return key;
}

void poll_keypress(chip8 *chip8)
{
    for (size_t i = 0; i < 0x10; i++)
        chip8->keypad[i] = 0;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.key.keysym.sym)
        {
            case SDLK_1:
                chip8->keypad[0x01] = 1;
                break;
            case SDLK_2:
                chip8->keypad[0x02] = 1;
                break;
            case SDLK_3:
                chip8->keypad[0x03] = 1;
                break;
            case SDLK_4:
                chip8->keypad[0x0C] = 1;
                break;
            case SDLK_q:
                chip8->keypad[0x04] = 1;
                break;
            case SDLK_w:
                chip8->keypad[0x05] = 1;
                break;
            case SDLK_e:
                chip8->keypad[0x06] = 1;
                break;
            case SDLK_r:
                chip8->keypad[0x0D] = 1;
                break;
            case SDLK_a:
                chip8->keypad[0x07] = 1;
                break;
            case SDLK_s:
                chip8->keypad[0x08] = 1;
                break;
            case SDLK_d:
                chip8->keypad[0x09] = 1;
                break;
            case SDLK_f:
                chip8->keypad[0x0E] = 1;
                break;
            case SDLK_z:
                chip8->keypad[0x0A] = 1;
                break;
            case SDLK_x:
                chip8->keypad[0x00] = 1;
                break;
            case SDLK_c:
                chip8->keypad[0x0B] = 1;
                break;
            case SDLK_v:
                chip8->keypad[0x0F] = 1;
                break;
            default:
                break;
        }
    }
}
