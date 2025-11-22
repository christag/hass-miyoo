/**
 * ha_client.c - Home Assistant REST API Client Implementation
 */

#include "ha_client.h"
#include <curl/curl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_TIMEOUT 30
#define USER_AGENT "HACompanion/1.0 (Miyoo Mini Plus)"

/**
 * Response buffer structure for curl callbacks
 */
typedef struct {
    char *data;
    size_t size;
} response_buffer_t;

/**
 * Callback function for curl to write response data
 */
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    response_buffer_t *buffer = (response_buffer_t *)userp;

    char *ptr = realloc(buffer->data, buffer->size + real_size + 1);
    if (!ptr) {
        fprintf(stderr, "Out of memory in write_callback\n");
        return 0;
    }

    buffer->data = ptr;
    memcpy(&(buffer->data[buffer->size]), contents, real_size);
    buffer->size += real_size;
    buffer->data[buffer->size] = '\0';

    return real_size;
}

ha_client_t* ha_client_create(const char *url, int port, const char *token) {
    if (!url || !token) {
        return NULL;
    }

    ha_client_t *client = calloc(1, sizeof(ha_client_t));
    if (!client) {
        return NULL;
    }

    // Build full base URL
    snprintf(client->base_url, sizeof(client->base_url), "%s:%d", url, port);
    strncpy(client->token, token, sizeof(client->token) - 1);
    client->timeout = DEFAULT_TIMEOUT;
    client->insecure = 0;  // Default: verify SSL certificates

    // Initialize curl globally (should be done once per application)
    static int curl_initialized = 0;
    if (!curl_initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_initialized = 1;
    }

    return client;
}

void ha_client_destroy(ha_client_t *client) {
    if (client) {
        free(client);
    }
}

/**
 * Perform HTTP GET request
 */
static ha_response_t* ha_get(ha_client_t *client, const char *endpoint) {
    if (!client || !endpoint) {
        return NULL;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        return NULL;
    }

    ha_response_t *response = calloc(1, sizeof(ha_response_t));
    if (!response) {
        curl_easy_cleanup(curl);
        return NULL;
    }

    response_buffer_t buffer = {0};

    // Build full URL
    char url[512];
    snprintf(url, sizeof(url), "%s%s", client->base_url, endpoint);

    // Build authorization header
    char auth_header[768];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", client->token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Configure curl
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)client->timeout);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // SSL certificate verification (skip if insecure flag is set)
    if (client->insecure) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    // Perform request
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        snprintf(response->error_message, sizeof(response->error_message),
                 "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        response->success = 0;
        free(buffer.data);
    } else {
        long status_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        response->status_code = (int)status_code;
        response->success = (status_code >= 200 && status_code < 300);
        response->data = buffer.data;
        response->size = buffer.size;

        if (!response->success) {
            snprintf(response->error_message, sizeof(response->error_message),
                     "HTTP %d", response->status_code);
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}

/**
 * Perform HTTP POST request
 */
static ha_response_t* ha_post(ha_client_t *client, const char *endpoint, const char *post_data) {
    if (!client || !endpoint) {
        return NULL;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        return NULL;
    }

    ha_response_t *response = calloc(1, sizeof(ha_response_t));
    if (!response) {
        curl_easy_cleanup(curl);
        return NULL;
    }

    response_buffer_t buffer = {0};

    // Build full URL
    char url[512];
    snprintf(url, sizeof(url), "%s%s", client->base_url, endpoint);

    // Build authorization header
    char auth_header[768];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", client->token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Configure curl
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data ? post_data : "{}");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)client->timeout);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // SSL certificate verification (skip if insecure flag is set)
    if (client->insecure) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    // Perform request
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        snprintf(response->error_message, sizeof(response->error_message),
                 "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        response->success = 0;
        free(buffer.data);
    } else {
        long status_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        response->status_code = (int)status_code;
        response->success = (status_code >= 200 && status_code < 300);
        response->data = buffer.data;
        response->size = buffer.size;

        if (!response->success) {
            snprintf(response->error_message, sizeof(response->error_message),
                     "HTTP %d", response->status_code);
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}

ha_response_t* ha_client_test_connection(ha_client_t *client) {
    return ha_get(client, "/api/");
}

ha_response_t* ha_client_get_states(ha_client_t *client) {
    return ha_get(client, "/api/states");
}

ha_response_t* ha_client_get_state(ha_client_t *client, const char *entity_id) {
    if (!entity_id) {
        return NULL;
    }

    char endpoint[256];
    snprintf(endpoint, sizeof(endpoint), "/api/states/%s", entity_id);
    return ha_get(client, endpoint);
}

ha_response_t* ha_client_call_service(ha_client_t *client,
                                       const char *domain,
                                       const char *service,
                                       const char *entity_id,
                                       const char *params_json) {
    if (!domain || !service) {
        return NULL;
    }

    char endpoint[256];
    snprintf(endpoint, sizeof(endpoint), "/api/services/%s/%s", domain, service);

    // Build POST data
    char post_data[1024];
    if (entity_id && params_json) {
        snprintf(post_data, sizeof(post_data),
                 "{\"entity_id\":\"%s\",%s}",
                 entity_id, params_json + 1); // Skip opening brace of params_json
    } else if (entity_id) {
        snprintf(post_data, sizeof(post_data), "{\"entity_id\":\"%s\"}", entity_id);
    } else if (params_json) {
        strncpy(post_data, params_json, sizeof(post_data) - 1);
    } else {
        strcpy(post_data, "{}");
    }

    return ha_post(client, endpoint, post_data);
}

ha_response_t* ha_client_get_services(ha_client_t *client) {
    return ha_get(client, "/api/services");
}

ha_response_t* ha_client_get_entity_registry(ha_client_t *client) {
    // Use template API to get entity-area mappings
    // This Jinja template outputs JSON with entity_id -> area_id mappings
    const char *template_body =
        "{\"template\": \"{% set ns = namespace(result=[]) %}"
        "{% for entity in states %}"
        "{% set area = area_id(entity.entity_id) %}"
        "{% if area %}"
        "{% set ns.result = ns.result + ['{\\\"e\\\":\\\"' ~ entity.entity_id ~ '\\\",\\\"a\\\":\\\"' ~ area ~ '\\\"}'] %}"
        "{% endif %}"
        "{% endfor %}"
        "[{{ ns.result | join(',') }}]\"}";

    return ha_post(client, "/api/template", template_body);
}

void ha_response_free(ha_response_t *response) {
    if (response) {
        if (response->data) {
            free(response->data);
        }
        free(response);
    }
}
