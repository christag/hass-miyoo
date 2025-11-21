# Debugging on Miyoo Mini Plus

## Quick Debug Steps

If the app crashes on your Miyoo device, follow these steps to gather debug information:

### Method 1: Check the debug.log file

After the app crashes:
1. Connect Miyoo SD card to your computer
2. Navigate to `/mnt/SDCARD/App/HACompanion/`
3. Check if `debug.log` exists and read its contents
4. Share the contents for troubleshooting

### Method 2: SSH/Telnet Debug (Advanced)

If you have SSH or telnet access to your Miyoo:

```bash
# Connect via SSH (if configured)
ssh root@<miyoo-ip>

# Navigate to app directory
cd /mnt/SDCARD/App/HACompanion

# Run the debug launch script
./launch_debug.sh

# Check the debug.log after crash
cat debug.log
```

### Method 3: Manual Debug Launch

```bash
# SSH into Miyoo
cd /mnt/SDCARD/App/HACompanion

# Check if binary exists
ls -la hacompanion

# Check library dependencies
ldd hacompanion

# Try running directly
./hacompanion 2>&1 | tee manual_debug.log
```

## Common Issues on Miyoo

### 1. Missing Library Dependencies
If `ldd` shows "not found" for any libraries, the ARM build may be missing static linking.

**Check for:**
- SDL2
- SDL2_ttf
- SDL2_image
- libcurl
- sqlite3
- cJSON

### 2. Video Driver Issues
The Miyoo uses a specific SDL2 video driver. Try setting:
```bash
export SDL_VIDEODRIVER=mmiyoo
./hacompanion
```

### 3. File Permissions
Ensure executable permissions:
```bash
chmod +x hacompanion
chmod +x launch.sh
```

### 4. Missing Assets
Check that all required files are present:
```bash
ls -la assets/fonts/
ls -la assets/icons/
```

Required:
- `assets/fonts/PressStart2P.ttf`
- Icon files (though these have fallbacks)

### 5. Configuration File
Make sure `servers.json` exists and is valid JSON:
```bash
cat servers.json
```

## Collecting Debug Info

When reporting a crash, please provide:

1. **Contents of debug.log** (or manual_debug.log)
2. **Output of `ldd hacompanion`**
3. **OnionOS version** (Settings > About)
4. **Miyoo Mini Plus variant** (v1, v2, v3, or Plus)
5. **File listing**: `ls -la /mnt/SDCARD/App/HACompanion/`

## Known Working Configuration

The app has been tested and works on:
- **Device**: Miyoo Mini Plus
- **OS**: OnionOS 4.3.x
- **Architecture**: ARMv7 with NEON and hard-float ABI

## Next Steps

If none of the above helps:

1. Create a GitHub issue with all debug output
2. Include the exact steps to reproduce
3. Mention if the loading screen appears before the crash
4. Note if other SDL2 apps work on your device
