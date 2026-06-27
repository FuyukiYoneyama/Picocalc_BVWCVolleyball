/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 *
 * Non-blocking PWM tone generator.  Unlike a busy-wait beep, this configures
 * the audio PWM slice to emit a square wave at the requested frequency and
 * returns immediately; service() turns it off / advances the queue.  This keeps
 * the 40 fps game loop responsive while ball-hit beeps play.
 */

#pragma once

#include <stdint.h>

namespace pcvb::audio {

void init();
// Play a single tone for `duration_ms`.  Replaces any tone in progress.
void tone(uint16_t frequency_hz, uint16_t duration_ms);
// Play two tones back to back (ArduboyTones' 2-note form).
void tone2(uint16_t f1, uint16_t d1, uint16_t f2, uint16_t d2);
// Called every loop iteration; stops the tone or advances to the next segment
// when the current segment's duration has elapsed.
void service();
void stop();

}  // namespace pcvb::audio
