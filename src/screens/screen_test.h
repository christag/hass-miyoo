/**
 * screen_test.h - Test Screen for UI Components
 *
 * Displays all UI components for visual verification.
 * Phase 4: UI Design System
 */

#ifndef SCREEN_TEST_H
#define SCREEN_TEST_H

#include <SDL.h>
#include "../ui/fonts.h"
#include "../ui/icons.h"
#include "../ui/components.h"

/**
 * Test screen state
 */
typedef struct {
    // Dependencies
    SDL_Renderer *renderer;
    font_manager_t *fonts;
    icon_manager_t *icons;

    // Demo state
    int demo_mode;         // 0=components, 1=list, 2=dialog
    int is_connected;
    int toggle_state;
    float slider_value;

    // List demo
    list_view_t demo_list;
    list_item_t demo_items[10];

    // Tab demo
    tab_bar_t demo_tabs;

    // Dialog demo
    dialog_t demo_dialog;
} test_screen_t;

/**
 * Create test screen
 *
 * @param renderer SDL renderer
 * @param fonts Font manager
 * @param icons Icon manager
 * @return test_screen_t pointer or NULL on failure
 */
test_screen_t* test_screen_create(SDL_Renderer *renderer,
                                   font_manager_t *fonts,
                                   icon_manager_t *icons);

/**
 * Destroy test screen
 *
 * @param screen Test screen to destroy
 */
void test_screen_destroy(test_screen_t *screen);

/**
 * Handle input for test screen
 *
 * @param screen Test screen
 * @param event SDL event
 * @return 1 if event was handled, 0 otherwise
 */
int test_screen_handle_input(test_screen_t *screen, SDL_Event *event);

/**
 * Render test screen
 *
 * @param screen Test screen
 */
void test_screen_render(test_screen_t *screen);

#endif // SCREEN_TEST_H
