/**
 * fonts.h - Font Management for Home Assistant Companion
 *
 * Loads and manages pixel fonts (Press Start 2P) at various sizes.
 * Phase 4: UI Design System
 */

#ifndef FONTS_H
#define FONTS_H

#include <SDL.h>
#include <SDL_ttf.h>

/**
 * Font size definitions
 */
typedef enum {
    FONT_SIZE_SMALL  = 8,   // Button hints, status text
    FONT_SIZE_BODY   = 12,  // List items, normal text
    FONT_SIZE_HEADER = 16   // Screen titles, headers
} font_size_t;

/**
 * Font manager structure
 */
typedef struct {
    TTF_Font *font_small;    // 8px
    TTF_Font *font_body;     // 12px
    TTF_Font *font_header;   // 16px
    char font_path[256];     // Path to font file
} font_manager_t;

/**
 * Initialize font manager and load fonts
 *
 * @param font_path Path to TTF font file (Press Start 2P)
 * @return font_manager_t pointer or NULL on failure
 */
font_manager_t* fonts_init(const char *font_path);

/**
 * Destroy font manager and free resources
 *
 * @param fonts Font manager to destroy
 */
void fonts_destroy(font_manager_t *fonts);

/**
 * Get font by size enum
 *
 * @param fonts Font manager
 * @param size Font size enum
 * @return TTF_Font pointer or NULL
 */
TTF_Font* fonts_get(font_manager_t *fonts, font_size_t size);

/**
 * Get text dimensions
 *
 * @param font TTF_Font to measure with
 * @param text Text to measure
 * @param width Output: text width in pixels
 * @param height Output: text height in pixels
 * @return 1 on success, 0 on failure
 */
int fonts_measure_text(TTF_Font *font, const char *text, int *width, int *height);

#endif // FONTS_H
