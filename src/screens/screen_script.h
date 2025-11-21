/**
 * screen_script.h - Script Detail Screen
 *
 * Displays script info and provides run control.
 *
 * Phase 9.1: Screen Architecture Refactor
 */

#ifndef SCREEN_SCRIPT_H
#define SCREEN_SCRIPT_H

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

    char description[512];
    char last_triggered[64];
    char mode[32];
    int is_favorite;

    char status_message[128];
} script_screen_t;

script_screen_t* script_screen_create(SDL_Renderer *renderer,
                                       font_manager_t *fonts,
                                       icon_manager_t *icons,
                                       cache_manager_t *cache_mgr,
                                       ha_client_t **client_ptr);

void script_screen_destroy(script_screen_t *screen);
int script_screen_set_entity(script_screen_t *screen, const char *entity_id);
int script_screen_handle_input(script_screen_t *screen, SDL_Event *event);
void script_screen_render(script_screen_t *screen);

#endif // SCREEN_SCRIPT_H
