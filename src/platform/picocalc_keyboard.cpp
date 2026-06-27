/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 *
 * I2C keyboard polling, from the proven `life` implementation.
 */

#include "platform/picocalc_keyboard.h"

#include "config/board_config.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

namespace pcvb::keyboard {
namespace {

constexpr uint8_t kRegKey = 0x04;
constexpr uint8_t kRegFifo = 0x09;
constexpr uint8_t kRegBacklight = 0x05;
constexpr uint8_t kWriteMask = 0x80;

// Write the register number with a held repeated-start, then read the bytes.
bool read_reg(uint8_t reg, uint8_t* data, size_t len) {
    if (i2c_write_blocking(i2c1, board::kKeyboardI2cAddress, &reg, 1, true) != 1) {
        return false;
    }
    return i2c_read_blocking(i2c1, board::kKeyboardI2cAddress, data, len, false) ==
           static_cast<int>(len);
}

}  // namespace

void init() {
    i2c_init(i2c1, board::kKeyboardI2cHz);
    gpio_set_function(board::kKeyboardI2cSda, GPIO_FUNC_I2C);
    gpio_set_function(board::kKeyboardI2cScl, GPIO_FUNC_I2C);
    gpio_pull_up(board::kKeyboardI2cSda);
    gpio_pull_up(board::kKeyboardI2cScl);
}

bool read_event(KeyEvent* event) {
    if (event == nullptr) {
        return false;
    }

    // 0x04 reports the FIFO count in its low 5 bits.  Skip the FIFO read when
    // the keyboard is idle so we never pop stale data.
    uint8_t key_info[2] = {0, 0};
    if (!read_reg(kRegKey, key_info, sizeof(key_info))) {
        return false;
    }
    if ((key_info[0] & 0x1f) == 0) {
        return false;
    }

    uint8_t fifo_item[2] = {0, 0};
    if (!read_reg(kRegFifo, fifo_item, sizeof(fifo_item))) {
        return false;
    }

    event->state = static_cast<KeyState>(fifo_item[0]);
    event->key = fifo_item[1];
    return event->key != 0;
}

bool set_backlight(uint8_t level) {
    // Writable registers are addressed with the write mask (reg | 0x80).
    uint8_t data[2] = {static_cast<uint8_t>(kRegBacklight | kWriteMask), level};
    return i2c_write_blocking(i2c1, board::kKeyboardI2cAddress, data, 2, false) == 2;
}

}  // namespace pcvb::keyboard
