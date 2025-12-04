#!/bin/sh
# Minimal SDL2 Display Test - Uses Miyoo toolchain-built SDL2
#
# This test uses SDL2 compiled with the official miyoocfw/toolchain,
# which has proper MMIYOO video driver support and correct ABI.

cd /mnt/SDCARD/App/test_display

# Set all Miyoo-specific drivers
export SDL_VIDEODRIVER=mmiyoo
export SDL_AUDIODRIVER=mmiyoo
export EGL_VIDEODRIVER=mmiyoo

# Use our bundled toolchain SDL2 library FIRST, then fall back to system
export LD_LIBRARY_PATH="./lib:/mnt/SDCARD/miyoo/lib:/mnt/SDCARD/.tmp_update/lib:/mnt/SDCARD/.tmp_update/lib/parasyte:$LD_LIBRARY_PATH"

echo "=== Test Display Launcher ==="
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
echo "SDL_VIDEODRIVER: $SDL_VIDEODRIVER"
echo ""

# Run the test - output goes to both screen and log file
./test_display 2>&1 | tee test_display.log

echo ""
echo "Test complete. Check test_display.log for output."
