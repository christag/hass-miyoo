# Miyoo SDL2 Library

This directory contains the Miyoo Mini Plus native SDL2 library with the custom MMIYOO video driver.

## Files

- `libSDL2-2.0.so.0` - SDL2 shared library with MMIYOO video backend (5.0MB)

## Source

Extracted from Miyoo Mini Plus OnionOS:
- Path: `/mnt/SDCARD/.tmp_update/lib/parasyte/libSDL2-2.0.so.0`
- OnionOS Version: Latest (as of 2025-11-22)

## Purpose

The Miyoo Mini Plus requires a custom SDL2 video driver called "MMIYOO" to render to the device's framebuffer. This library is bundled with our application to ensure proper video output on the Miyoo hardware.

Our statically-built SDL2 from source doesn't include this custom driver, so we bundle the Miyoo's system library instead.

## Usage

The build workflow automatically copies this library to `HACompanion/lib/` in the release package. The launch scripts set `LD_LIBRARY_PATH` to load this library at runtime.

## License

SDL2 is licensed under the zlib license. This is a binary distribution from the Miyoo OnionOS team.
