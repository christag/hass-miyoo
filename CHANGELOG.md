# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-11-21

### Added
- Multi-server configuration via `servers.json`
- Domain view: Browse entities by type (lights, sensors, climate, fans, switches, etc.)
- Room view: Browse entities by Home Assistant area/room assignment
- Favorites view: Quick access to starred entities
- Entity controls: Toggle lights/switches/fans, activate scenes/scripts, press buttons
- Detail screens for all entity types with relevant controls
- Offline mode with SQLite caching
- Background sync every 60 seconds when online
- Exit confirmation dialog
- Retro Game Boy DMG-01 aesthetic with Press Start 2P font

### Controls
- **D-Pad**: Navigate menus and lists
- **A**: Toggle / Activate / Confirm
- **B**: Back / Cancel
- **X**: Toggle view mode (Domain/Room/Favorites)
- **Y**: Toggle favorite status
- **L1/R1**: Switch tabs within view
- **SELECT**: View entity details
- **START**: Manual sync/refresh
- **MENU**: Exit (with confirmation)

### Entity Types Supported
- Lights (toggle on/off)
- Switches (toggle on/off)
- Fans (toggle on/off)
- Sensors (view state)
- Binary sensors (view state)
- Climate (view state)
- Scenes (activate)
- Scripts (run)
- Automations (trigger)
- Buttons (press)
- Humidifiers (toggle)
- Select entities (view state)

### Known Limitations
- Text input not supported (edit `servers.json` manually)
- No WebSocket support (REST API polling only)
- No automation/script editing
- No camera view support
- Climate control limited to viewing state

## [Unreleased]

### Planned
- WebSocket support for real-time entity updates
- In-app text input for server configuration
- Sound effects (navigation, toggle, error)
- Camera view support (MJPEG streams)
- Theme selector (Game Boy / SNES / NES palettes)
- History graphs for sensors
- More entity type controls (cover, lock, media_player)
