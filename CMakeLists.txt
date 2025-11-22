cmake_minimum_required(VERSION 3.10)
project(HACompanion C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Use pkg-config to find SDL2 libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})
include_directories(src)

# Library directories
link_directories(${SDL2_LIBRARY_DIRS})
link_directories(${SDL2_IMAGE_LIBRARY_DIRS})
link_directories(${SDL2_TTF_LIBRARY_DIRS})

# Source files
set(SOURCES
    src/main.c
    src/utils/input.c
    src/utils/json_helpers.c
    src/utils/config.c
    src/ha_client.c
    src/database.c
    src/cache_manager.c
    src/ui/fonts.c
    src/ui/components.c
    src/ui/icons.c
    src/screens/screen_test.c
    src/screens/screen_setup.c
    src/screens/screen_list.c
    src/screens/screen_device.c
    src/screens/screen_info.c
    src/screens/screen_automation.c
    src/screens/screen_script.c
    src/screens/screen_scene.c
)

# Executable
add_executable(hacompanion ${SOURCES})

# Link libraries
target_link_libraries(hacompanion
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    curl
    ssl
    crypto
    dl
    cjson
    sqlite3
    m
)

# Compiler warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(hacompanion PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install target for deployment
install(TARGETS hacompanion DESTINATION bin)
