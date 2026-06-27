/*
 * Arduboy2-compatible drawing/text/button layer for the PicoCalc port.
 *
 * The drawing and text routines are reimplementations of the Arduboy2 library
 * primitives (drawPixel/drawCircle/fillCircle/drawBitmap/drawChar, etc.) so the
 * ported sketch renders pixel-identically.  As a derivative of Arduboy2 this
 * file is distributed under Arduboy2's license, not the project's MIT license.
 *
 * Copyright (c) 2016-2018 Scott Allen and the Arduboy/Arduboy2 contributors.
 * Copyright (c) 2012 Adafruit Industries (Adafruit_SSD1306-derived portions).
 * SPDX-License-Identifier: BSD-3-Clause
 * See THIRD_PARTY_LICENSES.md for the full license text.
 */

#include "compat/Arduboy2.h"

#include "compat/glcdfont.h"
#include "platform/picocalc_audio.h"
#include "platform/picocalc_display.h"
#include "platform/picocalc_keyboard.h"
#include "platform/picocalc_key_table.h"
#include "platform/picocalc_uart_log.h"

#ifndef PCVB_FRAME_STATS
#define PCVB_FRAME_STATS 0
#endif

// Globals backing the compat objects.
EEPROMClass EEPROM;
bool Arduboy2Audio::audio_enabled_ = true;

namespace {
constexpr uint16_t kRgbWhite = 0xffff;
constexpr uint16_t kRgbBlack = 0x0000;
constexpr int kW = Arduboy2::kWidth;
constexpr int kH = Arduboy2::kHeight;

inline uint8_t bit_mask(int b) { return static_cast<uint8_t>(1u << (b & 7)); }
}  // namespace

// ---------------------------------------------------------------- audio -----
void Arduboy2Audio::on() { audio_enabled_ = true; }
void Arduboy2Audio::off() {
    audio_enabled_ = false;
    pcvb::audio::stop();
}
void Arduboy2Audio::toggle() { audio_enabled_ = !audio_enabled_; }
bool Arduboy2Audio::enabled() { return audio_enabled_; }

// ---------------------------------------------------------- frame / system --
void Arduboy2::begin() {
    clear();
    firstFrame_ = true;
}

void Arduboy2::setFrameRate(uint8_t rate) {
    if (rate == 0) {
        rate = 1;
    }
    eachFrameMillis_ = static_cast<uint8_t>(1000u / rate);
    if (eachFrameMillis_ == 0) {
        eachFrameMillis_ = 1;
    }
}

bool Arduboy2::nextFrame() {
    const uint32_t now = millis();
    if (firstFrame_) {
        firstFrame_ = false;
        lastFrameStart_ = now;
        ++frameCount;
        return true;
    }
    if (static_cast<uint32_t>(now - lastFrameStart_) < eachFrameMillis_) {
        return false;
    }
    lastFrameStart_ += eachFrameMillis_;
    // If we fell far behind (e.g. a long transfer), rebase to avoid a catch-up
    // spiral rather than trying to render skipped frames.
    if (static_cast<uint32_t>(now - lastFrameStart_) > static_cast<uint32_t>(eachFrameMillis_) * 4u) {
        lastFrameStart_ = now;
    }
    ++frameCount;
    return true;
}

// --------------------------------------------------------------- buttons ----
uint8_t Arduboy2::buttonsState() {
    pcvb::keyboard::KeyEvent event{};
    while (pcvb::keyboard::read_event(&event)) {
        uint8_t mask = 0;
        switch (event.key) {
            case pcvb::keys::Left:
            case pcvb::keys::JoyLeft:
                mask = LEFT_BUTTON;
                break;
            case pcvb::keys::Right:
            case pcvb::keys::JoyRight:
                mask = RIGHT_BUTTON;
                break;
            case pcvb::keys::Up:
            case pcvb::keys::JoyUp:
                mask = UP_BUTTON;
                break;
            case pcvb::keys::Down:
            case pcvb::keys::JoyDown:
                mask = DOWN_BUTTON;
                break;
            case 'a':
            case 'A':
            case '[':  // ACT-style action key (PicoCalc)
            case pcvb::keys::Space:
            case pcvb::keys::Enter:
            case pcvb::keys::JoyCenter:
                mask = A_BUTTON;
                break;
            case 'b':
            case 'B':
            case ']':  // ESC-style back/pause key (PicoCalc)
            case pcvb::keys::Escape:
                mask = B_BUTTON;
                break;
            default:
                break;
        }
        if (mask == 0) {
            continue;
        }
        if (event.state == pcvb::keyboard::KeyState::Pressed ||
            event.state == pcvb::keyboard::KeyState::Hold) {
            heldButtonState_ |= mask;
        } else if (event.state == pcvb::keyboard::KeyState::Released) {
            heldButtonState_ = static_cast<uint8_t>(heldButtonState_ & ~mask);
        }
    }
    return heldButtonState_;
}

void Arduboy2::pollButtons() {
    previousButtonState_ = currentButtonState_;
    currentButtonState_ = buttonsState();
}

bool Arduboy2::pressed(uint8_t buttons) const {
    return (currentButtonState_ & buttons) == buttons;
}
bool Arduboy2::notPressed(uint8_t buttons) const {
    return (currentButtonState_ & buttons) == 0;
}
bool Arduboy2::justPressed(uint8_t button) const {
    return (!(previousButtonState_ & button) && (currentButtonState_ & button));
}
bool Arduboy2::justReleased(uint8_t button) const {
    return ((previousButtonState_ & button) && !(currentButtonState_ & button));
}

// --------------------------------------------------------------- display ----
void Arduboy2::clear() {
    for (auto& b : sBuffer_) {
        b = 0;
    }
}

void Arduboy2::display() {
    pcvb::display::blit_arduboy(sBuffer_, kRgbWhite, kRgbBlack);

#if PCVB_FRAME_STATS
    // display() runs exactly once per rendered game frame, so the interval
    // between calls is the real frame period.  The 40 fps gate caps this at
    // ~25000 us; if a frame's work overruns the budget it rises above that.
    // avg ~= 25000 us (40 fps) means the port matches the original's pace.
    static uint32_t prev_us = 0, sum_us = 0, worst_us = 0, count = 0;
    const uint32_t now = time_us_32();
    if (prev_us != 0) {
        const uint32_t dt = now - prev_us;
        sum_us += dt;
        if (dt > worst_us) worst_us = dt;
        if (++count >= 80) {
            const uint32_t avg = sum_us / count;
            pcvb::log_printf("PERF", "frame avg=%lu us (%lu fps) worst=%lu us",
                             static_cast<unsigned long>(avg),
                             static_cast<unsigned long>(avg ? 1000000u / avg : 0),
                             static_cast<unsigned long>(worst_us));
            sum_us = 0;
            worst_us = 0;
            count = 0;
        }
    }
    prev_us = now;
#endif
}

void Arduboy2::display(bool clearBuffer) {
    display();
    if (clearBuffer) {
        clear();
    }
}

// ------------------------------------------------------------- primitives ---
void Arduboy2::drawPixel(int16_t x, int16_t y, uint8_t color) {
    if (x < 0 || x >= kW || y < 0 || y >= kH) {
        return;
    }
    const int index = (y / 8) * kW + x;
    const uint8_t mask = bit_mask(y % 8);
    if (color) {
        sBuffer_[index] |= mask;
    } else {
        sBuffer_[index] = static_cast<uint8_t>(sBuffer_[index] & ~mask);
    }
}

void Arduboy2::drawFastVLine(int16_t x, int16_t y, uint8_t h, uint8_t color) {
    const int end = y + h;
    for (int a = (y > 0 ? y : 0); a < end && a < kH; ++a) {
        drawPixel(x, static_cast<int16_t>(a), color);
    }
}

void Arduboy2::drawFastHLine(int16_t x, int16_t y, uint8_t w, uint8_t color) {
    const int end = x + w;
    for (int a = (x > 0 ? x : 0); a < end && a < kW; ++a) {
        drawPixel(static_cast<int16_t>(a), y, color);
    }
}

void Arduboy2::drawRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color) {
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, static_cast<int16_t>(y + h - 1), w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(static_cast<int16_t>(x + w - 1), y, h, color);
}

void Arduboy2::fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color) {
    for (int16_t i = x; i < x + w; ++i) {
        drawFastVLine(i, y, h, color);
    }
}

void Arduboy2::drawCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

void Arduboy2::fillCircleHelper(int16_t x0, int16_t y0, uint8_t r, uint8_t sides, int16_t delta,
                                uint8_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (sides & 0x1) {
            drawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
            drawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
        }
        if (sides & 0x2) {
            drawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
            drawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
        }
    }
}

void Arduboy2::fillCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color) {
    drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
}

void Arduboy2::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, uint8_t w, uint8_t h,
                          uint8_t color) {
    if (x + w < 0 || x > kW - 1 || y + h < 0 || y > kH - 1) {
        return;
    }

    int yOffset = abs(y) % 8;
    int sRow = y / 8;
    if (y < 0) {
        sRow--;
        yOffset = 8 - yOffset;
    }
    int rows = h / 8;
    if (h % 8 != 0) {
        rows++;
    }
    for (int a = 0; a < rows; a++) {
        int bRow = sRow + a;
        if (bRow > (kH / 8) - 1) {
            break;
        }
        if (bRow > -2) {
            for (int iCol = 0; iCol < w; iCol++) {
                if (iCol + x > (kW - 1)) {
                    break;
                }
                if (iCol + x >= 0) {
                    const uint8_t byte = pgm_read_byte(bitmap + (a * w) + iCol);
                    if (bRow >= 0) {
                        if (color == WHITE) {
                            sBuffer_[(bRow * kW) + x + iCol] |= byte << yOffset;
                        } else if (color == BLACK) {
                            sBuffer_[(bRow * kW) + x + iCol] &= ~(byte << yOffset);
                        } else {
                            sBuffer_[(bRow * kW) + x + iCol] ^= byte << yOffset;
                        }
                    }
                    if (yOffset && bRow < (kH / 8) - 1 && bRow > -2) {
                        if (color == WHITE) {
                            sBuffer_[((bRow + 1) * kW) + x + iCol] |= byte >> (8 - yOffset);
                        } else if (color == BLACK) {
                            sBuffer_[((bRow + 1) * kW) + x + iCol] &= ~(byte >> (8 - yOffset));
                        } else {
                            sBuffer_[((bRow + 1) * kW) + x + iCol] ^= byte >> (8 - yOffset);
                        }
                    }
                }
            }
        }
    }
}

// ------------------------------------------------------------------ text ----
void Arduboy2::setCursor(int16_t x, int16_t y) {
    cursor_x_ = x;
    cursor_y_ = y;
}
void Arduboy2::setTextSize(uint8_t s) { textSize_ = s < 1 ? 1 : s; }
void Arduboy2::setTextColor(uint8_t color) { textColor_ = color; }
void Arduboy2::setTextBackground(uint8_t bg) { textBackground_ = bg; }

void Arduboy2::drawChar(int16_t x, int16_t y, uint8_t c, uint8_t color, uint8_t bg, uint8_t size) {
    constexpr uint8_t characterWidth = 5;
    constexpr uint8_t characterHeight = 8;
    constexpr uint8_t fullCharacterWidth = 6;   // 5 + 1 spacing
    const bool drawBackground = bg != color;
    const uint8_t* bitmap = &font5x7[c * characterWidth];

    for (uint8_t i = 0; i < fullCharacterWidth; i++) {
        uint8_t column = (i < characterWidth) ? *bitmap++ : 0;
        for (uint8_t j = 0; j < characterHeight; j++) {
            const uint8_t pixelIsSet = column & 0x01;
            if (pixelIsSet || drawBackground) {
                for (uint8_t a = 0; a < size; a++) {
                    for (uint8_t b = 0; b < size; b++) {
                        drawPixel(static_cast<int16_t>(x + i * size + a),
                                  static_cast<int16_t>(y + j * size + b),
                                  pixelIsSet ? color : bg);
                    }
                }
            }
            column >>= 1;
        }
    }
}

size_t Arduboy2::write(uint8_t c) {
    if (c == '\r') {
        return 1;
    }
    if (c == '\n') {
        cursor_x_ = 0;
        cursor_y_ = static_cast<int16_t>(cursor_y_ + 8 * textSize_);
        return 1;
    }
    drawChar(cursor_x_, cursor_y_, c, textColor_, textBackground_, textSize_);
    cursor_x_ = static_cast<int16_t>(cursor_x_ + 6 * textSize_);
    return 1;
}

void Arduboy2::print(const char* s) {
    if (s == nullptr) {
        return;
    }
    while (*s) {
        write(static_cast<uint8_t>(*s++));
    }
}
void Arduboy2::print(char c) { write(static_cast<uint8_t>(c)); }
void Arduboy2::print(int n) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", n);
    print(buf);
}
void Arduboy2::print(unsigned int n) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%u", n);
    print(buf);
}
void Arduboy2::print(long n) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%ld", n);
    print(buf);
}
