#include "chip8.h"
#include <SDL2/SDL_render.h>

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    chip8_t chip8;
    initialize(&chip8);
    if (argc > 1)
        load_rom(&chip8, argv[1]);

    SDL_Window *window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, 1280, 640, 0);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    int running = 1;
    while (running)
    {
        cycle(&chip8);
        draw_screen(&chip8, window, renderer);
        SDL_Event event;
        for (size_t i = 0; i < 0x10; i++)
            //chip8.keypad[i] = 0;
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
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}
