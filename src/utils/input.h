/**
 * input.h - Input handling system for Miyoo Mini Plus controller
 *
 * Maps Miyoo controller buttons to SDL2 keyboard events and provides
 * button state tracking with repeat handling.
 *
 * Controller Layout (Nintendo defaults on Miyoo):
 * - D-Pad: Navigate menus (up/down/left/right)
 * - A Button: Confirm / Toggle / Activate
 * - B Button: Back / Cancel
 * - X Button: Filter / Sort
 * - Y Button: Alternative actions
 * - L1/R1: Tab navigation
 * - L2/R2: Reserved for future features
 * - Select: View details / Enter entity detail screen
 * - Start: Quick menu / Refresh
 * - Menu: Exit confirmation dialog
 */

#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>

/**
 * Miyoo button enumeration
 */
typedef enum {
    BTN_A,
    BTN_B,
    BTN_X,
    BTN_Y,
    BTN_START,
    BTN_SELECT,
    BTN_MENU,
    BTN_L1,
    BTN_R1,
    BTN_L2,
    BTN_R2,
    BTN_DPAD_UP,
    BTN_DPAD_DOWN,
    BTN_DPAD_LEFT,
    BTN_DPAD_RIGHT,
    BTN_COUNT  // Total number of buttons
} button_t;

/**
 * Initialize the input system
 * Call once at application startup
 */
void input_init(void);

/**
 * Update input state based on SDL event
 * Call this for each SDL keyboard event in the event loop
 *
 * @param event SDL_Event pointer (should be SDL_KEYDOWN or SDL_KEYUP)
 */
void input_update(SDL_Event *event);

/**
 * Check if a button was just pressed (single frame)
 * Returns true only on the frame the button was first pressed
 *
 * @param button Button to check
 * @return 1 if button was just pressed, 0 otherwise
 */
int input_button_pressed(button_t button);

/**
 * Check if a button is currently held down
 * Returns true for all frames while button is held
 *
 * @param button Button to check
 * @return 1 if button is currently down, 0 otherwise
 */
int input_button_down(button_t button);

/**
 * Check if a button should trigger a repeat action
 * Implements key repeat with initial delay and repeat rate
 * Used for scrolling through lists with held D-pad
 *
 * @param button Button to check
 * @return 1 if button should repeat, 0 otherwise
 */
int input_button_repeat(button_t button);

/**
 * Reset all button states
 * Call at the end of each frame
 */
void input_reset(void);

#endif // INPUT_H
