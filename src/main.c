#include "chip8.h"

int main(int argc, char **argv)
{
    chip8 chip8;
    initialize(&chip8);
    cycle(&chip8);
    execute_opcode(&chip8);

    return 0;
}
