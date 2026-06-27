cmake_minimum_required(VERSION 3.18)

if(DEFINED ENV{PICO_SDK_PATH} AND NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif()

if(NOT DEFINED PICO_SDK_PATH)
    message(FATAL_ERROR "PICO_SDK_PATH is required, e.g. -DPICO_SDK_PATH=/path/to/pico-sdk")
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(picocalc_bvwc_volleyball C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# ---------------------------------------------------------------------------
# Game source selection.
#
# The game sketch itself is NOT bundled (its copyright is unclear, so it cannot
# be redistributed).  Instead the build fetches a single .ino from the upstream
# ESPboy repository at configure time and compiles it as C++.
#
#   * If you drop a local *.cpp into src/game/, that is compiled instead (no
#     download happens) - useful for offline work.
#   * Otherwise GAME_NAME selects which game to download.  Upstream layout is
#     GAMES/<NAME>/<NAME>.ino, so GAME_NAME is the folder/file base name.
#   * GAME_INO_URL overrides the URL entirely (for forks/other sources).
#
# Examples:
#   cmake -S . -B build -G Ninja
#   cmake -S . -B build -G Ninja -DGAME_NAME=Artillery
#   cmake -S . -B build -G Ninja -DGAME_NAME=CraitsFestiveFight
#
# NOTE: only single-file sketches are supported, and the sketch must use the
# subset of the Arduboy2/ArduboyTones API covered by src/compat/.  Games that
# pull in Sprites/ArduboyFX/etc. need those added to the compat layer first.
# ---------------------------------------------------------------------------
set(GAME_NAME "BVWCVolleyball_1.01" CACHE STRING "Upstream game name (GAMES/<NAME>/<NAME>.ino)")
set(GAME_INO_URL "" CACHE STRING "Full raw .ino URL (overrides GAME_NAME)")
set(GAME_REPO_RAW "https://raw.githubusercontent.com/ESPboy-edu/ESPboy_Arduboy2_lib/master/GAMES"
    CACHE STRING "Base raw URL for the upstream GAMES directory")

file(GLOB LOCAL_GAME_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.cpp)

if(LOCAL_GAME_SOURCES)
    set(GAME_SOURCES ${LOCAL_GAME_SOURCES})
    message(STATUS "Game source: local ${GAME_SOURCES}")
else()
    if(GAME_INO_URL STREQUAL "")
        set(_game_url "${GAME_REPO_RAW}/${GAME_NAME}/${GAME_NAME}.ino")
    else()
        set(_game_url "${GAME_INO_URL}")
    endif()
    set(_game_dst "${CMAKE_BINARY_DIR}/game_src/${GAME_NAME}.cpp")

    if(NOT EXISTS "${_game_dst}")
        message(STATUS "Downloading game sketch: ${_game_url}")
        file(DOWNLOAD "${_game_url}" "${_game_dst}"
            STATUS _dl_status
            TLS_VERIFY ON)
        list(GET _dl_status 0 _dl_code)
        list(GET _dl_status 1 _dl_msg)
        if(NOT _dl_code EQUAL 0)
            file(REMOVE "${_game_dst}")
            message(FATAL_ERROR "Failed to download '${_game_url}': ${_dl_msg}")
        endif()
        # A 404 still returns success with HTML; reject obviously wrong payloads.
        file(SIZE "${_game_dst}" _dl_size)
        if(_dl_size LESS 200)
            file(READ "${_game_dst}" _dl_head LIMIT 200)
            file(REMOVE "${_game_dst}")
            message(FATAL_ERROR "Downloaded sketch looks invalid (size=${_dl_size}): ${_dl_head}")
        endif()
    else()
        message(STATUS "Game source: cached ${_game_dst}")
    endif()
    set(GAME_SOURCES "${_game_dst}")
endif()

add_executable(picocalc_bvwc_volleyball
    src/main.cpp
    ${GAME_SOURCES}
    src/compat/Arduboy2.cpp
    src/compat/ArduboyTones.cpp
    src/platform/picocalc_display.cpp
    src/platform/picocalc_keyboard.cpp
    src/platform/picocalc_audio.cpp
    src/platform/picocalc_uart_log.cpp
)

# Generate the LCD PIO shifter header (lcd_spi_min.pio.h).
pico_generate_pio_header(picocalc_bvwc_volleyball
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/lcd_spi_min.pio
)

target_include_directories(picocalc_bvwc_volleyball PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/compat
)

target_compile_options(picocalc_bvwc_volleyball PRIVATE
    -O2
    -Wall
    -Wextra
    -Wno-unused-parameter
)

target_compile_definitions(picocalc_bvwc_volleyball PRIVATE
    PICO_DEFAULT_UART_BAUD_RATE=115200
)

target_link_libraries(picocalc_bvwc_volleyball
    pico_stdlib
    hardware_clocks
    hardware_gpio
    hardware_i2c
    hardware_pio
    hardware_pwm
)

# Logs go out UART0 via the on-board USB-UART bridge, not native USB CDC.
pico_enable_stdio_usb(picocalc_bvwc_volleyball 0)
pico_enable_stdio_uart(picocalc_bvwc_volleyball 1)
pico_add_extra_outputs(picocalc_bvwc_volleyball)

add_custom_command(TARGET picocalc_bvwc_volleyball POST_BUILD
    COMMAND arm-none-eabi-size $<TARGET_FILE:picocalc_bvwc_volleyball>
    VERBATIM
)
