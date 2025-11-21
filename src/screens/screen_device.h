/**
 * screen_device.h - Device/Entity Detail Screen
 *
 * Shows detailed view of an entity with inline controls.
 * Supports lights (brightness), climate (temperature), covers, locks, etc.
 *
 * Phase 7: Device Detail
 */

#ifndef SCREEN_DEVICE_H
#define SCREEN_DEVICE_H

#include <SDL.h>
#include "../ui/fonts.h"
#include "../ui/icons.h"
#include "../ui/components.h"
#include "../cache_manager.h"
#include "../ha_client.h"

/**
 * Control types for different entity domains
 */
typedef enum {
    CTRL_TOGGLE,       // Simple on/off (light, switch, fan)
    CTRL_BRIGHTNESS,   // Slider 0-255 (light with brightness)
    CTRL_TEMPERATURE,  // Temperature adjustment (climate)
    CTRL_POSITION,     // Position slider 0-100 (cover)
    CTRL_LOCK,         // Lock/unlock (lock)
    CTRL_ACTIVATE,     // One-shot activation (scene, script, automation)
    CTRL_NONE          // Read-only (sensor)
} control_type_t;

/**
 * Device screen state
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

    // Control state
    control_type_t control_type;
    int control_value;       // Current slider value (legacy/generic)
    int control_min;
    int control_max;
    int control_step;

    // Light-specific controls
    int has_brightness;      // Light supports brightness
    int has_color_temp;      // Light supports color temperature
    int brightness_value;    // 0-255
    int color_temp_value;    // In mireds (lower = cooler, higher = warmer)
    int color_temp_min;      // Min mireds (coolest, e.g., 153 = 6500K)
    int color_temp_max;      // Max mireds (warmest, e.g., 500 = 2000K)

    // UI state
    int selected_control;    // 0=main toggle, 1=brightness, 2=color_temp, 3=favorite
    int max_controls;        // Total number of controls (dynamic based on features)
    int is_favorite;

    // Status
    char status_message[128];
    int action_pending;
} device_screen_t;

/**
 * Create device screen
 *
 * @param renderer SDL renderer
 * @param fonts Font manager
 * @param icons Icon manager
 * @param cache_mgr Cache manager
 * @param client_ptr Pointer to HA client
 * @return device_screen_t pointer or NULL on failure
 */
device_screen_t* device_screen_create(SDL_Renderer *renderer,
                                       font_manager_t *fonts,
                                       icon_manager_t *icons,
                                       cache_manager_t *cache_mgr,
                                       ha_client_t **client_ptr);

/**
 * Destroy device screen
 *
 * @param screen Device screen to destroy
 */
void device_screen_destroy(device_screen_t *screen);

/**
 * Set entity to display
 *
 * @param screen Device screen
 * @param entity_id Entity ID to display
 * @return 1 on success, 0 on failure
 */
int device_screen_set_entity(device_screen_t *screen, const char *entity_id);

/**
 * Handle input for device screen
 *
 * @param screen Device screen
 * @param event SDL event
 * @return 0 to stay, -1 to go back
 */
int device_screen_handle_input(device_screen_t *screen, SDL_Event *event);

/**
 * Render device screen
 *
 * @param screen Device screen
 */
void device_screen_render(device_screen_t *screen);

/**
 * Refresh entity data
 *
 * @param screen Device screen
 */
void device_screen_refresh(device_screen_t *screen);

#endif // SCREEN_DEVICE_H
