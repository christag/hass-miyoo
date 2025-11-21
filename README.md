# Home Assistant Companion for Miyoo Mini Plus

Control your smart home from a retro gaming handheld! This native companion app brings Home Assistant's powerful automation platform to the Miyoo Mini Plus with a nostalgic Game Boy-inspired interface.

![Version](https://img.shields.io/badge/Version-1.0.0-green)
![License](https://img.shields.io/badge/License-MIT-blue)
![Platform](https://img.shields.io/badge/Platform-Miyoo%20Mini%20Plus-orange)

---

## Features

- **Multi-Server Support**: Configure and switch between multiple Home Assistant instances
- **Three View Modes**: Browse by Domain, Room, or Favorites
- **Entity Control**: Toggle lights, switches, fans, activate scenes, run scripts
- **Favorites Dashboard**: Star your most-used entities for quick access
- **Offline Mode**: View cached data when disconnected
- **Background Sync**: Automatic refresh every 60 seconds
- **Retro UI**: Game Boy DMG-01 color palette with pixel font

---

## Controller Layout

| Button | Action |
|--------|--------|
| **D-Pad** | Navigate menus and lists |
| **A** | Toggle / Activate / Confirm |
| **B** | Back / Cancel |
| **X** | Toggle view mode (Domain/Room/Favorites) |
| **Y** | Toggle favorite status |
| **L1/R1** | Switch tabs within view |
| **SELECT** | View entity details |
| **START** | Manual sync/refresh |
| **MENU** | Exit (with confirmation) |

---

## Installation

### Prerequisites

1. **Miyoo Mini Plus** with OnionOS installed
2. **WiFi** connection configured on Miyoo
3. **Home Assistant** server accessible on your network
4. **Long-lived access token** from Home Assistant

### Quick Start

1. **Download** the latest release from [GitHub Releases](https://github.com/yourusername/hass-miyoo/releases)

2. **Extract** to `/mnt/SDCARD/App/HACompanion/` on your Miyoo SD card

3. **Configure** your Home Assistant server:
   ```bash
   cd /mnt/SDCARD/App/HACompanion/
   cp servers.example.json servers.json
   # Edit servers.json with your server details
   ```

4. **Launch** "HA Companion" from OnionOS Apps menu

### Getting a Long-Lived Access Token

1. Open Home Assistant web interface
2. Click your profile (bottom left)
3. Scroll to "Long-Lived Access Tokens"
4. Click "Create Token"
5. Name it "Miyoo Companion" and copy the token
6. Paste into `servers.json`

---

## Configuration

Create/edit `servers.json`:

```json
{
  "servers": [
    {
      "name": "Home",
      "url": "http://homeassistant.local",
      "port": 8123,
      "token": "your_long_lived_access_token_here"
    }
  ],
  "default_server": 0
}
```

**Fields**:
- `name`: Display name for this server
- `url`: Home Assistant URL (local IP or domain)
- `port`: Home Assistant port (usually 8123)
- `token`: Long-lived access token
- `default_server`: Index of default server (0 = first)

---

## Supported Entity Types

| Entity Type | Controls |
|-------------|----------|
| Lights | Toggle on/off |
| Switches | Toggle on/off |
| Fans | Toggle on/off |
| Scenes | Activate |
| Scripts | Run |
| Automations | Trigger |
| Buttons | Press |
| Humidifiers | Toggle |
| Sensors | View state |
| Binary Sensors | View state |
| Climate | View state |
| Select | View state |

---

## Building from Source

### Desktop Build (for testing)

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \
                     libcurl4-openssl-dev libsqlite3-dev libcjson-dev

# Build
mkdir build && cd build
cmake ..
make
./hacompanion
```

### ARM Build (for Miyoo)

```bash
mkdir build-arm && cd build-arm
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-toolchain.cmake ..
make
# Copy hacompanion to Miyoo SD card
```

---

## Design

### Game Boy DMG-01 Palette

| Color | Hex | Usage |
|-------|-----|-------|
| Darkest | `#0f380f` | Background |
| Dark | `#306230` | Panels, borders |
| Light | `#8bac0f` | Selection highlight |
| Lightest | `#9bbc0f` | Text, icons |

### Typography

- **Font**: Press Start 2P (pixel font)
- **Sizes**: 16px headers, 12px body, 8px small

---

## Technical Stack

| Component | Technology |
|-----------|-----------|
| Language | C99 |
| UI Framework | SDL2 + SDL2_image + SDL2_ttf |
| HTTP Client | libcurl |
| JSON Parser | cJSON |
| Database | SQLite3 |
| Platform | Miyoo Mini Plus (ARM, OnionOS) |
| Display | 640x480 @ 60Hz |
| Build System | CMake |

---

## Project Structure

```
hass-miyoo/
├── src/
│   ├── main.c              # Application entry point
│   ├── ha_client.c/h       # Home Assistant API client
│   ├── database.c/h        # SQLite cache
│   ├── cache_manager.c/h   # Sync and offline mode
│   ├── ui/                 # UI components
│   │   ├── colors.h        # Game Boy palette
│   │   ├── fonts.c/h       # Font management
│   │   ├── icons.c/h       # Icon rendering
│   │   └── components.c/h  # UI widgets
│   ├── screens/            # Screen implementations
│   └── utils/              # Helpers (input, config, JSON)
├── assets/
│   └── fonts/              # Press Start 2P font
├── planning/               # Phase documentation
├── servers.example.json    # Configuration template
├── CMakeLists.txt          # Build configuration
└── README.md
```

---

## Roadmap

### v1.1
- WebSocket support for real-time updates
- Sound effects

### v1.2
- In-app text input for configuration
- Theme selector (Game Boy / SNES / NES)

### v1.3
- Camera view support
- History graphs for sensors

### v2.0
- Automation editing
- Custom dashboards

---

## Contributing

Contributions welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md).

---

## License

MIT License - see [LICENSE](LICENSE)

---

## Credits

- Built for [Miyoo Mini Plus](https://www.miyoogame.com/) with OnionOS
- Powered by [Home Assistant](https://www.home-assistant.io/)
- Libraries: SDL2, libcurl, cJSON, SQLite
- Font: Press Start 2P (Google Fonts)

---

## Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/hass-miyoo/issues)
- **Community**: [r/MiyooMini](https://reddit.com/r/MiyooMini), [Miyoo Discord](https://discord.gg/miyoo)
