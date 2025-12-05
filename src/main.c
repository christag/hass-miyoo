/**
 * main.c - Home Assistant Companion for Miyoo Mini Plus
 *
 * A retro-styled native companion app for Home Assistant,
 * built for the Miyoo Mini Plus handheld console running OnionOS.
 *
 * Phase 1: SDL2 boilerplate with input handling and Game Boy theme
 * Phase 2: Home Assistant API client integration
 * Phase 3: Local database and cache management
 */

// DEBUG: Set to 1 to skip all network/database and just render test screen
#define SKIP_NETWORK_TEST 1

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string.h>

#include "ui/colors.h"
#include "ui/fonts.h"
#include "ui/icons.h"
#include "ui/components.h"
#include "screens/screen_test.h"
#include "screens/screen_setup.h"
#include "screens/screen_list.h"
#include "screens/screen_device.h"
#include "screens/screen_info.h"
#include "screens/screen_automation.h"
#include "screens/screen_script.h"
#include "screens/screen_scene.h"
#include "utils/input.h"
#include "audio.h"
#include "utils/config.h"
#include "ha_client.h"
#include "database.h"
#include "cache_manager.h"

// Miyoo Mini Plus screen dimensions
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define FRAME_DELAY   16  // ~60 FPS (16.67ms)

// Application state
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int running;

    // Phase 2: API client and configuration
    app_config_t *config;
    ha_client_t *ha_client;

    // Phase 3: Database and cache
    database_t *db;
    cache_manager_t *cache_mgr;

    // Phase 4: UI system
    font_manager_t *fonts;
    icon_manager_t *icons;
    test_screen_t *test_screen;

    // Phase 5-9: Screens
    setup_screen_t *setup_screen;
    list_screen_t *list_screen;
    device_screen_t *device_screen;
    info_screen_t *info_screen;
    automation_screen_t *automation_screen;
    script_screen_t *script_screen;
    scene_screen_t *scene_screen;
    int current_screen;

    // Phase 11: Exit confirmation dialog
    int show_exit_dialog;

    // Phase 12: Background sync
    Uint32 last_sync_check;
} app_state_t;

// Screen IDs
#define SCREEN_SETUP      0
#define SCREEN_LIST       1
#define SCREEN_DEVICE     2
#define SCREEN_INFO       3
#define SCREEN_AUTOMATION 4
#define SCREEN_SCRIPT     5
#define SCREEN_SCENE      6
#define SCREEN_TEST       7

/**
 * Initialize SDL2 and create window/renderer
 */
static int init_sdl(app_state_t *app) {
    // CRITICAL: Set Miyoo double buffer flag BEFORE SDL_Init()
    // This MUST be done in C code, not shell script, for proper SDL initialization
    SDL_setenv("SDL_MMIYOO_DOUBLE_BUFFER", "1", 1);

    // Initialize SDL VIDEO first (required for MMIYOO driver)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init VIDEO failed: %s\n", SDL_GetError());
        return 0;
    }

    // Try to initialize audio separately (optional, may not be available)
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Warning: Audio init failed: %s (continuing without audio)\n", SDL_GetError());
        // Continue without audio - not a fatal error
    }

    // Initialize SDL_image (PNG support)
    int img_flags = IMG_INIT_PNG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        fprintf(stderr, "SDL_image init failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 0;
    }

    // Initialize SDL_ttf (font rendering)
    if (TTF_Init() < 0) {
        fprintf(stderr, "SDL_ttf init failed: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return 0;
    }

    // Create window
    // NOTE: Do NOT use SDL_WINDOW_FULLSCREEN on Miyoo - the MMIYOO driver handles this automatically
    app->window = SDL_CreateWindow(
        "HA Companion",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!app->window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 0;
    }

    // Create renderer with ACCELERATED | PRESENTVSYNC for Miyoo MMIYOO driver
    // XK9274's miyoo_sdl2_benchmarks confirms this is the correct pattern
    app->renderer = SDL_CreateRenderer(
        app->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!app->renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(app->window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 0;
    }

    // Disable texture filtering for pixel-perfect rendering
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    // Debug: Print SDL driver information
    SDL_RendererInfo renderer_info;
    if (SDL_GetRendererInfo(app->renderer, &renderer_info) == 0) {
        printf("SDL Renderer: %s\n", renderer_info.name);
        printf("  Flags: %u\n", renderer_info.flags);
        printf("  Texture formats: %u\n", renderer_info.num_texture_formats);
        printf("  Max texture: %dx%d\n", renderer_info.max_texture_width, renderer_info.max_texture_height);
    }

    const char *video_driver = SDL_GetCurrentVideoDriver();
    if (video_driver) {
        printf("SDL Video Driver: %s\n", video_driver);
    }

    printf("SDL2 initialized successfully\n");
    printf("Screen: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);

    return 1;
}

/**
 * Handle SDL events
 */
static void handle_events(app_state_t *app) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                app->running = 0;
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // Debug: Print key events
                if (event.type == SDL_KEYDOWN) {
                    printf("Key pressed: %d\n", event.key.keysym.sym);
                    // Play button press beep for debugging (proves app is running)
                    audio_play_button();
                }

                input_update(&event);

                // Handle exit dialog first (blocks other input)
                if (app->show_exit_dialog && event.type == SDL_KEYDOWN) {
                    if (input_button_pressed(BTN_A)) {
                        app->running = 0;
                    } else if (input_button_pressed(BTN_B)) {
                        app->show_exit_dialog = 0;
                    }
                    break;  // Don't process other input when dialog is shown
                }

                // Handle menu button (show exit dialog)
                if (event.type == SDL_KEYDOWN && input_button_pressed(BTN_MENU)) {
                    app->show_exit_dialog = 1;
                    break;
                }

                // Route input to current screen
                if (event.type == SDL_KEYDOWN) {
                    if (app->current_screen == SCREEN_SETUP && app->setup_screen) {
                        if (setup_screen_handle_input(app->setup_screen, &event)) {
                            // Switch to list screen
                            app->current_screen = SCREEN_LIST;
                        }
                    } else if (app->current_screen == SCREEN_LIST && app->list_screen) {
                        int result = list_screen_handle_input(app->list_screen, &event);
                        if (result == -1) {
                            // Back to setup
                            app->current_screen = SCREEN_SETUP;
                        } else if (result == 1) {
                            // Go to detail screen based on entity domain
                            ha_entity_t *entity = list_screen_get_selected_entity(app->list_screen);
                            if (entity) {
                                const char *eid = entity->entity_id;
                                if (strncmp(eid, "automation.", 11) == 0 && app->automation_screen) {
                                    automation_screen_set_entity(app->automation_screen, eid);
                                    app->current_screen = SCREEN_AUTOMATION;
                                } else if (strncmp(eid, "script.", 7) == 0 && app->script_screen) {
                                    script_screen_set_entity(app->script_screen, eid);
                                    app->current_screen = SCREEN_SCRIPT;
                                } else if (strncmp(eid, "scene.", 6) == 0 && app->scene_screen) {
                                    scene_screen_set_entity(app->scene_screen, eid);
                                    app->current_screen = SCREEN_SCENE;
                                } else if (info_screen_should_handle(eid) && app->info_screen) {
                                    // Generic read-only info for sensors, etc.
                                    info_screen_set_entity(app->info_screen, eid);
                                    app->current_screen = SCREEN_INFO;
                                } else if (app->device_screen) {
                                    // Controllable entities (light, switch, fan, etc.)
                                    device_screen_set_entity(app->device_screen, eid);
                                    app->current_screen = SCREEN_DEVICE;
                                }
                            }
                        }
                    } else if (app->current_screen == SCREEN_INFO && app->info_screen) {
                        int result = info_screen_handle_input(app->info_screen, &event);
                        if (result == -1) {
                            app->current_screen = SCREEN_LIST;
                        }
                    } else if (app->current_screen == SCREEN_AUTOMATION && app->automation_screen) {
                        int result = automation_screen_handle_input(app->automation_screen, &event);
                        if (result == -1) {
                            app->current_screen = SCREEN_LIST;
                            list_screen_refresh(app->list_screen);
                        }
                    } else if (app->current_screen == SCREEN_SCRIPT && app->script_screen) {
                        int result = script_screen_handle_input(app->script_screen, &event);
                        if (result == -1) {
                            app->current_screen = SCREEN_LIST;
                            list_screen_refresh(app->list_screen);
                        }
                    } else if (app->current_screen == SCREEN_SCENE && app->scene_screen) {
                        int result = scene_screen_handle_input(app->scene_screen, &event);
                        if (result == -1) {
                            app->current_screen = SCREEN_LIST;
                            list_screen_refresh(app->list_screen);
                        }
                    } else if (app->current_screen == SCREEN_DEVICE && app->device_screen) {
                        int result = device_screen_handle_input(app->device_screen, &event);
                        if (result == -1) {
                            // Back to list
                            app->current_screen = SCREEN_LIST;
                            list_screen_refresh(app->list_screen);
                        }
                    } else if (app->current_screen == SCREEN_TEST && app->test_screen) {
                        test_screen_handle_input(app->test_screen, &event);
                    }
                }

                break;
        }
    }
}

/**
 * Render exit confirmation dialog
 */
static void render_exit_dialog(app_state_t *app) {
    SDL_Renderer *r = app->renderer;
    TTF_Font *font_header = fonts_get(app->fonts, FONT_SIZE_HEADER);
    TTF_Font *font_body = fonts_get(app->fonts, FONT_SIZE_BODY);

    // Semi-transparent overlay
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(r, &overlay);

    // Dialog box
    int dialog_w = 320;
    int dialog_h = 140;
    int dialog_x = (SCREEN_WIDTH - dialog_w) / 2;
    int dialog_y = (SCREEN_HEIGHT - dialog_h) / 2;

    SDL_Rect dialog_bg = {dialog_x, dialog_y, dialog_w, dialog_h};
    ui_draw_bordered_rect(r, dialog_bg, COLOR_GB_LIGHT, COLOR_GB_DARKEST, 3);

    // Title
    ui_draw_text(r, font_header, "EXIT APP?",
                SCREEN_WIDTH / 2, dialog_y + 30, COLOR_GB_DARKEST, TEXT_ALIGN_CENTER);

    // Instructions
    ui_draw_text(r, font_body, "[A] Yes    [B] No",
                SCREEN_WIDTH / 2, dialog_y + 85, COLOR_GB_DARK, TEXT_ALIGN_CENTER);
}

/**
 * Render the current frame
 */
static void render(app_state_t *app) {
    static int frame_count = 0;

    // Debug: Print which screen we're rendering (first 10 frames only)
    if (frame_count < 10) {
        printf("Frame %d: Rendering screen %d\n", frame_count, app->current_screen);
        frame_count++;
    }

    // Clear screen to Game Boy darkest green
    set_render_color(app->renderer, COLOR_BACKGROUND);
    SDL_RenderClear(app->renderer);

    // Render current screen
    if (app->current_screen == SCREEN_SETUP && app->setup_screen) {
        setup_screen_render(app->setup_screen);
    } else if (app->current_screen == SCREEN_LIST && app->list_screen) {
        list_screen_render(app->list_screen);
    } else if (app->current_screen == SCREEN_DEVICE && app->device_screen) {
        device_screen_render(app->device_screen);
    } else if (app->current_screen == SCREEN_INFO && app->info_screen) {
        info_screen_render(app->info_screen);
    } else if (app->current_screen == SCREEN_AUTOMATION && app->automation_screen) {
        automation_screen_render(app->automation_screen);
    } else if (app->current_screen == SCREEN_SCRIPT && app->script_screen) {
        script_screen_render(app->script_screen);
    } else if (app->current_screen == SCREEN_SCENE && app->scene_screen) {
        scene_screen_render(app->scene_screen);
    } else if (app->current_screen == SCREEN_TEST && app->test_screen) {
        test_screen_render(app->test_screen);
    }

    // Render exit dialog overlay if active
    if (app->show_exit_dialog) {
        render_exit_dialog(app);
    }

    // Present the frame
    SDL_RenderPresent(app->renderer);
}

/**
 * Main application loop
 */
static void main_loop(app_state_t *app) {
    Uint32 frame_start;
    int frame_time;

    while (app->running) {
        frame_start = SDL_GetTicks();

        // Process input
        handle_events(app);

        // Phase 12: Background sync check every 60 seconds
        if (app->cache_mgr && (frame_start - app->last_sync_check > 60000)) {
            if (cache_manager_should_sync(app->cache_mgr)) {
                int synced = cache_manager_sync(app->cache_mgr);
                if (synced > 0 && app->list_screen) {
                    list_screen_refresh(app->list_screen);
                }
            }
            app->last_sync_check = frame_start;
        }

        // Render frame
        render(app);

        // Reset input state for next frame
        input_reset();

        // Frame rate limiting
        frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }
}

/**
 * Cleanup resources
 */
static void cleanup(app_state_t *app) {
    // Phase 5-9: Cleanup screens
    if (app->scene_screen) {
        scene_screen_destroy(app->scene_screen);
    }
    if (app->script_screen) {
        script_screen_destroy(app->script_screen);
    }
    if (app->automation_screen) {
        automation_screen_destroy(app->automation_screen);
    }
    if (app->info_screen) {
        info_screen_destroy(app->info_screen);
    }
    if (app->device_screen) {
        device_screen_destroy(app->device_screen);
    }
    if (app->list_screen) {
        list_screen_destroy(app->list_screen);
    }
    if (app->setup_screen) {
        setup_screen_destroy(app->setup_screen);
    }

    // Phase 4: Cleanup UI system
    if (app->test_screen) {
        test_screen_destroy(app->test_screen);
    }
    if (app->icons) {
        icons_destroy(app->icons);
    }
    if (app->fonts) {
        fonts_destroy(app->fonts);
    }

    // Phase 3: Cleanup cache manager and database
    if (app->cache_mgr) {
        cache_manager_destroy(app->cache_mgr);
    }
    if (app->db) {
        database_close(app->db);
    }

    // Phase 2: Cleanup API client and config
    if (app->ha_client) {
        ha_client_destroy(app->ha_client);
    }
    if (app->config) {
        config_free(app->config);
    }

    // Phase 1: Cleanup SDL
    if (app->renderer) {
        SDL_DestroyRenderer(app->renderer);
    }
    if (app->window) {
        SDL_DestroyWindow(app->window);
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    printf("Cleanup complete\n");
}

/**
 * Application entry point
 */
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    // Disable stdout buffering so we can see output immediately in debug.log
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    printf("Home Assistant Companion v1.0.0\n");
    printf("Miyoo Mini Plus Edition\n");
    printf("-------------------------------------------\n");

    app_state_t app = {0};
    app.running = 1;

    // Initialize SDL2
    if (!init_sdl(&app)) {
        fprintf(stderr, "Failed to initialize SDL2\n");
        return 1;
    }

    // Initialize audio system (for debugging - beeps tell us the app is running)
    printf("Initializing audio...\n");
    if (audio_init()) {
        printf("Audio initialized\n");
        // Play startup chime - if you hear this, the app is loaded!
        audio_play_startup();
    } else {
        printf("Warning: Audio initialization failed (continuing without sound)\n");
    }

    // Initialize input system
    input_init();

#if SKIP_NETWORK_TEST
    printf("=== SKIP_NETWORK_TEST MODE: Bypassing all network/database initialization ===\n");
    // Skip all network and database init - go straight to minimal rendering test
#else
    // Phase 3: Open database
    printf("Opening database...\n");
    app.db = database_open("hacompanion.db");
    if (!app.db) {
        fprintf(stderr, "Failed to open database\n");
        cleanup(&app);
        return 1;
    }
    if (!database_init_schema(app.db)) {
        fprintf(stderr, "Failed to initialize database schema\n");
        cleanup(&app);
        return 1;
    }
    printf("Database ready (cached entities: %d)\n", database_get_entity_count(app.db));

    // Phase 2: Load configuration
    printf("Loading configuration...\n");
    app.config = config_load("servers.json");
    if (!app.config) {
        printf("Warning: No servers.json found - offline mode\n");
    } else {
        printf("Loaded %d server(s)\n", app.config->server_count);

        // Create HA client with default server
        server_config_t *server = config_get_default_server(app.config);
        if (server) {
            printf("Connecting to: %s (%s:%d)\n", server->name, server->url, server->port);
            app.ha_client = ha_client_create(server->url, server->port, server->token);
            if (app.ha_client) {
                // Set SSL verification mode from config
                app.ha_client->insecure = server->insecure;
                if (server->insecure) {
                    printf("Warning: SSL certificate verification disabled\n");
                }
            }
        }
    }

    // Phase 3: Create cache manager
    app.cache_mgr = cache_manager_create(app.db, app.ha_client);
    if (!app.cache_mgr) {
        fprintf(stderr, "Failed to create cache manager\n");
        cleanup(&app);
        return 1;
    }

    // Initial sync if online
    if (app.ha_client) {
        int synced = cache_manager_sync(app.cache_mgr);
        if (synced > 0) {
            printf("Synced %d entities from Home Assistant\n", synced);
        } else if (synced == 0) {
            printf("Connected (no entities to sync)\n");
        } else {
            printf("Sync failed - using cached data\n");
        }
    }

    printf("Cached entities: %d\n", cache_manager_get_entity_count(app.cache_mgr));
#endif

#if SKIP_NETWORK_TEST
    // Minimal rendering test - no fonts, icons, or screens needed
    printf("Starting minimal rendering test loop...\n");
    printf("Should see: WHITE -> RED -> GREEN -> BLUE cycling every 60 frames\n");

    Uint32 frame_start;
    int frame_time;
    int frame_count = 0;
    const SDL_Color colors[] = {
        {255, 255, 255, 255},  // White
        {255, 0, 0, 255},      // Red
        {0, 255, 0, 255},      // Green
        {0, 0, 255, 255}       // Blue
    };

    while (app.running) {
        frame_start = SDL_GetTicks();

        // Handle input (just quit on MENU/ESC)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                app.running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    app.running = 0;
                }
            }
        }

        // Cycle through colors every 60 frames
        int color_index = (frame_count / 60) % 4;
        SDL_Color color = colors[color_index];

        // Fill screen with current color
        SDL_SetRenderDrawColor(app.renderer, color.r, color.g, color.b, color.a);
        SDL_RenderClear(app.renderer);
        SDL_RenderPresent(app.renderer);

        // Log every 60 frames
        if (frame_count % 60 == 0) {
            printf("Frame %d: Color %d (R=%d G=%d B=%d)\n", frame_count, color_index, color.r, color.g, color.b);
        }

        frame_count++;

        // Frame rate limiting to 60 FPS
        frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < 16) {
            SDL_Delay(16 - frame_time);
        }
    }

    printf("Test complete after %d frames\n", frame_count);

#else
    // Phase 4: Initialize UI system
    printf("Loading fonts...\n");
    app.fonts = fonts_init("assets/fonts/PressStart2P.ttf");
    if (!app.fonts) {
        fprintf(stderr, "Failed to load fonts\n");
        cleanup(&app);
        return 1;
    }

    printf("Initializing icons...\n");
    app.icons = icons_init(app.renderer, "assets/icons");
    if (!app.icons) {
        fprintf(stderr, "Failed to initialize icons\n");
        cleanup(&app);
        return 1;
    }

    printf("Creating test screen...\n");
    app.test_screen = test_screen_create(app.renderer, app.fonts, app.icons);
    if (!app.test_screen) {
        fprintf(stderr, "Failed to create test screen\n");
        cleanup(&app);
        return 1;
    }

    // Phase 5: Create setup screen
    printf("Creating setup screen...\n");
    app.setup_screen = setup_screen_create(app.renderer, app.fonts, app.icons,
                                            app.config, &app.ha_client);
    if (!app.setup_screen) {
        fprintf(stderr, "Failed to create setup screen\n");
        cleanup(&app);
        return 1;
    }

    // Phase 6: Create list screen
    printf("Creating list screen...\n");
    app.list_screen = list_screen_create(app.renderer, app.fonts, app.icons,
                                          app.cache_mgr, &app.ha_client);
    if (!app.list_screen) {
        fprintf(stderr, "Failed to create list screen\n");
        cleanup(&app);
        return 1;
    }

    // Phase 7: Create device screen
    printf("Creating device screen...\n");
    app.device_screen = device_screen_create(app.renderer, app.fonts, app.icons,
                                              app.cache_mgr, &app.ha_client);
    if (!app.device_screen) {
        fprintf(stderr, "Failed to create device screen\n");
        cleanup(&app);
        return 1;
    }

    // Phase 9: Create info screen (generic fallback)
    printf("Creating info screen...\n");
    app.info_screen = info_screen_create(app.renderer, app.fonts, app.icons,
                                          app.cache_mgr, &app.ha_client);
    if (!app.info_screen) {
        fprintf(stderr, "Failed to create info screen\n");
        cleanup(&app);
        return 1;
    }

    // Phase 9.1: Create automation screen
    printf("Creating automation screen...\n");
    app.automation_screen = automation_screen_create(app.renderer, app.fonts, app.icons,
                                                      app.cache_mgr, &app.ha_client);
    if (!app.automation_screen) {
        fprintf(stderr, "Failed to create automation screen\n");
        cleanup(&app);
        return 1;
    }

    // Phase 9.1: Create script screen
    printf("Creating script screen...\n");
    app.script_screen = script_screen_create(app.renderer, app.fonts, app.icons,
                                              app.cache_mgr, &app.ha_client);
    if (!app.script_screen) {
        fprintf(stderr, "Failed to create script screen\n");
        cleanup(&app);
        return 1;
    }

    // Phase 9.1: Create scene screen
    printf("Creating scene screen...\n");
    app.scene_screen = scene_screen_create(app.renderer, app.fonts, app.icons,
                                            app.cache_mgr, &app.ha_client);
    if (!app.scene_screen) {
        fprintf(stderr, "Failed to create scene screen\n");
        cleanup(&app);
        return 1;
    }

    // Start on list screen (skip setup since we already connected during init)
    app.current_screen = SCREEN_LIST;

    printf("\nReady! Press Menu/Escape to exit\n");
    printf("Setup: D-Pad=Select, A=Connect, START=Continue\n");
    printf("List: L1/R1=Tabs, D-Pad=Navigate, A=Toggle, SEL=Detail, Y=Favorite\n");
    printf("Detail: D-Pad=Select, A=Action, Y=Favorite, B=Back\n\n");

    // Run main loop
    main_loop(&app);
#endif

    // Cleanup and exit
    cleanup(&app);

    return 0;
}
