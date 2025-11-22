/**
 * ha_client.h - Home Assistant REST API Client
 *
 * HTTP client wrapper for Home Assistant REST API using libcurl.
 * Handles authentication, requests, and response parsing.
 *
 * Phase 2: API Client
 */

#ifndef HA_CLIENT_H
#define HA_CLIENT_H

#include <stddef.h>

/**
 * Client configuration structure
 */
typedef struct {
    char base_url[256];      // Full URL: http://homeassistant.local:8123
    char token[512];         // Long-lived access token
    int timeout;             // Request timeout in seconds (default: 30)
    int insecure;            // Skip SSL certificate verification (1 = skip, 0 = verify)
} ha_client_t;

/**
 * HTTP response structure
 */
typedef struct {
    char *data;              // Response body (JSON string) - caller must free
    size_t size;             // Response body size in bytes
    int status_code;         // HTTP status code (200, 404, etc.)
    int success;             // 1 if successful (2xx status), 0 otherwise
    char error_message[256]; // Error description if failed
} ha_response_t;

/**
 * Create a new Home Assistant client
 *
 * @param url Base URL (e.g., "http://homeassistant.local")
 * @param port Port number (usually 8123)
 * @param token Long-lived access token
 * @return Pointer to ha_client_t or NULL on failure
 */
ha_client_t* ha_client_create(const char *url, int port, const char *token);

/**
 * Destroy client and free resources
 *
 * @param client Client to destroy
 */
void ha_client_destroy(ha_client_t *client);

/**
 * Test connection to Home Assistant
 * Calls GET /api/ to verify connectivity and authentication
 *
 * @param client HA client
 * @return Response with success indicator, NULL on complete failure
 */
ha_response_t* ha_client_test_connection(ha_client_t *client);

/**
 * Get all entity states
 * Calls GET /api/states to fetch all entities
 *
 * @param client HA client
 * @return Response with JSON array of entities, NULL on failure
 */
ha_response_t* ha_client_get_states(ha_client_t *client);

/**
 * Get single entity state
 * Calls GET /api/states/<entity_id>
 *
 * @param client HA client
 * @param entity_id Entity ID (e.g., "light.living_room")
 * @return Response with single entity JSON, NULL on failure
 */
ha_response_t* ha_client_get_state(ha_client_t *client, const char *entity_id);

/**
 * Call a Home Assistant service
 * Calls POST /api/services/<domain>/<service>
 *
 * @param client HA client
 * @param domain Service domain (e.g., "light", "switch")
 * @param service Service name (e.g., "turn_on", "toggle")
 * @param entity_id Entity to control (can be NULL for domain-wide services)
 * @param params_json Additional parameters as JSON string (can be NULL)
 * @return Response with service call result, NULL on failure
 *
 * Example: ha_client_call_service(client, "light", "turn_on", "light.living_room", "{\"brightness\": 128}")
 */
ha_response_t* ha_client_call_service(ha_client_t *client,
                                       const char *domain,
                                       const char *service,
                                       const char *entity_id,
                                       const char *params_json);

/**
 * Get list of available services
 * Calls GET /api/services
 *
 * @param client HA client
 * @return Response with services JSON, NULL on failure
 */
ha_response_t* ha_client_get_services(ha_client_t *client);

/**
 * Get entity registry (includes area_id assignments)
 * Uses POST /api/template to fetch entity registry data
 *
 * @param client HA client
 * @return Response with entity registry JSON, NULL on failure
 */
ha_response_t* ha_client_get_entity_registry(ha_client_t *client);

/**
 * Free response and its data
 *
 * @param response Response to free (can be NULL)
 */
void ha_response_free(ha_response_t *response);

#endif // HA_CLIENT_H
