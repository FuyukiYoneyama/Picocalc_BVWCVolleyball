/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

namespace pcvb::keyboard {

// FIFO events from the PicoCalc keyboard controller: byte 0 is the state code,
// byte 1 is the firmware key code (ASCII for printable keys, otherwise the
// constants in picocalc_key_table.h).
enum class KeyState : uint8_t {
    Idle = 0,
    Pressed = 1,
    Hold = 2,
    Released = 3,
};

struct KeyEvent {
    KeyState state;
    uint8_t key;
};

void init();
// Reads at most one FIFO event.  Returns false when no key is pending, when
// I2C fails, or when the FIFO reports key code 0.
bool read_event(KeyEvent* event);

}  // namespace pcvb::keyboard
