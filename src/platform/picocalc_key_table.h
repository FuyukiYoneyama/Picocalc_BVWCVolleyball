/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 *
 * PicoCalc keyboard codes (subset).  Printable keys keep their ASCII value;
 * navigation/function keys use the controller values below.
 */

#pragma once

#include <stdint.h>

namespace pcvb::keys {

constexpr uint8_t None = 0x00;
constexpr uint8_t JoyUp = 0x01;
constexpr uint8_t JoyDown = 0x02;
constexpr uint8_t JoyLeft = 0x03;
constexpr uint8_t JoyRight = 0x04;
constexpr uint8_t JoyCenter = 0x05;
constexpr uint8_t Enter = 0x0a;
constexpr uint8_t Space = 0x20;
constexpr uint8_t Escape = 0xb1;
constexpr uint8_t Left = 0xb4;
constexpr uint8_t Up = 0xb5;
constexpr uint8_t Down = 0xb6;
constexpr uint8_t Right = 0xb7;

}  // namespace pcvb::keys
