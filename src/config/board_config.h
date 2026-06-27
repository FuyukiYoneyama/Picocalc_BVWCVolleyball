/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 *
 * Central hardware constants for the PicoCalc.  Values follow the PicoCalc
 * mainboard V2.0 schematic and the author's "life" project on the same board.
 */

#pragma once

#include <stdint.h>

namespace pcvb::board {

// Run at 250 MHz like the other audio/video projects.  Must be set before
// stdio / peripheral init.
constexpr uint32_t kSystemClockKhz = 250000;
constexpr uint32_t kUartBaudRate = 115200;

// LCD (ST7365P) - PIO drives SCK/MOSI, GPIO drives CS/DC/RST.
constexpr unsigned kLcdPinSck = 10;
constexpr unsigned kLcdPinMosi = 11;
constexpr unsigned kLcdPinMiso = 12;  // readback only, unused here
constexpr unsigned kLcdPinCs = 13;
constexpr unsigned kLcdPinDc = 14;
constexpr unsigned kLcdPinRst = 15;
constexpr unsigned kLcdPinRamCs = 21;  // PSRAM clock, held high by LCD bring-up

// LCD geometry.  The panel is a square 320x320 RGB565 display.
constexpr int kScreenWidth = 320;
constexpr int kScreenHeight = 320;

// The Arduboy framebuffer is 128x64 monochrome.  We blit it with an integer
// 2x scale (256x128) centred on the panel, leaving a black border.
constexpr int kGameWidth = 128;
constexpr int kGameHeight = 64;
constexpr int kScale = 2;
constexpr int kBlitWidth = kGameWidth * kScale;    // 256
constexpr int kBlitHeight = kGameHeight * kScale;  // 128
constexpr int kBlitOriginX = (kScreenWidth - kBlitWidth) / 2;    // 32
constexpr int kBlitOriginY = (kScreenHeight - kBlitHeight) / 2;  // 96

// Keyboard controller on i2c1.
constexpr unsigned kKeyboardI2cSda = 6;
constexpr unsigned kKeyboardI2cScl = 7;
constexpr uint32_t kKeyboardI2cHz = 400 * 1000;
constexpr uint8_t kKeyboardI2cAddress = 0x1f;

// PWM audio (schematic V2.0): left=GP26, right=GP27, same PWM slice.
constexpr unsigned kAudioPwmLeft = 26;
constexpr unsigned kAudioPwmRight = 27;

}  // namespace pcvb::board
