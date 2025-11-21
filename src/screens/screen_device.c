/**
 * screen_device.c - Device/Entity Detail Screen Implementation
 */

#include "screen_device.h"
#include "../utils/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Light supported_features bitmask values */
#define SUPPORT_BRIGHTNESS 1
#define SUPPORT_COLOR_TEMP 2
#define SUPPORT_EFFECT 4
#define SUPPORT_FLASH 8
#define SUPPORT_COLOR 16
#define SUPPORT_TRANSITION 32
#define SUPPORT_WHITE_VALUE 128

/* Cover supported_features */
#define COVER_SUPPORT_OPEN 1
#define COVER_SUPPORT_CLOSE 2
#define COVER_SUPPORT_SET_POSITION 4
#define COVER_SUPPORT_STOP 8
#define COVER_SUPPORT_OPEN_TILT 16
#define COVER_SUPPORT_CLOSE_TILT 32
#define COVER_SUPPORT_SET_TILT_POSITION 128

/* Climate supported_features */
#define CLIMATE_SUPPORT_TARGET_TEMP 1
#define CLIMATE_SUPPORT_TARGET_TEMP_RANGE 2
#define CLIMATE_SUPPORT_TARGET_HUMIDITY 4
#define CLIMATE_SUPPORT_FAN_MODE 8
#define CLIMATE_SUPPORT_PRESET_MODE 16
#define CLIMATE_SUPPORT_SWING_MODE 32
#define CLIMATE_SUPPORT_AUX_HEAT 64

/* Forward declarations */
static void determine_control_type(device_screen_t *screen);
static void extract_control_value(device_screen_t *screen);
static int send_control_action(device_screen_t *screen);
static void draw_large_icon(device_screen_t *screen, int x, int y);
static void draw_slider_row(device_screen_t *screen, int y, const char *label,
                            int value, int min_val, int max_val,
                            int is_selected, int is_temp_kelvin);
static void draw_control(device_screen_t *screen);
static void format_last_changed(const char *iso_time, char *output, size_t output_size);

device_screen_t* device_screen_create(SDL_Renderer *renderer,
                                       font_manager_t *fonts,
                                       icon_manager_t *icons,
                                       cache_manager_t *cache_mgr,
                                       ha_client_t **client_ptr) {
    if (!renderer || !fonts || !icons) {
        return NULL;
    }

    device_screen_t *screen = calloc(1, sizeof(device_screen_t));
    if (!screen) {
        return NULL;
    }

    screen->renderer = renderer;
    screen->fonts = fonts;
    screen->icons = icons;
    screen->cache_mgr = cache_mgr;
    screen->client_ptr = client_ptr;

    screen->entity = NULL;
    screen->control_type = CTRL_NONE;
    screen->selected_control = 0;

    return screen;
}

void device_screen_destroy(device_screen_t *screen) {
    if (!screen) return;

    if (screen->entity) {
        free_entity(screen->entity);
    }

    free(screen);
}

int device_screen_set_entity(device_screen_t *screen, const char *entity_id) {
    if (!screen || !entity_id) return 0;

    // Free previous entity
    if (screen->entity) {
        free_entity(screen->entity);
        screen->entity = NULL;
    }

    strncpy(screen->entity_id, entity_id, sizeof(screen->entity_id) - 1);

    // Load entity from cache
    if (screen->cache_mgr) {
        screen->entity = cache_manager_get_entity(screen->cache_mgr, entity_id);
    }

    if (!screen->entity) {
        strcpy(screen->status_message, "Entity not found");
        return 0;
    }

    // Check favorite status
    if (screen->cache_mgr) {
        screen->is_favorite = cache_manager_is_favorite(screen->cache_mgr, entity_id);
    }

    // Determine control type and extract current value
    determine_control_type(screen);
    extract_control_value(screen);

    screen->selected_control = 0;
    strcpy(screen->status_message, "");

    return 1;
}

int device_screen_handle_input(device_screen_t *screen, SDL_Event *event) {
    if (!screen || !event || event->type != SDL_KEYDOWN) return 0;

    // Back button
    if (input_button_pressed(BTN_B)) {
        return -1;
    }

    // Calculate which control indices map to what
    // For lights: 0=toggle, 1=brightness (if has), 2=color_temp (if has), last=favorite
    int brightness_idx = screen->has_brightness ? 1 : -1;
    int color_temp_idx = screen->has_color_temp ? (screen->has_brightness ? 2 : 1) : -1;
    int favorite_idx = screen->max_controls - 1;

    // Navigation between controls
    if (input_button_pressed(BTN_DPAD_UP)) {
        if (screen->selected_control > 0) {
            screen->selected_control--;
        }
        return 0;
    }
    if (input_button_pressed(BTN_DPAD_DOWN)) {
        if (screen->selected_control < screen->max_controls - 1) {
            screen->selected_control++;
        }
        return 0;
    }

    // Slider adjustment with left/right
    // Check if on brightness slider
    if (screen->selected_control == brightness_idx && screen->has_brightness) {
        if (input_button_pressed(BTN_DPAD_LEFT)) {
            screen->brightness_value -= 25;
            if (screen->brightness_value < 0) screen->brightness_value = 0;
            screen->control_value = screen->brightness_value;
            return 0;
        }
        if (input_button_pressed(BTN_DPAD_RIGHT)) {
            screen->brightness_value += 25;
            if (screen->brightness_value > 255) screen->brightness_value = 255;
            screen->control_value = screen->brightness_value;
            return 0;
        }
    }

    // Check if on color temp slider
    if (screen->selected_control == color_temp_idx && screen->has_color_temp) {
        int step = (screen->color_temp_max - screen->color_temp_min) / 10;
        if (step < 10) step = 10;
        if (input_button_pressed(BTN_DPAD_LEFT)) {
            screen->color_temp_value -= step;
            if (screen->color_temp_value < screen->color_temp_min) {
                screen->color_temp_value = screen->color_temp_min;
            }
            return 0;
        }
        if (input_button_pressed(BTN_DPAD_RIGHT)) {
            screen->color_temp_value += step;
            if (screen->color_temp_value > screen->color_temp_max) {
                screen->color_temp_value = screen->color_temp_max;
            }
            return 0;
        }
    }

    // Legacy slider handling for climate/cover
    if (screen->selected_control == 1 &&
        !screen->has_brightness && !screen->has_color_temp &&
        (screen->control_type == CTRL_TEMPERATURE ||
         screen->control_type == CTRL_POSITION)) {

        if (input_button_pressed(BTN_DPAD_LEFT)) {
            screen->control_value -= screen->control_step;
            if (screen->control_value < screen->control_min) {
                screen->control_value = screen->control_min;
            }
            return 0;
        }
        if (input_button_pressed(BTN_DPAD_RIGHT)) {
            screen->control_value += screen->control_step;
            if (screen->control_value > screen->control_max) {
                screen->control_value = screen->control_max;
            }
            return 0;
        }
    }

    // A button - activate/toggle
    if (input_button_pressed(BTN_A)) {
        if (screen->selected_control == 0) {
            // Main toggle/activate
            if (send_control_action(screen)) {
                strcpy(screen->status_message, "Action sent!");
            } else {
                strcpy(screen->status_message, "Action failed");
            }
        } else if (screen->selected_control == brightness_idx && screen->has_brightness) {
            // Apply brightness value
            if (send_control_action(screen)) {
                strcpy(screen->status_message, "Brightness applied!");
            } else {
                strcpy(screen->status_message, "Failed to apply");
            }
        } else if (screen->selected_control == color_temp_idx && screen->has_color_temp) {
            // Apply color temp value
            if (send_control_action(screen)) {
                strcpy(screen->status_message, "Color temp applied!");
            } else {
                strcpy(screen->status_message, "Failed to apply");
            }
        } else if (screen->selected_control == favorite_idx) {
            // Favorite toggle
            if (screen->cache_mgr) {
                int result = cache_manager_toggle_favorite(screen->cache_mgr, screen->entity_id);
                if (result == 1) {
                    screen->is_favorite = 1;
                    strcpy(screen->status_message, "Added to favorites");
                } else if (result == 0) {
                    screen->is_favorite = 0;
                    strcpy(screen->status_message, "Removed from favorites");
                }
            }
        } else if (screen->control_type == CTRL_TEMPERATURE ||
                   screen->control_type == CTRL_POSITION) {
            // Legacy slider action
            if (send_control_action(screen)) {
                strcpy(screen->status_message, "Value applied!");
            } else {
                strcpy(screen->status_message, "Failed to apply");
            }
        }
        return 0;
    }

    // Y button - quick toggle favorite
    if (input_button_pressed(BTN_Y)) {
        if (screen->cache_mgr) {
            int result = cache_manager_toggle_favorite(screen->cache_mgr, screen->entity_id);
            if (result == 1) {
                screen->is_favorite = 1;
                strcpy(screen->status_message, "Added to favorites");
            } else if (result == 0) {
                screen->is_favorite = 0;
                strcpy(screen->status_message, "Removed from favorites");
            }
        }
        return 0;
    }

    // START - refresh
    if (input_button_pressed(BTN_START)) {
        device_screen_refresh(screen);
        strcpy(screen->status_message, "Refreshed");
        return 0;
    }

    return 0;
}

void device_screen_render(device_screen_t *screen) {
    if (!screen) return;

    SDL_Renderer *r = screen->renderer;
    TTF_Font *font_header = fonts_get(screen->fonts, FONT_SIZE_HEADER);
    TTF_Font *font_body = fonts_get(screen->fonts, FONT_SIZE_BODY);
    TTF_Font *font_small = fonts_get(screen->fonts, FONT_SIZE_SMALL);

    // Clear background
    set_render_color(r, COLOR_BACKGROUND);
    SDL_RenderClear(r);

    // Header
    int is_online = screen->cache_mgr ? cache_manager_is_online(screen->cache_mgr) : 0;
    ui_draw_header(r, font_header, font_small, "ENTITY DETAIL", is_online);

    if (!screen->entity) {
        ui_draw_text(r, font_body, "No entity selected",
                    320, 200, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
        return;
    }

    // Detail card background
    SDL_Rect card = {60, 70, 520, 340};
    ui_draw_bordered_rect(r, card, COLOR_GB_DARK, COLOR_GB_DARKEST, 2);

    // Entity name (title)
    const char *name = strlen(screen->entity->friendly_name) > 0 ?
                       screen->entity->friendly_name : screen->entity->entity_id;
    ui_draw_text_truncated(r, font_header, name, 320, 85, 480, COLOR_TEXT_PRIMARY);
    // Center align hack - draw at center
    int name_width = 0;
    TTF_SizeText(font_header, name, &name_width, NULL);
    ui_draw_text_truncated(r, font_header, name, 320 - name_width/2, 85, 480, COLOR_TEXT_PRIMARY);

    // Large icon
    draw_large_icon(screen, 320, 140);

    // Current state
    char state_text[64];
    snprintf(state_text, sizeof(state_text), "%s", screen->entity->state);
    // Capitalize first letter
    if (state_text[0] >= 'a' && state_text[0] <= 'z') {
        state_text[0] -= 32;
    }
    ui_draw_text(r, font_header, state_text, 320, 210, COLOR_TEXT_PRIMARY, TEXT_ALIGN_CENTER);

    // Draw control(s)
    draw_control(screen);

    // Last changed
    char last_changed_text[64];
    format_last_changed(screen->entity->last_changed, last_changed_text, sizeof(last_changed_text));
    ui_draw_text(r, font_small, last_changed_text, 80, 345, COLOR_TEXT_SECONDARY, TEXT_ALIGN_LEFT);

    // Entity ID
    char id_text[160];
    snprintf(id_text, sizeof(id_text), "ID: %s", screen->entity->entity_id);
    ui_draw_text_truncated(r, font_small, id_text, 80, 362, 460, COLOR_TEXT_SECONDARY);

    // Favorite indicator
    int fav_y = 380;
    int favorite_idx = screen->max_controls - 1;
    int fav_selected = (screen->selected_control == favorite_idx);

    if (fav_selected) {
        SDL_Rect sel_bg = {70, fav_y - 2, 200, 20};
        ui_draw_filled_rect(r, sel_bg, COLOR_SELECTED);
    }

    icons_draw(screen->icons, screen->is_favorite ? "star_filled" : "star_empty", 80, fav_y, 16);
    ui_draw_text(r, font_body, screen->is_favorite ? "Favorited" : "Add to Favorites",
                100, fav_y, fav_selected ? COLOR_GB_DARKEST : COLOR_TEXT_PRIMARY, TEXT_ALIGN_LEFT);

    // Status message
    if (strlen(screen->status_message) > 0) {
        ui_draw_text(r, font_small, screen->status_message,
                    320, 420, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
    }

    // Button hints
    const char *hints[] = {"[A] Action", "[Y] Fav", "[B] Back", "[START] Refresh"};
    ui_draw_button_hints(r, font_body, hints, 4);
}

void device_screen_refresh(device_screen_t *screen) {
    if (!screen || !screen->cache_mgr) return;

    // Refresh from API
    ha_entity_t *updated = cache_manager_refresh_entity(screen->cache_mgr, screen->entity_id);
    if (updated) {
        if (screen->entity) {
            free_entity(screen->entity);
        }
        screen->entity = updated;
        extract_control_value(screen);
    }
}

/* ============================================
 * Static Helper Functions
 * ============================================ */

static void determine_control_type(device_screen_t *screen) {
    if (!screen || !screen->entity) {
        screen->control_type = CTRL_NONE;
        return;
    }

    // Reset light-specific controls
    screen->has_brightness = 0;
    screen->has_color_temp = 0;
    screen->brightness_value = 0;
    screen->color_temp_value = 0;
    screen->color_temp_min = 153;  // ~6500K (cool white)
    screen->color_temp_max = 500;  // ~2000K (warm white)

    // Extract domain from entity_id
    char domain[32] = {0};
    const char *dot = strchr(screen->entity->entity_id, '.');
    if (dot) {
        size_t len = dot - screen->entity->entity_id;
        if (len < sizeof(domain)) {
            strncpy(domain, screen->entity->entity_id, len);
        }
    }

    int features = screen->entity->supported_features;

    // Determine control type based on domain and supported_features
    if (strcmp(domain, "light") == 0) {
        // Check supported features for lights
        screen->has_brightness = (features & SUPPORT_BRIGHTNESS) ? 1 : 0;
        screen->has_color_temp = (features & SUPPORT_COLOR_TEMP) ? 1 : 0;

        // Calculate max_controls: toggle + brightness? + color_temp? + favorite
        screen->max_controls = 1;  // Always have toggle (index 0)
        if (screen->has_brightness) screen->max_controls++;
        if (screen->has_color_temp) screen->max_controls++;
        screen->max_controls++;  // Favorite is always last

        if (screen->has_brightness) {
            screen->control_type = CTRL_BRIGHTNESS;
            screen->control_min = 0;
            screen->control_max = 255;
            screen->control_step = 25;
        } else {
            screen->control_type = CTRL_TOGGLE;
            screen->max_controls = 2;  // Just toggle + favorite
        }
    } else if (strcmp(domain, "switch") == 0 || strcmp(domain, "fan") == 0 ||
               strcmp(domain, "input_boolean") == 0) {
        screen->control_type = CTRL_TOGGLE;
    } else if (strcmp(domain, "climate") == 0) {
        // Check if climate supports target temperature
        if (features & CLIMATE_SUPPORT_TARGET_TEMP) {
            screen->control_type = CTRL_TEMPERATURE;
            screen->control_min = 60;   // Fahrenheit range
            screen->control_max = 85;
            screen->control_step = 1;
        } else {
            screen->control_type = CTRL_TOGGLE;
        }
    } else if (strcmp(domain, "cover") == 0) {
        // Check if cover supports position control
        if (features & COVER_SUPPORT_SET_POSITION) {
            screen->control_type = CTRL_POSITION;
            screen->control_min = 0;
            screen->control_max = 100;
            screen->control_step = 10;
        } else {
            screen->control_type = CTRL_TOGGLE;
        }
    } else if (strcmp(domain, "lock") == 0) {
        screen->control_type = CTRL_LOCK;
    } else if (strcmp(domain, "scene") == 0 || strcmp(domain, "script") == 0 ||
               strcmp(domain, "automation") == 0) {
        screen->control_type = CTRL_ACTIVATE;
    } else {
        screen->control_type = CTRL_NONE;
    }
}

static void extract_control_value(device_screen_t *screen) {
    if (!screen || !screen->entity) return;

    screen->control_value = 0;

    // Extract light-specific values if applicable
    if (screen->has_brightness) {
        const char *brightness_ptr = strstr(screen->entity->attributes_json, "\"brightness\":");
        if (brightness_ptr) {
            sscanf(brightness_ptr, "\"brightness\":%d", &screen->brightness_value);
        }
        // Sync with control_value for backward compatibility
        screen->control_value = screen->brightness_value;
    }

    if (screen->has_color_temp) {
        // Parse color_temp (mireds)
        const char *color_temp_ptr = strstr(screen->entity->attributes_json, "\"color_temp\":");
        if (color_temp_ptr) {
            sscanf(color_temp_ptr, "\"color_temp\":%d", &screen->color_temp_value);
        }
        // Parse min/max from attributes if available
        const char *min_ptr = strstr(screen->entity->attributes_json, "\"min_mireds\":");
        if (min_ptr) {
            sscanf(min_ptr, "\"min_mireds\":%d", &screen->color_temp_min);
        }
        const char *max_ptr = strstr(screen->entity->attributes_json, "\"max_mireds\":");
        if (max_ptr) {
            sscanf(max_ptr, "\"max_mireds\":%d", &screen->color_temp_max);
        }
        // Default color temp to middle if not set
        if (screen->color_temp_value == 0) {
            screen->color_temp_value = (screen->color_temp_min + screen->color_temp_max) / 2;
        }
    }

    switch (screen->control_type) {
        case CTRL_BRIGHTNESS: {
            // Already extracted above for lights
            break;
        }
        case CTRL_TEMPERATURE: {
            // Parse temperature from attributes (climate)
            const char *temp_ptr = strstr(screen->entity->attributes_json, "\"temperature\":");
            if (temp_ptr) {
                sscanf(temp_ptr, "\"temperature\":%d", &screen->control_value);
            } else {
                screen->control_value = 72; // Default
            }
            break;
        }
        case CTRL_POSITION: {
            // Parse position from attributes
            const char *pos_ptr = strstr(screen->entity->attributes_json, "\"current_position\":");
            if (pos_ptr) {
                sscanf(pos_ptr, "\"current_position\":%d", &screen->control_value);
            }
            break;
        }
        default:
            break;
    }
}

static int send_control_action(device_screen_t *screen) {
    if (!screen || !screen->client_ptr || !*screen->client_ptr || !screen->entity) {
        return 0;
    }

    // Extract domain
    char domain[32] = {0};
    const char *dot = strchr(screen->entity->entity_id, '.');
    if (!dot) return 0;
    size_t len = dot - screen->entity->entity_id;
    if (len >= sizeof(domain)) return 0;
    strncpy(domain, screen->entity->entity_id, len);

    const char *service = NULL;
    char params[128] = {0};

    // Calculate control indices for lights
    int brightness_idx = screen->has_brightness ? 1 : -1;
    int color_temp_idx = screen->has_color_temp ? (screen->has_brightness ? 2 : 1) : -1;

    switch (screen->control_type) {
        case CTRL_TOGGLE:
            service = "toggle";
            break;

        case CTRL_BRIGHTNESS:
            if (screen->selected_control == brightness_idx && screen->has_brightness) {
                // Apply brightness value
                service = "turn_on";
                snprintf(params, sizeof(params), "{\"brightness\":%d}", screen->brightness_value);
            } else if (screen->selected_control == color_temp_idx && screen->has_color_temp) {
                // Apply color temp value
                service = "turn_on";
                snprintf(params, sizeof(params), "{\"color_temp\":%d}", screen->color_temp_value);
            } else {
                service = "toggle";
            }
            break;

        case CTRL_TEMPERATURE:
            if (screen->selected_control == 1) {
                service = "set_temperature";
                snprintf(params, sizeof(params), "{\"temperature\":%d}", screen->control_value);
            } else {
                // Toggle HVAC mode (simplified)
                service = "toggle";
            }
            break;

        case CTRL_POSITION:
            if (screen->selected_control == 1) {
                service = "set_cover_position";
                snprintf(params, sizeof(params), "{\"position\":%d}", screen->control_value);
            } else {
                // Toggle open/close
                if (strcmp(screen->entity->state, "open") == 0) {
                    service = "close_cover";
                } else {
                    service = "open_cover";
                }
            }
            break;

        case CTRL_LOCK:
            if (strcmp(screen->entity->state, "locked") == 0) {
                service = "unlock";
            } else {
                service = "lock";
            }
            break;

        case CTRL_ACTIVATE:
            if (strcmp(domain, "automation") == 0) {
                service = "trigger";
            } else {
                service = "turn_on";
            }
            break;

        case CTRL_NONE:
        default:
            return 0;
    }

    if (!service) return 0;

    ha_response_t *response = ha_client_call_service(
        *screen->client_ptr, domain, service, screen->entity->entity_id,
        strlen(params) > 0 ? params : NULL
    );

    int success = (response && response->success);
    if (response) {
        ha_response_free(response);
    }

    // Refresh entity after action
    if (success) {
        device_screen_refresh(screen);
    }

    return success;
}

static void draw_large_icon(device_screen_t *screen, int x, int y) {
    if (!screen || !screen->entity) return;

    // Draw 4x scaled icon (64x64)
    const char *icon_name = icons_get_for_domain(screen->entity->entity_id);

    // Draw icon at 4x scale (16*4 = 64)
    // We'll draw it 4 times offset
    int base_size = 16;
    int scale = 4;
    int offset_x = x - (base_size * scale / 2);
    int offset_y = y;

    // Simple scaling: draw the icon larger
    icons_draw(screen->icons, icon_name, offset_x, offset_y, base_size * scale);
}

static void format_last_changed(const char *iso_time, char *output, size_t output_size) {
    if (!iso_time || strlen(iso_time) == 0) {
        snprintf(output, output_size, "Last changed: Unknown");
        return;
    }

    // Parse ISO 8601 timestamp: "2024-01-15T10:30:45.123456+00:00"
    struct tm tm_time = {0};
    int year, month, day, hour, minute, second;

    if (sscanf(iso_time, "%d-%d-%dT%d:%d:%d",
               &year, &month, &day, &hour, &minute, &second) >= 6) {

        tm_time.tm_year = year - 1900;
        tm_time.tm_mon = month - 1;
        tm_time.tm_mday = day;
        tm_time.tm_hour = hour;
        tm_time.tm_min = minute;
        tm_time.tm_sec = second;

        time_t changed_time = mktime(&tm_time);
        time_t now = time(NULL);
        double diff_seconds = difftime(now, changed_time);

        if (diff_seconds < 0) {
            snprintf(output, output_size, "Last changed: Just now");
        } else if (diff_seconds < 60) {
            snprintf(output, output_size, "Last changed: %d sec ago", (int)diff_seconds);
        } else if (diff_seconds < 3600) {
            int minutes = (int)(diff_seconds / 60);
            snprintf(output, output_size, "Last changed: %d min ago", minutes);
        } else if (diff_seconds < 86400) {
            int hours = (int)(diff_seconds / 3600);
            snprintf(output, output_size, "Last changed: %d hr ago", hours);
        } else {
            int days = (int)(diff_seconds / 86400);
            snprintf(output, output_size, "Last changed: %d day%s ago", days, days > 1 ? "s" : "");
        }
    } else {
        snprintf(output, output_size, "Last changed: Unknown");
    }
}

static void draw_slider_row(device_screen_t *screen, int y, const char *label,
                            int value, int min_val, int max_val,
                            int is_selected, int is_temp_kelvin) {
    SDL_Renderer *r = screen->renderer;
    TTF_Font *font_small = fonts_get(screen->fonts, FONT_SIZE_SMALL);

    if (is_selected) {
        SDL_Rect sel_bg = {100, y - 2, 440, 28};
        ui_draw_filled_rect(r, sel_bg, COLOR_SELECTED);
    }

    SDL_Color slider_color = is_selected ? COLOR_GB_DARKEST : COLOR_TEXT_PRIMARY;

    ui_draw_text(r, font_small, label, 110, y + 2, slider_color, TEXT_ALIGN_LEFT);

    // Slider bar
    int slider_x = 220;
    int slider_width = 200;
    float percent = (float)(value - min_val) / (float)(max_val - min_val);
    if (percent < 0) percent = 0;
    if (percent > 1) percent = 1;

    SDL_Rect slider_bg = {slider_x, y + 4, slider_width, 16};
    ui_draw_bordered_rect(r, slider_bg, COLOR_GB_DARK, COLOR_GB_DARKEST, 1);

    int fill_width = (int)(slider_width * percent);
    if (fill_width > 0) {
        SDL_Rect slider_fill = {slider_x + 1, y + 5, fill_width - 2, 14};
        ui_draw_filled_rect(r, slider_fill, COLOR_GB_LIGHTEST);
    }

    // Value text
    char value_str[32];
    if (is_temp_kelvin) {
        // Convert mireds to Kelvin for display
        int kelvin = 1000000 / value;
        snprintf(value_str, sizeof(value_str), "%dK", kelvin);
    } else if (max_val == 255) {
        // Brightness as percentage
        int percent_val = (value * 100) / 255;
        snprintf(value_str, sizeof(value_str), "%d%%", percent_val);
    } else {
        snprintf(value_str, sizeof(value_str), "%d", value);
    }
    ui_draw_text(r, font_small, value_str, 430, y + 2, slider_color, TEXT_ALIGN_LEFT);

    // Hints for slider
    if (is_selected) {
        ui_draw_text(r, font_small, "[<>] Adjust", 500, y + 2,
                    COLOR_TEXT_SECONDARY, TEXT_ALIGN_RIGHT);
    }
}

static void draw_control(device_screen_t *screen) {
    if (!screen) return;

    SDL_Renderer *r = screen->renderer;
    TTF_Font *font_body = fonts_get(screen->fonts, FONT_SIZE_BODY);

    int control_y = 250;
    int row_height = 30;

    // Calculate control indices
    int brightness_idx = screen->has_brightness ? 1 : -1;

    // Main action control (row 0)
    int main_selected = (screen->selected_control == 0);
    if (main_selected) {
        SDL_Rect sel_bg = {100, control_y - 2, 440, 28};
        ui_draw_filled_rect(r, sel_bg, COLOR_SELECTED);
    }

    const char *action_text = "Toggle";
    switch (screen->control_type) {
        case CTRL_TOGGLE:
            action_text = "Toggle On/Off";
            break;
        case CTRL_BRIGHTNESS:
            action_text = "Toggle Light";
            break;
        case CTRL_TEMPERATURE:
            action_text = "Toggle HVAC";
            break;
        case CTRL_POSITION:
            action_text = strcmp(screen->entity->state, "open") == 0 ? "Close Cover" : "Open Cover";
            break;
        case CTRL_LOCK:
            action_text = strcmp(screen->entity->state, "locked") == 0 ? "Unlock" : "Lock";
            break;
        case CTRL_ACTIVATE:
            action_text = "Activate";
            break;
        case CTRL_NONE:
            action_text = "(Read Only)";
            break;
    }

    SDL_Color action_color = main_selected ? COLOR_GB_DARKEST : COLOR_TEXT_PRIMARY;
    ui_draw_text(r, font_body, "[A]", 110, control_y, action_color, TEXT_ALIGN_LEFT);
    ui_draw_text(r, font_body, action_text, 150, control_y, action_color, TEXT_ALIGN_LEFT);

    // Brightness slider (for lights with brightness support)
    if (screen->has_brightness) {
        control_y += row_height;
        draw_slider_row(screen, control_y, "Brightness",
                       screen->brightness_value, 0, 255,
                       screen->selected_control == brightness_idx, 0);
    }

    // Color temperature slider (for lights with color temp support)
    if (screen->has_color_temp) {
        control_y += row_height;
        int ct_idx = screen->has_brightness ? 2 : 1;
        draw_slider_row(screen, control_y, "Color Temp",
                       screen->color_temp_value,
                       screen->color_temp_min, screen->color_temp_max,
                       screen->selected_control == ct_idx, 1);
    }

    // Legacy climate/cover slider (non-light entities)
    if (!screen->has_brightness && !screen->has_color_temp &&
        (screen->control_type == CTRL_TEMPERATURE ||
         screen->control_type == CTRL_POSITION)) {
        control_y += row_height;
        const char *label = screen->control_type == CTRL_TEMPERATURE ? "Temperature" : "Position";
        draw_slider_row(screen, control_y, label,
                       screen->control_value, screen->control_min, screen->control_max,
                       screen->selected_control == 1, 0);
    }
}
