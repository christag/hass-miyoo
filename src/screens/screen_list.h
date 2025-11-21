/**
 * screen_list.h - Main List View with Tabbed Navigation
 *
 * Displays entities organized by tabs, supporting two view modes:
 * - VIEW_BY_DOMAIN: Groups by entity type (lights, sensors, etc.)
 * - VIEW_BY_ROOM: Groups by Home Assistant area/room
 *
 * Phase 6: List View (Refactored)
 */

#ifndef SCREEN_LIST_H
#define SCREEN_LIST_H

#include <SDL.h>
#include "../ui/fonts.h"
#include "../ui/icons.h"
#include "../ui/components.h"
#include "../cache_manager.h"
#include "../ha_client.h"

/**
 * Maximum number of dynamic tabs
 */
#define MAX_TABS 16

/**
 * MVP Domains - only these entity types are displayed
 */
#define MVP_DOMAIN_COUNT 10

/**
 * View mode - how entities are grouped into tabs
 */
typedef enum {
    VIEW_BY_DOMAIN = 0,  // Group by entity domain (light, sensor, etc.)
    VIEW_BY_ROOM,        // Group by Home Assistant area/room
    VIEW_FAVORITES       // Show favorited entities only
} view_mode_t;

/**
 * List screen state
 */
typedef struct {
    // Dependencies
    SDL_Renderer *renderer;
    font_manager_t *fonts;
    icon_manager_t *icons;
    cache_manager_t *cache_mgr;
    ha_client_t **client_ptr;

    // View mode
    view_mode_t view_mode;

    // Tab state (dynamic)
    tab_bar_t tabs;
    int current_tab;
    char tab_names[MAX_TABS][32];      // Storage for dynamic tab names
    char tab_values[MAX_TABS][64];     // Domain or area_id for each tab
    int tab_count;

    // List state per tab
    list_view_t entity_list;
    list_item_t *list_items;
    int list_capacity;

    // Cached entity data
    ha_entity_t **entities;
    int entity_count;

    // Status
    char status_message[128];
    int is_loading;
} list_screen_t;

/**
 * Create list screen
 *
 * @param renderer SDL renderer
 * @param fonts Font manager
 * @param icons Icon manager
 * @param cache_mgr Cache manager for entity data
 * @param client_ptr Pointer to HA client for actions
 * @return list_screen_t pointer or NULL on failure
 */
list_screen_t* list_screen_create(SDL_Renderer *renderer,
                                   font_manager_t *fonts,
                                   icon_manager_t *icons,
                                   cache_manager_t *cache_mgr,
                                   ha_client_t **client_ptr);

/**
 * Destroy list screen
 *
 * @param screen List screen to destroy
 */
void list_screen_destroy(list_screen_t *screen);

/**
 * Handle input for list screen
 *
 * @param screen List screen
 * @param event SDL event
 * @return 0 to stay, 1 to go to detail, -1 to go back to setup
 */
int list_screen_handle_input(list_screen_t *screen, SDL_Event *event);

/**
 * Render list screen
 *
 * @param screen List screen
 */
void list_screen_render(list_screen_t *screen);

/**
 * Refresh entity list for current tab
 *
 * @param screen List screen
 */
void list_screen_refresh(list_screen_t *screen);

/**
 * Get currently selected entity
 *
 * @param screen List screen
 * @return ha_entity_t pointer or NULL
 */
ha_entity_t* list_screen_get_selected_entity(list_screen_t *screen);

/**
 * Toggle/activate the selected entity
 *
 * @param screen List screen
 * @return 1 on success, 0 on failure
 */
int list_screen_toggle_selected(list_screen_t *screen);

#endif // SCREEN_LIST_H
