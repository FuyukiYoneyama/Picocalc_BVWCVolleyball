/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 *
 * Arduino-style runtime: bring up PicoCalc hardware, then drive the sketch's
 * setup()/loop().  Audio is serviced every iteration so the non-blocking tone
 * generator stops/advances on time without stalling the game loop.
 */

#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include "config/board_config.h"
#include "platform/picocalc_audio.h"
#include "platform/picocalc_display.h"
#include "platform/picocalc_keyboard.h"
#include "platform/picocalc_uart_log.h"
#include "version.h"

// Defined by the ported sketch (src/game/bvwc_volleyball.cpp).
extern void setup();
extern void loop();

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
    pcvb::log_printf("BOOT", "peripherals=ready scale=%dx region=%dx%d@(%d,%d)", pcvb::board::kScale,
                     pcvb::board::kBlitWidth, pcvb::board::kBlitHeight, pcvb::board::kBlitOriginX,
                     pcvb::board::kBlitOriginY);

    setup();
    pcvb::log_printf("MAIN", "setup complete, entering loop");

    for (;;) {
        loop();
        pcvb::audio::service();
    }
}
