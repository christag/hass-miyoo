/**
 * screen_test.c - Test Screen Implementation
 */

#include "screen_test.h"
#include "../utils/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

test_screen_t* test_screen_create(SDL_Renderer *renderer,
                                   font_manager_t *fonts,
                                   icon_manager_t *icons) {
    if (!renderer || !fonts || !icons) {
        return NULL;
    }

    test_screen_t *screen = calloc(1, sizeof(test_screen_t));
    if (!screen) {
        return NULL;
    }

    screen->renderer = renderer;
    screen->fonts = fonts;
    screen->icons = icons;

    // Initialize demo state
    screen->demo_mode = 0;
    screen->is_connected = 1;
    screen->toggle_state = 1;
    screen->slider_value = 75.0f;

    // Initialize demo list
    const char *items[] = {
        "Living Room Light",
        "Kitchen Switch",
        "Bedroom Climate",
        "Front Door Sensor",
        "Garage Cover",
        "Office Fan",
        "Main Automation",
        "Morning Script",
        "Movie Scene",
        "Goodnight Scene"
    };
    const char *states[] = {
        "on", "off", "72F", "closed", "open",
        "off", "enabled", "idle", "active", "active"
    };
    const char *icon_names[] = {
        "light_bulb", "switch_toggle", "climate_thermo", "sensor_generic",
        "generic", "generic", "automation_robot", "script_code",
        "scene_stars", "scene_stars"
    };

    for (int i = 0; i < 10; i++) {
        strncpy(screen->demo_items[i].text, items[i], 127);
        strncpy(screen->demo_items[i].subtext, states[i], 63);
        strncpy(screen->demo_items[i].icon_name, icon_names[i], 31);
    }

    screen->demo_list.items = screen->demo_items;
    screen->demo_list.item_count = 10;
    ui_list_init(&screen->demo_list, 36);

    // Initialize demo tabs
    screen->demo_tabs.tabs[0] = "DEVICES";
    screen->demo_tabs.tabs[1] = "ENTITIES";
    screen->demo_tabs.tabs[2] = "AUTO";
    screen->demo_tabs.tabs[3] = "SCRIPTS";
    screen->demo_tabs.tabs[4] = "SCENES";
    screen->demo_tabs.tab_count = 5;
    screen->demo_tabs.active_tab = 0;

    // Initialize demo dialog
    strcpy(screen->demo_dialog.title, "CONFIRM EXIT");
    strcpy(screen->demo_dialog.message, "Are you sure you want to exit?");
    screen->demo_dialog.selected_option = 0;
    screen->demo_dialog.visible = 0;

    printf("Test screen created\n");
    return screen;
}

void test_screen_destroy(test_screen_t *screen) {
    if (screen) {
        free(screen);
    }
}

int test_screen_handle_input(test_screen_t *screen, SDL_Event *event) {
    if (!screen || !event) return 0;

    if (event->type == SDL_KEYDOWN) {
        // Handle dialog mode first
        if (screen->demo_dialog.visible) {
            if (input_button_pressed(BTN_DPAD_LEFT) || input_button_pressed(BTN_DPAD_RIGHT)) {
                ui_dialog_navigate(&screen->demo_dialog,
                    input_button_pressed(BTN_DPAD_RIGHT) ? 1 : -1);
                return 1;
            }
            if (input_button_pressed(BTN_A)) {
                screen->demo_dialog.visible = 0;
                printf("Dialog result: %s\n",
                    screen->demo_dialog.selected_option == 0 ? "YES" : "NO");
                return 1;
            }
            if (input_button_pressed(BTN_B)) {
                screen->demo_dialog.visible = 0;
                return 1;
            }
            return 1; // Consume all input in dialog mode
        }

        // Navigation
        if (input_button_pressed(BTN_DPAD_UP)) {
            ui_list_navigate(&screen->demo_list, -1);
            return 1;
        }
        if (input_button_pressed(BTN_DPAD_DOWN)) {
            ui_list_navigate(&screen->demo_list, 1);
            return 1;
        }

        // Tab switching
        if (input_button_pressed(BTN_L1)) {
            ui_tab_navigate(&screen->demo_tabs, -1);
            return 1;
        }
        if (input_button_pressed(BTN_R1)) {
            ui_tab_navigate(&screen->demo_tabs, 1);
            return 1;
        }

        // Toggle action
        if (input_button_pressed(BTN_A)) {
            screen->toggle_state = !screen->toggle_state;
            return 1;
        }

        // Slider adjustment
        if (input_button_pressed(BTN_DPAD_LEFT)) {
            screen->slider_value -= 10;
            if (screen->slider_value < 0) screen->slider_value = 0;
            return 1;
        }
        if (input_button_pressed(BTN_DPAD_RIGHT)) {
            screen->slider_value += 10;
            if (screen->slider_value > 100) screen->slider_value = 100;
            return 1;
        }

        // Toggle connection status with X
        if (input_button_pressed(BTN_X)) {
            screen->is_connected = !screen->is_connected;
            return 1;
        }

        // Show dialog with Y
        if (input_button_pressed(BTN_Y)) {
            screen->demo_dialog.visible = 1;
            screen->demo_dialog.selected_option = 0;
            return 1;
        }

        // Switch demo mode with Select
        if (input_button_pressed(BTN_SELECT)) {
            screen->demo_mode = (screen->demo_mode + 1) % 3;
            return 1;
        }
    }

    return 0;
}

void test_screen_render(test_screen_t *screen) {
    if (!screen) return;

    SDL_Renderer *r = screen->renderer;
    TTF_Font *font_header = fonts_get(screen->fonts, FONT_SIZE_HEADER);
    TTF_Font *font_body = fonts_get(screen->fonts, FONT_SIZE_BODY);
    TTF_Font *font_small = fonts_get(screen->fonts, FONT_SIZE_SMALL);

    // Clear background
    set_render_color(r, COLOR_BACKGROUND);
    SDL_RenderClear(r);

    // Draw header
    ui_draw_header(r, font_header, font_small, "UI TEST SCREEN", screen->is_connected);

    // Draw tab bar
    ui_draw_tab_bar(r, &screen->demo_tabs, font_small, 60, 60, 520);

    switch (screen->demo_mode) {
        case 0: // Components demo
            {
                // Section: Toggles
                ui_draw_text(r, font_body, "TOGGLE:", 20, 105, COLOR_TEXT_PRIMARY, TEXT_ALIGN_LEFT);
                ui_draw_toggle(r, 120, 100, screen->toggle_state);
                ui_draw_text(r, font_small, screen->toggle_state ? "ON" : "OFF",
                            170, 105, COLOR_TEXT_SECONDARY, TEXT_ALIGN_LEFT);

                // Section: Slider
                ui_draw_text(r, font_body, "BRIGHTNESS:", 20, 140, COLOR_TEXT_PRIMARY, TEXT_ALIGN_LEFT);
                ui_draw_slider(r, 140, 140, 200, screen->slider_value, 0, 100);
                char pct[16];
                snprintf(pct, sizeof(pct), "%d%%", (int)screen->slider_value);
                ui_draw_text(r, font_small, pct, 350, 142, COLOR_TEXT_PRIMARY, TEXT_ALIGN_LEFT);

                // Section: Icons
                ui_draw_text(r, font_body, "ICONS:", 20, 175, COLOR_TEXT_PRIMARY, TEXT_ALIGN_LEFT);
                int icon_x = 100;
                icons_draw(screen->icons, "light_bulb", icon_x, 170, 16); icon_x += 24;
                icons_draw(screen->icons, "switch_toggle", icon_x, 170, 16); icon_x += 24;
                icons_draw(screen->icons, "climate_thermo", icon_x, 170, 16); icon_x += 24;
                icons_draw(screen->icons, "sensor_generic", icon_x, 170, 16); icon_x += 24;
                icons_draw(screen->icons, "automation_robot", icon_x, 170, 16); icon_x += 24;
                icons_draw(screen->icons, "script_code", icon_x, 170, 16); icon_x += 24;
                icons_draw(screen->icons, "scene_stars", icon_x, 170, 16); icon_x += 24;
                icons_draw(screen->icons, "star_filled", icon_x, 170, 16); icon_x += 24;
                icons_draw(screen->icons, "star_empty", icon_x, 170, 16); icon_x += 24;
                icons_draw(screen->icons, "wifi_on", icon_x, 170, 16); icon_x += 24;
                icons_draw(screen->icons, "wifi_off", icon_x, 170, 16); icon_x += 24;

                // Section: Buttons
                ui_draw_text(r, font_body, "BUTTONS:", 20, 210, COLOR_TEXT_PRIMARY, TEXT_ALIGN_LEFT);
                ui_button_t btn1 = {"PRIMARY", 120, 205, 100, 28, 1, 0};
                ui_button_t btn2 = {"NORMAL", 230, 205, 100, 28, 0, 0};
                ui_button_t btn3 = {"SELECTED", 340, 205, 100, 28, 0, 1};
                ui_draw_button(r, &btn1, font_small);
                ui_draw_button(r, &btn2, font_small);
                ui_draw_button(r, &btn3, font_small);

                // Section: Bordered rect / Panel
                ui_draw_text(r, font_body, "PANEL:", 20, 250, COLOR_TEXT_PRIMARY, TEXT_ALIGN_LEFT);
                SDL_Rect panel = {120, 245, 400, 60};
                ui_draw_bordered_rect(r, panel, COLOR_PANEL, COLOR_BORDER, 2);
                ui_draw_text(r, font_body, "This is a bordered panel",
                            320, 265, COLOR_TEXT_PRIMARY, TEXT_ALIGN_CENTER);

                // Section: List preview
                ui_draw_text(r, font_body, "LIST (3 items):", 20, 320, COLOR_TEXT_PRIMARY, TEXT_ALIGN_LEFT);
                ui_draw_list(r, &screen->demo_list, font_body, 20, 340, 600, 108);
            }
            break;

        case 1: // Full list demo
            ui_draw_list(r, &screen->demo_list, font_body, 20, 100, 600, 340);
            break;

        case 2: // Large icons demo
            {
                ui_draw_text(r, font_body, "LARGE ICONS (32x32):", 20, 105,
                            COLOR_TEXT_PRIMARY, TEXT_ALIGN_LEFT);

                int x = 50, y = 140;
                const char *icon_list[] = {
                    "light_bulb", "switch_toggle", "climate_thermo",
                    "sensor_generic", "automation_robot", "script_code",
                    "scene_stars", "star_filled", "star_empty",
                    "wifi_on", "wifi_off", "generic"
                };
                const char *labels[] = {
                    "Light", "Switch", "Climate",
                    "Sensor", "Auto", "Script",
                    "Scene", "Fav On", "Fav Off",
                    "WiFi On", "WiFi Off", "Generic"
                };

                for (int i = 0; i < 12; i++) {
                    icons_draw(screen->icons, icon_list[i], x, y, 32);
                    ui_draw_text(r, font_small, labels[i], x + 16, y + 38,
                                COLOR_TEXT_SECONDARY, TEXT_ALIGN_CENTER);
                    x += 70;
                    if (x > 500) {
                        x = 50;
                        y += 80;
                    }
                }
            }
            break;
    }

    // Button hints
    const char *hints[] = {"[A] Toggle", "[X] Conn", "[Y] Dialog", "[SEL] Mode"};
    ui_draw_button_hints(r, font_body, hints, 4);

    // Render dialog on top if visible
    ui_draw_dialog(r, &screen->demo_dialog, font_header, font_body);
}
