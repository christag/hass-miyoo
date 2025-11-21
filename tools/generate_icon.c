/**
 * generate_icon.c - OnionOS Icon Generator
 *
 * Creates a 128x128 pixel art icon for Home Assistant Companion
 * Features: House + gear icon in Game Boy palette
 *
 * Compile: gcc -o generate_icon generate_icon.c `sdl2-config --cflags --libs` -lSDL2_image
 * Run: ./generate_icon
 */

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

#define ICON_SIZE 128

// Game Boy colors
#define GB_DARKEST_R  15
#define GB_DARKEST_G  56
#define GB_DARKEST_B  15

#define GB_DARK_R  48
#define GB_DARK_G  98
#define GB_DARK_B  48

#define GB_LIGHT_R  139
#define GB_LIGHT_G  172
#define GB_LIGHT_B  15

#define GB_LIGHTEST_R  155
#define GB_LIGHTEST_G  188
#define GB_LIGHTEST_B  15

/**
 * Draw a filled rectangle
 */
static void draw_rect(SDL_Renderer *renderer, int x, int y, int w, int h) {
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

/**
 * Draw the house icon
 */
static void draw_house(SDL_Renderer *renderer) {
    // Roof (triangle - approximated with rectangles)
    SDL_SetRenderDrawColor(renderer, GB_LIGHTEST_R, GB_LIGHTEST_G, GB_LIGHTEST_B, 255);

    // Roof outline (stepped pyramid)
    draw_rect(renderer, 54, 28, 20, 4);   // Top
    draw_rect(renderer, 50, 32, 28, 4);
    draw_rect(renderer, 46, 36, 36, 4);
    draw_rect(renderer, 42, 40, 44, 4);
    draw_rect(renderer, 38, 44, 52, 4);

    // House body
    SDL_SetRenderDrawColor(renderer, GB_LIGHT_R, GB_LIGHT_G, GB_LIGHT_B, 255);
    draw_rect(renderer, 38, 48, 52, 52);

    // Window
    SDL_SetRenderDrawColor(renderer, GB_DARK_R, GB_DARK_G, GB_DARK_B, 255);
    draw_rect(renderer, 48, 58, 14, 14);
    draw_rect(renderer, 66, 58, 14, 14);

    // Door
    draw_rect(renderer, 54, 78, 20, 22);
}

/**
 * Draw the gear icon
 */
static void draw_gear(SDL_Renderer *renderer) {
    // Gear positioned in bottom-right corner
    SDL_SetRenderDrawColor(renderer, GB_LIGHTEST_R, GB_LIGHTEST_G, GB_LIGHTEST_B, 255);

    // Gear teeth (8 rectangles around center)
    draw_rect(renderer, 92, 80, 8, 6);   // Top
    draw_rect(renderer, 92, 98, 8, 6);   // Bottom
    draw_rect(renderer, 84, 88, 6, 8);   // Left
    draw_rect(renderer, 102, 88, 6, 8);  // Right

    // Diagonal teeth
    draw_rect(renderer, 86, 82, 6, 6);   // Top-left
    draw_rect(renderer, 100, 82, 6, 6);  // Top-right
    draw_rect(renderer, 86, 96, 6, 6);   // Bottom-left
    draw_rect(renderer, 100, 96, 6, 6);  // Bottom-right

    // Center circle
    draw_rect(renderer, 90, 86, 12, 12);

    // Center hole
    SDL_SetRenderDrawColor(renderer, GB_DARK_R, GB_DARK_G, GB_DARK_B, 255);
    draw_rect(renderer, 94, 90, 4, 4);
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("Home Assistant Companion - Icon Generator\n");
    printf("Generating 128x128 pixel art icon...\n");

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_image
    int img_flags = IMG_INIT_PNG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    // Create window and renderer
    SDL_Window *window = SDL_CreateWindow(
        "Icon Generator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        ICON_SIZE,
        ICON_SIZE,
        SDL_WINDOW_HIDDEN
    );

    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Clear background to darkest green
    SDL_SetRenderDrawColor(renderer, GB_DARKEST_R, GB_DARKEST_G, GB_DARKEST_B, 255);
    SDL_RenderClear(renderer);

    // Draw icon elements
    draw_house(renderer);
    draw_gear(renderer);

    SDL_RenderPresent(renderer);

    // Save to PNG
    SDL_Surface *surface = SDL_CreateRGBSurface(
        0, ICON_SIZE, ICON_SIZE, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000
    );

    if (!surface) {
        fprintf(stderr, "Surface creation failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, surface->pixels, surface->pitch);

    const char *output_path = "dist/HACompanion/icon.png";
    if (IMG_SavePNG(surface, output_path) != 0) {
        fprintf(stderr, "Failed to save PNG: %s\n", IMG_GetError());
        SDL_FreeSurface(surface);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    printf("Icon saved to: %s\n", output_path);

    // Cleanup
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
