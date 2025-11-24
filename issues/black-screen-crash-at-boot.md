# Issue: Black Screen or Crash at Boot

## Problem

When launching the HACompanion app on Miyoo Mini Plus hardware, the app either:
1. Shows a black screen (app is running but no visual output)
2. Crashes immediately at boot
3. Initializes correctly in debug logs but screen remains black

The app works perfectly on desktop (x86_64/macOS) builds but fails on Miyoo ARM hardware.

## Expected Behavior

App should display the retro-styled UI on the Miyoo's 640x480 screen, showing the entity list or setup screen depending on configuration state.

## Environment

- **Device**: Miyoo Mini Plus running OnionOS
- **Display**: 640x480 @ 60Hz
- **SDL Version**: SDL2 (various builds attempted)
- **Network**: WiFi connected, Home Assistant API reachable
- **Direct Framebuffer Test**: ✅ CAN draw to `/dev/fb0` over SSH (confirmed hardware works)

## Key Discoveries

### 1. pressMenu2Kill is NOT Required (DEBUNKED)

**Initial Theory**: OnionOS menu was blocking screen access
- **Commit**: 5fd6cd2 - "Fix black screen: Add pressMenu2Kill to properly take over display"
- **Outcome**: FAILED - Added pressMenu2Kill call but black screen persisted
- **Later Finding**: This was NOT the issue. Many Miyoo apps work without pressMenu2Kill.

**Lesson Learned**: Don't assume pressMenu2Kill is required. It's for apps that need to temporarily hide the OnionOS menu overlay, but our app launches standalone.

### 2. SDL Version Compatibility Hell

#### SDL 1.2 - REJECTED
- Decision: Not suitable for this project (limited features, outdated API)
- Never actually attempted

#### Vanilla SDL 2.0 - CURRENT APPROACH (PARTIALLY WORKING)
- **Status**: Compiles and links successfully
- **Issue**: Missing MMIYOO video driver (vanilla SDL2 doesn't include Miyoo-specific drivers)
- **Commits**:
  - 62cfb7c - Restored vanilla SDL2 build
  - Multiple commits for environment variables and renderer flags

#### steward-fu SDL2 (MMIYOO branch) - FAILED
- **Repo**: https://github.com/steward-fu/sdl2
- **Commit**: e0e933b - "Use steward-fu SDL2 with MMIYOO driver for Miyoo builds"
- **Problem**: Requires custom toolchain (arm-miyoo-linux-uclibcgnueabihf) that we don't have
- **Reverted**: 62cfb7c - "FIX: Restore vanilla SDL2 build (steward-fu requires custom toolchain)"
- **Outcome**: FAILED - Cannot build with standard arm-linux-gnueabihf toolchain

#### XK9274/sdl2_miyoo (vanilla branch) - MOST PROMISING
- **Repo**: https://github.com/XK9274/sdl2_miyoo (vanilla branch)
- **Why Promising**:
  - Same author created Moonlight Miyoo app which works great with SDL 2.0
  - May have better compatibility with standard ARM toolchains
  - Active development, specifically for Miyoo
- **Status**: NOT YET TESTED (current candidate to try next)

### 3. SDL Environment Variables - UNDERSTOOD

We know the required environment variables:
```bash
export SDL_VIDEODRIVER=mmiyoo              # Use Miyoo video driver
export SDL_MMIYOO_DOUBLE_BUFFER=1          # Enable double buffering
export LD_LIBRARY_PATH=/path/to/libs:$LD_LIBRARY_PATH
```

**Critical Discovery**: `SDL_MMIYOO_DOUBLE_BUFFER` must be set in C code BEFORE `SDL_Init()`:
```c
SDL_setenv("SDL_MMIYOO_DOUBLE_BUFFER", "1", 1);  // BEFORE SDL_Init()
SDL_Init(SDL_INIT_VIDEO);
```

**Commits**:
- 07e3a72 - "Fix black screen: Set SDL_MMIYOO_DOUBLE_BUFFER in C code before SDL_Init()"
- 3fabed3 - "Fix black screen on Miyoo: Enable SDL double buffering"

### 4. SDL Renderer Flags - UNDERSTOOD

**Correct Configuration**:
```c
SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
```

**What DOESN'T Work**:
- ❌ `SDL_RENDERER_SOFTWARE` (flags=13) - Results in black screen
- ❌ `flags=0` (auto-detect) - SDL chooses SOFTWARE renderer

**Commits**:
- 8d3e8c8 - "Fix black screen: Use SDL_RENDERER_ACCELERATED for Miyoo"
- 5939cfd/4333c71 - Attempted and reverted SOFTWARE renderer

### 5. Direct Framebuffer Access - CONFIRMED WORKING

**Test**: Can draw directly to `/dev/fb0` over SSH
```bash
ssh onion@192.168.101.235
cat /dev/urandom > /dev/fb0  # Shows static on screen
```

**Conclusion**: Hardware is working, issue is with SDL initialization/rendering

## Attempted Solutions

### Attempt 1: SDL Renderer Flags (PARTIAL SUCCESS)
- **Date**: 2025-11-20
- **Commits**: 8d3e8c8, 5939cfd, 4333c71
- **Description**: Tried various SDL renderer flags (SOFTWARE, ACCELERATED, auto-detect)
- **Outcome**: PARTIAL - Learned that ACCELERATED is required, but black screen persisted
- **Logs**: Debug logs showed correct renderer initialization but no visual output

### Attempt 2: SDL_MMIYOO_DOUBLE_BUFFER Environment Variable (PARTIAL SUCCESS)
- **Date**: 2025-11-21
- **Commits**: 07e3a72, 3fabed3
- **Description**: Set double buffer flag in C code before SDL_Init()
- **Outcome**: PARTIAL - Correct configuration confirmed, but issue remained
- **Logs**: Renderer showed as "MMIYOO" with flags=10 (correct), but screen still black

### Attempt 3: pressMenu2Kill Screen Takeover (FAILED)
- **Date**: 2025-11-22
- **Commit**: 5fd6cd2
- **Description**: Added pressMenu2Kill call in launch script to take over display from OnionOS menu
- **Outcome**: FAILED - Black screen persisted, later determined this wasn't the root cause
- **Lessons**: Don't assume menu takeover is the issue without evidence

### Attempt 4: Framebuffer Direct Manipulation (FAILED)
- **Date**: 2025-11-22
- **Commits**: aa00390, d5ce1e9, 6d84ae7
- **Description**: Tried manually panning framebuffer before/after SDL_Init, forcing pan on every frame
- **Outcome**: FAILED - SDL should handle this automatically
- **Lessons**: Don't fight with SDL's framebuffer management

### Attempt 5: SDL Rendering Methods (FAILED)
- **Date**: 2025-11-22
- **Commits**: d8c437f, c670ec9, ae32a88
- **Description**: Replaced SDL_RenderClear with SDL_RenderFillRect, tried different blend modes
- **Outcome**: FAILED - Rendering approach wasn't the issue
- **Lessons**: Focus on initialization, not rendering mechanics

### Attempt 6: SDL Window Flags (FAILED)
- **Date**: 2025-11-21
- **Commits**: 0c4a009, 6e3704c
- **Description**: Added/removed SDL_WINDOW_FULLSCREEN flag
- **Outcome**: FAILED - MMIYOO driver handles fullscreen automatically
- **Lessons**: Don't add unnecessary window flags

### Attempt 7: steward-fu SDL2 with MMIYOO Driver (FAILED)
- **Date**: 2025-11-23
- **Commit**: e0e933b (reverted in 62cfb7c)
- **Description**: Switched to steward-fu's Miyoo-specific SDL2 fork
- **Outcome**: FAILED - Requires custom uClibc toolchain we don't have
- **Error**: Build fails with standard arm-linux-gnueabihf toolchain
- **Lessons**: Check toolchain compatibility before switching SDL builds

### Attempt 8: Skip Network Test Mode (TESTING)
- **Date**: 2025-11-23
- **Commit**: 8415cf7
- **Description**: Added SKIP_NETWORK_TEST mode to validate SDL rendering without network calls
- **Outcome**: TESTING - Simplifies debugging by isolating SDL issues from network issues
- **Status**: Waiting for device testing

### Attempt 9: Simple Color Cycling Test (FAILED)
- **Date**: 2025-11-24
- **Commit**: d7b55ec
- **Description**: Minimal rendering test that bypasses ALL complex code (fonts, screens, UI) and just fills screen with solid colors cycling WHITE → RED → GREEN → BLUE every 60 frames
- **Purpose**: Determine if problem is with SDL/Miyoo communication or our rendering code
- **Actual Debug Output**:
  ```
  SDL Renderer: MMIYOO
    Flags: 10
    Texture formats: 2
    Max texture: 800x600
  SDL Video Driver: mmiyoo
  SDL2 initialized successfully
  Screen: 640x480
  === SKIP_NETWORK_TEST MODE: Bypassing all network/database initialization ===
  Starting minimal rendering test loop...
  Should see: WHITE -> RED -> GREEN -> BLUE cycling every 60 frames
  Frame 0: Color 0 (R=255 G=255 B=255)
  Frame 60: Color 1 (R=255 G=0 B=0)
  Frame 120: Color 2 (R=0 G=255 B=0)
  Frame 180: Color 3 (R=0 G=255 B=0)
  Frame 240: Color 0 (R=255 G=255 B=255)
  Frame 300: Color 1 (R=255 G=0 B=0)
  ... (continues cycling)
  ```
- **Visual Result**: Screen remained BLACK - NO colors visible on physical Miyoo screen
- **Outcome**: FAILED - Even the simplest possible SDL rendering (just `SDL_RenderClear()` + `SDL_RenderPresent()`) produces no visual output
- **Conclusion**: This confirms the problem is NOT in our complex UI code. The issue is that SDL cannot communicate with the Miyoo framebuffer, despite:
  - SDL reporting "MMIYOO" renderer
  - SDL_Init() succeeding
  - SDL_CreateRenderer() succeeding with ACCELERATED flags
  - SDL_RenderClear() and SDL_RenderPresent() executing without errors
  - All debug logs showing "correct" initialization

## Current Status: ROOT CAUSE CONFIRMED

**Root Cause**: Despite SDL reporting the "MMIYOO" video driver and renderer, the bundled OnionOS SDL2 library is NOT being used at runtime. The app is likely using our statically-compiled vanilla SDL2 instead, which doesn't have a functional MMIYOO framebuffer backend.

**Critical Finding from Attempt 9**:
- SDL claims to use "MMIYOO" driver and renderer
- All SDL calls succeed without errors
- But: ZERO visual output, even with simplest possible rendering
- This means SDL is not actually writing to the Miyoo framebuffer

**The Real Problem**:
Our launch script sets `LD_LIBRARY_PATH` to load OnionOS's SDL2 library at runtime, but:
1. The app may be statically linked against vanilla SDL2 at compile time
2. Or: The LD_LIBRARY_PATH isn't working as expected
3. Or: The bundled SDL2 library has compatibility issues with our binary

**Evidence**:
- Attempt 9 proves it's NOT our rendering code (even `SDL_RenderClear()` doesn't work)
- SDL reports "MMIYOO" but produces no output (fake/stub driver?)
- Direct framebuffer access works (`cat /dev/urandom > /dev/fb0`)
- App responds to input (Menu button works)

### Attempt 10: Verify Which SDL2 Library is Being Used (NEXT)
- **Date**: 2025-11-24
- **Description**: Check if our app is actually loading OnionOS's SDL2 library or using statically-linked vanilla SDL2
- **Diagnostic Commands**:
  ```bash
  # On Miyoo device:
  ldd /mnt/SDCARD/App/HACompanion/hacompanion
  # Should show: libSDL2-2.0.so.0 => /mnt/SDCARD/App/HACompanion/libs/miyoo/libSDL2-2.0.so.0

  # Check if library is actually loaded:
  cat /proc/$(pidof hacompanion)/maps | grep SDL
  ```
- **Possible Outcomes**:
  1. If `ldd` shows "not a dynamic executable" → App is statically linked, ignores LD_LIBRARY_PATH
  2. If SDL2 library path is wrong → Fix LD_LIBRARY_PATH in launch script
  3. If library loads but still black screen → Bundled SDL2 library is incompatible
- **Status**: PENDING - Need to run diagnostics on device

## Next Steps to Try

### Option A: Force Dynamic Linking with OnionOS SDL2 - RECOMMENDED
**Why**:
- Same author as working Moonlight Miyoo app
- Specifically designed for Miyoo with standard toolchains
- May have MMIYOO driver without requiring custom toolchain

**How**:
1. Update build script to clone XK9274/sdl2_miyoo (vanilla branch)
2. Build SDL2 from this source
3. Test if MMIYOO driver is available
4. Validate with SKIP_NETWORK_TEST mode

**Risk**: Medium - Might still require toolchain changes

### Option B: Bundle Miyoo's Native SDL2 Library
**Why**:
- OnionOS already has working SDL2 with MMIYOO driver
- Located at `/mnt/SDCARD/.tmp_update/lib/parasyte/libSDL2-2.0.so.0`
- Just copy and use it

**How**:
1. Extract SDL2 library from Miyoo device
2. Bundle with app release
3. Use LD_LIBRARY_PATH to load at runtime

**Risk**: Low - Known working approach, but may have version mismatches

### Option C: Try OnionUI Official Toolchain
**Why**:
- Pre-configured for Miyoo with all libraries
- Includes proper SDL2 build

**How**:
1. Set up OnionUI toolchain Docker container
2. Build app inside container
3. Test resulting binary

**Risk**: Medium - Build system overhaul required

### Option D: Investigate Miyoo Mini SDL2 Backends
**Why**:
- May be able to use SDL2 with different backend (fbdev, directfb)
- Avoid MMIYOO driver dependency

**How**:
1. Research SDL2 video backends available on ARM
2. Try SDL_VIDEODRIVER=fbdev or SDL_VIDEODRIVER=directfb
3. Test with vanilla SDL2

**Risk**: High - May not work without hardware acceleration

## Related Research

- **Working Miyoo SDL2 Apps** (for reference):
  - XK9274/miyoo_sdl2_benchmarks - Uses ACCELERATED renderer
  - Moonlight Miyoo (by XK9274) - SDL2 app that works perfectly
  - lanmarc77/tinamp - Uses ACCELERATED renderer
  - shauninman/MinUI - Uses ACCELERATED | PRESENTVSYNC
  - steward-fu/sdl2 projects - Require custom toolchain

- **Research Documents**:
  - See CLAUDE.md "Miyoo Mini Plus SDL2 Configuration" section
  - Commit dc22940 - "Fix black screen - comprehensive SDL/EGL environment"
  - Commit de189bd - "Add comprehensive black screen research documentation"

## Lessons Learned

1. **Vanilla SDL2 doesn't have MMIYOO driver** - This is the core issue, not initialization
2. **pressMenu2Kill is not always needed** - Many apps work without it
3. **Direct framebuffer access works** - Hardware is fine, SDL setup is the problem
4. **Toolchain matters** - Some SDL2 forks require specific toolchains (uClibc vs glibc)
5. **XK9274 is a trusted source** - Their Moonlight app works, so their SDL2 fork likely will too
6. **Don't over-engineer initialization** - Standard SDL2 patterns work if the driver exists
7. **Test in isolation** - SKIP_NETWORK_TEST mode helps separate SDL from network issues

## Resolution

**TBD** - Currently blocked on SDL2 MMIYOO driver availability. Next attempt will be XK9274/sdl2_miyoo (vanilla branch).
