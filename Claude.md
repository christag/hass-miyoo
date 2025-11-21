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

## Building from Source

### Setup Cross-Compilation Toolchain

```bash
# Install ARM toolchain
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# Clone repository
git clone https://github.com/yourusername/hass-miyoo.git
cd hass-miyoo

# Build dependencies (SDL2, libcurl, cJSON, SQLite3 for ARM)
./scripts/build-deps.sh

# Build the app
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-toolchain.cmake ..
make

# Output: build/hacompanion (ARM binary)
```

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
