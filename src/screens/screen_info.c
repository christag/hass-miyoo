/**
 * screen_info.c - Generic Entity Info Screen (Read-Only)
 *
 * Displays read-only information for entities without dedicated screens.
 * Used for: sensors, binary_sensors, input_*, person, zone, weather, etc.
 *
 * Phase 9.1: Screen Architecture Refactor
 */

#include "screen_info.h"
#include "../utils/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void parse_entity_info(info_screen_t *screen);
static void format_timestamp(const char *iso_time, char *output, size_t output_size);
static const char* get_domain_display_name(const char *entity_id);

info_screen_t* info_screen_create(SDL_Renderer *renderer,
                                   font_manager_t *fonts,
                                   icon_manager_t *icons,
                                   cache_manager_t *cache_mgr,
                                   ha_client_t **client_ptr) {
    if (!renderer || !fonts || !icons) return NULL;

    info_screen_t *screen = calloc(1, sizeof(info_screen_t));
    if (!screen) return NULL;

    screen->renderer = renderer;
    screen->fonts = fonts;
    screen->icons = icons;
    screen->cache_mgr = cache_mgr;
    screen->client_ptr = client_ptr;

    return screen;
}

void info_screen_destroy(info_screen_t *screen) {
    if (!screen) return;
    if (screen->entity) free_entity(screen->entity);
    free(screen);
}

int info_screen_set_entity(info_screen_t *screen, const char *entity_id) {
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
        strcpy(screen->status_message, "Entity not found");
        return 0;
    }

    if (screen->cache_mgr) {
        screen->is_favorite = cache_manager_is_favorite(screen->cache_mgr, entity_id);
    }

    parse_entity_info(screen);
    screen->status_message[0] = '\0';
    return 1;
}

int info_screen_handle_input(info_screen_t *screen, SDL_Event *event) {
    if (!screen || !event || event->type != SDL_KEYDOWN) return 0;

    if (input_button_pressed(BTN_B)) {
        return -1;
    }

    // Y button - toggle favorite (only action available on read-only screen)
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

void info_screen_render(info_screen_t *screen) {
    if (!screen) return;

    SDL_Renderer *r = screen->renderer;
    TTF_Font *font_header = fonts_get(screen->fonts, FONT_SIZE_HEADER);
    TTF_Font *font_body = fonts_get(screen->fonts, FONT_SIZE_BODY);
    TTF_Font *font_small = fonts_get(screen->fonts, FONT_SIZE_SMALL);

    set_render_color(r, COLOR_BACKGROUND);
    SDL_RenderClear(r);

    // Header - use domain name
    const char *header_title = get_domain_display_name(screen->entity_id);
    int is_online = screen->cache_mgr ? cache_manager_is_online(screen->cache_mgr) : 0;
    ui_draw_header(r, font_header, font_small, header_title, is_online);

    if (!screen->entity) {
        ui_draw_text(r, font_body, "No entity selected", 320, 200, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
        return;
    }

    // Card background
    SDL_Rect card = {40, 70, 560, 350};
    ui_draw_bordered_rect(r, card, COLOR_GB_DARK, COLOR_GB_DARKEST, 2);

    // Entity name
    const char *name = strlen(screen->entity->friendly_name) > 0 ?
                       screen->entity->friendly_name : screen->entity->entity_id;
    ui_draw_text(r, font_header, name, 320, 90, COLOR_TEXT_PRIMARY, TEXT_ALIGN_CENTER);

    // Large icon
    const char *icon_name = icons_get_for_domain(screen->entity->entity_id);
    icons_draw(screen->icons, icon_name, 320 - 32, 120, 64);

    // Current state (large, prominent)
    char state_display[128];
    snprintf(state_display, sizeof(state_display), "%s", screen->entity->state);
    // Capitalize first letter
    if (state_display[0] >= 'a' && state_display[0] <= 'z') {
        state_display[0] -= 32;
    }
    ui_draw_text(r, font_header, state_display, 320, 200, COLOR_TEXT_PRIMARY, TEXT_ALIGN_CENTER);

    // Last changed
    ui_draw_text(r, font_small, screen->last_triggered, 320, 230, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);

    // Attributes section
    SDL_Rect attr_box = {60, 260, 520, 100};
    ui_draw_bordered_rect(r, attr_box, COLOR_GB_DARK, COLOR_GB_DARKEST, 1);
    ui_draw_text(r, font_small, "Attributes:", 70, 268, COLOR_TEXT_PRIMARY, TEXT_ALIGN_LEFT);

    // Show some key attributes (simplified display)
    if (strlen(screen->description) > 0) {
        ui_draw_text_truncated(r, font_small, screen->description, 70, 288, 500, COLOR_TEXT_SECONDARY);
    } else {
        ui_draw_text(r, font_small, "(Read-only entity)", 320, 300, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
    }

    // Favorite
    icons_draw(screen->icons, screen->is_favorite ? "star_filled" : "star_empty", 60, 370, 16);
    ui_draw_text(r, font_small, screen->is_favorite ? "Favorited" : "Add to Favorites", 80, 372, COLOR_TEXT_PRIMARY, TEXT_ALIGN_LEFT);

    // Entity ID
    char id_text[160];
    snprintf(id_text, sizeof(id_text), "ID: %s", screen->entity->entity_id);
    ui_draw_text_truncated(r, font_small, id_text, 60, 395, 500, COLOR_TEXT_SECONDARY);

    // Status message
    if (strlen(screen->status_message) > 0) {
        ui_draw_text(r, font_small, screen->status_message, 320, 420, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
    }

    // Button hints - read only, just favorite and back
    const char *hints[] = {"[Y] Fav", "[B] Back"};
    ui_draw_button_hints(r, font_body, hints, 2);
}

int info_screen_should_handle(const char *entity_id) {
    // This is now the catch-all - returns true for entities NOT handled elsewhere
    // Handled elsewhere: light, switch, fan, climate, cover, lock, automation, script, scene
    if (!entity_id) return 0;

    // Check domains that have dedicated screens
    if (strncmp(entity_id, "light.", 6) == 0) return 0;
    if (strncmp(entity_id, "switch.", 7) == 0) return 0;
    if (strncmp(entity_id, "fan.", 4) == 0) return 0;
    if (strncmp(entity_id, "climate.", 8) == 0) return 0;
    if (strncmp(entity_id, "cover.", 6) == 0) return 0;
    if (strncmp(entity_id, "lock.", 5) == 0) return 0;
    if (strncmp(entity_id, "automation.", 11) == 0) return 0;
    if (strncmp(entity_id, "script.", 7) == 0) return 0;
    if (strncmp(entity_id, "scene.", 6) == 0) return 0;

    // Everything else uses info screen
    return 1;
}

/* ============================================
 * Static Helper Functions
 * ============================================ */

static void parse_entity_info(info_screen_t *screen) {
    if (!screen || !screen->entity) return;

    screen->description[0] = '\0';
    screen->last_triggered[0] = '\0';
    screen->mode[0] = '\0';
    screen->is_enabled = 1;

    // Format last_changed
    format_timestamp(screen->entity->last_changed, screen->last_triggered, sizeof(screen->last_triggered));

    // Extract some useful attributes for display
    const char *attrs = screen->entity->attributes_json;
    if (attrs && strlen(attrs) > 2) {
        // Try to extract unit_of_measurement
        const char *unit_ptr = strstr(attrs, "\"unit_of_measurement\":");
        if (unit_ptr) {
            unit_ptr += 22;
            while (*unit_ptr == ' ' || *unit_ptr == '"') unit_ptr++;
            char unit[32] = {0};
            int i = 0;
            while (*unit_ptr && *unit_ptr != '"' && i < 30) {
                unit[i++] = *unit_ptr++;
            }
            if (strlen(unit) > 0) {
                snprintf(screen->description, sizeof(screen->description), "Unit: %s", unit);
            }
        }

        // Try device_class
        if (strlen(screen->description) == 0) {
            const char *class_ptr = strstr(attrs, "\"device_class\":");
            if (class_ptr) {
                class_ptr += 15;
                while (*class_ptr == ' ' || *class_ptr == '"') class_ptr++;
                char device_class[64] = {0};
                int i = 0;
                while (*class_ptr && *class_ptr != '"' && i < 62) {
                    device_class[i++] = *class_ptr++;
                }
                if (strlen(device_class) > 0) {
                    snprintf(screen->description, sizeof(screen->description), "Type: %s", device_class);
                }
            }
        }
    }
}

static void format_timestamp(const char *iso_time, char *output, size_t output_size) {
    if (!iso_time || strlen(iso_time) == 0) {
        snprintf(output, output_size, "Last changed: Unknown");
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

        if (diff < 60) snprintf(output, output_size, "Last changed: %d sec ago", (int)diff);
        else if (diff < 3600) snprintf(output, output_size, "Last changed: %d min ago", (int)(diff/60));
        else if (diff < 86400) snprintf(output, output_size, "Last changed: %d hr ago", (int)(diff/3600));
        else snprintf(output, output_size, "Last changed: %d days ago", (int)(diff/86400));
    } else {
        snprintf(output, output_size, "Last changed: Unknown");
    }
}

static const char* get_domain_display_name(const char *entity_id) {
    if (!entity_id) return "INFO";

    if (strncmp(entity_id, "sensor.", 7) == 0) return "SENSOR";
    if (strncmp(entity_id, "binary_sensor.", 14) == 0) return "BINARY SENSOR";
    if (strncmp(entity_id, "input_boolean.", 14) == 0) return "INPUT BOOLEAN";
    if (strncmp(entity_id, "input_number.", 13) == 0) return "INPUT NUMBER";
    if (strncmp(entity_id, "input_select.", 13) == 0) return "INPUT SELECT";
    if (strncmp(entity_id, "input_text.", 11) == 0) return "INPUT TEXT";
    if (strncmp(entity_id, "input_datetime.", 15) == 0) return "INPUT DATETIME";
    if (strncmp(entity_id, "person.", 7) == 0) return "PERSON";
    if (strncmp(entity_id, "zone.", 5) == 0) return "ZONE";
    if (strncmp(entity_id, "weather.", 8) == 0) return "WEATHER";
    if (strncmp(entity_id, "sun.", 4) == 0) return "SUN";
    if (strncmp(entity_id, "media_player.", 13) == 0) return "MEDIA PLAYER";
    if (strncmp(entity_id, "camera.", 7) == 0) return "CAMERA";
    if (strncmp(entity_id, "device_tracker.", 15) == 0) return "DEVICE TRACKER";
    if (strncmp(entity_id, "update.", 7) == 0) return "UPDATE";
    if (strncmp(entity_id, "button.", 7) == 0) return "BUTTON";
    if (strncmp(entity_id, "number.", 7) == 0) return "NUMBER";
    if (strncmp(entity_id, "select.", 7) == 0) return "SELECT";
    if (strncmp(entity_id, "text.", 5) == 0) return "TEXT";

    return "ENTITY INFO";
}
