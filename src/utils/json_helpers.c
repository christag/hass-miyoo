/**
 * json_helpers.c - JSON Parsing Implementation
 */

#include "json_helpers.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

cJSON* parse_json_response(const char *json_string) {
    if (!json_string) {
        return NULL;
    }

    cJSON *json = cJSON_Parse(json_string);
    if (!json) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            fprintf(stderr, "JSON parse error before: %s\n", error_ptr);
        }
        return NULL;
    }

    return json;
}

const char* json_get_string(cJSON *obj, const char *key, const char *default_val) {
    if (!obj || !key) {
        return default_val;
    }

    cJSON *item = cJSON_GetObjectItem(obj, key);
    if (item && cJSON_IsString(item) && item->valuestring) {
        return item->valuestring;
    }

    return default_val;
}

int json_get_int(cJSON *obj, const char *key, int default_val) {
    if (!obj || !key) {
        return default_val;
    }

    cJSON *item = cJSON_GetObjectItem(obj, key);
    if (item && cJSON_IsNumber(item)) {
        return item->valueint;
    }

    return default_val;
}

void extract_domain(const char *entity_id, char *domain) {
    if (!entity_id || !domain) {
        return;
    }

    // Find the first dot
    const char *dot = strchr(entity_id, '.');
    if (dot) {
        size_t len = dot - entity_id;
        if (len > 31) len = 31; // Max domain length
        strncpy(domain, entity_id, len);
        domain[len] = '\0';
    } else {
        strncpy(domain, entity_id, 31);
        domain[31] = '\0';
    }
}

ha_entity_t* parse_entity_from_json(cJSON *json) {
    if (!json || !cJSON_IsObject(json)) {
        return NULL;
    }

    ha_entity_t *entity = calloc(1, sizeof(ha_entity_t));
    if (!entity) {
        return NULL;
    }

    // Extract entity_id (required)
    const char *entity_id = json_get_string(json, "entity_id", NULL);
    if (!entity_id) {
        free(entity);
        return NULL;
    }
    strncpy(entity->entity_id, entity_id, sizeof(entity->entity_id) - 1);

    // Extract domain from entity_id
    extract_domain(entity_id, entity->domain);

    // Extract state (required)
    const char *state = json_get_string(json, "state", "unknown");
    strncpy(entity->state, state, sizeof(entity->state) - 1);

    // Extract timestamps
    const char *last_changed = json_get_string(json, "last_changed", "");
    strncpy(entity->last_changed, last_changed, sizeof(entity->last_changed) - 1);

    const char *last_updated = json_get_string(json, "last_updated", "");
    strncpy(entity->last_updated, last_updated, sizeof(entity->last_updated) - 1);

    // Extract attributes
    cJSON *attributes = cJSON_GetObjectItem(json, "attributes");
    if (attributes && cJSON_IsObject(attributes)) {
        // Get friendly_name
        const char *friendly_name = json_get_string(attributes, "friendly_name", entity_id);
        strncpy(entity->friendly_name, friendly_name, sizeof(entity->friendly_name) - 1);

        // Get icon
        const char *icon = json_get_string(attributes, "icon", "");
        strncpy(entity->icon, icon, sizeof(entity->icon) - 1);

        // Get supported_features
        entity->supported_features = json_get_int(attributes, "supported_features", 0);

        // Get area_id (for room grouping)
        const char *area_id = json_get_string(attributes, "area_id", "");
        strncpy(entity->area_id, area_id, sizeof(entity->area_id) - 1);

        // Store full attributes as JSON string
        char *attrs_str = cJSON_PrintUnformatted(attributes);
        if (attrs_str) {
            entity->attributes_json = attrs_str; // Caller must free this
        }
    } else {
        // No attributes, use entity_id as friendly name
        strncpy(entity->friendly_name, entity_id, sizeof(entity->friendly_name) - 1);
    }

    return entity;
}

ha_entity_t* parse_single_entity(const char *json_string) {
    cJSON *json = parse_json_response(json_string);
    if (!json) {
        return NULL;
    }

    ha_entity_t *entity = parse_entity_from_json(json);
    cJSON_Delete(json);

    return entity;
}

ha_entity_t** parse_entities_array(const char *json_string, int *count) {
    if (!json_string || !count) {
        return NULL;
    }

    *count = 0;

    cJSON *json = parse_json_response(json_string);
    if (!json) {
        return NULL;
    }

    if (!cJSON_IsArray(json)) {
        cJSON_Delete(json);
        return NULL;
    }

    int array_size = cJSON_GetArraySize(json);
    if (array_size == 0) {
        cJSON_Delete(json);
        return NULL;
    }

    // Allocate array of entity pointers
    ha_entity_t **entities = calloc(array_size, sizeof(ha_entity_t *));
    if (!entities) {
        cJSON_Delete(json);
        return NULL;
    }

    int parsed_count = 0;
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, json) {
        ha_entity_t *entity = parse_entity_from_json(item);
        if (entity) {
            entities[parsed_count++] = entity;
        }
    }

    cJSON_Delete(json);

    if (parsed_count == 0) {
        free(entities);
        return NULL;
    }

    *count = parsed_count;
    return entities;
}

void free_entity(ha_entity_t *entity) {
    if (entity) {
        if (entity->attributes_json) {
            free(entity->attributes_json);
        }
        free(entity);
    }
}

void free_entities(ha_entity_t **entities, int count) {
    if (entities) {
        for (int i = 0; i < count; i++) {
            free_entity(entities[i]);
        }
        free(entities);
    }
}
