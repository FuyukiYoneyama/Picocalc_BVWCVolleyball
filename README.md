# Picocalc BVWC Volleyball

Beach Volley World Championship — a PicoCalc port of the Arduboy game
**BVWCVolleyball** by *"Jet Suricat"* Konstantin Tor (melanjet).

The original is a 128×64 monochrome Arduboy game built on the **Arduboy2** +
**ArduboyTones** libraries. This port keeps the game logic byte-for-byte and
runs it on the PicoCalc through a small **Arduboy2 compatibility layer** plus
the PicoCalc platform drivers (LCD / keyboard / PWM audio).

---

## Quick Start (just run it!)

You need the Pico SDK + `arm-none-eabi` GCC + CMake + Ninja, and internet (the
game sketch is fetched on the first `cmake`).

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S . -B build -G Ninja && cmake --build build
```

Then flash `build/picocalc_bvwc_volleyball.uf2`, either way:

- **USB / BOOTSEL:** hold **BOOTSEL**, plug in USB, drag the `.uf2` onto the
  `RPI-RP2` drive.
- **SD card ([UF2 Loader](https://github.com/pelrun/uf2loader)):** copy the
  `.uf2` into the `pico1-apps` folder on the SD card (`pico2-apps` for a
  Pico 2), then pick it from the loader menu (hold **Up** at power-on).

Done — play with the **arrow keys** + **`[`** (jump/serve) and **`]`**
(pause/back).

Details, controls, other games, and licensing are below.

---

## How it works

```
src/game/                      ← empty; the sketch is downloaded at build time (see below)
src/compat/Arduboy2.*          ← 128×64 1bpp framebuffer + Arduboy2 drawing/text/buttons
src/compat/ArduboyTones.*      ← tone() → non-blocking PWM square wave
src/compat/EEPROM.h            ← RAM-backed EEPROM.get/put/commit
src/compat/arduino_compat.h    ← random()/F()/PROGMEM/millis()/abs/min/max
src/compat/glcdfont.h          ← classic 5×7 font (verbatim from Arduboy2)
src/platform/picocalc_display.*  ← ST7365P LCD (PIO+GPIO), + 2× framebuffer blit
src/platform/picocalc_keyboard.* ← i2c1 keyboard FIFO polling
src/platform/picocalc_audio.*    ← non-blocking PWM tone generator (GP26/27)
src/main.cpp                   ← Arduino-style runtime: init → setup() → loop()
```

The Arduboy framebuffer (128×64, 1 bpp) is rendered exactly as on the Arduboy,
then **scaled 2× and centred** on the 320×320 panel (a 256×128 image at origin
`(32, 96)` with a black border).

---

## Build

Requires the [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk),
the `arm-none-eabi` GCC toolchain, CMake (3.18+) and Ninja. For toolchain setup
see the official
[Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S . -B build -G Ninja      # downloads the game sketch (needs internet)
cmake --build build
# Artifacts: build/picocalc_bvwc_volleyball.uf2 / .elf
```

Flash `build/picocalc_bvwc_volleyball.uf2` one of two ways:

- **USB / BOOTSEL:** hold **BOOTSEL** on the Pico, plug in USB, and copy the
  `.uf2` to the `RPI-RP2` drive.
- **SD card via [UF2 Loader](https://github.com/pelrun/uf2loader):** if you run
  the PicoCalc SD bootloader, just copy the `.uf2` into the SD card's
  `pico1-apps` folder (or `pico2-apps` for a Pico 2 / 2 W — both can coexist),
  then power on holding **Up** (or **F1** / **F5**) to open the menu and select
  it. This port is a plain pico-sdk app and does not write to flash, so it runs
  under UF2 Loader unmodified.

### The game sketch is downloaded, not bundled

To avoid redistributing a sketch whose license is unclear, **no `.ino` is
shipped in this repo**. The `configure` step downloads one single-file sketch
from the upstream ESPboy repository and compiles it as C++:

```sh
cmake -S . -B build -G Ninja                              # default: BVWCVolleyball_1.01
cmake -S . -B build -G Ninja -DGAME_NAME=Artillery        # a different game
cmake -S . -B build -G Ninja -DGAME_INO_URL=<raw-url>     # any URL / fork
```

Upstream layout is `GAMES/<NAME>/<NAME>.ino`, so `GAME_NAME` is the folder/file
base name. To build fully offline, drop a local `*.cpp` into `src/game/` and it
is compiled instead of downloading (that folder is git-ignored). See
`src/game/README.md`.

> **Per-game compatibility.** The downloader is generic, but a sketch only
> *compiles* if it sticks to the Arduboy2/ArduboyTones API implemented in
> `src/compat/` and does not rely on Arduino IDE auto-prototyping. Tested OK:
> BVWCVolleyball. Many other games need extra shim work first — e.g. `Artillery`
> additionally uses `fillTriangle`, `drawLine`, `everyXFrames`, `initRandomSeed`,
> `micros`/`delay`/`Serial`, plus forward declarations the IDE would generate.

- sysclk **250 MHz**, UART log **115200 8N1** on UART0 (USB-C → CH340C).
- Boot log prints app name, version, and build id.
- Footprint: ~43 KB flash, ~3 KB RAM (fits the 264 KB SRAM comfortably).

### Troubleshooting

**Assembler errors in `boot2_w25q080.S` (`unknown pseudo-op '.syntax'`,
`no such instruction 'ldr r3,=...'`, `expected (%rsi)`), and the failing
command uses `/usr/bin/cc`.** Your `build/` directory was configured with the
host x86 compiler instead of the ARM cross-compiler — usually because an IDE
(e.g. VS Code CMake Tools) configured it first with a host "kit", or `cmake`
ran once before `PICO_SDK_PATH` was set. The cached compiler is then reused.
Fix it with a clean reconfigure from the command line:

```sh
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
```

A correct configure prints `Check for working C compiler:
.../arm-none-eabi-gcc`. If an IDE keeps re-configuring with the host kit, build
from the CLI or select an `arm-none-eabi` kit.

---

## Controls (input mapping)

| Arduboy button | PicoCalc key(s)            | In-game use                         |
|----------------|----------------------------|-------------------------------------|
| LEFT           | ← (arrow) / `JoyLeft`      | move left, menu, beach select       |
| RIGHT          | → (arrow) / `JoyRight`     | move right, beach select            |
| UP             | ↑ (arrow) / `JoyUp`        | menu: Tour, debug toggle            |
| DOWN           | ↓ (arrow) / `JoyDown`      | menu: Single Match, reset stats     |
| A              | `[`, `A`, `Space`, `Enter` | jump / serve / confirm (ACT)        |
| B              | `]`, `B`, `Esc`            | pause / back / stats screen (ESC)   |

Press/Hold set the button; Release clears it — both edges come from the
keyboard controller FIFO, drained every frame in `pollButtons()`.

---

## Feature mapping (original → PicoCalc)

| Original (Arduboy)              | PicoCalc port                                             |
|---------------------------------|-----------------------------------------------------------|
| 128×64 OLED, white-on-black     | 128×64 framebuffer → 2× scaled, centred on 320×320 LCD    |
| 6 hardware buttons              | Mapped to PicoCalc keyboard keys (table above)            |
| ArduboyTones beeps              | Non-blocking PWM square-wave tones on GP26/27             |
| EEPROM stats / tour progress    | RAM-backed EEPROM emulation (see limitations)             |
| `Arduboy2` graphics + 5×7 font  | Reimplemented pixel-identical in the compat layer         |
| Game logic / AI / physics       | **Unchanged** (sketch downloaded & compiled verbatim)     |

### Screen-size handling

The Arduboy is 128×64 (2:1). The PicoCalc panel is 320×320. We use an integer
**2× scale** → 256×128, centred at `(32, 96)`, leaving a black border. Integer
scaling keeps pixels crisp and the full frame transfers comfortably within the
40 fps budget. (Scale/origin are constants in `src/config/board_config.h`.)

---

## Known limitations / changes

- **EEPROM is not persistent across power-off.** Stats and tour progress are
  kept in SRAM for the current session only (the PicoCalc has no EEPROM; the
  board's persistent store is the SD card). Persisting to SD is a possible
  future extension — the `EEPROM` API is already in place, only `commit()`
  needs a backing store.
- **Audio is a single square-wave tone** (the sketch only uses simple
  `tone()` beeps), generated directly by the PWM slice. There is no DMA mixer;
  none is needed for this game.
- The two compiler warnings during build (`pointScored` set-but-unused,
  `beachNamesFull` unused) come from the **original sketch** and are left as-is
  to keep the game code unmodified.

---

## License

- **Project code** (the PicoCalc port: `src/main.cpp`, `src/platform/*`,
  `src/config/*`, `src/compat/ArduboyTones.*`, `src/compat/EEPROM.h`,
  `src/compat/arduino_compat.h`, build files, docs) — **MIT**, see `LICENSE`.
- **`src/compat/Arduboy2.*` and `src/compat/glcdfont.h`** are derived from the
  **Arduboy2** (BSD-3-Clause) and **Adafruit-GFX** (BSD-2-Clause) libraries and
  remain under those licenses — see `THIRD_PARTY_LICENSES.md`.
- **The game sketch is NOT included** in this repository. It is downloaded from
  its upstream source at build time; its copyright belongs to its author
  (BVWCVolleyball © *"Jet Suricat"* Konstantin Tor / melanjet) and is not
  redistributed here.

## Credits

- Game: **BVWCVolleyball** © *"Jet Suricat"* Konstantin Tor (melanjet).
- Arduboy2 © 2016-2018 Scott Allen and contributors (BSD-3-Clause);
  ArduboyTones © Scott Allen.
- PicoCalc platform drivers adapted from the author's `life` project (MIT).
