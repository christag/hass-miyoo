/**
 * cache_manager.h - Cache Management for Home Assistant Companion
 *
 * High-level caching logic that coordinates between the API client
 * and local SQLite database. Handles sync, offline mode, and updates.
 *
 * Phase 3: Data Storage
 */

#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

#include "database.h"
#include "ha_client.h"
#include <time.h>

/**
 * Default sync interval in seconds (5 minutes)
 */
#define DEFAULT_SYNC_INTERVAL 300

/**
 * Cache manager context
 */
typedef struct {
    database_t *db;
    ha_client_t *ha_client;
    time_t last_sync;
    int sync_interval;
    int online;            // 1 if connected to HA, 0 if offline
} cache_manager_t;

/**
 * Create cache manager
 *
 * @param db Database connection (required)
 * @param client HA client (can be NULL for offline mode)
 * @return cache_manager_t pointer or NULL on failure
 */
cache_manager_t* cache_manager_create(database_t *db, ha_client_t *client);

/**
 * Destroy cache manager
 *
 * @param manager Manager to destroy
 */
void cache_manager_destroy(cache_manager_t *manager);

/**
 * Set sync interval
 *
 * @param manager Cache manager
 * @param seconds Interval in seconds (minimum 60)
 */
void cache_manager_set_sync_interval(cache_manager_t *manager, int seconds);

/* ============================================
 * Sync Operations
 * ============================================ */

/**
 * Perform full sync with Home Assistant
 * Fetches all entities and updates the cache
 *
 * @param manager Cache manager
 * @return Number of entities synced, or -1 on failure
 */
int cache_manager_sync(cache_manager_t *manager);

/**
 * Check if sync is needed based on interval
 *
 * @param manager Cache manager
 * @return 1 if sync is needed, 0 otherwise
 */
int cache_manager_should_sync(cache_manager_t *manager);

/**
 * Sync only if interval has elapsed
 *
 * @param manager Cache manager
 * @return Number of entities synced, 0 if skipped, -1 on failure
 */
int cache_manager_sync_if_needed(cache_manager_t *manager);

/**
 * Get last sync timestamp
 *
 * @param manager Cache manager
 * @return time_t of last sync, or 0 if never synced
 */
time_t cache_manager_get_last_sync(cache_manager_t *manager);

/* ============================================
 * Entity Operations (through cache)
 * ============================================ */

/**
 * Get all entities from cache
 * Returns cached data, syncs first if stale
 *
 * @param manager Cache manager
 * @param count Output: number of entities
 * @return Array of entities (caller must free with free_entities)
 */
ha_entity_t** cache_manager_get_entities(cache_manager_t *manager, int *count);

/**
 * Get entities by domain from cache
 *
 * @param manager Cache manager
 * @param domain Domain filter (e.g., "light")
 * @param count Output: number of entities
 * @return Array of entities (caller must free with free_entities)
 */
ha_entity_t** cache_manager_get_entities_by_domain(cache_manager_t *manager,
                                                    const char *domain,
                                                    int *count);

/**
 * Get single entity from cache
 *
 * @param manager Cache manager
 * @param entity_id Entity ID
 * @return Entity (caller must free) or NULL
 */
ha_entity_t* cache_manager_get_entity(cache_manager_t *manager, const char *entity_id);

/**
 * Refresh single entity from API and update cache
 *
 * @param manager Cache manager
 * @param entity_id Entity ID to refresh
 * @return Updated entity (caller must free) or NULL on failure
 */
ha_entity_t* cache_manager_refresh_entity(cache_manager_t *manager, const char *entity_id);

/**
 * Update entity state in cache after control action
 * Used for optimistic updates before API confirms
 *
 * @param manager Cache manager
 * @param entity_id Entity ID
 * @param new_state New state value
 * @return 1 on success, 0 on failure
 */
int cache_manager_update_entity_state(cache_manager_t *manager,
                                       const char *entity_id,
                                       const char *new_state);

/* ============================================
 * Favorites Operations (through cache)
 * ============================================ */

/**
 * Get favorite entities
 *
 * @param manager Cache manager
 * @param count Output: number of favorites
 * @return Array of entities (caller must free with free_entities)
 */
ha_entity_t** cache_manager_get_favorites(cache_manager_t *manager, int *count);

/**
 * Add entity to favorites
 *
 * @param manager Cache manager
 * @param entity_id Entity ID
 * @return 1 on success, 0 on failure
 */
int cache_manager_add_favorite(cache_manager_t *manager, const char *entity_id);

/**
 * Remove entity from favorites
 *
 * @param manager Cache manager
 * @param entity_id Entity ID
 * @return 1 on success, 0 on failure
 */
int cache_manager_remove_favorite(cache_manager_t *manager, const char *entity_id);

/**
 * Toggle favorite status
 *
 * @param manager Cache manager
 * @param entity_id Entity ID
 * @return 1 if now favorited, 0 if now unfavorited, -1 on error
 */
int cache_manager_toggle_favorite(cache_manager_t *manager, const char *entity_id);

/**
 * Check if entity is favorited
 *
 * @param manager Cache manager
 * @param entity_id Entity ID
 * @return 1 if favorite, 0 if not
 */
int cache_manager_is_favorite(cache_manager_t *manager, const char *entity_id);

/* ============================================
 * Status
 * ============================================ */

/**
 * Check if connected to Home Assistant
 *
 * @param manager Cache manager
 * @return 1 if online, 0 if offline
 */
int cache_manager_is_online(cache_manager_t *manager);

/**
 * Get entity count in cache
 *
 * @param manager Cache manager
 * @return Number of cached entities
 */
int cache_manager_get_entity_count(cache_manager_t *manager);

#endif // CACHE_MANAGER_H
