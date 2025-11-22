/**
 * config.c - Configuration File Parser Implementation
 */

#include "config.h"
#include "json_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Read entire file into string
 */
static char* read_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Failed to open config file: %s\n", filepath);
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size <= 0) {
        fclose(file);
        return NULL;
    }

    // Allocate buffer
    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    // Read file
    size_t read_size = fread(buffer, 1, size, file);
    buffer[read_size] = '\0';

    fclose(file);
    return buffer;
}

app_config_t* config_load(const char *filepath) {
    if (!filepath) {
        return NULL;
    }

    // Read file
    char *json_str = read_file(filepath);
    if (!json_str) {
        return NULL;
    }

    // Parse JSON
    cJSON *root = parse_json_response(json_str);
    free(json_str);

    if (!root) {
        return NULL;
    }

    // Allocate config
    app_config_t *config = calloc(1, sizeof(app_config_t));
    if (!config) {
        cJSON_Delete(root);
        return NULL;
    }

    // Get default_server
    config->default_server = json_get_int(root, "default_server", 0);

    // Get servers array
    cJSON *servers_array = cJSON_GetObjectItem(root, "servers");
    if (!servers_array || !cJSON_IsArray(servers_array)) {
        cJSON_Delete(root);
        free(config);
        return NULL;
    }

    int server_count = cJSON_GetArraySize(servers_array);
    if (server_count == 0) {
        cJSON_Delete(root);
        free(config);
        return NULL;
    }

    // Allocate servers array
    config->servers = calloc(server_count, sizeof(server_config_t));
    if (!config->servers) {
        cJSON_Delete(root);
        free(config);
        return NULL;
    }

    // Parse each server
    int parsed = 0;
    cJSON *server_json = NULL;
    cJSON_ArrayForEach(server_json, servers_array) {
        server_config_t *server = &config->servers[parsed];

        // Get required fields
        const char *name = json_get_string(server_json, "name", "Unnamed");
        strncpy(server->name, name, sizeof(server->name) - 1);

        const char *url = json_get_string(server_json, "url", NULL);
        if (!url) {
            continue; // Skip servers without URL
        }
        strncpy(server->url, url, sizeof(server->url) - 1);

        server->port = json_get_int(server_json, "port", 8123);

        const char *token = json_get_string(server_json, "token", NULL);
        if (!token) {
            continue; // Skip servers without token
        }
        strncpy(server->token, token, sizeof(server->token) - 1);

        const char *username = json_get_string(server_json, "username", "");
        strncpy(server->username, username, sizeof(server->username) - 1);

        // Get insecure flag (default: 0 = verify certificates)
        server->insecure = json_get_int(server_json, "insecure", 0);

        parsed++;
    }

    cJSON_Delete(root);

    if (parsed == 0) {
        free(config->servers);
        free(config);
        return NULL;
    }

    config->server_count = parsed;

    // Validate default_server index
    if (config->default_server >= config->server_count) {
        config->default_server = 0;
    }

    return config;
}

void config_free(app_config_t *config) {
    if (config) {
        if (config->servers) {
            free(config->servers);
        }
        free(config);
    }
}

server_config_t* config_get_server(app_config_t *config, int index) {
    if (!config || !config->servers || index < 0 || index >= config->server_count) {
        return NULL;
    }

    return &config->servers[index];
}

server_config_t* config_get_default_server(app_config_t *config) {
    if (!config) {
        return NULL;
    }

    return config_get_server(config, config->default_server);
}
