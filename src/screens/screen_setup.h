/**
 * screen_setup.h - Server Setup Screen
 *
 * Displays list of configured Home Assistant servers and allows
 * connection testing. View-only (no editing configuration).
 *
 * Phase 5: Setup Screen
 */

#ifndef SCREEN_SETUP_H
#define SCREEN_SETUP_H

#include <SDL.h>
#include "../ui/fonts.h"
#include "../ui/icons.h"
#include "../ui/components.h"
#include "../utils/config.h"
#include "../ha_client.h"

/**
 * Connection status for each server
 */
typedef enum {
    CONN_STATUS_UNKNOWN,
    CONN_STATUS_TESTING,
    CONN_STATUS_CONNECTED,
    CONN_STATUS_FAILED
} connection_status_t;

/**
 * Setup screen state
 */
typedef struct {
    // Dependencies
    SDL_Renderer *renderer;
    font_manager_t *fonts;
    icon_manager_t *icons;
    app_config_t *config;

    // State
    int selected_index;
    connection_status_t *server_status;  // Array of status per server
    char status_message[128];

    // For passing back selected server
    ha_client_t **client_ptr;  // Pointer to app's ha_client
} setup_screen_t;

/**
 * Create setup screen
 *
 * @param renderer SDL renderer
 * @param fonts Font manager
 * @param icons Icon manager
 * @param config Application config (may be NULL)
 * @param client_ptr Pointer to app's ha_client pointer (for updating on connect)
 * @return setup_screen_t pointer or NULL on failure
 */
setup_screen_t* setup_screen_create(SDL_Renderer *renderer,
                                     font_manager_t *fonts,
                                     icon_manager_t *icons,
                                     app_config_t *config,
                                     ha_client_t **client_ptr);

/**
 * Destroy setup screen
 *
 * @param screen Setup screen to destroy
 */
void setup_screen_destroy(setup_screen_t *screen);

/**
 * Handle input for setup screen
 *
 * @param screen Setup screen
 * @param event SDL event
 * @return 1 if should switch to main screen, 0 to stay on setup
 */
int setup_screen_handle_input(setup_screen_t *screen, SDL_Event *event);

/**
 * Render setup screen
 *
 * @param screen Setup screen
 */
void setup_screen_render(setup_screen_t *screen);

/**
 * Test connection to selected server
 *
 * @param screen Setup screen
 * @return 1 if connection successful, 0 if failed
 */
int setup_screen_test_connection(setup_screen_t *screen);

/**
 * Get currently selected server config
 *
 * @param screen Setup screen
 * @return server_config_t pointer or NULL
 */
server_config_t* setup_screen_get_selected_server(setup_screen_t *screen);

#endif // SCREEN_SETUP_H
