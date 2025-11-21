# Development Environment Setup

Complete guide for setting up the Home Assistant Companion development environment for Miyoo Mini Plus.

## Table of Contents

- [macOS Setup](#macos-setup)
- [Linux Setup](#linux-setup)
- [Windows (WSL2) Setup](#windows-wsl2-setup)
- [Desktop Development Build](#desktop-development-build)
- [ARM Cross-Compilation](#arm-cross-compilation)
- [Deploying to Miyoo](#deploying-to-miyoo)
- [Troubleshooting](#troubleshooting)

---

## macOS Setup

### Prerequisites

- macOS 11 (Big Sur) or later
- Homebrew package manager
- Xcode Command Line Tools

### 1. Install Xcode Command Line Tools

```bash
xcode-select --install
```

### 2. Install SDL2 Libraries

```bash
brew install sdl2 sdl2_image sdl2_ttf
brew install cmake
```

### 3. Install Additional Libraries

```bash
# For future phases (API client, database)
brew install curl sqlite3
```

### 4. Install ARM Cross-Compilation Toolchain

macOS uses the native `container` command for Linux compatibility:

```bash
# Create a container environment for ARM cross-compilation
container create --name miyoo-build ubuntu:22.04

# Enter the container
container exec -it miyoo-build bash

# Inside the container, install ARM toolchain
apt-get update
apt-get install -y gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
    cmake build-essential git curl

# Install ARM SDL2 libraries (cross-compiled)
# See "ARM Cross-Compilation" section below
```

### 5. Verify Installation

```bash
# Check SDL2 installation
sdl2-config --version  # Should show 2.x.x

# Check CMake
cmake --version  # Should show 3.10 or later
```

---

## Linux Setup

### Ubuntu/Debian

```bash
# Install development tools
sudo apt-get update
sudo apt-get install -y build-essential cmake git

# Install SDL2 libraries (native)
sudo apt-get install -y libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev

# Install additional libraries
sudo apt-get install -y libcurl4-openssl-dev libsqlite3-dev

# Install ARM cross-compilation toolchain
sudo apt-get install -y gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

### Fedora/RHEL

```bash
# Install development tools
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git

# Install SDL2 libraries
sudo dnf install SDL2-devel SDL2_image-devel SDL2_ttf-devel

# Install additional libraries
sudo dnf install libcurl-devel sqlite-devel

# Install ARM cross-compilation toolchain
sudo dnf install gcc-arm-linux-gnu g++-arm-linux-gnu
```

---

## Windows (WSL2) Setup

### 1. Install WSL2

```powershell
# In PowerShell (Administrator)
wsl --install -d Ubuntu-22.04
```

### 2. Follow Linux Setup

Once WSL2 is installed, follow the Ubuntu/Debian setup instructions above.

---

## Desktop Development Build

Build and test the application on your local machine (x86_64/ARM64) for rapid iteration.

### 1. Create Build Directory

```bash
cd /path/to/hass-miyoo
mkdir build-desktop
cd build-desktop
```

### 2. Configure CMake

```bash
cmake ..
```

### 3. Build

```bash
make -j$(nproc)
```

### 4. Run

```bash
./hacompanion
```

**Controls (Desktop)**:
- **Arrow Keys**: D-Pad navigation
- **Space**: A button (confirm)
- **Left Ctrl**: B button (back)
- **Escape**: Menu button (exit)
- **E**: L1 button
- **T**: R1 button
- **Enter**: Start button
- **Right Ctrl**: Select button

### 5. Test

```bash
# Run with valgrind to check for memory leaks
valgrind --leak-check=full ./hacompanion
```

---

## ARM Cross-Compilation

Build binaries for Miyoo Mini Plus (ARM Cortex-A7).

### Prerequisites

You need ARM versions of SDL2 libraries. You can either:

1. **Cross-compile SDL2 yourself** (recommended for full control)
2. **Use pre-built ARM libraries** (faster but less flexible)

### Option 1: Cross-Compile SDL2 for ARM

```bash
# Create a directory for ARM libraries
mkdir -p ~/arm-sysroot/usr/local

# Download and build SDL2
wget https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-2.32.10.tar.gz
tar xzf SDL2-2.32.10.tar.gz
cd SDL2-2.32.10

./configure --host=arm-linux-gnueabihf --prefix=$HOME/arm-sysroot/usr/local
make -j$(nproc)
make install

# Repeat for SDL2_image, SDL2_ttf
```

### Option 2: Using Container Environment (macOS)

```bash
# On macOS, use the container created earlier
container exec -it miyoo-build bash

# Inside container, follow Linux ARM cross-compilation steps
```

### Build for ARM

```bash
cd /path/to/hass-miyoo
mkdir build-arm
cd build-arm

# Configure with ARM toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-toolchain.cmake \
      -DCMAKE_PREFIX_PATH=$HOME/arm-sysroot/usr/local \
      ..

# Build
make -j$(nproc)

# Verify it's an ARM binary
file hacompanion
# Output should show: "ARM, EABI5 version 1 (SYSV), dynamically linked..."
```

---

## Deploying to Miyoo

### 1. Prepare SD Card

Your Miyoo SD card should be accessible at a mount point (e.g., `/Volumes/SDCARD` on macOS).

### 2. Create App Directory

```bash
# On your Miyoo SD card
mkdir -p /Volumes/SDCARD/App/HACompanion
```

### 3. Copy Files

```bash
# Copy binary
cp build-arm/hacompanion /Volumes/SDCARD/App/HACompanion/

# Copy launch script and icon
cp dist/HACompanion/launch.sh /Volumes/SDCARD/App/HACompanion/
cp dist/HACompanion/icon.png /Volumes/SDCARD/App/HACompanion/

# Copy configuration template
cp dist/HACompanion/servers.json.example /Volumes/SDCARD/App/HACompanion/

# Make launch script executable
chmod +x /Volumes/SDCARD/App/HACompanion/launch.sh
```

### 4. Configure Server

On the Miyoo, create `servers.json` from the example:

```bash
# Option 1: Edit directly on SD card (if you have a text editor)
cd /mnt/SDCARD/App/HACompanion
cp servers.json.example servers.json
vi servers.json

# Option 2: Create on your computer and copy
cp dist/HACompanion/servers.json.example servers.json
# Edit servers.json with your Home Assistant details
# Copy to /Volumes/SDCARD/App/HACompanion/servers.json
```

**servers.json example**:
```json
{
  "servers": [
    {
      "name": "Home",
      "url": "http://192.168.1.100",
      "port": 8123,
      "token": "YOUR_LONG_LIVED_ACCESS_TOKEN",
      "username": "admin"
    }
  ],
  "default_server": 0
}
```

### 5. Test on Miyoo

1. Safely eject SD card from computer
2. Insert SD card into Miyoo Mini Plus
3. Power on device
4. Navigate to Apps section in OnionOS
5. Launch "HA Companion"
6. Verify:
   - App launches without errors
   - 640x480 window displays
   - Game Boy green background shows
   - Menu button exits app

---

## Troubleshooting

### SDL2 Headers Not Found

**Error**: `fatal error: SDL.h: No such file or directory`

**Solution**:
```bash
# macOS
brew install sdl2 sdl2_image sdl2_ttf

# Linux
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

### CMake Can't Find SDL2

**Error**: `Could not find SDL2`

**Solution**:
```bash
# Set SDL2_DIR environment variable
export SDL2_DIR=/opt/homebrew/lib/cmake/SDL2  # macOS
export SDL2_DIR=/usr/lib/x86_64-linux-gnu/cmake/SDL2  # Linux
```

### ARM Binary Won't Run on Miyoo

**Error**: Binary crashes or doesn't launch

**Checklist**:
1. Verify it's an ARM binary: `file hacompanion`
2. Check `launch.sh` has `SDL_VIDEODRIVER=mmiyoo`
3. Verify `launch.sh` is executable
4. Check OnionOS logs: `/mnt/SDCARD/App/HACompanion/log.txt`

### Black Screen on Miyoo

**Possible Causes**:
- SDL video driver not set correctly
- Missing SDL2 libraries on device
- Incorrect window creation

**Solution**:
1. Verify `export SDL_VIDEODRIVER=mmiyoo` in `launch.sh`
2. Test on desktop first to rule out code issues
3. Check OnionOS version compatibility

### Permission Denied on launch.sh

**Error**: `Permission denied` when launching

**Solution**:
```bash
chmod +x /Volumes/SDCARD/App/HACompanion/launch.sh
```

### Memory Leaks Detected

**Error**: Valgrind reports memory leaks

**Solution**:
- Ensure all `SDL_Create*` have matching `SDL_Destroy*`
- Verify `TTF_Quit()`, `IMG_Quit()`, `SDL_Quit()` are called
- Check for missing `SDL_FreeSurface()` calls

---

## Next Steps

After successful setup:

1. **Phase 2**: Implement Home Assistant API client
2. **Phase 3**: Create SQLite database for caching
3. **Phase 4**: Build UI design system with components
4. **Phases 5-10**: Implement all application screens

See `/planning/` directory for detailed phase documentation.

---

## Additional Resources

- [SDL2 Documentation](https://wiki.libsdl.org/)
- [OnionOS Development Guide](https://github.com/OnionUI/Onion/wiki)
- [Miyoo Mini Plus Hardware Specs](../research/miyoo-development.md)
- [Home Assistant API Reference](https://developers.home-assistant.io/docs/api/rest/)

---

**Last Updated**: 2025-01-20
**Phase**: 1 (Project Setup)
