/**
 * colors.h - SNES RPG-Inspired Color Palette
 *
 * Inspired by Final Fantasy VI / Chrono Trigger menu systems.
 * Deep indigo backgrounds, white text, high contrast for readability.
 */

#ifndef COLORS_H
#define COLORS_H

#include <SDL.h>

// === Core Palette ===
// Deep indigo/navy background (like SNES RPG menus)
static const SDL_Color COLOR_BG_DARK     = {16,  16,  48,  255};  // Deepest background
static const SDL_Color COLOR_BG_PANEL    = {32,  32,  80,  255};  // Panel/window fill
static const SDL_Color COLOR_BG_HOVER    = {48,  48,  112, 255};  // Lighter panel variant

// Borders - classic RPG double-line window borders
static const SDL_Color COLOR_BORDER_OUTER = {160, 168, 224, 255};  // Light periwinkle outer
static const SDL_Color COLOR_BORDER_INNER = {88,  96,  160, 255};  // Mid-blue inner

// Text - maximum readability
static const SDL_Color COLOR_WHITE       = {255, 255, 255, 255};  // Primary text
static const SDL_Color COLOR_GRAY        = {176, 176, 192, 255};  // Secondary/dimmed text
static const SDL_Color COLOR_BLUE_TEXT   = {128, 160, 255, 255};  // Accent text (labels)

// Selection - bright and unmistakable
static const SDL_Color COLOR_SELECT_BG   = {64,  64,  160, 255};  // Selection background
static const SDL_Color COLOR_CURSOR      = {255, 208, 64,  255};  // Golden cursor arrow

// State indicators
static const SDL_Color COLOR_ON          = {96,  232, 96,  255};  // Green = on/active
static const SDL_Color COLOR_OFF         = {128, 128, 144, 255};  // Gray = off
static const SDL_Color COLOR_WARN        = {255, 192, 64,  255};  // Amber = warning
static const SDL_Color COLOR_UNAVAIL     = {96,  96,  112, 255};  // Dark gray = unavailable

// Tab bar
static const SDL_Color COLOR_TAB_ACTIVE  = {255, 255, 255, 255};  // Active tab text
static const SDL_Color COLOR_TAB_INACTIVE = {96, 112, 160, 255};  // Inactive tab text
static const SDL_Color COLOR_TAB_UNDERLINE = {255, 208, 64, 255}; // Golden underline

// === Semantic Aliases ===
#define COLOR_BACKGROUND        COLOR_BG_DARK
#define COLOR_PANEL             COLOR_BG_PANEL
#define COLOR_TEXT_PRIMARY      COLOR_WHITE
#define COLOR_TEXT_SECONDARY    COLOR_GRAY
#define COLOR_TEXT_ACCENT       COLOR_BLUE_TEXT
#define COLOR_BORDER            COLOR_BORDER_OUTER
#define COLOR_ACCENT            COLOR_TAB_UNDERLINE
#define COLOR_SELECTED          COLOR_SELECT_BG

// State colors for Home Assistant entities
#define COLOR_STATE_ON          COLOR_ON
#define COLOR_STATE_OFF         COLOR_OFF
#define COLOR_STATE_UNAVAILABLE COLOR_UNAVAIL
#define COLOR_STATE_WARNING     COLOR_WARN
#define COLOR_STATE_ERROR       COLOR_WARN

// Legacy aliases (for code that still references GB colors)
#define COLOR_GB_DARKEST        COLOR_BG_DARK
#define COLOR_GB_DARK           COLOR_BG_PANEL
#define COLOR_GB_LIGHT          COLOR_GRAY
#define COLOR_GB_LIGHTEST       COLOR_WHITE

/**
 * Helper function to set SDL renderer color from SDL_Color struct
 */
static inline void set_render_color(SDL_Renderer *renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

#endif // COLORS_H
