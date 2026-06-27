/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 *
 * Subset of the ArduboyTones API used by the sketch: the two tone() forms,
 * gated by the audio-enabled callback and forwarded to the non-blocking
 * PicoCalc PWM tone generator.
 */

#pragma once

#include <stdint.h>

class ArduboyTones {
   public:
    explicit ArduboyTones(bool (*outputEnabled)()) : outputEnabled_(outputEnabled) {}

    void tone(uint16_t frequency, uint16_t duration);
    void tone(uint16_t freq1, uint16_t dur1, uint16_t freq2, uint16_t dur2);
    void noTone();

   private:
    bool (*outputEnabled_)();
};
