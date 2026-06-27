/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 *
 * Arduino-style runtime: bring up PicoCalc hardware, then drive the sketch's
 * setup()/loop().  Audio is serviced every iteration so the non-blocking tone
 * generator stops/advances on time without stalling the game loop.
 */

#include <cstring>

#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include "config/board_config.h"
#include "platform/picocalc_audio.h"
#include "platform/picocalc_display.h"
#include "platform/picocalc_keyboard.h"
#include "platform/picocalc_uart_log.h"
#include "version.h"

#ifndef PCVB_LCD_SELFTEST
#define PCVB_LCD_SELFTEST 0
#endif

// Defined by the ported sketch (src/game/bvwc_volleyball.cpp).
extern void setup();
extern void loop();

#if PCVB_LCD_SELFTEST
// One-shot bring-up test, independent of the game and the Arduboy buffer.
// Solid R/G/B fills check the panel + backlight + PIO/DMA path; the framebuffer
// pattern then checks the 1bpp blit + 2x scale path specifically.
static void lcd_selftest() {
    pcvb::log_printf("TEST", "lcd selftest: red");
    pcvb::display::fill_screen(0xf800);
    sleep_ms(500);
    pcvb::log_printf("TEST", "lcd selftest: green");
    pcvb::display::fill_screen(0x07e0);
    sleep_ms(500);
    pcvb::log_printf("TEST", "lcd selftest: blue");
    pcvb::display::fill_screen(0x001f);
    sleep_ms(500);

    pcvb::log_printf("TEST", "lcd selftest: framebuffer pattern");
    static uint8_t fb[pcvb::board::kGameWidth * pcvb::board::kGameHeight / 8];
    std::memset(fb, 0, sizeof(fb));
    auto set_px = [&](int x, int y) {
        if (x < 0 || x >= pcvb::board::kGameWidth || y < 0 || y >= pcvb::board::kGameHeight) return;
        fb[(y / 8) * pcvb::board::kGameWidth + x] |= static_cast<uint8_t>(1u << (y % 8));
    };
    for (int x = 0; x < pcvb::board::kGameWidth; ++x) {
        set_px(x, 0);
        set_px(x, pcvb::board::kGameHeight - 1);
        set_px(x, x * pcvb::board::kGameHeight / pcvb::board::kGameWidth);  // diagonal
    }
    for (int y = 0; y < pcvb::board::kGameHeight; ++y) {
        set_px(0, y);
        set_px(pcvb::board::kGameWidth - 1, y);
    }
    pcvb::display::fill_screen(0x0000);
    pcvb::display::blit_arduboy(fb, 0xffff, 0x0000);
    sleep_ms(900);
    pcvb::log_printf("TEST", "lcd selftest: done");
}
#endif

int main() {
    // Clock must be set before stdio / peripheral init so UART and PWM dividers
    // are computed against the final 250 MHz sysclk.
    set_sys_clock_khz(pcvb::board::kSystemClockKhz, true);

    pcvb::log_init();
    uart_set_baudrate(uart_default, pcvb::board::kUartBaudRate);
    sleep_ms(50);
    pcvb::log_printf("BOOT", "%s version %s", PCVB_APP_NAME, PCVB_VERSION);
    pcvb::log_printf("BOOT", "build id time=\"%s %s\" baud=%lu sysclk=%lukHz", __DATE__, __TIME__,
                     static_cast<unsigned long>(pcvb::board::kUartBaudRate),
                     static_cast<unsigned long>(pcvb::board::kSystemClockKhz));

    pcvb::display::init();
    pcvb::keyboard::init();
    pcvb::audio::init();

    // Note: the LCD backlight is left untouched so the user's brightness setting
    // is preserved.

    pcvb::log_printf("BOOT", "peripherals=ready scale=%dx region=%dx%d@(%d,%d)", pcvb::board::kScale,
                     pcvb::board::kBlitWidth, pcvb::board::kBlitHeight, pcvb::board::kBlitOriginX,
                     pcvb::board::kBlitOriginY);

#if PCVB_LCD_SELFTEST
    lcd_selftest();
#endif

    setup();
    pcvb::log_printf("MAIN", "setup complete, entering loop");

    for (;;) {
        loop();
        pcvb::audio::service();
    }
}
