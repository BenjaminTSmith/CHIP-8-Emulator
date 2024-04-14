#include "chip8.h"

#define CYCLES_PER_FRAME 10

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    chip8_t chip8;
    initialize(&chip8);
    if (argc == 1)
        printf("Usage: ./main [ROM binary] [CYCLES_PER_FRAME]\n");
    if (argc > 1)
        load_rom(&chip8, argv[1]);

    SDL_Window *window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, 1280, 640, 0);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    int running = 1;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    running = 0;
                    break;
            }
            poll_keypress(&chip8, event);
        }
        for (Uint32 i = 0; i < CYCLES_PER_FRAME; i++) // set clock rate of CHIP-8 to about 60hz
            cycle(&chip8);

        draw_screen(&chip8, window, renderer);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}
