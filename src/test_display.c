/*
 * test_display.c - Minimal SDL2 display test for Miyoo Mini Plus
 *
 * Purpose: Test if SDL2 can render to the physical screen when using
 * Moonlight's complete library set (including EGL/GLES libraries).
 *
 * Expected behavior: Display a RED screen for 5 seconds, then exit.
 * If you see RED: Libraries are the issue - HACompanion needs EGL/GLES
 * If you see BLACK: Problem is in compilation/linking
 */

#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("=== Minimal SDL2 Display Test ===\n");
    printf("Expected: RED screen for 5 seconds\n\n");

    /* Set double buffer BEFORE SDL_Init - critical for MMIYOO driver */
    SDL_setenv("SDL_MMIYOO_DOUBLE_BUFFER", "1", 1);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    printf("SDL_Init: OK\n");
    printf("Video driver: %s\n", SDL_GetCurrentVideoDriver());

    SDL_Window *window = SDL_CreateWindow(
        "Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640, 480,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("SDL_CreateWindow: OK\n");

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    /* Print renderer info */
    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(renderer, &info) == 0) {
        printf("Renderer: %s, Flags: %u\n", info.name, info.flags);
    }

    printf("\nStarting render loop (300 frames @ ~60fps = 5 seconds)...\n");
    printf("You should see a RED screen now!\n\n");

    /* Render RED for 5 seconds (300 frames at 60fps) */
    for (int i = 0; i < 300; i++) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  /* RED */
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);  /* ~60fps */

        if (i % 60 == 0) {
            printf("Frame %d\n", i);
        }
    }

    printf("\nTest complete. Cleaning up...\n");

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("Done!\n");
    return 0;
}
