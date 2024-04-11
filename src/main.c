#include <SDL2/SDL.h>
#include "chip8.h"

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    chip8 chip8;
    initialize(&chip8);
    if (argc >= 1)
        load_rom(&chip8, argv[1]);

    SDL_Window *window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_RESIZABLE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int running = 1;
    while (running)
    {
        cycle(&chip8);
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    running = 0;
                    exit(0);
                    break;
                default:
                    break;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}
