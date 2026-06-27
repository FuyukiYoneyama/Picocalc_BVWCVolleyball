# AGENTS — Picocalc BVWC Volleyball

PicoCalc port of the Arduboy game **BVWCVolleyball** (melanjet). Know the
PicoCalc hardware basics before touching hardware code: ST7365P 320x320 RGB565
LCD driven by PIO (CS/DC/RST on GPIO), an i2c1 keyboard controller at 0x1F, and
PWM audio on GP26/GP27.

## Layout
- `src/game/bvwc_volleyball.cpp` — the **original sketch, verbatim**. Do not
  refactor; the port works by keeping game logic byte-identical.
- `src/compat/` — Arduboy2 / ArduboyTones / EEPROM / Arduino shims. Drawing and
  text routines are pixel-faithful reimplementations of Arduboy2.
- `src/platform/` — PicoCalc drivers (LCD, keyboard, PWM audio, UART log),
  adapted from the proven `life` project.
- `src/config/board_config.h` — all pins, clock, and blit geometry. No
  hard-coded pin numbers elsewhere.

## Rules
- Keep `src/game/*` unmodified except where a genuine Arduino/Pico API gap
  forces a change (document any such change in the README).
- LCD contract: `set_window` → stream pixels → `wait_idle` before changing the
  window. Audio must stay non-blocking (`audio::service()` each loop iteration).
- Bump `PCVB_VERSION` in `src/version.h` on any behavioural change; the boot log
  prints version + build id so a UF2 can be matched to a build.
- One build dir: `build/`. Logs are UART0 @ 115200 8N1.

## Definition of Done
For this project: builds to `build/picocalc_bvwc_volleyball.uf2`, boots with a
version/build-id UART log, shows the title screen, accepts the mapped keys, and
plays a full match without hanging.
