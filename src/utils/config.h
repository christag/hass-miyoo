/**
 * config.h - Configuration File Parser
 *
 * Reads servers.json configuration file and provides structured access
 * to Home Assistant server configurations.
 *
 * Phase 2: API Client
 */

#ifndef CONFIG_H
#define CONFIG_H

/**
 * Single server configuration
 */
typedef struct {
    char name[64];        // Display name (e.g., "Home", "Remote")
    char url[256];        // Base URL (e.g., "http://homeassistant.local")
    int port;             // Port number (usually 8123)
    char token[512];      // Long-lived access token
    char username[64];    // Home Assistant username
    int insecure;         // Skip SSL certificate verification (1 = skip, 0 = verify)
} server_config_t;

/**
 * Application configuration
 */
typedef struct {
    server_config_t *servers;  // Array of server configurations
    int server_count;          // Number of servers
    int default_server;        // Index of default server (0-based)
} app_config_t;

/**
 * Load configuration from JSON file
 *
 * Expected JSON structure:
 * {
 *   "servers": [
 *     {
 *       "name": "Home",
 *       "url": "http://homeassistant.local",
 *       "port": 8123,
 *       "token": "...",
 *       "username": "admin"
 *     }
 *   ],
 *   "default_server": 0
 * }
 *
 * @param filepath Path to servers.json file
 * @return app_config_t pointer (caller must free with config_free) or NULL on error
 */
app_config_t* config_load(const char *filepath);

/**
 * Free configuration and all servers
 *
 * @param config Configuration to free (can be NULL)
 */
void config_free(app_config_t *config);

/**
 * Get server by index
 *
 * @param config Application configuration
 * @param index Server index (0-based)
 * @return Pointer to server config or NULL if index out of bounds
 */
server_config_t* config_get_server(app_config_t *config, int index);

/**
 * Get default server
 *
 * @param config Application configuration
 * @return Pointer to default server config or NULL if none configured
 */
server_config_t* config_get_default_server(app_config_t *config);

#endif // CONFIG_H
