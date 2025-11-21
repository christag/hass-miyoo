# Contributing to Home Assistant Companion for Miyoo

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## Getting Started

### Prerequisites

- C compiler (GCC or Clang)
- CMake 3.10+
- SDL2 development libraries
- libcurl development libraries
- cJSON development libraries
- SQLite3 development libraries

### Setting Up Development Environment

```bash
# Clone the repository
git clone https://github.com/yourusername/hass-miyoo.git
cd hass-miyoo

# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential cmake \
    libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \
    libcurl4-openssl-dev libsqlite3-dev libcjson-dev

# Build for desktop testing
mkdir build && cd build
cmake ..
make

# Run the app
./hacompanion
```

### Configuration for Testing

Copy `servers.example.json` to `servers.json` and add your Home Assistant credentials:

```bash
cp servers.example.json servers.json
# Edit servers.json with your server details
```

## Code Style

### C Style Guidelines

- Use C99 standard
- 4-space indentation (no tabs)
- Opening braces on same line
- Maximum line length: 100 characters
- Use snake_case for functions and variables
- Use UPPER_CASE for macros and constants

### Example

```c
/**
 * Brief description of function
 *
 * @param param1 Description
 * @return Description
 */
int function_name(int param1) {
    if (param1 > 0) {
        return do_something(param1);
    }
    return 0;
}
```

### Comments

- Use `//` for single-line comments
- Use `/** */` for function documentation
- Document all public functions in headers

## Project Structure

```
src/
├── main.c              # Entry point, main loop
├── ha_client.c/h       # Home Assistant API
├── database.c/h        # SQLite operations
├── cache_manager.c/h   # Sync logic
├── ui/                 # UI components
│   ├── colors.h        # Color definitions
│   ├── fonts.c/h       # Font loading
│   ├── icons.c/h       # Icon rendering
│   └── components.c/h  # Reusable widgets
├── screens/            # Screen implementations
│   ├── screen_setup.c/h
│   ├── screen_list.c/h
│   ├── screen_device.c/h
│   └── ...
└── utils/              # Utilities
    ├── input.c/h       # Input handling
    ├── config.c/h      # Configuration
    └── json_helpers.c/h
```

## Making Changes

### Branch Naming

- `feature/description` - New features
- `fix/description` - Bug fixes
- `docs/description` - Documentation
- `refactor/description` - Code refactoring

### Commit Messages

Use clear, descriptive commit messages:

```
type: brief description

Longer description if needed. Explain what and why,
not how (the code shows how).

Fixes #123
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

### Pull Requests

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on desktop (and Miyoo if possible)
5. Submit a pull request

Include in your PR:
- Description of changes
- Screenshots (for UI changes)
- Testing performed

## Adding New Features

### Adding a New Entity Type

1. Add domain to `MVP_DOMAINS` in `screen_list.c` if needed
2. Add icon mapping in `icons.c`
3. Handle in `list_screen_toggle_selected()` if controllable
4. Create detail screen if needed

### Adding a New Screen

1. Create `screen_name.c` and `screen_name.h` in `src/screens/`
2. Follow the pattern of existing screens:
   - `*_create()` - Allocate and initialize
   - `*_destroy()` - Free resources
   - `*_handle_input()` - Process input, return navigation hints
   - `*_render()` - Draw the screen
3. Add to `main.c` screen routing

## Testing

### Desktop Testing

Use keyboard mappings:
| Button | Key |
|--------|-----|
| A | Space |
| B | Left Ctrl |
| X | Left Shift |
| Y | Left Alt |
| L1 | E |
| R1 | T |
| Select | Right Ctrl |
| Start | Enter |
| Menu | Escape |
| D-Pad | Arrow keys |

### Hardware Testing

If you have a Miyoo Mini Plus:
1. Build with ARM toolchain
2. Copy binary to SD card
3. Test all functionality

## Reporting Issues

When reporting bugs, include:
- Steps to reproduce
- Expected vs actual behavior
- Home Assistant version
- Miyoo firmware/OnionOS version
- Error messages or screenshots

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
