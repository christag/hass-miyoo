/**
 * test_api_client.c - Home Assistant API Client Test Program
 *
 * Standalone test program for Phase 2 API client validation.
 * Tests connection, entity retrieval, and service calls.
 *
 * Compile:
 *   gcc -o test_api tests/test_api_client.c src/ha_client.c src/utils/json_helpers.c \
 *       src/utils/config.c -Isrc -lcurl -lcjson -lm
 *
 * Run:
 *   ./test_api
 */

#include <stdio.h>
#include <string.h>
#include "ha_client.h"
#include "utils/json_helpers.h"
#include "utils/config.h"

// Test results
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    printf("\n[TEST] %s\n", name); \
    tests_run++;

#define PASS() \
    printf("  ✓ PASSED\n"); \
    tests_passed++;

#define FAIL(msg) \
    printf("  ✗ FAILED: %s\n", msg);

/**
 * Test 1: Create and destroy client
 */
void test_create_client() {
    TEST("Client creation and destruction");

    ha_client_t *client = ha_client_create("http://homeassistant.local", 8123, "test_token");
    if (!client) {
        FAIL("Failed to create client");
        return;
    }

    printf("  - Client created successfully\n");
    printf("  - Base URL: %s\n", client->base_url);
    printf("  - Timeout: %d seconds\n", client->timeout);

    ha_client_destroy(client);
    PASS();
}

/**
 * Test 2: Test connection (requires real HA instance)
 */
void test_connection(ha_client_t *client) {
    TEST("Connection test");

    if (!client) {
        FAIL("No client provided");
        return;
    }

    ha_response_t *response = ha_client_test_connection(client);
    if (!response) {
        FAIL("Got NULL response");
        return;
    }

    printf("  - HTTP Status: %d\n", response->status_code);
    printf("  - Success: %s\n", response->success ? "Yes" : "No");

    if (response->success) {
        printf("  - Response: %s\n", response->data);
        PASS();
    } else {
        printf("  - Error: %s\n", response->error_message);
        FAIL("Connection failed");
    }

    ha_response_free(response);
}

/**
 * Test 3: Get all states (requires real HA instance)
 */
void test_get_states(ha_client_t *client) {
    TEST("Get all entity states");

    if (!client) {
        FAIL("No client provided");
        return;
    }

    ha_response_t *response = ha_client_get_states(client);
    if (!response) {
        FAIL("Got NULL response");
        return;
    }

    printf("  - HTTP Status: %d\n", response->status_code);
    printf("  - Success: %s\n", response->success ? "Yes" : "No");

    if (response->success) {
        // Parse entities
        int count = 0;
        ha_entity_t **entities = parse_entities_array(response->data, &count);

        if (entities) {
            printf("  - Found %d entities\n", count);

            // Show first 5 entities
            int show_count = count < 5 ? count : 5;
            printf("  - First %d entities:\n", show_count);
            for (int i = 0; i < show_count; i++) {
                printf("    %d. %s (%s) = %s\n",
                       i + 1,
                       entities[i]->friendly_name,
                       entities[i]->entity_id,
                       entities[i]->state);
            }

            free_entities(entities, count);
            PASS();
        } else {
            FAIL("Failed to parse entities");
        }
    } else {
        printf("  - Error: %s\n", response->error_message);
        FAIL("Get states failed");
    }

    ha_response_free(response);
}

/**
 * Test 4: Get single entity state
 */
void test_get_single_state(ha_client_t *client, const char *entity_id) {
    TEST("Get single entity state");

    if (!client || !entity_id) {
        FAIL("No client or entity_id provided");
        return;
    }

    printf("  - Entity ID: %s\n", entity_id);

    ha_response_t *response = ha_client_get_state(client, entity_id);
    if (!response) {
        FAIL("Got NULL response");
        return;
    }

    printf("  - HTTP Status: %d\n", response->status_code);

    if (response->success) {
        ha_entity_t *entity = parse_single_entity(response->data);
        if (entity) {
            printf("  - Name: %s\n", entity->friendly_name);
            printf("  - State: %s\n", entity->state);
            printf("  - Domain: %s\n", entity->domain);
            printf("  - Icon: %s\n", entity->icon);
            free_entity(entity);
            PASS();
        } else {
            FAIL("Failed to parse entity");
        }
    } else {
        printf("  - Error: %s\n", response->error_message);
        FAIL("Get entity failed");
    }

    ha_response_free(response);
}

/**
 * Test 5: Configuration file loading
 */
void test_config_load() {
    TEST("Configuration file loading");

    app_config_t *config = config_load("servers.json");
    if (!config) {
        printf("  - No servers.json found (this is OK for testing)\n");
        FAIL("Config not found");
        return;
    }

    printf("  - Loaded %d server(s)\n", config->server_count);
    printf("  - Default server: %d\n", config->default_server);

    for (int i = 0; i < config->server_count; i++) {
        server_config_t *server = config_get_server(config, i);
        printf("  - Server %d: %s (%s:%d)\n", i, server->name, server->url, server->port);
    }

    config_free(config);
    PASS();
}

/**
 * Main test runner
 */
int main(int argc, char *argv[]) {
    printf("======================================\n");
    printf("Home Assistant API Client Test Suite\n");
    printf("======================================\n");

    // Test 1: Basic client creation (no network required)
    test_create_client();

    // Test 5: Config loading (no network required)
    test_config_load();

    // Load config for networked tests
    app_config_t *config = config_load("servers.json");
    ha_client_t *client = NULL;

    if (config && config->server_count > 0) {
        server_config_t *server = config_get_default_server(config);
        printf("\n[INFO] Using server: %s (%s:%d)\n", server->name, server->url, server->port);

        client = ha_client_create(server->url, server->port, server->token);

        if (client) {
            // Test 2: Connection test
            test_connection(client);

            // Test 3: Get all states
            test_get_states(client);

            // Test 4: Get single state (use first entity from config or sun.sun)
            const char *test_entity = "sun.sun"; // Sun entity exists on all HA instances
            test_get_single_state(client, test_entity);

            ha_client_destroy(client);
        }
    } else {
        printf("\n[WARNING] No servers.json found or no servers configured\n");
        printf("[INFO] Network tests skipped - create servers.json to enable\n");
    }

    if (config) {
        config_free(config);
    }

    // Print summary
    printf("\n======================================\n");
    printf("Test Summary\n");
    printf("======================================\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("\n");

    if (tests_passed == tests_run) {
        printf("✓ All tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
