# Issue: No Sound / Audio Initialization Failure

## Problem

Audio initialization fails on Miyoo Mini Plus hardware with MI (Miyoo Interface) errors. The app continues to run without sound.

## Expected Behavior

Audio should initialize successfully and play sound effects (button presses, notifications, etc.) through the Miyoo's built-in speaker.

## Environment

- **Device**: Miyoo Mini Plus running OnionOS
- **Audio Hardware**: Built-in speaker
- **SDL Audio**: Attempting to use SDL2 audio subsystem
- **Driver**: MMIYOO audio driver

## Error Messages

From `debug.log`:

```
Initializing audio...
MMIYOO_OpenDevice, freq:22050 sample:512 channels:1
[MI ERR ]: MI_AO_SetPubAttr[3364]: Dev0 failed to set pub attr!!! error number:0xa0052009!!!
MMIYOO_OpenDevice, failed to set PubAttr
[MI ERR ]: MI_AO_DisableChn[3667]: Dev0 has not been enabled.
Failed to open audio device:
Warning: Audio initialization failed (continuing without sound)
```

## Error Analysis

### MI_AO_SetPubAttr Failure
- **Error Code**: `0xa0052009`
- **Function**: `MI_AO_SetPubAttr[3364]`
- **Device**: Dev0 (primary audio output device)
- **Message**: "failed to set pub attr" (public attributes)

This error occurs when trying to configure the Miyoo's audio output device (AO = Audio Output) through the MI (Miyoo Interface) driver.

### Audio Parameters

The app is requesting:
- **Frequency**: 22050 Hz (22.05 kHz sample rate)
- **Buffer Size**: 512 samples
- **Channels**: 1 (mono)

These parameters may not match what the Miyoo hardware/driver expects.

## Current Status: NOT YET DEBUGGED

**Severity**: Low - App works without sound, but audio feedback would enhance UX

**Priority**: Medium - Should fix after black screen issue is resolved

**Blocked By**: Black screen issue (can't test audio until we have visual output)

## Known Information

### What We Know
1. Audio initialization is attempted but fails gracefully
2. App continues running without sound (non-fatal error)
3. Error is at the MI driver level (Miyoo Interface), not SDL level
4. The MMIYOO audio driver is being invoked correctly
5. Error code 0xa0052009 suggests parameter/configuration mismatch

### What We DON'T Know
1. What audio parameters the Miyoo hardware expects
2. Whether audio device is already in use by OnionOS
3. If audio device needs to be "claimed" like the display
4. What the error code 0xa0052009 specifically means
5. If other Miyoo SDL2 apps have audio working (need to research)

## Research Needed

Before attempting fixes, research:

1. **Working Miyoo SDL2 Apps with Audio**:
   - How does Moonlight Miyoo handle audio?
   - What parameters do successful apps use?
   - Do they use SDL audio or direct MI driver access?

2. **Miyoo Audio Hardware Specs**:
   - Supported sample rates (44100? 48000? 22050?)
   - Mono vs stereo support
   - Buffer size requirements
   - Audio format (S16? U8?)

3. **OnionOS Audio Management**:
   - Does OnionOS lock the audio device?
   - Is there an audio equivalent of pressMenu2Kill?
   - Do apps need special permissions?

4. **MI Driver Documentation**:
   - What does error 0xa0052009 mean?
   - What are valid MI_AO_SetPubAttr parameters?
   - Is there MI driver documentation available?

5. **SDL2 Audio Backend**:
   - Is SDL2 using the correct audio driver for Miyoo?
   - Should we set SDL_AUDIODRIVER environment variable?
   - Does MMIYOO driver support audio or just video?

## Possible Causes

### Hypothesis 1: Wrong Audio Parameters
**Theory**: The Miyoo hardware doesn't support 22050 Hz mono audio

**Evidence**:
- Error is "failed to set pub attr" (configuration rejected)
- May need 44100 Hz or 48000 Hz
- May need stereo (2 channels) instead of mono

**How to Test**:
1. Research working Miyoo apps' audio config
2. Try different sample rates (44100, 48000)
3. Try stereo (channels=2) instead of mono

### Hypothesis 2: Audio Device Already in Use
**Theory**: OnionOS or another process has the audio device locked

**Evidence**:
- Similar to display issue (but that wasn't actually the problem)
- "Dev0 has not been enabled" suggests device state issue

**How to Test**:
1. Check for audio equivalent of pressMenu2Kill
2. Check if audio device is open: `lsof | grep audio` over SSH
3. Try closing OnionOS audio processes before launch

### Hypothesis 3: Missing Audio Initialization Sequence
**Theory**: MI driver requires specific initialization order

**Evidence**:
- "Dev0 has not been enabled" error
- May need to enable device before setting attributes

**How to Test**:
1. Research MI audio API initialization sequence
2. Check if we need to call MI_AO_Enable before MI_AO_SetPubAttr
3. Look at working Miyoo app source code

### Hypothesis 4: SDL Audio Driver Mismatch
**Theory**: SDL2 is using wrong audio backend for Miyoo

**Evidence**:
- MMIYOO driver is primarily for video
- May need different audio driver (ALSA? OSS?)

**How to Test**:
1. Try setting `SDL_AUDIODRIVER=alsa` or `SDL_AUDIODRIVER=dsp`
2. Check what audio drivers vanilla SDL2 supports on ARM
3. Verify MMIYOO driver includes audio support

### Hypothesis 5: Unsupported Audio Format
**Theory**: Miyoo hardware doesn't support the requested audio format

**Evidence**:
- We're requesting AUDIO_S16SYS (likely)
- Miyoo might only support specific formats

**How to Test**:
1. Try different SDL_AudioSpec formats (U8, S16, S32)
2. Check Miyoo hardware audio capabilities
3. Test with minimal audio config

## Attempted Solutions

### Attempt 0: Initial Implementation (FAILED)
- **Date**: 2025-11 (MVP development)
- **Commit**: (TBD - find in git history)
- **Description**: Basic SDL audio initialization with 22050 Hz mono
- **Outcome**: FAILED - MI_AO_SetPubAttr error 0xa0052009
- **Logs**: See error messages above
- **Next**: Need to research before trying fixes

## Next Steps to Try

**IMPORTANT**: Do NOT attempt fixes until black screen issue is resolved. We need visual output to properly debug audio.

### Step 1: Research Phase (REQUIRED FIRST)
1. Find working Miyoo SDL2 apps with audio
2. Check their source code for audio parameters
3. Research MI driver documentation
4. Document findings in this issue file

### Step 2: Parameter Experimentation
Once research is complete:
1. Try sample rate 44100 Hz (CD quality)
2. Try stereo (channels=2)
3. Try buffer size 1024 or 2048
4. Document each attempt's outcome

### Step 3: Driver Experimentation
If parameters don't help:
1. Try SDL_AUDIODRIVER environment variable
2. Test ALSA vs OSS vs default
3. Check if MMIYOO driver handles audio at all

### Step 4: Direct MI Driver Access
If SDL audio doesn't work:
1. Consider bypassing SDL audio
2. Use MI driver directly (if documented)
3. Look at how OnionOS apps handle audio

## Related Code

- **File**: `src/audio.c` (TBD - need to locate audio init code)
- **Function**: Audio initialization (need to find function name)
- **SDL Call**: `SDL_OpenAudioDevice()` or similar

## Related Issues

- **Black Screen Issue**: Must be fixed first before we can properly test audio
- **SDL Version Issue**: Different SDL2 builds may have different audio support

## Lessons Learned

(None yet - issue not yet investigated)

## Resolution

**TBD** - Blocked by black screen issue. Will investigate once we have visual output working.

## Testing Plan

Once we attempt fixes:

1. **Verify audio device exists**: SSH in and check `/dev/dsp*` or `/proc/asound/`
2. **Test with simple tone**: Generate 440 Hz sine wave to verify hardware
3. **Check OnionOS audio**: Verify Miyoo speaker works with other apps
4. **Enable verbose logging**: Add more debug output around audio init
5. **Test different configs**: Systematically try parameter combinations

## Priority

**Current**: Low (blocked by black screen issue)
**After black screen fixed**: Medium (audio enhances UX but isn't critical for core functionality)
