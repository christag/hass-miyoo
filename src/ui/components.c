/**
 * components.c - SNES RPG-Style UI Component Library
 *
 * Inspired by Final Fantasy VI / Chrono Trigger menu systems.
 * Double-bordered windows, golden selection cursors, high contrast.
 */

#include "components.h"
#include <stdio.h>
#include <string.h>

/* ============================================
 * Text Rendering
 * ============================================ */

void ui_draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                  int x, int y, SDL_Color color, text_align_t align) {
    if (!renderer || !font || !text || strlen(text) == 0) {
        return;
    }

    SDL_Surface *surface = TTF_RenderUTF8_Solid(font, text, color);
    if (!surface) {
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dest = {x, y, surface->w, surface->h};

    switch (align) {
        case TEXT_ALIGN_CENTER:
            dest.x = x - (surface->w / 2);
            break;
        case TEXT_ALIGN_RIGHT:
            dest.x = x - surface->w;
            break;
        case TEXT_ALIGN_LEFT:
        default:
            break;
    }

    SDL_RenderCopy(renderer, texture, NULL, &dest);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void ui_draw_text_truncated(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                            int x, int y, int max_width, SDL_Color color) {
    if (!renderer || !font || !text || strlen(text) == 0) {
        return;
    }

    int text_width, text_height;
    TTF_SizeText(font, text, &text_width, &text_height);

    if (text_width <= max_width) {
        ui_draw_text(renderer, font, text, x, y, color, TEXT_ALIGN_LEFT);
        return;
    }

    char truncated[256];
    strncpy(truncated, text, sizeof(truncated) - 4);
    truncated[sizeof(truncated) - 4] = '\0';

    int len = strlen(truncated);
    while (len > 0) {
        truncated[len] = '\0';
        strcat(truncated, "..");
        TTF_SizeText(font, truncated, &text_width, &text_height);
        if (text_width <= max_width) {
            break;
        }
        truncated[len] = '\0';
        len--;
    }

    ui_draw_text(renderer, font, truncated, x, y, color, TEXT_ALIGN_LEFT);
}

/* ============================================
 * RPG-Style Window Drawing
 * ============================================ */

/**
 * Draw an RPG-style double-bordered window panel
 * Outer border: light periwinkle
 * Inner border: mid-blue
 * Fill: deep panel blue
 */
static void ui_draw_rpg_window(SDL_Renderer *renderer, SDL_Rect rect) {
    // Outer border (2px)
    set_render_color(renderer, COLOR_BORDER_OUTER);
    SDL_RenderDrawRect(renderer, &rect);
    SDL_Rect inner1 = {rect.x + 1, rect.y + 1, rect.w - 2, rect.h - 2};
    SDL_RenderDrawRect(renderer, &inner1);

    // Inner border (1px darker)
    set_render_color(renderer, COLOR_BORDER_INNER);
    SDL_Rect inner2 = {rect.x + 2, rect.y + 2, rect.w - 4, rect.h - 4};
    SDL_RenderDrawRect(renderer, &inner2);

    // Fill
    SDL_Rect fill = {rect.x + 3, rect.y + 3, rect.w - 6, rect.h - 6};
    set_render_color(renderer, COLOR_BG_PANEL);
    SDL_RenderFillRect(renderer, &fill);
}

void ui_draw_filled_rect(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color) {
    set_render_color(renderer, color);
    SDL_RenderFillRect(renderer, &rect);
}

void ui_draw_bordered_rect(SDL_Renderer *renderer, SDL_Rect rect,
                           SDL_Color fill_color, SDL_Color border_color,
                           int border_width) {
    set_render_color(renderer, fill_color);
    SDL_RenderFillRect(renderer, &rect);

    set_render_color(renderer, border_color);
    for (int i = 0; i < border_width; i++) {
        SDL_Rect border = {
            rect.x + i,
            rect.y + i,
            rect.w - i * 2,
            rect.h - i * 2
        };
        SDL_RenderDrawRect(renderer, &border);
    }
}

/* ============================================
 * List View
 * ============================================ */

void ui_list_init(list_view_t *list, int item_height) {
    if (!list) return;
    list->item_height = item_height > 0 ? item_height : 44;
    list->selected_index = 0;
    list->scroll_offset = 0;
    list->visible_items = 0;
}

void ui_draw_list(SDL_Renderer *renderer, list_view_t *list, TTF_Font *font,
                  int x, int y, int width, int height) {
    if (!renderer || !list || !font || list->item_count == 0) {
        return;
    }

    list->visible_items = height / list->item_height;

    if (list->selected_index < list->scroll_offset) {
        list->scroll_offset = list->selected_index;
    }
    if (list->selected_index >= list->scroll_offset + list->visible_items) {
        list->scroll_offset = list->selected_index - list->visible_items + 1;
    }

    for (int i = 0; i < list->visible_items && (i + list->scroll_offset) < list->item_count; i++) {
        int index = i + list->scroll_offset;
        list_item_t *item = &list->items[index];
        int item_y = y + (i * list->item_height);
        int is_selected = (index == list->selected_index);

        // Selection highlight bar
        if (is_selected) {
            SDL_Rect bg = {x, item_y, width - 6, list->item_height - 2};
            ui_draw_filled_rect(renderer, bg, COLOR_SELECTED);
        }

        // Golden cursor for selected item
        if (is_selected) {
            ui_draw_text(renderer, font, "\x10", x + 6, item_y + (list->item_height - 14) / 2,
                        COLOR_CURSOR, TEXT_ALIGN_LEFT);
        }

        // Item text
        SDL_Color text_color = COLOR_TEXT_PRIMARY;
        ui_draw_text_truncated(renderer, font, item->text, x + 28,
                               item_y + (list->item_height - 14) / 2,
                               width - 50, text_color);

        // Subtext (state)
        if (strlen(item->subtext) > 0) {
            SDL_Color sub_color = COLOR_TEXT_SECONDARY;
            ui_draw_text(renderer, font, item->subtext, x + width - 16,
                        item_y + (list->item_height - 14) / 2,
                        sub_color, TEXT_ALIGN_RIGHT);
        }
    }

    if (list->item_count > list->visible_items) {
        ui_draw_scrollbar(renderer, x + width - 4, y, height,
                          list->item_count, list->visible_items, list->scroll_offset);
    }
}

void ui_list_navigate(list_view_t *list, int delta) {
    if (!list || list->item_count == 0) return;

    list->selected_index += delta;

    if (list->selected_index < 0) {
        list->selected_index = list->item_count - 1;
    } else if (list->selected_index >= list->item_count) {
        list->selected_index = 0;
    }
}

/* ============================================
 * Tab Bar - Clean horizontal tabs
 * ============================================ */

void ui_draw_tab_bar(SDL_Renderer *renderer, tab_bar_t *tabs, TTF_Font *font,
                     int x, int y, int width) {
    if (!renderer || !tabs || !font || tabs->tab_count == 0) {
        return;
    }

    int tab_height = 28;
    int visible_count = tabs->tab_count < MAX_VISIBLE_TABS ? tabs->tab_count : MAX_VISIBLE_TABS;
    int tab_width = width / visible_count;

    if (tabs->active_tab < tabs->visible_start) {
        tabs->visible_start = tabs->active_tab;
    } else if (tabs->active_tab >= tabs->visible_start + visible_count) {
        tabs->visible_start = tabs->active_tab - visible_count + 1;
    }
    if (tabs->visible_start < 0) tabs->visible_start = 0;
    if (tabs->visible_start > tabs->tab_count - visible_count) {
        tabs->visible_start = tabs->tab_count - visible_count;
    }
    if (tabs->visible_start < 0) tabs->visible_start = 0;

    // L1/R1 arrows if more tabs exist
    if (tabs->visible_start > 0) {
        ui_draw_text(renderer, font, "<", x - 14, y + 6, COLOR_CURSOR, TEXT_ALIGN_LEFT);
    }
    if (tabs->visible_start + visible_count < tabs->tab_count) {
        ui_draw_text(renderer, font, ">", x + width + 4, y + 6, COLOR_CURSOR, TEXT_ALIGN_LEFT);
    }

    for (int i = 0; i < visible_count; i++) {
        int tab_index = tabs->visible_start + i;
        if (tab_index >= tabs->tab_count) break;

        int tab_x = x + (i * tab_width);
        int is_active = (tab_index == tabs->active_tab);

        // Tab label
        SDL_Color color = is_active ? COLOR_TAB_ACTIVE : COLOR_TAB_INACTIVE;

        const char *tab_name = tabs->tabs[tab_index];
        if (!tab_name) tab_name = "???";

        char truncated[MAX_TAB_LABEL_LEN + 1];
        if (strlen(tab_name) > MAX_TAB_LABEL_LEN) {
            strncpy(truncated, tab_name, MAX_TAB_LABEL_LEN - 1);
            truncated[MAX_TAB_LABEL_LEN - 1] = '.';
            truncated[MAX_TAB_LABEL_LEN] = '\0';
        } else {
            strncpy(truncated, tab_name, MAX_TAB_LABEL_LEN);
            truncated[MAX_TAB_LABEL_LEN] = '\0';
        }

        ui_draw_text(renderer, font, truncated,
                    tab_x + tab_width / 2, y + 6, color, TEXT_ALIGN_CENTER);

        // Golden underline for active tab
        if (is_active) {
            SDL_Rect underline = {tab_x + 4, y + tab_height - 3, tab_width - 8, 3};
            ui_draw_filled_rect(renderer, underline, COLOR_TAB_UNDERLINE);
        }
    }

    // Separator line under tabs
    SDL_Rect border = {x, y + tab_height, width, 1};
    ui_draw_filled_rect(renderer, border, COLOR_BORDER_INNER);
}

void ui_tab_navigate(tab_bar_t *tabs, int delta) {
    if (!tabs || tabs->tab_count == 0) return;

    tabs->active_tab += delta;

    if (tabs->active_tab < 0) {
        tabs->active_tab = tabs->tab_count - 1;
    } else if (tabs->active_tab >= tabs->tab_count) {
        tabs->active_tab = 0;
    }
}

/* ============================================
 * Button
 * ============================================ */

void ui_draw_button(SDL_Renderer *renderer, ui_button_t *button, TTF_Font *font) {
    if (!renderer || !button || !font) return;

    SDL_Rect rect = {button->x, button->y, button->width, button->height};

    if (button->is_selected) {
        ui_draw_rpg_window(renderer, rect);
        ui_draw_text(renderer, font, button->label,
                    button->x + button->width / 2,
                    button->y + (button->height - 14) / 2,
                    COLOR_CURSOR, TEXT_ALIGN_CENTER);
    } else {
        SDL_Color bg_color = button->is_primary ? COLOR_BG_HOVER : COLOR_BG_PANEL;
        ui_draw_bordered_rect(renderer, rect, bg_color, COLOR_BORDER_INNER, 1);
        ui_draw_text(renderer, font, button->label,
                    button->x + button->width / 2,
                    button->y + (button->height - 14) / 2,
                    COLOR_TEXT_PRIMARY, TEXT_ALIGN_CENTER);
    }
}

/* ============================================
 * Toggle Switch
 * ============================================ */

void ui_draw_toggle(SDL_Renderer *renderer, int x, int y, int is_on) {
    if (!renderer) return;

    int width = 44;
    int height = 22;

    SDL_Color bg = is_on ? COLOR_STATE_ON : COLOR_STATE_OFF;
    SDL_Rect track = {x, y, width, height};
    ui_draw_filled_rect(renderer, track, bg);

    set_render_color(renderer, COLOR_BORDER_OUTER);
    SDL_RenderDrawRect(renderer, &track);

    int knob_x = is_on ? (x + width - 20) : (x + 2);
    SDL_Rect knob = {knob_x, y + 2, 18, 18};
    ui_draw_filled_rect(renderer, knob, COLOR_WHITE);
}

/* ============================================
 * Slider
 * ============================================ */

void ui_draw_slider(SDL_Renderer *renderer, int x, int y, int width,
                    float value, float min_val, float max_val) {
    if (!renderer || max_val <= min_val) return;

    int height = 18;

    SDL_Rect track = {x, y, width, height};
    ui_draw_filled_rect(renderer, track, COLOR_BG_DARK);
    set_render_color(renderer, COLOR_BORDER_INNER);
    SDL_RenderDrawRect(renderer, &track);

    float percent = (value - min_val) / (max_val - min_val);
    if (percent < 0) percent = 0;
    if (percent > 1) percent = 1;

    int fill_width = (int)((width - 4) * percent);
    if (fill_width > 0) {
        SDL_Rect fill = {x + 2, y + 2, fill_width, height - 4};
        ui_draw_filled_rect(renderer, fill, COLOR_CURSOR);
    }
}

/* ============================================
 * Header Bar - RPG-style title window
 * ============================================ */

void ui_draw_header(SDL_Renderer *renderer, TTF_Font *font_title,
                    TTF_Font *font_status, const char *title, int is_connected) {
    if (!renderer || !font_title) return;

    // Header window
    SDL_Rect header = {8, 6, 624, 42};
    ui_draw_rpg_window(renderer, header);

    // Title
    ui_draw_text(renderer, font_title, title, 320, 16,
                COLOR_WHITE, TEXT_ALIGN_CENTER);

    // Connection status dot + text
    if (font_status) {
        SDL_Color status_color = is_connected ? COLOR_ON : COLOR_OFF;
        const char *status = is_connected ? "ONLINE" : "OFFLINE";

        // Status dot
        SDL_Rect dot = {590, 22, 8, 8};
        ui_draw_filled_rect(renderer, dot, status_color);

        ui_draw_text(renderer, font_status, status, 586, 20,
                    status_color, TEXT_ALIGN_RIGHT);
    }
}

/* ============================================
 * Button Hints Bar
 * ============================================ */

void ui_draw_button_hints(SDL_Renderer *renderer, TTF_Font *font,
                          const char **hints, int count) {
    if (!renderer || !font || !hints || count == 0) return;

    // Bottom hints in a subtle RPG window
    SDL_Rect bar = {8, 452, 624, 24};
    ui_draw_rpg_window(renderer, bar);

    int total_width = 600;
    int spacing = total_width / count;

    for (int i = 0; i < count; i++) {
        ui_draw_text(renderer, font, hints[i],
                    20 + (i * spacing) + spacing / 2, 456,
                    COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
    }
}

/* ============================================
 * Dialog - RPG window style
 * ============================================ */

void ui_draw_dialog(SDL_Renderer *renderer, dialog_t *dialog,
                    TTF_Font *font_title, TTF_Font *font_body) {
    if (!renderer || !dialog || !dialog->visible) return;

    // Dim background
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect screen = {0, 0, 640, 480};
    SDL_RenderFillRect(renderer, &screen);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Dialog window
    SDL_Rect box = {120, 140, 400, 200};
    ui_draw_rpg_window(renderer, box);

    if (font_title) {
        ui_draw_text(renderer, font_title, dialog->title,
                    320, 160, COLOR_WHITE, TEXT_ALIGN_CENTER);
    }

    if (font_body) {
        ui_draw_text(renderer, font_body, dialog->message,
                    320, 210, COLOR_GRAY, TEXT_ALIGN_CENTER);
    }

    if (font_body) {
        SDL_Color yes_color = (dialog->selected_option == 0) ? COLOR_CURSOR : COLOR_GRAY;
        SDL_Color no_color = (dialog->selected_option == 1) ? COLOR_CURSOR : COLOR_GRAY;

        const char *yes_text = (dialog->selected_option == 0) ? "> YES" : "  YES";
        const char *no_text = (dialog->selected_option == 1) ? "> NO" : "  NO";

        ui_draw_text(renderer, font_body, yes_text, 220, 280, yes_color, TEXT_ALIGN_CENTER);
        ui_draw_text(renderer, font_body, no_text, 420, 280, no_color, TEXT_ALIGN_CENTER);
    }
}

void ui_dialog_navigate(dialog_t *dialog, int delta) {
    if (!dialog) return;
    dialog->selected_option = (dialog->selected_option + delta + 2) % 2;
}

/* ============================================
 * Scrollbar
 * ============================================ */

void ui_draw_scrollbar(SDL_Renderer *renderer, int x, int y, int height,
                       int total_items, int visible_items, int scroll_offset) {
    if (!renderer || total_items <= visible_items) return;

    int scrollbar_height = (visible_items * height) / total_items;
    if (scrollbar_height < 12) scrollbar_height = 12;

    int scrollbar_y = y + ((scroll_offset * (height - scrollbar_height)) / (total_items - visible_items));

    // Track
    SDL_Rect track = {x, y, 6, height};
    ui_draw_filled_rect(renderer, track, COLOR_BG_DARK);

    // Thumb
    SDL_Rect thumb = {x, scrollbar_y, 6, scrollbar_height};
    ui_draw_filled_rect(renderer, thumb, COLOR_BORDER_OUTER);
}
