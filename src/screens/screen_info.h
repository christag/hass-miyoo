/**
 * screen_info.h - Info View for Automations/Scripts/Scenes
 *
 * Read-only information display for automation, script, and scene entities.
 * Shows metadata like name, status, last_triggered, and description.
 *
 * Phase 9: Info View
 */

#ifndef SCREEN_INFO_H
#define SCREEN_INFO_H

#include <SDL.h>
#include "../ui/fonts.h"
#include "../ui/icons.h"
#include "../ui/components.h"
#include "../cache_manager.h"
#include "../ha_client.h"

/**
 * Info screen state
 */
typedef struct {
    // Dependencies
    SDL_Renderer *renderer;
    font_manager_t *fonts;
    icon_manager_t *icons;
    cache_manager_t *cache_mgr;
    ha_client_t **client_ptr;

    // Current entity
    ha_entity_t *entity;
    char entity_id[128];

    // Parsed info
    char description[512];
    char last_triggered[64];
    char mode[32];
    int is_enabled;
    int is_favorite;

    // Status
    char status_message[128];
} info_screen_t;

/**
 * Create info screen
 */
info_screen_t* info_screen_create(SDL_Renderer *renderer,
                                   font_manager_t *fonts,
                                   icon_manager_t *icons,
                                   cache_manager_t *cache_mgr,
                                   ha_client_t **client_ptr);

/**
 * Destroy info screen
 */
void info_screen_destroy(info_screen_t *screen);

/**
 * Set entity to display
 */
int info_screen_set_entity(info_screen_t *screen, const char *entity_id);

/**
 * Handle input for info screen
 * @return 0 to stay, -1 to go back
 */
int info_screen_handle_input(info_screen_t *screen, SDL_Event *event);

/**
 * Render info screen
 */
void info_screen_render(info_screen_t *screen);

/**
 * Check if entity should use info screen (automation/script/scene)
 */
int info_screen_should_handle(const char *entity_id);

#endif // SCREEN_INFO_H
