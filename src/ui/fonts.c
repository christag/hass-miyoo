/**
 * fonts.c - Font Management Implementation
 */

#include "fonts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

font_manager_t* fonts_init(const char *font_path) {
    if (!font_path) {
        fprintf(stderr, "Font path is NULL\n");
        return NULL;
    }

    font_manager_t *fonts = calloc(1, sizeof(font_manager_t));
    if (!fonts) {
        fprintf(stderr, "Failed to allocate font manager\n");
        return NULL;
    }

    strncpy(fonts->font_path, font_path, sizeof(fonts->font_path) - 1);

    // Load font at different sizes
    fonts->font_small = TTF_OpenFont(font_path, FONT_SIZE_SMALL);
    if (!fonts->font_small) {
        fprintf(stderr, "Failed to load font at %dpx: %s\n",
                FONT_SIZE_SMALL, TTF_GetError());
        fonts_destroy(fonts);
        return NULL;
    }

    fonts->font_body = TTF_OpenFont(font_path, FONT_SIZE_BODY);
    if (!fonts->font_body) {
        fprintf(stderr, "Failed to load font at %dpx: %s\n",
                FONT_SIZE_BODY, TTF_GetError());
        fonts_destroy(fonts);
        return NULL;
    }

    fonts->font_header = TTF_OpenFont(font_path, FONT_SIZE_HEADER);
    if (!fonts->font_header) {
        fprintf(stderr, "Failed to load font at %dpx: %s\n",
                FONT_SIZE_HEADER, TTF_GetError());
        fonts_destroy(fonts);
        return NULL;
    }

    // Set font rendering hints for pixel-perfect rendering
    TTF_SetFontHinting(fonts->font_small, TTF_HINTING_MONO);
    TTF_SetFontHinting(fonts->font_body, TTF_HINTING_MONO);
    TTF_SetFontHinting(fonts->font_header, TTF_HINTING_MONO);

    printf("Fonts loaded: %s (8px, 12px, 16px)\n", font_path);

    return fonts;
}

void fonts_destroy(font_manager_t *fonts) {
    if (!fonts) return;

    if (fonts->font_small) {
        TTF_CloseFont(fonts->font_small);
    }
    if (fonts->font_body) {
        TTF_CloseFont(fonts->font_body);
    }
    if (fonts->font_header) {
        TTF_CloseFont(fonts->font_header);
    }

    free(fonts);
}

TTF_Font* fonts_get(font_manager_t *fonts, font_size_t size) {
    if (!fonts) return NULL;

    switch (size) {
        case FONT_SIZE_SMALL:
            return fonts->font_small;
        case FONT_SIZE_BODY:
            return fonts->font_body;
        case FONT_SIZE_HEADER:
            return fonts->font_header;
        default:
            return fonts->font_body;
    }
}

int fonts_measure_text(TTF_Font *font, const char *text, int *width, int *height) {
    if (!font || !text) {
        if (width) *width = 0;
        if (height) *height = 0;
        return 0;
    }

    return TTF_SizeText(font, text, width, height) == 0;
}
