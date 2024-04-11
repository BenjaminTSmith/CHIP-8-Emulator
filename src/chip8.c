#include "chip8.h"

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
        chip8->memory[i] = 0;
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
    if (chip8->sound_timer >= 1)
        printf("EEE!\n");
    else
        chip8->sound_timer--;
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
                //TODO
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
                    chip8->V[0xF] = chip8->V[(chip8->opcode & 0x0F00) >> 8] & 0x8000;
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
                unsigned char key = chip8->V[(chip8->opcode & 0x0F00) >> 8];
                case 0x000E:
                    if (chip8->keypad[key] == 1)
                        chip8->PC += 4;
                    else
                        chip8->PC += 2;
                    break;
                case 0x0001:
                    if (chip8->keypad[key] == 0)
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
                case 0x0007:
                    break;
                case 0x000A:
                    break;
                case 0x15:
                    break;
                case 0x0018:
                    break;
                case 0x001E:
                    break;
                case 0x0029:
                    break;
                case 0x0033:
                    break;
                case 0x0055:
                    break;
                case 0x0065:
                    break;
                default:
                    printf("Unknown opcode: 0x%X\n", chip8->opcode);
                    break;
            }
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

void load_rom(chip8 *chip8, char **rom)
{

}