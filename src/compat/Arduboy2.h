/*
 * Drop-in subset of the Arduboy2 API, reimplemented on top of the PicoCalc
 * platform layer.  The drawing primitives mirror Arduboy2's exactly (same
 * 128x64 1bpp SSD1306-page framebuffer and the same Bresenham/bitmap math) so
 * the ported sketch renders pixel-for-pixel like the original, then the buffer
 * is scaled and blitted to the 320x320 LCD by display::blit_arduboy().
 *
 * As a derivative of Arduboy2 (API surface and constants), this file is
 * distributed under Arduboy2's license, not the project's MIT license.
 *
 * Copyright (c) 2016-2018 Scott Allen and the Arduboy/Arduboy2 contributors.
 * SPDX-License-Identifier: BSD-3-Clause
 * See THIRD_PARTY_LICENSES.md for the full license text.
 */

#pragma once

#include "compat/arduino_compat.h"
#include "compat/EEPROM.h"

// Colours (monochrome panel).
#ifndef BLACK
#define BLACK 0
#endif
#ifndef WHITE
#define WHITE 1
#endif
#define INVERT 2

// Button bit masks (same values as real Arduboy2).
#define LEFT_BUTTON 0x20
#define RIGHT_BUTTON 0x40
#define UP_BUTTON 0x80
#define DOWN_BUTTON 0x10
#define A_BUTTON 0x08
#define B_BUTTON 0x04

// Audio on/off state, exposed as a function pointer to ArduboyTones, matching
// `ArduboyTones sound(arduboy.audio.enabled);`.
class Arduboy2Audio {
   public:
    void begin() {}
    static void on();
    static void off();
    static void toggle();
    static void saveOnOff() {}
    static bool enabled();

   private:
    static bool audio_enabled_;
};

class Arduboy2 {
   public:
    static constexpr int kWidth = 128;
    static constexpr int kHeight = 64;

    Arduboy2Audio audio;
    uint16_t frameCount = 0;

    void begin();
    void setFrameRate(uint8_t rate);
    bool nextFrame();
    void pollButtons();
    bool pressed(uint8_t buttons) const;
    bool notPressed(uint8_t buttons) const;
    bool justPressed(uint8_t button) const;
    bool justReleased(uint8_t button) const;

    void clear();
    void display();
    void display(bool clearBuffer);
    uint8_t* getBuffer() { return sBuffer_; }

    // Drawing primitives (Arduboy2-compatible).
    void drawPixel(int16_t x, int16_t y, uint8_t color);
    void drawFastVLine(int16_t x, int16_t y, uint8_t h, uint8_t color);
    void drawFastHLine(int16_t x, int16_t y, uint8_t w, uint8_t color);
    void drawRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color);
    void fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color);
    void drawCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color);
    void fillCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color);
    void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, uint8_t w, uint8_t h,
                    uint8_t color);

    // Text.
    void setCursor(int16_t x, int16_t y);
    void setTextSize(uint8_t s);
    void setTextColor(uint8_t color);
    void setTextBackground(uint8_t bg);
    size_t write(uint8_t c);
    void print(const char* s);
    void print(char c);
    void print(int n);
    void print(unsigned int n);
    void print(long n);

   private:
    void fillCircleHelper(int16_t x0, int16_t y0, uint8_t r, uint8_t sides, int16_t delta,
                          uint8_t color);
    void drawChar(int16_t x, int16_t y, uint8_t c, uint8_t color, uint8_t bg, uint8_t size);
    uint8_t buttonsState();

    uint8_t sBuffer_[kWidth * kHeight / 8] = {0};

    int16_t cursor_x_ = 0;
    int16_t cursor_y_ = 0;
    uint8_t textColor_ = WHITE;
    uint8_t textBackground_ = BLACK;
    uint8_t textSize_ = 1;

    uint8_t currentButtonState_ = 0;
    uint8_t previousButtonState_ = 0;
    uint8_t heldButtonState_ = 0;

    uint8_t eachFrameMillis_ = 25;  // 40 fps default
    uint32_t lastFrameStart_ = 0;
    bool firstFrame_ = true;
};
