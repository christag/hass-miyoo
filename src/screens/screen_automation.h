/**
 * screen_automation.h - Automation Detail Screen
 *
 * Displays automation info and provides trigger control.
 *
 * Phase 9.1: Screen Architecture Refactor
 */

#ifndef SCREEN_AUTOMATION_H
#define SCREEN_AUTOMATION_H

#include <SDL.h>
#include "../ui/fonts.h"
#include "../ui/icons.h"
#include "../ui/components.h"
#include "../cache_manager.h"
#include "../ha_client.h"

typedef struct {
    SDL_Renderer *renderer;
    font_manager_t *fonts;
    icon_manager_t *icons;
    cache_manager_t *cache_mgr;
    ha_client_t **client_ptr;

    ha_entity_t *entity;
    char entity_id[128];

    // Parsed info
    char description[512];
    char last_triggered[64];
    char mode[32];
    int is_enabled;
    int is_favorite;

    char status_message[128];
} automation_screen_t;

automation_screen_t* automation_screen_create(SDL_Renderer *renderer,
                                               font_manager_t *fonts,
                                               icon_manager_t *icons,
                                               cache_manager_t *cache_mgr,
                                               ha_client_t **client_ptr);

void automation_screen_destroy(automation_screen_t *screen);
int automation_screen_set_entity(automation_screen_t *screen, const char *entity_id);
int automation_screen_handle_input(automation_screen_t *screen, SDL_Event *event);
void automation_screen_render(automation_screen_t *screen);

#endif // SCREEN_AUTOMATION_H
