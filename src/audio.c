#include "audio.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define SAMPLE_RATE 22050
#define AMPLITUDE 28000
#define M_PI 3.14159265358979323846

// Audio device ID
static SDL_AudioDeviceID audio_device = 0;

// Audio callback data
typedef struct {
    double frequency;
    int samples_left;
    int sample_index;
} tone_data_t;

static tone_data_t current_tone = {0};

/**
 * Audio callback - generates sine wave tones
 */
static void audio_callback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    Sint16 *buffer = (Sint16 *)stream;
    int samples = len / 2;  // 16-bit samples

    for (int i = 0; i < samples; i++) {
        if (current_tone.samples_left > 0) {
            double time = (double)current_tone.sample_index / SAMPLE_RATE;
            double value = sin(2.0 * M_PI * current_tone.frequency * time) * AMPLITUDE;
            buffer[i] = (Sint16)value;
            current_tone.sample_index++;
            current_tone.samples_left--;
        } else {
            buffer[i] = 0;  // Silence
        }
    }
}

/**
 * Initialize audio system
 */
int audio_init(void) {
    SDL_AudioSpec wanted, obtained;

    SDL_zero(wanted);
    wanted.freq = SAMPLE_RATE;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = 1;
    wanted.samples = 512;
    wanted.callback = audio_callback;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &wanted, &obtained, 0);
    if (audio_device == 0) {
        fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
        return 0;
    }

    // Start audio playback
    SDL_PauseAudioDevice(audio_device, 0);

    printf("Audio initialized: %d Hz, %d channels\n", obtained.freq, obtained.channels);
    return 1;
}

/**
 * Cleanup audio system
 */
void audio_cleanup(void) {
    if (audio_device != 0) {
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }
}

/**
 * Play a tone
 */
static void play_tone(double frequency, int duration_ms) {
    if (audio_device == 0) {
        return;  // Audio not initialized
    }

    SDL_LockAudioDevice(audio_device);
    current_tone.frequency = frequency;
    current_tone.samples_left = (SAMPLE_RATE * duration_ms) / 1000;
    current_tone.sample_index = 0;
    SDL_UnlockAudioDevice(audio_device);
}

/**
 * Play a startup chime (ascending tones)
 */
void audio_play_startup(void) {
    printf("BEEP! Startup chime\n");
    play_tone(523.25, 100);  // C5
    SDL_Delay(120);
    play_tone(659.25, 100);  // E5
    SDL_Delay(120);
    play_tone(783.99, 150);  // G5
}

/**
 * Play a button press beep
 */
void audio_play_button(void) {
    play_tone(440.0, 50);  // A4 - short beep
}

/**
 * Play an error beep
 */
void audio_play_error(void) {
    play_tone(220.0, 200);  // A3 - low tone
}
