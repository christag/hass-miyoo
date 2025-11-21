/**
 * screen_automation.c - Automation Detail Screen Implementation
 */

#include "screen_automation.h"
#include "../utils/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void parse_automation_info(automation_screen_t *screen);
static void format_timestamp(const char *iso_time, char *output, size_t output_size, const char *prefix);
static int trigger_automation(automation_screen_t *screen);

automation_screen_t* automation_screen_create(SDL_Renderer *renderer,
                                               font_manager_t *fonts,
                                               icon_manager_t *icons,
                                               cache_manager_t *cache_mgr,
                                               ha_client_t **client_ptr) {
    if (!renderer || !fonts || !icons) return NULL;

    automation_screen_t *screen = calloc(1, sizeof(automation_screen_t));
    if (!screen) return NULL;

    screen->renderer = renderer;
    screen->fonts = fonts;
    screen->icons = icons;
    screen->cache_mgr = cache_mgr;
    screen->client_ptr = client_ptr;

    return screen;
}

void automation_screen_destroy(automation_screen_t *screen) {
    if (!screen) return;
    if (screen->entity) free_entity(screen->entity);
    free(screen);
}

int automation_screen_set_entity(automation_screen_t *screen, const char *entity_id) {
    if (!screen || !entity_id) return 0;

    if (screen->entity) {
        free_entity(screen->entity);
        screen->entity = NULL;
    }

    strncpy(screen->entity_id, entity_id, sizeof(screen->entity_id) - 1);

    if (screen->cache_mgr) {
        screen->entity = cache_manager_get_entity(screen->cache_mgr, entity_id);
    }

    if (!screen->entity) {
        strcpy(screen->status_message, "Automation not found");
        return 0;
    }

    if (screen->cache_mgr) {
        screen->is_favorite = cache_manager_is_favorite(screen->cache_mgr, entity_id);
    }

    parse_automation_info(screen);
    screen->status_message[0] = '\0';
    return 1;
}

int automation_screen_handle_input(automation_screen_t *screen, SDL_Event *event) {
    if (!screen || !event || event->type != SDL_KEYDOWN) return 0;

    if (input_button_pressed(BTN_B)) {
        return -1;
    }

    if (input_button_pressed(BTN_A)) {
        if (trigger_automation(screen)) {
            strcpy(screen->status_message, "Triggered!");
        } else {
            strcpy(screen->status_message, "Trigger failed");
        }
        return 0;
    }

    if (input_button_pressed(BTN_Y)) {
        if (screen->cache_mgr) {
            int result = cache_manager_toggle_favorite(screen->cache_mgr, screen->entity_id);
            screen->is_favorite = (result == 1);
            strcpy(screen->status_message, result == 1 ? "Added to favorites" : "Removed from favorites");
        }
        return 0;
    }

    return 0;
}

void automation_screen_render(automation_screen_t *screen) {
    if (!screen) return;

    SDL_Renderer *r = screen->renderer;
    TTF_Font *font_header = fonts_get(screen->fonts, FONT_SIZE_HEADER);
    TTF_Font *font_body = fonts_get(screen->fonts, FONT_SIZE_BODY);
    TTF_Font *font_small = fonts_get(screen->fonts, FONT_SIZE_SMALL);

    set_render_color(r, COLOR_BACKGROUND);
    SDL_RenderClear(r);

    int is_online = screen->cache_mgr ? cache_manager_is_online(screen->cache_mgr) : 0;
    ui_draw_header(r, font_header, font_small, "AUTOMATION", is_online);

    if (!screen->entity) {
        ui_draw_text(r, font_body, "No automation selected", 320, 200, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
        return;
    }

    // Card background
    SDL_Rect card = {40, 70, 560, 350};
    ui_draw_bordered_rect(r, card, COLOR_GB_DARK, COLOR_GB_DARKEST, 2);

    // Name
    const char *name = strlen(screen->entity->friendly_name) > 0 ?
                       screen->entity->friendly_name : screen->entity->entity_id;
    ui_draw_text(r, font_header, name, 320, 90, COLOR_TEXT_PRIMARY, TEXT_ALIGN_CENTER);

    // Icon
    icons_draw(screen->icons, "automation_robot", 320 - 32, 120, 64);

    // Status
    char status_text[64];
    snprintf(status_text, sizeof(status_text), "Status: %s", screen->is_enabled ? "Enabled" : "Disabled");
    ui_draw_text(r, font_body, status_text, 320, 200, COLOR_TEXT_PRIMARY, TEXT_ALIGN_CENTER);

    // Last triggered
    ui_draw_text(r, font_small, screen->last_triggered, 320, 225, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);

    // Mode
    if (strlen(screen->mode) > 0) {
        char mode_text[64];
        snprintf(mode_text, sizeof(mode_text), "Mode: %s", screen->mode);
        ui_draw_text(r, font_small, mode_text, 320, 245, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
    }

    // Description box
    SDL_Rect desc_box = {60, 270, 520, 60};
    ui_draw_bordered_rect(r, desc_box, COLOR_GB_DARK, COLOR_GB_DARKEST, 1);
    if (strlen(screen->description) > 0) {
        ui_draw_text_truncated(r, font_small, screen->description, 70, 280, 500, COLOR_TEXT_SECONDARY);
    } else {
        ui_draw_text(r, font_small, "No description", 320, 290, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
    }

    // Trigger button
    SDL_Rect btn = {220, 340, 200, 30};
    ui_draw_bordered_rect(r, btn, COLOR_GB_LIGHT, COLOR_GB_DARKEST, 2);
    ui_draw_text(r, font_body, "[A] TRIGGER", 320, 347, COLOR_GB_DARKEST, TEXT_ALIGN_CENTER);

    // Favorite
    icons_draw(screen->icons, screen->is_favorite ? "star_filled" : "star_empty", 60, 380, 16);
    ui_draw_text(r, font_small, screen->is_favorite ? "Favorited" : "Add to Favorites", 80, 382, COLOR_TEXT_PRIMARY, TEXT_ALIGN_LEFT);

    // Entity ID
    char id_text[160];
    snprintf(id_text, sizeof(id_text), "ID: %s", screen->entity->entity_id);
    ui_draw_text_truncated(r, font_small, id_text, 60, 400, 500, COLOR_TEXT_SECONDARY);

    // Status message
    if (strlen(screen->status_message) > 0) {
        ui_draw_text(r, font_small, screen->status_message, 320, 420, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
    }

    const char *hints[] = {"[A] Trigger", "[Y] Fav", "[B] Back"};
    ui_draw_button_hints(r, font_body, hints, 3);
}

static void parse_automation_info(automation_screen_t *screen) {
    if (!screen || !screen->entity) return;

    screen->description[0] = '\0';
    screen->last_triggered[0] = '\0';
    screen->mode[0] = '\0';
    screen->is_enabled = (strcmp(screen->entity->state, "on") == 0);

    const char *attrs = screen->entity->attributes_json;
    if (!attrs) return;

    // Parse last_triggered
    const char *triggered_ptr = strstr(attrs, "\"last_triggered\":");
    if (triggered_ptr) {
        triggered_ptr += 17;
        while (*triggered_ptr == ' ' || *triggered_ptr == '"') triggered_ptr++;
        if (strncmp(triggered_ptr, "null", 4) != 0) {
            char iso_time[64] = {0};
            int i = 0;
            while (*triggered_ptr && *triggered_ptr != '"' && i < 63) {
                iso_time[i++] = *triggered_ptr++;
            }
            format_timestamp(iso_time, screen->last_triggered, sizeof(screen->last_triggered), "Last triggered");
        } else {
            strcpy(screen->last_triggered, "Never triggered");
        }
    }

    // Parse mode
    const char *mode_ptr = strstr(attrs, "\"mode\":");
    if (mode_ptr) {
        mode_ptr += 7;
        while (*mode_ptr == ' ' || *mode_ptr == '"') mode_ptr++;
        int i = 0;
        while (*mode_ptr && *mode_ptr != '"' && i < 30) {
            screen->mode[i++] = *mode_ptr++;
        }
    }

    // Parse description
    const char *desc_ptr = strstr(attrs, "\"description\":");
    if (desc_ptr) {
        desc_ptr += 14;
        while (*desc_ptr == ' ' || *desc_ptr == '"') desc_ptr++;
        int i = 0;
        while (*desc_ptr && *desc_ptr != '"' && i < 510) {
            screen->description[i++] = *desc_ptr++;
        }
    }
}

static void format_timestamp(const char *iso_time, char *output, size_t output_size, const char *prefix) {
    if (!iso_time || strlen(iso_time) == 0) {
        snprintf(output, output_size, "%s: Unknown", prefix);
        return;
    }

    struct tm tm_time = {0};
    int year, month, day, hour, minute, second;

    if (sscanf(iso_time, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) >= 6) {
        tm_time.tm_year = year - 1900;
        tm_time.tm_mon = month - 1;
        tm_time.tm_mday = day;
        tm_time.tm_hour = hour;
        tm_time.tm_min = minute;
        tm_time.tm_sec = second;

        time_t changed_time = mktime(&tm_time);
        time_t now = time(NULL);
        double diff = difftime(now, changed_time);

        if (diff < 60) snprintf(output, output_size, "%s: %d sec ago", prefix, (int)diff);
        else if (diff < 3600) snprintf(output, output_size, "%s: %d min ago", prefix, (int)(diff/60));
        else if (diff < 86400) snprintf(output, output_size, "%s: %d hr ago", prefix, (int)(diff/3600));
        else snprintf(output, output_size, "%s: %d days ago", prefix, (int)(diff/86400));
    } else {
        snprintf(output, output_size, "%s: Unknown", prefix);
    }
}

static int trigger_automation(automation_screen_t *screen) {
    if (!screen || !screen->client_ptr || !*screen->client_ptr || !screen->entity) return 0;

    ha_response_t *response = ha_client_call_service(
        *screen->client_ptr, "automation", "trigger", screen->entity->entity_id, NULL
    );

    int success = (response && response->success);
    if (response) ha_response_free(response);
    return success;
}
