/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 *
 * Minimal Arduino-core shim: just enough of the Arduino global namespace for
 * the BVWCVolleyball sketch to compile unchanged on the Pico SDK.
 */

#pragma once

// Pull in the std headers first so their own definitions are not clobbered by
// the abs()/min()/max() macros defined at the bottom of this file.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "pico/stdlib.h"

// --- PROGMEM / flash helpers -------------------------------------------------
// The RP2040 executes from XIP flash with a flat address space, so PROGMEM is a
// no-op and pgm_read_* are plain dereferences.
#ifndef PROGMEM
#define PROGMEM
#endif
#define pgm_read_byte(addr) (*reinterpret_cast<const uint8_t*>(addr))
#define pgm_read_word(addr) (*reinterpret_cast<const uint16_t*>(addr))

// F("...") wraps a flash string on AVR; here strings already live in flash, so
// F(x) is the identity and the argument stays a plain const char*.
class __FlashStringHelper;
#define F(string_literal) (string_literal)

// --- random / analog ---------------------------------------------------------
namespace pcvb_arduino {

// Simple xorshift32 PRNG so random(min,max) keeps Arduino's half-open range
// semantics independent of the C library's rand().
inline uint32_t& rng_state() {
    static uint32_t state = 0x1234567u;
    return state;
}

inline uint32_t rng_next() {
    uint32_t x = rng_state();
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state() = x;
    return x;
}

}  // namespace pcvb_arduino

inline void randomSeed(uint32_t seed) {
    if (seed == 0) {
        seed = 1;
    }
    pcvb_arduino::rng_state() = seed;
}

// random(max) -> [0, max), random(min, max) -> [min, max), matching Arduino.
inline long random(long howbig) {
    if (howbig <= 0) {
        return 0;
    }
    return static_cast<long>(pcvb_arduino::rng_next() % static_cast<uint32_t>(howbig));
}

inline long random(long howsmall, long howbig) {
    if (howsmall >= howbig) {
        return howsmall;
    }
    return howsmall + random(howbig - howsmall);
}

// No analog joystick on PicoCalc; A0 reads are only used to seed the PRNG, so
// return a time-derived noise value.
constexpr int A0 = 26;
inline int analogRead(int) {
    return static_cast<int>(time_us_32() & 0x3ff);
}

inline uint32_t millis() {
    return to_ms_since_boot(get_absolute_time());
}

// --- abs / min / max (Arduino macro forms, work for float and int) ----------
#ifdef abs
#undef abs
#endif
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#define abs(x) ((x) > 0 ? (x) : -(x))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
