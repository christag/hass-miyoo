/**
 * cache_manager.c - Cache Manager Implementation
 */

#include "cache_manager.h"
#include "ha_client.h"
#include "utils/json_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cache_manager_t* cache_manager_create(database_t *db, ha_client_t *client) {
    if (!db) {
        return NULL;
    }

    cache_manager_t *manager = calloc(1, sizeof(cache_manager_t));
    if (!manager) {
        return NULL;
    }

    manager->db = db;
    manager->ha_client = client;
    manager->sync_interval = DEFAULT_SYNC_INTERVAL;
    manager->online = (client != NULL) ? 1 : 0;

    // Try to load last sync time from database
    char *last_sync_str = database_get_metadata(db, "last_sync");
    if (last_sync_str) {
        manager->last_sync = (time_t)atol(last_sync_str);
        free(last_sync_str);
    }

    return manager;
}

void cache_manager_destroy(cache_manager_t *manager) {
    if (manager) {
        // Note: We don't own db or ha_client, so don't free them
        free(manager);
    }
}

void cache_manager_set_sync_interval(cache_manager_t *manager, int seconds) {
    if (manager && seconds >= 60) {
        manager->sync_interval = seconds;
    }
}

/* ============================================
 * Sync Operations
 * ============================================ */

/**
 * Parse entity-area mappings from template API response
 * JSON format: [{"e":"entity_id","a":"area_id"},...]
 */
static void parse_and_update_areas(cache_manager_t *manager, const char *json) {
    if (!manager || !json || strlen(json) < 2) {
        return;
    }

    int updated = 0;
    const char *ptr = json;

    // Simple JSON array parser for [{"e":"...","a":"..."},...]
    while ((ptr = strstr(ptr, "\"e\":\"")) != NULL) {
        ptr += 5; // Skip "e":"

        // Extract entity_id
        char entity_id[128] = {0};
        int i = 0;
        while (*ptr && *ptr != '"' && i < 127) {
            entity_id[i++] = *ptr++;
        }
        if (*ptr != '"') continue;
        ptr++; // Skip closing quote

        // Find area_id
        const char *area_ptr = strstr(ptr, "\"a\":\"");
        if (!area_ptr || area_ptr > ptr + 20) continue;
        area_ptr += 5; // Skip "a":"

        char area_id[64] = {0};
        i = 0;
        while (*area_ptr && *area_ptr != '"' && i < 63) {
            area_id[i++] = *area_ptr++;
        }

        if (strlen(entity_id) > 0 && strlen(area_id) > 0) {
            database_update_entity_area(manager->db, entity_id, area_id);
            updated++;
        }
    }

    printf("Updated area assignments for %d entities\n", updated);
}

int cache_manager_sync(cache_manager_t *manager) {
    if (!manager || !manager->ha_client) {
        return -1;
    }

    printf("Syncing with Home Assistant...\n");

    // Fetch all states from HA
    ha_response_t *response = ha_client_get_states(manager->ha_client);
    if (!response) {
        fprintf(stderr, "Sync failed: no response from HA\n");
        manager->online = 0;
        return -1;
    }

    if (!response->success) {
        fprintf(stderr, "Sync failed: %s (HTTP %d)\n",
                response->error_message, response->status_code);
        ha_response_free(response);
        manager->online = 0;
        return -1;
    }

    // Parse entities
    int count = 0;
    ha_entity_t **entities = parse_entities_array(response->data, &count);
    ha_response_free(response);

    if (!entities || count == 0) {
        fprintf(stderr, "Sync failed: no entities parsed\n");
        return -1;
    }

    printf("Parsed %d entities from Home Assistant\n", count);

    // Save to database
    int saved = database_save_entities(manager->db, entities, count);
    printf("Saved %d entities to cache\n", saved);

    // Cleanup entities
    free_entities(entities, count);

    // Fetch and merge area assignments from entity registry
    printf("Fetching area assignments...\n");
    ha_response_t *area_response = ha_client_get_entity_registry(manager->ha_client);
    if (area_response && area_response->success && area_response->data) {
        parse_and_update_areas(manager, area_response->data);
    } else {
        printf("Area fetch skipped (no response or error)\n");
    }
    if (area_response) {
        ha_response_free(area_response);
    }

    // Update sync metadata
    manager->last_sync = time(NULL);
    manager->online = 1;

    char timestamp[32];
    snprintf(timestamp, sizeof(timestamp), "%ld", (long)manager->last_sync);
    database_set_metadata(manager->db, "last_sync", timestamp);

    return saved;
}

int cache_manager_should_sync(cache_manager_t *manager) {
    if (!manager || !manager->ha_client) {
        return 0;
    }

    time_t now = time(NULL);
    return (now - manager->last_sync) >= manager->sync_interval;
}

int cache_manager_sync_if_needed(cache_manager_t *manager) {
    if (!cache_manager_should_sync(manager)) {
        return 0;
    }

    return cache_manager_sync(manager);
}

time_t cache_manager_get_last_sync(cache_manager_t *manager) {
    return manager ? manager->last_sync : 0;
}

/* ============================================
 * Entity Operations
 * ============================================ */

ha_entity_t** cache_manager_get_entities(cache_manager_t *manager, int *count) {
    if (!manager || !count) {
        return NULL;
    }

    return database_get_all_entities(manager->db, count);
}

ha_entity_t** cache_manager_get_entities_by_domain(cache_manager_t *manager,
                                                    const char *domain,
                                                    int *count) {
    if (!manager || !domain || !count) {
        return NULL;
    }

    return database_get_entities_by_domain(manager->db, domain, count);
}

ha_entity_t* cache_manager_get_entity(cache_manager_t *manager, const char *entity_id) {
    if (!manager || !entity_id) {
        return NULL;
    }

    return database_get_entity(manager->db, entity_id);
}

ha_entity_t* cache_manager_refresh_entity(cache_manager_t *manager, const char *entity_id) {
    if (!manager || !entity_id || !manager->ha_client) {
        return NULL;
    }

    // Fetch from API
    ha_response_t *response = ha_client_get_state(manager->ha_client, entity_id);
    if (!response || !response->success) {
        if (response) {
            ha_response_free(response);
        }
        // Return cached version on failure
        return database_get_entity(manager->db, entity_id);
    }

    // Parse and save
    ha_entity_t *entity = parse_single_entity(response->data);
    ha_response_free(response);

    if (entity) {
        database_save_entity(manager->db, entity);
    }

    return entity;
}

int cache_manager_update_entity_state(cache_manager_t *manager,
                                       const char *entity_id,
                                       const char *new_state) {
    if (!manager || !entity_id || !new_state) {
        return 0;
    }

    // Get current entity from cache
    ha_entity_t *entity = database_get_entity(manager->db, entity_id);
    if (!entity) {
        return 0;
    }

    // Update state
    strncpy(entity->state, new_state, sizeof(entity->state) - 1);

    // Save back to database
    int result = database_save_entity(manager->db, entity);
    free_entity(entity);

    return result;
}

/* ============================================
 * Favorites Operations
 * ============================================ */

ha_entity_t** cache_manager_get_favorites(cache_manager_t *manager, int *count) {
    if (!manager || !count) {
        return NULL;
    }

    return database_get_favorites(manager->db, count);
}

int cache_manager_add_favorite(cache_manager_t *manager, const char *entity_id) {
    if (!manager || !entity_id) {
        return 0;
    }

    return database_add_favorite(manager->db, entity_id);
}

int cache_manager_remove_favorite(cache_manager_t *manager, const char *entity_id) {
    if (!manager || !entity_id) {
        return 0;
    }

    return database_remove_favorite(manager->db, entity_id);
}

int cache_manager_toggle_favorite(cache_manager_t *manager, const char *entity_id) {
    if (!manager || !entity_id) {
        return -1;
    }

    if (database_is_favorite(manager->db, entity_id)) {
        database_remove_favorite(manager->db, entity_id);
        return 0;
    } else {
        database_add_favorite(manager->db, entity_id);
        return 1;
    }
}

int cache_manager_is_favorite(cache_manager_t *manager, const char *entity_id) {
    if (!manager || !entity_id) {
        return 0;
    }

    return database_is_favorite(manager->db, entity_id);
}

/* ============================================
 * Status
 * ============================================ */

int cache_manager_is_online(cache_manager_t *manager) {
    return manager ? manager->online : 0;
}

int cache_manager_get_entity_count(cache_manager_t *manager) {
    if (!manager) {
        return 0;
    }

    return database_get_entity_count(manager->db);
}
