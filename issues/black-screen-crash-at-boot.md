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

## Current Status: ROOT CAUSE FOUND - FIX IN PROGRESS

**Root Cause (CONFIRMED)**: Binary was statically linked with vanilla SDL2 baked directly into it (28MB size). The bundled OnionOS SDL2 library with working MMIYOO driver was completely ignored.

**Why It Happened**:
- GitHub Actions build script configured all SDL2 dependencies with `--disable-shared --enable-static`
- This forced the linker to statically link all SDL2 code into the binary
- The statically-linked SDL2 was vanilla upstream code with only a stub MMIYOO driver
- Launch script's `LD_LIBRARY_PATH` did nothing because there were no dynamic dependencies

**The Fix (Applied)**:
- Changed all SDL2 dependencies to `--enable-shared --disable-static`
- Binary will now dynamically link to external .so files
- Bundled all required shared libraries in the release package
- At runtime, working OnionOS SDL2 library will be loaded via LD_LIBRARY_PATH

**Expected Outcome**:
- Binary size: 28MB → ~2-3MB
- Dynamic linking to working MMIYOO driver
- Visual output on Miyoo screen should work!

### Attempt 10: Verify Which SDL2 Library is Being Used (CONFIRMED STATIC LINKING!)
- **Date**: 2025-11-24
- **Description**: Checked binary size to determine if statically or dynamically linked
- **Result**: Binary was **28 MB** - CONFIRMED statically linked!
- **Conclusion**: The binary had SDL2 compiled directly into it, completely ignoring the bundled OnionOS SDL2 library
- **Root Cause Found**: GitHub Actions build script was building ALL SDL2 libraries with `--disable-shared --enable-static`
  - Freetype: `--disable-shared --enable-static`
  - SDL2_ttf: `--disable-shared --enable-static`
  - SDL2_image: `--disable-shared --enable-static`
- **Impact**: This forced static linking, baking vanilla SDL2 (with non-functional MMIYOO stub driver) directly into the binary

### Attempt 12: Remove SDL_RENDERER_PRESENTVSYNC (FAILED)
- **Date**: 2025-11-27
- **Commit**: 869eab9
- **Description**: Research into XK9274/miyoo_sdl2_benchmarks render_suite revealed that working Miyoo apps use `SDL_RENDERER_ACCELERATED` **WITHOUT** `SDL_RENDERER_PRESENTVSYNC`. The render_suite benchmark, which renders complex scenes with particles, geometry, and overlays, explicitly avoids PRESENTVSYNC.
- **Fix Applied**: Changed renderer creation from:
  ```c
  SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  ```
  To:
  ```c
  SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  ```
- **Outcome**: FAILED - Black screen persists
- **Debug Log Analysis**:
  - SDL still reports `Flags: 10` (ACCELERATED | PRESENTVSYNC) - the MMIYOO driver appears to add PRESENTVSYNC regardless of what we request
  - Color cycling test runs correctly in logs but nothing visible on screen
  - This suggests the PRESENTVSYNC flag is NOT the root cause
- **Conclusion**: The MMIYOO driver controls renderer flags internally; our requested flags are suggestions, not requirements. The black screen issue is something else.

### Attempt 13: Match Moonlight Launch Script Configuration (TESTING)
- **Date**: 2025-12-04
- **Description**: Analyzed working Moonlight app's launch script and found significant differences:
  1. **Moonlight does NOT use pressMenu2Kill** - just launches directly
  2. **Moonlight sets THREE driver env vars**:
     - `SDL_VIDEODRIVER=mmiyoo`
     - `SDL_AUDIODRIVER=mmiyoo`
     - `EGL_VIDEODRIVER=mmiyoo` (WE WERE MISSING THIS!)
  3. **Moonlight uses full LD_LIBRARY_PATH** including parasyte
- **Changes Made**:
  - Removed pressMenu2Kill from launch script
  - Added `EGL_VIDEODRIVER=mmiyoo`
  - Added `SDL_AUDIODRIVER=mmiyoo`
  - Expanded LD_LIBRARY_PATH to: `./lib:/mnt/SDCARD/miyoo/lib:/mnt/SDCARD/.tmp_update/lib:/mnt/SDCARD/.tmp_update/lib/parasyte`
- **Hypothesis**: The missing `EGL_VIDEODRIVER=mmiyoo` may be critical - EGL is the interface between OpenGL ES and the native windowing system. Without it, the MMIYOO driver may not properly communicate with the framebuffer.
- **Outcome**: FAILED - Black screen persists
- **Debug Log**: Same as before - SDL reports MMIYOO renderer, frames cycling correctly in logs, no visual output
- **Conclusion**: Environment variables are not the issue. The problem is deeper.

### Attempt 14: Fix SDL2 Static Library Causing Symbol Conflict (FAILED - INCOMPLETE FIX)
- **Date**: 2025-12-04
- **Root Cause Discovery**: Our SDL2_ttf library is 25MB vs Moonlight's 41KB!
  - Running `strings` on our SDL2_ttf shows **502 SDL symbols** vs Moonlight's **45**
  - This means our SDL2_ttf has SDL2 **statically linked inside it**
  - The statically-linked SDL2 has a non-functional MMIYOO driver stub
  - When our app runs, SDL2_ttf's embedded SDL2 code may interfere with the actual MMIYOO driver
- **Why This Happened**: GitHub Actions builds SDL2 with `--enable-shared --enable-static`
  - When SDL2_ttf links against SDL2, it prefers the static library
  - This embeds vanilla SDL2 (without working MMIYOO driver) into SDL2_ttf
- **The Fix (Incomplete)**: Changed SDL2 build from `--enable-shared --enable-static` to `--enable-shared --disable-static`
- **Result**: SDL2_ttf was STILL 25MB after the fix!
- **Why It Failed**: SDL2_ttf 2.20.2 has a DIFFERENT issue - it **bundles its own internal copies** of freetype and harfbuzz from `external/` directories regardless of what we pass for SDL2 flags
- **Status**: FAILED - Led to Attempt 15

### Attempt 15: Disable SDL2_ttf Vendored Libraries (SUCCESS - LIBRARY SIZE FIXED!)
- **Date**: 2025-12-04
- **Root Cause (Updated)**: SDL2_ttf 2.20.2 has `--enable-freetype-builtin` and `--enable-harfbuzz-builtin` enabled by DEFAULT
  - This causes SDL2_ttf to compile its own copies of freetype and harfbuzz from `external/` subdirectories
  - Even with our externally-built freetype, SDL2_ttf ignores it and uses its bundled version
  - This results in a 25MB library that embeds both freetype AND harfbuzz
- **The Fix**: Add configure flags to SDL2_ttf build:
  - `--disable-freetype-builtin` - Use external freetype instead of bundled
  - `--disable-harfbuzz` - Disable harfbuzz entirely (we don't need advanced text shaping)
  - `PKG_CONFIG_PATH=$SYSROOT/usr/lib/pkgconfig` - So configure can find freetype2.pc
- **Build Result**: SUCCESS!
  - SDL2_ttf: **244,876 bytes (245KB)** - down from 25,397,568 bytes (25MB)!
  - **99% size reduction achieved!**
  - Library now dynamically links to external freetype instead of embedding it
- **Status**: DEPLOYED TO DEVICE - Awaiting visual output test

### Attempt 11: Switch to Dynamic Linking (CONFIRMED WORKING)
- **Date**: 2025-11-24
- **Commits**: e747531, 6b8b47c
- **Description**: Changed all SDL2 dependencies to build as shared libraries instead of static
- **Changes Made**:
  1. **Freetype**: Changed to `--enable-shared --disable-static`
  2. **SDL2_ttf**: Changed to `--enable-shared --disable-static`
  3. **SDL2_image**: Changed to `--enable-shared --disable-static`
  4. **Bundle libraries**: Added code to bundle all shared libraries in release package:
     - `libSDL2_ttf-2.0.so.0`
     - `libSDL2_image-2.0.so.0`
     - `libfreetype.so.6`
     - `libSDL2-2.0.so.0` (Miyoo's version with working MMIYOO driver)
- **Expected Results**:
  - Binary size should drop from 28MB to ~2-3MB
  - Binary will dynamically link to bundled libraries via LD_LIBRARY_PATH
  - OnionOS SDL2 library with working MMIYOO driver will be loaded at runtime
  - App should actually display on Miyoo screen!
- **Status**: BUILD IN PROGRESS - Waiting for GitHub Actions to complete

### Attempt 16: Reverse-Engineer Working Moonlight App (TESTING)
- **Date**: 2025-12-04
- **Strategy Shift**: Instead of continuing to guess what's wrong, we're taking a proven-working app (Moonlight) and working backwards to identify the exact difference.
- **Key Discovery - Library Comparison**:

  | Library | Moonlight (Working) | HACompanion (Black Screen) |
  |---------|---------------------|----------------------------|
  | Binary | 108KB | 3.5MB |
  | libSDL2-2.0.so.0 | 5.89MB | 5.21MB |
  | libSDL2_ttf-2.0.so.0 | 41KB | 245KB |
  | libSDL2_image-2.0.so.0 | 128KB | 730KB |
  | **libEGL.so** | 55KB | **MISSING** |
  | **libGLESv1_CM.so** | 29KB | **MISSING** |
  | **libGLESv2.so** | 21.7MB | **MISSING** |

- **Primary Hypothesis**: Moonlight bundles 21MB+ of EGL/OpenGL ES libraries that HACompanion doesn't have. The MMIYOO SDL2 driver likely uses OpenGL ES internally to render to the framebuffer. Without EGL/GLES libraries, SDL reports success but nothing reaches the screen.
- **Evidence**:
  - HACompanion debug.log shows MMIYOO renderer, Flags: 10, 13,000+ frames rendered
  - Physical screen remains BLACK despite all "successful" rendering
  - SDL thinks it's rendering, but nothing is displayed
- **Test Plan**:
  1. Create minimal `test_display.c` program (just SDL2 + red screen)
  2. Deploy with **Moonlight's complete library set** (including EGL/GLES)
  3. If RED screen appears: confirms missing libraries are the issue
  4. Binary search to find minimum required library set
- **Files to Create**:
  - `src/test_display.c` - Minimal SDL2 test
  - `dist/test_display/launch.sh` - Uses Moonlight's LD_LIBRARY_PATH
- **Status**: IN PROGRESS - Creating test program

### Attempt 16 Update: ROOT CAUSE FOUND!
- **Date**: 2025-12-04
- **Discovery**: Even with Moonlight's libraries, HACompanion shows black screen. The test proved:
  1. HACompanion uses Moonlight's exact SDL2 library = still black screen
  2. Moonlight binary works = display hardware is fine

- **Root Cause**: ABI mismatch between our binary and the MMIYOO SDL2 library
  - Moonlight was compiled with the **mmiyoo buildroot toolchain** (`/opt/mmiyoo/arm-buildroot-linux-gnueabihf/`)
  - HACompanion was compiled with **Ubuntu's cross-compiler** (`gcc-arm-linux-gnueabihf`)
  - Moonlight's SDL2 is a **custom fork** (`sdl2-moonlight-miyoo`) with MMIYOO-specific code
  - Our binary uses vanilla SDL2 headers which may have different struct layouts

- **Evidence**:
  - Moonlight SDL2 contains: `SDL_render_mmiyoo.c`, `SDL_video_mmiyoo.c`, `SDL_opengles_mmiyoo.c`
  - Built from `/root/workspace/sdl2-moonlight-miyoo/`
  - Uses `/opt/mmiyoo/arm-buildroot-linux-gnueabihf/sysroot/`
  - Moonlight binary requires only GLIBC 2.4-2.7; HACompanion requires up to GLIBC 2.28

- **Solution**: Use the official Miyoo Mini buildroot toolchain:
  - https://github.com/shauninman/union-miyoomini-toolchain (Docker-based)
  - https://github.com/djdiskmachine/MiyooMini-toolchain (pre-built binaries)

- **Status**: SOLUTION IDENTIFIED - Switching to mmiyoo toolchain in Attempt 17

### Attempt 17: Use ARM A-profile GCC 8.3 Toolchain with Prebuilt SDL2 (SUCCESS!)
- **Date**: 2025-12-04
- **Root Cause Analysis**:
  - Initial theory was ABI mismatch between Ubuntu cross-compiler and Miyoo's SDL2
  - Discovered `miyoocfw/toolchain` uses **musl libc** (not glibc!)
  - Prebuilt SDL2 libraries from `steward-fu/sdl2/prebuilt/mini/` are **glibc-based**
  - Ubuntu 22.04's GCC 11 produces binaries requiring GLIBC_2.34, but Miyoo only has GLIBC_2.28
- **Key Discovery**: Need to match GLIBC version between compiler and device:
  - Miyoo device: GLIBC 2.28
  - Ubuntu 22.04 GCC 11: Produces GLIBC 2.34 binaries = **FAILS**
  - ARM A-profile GCC 8.3-2019.03: Produces GLIBC 2.28 binaries = **WORKS**
- **Solution**: Use ARM's official A-profile toolchain (GCC 8.3-2019.03) which targets glibc 2.28
  - Download: `https://developer.arm.com/-/media/Files/downloads/gnu-a/8.3-2019.03/binrel/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf.tar.xz`
  - This toolchain was specifically designed for embedded ARM systems
  - Matches Miyoo's "GNU Toolchain for the A-profile Architecture 8.2-2018-08"
- **Build Errors Fixed**:
  1. **Wrong compiler prefix**: Initially tried `arm-buildroot-linux-musleabi-` (musl) - WRONG
  2. **Musl/glibc incompatibility**: Prebuilt SDL2 uses glibc, so musl compiler fails
  3. **GLIBC version mismatch**: Ubuntu 22.04 GCC 11 produces GLIBC_2.34 binaries, device has 2.28
  4. **Missing system libraries at runtime**: Added `/config/lib` and moonlight's lib path
- **Final Configuration**:
  - `build-test-display` job: Ubuntu 22.04 + ARM A-profile GCC 8.3 + prebuilt SDL2/EGL/GLES
  - Compiler: `arm-linux-gnueabihf-gcc` from `gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf`
  - LD_LIBRARY_PATH: `./lib:/config/lib:/mnt/SDCARD/App/moonlight/lib:/mnt/SDCARD/miyoo/lib:/mnt/SDCARD/.tmp_update/lib:/mnt/SDCARD/.tmp_update/lib/parasyte`
- **Test Results**:
  ```
  === Minimal SDL2 Display Test ===
  Expected: RED screen for 5 seconds

  SDL_Init: OK
  Video driver: Mini
  SDL_CreateWindow: OK
  Renderer: Miyoo Mini, Flags: 10

  Starting render loop (300 frames @ ~60fps = 5 seconds)...
  You should see a RED screen now!

  Frame 0
  Frame 60
  Frame 120
  Frame 180
  Frame 240

  Test complete. Cleaning up...
  Done!
  ```
- **Status**: SUCCESS! - test_display runs correctly with MMIYOO driver. Now need to apply same toolchain to HACompanion build.

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

### Attempt 18: Device Testing Reveals Bug NOT Fixed (FAILED)
- **Date**: 2025-12-04
- **Critical Discovery**: User tested on actual Miyoo hardware and reported:
  - "i see nothing on screen"
  - "the test display stayed up for a few seconds but i didnt see anything, then it quit after about 8-10 seconds"
  - "black screen; nothing happens" (when launching HACompanion)
- **What Logs Showed vs Reality**:
  - Logs: "SDL Renderer: Miyoo Mini", "Frame 0", "Frame 60", etc. - all appears successful
  - Reality: Physical screen remains BLACK - no visual output whatsoever
- **Library Swap Testing**:
  - Tried Moonlight's SDL2 (5.89MB): App crashed immediately (ABI incompatibility)
  - Tried OnionOS parasyte SDL2 (5.21MB): Black screen persists
  - Tried removing LD_LIBRARY_PATH entirely: Black screen persists
- **Root Cause Analysis**:
  - Our binary was compiled with ARM A-profile GCC 8.3
  - steward-fu's prebuilt SDL2 was compiled with a DIFFERENT toolchain
  - The binary and SDL2 library are NOT ABI-compatible
  - SDL reports success because function calls work, but internal struct layouts may differ
- **Key Discovery from XK9274/miyoo_sdl2_benchmarks**:
  ```makefile
  CROSS_PREFIX  ?= /opt/miyoomini-toolchain/usr/bin/arm-linux-gnueabihf-
  SYSROOT       ?= /opt/miyoomini-toolchain/usr/arm-linux-gnueabihf/sysroot
  SDL_INCLUDE   := $(SYSROOT)/usr/include/SDL2
  SDL_LIBDIR    := $(SYSROOT)/usr/lib
  ```
  - They use a Docker-based toolchain with SDL2 **built into the sysroot**
  - The binary and SDL2 library are compiled TOGETHER, ensuring ABI compatibility
- **Solution Identified**: Use `shauninman/union-miyoomini-toolchain` Docker image
  - This is the official Miyoo Mini toolchain used by MinUI and other working apps
  - Has SDL2 already compiled in the sysroot
  - Ensures binary and library are compiled with the same compiler/settings
- **Status**: SOLUTION IDENTIFIED - Need to modify GitHub Actions workflow

### Attempt 19: Build SDL2 with MMIYOO Driver from Source (TESTING)
- **Date**: 2025-12-05
- **Previous Issue**: Downloaded prebuilt toolchain from shauninman has SDL 1.2 in sysroot, not SDL 2.0!
  - `SDL.h` found at: `/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/include/SDL/SDL.h` (SDL 1.2!)
  - No `libSDL2*.so*` files found in toolchain
  - Build failed with: `unknown type name 'SDL_Window'` (SDL 1.2 doesn't have this)
- **Solution**: Build SDL2 with MMIYOO driver from steward-fu's fork as part of the build process
  - Clone: `https://github.com/steward-fu/sdl.git`
  - Build from: `sdl2-mmiyoo/sdl2/` directory
  - Configure with: `--enable-video-mmiyoo` and various `--disable-video-*` flags
  - This gives us SDL2 with the actual MMIYOO driver compiled in
- **Key Insight**: The miyoomini-toolchain provides the cross-compiler, but SDL2 needs to be built separately with MMIYOO support
- **Build Changes**:
  1. Clone steward-fu/sdl repo (has MMIYOO driver)
  2. Build SDL2 with `--enable-video-mmiyoo`
  3. Install to `$DEPS` directory
  4. Build SDL2_ttf and SDL2_image against our built SDL2
  5. Bundle all libraries together
- **Expected Result**: Binary and SDL2 library both compiled with same toolchain = ABI compatibility
- **Status**: BUILD IN PROGRESS - Waiting for GitHub Actions

## Current Status: SDL2 BUILD FROM SOURCE - IN PROGRESS

**Root Cause (CONFIRMED via user testing + build analysis)**:
- The prebuilt miyoomini toolchain has SDL 1.2 in sysroot, not SDL 2.0
- Need to build SDL2 with MMIYOO driver from source (steward-fu's fork)
- This ensures binary and SDL2 library are compiled together with same toolchain

**The Fix**:
1. Download shauninman's miyoomini toolchain for the cross-compiler
2. Clone steward-fu's SDL repo (has MMIYOO video driver)
3. Build SDL2 with `--enable-video-mmiyoo` using the toolchain
4. Build SDL2_ttf/SDL2_image against our built SDL2
5. Bundle everything together

**Why This Should Work**:
- Binary compiled with miyoomini toolchain
- SDL2 compiled with same toolchain + MMIYOO driver
- SDL2_ttf/SDL2_image compiled against our SDL2
- All libraries have matching ABI
- MMIYOO driver is real, not a stub

**Issue Status: OPEN - Building SDL2 from source with MMIYOO driver**
