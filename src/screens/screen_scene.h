/**
 * screen_scene.h - Scene Detail Screen
 *
 * Displays scene info and provides activate control.
 *
 * Phase 9.1: Screen Architecture Refactor
 */

#ifndef SCREEN_SCENE_H
#define SCREEN_SCENE_H

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

    char last_activated[64];
    int entity_count;  // Number of entities in scene
    int is_favorite;

    char status_message[128];
} scene_screen_t;

scene_screen_t* scene_screen_create(SDL_Renderer *renderer,
                                     font_manager_t *fonts,
                                     icon_manager_t *icons,
                                     cache_manager_t *cache_mgr,
                                     ha_client_t **client_ptr);

void scene_screen_destroy(scene_screen_t *screen);
int scene_screen_set_entity(scene_screen_t *screen, const char *entity_id);
int scene_screen_handle_input(scene_screen_t *screen, SDL_Event *event);
void scene_screen_render(scene_screen_t *screen);

#endif // SCREEN_SCENE_H
