/**
 * icons.h - Icon System for Home Assistant Companion
 *
 * Icon loading, caching, and domain mapping.
 * Phase 4: UI Design System
 */

#ifndef ICONS_H
#define ICONS_H

#include <SDL.h>

/**
 * Maximum number of cached icons
 */
#define MAX_ICONS 64

/**
 * Standard icon sizes
 */
#define ICON_SIZE_SMALL  16
#define ICON_SIZE_LARGE  32

/**
 * Icon cache entry
 */
typedef struct {
    char name[32];
    SDL_Texture *texture;
    int width;
    int height;
} icon_t;

/**
 * Icon manager structure
 */
typedef struct {
    SDL_Renderer *renderer;
    icon_t icons[MAX_ICONS];
    int icon_count;
    char base_path[256];
} icon_manager_t;

/**
 * Initialize icon manager
 *
 * @param renderer SDL renderer for texture creation
 * @param base_path Path to icons directory (e.g., "assets/icons")
 * @return icon_manager_t pointer or NULL on failure
 */
icon_manager_t* icons_init(SDL_Renderer *renderer, const char *base_path);

/**
 * Destroy icon manager and free all cached textures
 *
 * @param icons Icon manager to destroy
 */
void icons_destroy(icon_manager_t *icons);

/**
 * Load or get cached icon by name
 *
 * @param icons Icon manager
 * @param name Icon name (without path or extension)
 * @return SDL_Texture pointer or NULL if not found
 */
SDL_Texture* icons_get(icon_manager_t *icons, const char *name);

/**
 * Draw icon at position
 *
 * @param icons Icon manager
 * @param name Icon name
 * @param x X position
 * @param y Y position
 * @param size Size in pixels (will scale if needed)
 */
void icons_draw(icon_manager_t *icons, const char *name, int x, int y, int size);

/**
 * Get icon name for entity domain
 *
 * @param entity_id Full entity ID (e.g., "light.living_room")
 * @return Icon name string (static, do not free)
 */
const char* icons_get_for_domain(const char *entity_id);

/**
 * Get icon name for entity state
 *
 * @param entity_id Full entity ID
 * @param state Entity state string
 * @return Icon name string (static, do not free)
 */
const char* icons_get_for_state(const char *entity_id, const char *state);

/**
 * Preload essential icons
 *
 * @param icons Icon manager
 */
void icons_preload(icon_manager_t *icons);

/**
 * Generate built-in icons programmatically
 * Call this to create default icons if PNG files don't exist
 *
 * @param icons Icon manager
 */
void icons_generate_builtin(icon_manager_t *icons);

#endif // ICONS_H
