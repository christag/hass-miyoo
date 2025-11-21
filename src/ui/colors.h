/**
 * colors.h - Game Boy Color Palette for Home Assistant Companion
 *
 * Based on authentic Game Boy DMG-01 green palette
 * See: /design/color-palette.md for full specification
 */

#ifndef COLORS_H
#define COLORS_H

#include <SDL.h>

// Game Boy Color Palette (DMG-01)
static const SDL_Color COLOR_GB_DARKEST  = {15,  56,  15, 255};
static const SDL_Color COLOR_GB_DARK     = {48,  98,  48, 255};
static const SDL_Color COLOR_GB_LIGHT    = {139, 172, 15, 255};
static const SDL_Color COLOR_GB_LIGHTEST = {155, 188, 15, 255};

// Semantic color aliases for UI components
#define COLOR_BACKGROUND        COLOR_GB_DARKEST
#define COLOR_PANEL             COLOR_GB_DARK
#define COLOR_TEXT_PRIMARY      COLOR_GB_LIGHTEST
#define COLOR_TEXT_SECONDARY    COLOR_GB_LIGHT
#define COLOR_BORDER            COLOR_GB_LIGHTEST
#define COLOR_ACCENT            COLOR_GB_LIGHT
#define COLOR_SELECTED          COLOR_GB_LIGHT

// State colors for Home Assistant entities
#define COLOR_STATE_ON          COLOR_GB_LIGHT
#define COLOR_STATE_OFF         COLOR_GB_DARK
#define COLOR_STATE_UNAVAILABLE COLOR_GB_DARKEST
#define COLOR_STATE_WARNING     COLOR_GB_LIGHT
#define COLOR_STATE_ERROR       COLOR_GB_LIGHT

/**
 * Helper function to set SDL renderer color from SDL_Color struct
 *
 * @param renderer SDL2 renderer
 * @param color SDL_Color with RGBA values
 */
static inline void set_render_color(SDL_Renderer *renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

#endif // COLORS_H
