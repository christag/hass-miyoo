/**
 * icons.c - Icon System Implementation
 */

#include "icons.h"
#include "colors.h"
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================
 * Built-in Icon Pixel Data (16x16)
 * 0 = transparent, 1 = GB_LIGHTEST, 2 = GB_LIGHT
 * ============================================ */

// Light bulb icon
static const uint8_t ICON_LIGHT_BULB[256] = {
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,1,2,2,2,2,1,0,0,0,0,0,
    0,0,0,0,1,2,2,2,2,2,2,1,0,0,0,0,
    0,0,0,1,2,2,2,2,2,2,2,2,1,0,0,0,
    0,0,0,1,2,2,2,2,2,2,2,2,1,0,0,0,
    0,0,0,1,2,2,2,2,2,2,2,2,1,0,0,0,
    0,0,0,0,1,2,2,2,2,2,2,1,0,0,0,0,
    0,0,0,0,0,1,2,2,2,2,1,0,0,0,0,0,
    0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,1,2,2,2,2,1,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// Switch/toggle icon
static const uint8_t ICON_SWITCH[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,1,2,2,2,2,2,2,2,2,2,2,1,0,0,
    0,0,1,2,1,1,1,2,2,2,2,2,2,1,0,0,
    0,0,1,2,1,1,1,2,2,2,2,2,2,1,0,0,
    0,0,1,2,1,1,1,2,2,2,2,2,2,1,0,0,
    0,0,1,2,1,1,1,2,2,2,2,2,2,1,0,0,
    0,0,1,2,2,2,2,2,2,2,2,2,2,1,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// Thermometer icon
static const uint8_t ICON_CLIMATE[256] = {
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,1,2,2,2,1,0,0,0,0,0,0,
    0,0,0,0,0,1,2,1,2,1,0,0,0,0,0,0,
    0,0,0,0,0,1,2,2,2,1,0,0,0,0,0,0,
    0,0,0,0,0,1,2,1,2,1,0,0,0,0,0,0,
    0,0,0,0,0,1,2,2,2,1,0,0,0,0,0,0,
    0,0,0,0,0,1,2,1,2,1,0,0,0,0,0,0,
    0,0,0,0,0,1,2,2,2,1,0,0,0,0,0,0,
    0,0,0,0,1,2,2,2,2,2,1,0,0,0,0,0,
    0,0,0,1,2,2,2,2,2,2,2,1,0,0,0,0,
    0,0,0,1,2,2,1,1,1,2,2,1,0,0,0,0,
    0,0,0,1,2,2,1,1,1,2,2,1,0,0,0,0,
    0,0,0,1,2,2,2,2,2,2,2,1,0,0,0,0,
    0,0,0,0,1,2,2,2,2,2,1,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// Sensor icon (wave/signal)
static const uint8_t ICON_SENSOR[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,
    0,0,0,0,0,1,2,2,2,2,1,0,0,0,0,0,
    0,0,0,0,1,2,2,1,1,2,2,1,0,0,0,0,
    0,0,0,1,2,2,1,0,0,1,2,2,1,0,0,0,
    0,0,1,2,2,1,0,0,0,0,1,2,2,1,0,0,
    0,0,1,2,1,0,0,1,1,0,0,1,2,1,0,0,
    0,0,1,2,1,0,0,1,1,0,0,1,2,1,0,0,
    0,0,1,2,2,1,0,0,0,0,1,2,2,1,0,0,
    0,0,0,1,2,2,1,0,0,1,2,2,1,0,0,0,
    0,0,0,0,1,2,2,1,1,2,2,1,0,0,0,0,
    0,0,0,0,0,1,2,2,2,2,1,0,0,0,0,0,
    0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// Automation robot icon
static const uint8_t ICON_AUTOMATION[256] = {
    0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,2,2,2,2,2,2,2,2,1,0,0,0,
    0,0,0,1,2,1,1,2,2,1,1,2,1,0,0,0,
    0,0,0,1,2,1,1,2,2,1,1,2,1,0,0,0,
    0,0,0,1,2,2,2,2,2,2,2,2,1,0,0,0,
    0,0,0,1,2,1,1,1,1,1,1,2,1,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,1,2,2,2,2,2,2,2,2,2,2,1,0,0,
    0,0,1,2,2,2,2,2,2,2,2,2,2,1,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,
    0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,
    0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// Script icon (code brackets)
static const uint8_t ICON_SCRIPT[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,1,2,2,0,0,0,0,0,0,0,0,
    0,0,0,0,1,2,2,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,2,2,0,0,0,0,0,0,0,0,0,
    0,0,0,1,2,2,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,2,2,0,0,0,0,0,0,0,0,0,0,
    0,0,1,2,2,0,0,0,0,0,0,0,1,2,1,0,
    0,0,1,2,2,0,0,0,0,0,0,0,1,2,1,0,
    0,0,0,1,2,2,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,2,2,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,2,2,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,2,2,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,2,2,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// Scene stars icon
static const uint8_t ICON_SCENE[256] = {
    0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,0,
    0,0,0,1,1,1,2,2,2,1,1,1,0,0,0,0,
    0,0,0,0,1,2,2,2,2,2,1,0,0,0,0,0,
    0,0,0,0,0,1,2,2,2,1,0,0,0,0,0,0,
    0,0,0,0,1,2,1,1,1,2,1,0,0,0,0,0,
    0,0,0,1,2,1,0,0,0,1,2,1,0,0,0,0,
    0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    0,1,0,0,0,0,0,0,0,0,0,0,1,2,1,0,
    1,2,1,0,0,0,0,0,0,0,0,0,0,1,0,0,
    0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// Star filled (favorite)
static const uint8_t ICON_STAR_FILLED[256] = {
    0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,2,1,1,1,1,1,1,0,0,
    0,0,1,2,2,2,2,2,2,2,2,2,1,0,0,0,
    0,0,0,1,2,2,2,2,2,2,2,1,0,0,0,0,
    0,0,0,0,1,2,2,2,2,2,1,0,0,0,0,0,
    0,0,0,0,1,2,2,2,2,2,1,0,0,0,0,0,
    0,0,0,1,2,2,2,2,2,2,2,1,0,0,0,0,
    0,0,1,2,2,2,1,1,1,2,2,2,1,0,0,0,
    0,0,1,2,2,1,0,0,0,1,2,2,1,0,0,0,
    0,0,1,2,1,0,0,0,0,0,1,2,1,0,0,0,
    0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// Star empty (not favorite)
static const uint8_t ICON_STAR_EMPTY[256] = {
    0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,0,
    0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,
    0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
    0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,
    0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,
    0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
    0,0,1,0,0,0,1,1,1,0,0,0,1,0,0,0,
    0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,
    0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,
    0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// WiFi on icon
static const uint8_t ICON_WIFI_ON[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,
    0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,
    0,1,1,0,0,0,1,1,1,1,0,0,0,1,1,0,
    0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,
    0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// WiFi off icon
static const uint8_t ICON_WIFI_OFF[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,1,0,0,1,0,0,
    0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,
    0,0,1,1,0,0,0,0,0,0,0,1,1,1,0,0,
    0,1,1,0,0,0,1,1,1,1,1,0,1,1,1,0,
    0,0,0,0,0,1,1,0,1,1,1,0,0,1,0,0,
    0,0,0,0,1,1,0,1,0,1,1,1,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,1,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// Generic fallback icon
static const uint8_t ICON_GENERIC[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,1,2,2,2,2,2,2,2,2,2,2,1,0,0,
    0,1,2,2,2,2,2,2,2,2,2,2,2,2,1,0,
    0,1,2,2,1,1,1,1,1,1,1,1,2,2,1,0,
    0,1,2,2,1,0,0,0,0,0,0,1,2,2,1,0,
    0,1,2,2,1,0,0,0,0,0,0,1,2,2,1,0,
    0,1,2,2,1,0,0,0,0,0,0,1,2,2,1,0,
    0,1,2,2,1,0,0,0,0,0,0,1,2,2,1,0,
    0,1,2,2,1,0,0,0,0,0,0,1,2,2,1,0,
    0,1,2,2,1,0,0,0,0,0,0,1,2,2,1,0,
    0,1,2,2,1,1,1,1,1,1,1,1,2,2,1,0,
    0,1,2,2,2,2,2,2,2,2,2,2,2,2,1,0,
    0,0,1,2,2,2,2,2,2,2,2,2,2,1,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

/* ============================================
 * Helper Functions
 * ============================================ */

static SDL_Texture* create_icon_texture(SDL_Renderer *renderer, const uint8_t *data) {
    // Create surface
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
        0, 16, 16, 32, SDL_PIXELFORMAT_RGBA8888
    );
    if (!surface) {
        return NULL;
    }

    // Define palette
    SDL_Color palette[3] = {
        {0, 0, 0, 0},           // 0 = Transparent
        COLOR_GB_LIGHTEST,      // 1 = Light
        COLOR_GB_LIGHT          // 2 = Medium
    };

    // Fill pixels
    uint32_t *pixels = (uint32_t *)surface->pixels;
    for (int i = 0; i < 256; i++) {
        uint8_t idx = data[i];
        if (idx < 3) {
            SDL_Color c = palette[idx];
            pixels[i] = SDL_MapRGBA(surface->format, c.r, c.g, c.b, c.a);
        } else {
            pixels[i] = 0; // Transparent
        }
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    return texture;
}

static void cache_icon(icon_manager_t *icons, const char *name, SDL_Texture *texture) {
    if (icons->icon_count >= MAX_ICONS) {
        return;
    }

    strncpy(icons->icons[icons->icon_count].name, name, 31);
    icons->icons[icons->icon_count].name[31] = '\0';
    icons->icons[icons->icon_count].texture = texture;
    icons->icons[icons->icon_count].width = 16;
    icons->icons[icons->icon_count].height = 16;
    icons->icon_count++;
}

/* ============================================
 * Public Functions
 * ============================================ */

icon_manager_t* icons_init(SDL_Renderer *renderer, const char *base_path) {
    if (!renderer) {
        return NULL;
    }

    icon_manager_t *icons = calloc(1, sizeof(icon_manager_t));
    if (!icons) {
        return NULL;
    }

    icons->renderer = renderer;
    icons->icon_count = 0;

    if (base_path) {
        strncpy(icons->base_path, base_path, sizeof(icons->base_path) - 1);
    } else {
        strcpy(icons->base_path, "assets/icons");
    }

    // Generate built-in icons
    icons_generate_builtin(icons);

    printf("Icon system initialized (%d built-in icons)\n", icons->icon_count);

    return icons;
}

void icons_destroy(icon_manager_t *icons) {
    if (!icons) return;

    for (int i = 0; i < icons->icon_count; i++) {
        if (icons->icons[i].texture) {
            SDL_DestroyTexture(icons->icons[i].texture);
        }
    }

    free(icons);
}

SDL_Texture* icons_get(icon_manager_t *icons, const char *name) {
    if (!icons || !name) return NULL;

    // Check cache first
    for (int i = 0; i < icons->icon_count; i++) {
        if (strcmp(icons->icons[i].name, name) == 0) {
            return icons->icons[i].texture;
        }
    }

    // Try to load from file
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.png", icons->base_path, name);

    SDL_Surface *surface = IMG_Load(path);
    if (surface) {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(icons->renderer, surface);
        SDL_FreeSurface(surface);

        if (texture) {
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            cache_icon(icons, name, texture);
            return texture;
        }
    }

    // Return generic icon as fallback
    return icons_get(icons, "generic");
}

void icons_draw(icon_manager_t *icons, const char *name, int x, int y, int size) {
    SDL_Texture *texture = icons_get(icons, name);
    if (!texture) return;

    SDL_Rect dest = {x, y, size, size};
    SDL_RenderCopy(icons->renderer, texture, NULL, &dest);
}

const char* icons_get_for_domain(const char *entity_id) {
    if (!entity_id) return "generic";

    // Extract domain from entity_id (before '.')
    const char *dot = strchr(entity_id, '.');
    if (!dot) return "generic";

    size_t len = dot - entity_id;
    if (len >= 32) return "generic";

    // Match domain to icon
    if (strncmp(entity_id, "light", len) == 0) return "light_bulb";
    if (strncmp(entity_id, "switch", len) == 0) return "switch_toggle";
    if (strncmp(entity_id, "climate", len) == 0) return "climate_thermo";
    if (strncmp(entity_id, "sensor", len) == 0) return "sensor_generic";
    if (strncmp(entity_id, "binary_sensor", len) == 0) return "sensor_generic";
    if (strncmp(entity_id, "cover", len) == 0) return "generic";
    if (strncmp(entity_id, "fan", len) == 0) return "generic";
    if (strncmp(entity_id, "lock", len) == 0) return "generic";
    if (strncmp(entity_id, "media_player", len) == 0) return "generic";
    if (strncmp(entity_id, "camera", len) == 0) return "generic";
    if (strncmp(entity_id, "automation", len) == 0) return "automation_robot";
    if (strncmp(entity_id, "script", len) == 0) return "script_code";
    if (strncmp(entity_id, "scene", len) == 0) return "scene_stars";

    return "generic";
}

const char* icons_get_for_state(const char *entity_id, const char *state) {
    // For now, just return domain icon
    // Future: could return state-specific icons (e.g., light_on/light_off)
    (void)state;
    return icons_get_for_domain(entity_id);
}

void icons_preload(icon_manager_t *icons) {
    // Built-in icons are already loaded in icons_generate_builtin
    // This could be used to pre-load additional PNG icons
    (void)icons;
}

void icons_generate_builtin(icon_manager_t *icons) {
    if (!icons || !icons->renderer) return;

    // Generate all built-in icons
    struct {
        const char *name;
        const uint8_t *data;
    } builtins[] = {
        {"light_bulb", ICON_LIGHT_BULB},
        {"switch_toggle", ICON_SWITCH},
        {"climate_thermo", ICON_CLIMATE},
        {"sensor_generic", ICON_SENSOR},
        {"automation_robot", ICON_AUTOMATION},
        {"script_code", ICON_SCRIPT},
        {"scene_stars", ICON_SCENE},
        {"star_filled", ICON_STAR_FILLED},
        {"star_empty", ICON_STAR_EMPTY},
        {"wifi_on", ICON_WIFI_ON},
        {"wifi_off", ICON_WIFI_OFF},
        {"generic", ICON_GENERIC},
        {NULL, NULL}
    };

    for (int i = 0; builtins[i].name != NULL; i++) {
        SDL_Texture *texture = create_icon_texture(icons->renderer, builtins[i].data);
        if (texture) {
            cache_icon(icons, builtins[i].name, texture);
        }
    }
}
