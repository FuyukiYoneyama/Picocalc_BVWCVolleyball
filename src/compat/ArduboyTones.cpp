/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 */

#include "compat/ArduboyTones.h"

#include "platform/picocalc_audio.h"

void ArduboyTones::tone(uint16_t frequency, uint16_t duration) {
    if (outputEnabled_ != nullptr && !outputEnabled_()) {
        return;
    }
    pcvb::audio::tone(frequency, duration);
}

void ArduboyTones::tone(uint16_t freq1, uint16_t dur1, uint16_t freq2, uint16_t dur2) {
    if (outputEnabled_ != nullptr && !outputEnabled_()) {
        return;
    }
    pcvb::audio::tone2(freq1, dur1, freq2, dur2);
}

void ArduboyTones::noTone() { pcvb::audio::stop(); }
