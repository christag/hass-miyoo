/**
 * database.c - SQLite Database Implementation
 */

#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================
 * SQL Schema
 * ============================================ */

static const char *SCHEMA_SQL =
    "CREATE TABLE IF NOT EXISTS entities ("
    "    entity_id TEXT PRIMARY KEY,"
    "    state TEXT,"
    "    friendly_name TEXT,"
    "    icon TEXT,"
    "    domain TEXT,"
    "    area_id TEXT,"
    "    attributes_json TEXT,"
    "    supported_features INTEGER,"
    "    last_changed TEXT,"
    "    last_updated TEXT"
    ");"
    "CREATE TABLE IF NOT EXISTS favorites ("
    "    entity_id TEXT PRIMARY KEY,"
    "    added_at TEXT DEFAULT CURRENT_TIMESTAMP"
    ");"
    "CREATE TABLE IF NOT EXISTS metadata ("
    "    key TEXT PRIMARY KEY,"
    "    value TEXT"
    ");"
    "CREATE INDEX IF NOT EXISTS idx_entities_domain ON entities(domain);"
    "CREATE INDEX IF NOT EXISTS idx_entities_area ON entities(area_id);";

/* ============================================
 * Database Lifecycle
 * ============================================ */

database_t* database_open(const char *path) {
    if (!path) {
        return NULL;
    }

    database_t *db = calloc(1, sizeof(database_t));
    if (!db) {
        return NULL;
    }

    strncpy(db->db_path, path, sizeof(db->db_path) - 1);

    int rc = sqlite3_open(path, &db->db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db->db));
        free(db);
        return NULL;
    }

    // Enable foreign keys
    sqlite3_exec(db->db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);

    return db;
}

void database_close(database_t *db) {
    if (db) {
        if (db->db) {
            sqlite3_close(db->db);
        }
        free(db);
    }
}

int database_init_schema(database_t *db) {
    if (!db || !db->db) {
        return 0;
    }

    char *err_msg = NULL;
    int rc = sqlite3_exec(db->db, SCHEMA_SQL, NULL, NULL, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Schema creation error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 0;
    }

    return 1;
}

/* ============================================
 * Entity Operations
 * ============================================ */

int database_save_entity(database_t *db, ha_entity_t *entity) {
    if (!db || !db->db || !entity) {
        return 0;
    }

    const char *sql =
        "INSERT OR REPLACE INTO entities "
        "(entity_id, state, friendly_name, icon, domain, area_id, attributes_json, "
        "supported_features, last_changed, last_updated) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Prepare error: %s\n", sqlite3_errmsg(db->db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, entity->entity_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, entity->state, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, entity->friendly_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, entity->icon, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, entity->domain, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, entity->area_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, entity->attributes_json, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, entity->supported_features);
    sqlite3_bind_text(stmt, 9, entity->last_changed, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 10, entity->last_updated, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 1 : 0;
}

int database_save_entities(database_t *db, ha_entity_t **entities, int count) {
    if (!db || !db->db || !entities || count <= 0) {
        return 0;
    }

    // Begin transaction for better performance
    sqlite3_exec(db->db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    int saved = 0;
    for (int i = 0; i < count; i++) {
        if (database_save_entity(db, entities[i])) {
            saved++;
        }
    }

    sqlite3_exec(db->db, "COMMIT;", NULL, NULL, NULL);

    return saved;
}

/**
 * Helper: Create entity from SQLite row
 */
static ha_entity_t* entity_from_row(sqlite3_stmt *stmt) {
    ha_entity_t *entity = calloc(1, sizeof(ha_entity_t));
    if (!entity) {
        return NULL;
    }

    const char *text;

    text = (const char *)sqlite3_column_text(stmt, 0);
    if (text) strncpy(entity->entity_id, text, sizeof(entity->entity_id) - 1);

    text = (const char *)sqlite3_column_text(stmt, 1);
    if (text) strncpy(entity->state, text, sizeof(entity->state) - 1);

    text = (const char *)sqlite3_column_text(stmt, 2);
    if (text) strncpy(entity->friendly_name, text, sizeof(entity->friendly_name) - 1);

    text = (const char *)sqlite3_column_text(stmt, 3);
    if (text) strncpy(entity->icon, text, sizeof(entity->icon) - 1);

    text = (const char *)sqlite3_column_text(stmt, 4);
    if (text) strncpy(entity->domain, text, sizeof(entity->domain) - 1);

    text = (const char *)sqlite3_column_text(stmt, 5);
    if (text) strncpy(entity->area_id, text, sizeof(entity->area_id) - 1);

    text = (const char *)sqlite3_column_text(stmt, 6);
    if (text) entity->attributes_json = strdup(text);

    entity->supported_features = sqlite3_column_int(stmt, 7);

    text = (const char *)sqlite3_column_text(stmt, 8);
    if (text) strncpy(entity->last_changed, text, sizeof(entity->last_changed) - 1);

    text = (const char *)sqlite3_column_text(stmt, 9);
    if (text) strncpy(entity->last_updated, text, sizeof(entity->last_updated) - 1);

    return entity;
}

ha_entity_t** database_get_all_entities(database_t *db, int *count) {
    if (!db || !db->db || !count) {
        return NULL;
    }

    *count = 0;

    // First, get count
    const char *count_sql = "SELECT COUNT(*) FROM entities;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, count_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return NULL;
    }

    int total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (total == 0) {
        return NULL;
    }

    // Allocate array
    ha_entity_t **entities = calloc(total, sizeof(ha_entity_t *));
    if (!entities) {
        return NULL;
    }

    // Fetch entities
    const char *sql =
        "SELECT entity_id, state, friendly_name, icon, domain, area_id, "
        "attributes_json, supported_features, last_changed, last_updated "
        "FROM entities ORDER BY friendly_name;";

    rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        free(entities);
        return NULL;
    }

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < total) {
        entities[i] = entity_from_row(stmt);
        if (entities[i]) {
            i++;
        }
    }
    sqlite3_finalize(stmt);

    *count = i;
    return entities;
}

ha_entity_t** database_get_entities_by_domain(database_t *db, const char *domain, int *count) {
    if (!db || !db->db || !domain || !count) {
        return NULL;
    }

    *count = 0;

    // First, get count
    const char *count_sql = "SELECT COUNT(*) FROM entities WHERE domain = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, count_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, domain, -1, SQLITE_STATIC);

    int total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (total == 0) {
        return NULL;
    }

    // Allocate array
    ha_entity_t **entities = calloc(total, sizeof(ha_entity_t *));
    if (!entities) {
        return NULL;
    }

    // Fetch entities
    const char *sql =
        "SELECT entity_id, state, friendly_name, icon, domain, area_id, "
        "attributes_json, supported_features, last_changed, last_updated "
        "FROM entities WHERE domain = ? ORDER BY friendly_name;";

    rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        free(entities);
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, domain, -1, SQLITE_STATIC);

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < total) {
        entities[i] = entity_from_row(stmt);
        if (entities[i]) {
            i++;
        }
    }
    sqlite3_finalize(stmt);

    *count = i;
    return entities;
}

ha_entity_t* database_get_entity(database_t *db, const char *entity_id) {
    if (!db || !db->db || !entity_id) {
        return NULL;
    }

    const char *sql =
        "SELECT entity_id, state, friendly_name, icon, domain, area_id, "
        "attributes_json, supported_features, last_changed, last_updated "
        "FROM entities WHERE entity_id = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, entity_id, -1, SQLITE_STATIC);

    ha_entity_t *entity = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        entity = entity_from_row(stmt);
    }
    sqlite3_finalize(stmt);

    return entity;
}

int database_delete_entity(database_t *db, const char *entity_id) {
    if (!db || !db->db || !entity_id) {
        return 0;
    }

    const char *sql = "DELETE FROM entities WHERE entity_id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, entity_id, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 1 : 0;
}

int database_clear_entities(database_t *db) {
    if (!db || !db->db) {
        return 0;
    }

    int rc = sqlite3_exec(db->db, "DELETE FROM entities;", NULL, NULL, NULL);
    return (rc == SQLITE_OK) ? 1 : 0;
}

/* ============================================
 * Favorites Operations
 * ============================================ */

int database_add_favorite(database_t *db, const char *entity_id) {
    if (!db || !db->db || !entity_id) {
        return 0;
    }

    const char *sql = "INSERT OR IGNORE INTO favorites (entity_id) VALUES (?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, entity_id, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 1 : 0;
}

int database_remove_favorite(database_t *db, const char *entity_id) {
    if (!db || !db->db || !entity_id) {
        return 0;
    }

    const char *sql = "DELETE FROM favorites WHERE entity_id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, entity_id, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 1 : 0;
}

int database_is_favorite(database_t *db, const char *entity_id) {
    if (!db || !db->db || !entity_id) {
        return 0;
    }

    const char *sql = "SELECT 1 FROM favorites WHERE entity_id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, entity_id, -1, SQLITE_STATIC);
    int found = (sqlite3_step(stmt) == SQLITE_ROW) ? 1 : 0;
    sqlite3_finalize(stmt);

    return found;
}

ha_entity_t** database_get_favorites(database_t *db, int *count) {
    if (!db || !db->db || !count) {
        return NULL;
    }

    *count = 0;

    // Get count
    const char *count_sql = "SELECT COUNT(*) FROM favorites;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, count_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return NULL;
    }

    int total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (total == 0) {
        return NULL;
    }

    // Allocate array
    ha_entity_t **entities = calloc(total, sizeof(ha_entity_t *));
    if (!entities) {
        return NULL;
    }

    // Fetch favorites with entity data
    const char *sql =
        "SELECT e.entity_id, e.state, e.friendly_name, e.icon, e.domain, e.area_id, "
        "e.attributes_json, e.supported_features, e.last_changed, e.last_updated "
        "FROM entities e INNER JOIN favorites f ON e.entity_id = f.entity_id "
        "ORDER BY f.added_at;";

    rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        free(entities);
        return NULL;
    }

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < total) {
        entities[i] = entity_from_row(stmt);
        if (entities[i]) {
            i++;
        }
    }
    sqlite3_finalize(stmt);

    *count = i;
    return entities;
}

/* ============================================
 * Metadata Operations
 * ============================================ */

int database_set_metadata(database_t *db, const char *key, const char *value) {
    if (!db || !db->db || !key || !value) {
        return 0;
    }

    const char *sql = "INSERT OR REPLACE INTO metadata (key, value) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, value, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 1 : 0;
}

char* database_get_metadata(database_t *db, const char *key) {
    if (!db || !db->db || !key) {
        return NULL;
    }

    const char *sql = "SELECT value FROM metadata WHERE key = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);

    char *value = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *text = (const char *)sqlite3_column_text(stmt, 0);
        if (text) {
            value = strdup(text);
        }
    }
    sqlite3_finalize(stmt);

    return value;
}

int database_get_entity_count(database_t *db) {
    if (!db || !db->db) {
        return 0;
    }

    const char *sql = "SELECT COUNT(*) FROM entities;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    return count;
}

int database_update_entity_area(database_t *db, const char *entity_id, const char *area_id) {
    if (!db || !db->db || !entity_id) {
        return 0;
    }

    const char *sql = "UPDATE entities SET area_id = ? WHERE entity_id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, area_id ? area_id : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, entity_id, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 1 : 0;
}
