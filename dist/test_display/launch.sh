#!/bin/sh
# Minimal SDL2 Display Test - Uses Moonlight's library set
#
# This script uses Moonlight's complete library set (including EGL/GLES)
# to test if those libraries are required for display output.

cd /mnt/SDCARD/App/test_display

# Set all Miyoo-specific drivers
export SDL_VIDEODRIVER=mmiyoo
export SDL_AUDIODRIVER=mmiyoo
export EGL_VIDEODRIVER=mmiyoo

# Use Moonlight's FULL library set - the key test!
# Moonlight has EGL/GLES libraries that HACompanion is missing
export LD_LIBRARY_PATH="/mnt/SDCARD/App/moonlight/lib:/mnt/SDCARD/miyoo/lib:/mnt/SDCARD/.tmp_update/lib:/mnt/SDCARD/.tmp_update/lib/parasyte:$LD_LIBRARY_PATH"

echo "=== Test Display Launcher ==="
echo "Using Moonlight's library path: $LD_LIBRARY_PATH"
echo ""

# Run the test - output goes to both screen and log file
./test_display 2>&1 | tee test_display.log

echo ""
echo "Test complete. Check test_display.log for output."
