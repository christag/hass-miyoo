#ifndef AUDIO_H
#define AUDIO_H

#include <SDL.h>

/**
 * Audio system for debugging and user feedback
 * Generates simple beep tones programmatically
 */

// Initialize audio system
int audio_init(void);

// Cleanup audio system
void audio_cleanup(void);

// Play a startup chime (high tone)
void audio_play_startup(void);

// Play a button press beep (mid tone)
void audio_play_button(void);

// Play an error beep (low tone)
void audio_play_error(void);

#endif // AUDIO_H
