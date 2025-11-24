#include "GameBox.h"
#include <SDL2/SDL.h>

// ===== Efek Panah =====
Uint32 arrowTimer = 0;
int arrowDir = 0; // 1 kanan, -1 kiri, 2 atas, 3 bawah

bool runGameBox(SDL_Renderer* renderer)
{
    bool running = true;

    // ===== Posisi kotak =====
    int boxX = 300;
    int boxY = 200;
    const int boxSize = 50;

    SDL_Event event;

    while (running)
    {
        // ------- EVENT -------
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                return false;

            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    return false;

                case SDLK_RIGHT:
                    boxX += 5;
                    arrowDir = 1;
                    arrowTimer = SDL_GetTicks() + 150;
                    break;

                case SDLK_LEFT:
                    boxX -= 5;
                    arrowDir = -1;
                    arrowTimer = SDL_GetTicks() + 150;
                    break;

                case SDLK_UP:
                    boxY -= 5;
                    arrowDir = 2;
                    arrowTimer = SDL_GetTicks() + 150;
                    break;

                case SDLK_DOWN:
                    boxY += 5;
                    arrowDir = 3;
                    arrowTimer = SDL_GetTicks() + 150;
                    break;
                }
            }
        }

        // ------- RENDER BACKGROUND -------
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        // ------- RENDER BOX -------
        SDL_Rect boxRect;
        boxRect.x = boxX;
        boxRect.y = boxY;
        boxRect.w = boxSize;
        boxRect.h = boxSize;

        SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255);
        SDL_RenderFillRect(renderer, &boxRect);

        // ------- RENDER PANAH -------
        if (SDL_GetTicks() < arrowTimer)
        {
            SDL_Rect arrow;
            arrow.w = 10;
            arrow.h = 10;

            if (arrowDir == 1) {
                arrow.x = boxRect.x + boxRect.w + 5;
                arrow.y = boxRect.y + boxRect.h / 2 - 5;
            }
            else if (arrowDir == -1) {
                arrow.x = boxRect.x - 15;
                arrow.y = boxRect.y + boxRect.h / 2 - 5;
            }
            else if (arrowDir == 2) {
                arrow.x = boxRect.x + boxRect.w / 2 - 5;
                arrow.y = boxRect.y - 15;
            }
            else if (arrowDir == 3) {
                arrow.x = boxRect.x + boxRect.w / 2 - 5;
                arrow.y = boxRect.y + boxRect.h + 5;
            }

            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &arrow);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    return true;
}
