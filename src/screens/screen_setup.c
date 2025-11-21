/**
 * screen_setup.c - Server Setup Screen Implementation
 */

#include "screen_setup.h"
#include "../utils/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

setup_screen_t* setup_screen_create(SDL_Renderer *renderer,
                                     font_manager_t *fonts,
                                     icon_manager_t *icons,
                                     app_config_t *config,
                                     ha_client_t **client_ptr) {
    if (!renderer || !fonts || !icons) {
        return NULL;
    }

    setup_screen_t *screen = calloc(1, sizeof(setup_screen_t));
    if (!screen) {
        return NULL;
    }

    screen->renderer = renderer;
    screen->fonts = fonts;
    screen->icons = icons;
    screen->config = config;
    screen->client_ptr = client_ptr;
    screen->selected_index = 0;

    // Allocate status array if we have servers
    if (config && config->server_count > 0) {
        screen->server_status = calloc(config->server_count, sizeof(connection_status_t));
        if (!screen->server_status) {
            free(screen);
            return NULL;
        }
        // Set default server as selected
        screen->selected_index = config->default_server;
    }

    strcpy(screen->status_message, "Press A to test connection");

    return screen;
}

void setup_screen_destroy(setup_screen_t *screen) {
    if (screen) {
        if (screen->server_status) {
            free(screen->server_status);
        }
        free(screen);
    }
}

int setup_screen_handle_input(setup_screen_t *screen, SDL_Event *event) {
    if (!screen || !event) return 0;

    if (event->type != SDL_KEYDOWN) return 0;

    // No servers configured
    if (!screen->config || screen->config->server_count == 0) {
        // B button exits to main anyway
        if (input_button_pressed(BTN_B)) {
            return 1;
        }
        return 0;
    }

    // Navigation
    if (input_button_pressed(BTN_DPAD_UP)) {
        screen->selected_index--;
        if (screen->selected_index < 0) {
            screen->selected_index = screen->config->server_count - 1;
        }
        return 0;
    }

    if (input_button_pressed(BTN_DPAD_DOWN)) {
        screen->selected_index++;
        if (screen->selected_index >= screen->config->server_count) {
            screen->selected_index = 0;
        }
        return 0;
    }

    // Test connection
    if (input_button_pressed(BTN_A)) {
        if (setup_screen_test_connection(screen)) {
            // Connection successful - could auto-proceed or wait for user
            strcpy(screen->status_message, "Connected! Press B to continue");
        }
        return 0;
    }

    // Back/Continue to main screen
    if (input_button_pressed(BTN_B)) {
        // Only allow proceeding if connected
        if (screen->server_status[screen->selected_index] == CONN_STATUS_CONNECTED) {
            return 1;  // Signal to switch to main screen
        } else {
            strcpy(screen->status_message, "Connect to a server first");
        }
        return 0;
    }

    // Start to continue even without connection (offline mode)
    if (input_button_pressed(BTN_START)) {
        return 1;  // Allow offline mode
    }

    return 0;
}

void setup_screen_render(setup_screen_t *screen) {
    if (!screen) return;

    SDL_Renderer *r = screen->renderer;
    TTF_Font *font_header = fonts_get(screen->fonts, FONT_SIZE_HEADER);
    TTF_Font *font_body = fonts_get(screen->fonts, FONT_SIZE_BODY);
    TTF_Font *font_small = fonts_get(screen->fonts, FONT_SIZE_SMALL);

    // Clear background
    set_render_color(r, COLOR_BACKGROUND);
    SDL_RenderClear(r);

    // Header
    ui_draw_header(r, font_header, font_small, "SERVER SETUP", 0);

    // No config message
    if (!screen->config || screen->config->server_count == 0) {
        ui_draw_text(r, font_body, "No servers configured",
                    320, 180, COLOR_TEXT_PRIMARY, TEXT_ALIGN_CENTER);
        ui_draw_text(r, font_small, "Edit servers.json to add servers",
                    320, 220, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
        ui_draw_text(r, font_small, "Press START for offline mode",
                    320, 260, COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);

        const char *hints[] = {"[START] Offline"};
        ui_draw_button_hints(r, font_body, hints, 1);
        return;
    }

    // Server list
    int list_y = 70;
    int item_height = 60;

    for (int i = 0; i < screen->config->server_count; i++) {
        server_config_t *server = &screen->config->servers[i];
        int y = list_y + (i * item_height);

        // Selection background
        if (i == screen->selected_index) {
            SDL_Rect bg = {20, y, 600, item_height - 4};
            ui_draw_bordered_rect(r, bg, COLOR_SELECTED, COLOR_BORDER, 2);
        } else {
            SDL_Rect bg = {20, y, 600, item_height - 4};
            ui_draw_bordered_rect(r, bg, COLOR_PANEL, COLOR_BORDER, 1);
        }

        // Cursor
        SDL_Color text_color = (i == screen->selected_index) ?
            COLOR_GB_DARKEST : COLOR_TEXT_PRIMARY;

        if (i == screen->selected_index) {
            ui_draw_text(r, font_body, ">", 30, y + 12, text_color, TEXT_ALIGN_LEFT);
        }

        // Server name
        ui_draw_text(r, font_body, server->name, 50, y + 8, text_color, TEXT_ALIGN_LEFT);

        // Server URL (smaller)
        char url_str[128];
        snprintf(url_str, sizeof(url_str), "%s:%d", server->url, server->port);
        ui_draw_text(r, font_small, url_str, 50, y + 28,
                    (i == screen->selected_index) ? COLOR_GB_DARK : COLOR_TEXT_SECONDARY,
                    TEXT_ALIGN_LEFT);

        // Connection status icon and text
        const char *status_text;
        SDL_Color status_color;
        const char *status_icon;

        switch (screen->server_status[i]) {
            case CONN_STATUS_TESTING:
                status_text = "TESTING...";
                status_color = COLOR_TEXT_SECONDARY;
                status_icon = "wifi_off";
                break;
            case CONN_STATUS_CONNECTED:
                status_text = "CONNECTED";
                status_color = COLOR_ACCENT;
                status_icon = "wifi_on";
                break;
            case CONN_STATUS_FAILED:
                status_text = "FAILED";
                status_color = COLOR_TEXT_SECONDARY;
                status_icon = "wifi_off";
                break;
            default:
                status_text = "UNKNOWN";
                status_color = COLOR_TEXT_SECONDARY;
                status_icon = "wifi_off";
                break;
        }

        // Draw status
        icons_draw(screen->icons, status_icon, 520, y + 12, 16);
        ui_draw_text(r, font_small, status_text, 540, y + 14, status_color, TEXT_ALIGN_LEFT);

        // Default server indicator
        if (i == screen->config->default_server) {
            icons_draw(screen->icons, "star_filled", 490, y + 12, 16);
        }
    }

    // Status message at bottom
    ui_draw_text(r, font_body, screen->status_message,
                320, 380, COLOR_TEXT_PRIMARY, TEXT_ALIGN_CENTER);

    // Button hints
    const char *hints[] = {"[A] Connect", "[B] Continue", "[START] Offline"};
    ui_draw_button_hints(r, font_body, hints, 3);
}

int setup_screen_test_connection(setup_screen_t *screen) {
    if (!screen || !screen->config || screen->config->server_count == 0) {
        return 0;
    }

    server_config_t *server = &screen->config->servers[screen->selected_index];

    // Update status to testing
    screen->server_status[screen->selected_index] = CONN_STATUS_TESTING;
    snprintf(screen->status_message, sizeof(screen->status_message),
             "Testing %s...", server->name);

    // Force a render update (caller should render after this)
    // In a real app, this would be async

    // Create temporary client for testing
    ha_client_t *test_client = ha_client_create(server->url, server->port, server->token);
    if (!test_client) {
        screen->server_status[screen->selected_index] = CONN_STATUS_FAILED;
        strcpy(screen->status_message, "Failed to create client");
        return 0;
    }

    // Test connection
    ha_response_t *response = ha_client_test_connection(test_client);
    int success = (response && response->success);

    if (success) {
        screen->server_status[screen->selected_index] = CONN_STATUS_CONNECTED;
        snprintf(screen->status_message, sizeof(screen->status_message),
                 "Connected to %s!", server->name);

        // Update the app's client pointer
        if (screen->client_ptr) {
            // Destroy old client if exists
            if (*screen->client_ptr) {
                ha_client_destroy(*screen->client_ptr);
            }
            *screen->client_ptr = test_client;
            test_client = NULL;  // Don't destroy, we're keeping it
        }
    } else {
        screen->server_status[screen->selected_index] = CONN_STATUS_FAILED;
        if (response && response->error_message) {
            snprintf(screen->status_message, sizeof(screen->status_message),
                     "Failed: %s", response->error_message);
        } else {
            strcpy(screen->status_message, "Connection failed");
        }
    }

    if (response) {
        ha_response_free(response);
    }
    if (test_client) {
        ha_client_destroy(test_client);
    }

    return success;
}

server_config_t* setup_screen_get_selected_server(setup_screen_t *screen) {
    if (!screen || !screen->config || screen->config->server_count == 0) {
        return NULL;
    }

    return &screen->config->servers[screen->selected_index];
}
