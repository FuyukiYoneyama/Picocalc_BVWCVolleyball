/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 *
 * RAM-backed EEPROM emulation matching the Arduino EEPROM.get/put/commit API
 * used by the sketch for tour progress and stats.
 *
 * NOTE: this storage lives in SRAM only, so progress persists for the current
 * power session but resets on power-off.  Persisting to the SD card is a
 * possible future extension (see README "Known limitations").
 */

#pragma once

#include <cstdint>
#include <cstring>

class EEPROMClass {
   public:
    static constexpr int kSize = 256;

    template <typename T>
    T& get(int address, T& value) const {
        if (address >= 0 && address + static_cast<int>(sizeof(T)) <= kSize) {
            std::memcpy(&value, &data_[address], sizeof(T));
        }
        return value;
    }

    template <typename T>
    const T& put(int address, const T& value) {
        if (address >= 0 && address + static_cast<int>(sizeof(T)) <= kSize) {
            std::memcpy(&data_[address], &value, sizeof(T));
        }
        return value;
    }

    uint8_t read(int address) const {
        return (address >= 0 && address < kSize) ? data_[address] : 0;
    }
    void write(int address, uint8_t value) {
        if (address >= 0 && address < kSize) {
            data_[address] = value;
        }
    }

    // On real EEPROM-backed cores this flushes to storage; here it is a no-op.
    bool commit() { return true; }
    void begin(int) {}

   private:
    uint8_t data_[kSize] = {0};
};

extern EEPROMClass EEPROM;
