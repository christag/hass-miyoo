/**
 * input.c - Input handling implementation
 */

#include "input.h"
#include <string.h>

// SDL2 key code mappings for Miyoo Mini Plus (OnionOS)
#define MIYOO_BUTTON_A      SDLK_SPACE
#define MIYOO_BUTTON_B      SDLK_LCTRL
#define MIYOO_BUTTON_X      SDLK_LSHIFT
#define MIYOO_BUTTON_Y      SDLK_LALT
#define MIYOO_BUTTON_START  SDLK_RETURN
#define MIYOO_BUTTON_SELECT SDLK_RCTRL
#define MIYOO_BUTTON_MENU   SDLK_ESCAPE
#define MIYOO_BUTTON_L1     SDLK_e
#define MIYOO_BUTTON_R1     SDLK_t
#define MIYOO_BUTTON_L2     SDLK_TAB
#define MIYOO_BUTTON_R2     SDLK_BACKSPACE
#define MIYOO_DPAD_UP       SDLK_UP
#define MIYOO_DPAD_DOWN     SDLK_DOWN
#define MIYOO_DPAD_LEFT     SDLK_LEFT
#define MIYOO_DPAD_RIGHT    SDLK_RIGHT

// Key repeat timing (in milliseconds)
#define KEY_REPEAT_DELAY  300  // Initial delay before repeat starts
#define KEY_REPEAT_RATE   50   // Repeat interval once started

/**
 * Button state structure
 */
typedef struct {
    int current;        // Current frame state (1 = down, 0 = up)
    int previous;       // Previous frame state
    Uint32 press_time;  // SDL tick time when button was first pressed
    Uint32 last_repeat; // SDL tick time of last repeat trigger
} button_state_t;

// Button state array
static button_state_t button_states[BTN_COUNT];

// SDL keycode to button mapping table
static const SDL_Keycode keycode_map[BTN_COUNT] = {
    [BTN_A]          = MIYOO_BUTTON_A,
    [BTN_B]          = MIYOO_BUTTON_B,
    [BTN_X]          = MIYOO_BUTTON_X,
    [BTN_Y]          = MIYOO_BUTTON_Y,
    [BTN_START]      = MIYOO_BUTTON_START,
    [BTN_SELECT]     = MIYOO_BUTTON_SELECT,
    [BTN_MENU]       = MIYOO_BUTTON_MENU,
    [BTN_L1]         = MIYOO_BUTTON_L1,
    [BTN_R1]         = MIYOO_BUTTON_R1,
    [BTN_L2]         = MIYOO_BUTTON_L2,
    [BTN_R2]         = MIYOO_BUTTON_R2,
    [BTN_DPAD_UP]    = MIYOO_DPAD_UP,
    [BTN_DPAD_DOWN]  = MIYOO_DPAD_DOWN,
    [BTN_DPAD_LEFT]  = MIYOO_DPAD_LEFT,
    [BTN_DPAD_RIGHT] = MIYOO_DPAD_RIGHT,
};

/**
 * Find button enum from SDL keycode
 */
static button_t keycode_to_button(SDL_Keycode key) {
    for (int i = 0; i < BTN_COUNT; i++) {
        if (keycode_map[i] == key) {
            return (button_t)i;
        }
    }
    return BTN_COUNT; // Invalid button
}

void input_init(void) {
    memset(button_states, 0, sizeof(button_states));
}

void input_update(SDL_Event *event) {
    if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP) {
        return;
    }

    button_t btn = keycode_to_button(event->key.keysym.sym);
    if (btn >= BTN_COUNT) {
        return; // Unknown key
    }

    button_state_t *state = &button_states[btn];
    state->previous = state->current;

    if (event->type == SDL_KEYDOWN) {
        if (!state->current) {
            // Button just pressed
            state->current = 1;
            state->press_time = SDL_GetTicks();
            state->last_repeat = 0;
        }
    } else {
        // Button released
        state->current = 0;
        state->press_time = 0;
        state->last_repeat = 0;
    }
}

int input_button_pressed(button_t button) {
    if (button >= BTN_COUNT) {
        return 0;
    }

    button_state_t *state = &button_states[button];
    return state->current && !state->previous;
}

int input_button_down(button_t button) {
    if (button >= BTN_COUNT) {
        return 0;
    }

    return button_states[button].current;
}

int input_button_repeat(button_t button) {
    if (button >= BTN_COUNT) {
        return 0;
    }

    button_state_t *state = &button_states[button];

    if (!state->current) {
        return 0; // Button not held
    }

    Uint32 now = SDL_GetTicks();
    Uint32 held_time = now - state->press_time;

    // Initial press always returns true
    if (input_button_pressed(button)) {
        return 1;
    }

    // Check if we've passed the initial delay
    if (held_time < KEY_REPEAT_DELAY) {
        return 0;
    }

    // Check if enough time has passed since last repeat
    if (state->last_repeat == 0) {
        // First repeat after delay
        state->last_repeat = now;
        return 1;
    }

    if (now - state->last_repeat >= KEY_REPEAT_RATE) {
        state->last_repeat = now;
        return 1;
    }

    return 0;
}

void input_reset(void) {
    for (int i = 0; i < BTN_COUNT; i++) {
        button_states[i].previous = button_states[i].current;
    }
}
