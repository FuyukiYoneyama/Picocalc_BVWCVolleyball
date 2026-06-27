/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 */

#include "platform/picocalc_uart_log.h"

#include <cstdarg>
#include <cstdio>

#include "pico/stdlib.h"

namespace pcvb {

void log_init() {
    stdio_init_all();
}

void log_printf(const char* category, const char* fmt, ...) {
    std::printf("[%s] ", category);
    va_list args;
    va_start(args, fmt);
    std::vprintf(fmt, args);
    va_end(args);
    std::printf("\r\n");
}

}  // namespace pcvb
