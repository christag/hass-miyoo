/**
 * json_helpers.h - JSON Parsing Helpers for Home Assistant Responses
 *
 * Simplifies parsing of Home Assistant API JSON responses using cJSON.
 * Provides structured entity data and helper functions.
 *
 * Phase 2: API Client
 */

#ifndef JSON_HELPERS_H
#define JSON_HELPERS_H

#include <cjson/cJSON.h>

/**
 * Entity data structure
 * Represents a single Home Assistant entity with its state and attributes
 */
typedef struct {
    char entity_id[128];          // e.g., "light.living_room"
    char state[64];               // e.g., "on", "off", "23.5"
    char friendly_name[128];      // e.g., "Living Room Light"
    char icon[64];                // e.g., "mdi:lightbulb"
    char domain[32];              // e.g., "light", "switch", "sensor"
    char area_id[64];             // e.g., "living_room" (from HA area registry)
    char *attributes_json;        // Full attributes as JSON string (caller must free)
    int supported_features;       // Bitmask of supported features
    char last_changed[32];        // ISO timestamp
    char last_updated[32];        // ISO timestamp
} ha_entity_t;

/**
 * Parse JSON response string into cJSON object
 *
 * @param json_string JSON string from API response
 * @return cJSON object (caller must free with cJSON_Delete) or NULL on error
 */
cJSON* parse_json_response(const char *json_string);

/**
 * Parse array of entities from /api/states response
 *
 * @param json_string JSON array string
 * @param count Output parameter for number of entities parsed
 * @return Array of ha_entity_t pointers (caller must free with free_entities) or NULL on error
 */
ha_entity_t** parse_entities_array(const char *json_string, int *count);

/**
 * Parse single entity from /api/states/<entity_id> response
 *
 * @param json_string JSON object string
 * @return Single ha_entity_t (caller must free with free_entity) or NULL on error
 */
ha_entity_t* parse_single_entity(const char *json_string);

/**
 * Parse single entity from cJSON object (internal helper)
 *
 * @param json cJSON object representing an entity
 * @return Single ha_entity_t (caller must free with free_entity) or NULL on error
 */
ha_entity_t* parse_entity_from_json(cJSON *json);

/**
 * Free single entity and its attributes
 *
 * @param entity Entity to free (can be NULL)
 */
void free_entity(ha_entity_t *entity);

/**
 * Free array of entities
 *
 * @param entities Array of entities
 * @param count Number of entities in array
 */
void free_entities(ha_entity_t **entities, int count);

/**
 * Get string value from JSON object with default fallback
 *
 * @param obj JSON object
 * @param key Key to look up
 * @param default_val Default value if key not found or not a string
 * @return String value or default
 */
const char* json_get_string(cJSON *obj, const char *key, const char *default_val);

/**
 * Get integer value from JSON object with default fallback
 *
 * @param obj JSON object
 * @param key Key to look up
 * @param default_val Default value if key not found or not a number
 * @return Integer value or default
 */
int json_get_int(cJSON *obj, const char *key, int default_val);

/**
 * Extract domain from entity_id (e.g., "light" from "light.living_room")
 *
 * @param entity_id Full entity ID
 * @param domain Output buffer for domain (min 32 chars)
 */
void extract_domain(const char *entity_id, char *domain);

#endif // JSON_HELPERS_H
