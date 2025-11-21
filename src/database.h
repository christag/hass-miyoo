/**
 * database.h - SQLite Database for Home Assistant Companion
 *
 * Local storage for entities, favorites, and sync metadata.
 * Enables offline mode and fast startup.
 *
 * Phase 3: Data Storage
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include "utils/json_helpers.h"

/**
 * Database context structure
 */
typedef struct {
    sqlite3 *db;
    char db_path[256];
} database_t;

/**
 * Open database connection
 *
 * @param path Path to SQLite database file
 * @return database_t pointer or NULL on failure
 */
database_t* database_open(const char *path);

/**
 * Close database connection
 *
 * @param db Database to close
 */
void database_close(database_t *db);

/**
 * Initialize database schema (creates tables if not exist)
 *
 * @param db Database connection
 * @return 1 on success, 0 on failure
 */
int database_init_schema(database_t *db);

/* ============================================
 * Entity Operations
 * ============================================ */

/**
 * Save single entity to database (insert or update)
 *
 * @param db Database connection
 * @param entity Entity to save
 * @return 1 on success, 0 on failure
 */
int database_save_entity(database_t *db, ha_entity_t *entity);

/**
 * Save multiple entities to database (batch operation)
 *
 * @param db Database connection
 * @param entities Array of entities
 * @param count Number of entities
 * @return Number of entities successfully saved
 */
int database_save_entities(database_t *db, ha_entity_t **entities, int count);

/**
 * Get all entities from database
 *
 * @param db Database connection
 * @param count Output: number of entities returned
 * @return Array of entities (caller must free with free_entities)
 */
ha_entity_t** database_get_all_entities(database_t *db, int *count);

/**
 * Get entities filtered by domain
 *
 * @param db Database connection
 * @param domain Domain filter (e.g., "light", "switch")
 * @param count Output: number of entities returned
 * @return Array of entities (caller must free with free_entities)
 */
ha_entity_t** database_get_entities_by_domain(database_t *db, const char *domain, int *count);

/**
 * Get single entity by ID
 *
 * @param db Database connection
 * @param entity_id Entity ID to find
 * @return Entity or NULL if not found (caller must free)
 */
ha_entity_t* database_get_entity(database_t *db, const char *entity_id);

/**
 * Delete entity from database
 *
 * @param db Database connection
 * @param entity_id Entity ID to delete
 * @return 1 on success, 0 on failure
 */
int database_delete_entity(database_t *db, const char *entity_id);

/**
 * Clear all entities (for full resync)
 *
 * @param db Database connection
 * @return 1 on success, 0 on failure
 */
int database_clear_entities(database_t *db);

/**
 * Update area_id for an entity
 *
 * @param db Database connection
 * @param entity_id Entity ID to update
 * @param area_id New area ID
 * @return 1 on success, 0 on failure
 */
int database_update_entity_area(database_t *db, const char *entity_id, const char *area_id);

/* ============================================
 * Favorites Operations
 * ============================================ */

/**
 * Add entity to favorites
 *
 * @param db Database connection
 * @param entity_id Entity ID to favorite
 * @return 1 on success, 0 on failure
 */
int database_add_favorite(database_t *db, const char *entity_id);

/**
 * Remove entity from favorites
 *
 * @param db Database connection
 * @param entity_id Entity ID to unfavorite
 * @return 1 on success, 0 on failure
 */
int database_remove_favorite(database_t *db, const char *entity_id);

/**
 * Check if entity is favorited
 *
 * @param db Database connection
 * @param entity_id Entity ID to check
 * @return 1 if favorite, 0 if not
 */
int database_is_favorite(database_t *db, const char *entity_id);

/**
 * Get all favorited entities
 *
 * @param db Database connection
 * @param count Output: number of favorites returned
 * @return Array of entities (caller must free with free_entities)
 */
ha_entity_t** database_get_favorites(database_t *db, int *count);

/* ============================================
 * Metadata Operations
 * ============================================ */

/**
 * Set metadata key-value pair
 *
 * @param db Database connection
 * @param key Metadata key
 * @param value Metadata value
 * @return 1 on success, 0 on failure
 */
int database_set_metadata(database_t *db, const char *key, const char *value);

/**
 * Get metadata value by key
 *
 * @param db Database connection
 * @param key Metadata key
 * @return Value string (caller must free) or NULL if not found
 */
char* database_get_metadata(database_t *db, const char *key);

/**
 * Get entity count in database
 *
 * @param db Database connection
 * @return Number of entities
 */
int database_get_entity_count(database_t *db);

#endif // DATABASE_H
