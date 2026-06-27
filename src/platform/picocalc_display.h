/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

namespace pcvb::display {

void init();
void fill_screen(uint16_t rgb565);

// Blits the 1024-byte Arduboy framebuffer (128x64, 1bpp, SSD1306 page layout)
// to the panel with an integer 2x scale, centred.  `fg` is used for lit pixels
// and `bg` for clear pixels.
void blit_arduboy(const uint8_t* fb, uint16_t fg, uint16_t bg);

}  // namespace pcvb::display
