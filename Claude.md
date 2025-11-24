# Home Assistant Companion for Miyoo Mini Plus

A retro-styled native companion app for Home Assistant, built specifically for the Miyoo Mini Plus handheld console running OnionOS.

## Project Vision

Control your smart home from a retro gaming handheld! This app brings Home Assistant's powerful automation platform to the Miyoo Mini Plus with a nostalgic Game Boy/SNES-inspired interface, designed for game controller navigation.

## Features (MVP v1.0)

- **Multi-Server Support**: Configure and switch between multiple Home Assistant instances
- **Device Management**: Browse all devices and their entities
- **Entity Control**: View and control lights, switches, sensors, climate, and more
- **Quick Actions**: Run automations, scripts, and activate scenes
- **Favorites Dashboard**: Create a personalized quick-access dashboard
- **Offline Mode**: View cached data when disconnected
- **Retro UI**: Pixel art icons, classic color palettes, scanline effects

## Technical Stack

| Component | Technology |
|-----------|-----------|
| **Language** | C/C++ |
| **UI Framework** | SDL2 (with SDL2_image, SDL2_ttf, SDL2_gfx) |
| **HTTP Client** | libcurl |
| **JSON Parser** | cJSON |
| **Database** | SQLite3 |
| **Platform** | Miyoo Mini Plus (ARM, OnionOS) |
| **Display** | 640x480 @ 60Hz |
| **Build System** | CMake with ARM cross-compilation |

## Controller Layout (Nintendo Defaults)

- **D-Pad**: Navigate menus (up/down/left/right)
- **A Button**: Confirm / Toggle / Activate
- **B Button**: Back / Cancel
- **X Button**: Toggle view mode (Domain → Room → Favorites → Domain)
- **Y Button**: Toggle favorite status
- **L1/R1**: Tab navigation (switch between tabs in current view)
- **L2/R2**: (Reserved for future features)
- **Select**: View entity detail screen
- **Start**: Manual sync / Refresh
- **Menu**: Exit application (with confirmation dialog)

## Quick Start

### Prerequisites

1. Miyoo Mini Plus with OnionOS installed
2. WiFi connection configured on Miyoo
3. Home Assistant server accessible on your network
4. Long-lived access token from Home Assistant

### Installation

1. Download latest release from GitHub releases
2. Extract to `/mnt/SDCARD/App/HACompanion/` on your Miyoo SD card
3. Copy `servers.example.json` to `servers.json` and edit with your Home Assistant details (see Configuration)
4. Launch "HA Companion" from OnionOS Apps menu

### Configuration

Create/edit `/mnt/SDCARD/App/HACompanion/servers.json`:

```json
{
  "servers": [
    {
      "name": "Home",
      "url": "http://homeassistant.local",
      "port": 8123,
      "token": "your_long_lived_access_token_here",
      "username": "admin"
    },
    {
      "name": "Remote",
      "url": "https://your-instance.duckdns.org",
      "port": 8123,
      "token": "your_remote_token_here",
      "username": "remote_user"
    }
  ],
  "default_server": 0
}
```

**Getting a Long-Lived Access Token:**
1. Open Home Assistant web interface
2. Click your profile (bottom left)
3. Scroll to "Long-Lived Access Tokens"
4. Click "Create Token"
5. Name it "Miyoo Companion" and copy the token

## Project Structure

```
hass-miyoo/
├── CLAUDE.md                   # This file - project overview
├── README.md                   # User-facing documentation
├── CMakeLists.txt              # Build configuration
├── servers.example.json        # Template server configuration
├── .gitignore                  # Git ignore rules
├── issues/                     # Bug/issue tracking (one .md per issue)
├── src/                        # Source code
│   ├── main.c
│   ├── ha_client.c/h           # Home Assistant API client
│   ├── database.c/h            # SQLite cache management
│   ├── cache_manager.c/h       # Entity caching layer
│   ├── ui/                     # UI components
│   │   ├── fonts.c/h           # Font management
│   │   ├── icons.c/h           # Icon rendering
│   │   └── components.c/h      # Reusable UI components
│   ├── screens/                # Screen implementations
│   │   ├── screen_setup.c/h    # Server selection
│   │   ├── screen_list.c/h     # Entity list with tabs
│   │   ├── screen_device.c/h   # Device control screen
│   │   ├── screen_info.c/h     # Read-only entity info
│   │   ├── screen_automation.c/h
│   │   ├── screen_script.c/h
│   │   └── screen_scene.c/h
│   └── utils/                  # Helper functions
│       ├── input.c/h           # Controller input handling
│       ├── config.c/h          # Configuration loading
│       └── json_helpers.c/h    # JSON parsing utilities
├── assets/                     # Icons, fonts, graphics
│   └── fonts/
├── research/                   # Research documentation
├── design/                     # Design specifications
└── planning/                   # Phase-by-phase plans
```

## Development Phases

This project is organized into 15 development phases. Each phase has detailed documentation in the `/planning/` directory.

### Foundation (Phases 1-4)
1. **[Project Setup](planning/phase-01-project-setup.md)**: Development environment, toolchain, SDL2 boilerplate
2. **[API Client](planning/phase-02-api-client.md)**: HTTP client, Home Assistant API integration
3. **[Data Storage](planning/phase-03-data-storage.md)**: SQLite schema, caching system
4. **[UI Design System](planning/phase-04-ui-design-system.md)**: Retro theme, pixel art icons, UI components

### Core Screens (Phases 5-10)
5. **[Setup Screen](planning/phase-05-setup-screen.md)**: Server configuration (view-only)
6. **[List View](planning/phase-06-list-view.md)**: Tabbed navigation with domain/room view modes
7. **[Device Detail](planning/phase-07-device-detail.md)**: Device screen with entity controls
8. **[Entity Detail](planning/phase-08-entity-detail.md)**: Entity detail with primary control
9. **[Info View](planning/phase-09-info-view.md)**: Detail screens for automations, scripts, scenes, and read-only entities
10. **[Favorites](planning/phase-10-favorites.md)**: Favorites dashboard

### Integration & Polish (Phases 11-15)
11. **[Navigation](planning/phase-11-navigation.md)**: Screen navigation, state management
12. **[Network & Sync](planning/phase-12-network-sync.md)**: Background sync, offline mode
13. **[Polish](planning/phase-13-polish.md)**: Sound, animations, optimization
14. **[Testing](planning/phase-14-testing.md)**: Hardware testing, validation
15. **[Distribution](planning/phase-15-distribution.md)**: Documentation, packaging, release

## Research & Design

Before implementation, comprehensive research was conducted:

### Research Documents (`/research/`)
- **[Miyoo Development](research/miyoo-development.md)**: Hardware specs, OnionOS patterns, input mappings, SDL2 setup
- **[Home Assistant API](research/home-assistant-api.md)**: REST API endpoints, entity structure, authentication methods
- **[Retro Design](research/retro-design.md)**: Color palettes, pixel fonts, classic UI patterns, icon styles

### Design Specifications (`/design/`)
- **[UI Components](design/ui-components.md)**: Buttons, lists, cards, toggles, sliders
- **[Screen Layouts](design/screen-layouts.md)**: Wireframes for all screens with pixel dimensions
- **[Color Palette](design/color-palette.md)**: Final color scheme (Game Boy green or SNES purple themes)
- **[Icon Specifications](design/icon-specifications.md)**: 16x16px pixel art icon library

## Development Workflow

### Development Environment

**Mac Development**: This project is primarily developed on macOS. When using container commands, use `container` instead of `docker` (e.g., `podman` or Docker Desktop for Mac).

### CRITICAL: Testing & Deployment Workflow

**NEVER copy binaries directly to Miyoo via SSH/SCP - it will usually fail!**

#### When User is On-Site with Miyoo

When the Miyoo device is physically nearby:

1. **SSH Access Available**: `sshpass -p onion ssh onion@192.168.101.235`
   - Can read debug logs: `cat /mnt/SDCARD/App/HACompanion/debug.log`
   - Can run diagnostic commands on-device
   - **NEVER copy new binaries over SSH** - always use GitHub release workflow

2. **Testing Workflow**:
   - Push commits to GitHub
   - Wait for GitHub Actions to complete build
   - Download compiled binary from GitHub release
   - Transfer manually to Miyoo SD card
   - Test on device
   - SSH in to read debug.log for diagnostics

#### When User is Remote (Not On-Site)

When the Miyoo device is not nearby:

1. User downloads the compiled binary from GitHub releases
2. User tests on physical device
3. User reports back with test results and debug.log contents
4. Debug issues based on reported logs

### GitHub Actions Build & Release Workflow

**IMPORTANT: Every commit updates the SAME release until validated as working**

#### For Every Code Change:

1. **Commit and Push**
   ```bash
   git add .
   git commit -m "Description of changes"
   git push origin main
   ```

2. **Monitor GitHub Actions**
   - Check that all build jobs complete successfully
   - If builds fail, fix immediately and push again
   - Use sleep/polling to verify completion:
   ```bash
   # Get the latest run ID
   run_id=$(gh run list --limit 1 --json databaseId --jq '.[0].databaseId')

   # Poll until completion (check every 30s)
   while true; do
     status=$(gh run view $run_id --json status --jq '.status')
     echo "Build status: $status"
     if [ "$status" = "completed" ]; then
       conclusion=$(gh run view $run_id --json conclusion --jq '.conclusion')
       echo "Build $conclusion"
       break
     fi
     sleep 30
   done
   ```

3. **Verify Release Artifacts**
   - Ensure Miyoo ARM build artifact is created
   - **DO NOT build Linux/macOS builds** (not necessary for this project)
   - Verify artifact is pushed to the release (not just created)
   - Each commit should update the SAME release

4. **Release Version Management**
   - Keep re-using the SAME release (e.g., v0.5.0-testing) for all test iterations
   - Only create a NEW release version when user validates it as working
   - Format: `v0.X.X` for stable releases, `v0.X.X-testing` for development

#### Creating a New Stable Release

Only after user confirms a build is working:

```bash
git tag v0.X.X
git push --tags
```

This creates a new stable release, then continue using `v0.X.X-testing` for the next development cycle.

### Issue Tracking

All bugs and issues must be documented in the `/issues/` folder:

- Create a new markdown file for each issue: `issues/issue-description.md`
- Document:
  - Problem description
  - Expected behavior
  - Each attempted solution with outcome
  - Final resolution (when fixed)
- Update the issue file every time you try a new approach
- **NEVER repeat the same failed solution** - always check the issue file first

Example issue file structure:

```markdown
# Issue: Black Screen on Miyoo Device

## Problem
App launches but Miyoo screen remains black. Desktop build works fine.

## Expected Behavior
App should display on physical Miyoo screen.

## Attempted Solutions

### Attempt 1: SDL Renderer Flags (FAILED)
- Changed to SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
- Outcome: Still black screen
- Debug log: Shows "software" renderer being used

### Attempt 2: Set SDL_MMIYOO_DOUBLE_BUFFER in C code (TESTING)
- Added SDL_setenv("SDL_MMIYOO_DOUBLE_BUFFER", "1", 1) before SDL_Init()
- Commit: abc123
- Waiting for user to test...

## Resolution
TBD
```

### Building from Source

#### GitHub Actions (Recommended for Miyoo ARM builds)

All commits to `main` trigger builds, but releases are only created on version tags:

```bash
# Create a new release
git tag v0.X.X
git push --tags

# GitHub Actions will:
# 1. Build for Miyoo ARM, Linux x86_64, and macOS
# 2. Create/update the release with all artifacts
# 3. Attach HACompanion-miyoo-v0.X.X.zip for Miyoo users
```

#### Local Cross-Compilation (Advanced)

**Option 1: Using OnionUI Official Toolchain (Recommended for Local Development)**

The OnionUI project provides an official Docker-based toolchain with pre-built Miyoo SDK:

```bash
# Clone OnionUI toolchain
git clone https://github.com/OnionUI/dev-miyoomini-toolchain.git
cd dev-miyoomini-toolchain

# Build the toolchain Docker image (one-time setup)
docker build -t mmiyoo-toolchain .

# Use the toolchain to build our app
cd /path/to/hass-miyoo
docker run --rm -v $(pwd):/root/workspace mmiyoo-toolchain /bin/bash -c "
  cd /root/workspace &&
  mkdir -p build-miyoo &&
  cd build-miyoo &&
  cmake .. &&
  make
"

# Output: build-miyoo/hacompanion (ARM binary ready for Miyoo)
```

**Benefits:**
- Pre-configured Miyoo SDK with all libraries
- Consistent build environment
- Same toolchain used by OnionUI developers
- No manual dependency compilation

**Option 2: Manual Cross-Compilation (What GitHub Actions Uses)**

```bash
# Install ARM toolchain
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# Clone repository
git clone https://github.com/christag/hass-miyoo.git
cd hass-miyoo

# Build dependencies (complex - see .github/workflows/build.yml for full steps)
# Requires building: cJSON, SQLite, SDL2, Freetype, SDL2_ttf, SDL2_image, OpenSSL, libcurl

# Build the app
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-toolchain.cmake ..
make

# Output: build/hacompanion (ARM binary)
```

**Note:** Manual cross-compilation is complex due to dependency building. The GitHub Actions workflow or OnionUI toolchain are recommended.

### Desktop Development Build (for testing)

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \
                     libcurl4-openssl-dev libsqlite3-dev

# Build for x86_64
mkdir build-desktop && cd build-desktop
cmake ..
make

# Run locally
./hacompanion
```

### Development Environment Variables

For local testing, create a `.env` file in the project root with your Home Assistant credentials:

```bash
HASS_API_KEY=your_long_lived_access_token
HASS_HTTPS=true
HASS_HOST=your-ha-instance.example.com
HASS_PORT=443
```

Test environment values:
- `HASS_HOST`: home.christagliaferro.com
- `HASS_PORT`: 443
- `HASS_HTTPS`: true
- `HASS_API_KEY`: (see .env file in project root)

**Note:** The `.env` file is gitignored and contains real credentials for testing.

## Miyoo Mini Plus SDL2 Configuration (CRITICAL)

### Correct SDL2 Initialization Sequence

**CRITICAL: `SDL_MMIYOO_DOUBLE_BUFFER` MUST be set in C code BEFORE `SDL_Init()`**

```c
// CORRECT - This is the pattern ALL working Miyoo SDL2 apps use
static int init_sdl(app_state_t *app) {
    // CRITICAL: Set double buffer flag BEFORE SDL_Init()
    // Setting this in shell scripts does NOT work reliably!
    SDL_setenv("SDL_MMIYOO_DOUBLE_BUFFER", "1", 1);

    // Now initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        // error handling
    }

    // Create window
    app->window = SDL_CreateWindow(...);

    // ALWAYS use ACCELERATED renderer for Miyoo
    app->renderer = SDL_CreateRenderer(
        app->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
}
```

**NEVER use:**
- ❌ `SDL_RENDERER_SOFTWARE` - Will result in black screen
- ❌ `flags = 0` (auto-select) - SDL chooses SOFTWARE, which doesn't work
- ❌ Setting `SDL_MMIYOO_DOUBLE_BUFFER` only in shell scripts - Not reliable!

### Required Environment Variables

Set in launch scripts:

```bash
# launch.sh
export SDL_VIDEODRIVER=mmiyoo              # Use Miyoo's custom video driver
export LD_LIBRARY_PATH=/path/to/libs:$LD_LIBRARY_PATH  # Load bundled Miyoo SDL2 library

# NOTE: SDL_MMIYOO_DOUBLE_BUFFER is set in C code via SDL_setenv() BEFORE SDL_Init()
# Setting it here in the shell script is NOT sufficient!
```

### Research-Backed Configuration

This configuration is based on analysis of 5 working Miyoo SDL2 projects:

1. **steward-fu/sdl2** - `ACCELERATED | PRESENTVSYNC`
2. **XK9274/miyoo_sdl2_benchmarks** - `ACCELERATED` + `SDL_MMIYOO_DOUBLE_BUFFER=1`
3. **lanmarc77/tinamp** - `ACCELERATED`
4. **shauninman/MinUI** - `ACCELERATED | PRESENTVSYNC`
5. Multiple other Miyoo apps - All use ACCELERATED

### Why ACCELERATED is Required

The Miyoo's custom MMIYOO video driver is a hardware-accelerated backend that directly renders to the framebuffer. It does NOT work with SDL's software renderer. When you pass `flags=0`, SDL chooses the SOFTWARE renderer (flags=13 = SOFTWARE + PRESENTVSYNC + TARGETTEXTURE), which cannot communicate with the MMIYOO driver.

### Debug Log Analysis

**Symptoms of incorrect configuration:**
- `SDL Renderer: software` with `Flags: 13` = SOFTWARE renderer fallback
- Physical screen remains black despite app rendering frames
- All other initialization appears successful (fonts, entities, screens created)

**Correct configuration shows:**
- `SDL Renderer: MMIYOO` (or accelerated renderer name)
- `Flags: 10` (ACCELERATED=2 + PRESENTVSYNC=8)
- Visual output on physical screen

### Troubleshooting Black Screen Issues

If you encounter a black screen on Miyoo hardware:

1. **Check debug.log for renderer type**
   - If it shows "software" renderer, the SDL_MMIYOO_DOUBLE_BUFFER flag wasn't set properly
   - Solution: Ensure `SDL_setenv("SDL_MMIYOO_DOUBLE_BUFFER", "1", 1)` is called BEFORE `SDL_Init()`

2. **Verify SDL renderer flags**
   - Should be `SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC`
   - Never use `SDL_RENDERER_SOFTWARE` or `flags=0`

3. **Check environment variables in launch script**
   - `SDL_VIDEODRIVER=mmiyoo` must be set
   - `LD_LIBRARY_PATH` must include the bundled SDL2 library path

4. **Common mistakes:**
   - ❌ Only setting SDL_MMIYOO_DOUBLE_BUFFER in shell script (not in C code)
   - ❌ Setting the flag AFTER SDL_Init() instead of BEFORE
   - ❌ Using auto-detect renderer flags (flags=0) instead of explicitly setting ACCELERATED
   - ❌ Forgetting to bundle the Miyoo SDL2 library (libSDL2-2.0.so.0)

### Bundled SDL2 Library

The Miyoo requires a custom SDL2 build with the MMIYOO video driver. We bundle this library from OnionOS:
- **Location**: `libs/miyoo/libSDL2-2.0.so.0`
- **Source**: `/mnt/SDCARD/.tmp_update/lib/parasyte/libSDL2-2.0.so.0` on Miyoo
- **Size**: ~5.2MB
- **Why**: Our statically-built SDL2 doesn't include the MMIYOO driver

The launch scripts use `LD_LIBRARY_PATH` to load this bundled library at runtime.

## Contributing

Contributions welcome! Please see:
- [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines
- [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) for community standards
- Phase planning documents for implementation roadmap

## Roadmap (Post-MVP)

- **v1.1**: WebSocket support for real-time updates
- **v1.2**: Text input screen for in-app server configuration
- **v1.3**: Custom dashboard layouts
- **v1.4**: Camera view support
- **v2.0**: Automation/script editing
- **v2.1**: Energy dashboard
- **v2.2**: Graph/history views

## License

MIT License - see [LICENSE](LICENSE) file

## Credits

- Built for [Miyoo Mini Plus](https://www.miyoogame.com/) handheld console
- Powered by [Home Assistant](https://www.home-assistant.io/)
- Uses [SDL2](https://www.libsdl.org/), [libcurl](https://curl.se/), [cJSON](https://github.com/DaveGamble/cJSON), [SQLite](https://www.sqlite.org/)
- Pixel art inspiration from Game Boy, SNES, and retro gaming classics

## Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/hass-miyoo/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/hass-miyoo/discussions)
- **Community**: [Miyoo Mini Discord](https://discord.gg/miyoo), [r/MiyooMini](https://reddit.com/r/MiyooMini)

---

**Status**: ✅ v1.0.0 Complete - All 15 Phases Done

Last Updated: 2025-11-21
