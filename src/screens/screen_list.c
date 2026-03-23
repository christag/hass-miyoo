/**
 * screen_list.c - Main List View Implementation
 *
 * Supports two view modes:
 * - VIEW_BY_DOMAIN: Tabs are entity domains (lights, sensors, etc.)
 * - VIEW_BY_ROOM: Tabs are Home Assistant areas/rooms
 */

#include "screen_list.h"
#include "../utils/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* MVP Domains - only these entity types are displayed */
static const char* MVP_DOMAINS[] = {
    "light", "sensor", "binary_sensor", "button", "humidifier",
    "scene", "switch", "select", "fan", "climate"
};

/* Display names for domains (uppercase for tab display) */
static const char* DOMAIN_DISPLAY_NAMES[][2] = {
    {"light", "LIGHTS"},
    {"sensor", "SENSORS"},
    {"binary_sensor", "BINARY"},
    {"button", "BUTTONS"},
    {"humidifier", "HUMID"},
    {"scene", "SCENES"},
    {"switch", "SWITCHES"},
    {"select", "SELECT"},
    {"fan", "FANS"},
    {"climate", "CLIMATE"},
    {NULL, NULL}
};

/* Forward declarations */
static void load_entities_for_tab(list_screen_t *screen);
static void populate_list_items(list_screen_t *screen);
static int is_mvp_domain(const char *domain);
static void build_domain_tabs(list_screen_t *screen, ha_entity_t **all_entities, int total_count);
static void build_room_tabs(list_screen_t *screen, ha_entity_t **all_entities, int total_count);
static const char* get_domain_display_name(const char *domain);
static void format_area_display_name(const char *area_id, char *output, size_t output_size);

list_screen_t* list_screen_create(SDL_Renderer *renderer,
                                   font_manager_t *fonts,
                                   icon_manager_t *icons,
                                   cache_manager_t *cache_mgr,
                                   ha_client_t **client_ptr) {
    if (!renderer || !fonts || !icons) {
        return NULL;
    }

    list_screen_t *screen = calloc(1, sizeof(list_screen_t));
    if (!screen) {
        return NULL;
    }

    screen->renderer = renderer;
    screen->fonts = fonts;
    screen->icons = icons;
    screen->cache_mgr = cache_mgr;
    screen->client_ptr = client_ptr;

    // Start with domain view
    screen->view_mode = VIEW_BY_DOMAIN;
    screen->current_tab = 0;
    screen->tab_count = 0;

    // Initialize list
    screen->list_capacity = 512;
    screen->list_items = calloc(screen->list_capacity, sizeof(list_item_t));
    if (!screen->list_items) {
        free(screen);
        return NULL;
    }

    screen->entity_list.items = screen->list_items;
    screen->entity_list.item_count = 0;
    ui_list_init(&screen->entity_list, 44);

    strcpy(screen->status_message, "");

    // Load initial entities and build tabs
    list_screen_refresh(screen);

    return screen;
}

void list_screen_destroy(list_screen_t *screen) {
    if (!screen) return;

    if (screen->list_items) {
        free(screen->list_items);
    }
    if (screen->entities) {
        free_entities(screen->entities, screen->entity_count);
    }

    free(screen);
}

int list_screen_handle_input(list_screen_t *screen, SDL_Event *event) {
    if (!screen || !event || event->type != SDL_KEYDOWN) return 0;

    // View mode toggle (Y button) - cycles Domain → Room → Favorites → Domain
    // NOTE: X button (LSHIFT) triggers MMIYOO virtual keyboard, so we use Y instead
    if (input_button_pressed(BTN_Y)) {
        if (screen->view_mode == VIEW_BY_DOMAIN) {
            screen->view_mode = VIEW_BY_ROOM;
            strcpy(screen->status_message, "View: By Room");
        } else if (screen->view_mode == VIEW_BY_ROOM) {
            screen->view_mode = VIEW_FAVORITES;
            strcpy(screen->status_message, "View: Favorites");
        } else {
            screen->view_mode = VIEW_BY_DOMAIN;
            strcpy(screen->status_message, "View: By Domain");
        }
        screen->current_tab = 0;
        screen->tabs.active_tab = 0;
        list_screen_refresh(screen);
        return 0;
    }

    // Tab switching
    if (input_button_pressed(BTN_L1)) {
        if (screen->tab_count > 0) {
            ui_tab_navigate(&screen->tabs, -1);
            screen->current_tab = screen->tabs.active_tab;
            load_entities_for_tab(screen);
            populate_list_items(screen);
            screen->entity_list.selected_index = 0;
            screen->entity_list.scroll_offset = 0;
        }
        return 0;
    }
    if (input_button_pressed(BTN_R1)) {
        if (screen->tab_count > 0) {
            ui_tab_navigate(&screen->tabs, 1);
            screen->current_tab = screen->tabs.active_tab;
            load_entities_for_tab(screen);
            populate_list_items(screen);
            screen->entity_list.selected_index = 0;
            screen->entity_list.scroll_offset = 0;
        }
        return 0;
    }

    // List navigation
    if (input_button_pressed(BTN_DPAD_UP)) {
        ui_list_navigate(&screen->entity_list, -1);
        return 0;
    }
    if (input_button_pressed(BTN_DPAD_DOWN)) {
        ui_list_navigate(&screen->entity_list, 1);
        return 0;
    }

    // Toggle/activate entity
    if (input_button_pressed(BTN_A)) {
        if (list_screen_toggle_selected(screen)) {
            strcpy(screen->status_message, "Action sent!");
        } else {
            strcpy(screen->status_message, "Action failed");
        }
        return 0;
    }

    // Go to detail screen
    if (input_button_pressed(BTN_SELECT)) {
        return 1;
    }

    // Refresh
    if (input_button_pressed(BTN_START)) {
        strcpy(screen->status_message, "Refreshing...");
        if (screen->cache_mgr) {
            cache_manager_sync(screen->cache_mgr);
        }
        list_screen_refresh(screen);
        strcpy(screen->status_message, "Refreshed");
        return 0;
    }

    // NOTE: X button (BTN_X / LSHIFT) is NOT used - it triggers the
    // MMIYOO driver's virtual keyboard overlay which we cannot disable.
    // Favorites toggle moved to SELECT + A in detail screen instead.

    // Back to setup
    if (input_button_pressed(BTN_B)) {
        return -1;
    }

    return 0;
}

/**
 * Draw the rotating wheel tab selector.
 * Shows 3 tabs: prev (small/dim), current (large/bright), next (small/dim)
 * with arrows on each side indicating more tabs available.
 */
static void draw_wheel_tabs(list_screen_t *screen, SDL_Renderer *r,
                            TTF_Font *font_big, TTF_Font *font_small, int y) {
    if (screen->tab_count == 0) return;

    int center_x = 320;
    int active = screen->tabs.active_tab;

    // Left arrow if not at first tab
    if (active > 0) {
        ui_draw_text(r, font_small, "<< L1", 30, y + 8, COLOR_TAB_INACTIVE, TEXT_ALIGN_LEFT);
    }

    // Right arrow if not at last tab
    if (active < screen->tab_count - 1) {
        ui_draw_text(r, font_small, "R1 >>", 610, y + 8, COLOR_TAB_INACTIVE, TEXT_ALIGN_RIGHT);
    }

    // Previous tab (smaller, dimmer, left of center)
    if (active > 0) {
        const char *prev_name = screen->tabs.tabs[active - 1];
        if (prev_name) {
            ui_draw_text(r, font_small, prev_name, center_x - 160, y + 8,
                        COLOR_TAB_INACTIVE, TEXT_ALIGN_CENTER);
        }
    }

    // Current tab (LARGE, bright, centered with underline)
    const char *cur_name = screen->tabs.tabs[active];
    if (cur_name) {
        ui_draw_text(r, font_big, cur_name, center_x, y + 2,
                    COLOR_WHITE, TEXT_ALIGN_CENTER);

        // Golden underline
        int tw = 0, th = 0;
        TTF_SizeText(font_big, cur_name, &tw, &th);
        SDL_Rect underline = {center_x - tw/2, y + th + 6, tw, 3};
        ui_draw_filled_rect(r, underline, COLOR_CURSOR);
    }

    // Next tab (smaller, dimmer, right of center)
    if (active < screen->tab_count - 1) {
        const char *next_name = screen->tabs.tabs[active + 1];
        if (next_name) {
            ui_draw_text(r, font_small, next_name, center_x + 160, y + 8,
                        COLOR_TAB_INACTIVE, TEXT_ALIGN_CENTER);
        }
    }
}

void list_screen_render(list_screen_t *screen) {
    if (!screen) return;

    SDL_Renderer *r = screen->renderer;
    TTF_Font *font_header = fonts_get(screen->fonts, FONT_SIZE_HEADER);
    TTF_Font *font_body = fonts_get(screen->fonts, FONT_SIZE_BODY);
    TTF_Font *font_small = fonts_get(screen->fonts, FONT_SIZE_SMALL);

    // === HEADER (simple centered title) ===
    const char *mode_text;
    if (screen->view_mode == VIEW_BY_DOMAIN) {
        mode_text = "BY DOMAIN";
    } else if (screen->view_mode == VIEW_BY_ROOM) {
        mode_text = "BY ROOM";
    } else {
        mode_text = "FAVORITES";
    }
    ui_draw_text(r, font_small, mode_text, 320, 8, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);

    // === ROTATING WHEEL TAB SELECTOR ===
    int tab_y = 24;
    if (screen->tab_count > 0 && screen->view_mode != VIEW_FAVORITES) {
        draw_wheel_tabs(screen, r, font_header, font_small, tab_y);
        // Separator line
        SDL_Rect sep = {40, tab_y + 50, 560, 1};
        ui_draw_filled_rect(r, sep, COLOR_BORDER_INNER);
    }

    // === ENTITY LIST ===
    int list_y = 80;
    int list_bottom = 440;
    int list_height = list_bottom - list_y;
    int item_height = 56;  // Big items for readability

    if (screen->entity_list.item_count > 0) {
        int visible = list_height / item_height;
        if (visible < 1) visible = 1;

        // Adjust scroll to keep selection visible
        if (screen->entity_list.selected_index < screen->entity_list.scroll_offset) {
            screen->entity_list.scroll_offset = screen->entity_list.selected_index;
        }
        if (screen->entity_list.selected_index >= screen->entity_list.scroll_offset + visible) {
            screen->entity_list.scroll_offset = screen->entity_list.selected_index - visible + 1;
        }

        for (int i = 0; i < visible && (i + screen->entity_list.scroll_offset) < screen->entity_list.item_count; i++) {
            int idx = i + screen->entity_list.scroll_offset;
            list_item_t *item = &screen->list_items[idx];
            int y = list_y + (i * item_height);
            int is_selected = (idx == screen->entity_list.selected_index);

            if (is_selected) {
                // Selected: highlight with golden border
                SDL_Rect bg = {16, y + 2, 608, item_height - 4};
                ui_draw_filled_rect(r, bg, COLOR_SELECTED);
                set_render_color(r, COLOR_CURSOR);
                SDL_Rect b1 = {16, y + 2, 608, item_height - 4};
                SDL_RenderDrawRect(r, &b1);
                SDL_Rect b2 = {17, y + 3, 606, item_height - 6};
                SDL_RenderDrawRect(r, &b2);
                // Golden cursor arrow
                ui_draw_text(r, font_header, ">", 24, y + 16, COLOR_CURSOR, TEXT_ALIGN_LEFT);
            } else if (i > 0) {
                // Separator between unselected items
                SDL_Rect line = {40, y, 560, 1};
                ui_draw_filled_rect(r, line, COLOR_BORDER_INNER);
            }

            // Entity name (larger font for selected)
            TTF_Font *name_font = is_selected ? font_header : font_body;
            int name_y = is_selected ? y + 16 : y + 18;
            ui_draw_text_truncated(r, name_font, item->text, 52, name_y, 440, COLOR_WHITE);

            // State - color coded, right side
            SDL_Color state_color = COLOR_TEXT_SECONDARY;
            if (strcmp(item->subtext, "on") == 0) {
                state_color = COLOR_STATE_ON;
            } else if (strcmp(item->subtext, "off") == 0) {
                state_color = COLOR_STATE_OFF;
            } else if (strcmp(item->subtext, "unavailable") == 0) {
                state_color = COLOR_UNAVAIL;
            }
            ui_draw_text(r, font_body, item->subtext, 610, y + 20, state_color, TEXT_ALIGN_RIGHT);
        }

        // Scrollbar
        if (screen->entity_list.item_count > visible) {
            ui_draw_scrollbar(r, 630, list_y, list_height,
                             screen->entity_list.item_count, visible,
                             screen->entity_list.scroll_offset);
        }
    } else {
        ui_draw_text(r, font_body, "No entities",
                    320, 240, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
    }

    // === BOTTOM HINTS ===
    const char *hints[] = {"A:Act", "Y:View", "B:Back", "St:Sync"};
    ui_draw_button_hints(r, font_small, hints, 4);
}

void list_screen_refresh(list_screen_t *screen) {
    if (!screen || !screen->cache_mgr) return;

    // Free old entities
    if (screen->entities) {
        free_entities(screen->entities, screen->entity_count);
        screen->entities = NULL;
        screen->entity_count = 0;
    }

    // Favorites mode - load directly from favorites
    if (screen->view_mode == VIEW_FAVORITES) {
        screen->tab_count = 0;  // No tabs in favorites mode
        screen->tabs.tab_count = 0;
        memset(screen->tabs.tabs, 0, sizeof(screen->tabs.tabs));
        screen->entities = cache_manager_get_favorites(screen->cache_mgr, &screen->entity_count);
        populate_list_items(screen);
        screen->entity_list.selected_index = 0;
        screen->entity_list.scroll_offset = 0;
        return;
    }

    // Get all entities and build tabs based on view mode
    int total_count = 0;
    ha_entity_t **all_entities = cache_manager_get_entities(screen->cache_mgr, &total_count);

    if (!all_entities || total_count == 0) {
        screen->tab_count = 0;
        screen->entity_list.item_count = 0;
        return;
    }

    // Build tabs based on current view mode
    if (screen->view_mode == VIEW_BY_DOMAIN) {
        build_domain_tabs(screen, all_entities, total_count);
    } else {
        build_room_tabs(screen, all_entities, total_count);
    }

    // Free the full entity list (we'll load filtered ones below)
    free_entities(all_entities, total_count);

    // Ensure current tab is valid
    if (screen->current_tab >= screen->tab_count) {
        screen->current_tab = 0;
        screen->tabs.active_tab = 0;
    }

    // Load entities for current tab
    load_entities_for_tab(screen);
    populate_list_items(screen);

    // Reset scroll position
    screen->entity_list.selected_index = 0;
    screen->entity_list.scroll_offset = 0;
}

ha_entity_t* list_screen_get_selected_entity(list_screen_t *screen) {
    if (!screen || screen->entity_list.item_count == 0) {
        return NULL;
    }

    int idx = screen->entity_list.selected_index;
    if (idx >= 0 && idx < screen->entity_list.item_count) {
        return (ha_entity_t*)screen->list_items[idx].user_data;
    }

    return NULL;
}

int list_screen_toggle_selected(list_screen_t *screen) {
    if (!screen || !screen->client_ptr || !*screen->client_ptr) {
        return 0;
    }

    ha_entity_t *entity = list_screen_get_selected_entity(screen);
    if (!entity) return 0;

    // Determine domain and service
    char domain[32];
    const char *dot = strchr(entity->entity_id, '.');
    if (!dot) return 0;

    size_t len = dot - entity->entity_id;
    if (len >= sizeof(domain)) return 0;
    strncpy(domain, entity->entity_id, len);
    domain[len] = '\0';

    // Choose service based on domain and state
    const char *service = NULL;

    if (strcmp(domain, "light") == 0 || strcmp(domain, "switch") == 0 ||
        strcmp(domain, "fan") == 0) {
        service = "toggle";
    } else if (strcmp(domain, "button") == 0) {
        service = "press";
    } else if (strcmp(domain, "scene") == 0) {
        service = "turn_on";
    } else if (strcmp(domain, "humidifier") == 0) {
        service = "toggle";
    } else if (strcmp(domain, "climate") == 0) {
        // Can't simple toggle climate
        return 0;
    } else if (strcmp(domain, "select") == 0) {
        // Can't toggle select without knowing options
        return 0;
    } else {
        return 0;
    }

    // Call service
    ha_response_t *response = ha_client_call_service(
        *screen->client_ptr, domain, service, entity->entity_id, NULL
    );

    int success = (response && response->success);
    if (response) {
        ha_response_free(response);
    }

    // Refresh entity after action
    if (success && screen->cache_mgr) {
        cache_manager_refresh_entity(screen->cache_mgr, entity->entity_id);
        load_entities_for_tab(screen);
        populate_list_items(screen);
    }

    return success;
}

/* ============================================
 * Static Helper Functions
 * ============================================ */

static int is_mvp_domain(const char *domain) {
    if (!domain) return 0;
    for (int i = 0; i < MVP_DOMAIN_COUNT; i++) {
        if (strcmp(domain, MVP_DOMAINS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static const char* get_domain_display_name(const char *domain) {
    for (int i = 0; DOMAIN_DISPLAY_NAMES[i][0] != NULL; i++) {
        if (strcmp(domain, DOMAIN_DISPLAY_NAMES[i][0]) == 0) {
            return DOMAIN_DISPLAY_NAMES[i][1];
        }
    }
    return domain;
}

static void format_area_display_name(const char *area_id, char *output, size_t output_size) {
    if (!area_id || strlen(area_id) == 0) {
        strncpy(output, "UNASSIGNED", output_size - 1);
        return;
    }

    // Convert underscores to spaces and uppercase
    size_t i = 0;
    while (area_id[i] && i < output_size - 1) {
        if (area_id[i] == '_') {
            output[i] = ' ';
        } else {
            output[i] = toupper((unsigned char)area_id[i]);
        }
        i++;
    }
    output[i] = '\0';

    // Truncate if too long for tab display
    if (strlen(output) > 10) {
        output[10] = '\0';
    }
}

static void build_domain_tabs(list_screen_t *screen, ha_entity_t **all_entities, int total_count) {
    // Clear tabs array first
    memset(screen->tabs.tabs, 0, sizeof(screen->tabs.tabs));

    // Find unique MVP domains that have entities
    int found_domains[MVP_DOMAIN_COUNT] = {0};

    for (int i = 0; i < total_count; i++) {
        for (int d = 0; d < MVP_DOMAIN_COUNT; d++) {
            if (strcmp(all_entities[i]->domain, MVP_DOMAINS[d]) == 0) {
                found_domains[d] = 1;
                break;
            }
        }
    }

    // Build tabs for domains that have entities
    screen->tab_count = 0;
    for (int d = 0; d < MVP_DOMAIN_COUNT && screen->tab_count < MAX_TABS; d++) {
        if (found_domains[d]) {
            strncpy(screen->tab_values[screen->tab_count], MVP_DOMAINS[d], 63);
            const char *display = get_domain_display_name(MVP_DOMAINS[d]);
            strncpy(screen->tab_names[screen->tab_count], display, 31);
            screen->tabs.tabs[screen->tab_count] = screen->tab_names[screen->tab_count];
            screen->tab_count++;
        }
    }

    screen->tabs.tab_count = screen->tab_count;
}

static void build_room_tabs(list_screen_t *screen, ha_entity_t **all_entities, int total_count) {
    // Clear tabs array first
    memset(screen->tabs.tabs, 0, sizeof(screen->tabs.tabs));

    // Find unique area_ids from MVP-domain entities
    char unique_areas[MAX_TABS][64];
    int area_count = 0;
    int has_unassigned = 0;

    for (int i = 0; i < total_count; i++) {
        // Only consider MVP domains
        if (!is_mvp_domain(all_entities[i]->domain)) {
            continue;
        }

        const char *area = all_entities[i]->area_id;

        // Handle unassigned entities
        if (!area || strlen(area) == 0) {
            has_unassigned = 1;
            continue;
        }

        // Check if already in list
        int found = 0;
        for (int a = 0; a < area_count; a++) {
            if (strcmp(unique_areas[a], area) == 0) {
                found = 1;
                break;
            }
        }

        if (!found && area_count < MAX_TABS - 1) {
            strncpy(unique_areas[area_count], area, 63);
            area_count++;
        }
    }

    // Build tabs - unassigned first if present
    screen->tab_count = 0;

    if (has_unassigned && screen->tab_count < MAX_TABS) {
        strcpy(screen->tab_values[screen->tab_count], "");
        strcpy(screen->tab_names[screen->tab_count], "UNASSIGNED");
        screen->tabs.tabs[screen->tab_count] = screen->tab_names[screen->tab_count];
        screen->tab_count++;
    }

    // Then add sorted areas (we'll just add in order found for simplicity)
    for (int a = 0; a < area_count && screen->tab_count < MAX_TABS; a++) {
        strncpy(screen->tab_values[screen->tab_count], unique_areas[a], 63);
        format_area_display_name(unique_areas[a], screen->tab_names[screen->tab_count], 32);
        screen->tabs.tabs[screen->tab_count] = screen->tab_names[screen->tab_count];
        screen->tab_count++;
    }

    screen->tabs.tab_count = screen->tab_count;
}

static void load_entities_for_tab(list_screen_t *screen) {
    if (!screen || !screen->cache_mgr || screen->tab_count == 0) {
        screen->entity_count = 0;
        return;
    }

    // Free old entities
    if (screen->entities) {
        free_entities(screen->entities, screen->entity_count);
        screen->entities = NULL;
        screen->entity_count = 0;
    }

    // Get all entities
    int total_count = 0;
    ha_entity_t **all_entities = cache_manager_get_entities(screen->cache_mgr, &total_count);

    if (!all_entities || total_count == 0) {
        return;
    }

    // Allocate for filtered entities
    screen->entities = calloc(total_count, sizeof(ha_entity_t*));
    screen->entity_count = 0;

    // Safety check: ensure current_tab is valid
    if (screen->current_tab < 0 || screen->current_tab >= screen->tab_count) {
        screen->current_tab = 0;
    }

    const char *filter_value = (screen->tab_count > 0) ? screen->tab_values[screen->current_tab] : NULL;

    for (int i = 0; i < total_count; i++) {
        ha_entity_t *e = all_entities[i];
        int match = 0;

        // Only consider MVP domains
        if (!is_mvp_domain(e->domain)) {
            continue;
        }

        if (screen->view_mode == VIEW_BY_DOMAIN) {
            // Match by domain
            match = (filter_value && strcmp(e->domain, filter_value) == 0);
        } else {
            // Match by area_id
            if (!filter_value || strlen(filter_value) == 0) {
                // Unassigned tab - match entities with empty area_id
                match = (strlen(e->area_id) == 0);
            } else {
                match = (strcmp(e->area_id, filter_value) == 0);
            }
        }

        if (match) {
            screen->entities[screen->entity_count++] = e;
            all_entities[i] = NULL;  // Don't free this one
        }
    }

    // Free entities we didn't keep
    for (int i = 0; i < total_count; i++) {
        if (all_entities[i]) {
            free_entity(all_entities[i]);
        }
    }
    free(all_entities);
}

static void populate_list_items(list_screen_t *screen) {
    if (!screen) return;

    int count = screen->entity_count;
    if (count > screen->list_capacity) {
        count = screen->list_capacity;
    }

    for (int i = 0; i < count; i++) {
        ha_entity_t *entity = screen->entities[i];

        // Use friendly name if available, otherwise entity_id
        if (strlen(entity->friendly_name) > 0) {
            strncpy(screen->list_items[i].text, entity->friendly_name, 127);
        } else {
            strncpy(screen->list_items[i].text, entity->entity_id, 127);
        }

        // State as subtext
        strncpy(screen->list_items[i].subtext, entity->state, 63);

        // Icon based on domain
        const char *icon = icons_get_for_domain(entity->entity_id);
        strncpy(screen->list_items[i].icon_name, icon, 31);

        // Store entity pointer for later use
        screen->list_items[i].user_data = entity;
    }

    screen->entity_list.item_count = count;
    screen->entity_list.items = screen->list_items;
}
