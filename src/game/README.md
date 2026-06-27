# src/game — game sketch (not bundled)

The actual game sketch is **intentionally not included** in this repository.
The upstream `.ino` (e.g. `BVWCVolleyball_1.01.ino` by melanjet) has no clear
redistribution license, so it must not be shipped here.

Instead the build **downloads one sketch at configure time** from the upstream
ESPboy repository and compiles it. See the top-level `CMakeLists.txt`.

## Usage

Default (downloads BVWCVolleyball):
```sh
cmake -S . -B build -G Ninja
cmake --build build
```

Pick another single-file game (upstream layout is `GAMES/<NAME>/<NAME>.ino`):
```sh
cmake -S . -B build -G Ninja -DGAME_NAME=Artillery
cmake -S . -B build -G Ninja -DGAME_NAME=CraitsFestiveFight
```

Use an arbitrary URL or a fork:
```sh
cmake -S . -B build -G Ninja -DGAME_INO_URL=https://example/MyGame.ino
```

## Offline / local override

Drop a `*.cpp` here and it is compiled instead of downloading anything:
```sh
cp /path/to/BVWCVolleyball_1.01.ino src/game/bvwc_volleyball.cpp
```
(Any `*.cpp` placed in this folder is git-ignored — see `.gitignore` — so a
local copyrighted sketch is never accidentally committed.)

## Limitations

- Single-file sketches only.
- The sketch must use the subset of the Arduboy2 / ArduboyTones API implemented
  in `src/compat/`. Games using `Sprites`, `ArduboyFX`, external music
  libraries, etc. need those added to the compat layer first.
