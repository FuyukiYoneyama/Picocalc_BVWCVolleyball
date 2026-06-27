# Third-party licenses

This project's own code is MIT-licensed (see `LICENSE`). The following files in
`src/compat/` are **derived from third-party libraries** and remain under their
original licenses, reproduced in full below.

| File | Derived from | License |
|------|--------------|---------|
| `src/compat/glcdfont.h` | Adafruit-GFX classic 5×7 font (via Arduboy2 `glcdfont.c`) | BSD-2-Clause |
| `src/compat/Arduboy2.cpp` | Arduboy2 drawing/text primitives (`Arduboy2Base`/`Arduboy2`) | BSD-3-Clause |
| `src/compat/Arduboy2.h` | Arduboy2 public API / constants | BSD-3-Clause |

`src/compat/ArduboyTones.*` is an original implementation; it only mirrors the
*API* of Scott Allen's ArduboyTones library (BSD-3-Clause) and contains none of
its code.

The game sketch (e.g. `BVWCVolleyball_1.01.ino` by melanjet) is **not bundled**
here; it is downloaded from its upstream repository at build time and its
copyright belongs to its respective author.

---

## Arduboy2 library — BSD 3-Clause License

```
Arduboy2 library:
Copyright (c) 2016-2018, Scott Allen
All rights reserved.

The Arduboy2 library was forked from the Arduboy library:
https://github.com/Arduboy/Arduboy
Copyright (c) 2016, Kevin "Arduboy" Bates
Copyright (c) 2016, Chris Martinez
Copyright (c) 2016, Josh Goebel
Copyright (c) 2016, Scott Allen
All rights reserved.
which is in turn partially based on the Adafruit_SSD1306 library
https://github.com/adafruit/Adafruit_SSD1306
Copyright (c) 2012, Adafruit Industries
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holders nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

## Adafruit-GFX library (classic font) — BSD 2-Clause License

```
Portions of the Arduboy library, and thus portions of the Arduboy2 library,
based on the Adafruit-GFX library:
https://github.com/adafruit/Adafruit-GFX-Library
Copyright (c) 2012 Adafruit Industries
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
```
