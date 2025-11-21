/**
 * components.h - UI Component Library for Home Assistant Companion
 *
 * Reusable UI components with Game Boy styling.
 * Phase 4: UI Design System
 */

#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "colors.h"
#include "fonts.h"

/* ============================================
 * Text Alignment
 * ============================================ */

typedef enum {
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT
} text_align_t;

/* ============================================
 * List View Structures
 * ============================================ */

typedef struct {
    char text[128];
    char subtext[64];
    char icon_name[32];
    void *user_data;
} list_item_t;

typedef struct {
    list_item_t *items;
    int item_count;
    int selected_index;
    int scroll_offset;
    int visible_items;
    int item_height;
} list_view_t;

/* ============================================
 * Tab Bar Structure
 * ============================================ */

typedef struct {
    const char *tabs[16];  // Must match MAX_TABS in screen_list.h
    int tab_count;
    int active_tab;
    int visible_start;     // First visible tab index (for scrolling)
} tab_bar_t;

#define MAX_VISIBLE_TABS 4
#define MAX_TAB_LABEL_LEN 8

/* ============================================
 * UI Button Structure
 * ============================================ */

typedef struct {
    char label[32];
    int x, y, width, height;
    int is_primary;
    int is_selected;
} ui_button_t;

/* ============================================
 * Dialog Structure
 * ============================================ */

typedef struct {
    char title[64];
    char message[256];
    int selected_option; // 0 = Yes, 1 = No
    int visible;
} dialog_t;

/* ============================================
 * Text Rendering
 * ============================================ */

/**
 * Draw text with alignment
 *
 * @param renderer SDL renderer
 * @param font TTF font to use
 * @param text Text string to render
 * @param x X position (meaning depends on alignment)
 * @param y Y position (top)
 * @param color Text color
 * @param align Text alignment
 */
void ui_draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                  int x, int y, SDL_Color color, text_align_t align);

/**
 * Draw text with max width (truncates with "...")
 *
 * @param renderer SDL renderer
 * @param font TTF font to use
 * @param text Text string to render
 * @param x X position
 * @param y Y position
 * @param max_width Maximum width before truncation
 * @param color Text color
 */
void ui_draw_text_truncated(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                            int x, int y, int max_width, SDL_Color color);

/* ============================================
 * Rectangles and Panels
 * ============================================ */

/**
 * Draw bordered rectangle (panel/card)
 *
 * @param renderer SDL renderer
 * @param rect Rectangle dimensions
 * @param fill_color Fill color
 * @param border_color Border color
 * @param border_width Border thickness in pixels
 */
void ui_draw_bordered_rect(SDL_Renderer *renderer, SDL_Rect rect,
                           SDL_Color fill_color, SDL_Color border_color,
                           int border_width);

/**
 * Draw filled rectangle
 *
 * @param renderer SDL renderer
 * @param rect Rectangle dimensions
 * @param color Fill color
 */
void ui_draw_filled_rect(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color);

/* ============================================
 * List View
 * ============================================ */

/**
 * Initialize list view
 *
 * @param list List view structure
 * @param item_height Height of each item in pixels
 */
void ui_list_init(list_view_t *list, int item_height);

/**
 * Draw list view with scrolling
 *
 * @param renderer SDL renderer
 * @param list List view structure
 * @param font Font for item text
 * @param x X position
 * @param y Y position
 * @param width Width of list
 * @param height Height of list area
 */
void ui_draw_list(SDL_Renderer *renderer, list_view_t *list, TTF_Font *font,
                  int x, int y, int width, int height);

/**
 * Navigate list selection
 *
 * @param list List view structure
 * @param delta Direction (-1 for up, +1 for down)
 */
void ui_list_navigate(list_view_t *list, int delta);

/* ============================================
 * Tab Bar
 * ============================================ */

/**
 * Draw tab bar with L1/R1 indicators
 *
 * @param renderer SDL renderer
 * @param tabs Tab bar structure
 * @param font Font for tab labels
 * @param x X position
 * @param y Y position
 * @param width Width of tab bar
 */
void ui_draw_tab_bar(SDL_Renderer *renderer, tab_bar_t *tabs, TTF_Font *font,
                     int x, int y, int width);

/**
 * Navigate tabs
 *
 * @param tabs Tab bar structure
 * @param delta Direction (-1 for L1, +1 for R1)
 */
void ui_tab_navigate(tab_bar_t *tabs, int delta);

/* ============================================
 * Button
 * ============================================ */

/**
 * Draw button
 *
 * @param renderer SDL renderer
 * @param button Button structure
 * @param font Font for label
 */
void ui_draw_button(SDL_Renderer *renderer, ui_button_t *button, TTF_Font *font);

/* ============================================
 * Toggle Switch
 * ============================================ */

/**
 * Draw toggle switch
 *
 * @param renderer SDL renderer
 * @param x X position
 * @param y Y position
 * @param is_on Toggle state (1 = on, 0 = off)
 */
void ui_draw_toggle(SDL_Renderer *renderer, int x, int y, int is_on);

/* ============================================
 * Slider
 * ============================================ */

/**
 * Draw horizontal slider/progress bar
 *
 * @param renderer SDL renderer
 * @param x X position
 * @param y Y position
 * @param width Width of slider
 * @param value Current value
 * @param min_val Minimum value
 * @param max_val Maximum value
 */
void ui_draw_slider(SDL_Renderer *renderer, int x, int y, int width,
                    float value, float min_val, float max_val);

/* ============================================
 * Header Bar
 * ============================================ */

/**
 * Draw header bar with title and status
 *
 * @param renderer SDL renderer
 * @param font_title Title font
 * @param font_status Status font (smaller)
 * @param title Screen title
 * @param is_connected Connection status
 */
void ui_draw_header(SDL_Renderer *renderer, TTF_Font *font_title,
                    TTF_Font *font_status, const char *title, int is_connected);

/* ============================================
 * Button Hints Bar
 * ============================================ */

/**
 * Draw button hints at bottom of screen
 *
 * @param renderer SDL renderer
 * @param font Small font for hints
 * @param hints Array of hint strings (e.g., "[A] Select")
 * @param count Number of hints
 */
void ui_draw_button_hints(SDL_Renderer *renderer, TTF_Font *font,
                          const char **hints, int count);

/* ============================================
 * Dialog
 * ============================================ */

/**
 * Draw modal dialog
 *
 * @param renderer SDL renderer
 * @param dialog Dialog structure
 * @param font_title Title font
 * @param font_body Body font
 */
void ui_draw_dialog(SDL_Renderer *renderer, dialog_t *dialog,
                    TTF_Font *font_title, TTF_Font *font_body);

/**
 * Navigate dialog options
 *
 * @param dialog Dialog structure
 * @param delta Direction (-1 for left, +1 for right)
 */
void ui_dialog_navigate(dialog_t *dialog, int delta);

/* ============================================
 * Scrollbar
 * ============================================ */

/**
 * Draw vertical scrollbar
 *
 * @param renderer SDL renderer
 * @param x X position (right edge)
 * @param y Y position
 * @param height Total height
 * @param total_items Total number of items
 * @param visible_items Number of visible items
 * @param scroll_offset Current scroll position
 */
void ui_draw_scrollbar(SDL_Renderer *renderer, int x, int y, int height,
                       int total_items, int visible_items, int scroll_offset);

#endif // COMPONENTS_H
