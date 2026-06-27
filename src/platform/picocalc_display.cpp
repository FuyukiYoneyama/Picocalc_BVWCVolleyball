/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 *
 * ST7365P LCD bring-up and write path, adapted from the proven `life` project.
 * The screenshot/readback path is dropped; a monochrome framebuffer blit is
 * added for the Arduboy compatibility layer.
 */

#include "platform/picocalc_display.h"

#include <cstddef>
#include <cstdint>

#include "config/board_config.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "lcd_spi_min.pio.h"

namespace pcvb::display {
namespace {

// 250 MHz sysclk / (2 * 4.0) = ~31.25 MHz SPI-equivalent.  This is the rate the
// "life" project runs reliably on this exact panel/driver; a faster divider
// (e.g. 2.0) can leave the panel blank.  A 256x128 blit (~64 KB) still fits the
// 40 fps frame budget at this rate.
constexpr float kPioClkDiv = 4.0f;
PIO g_pio = pio0;
uint g_sm = 0;
uint g_pio_offset = 0;

void select() { gpio_put(board::kLcdPinCs, 0); }
void deselect() { gpio_put(board::kLcdPinCs, 1); }
void set_dc(bool data) { gpio_put(board::kLcdPinDc, data ? 1 : 0); }

void write_bytes(const uint8_t* data, size_t len) {
    while (len-- > 0) {
        lcd_spi_min_put(g_pio, g_sm, *data++);
    }
}

void wait_idle() { lcd_spi_min_wait_idle(g_pio, g_sm); }

void write_command(uint8_t cmd) {
    select();
    set_dc(false);
    write_bytes(&cmd, 1);
    wait_idle();
    deselect();
}

void write_data(const uint8_t* data, size_t len) {
    if (len == 0) {
        return;
    }
    select();
    set_dc(true);
    write_bytes(data, len);
    wait_idle();
    deselect();
}

void write_command1(uint8_t cmd, uint8_t data0) {
    write_command(cmd);
    write_data(&data0, 1);
}

void write_commandn(uint8_t cmd, const uint8_t* data, size_t len) {
    write_command(cmd);
    write_data(data, len);
}

void reset_panel() {
    // The ST7365P needs well over 100 ms after a hardware reset before it
    // reliably accepts the init sequence.  The shorter delay inherited from the
    // "life" driver could leave the panel un-initialised (blank) and in a state
    // later apps could not recover; the Picocalc_Clock baseline uses 200 ms.
    gpio_put(board::kLcdPinRst, 1);
    sleep_ms(10);
    gpio_put(board::kLcdPinRst, 0);
    sleep_ms(10);
    gpio_put(board::kLcdPinRst, 1);
    sleep_ms(200);
}

// CASET / PASET expect inclusive end coordinates, then RAMWR opens the stream.
void set_window(int x, int y, int w, int h) {
    const int x1 = x + w - 1;
    const int y1 = y + h - 1;
    const uint8_t col[] = {
        static_cast<uint8_t>((x >> 8) & 0xff), static_cast<uint8_t>(x & 0xff),
        static_cast<uint8_t>((x1 >> 8) & 0xff), static_cast<uint8_t>(x1 & 0xff),
    };
    const uint8_t row[] = {
        static_cast<uint8_t>((y >> 8) & 0xff), static_cast<uint8_t>(y & 0xff),
        static_cast<uint8_t>((y1 >> 8) & 0xff), static_cast<uint8_t>(y1 & 0xff),
    };
    write_commandn(0x2a, col, sizeof(col));
    write_commandn(0x2b, row, sizeof(row));
    write_command(0x2c);
}

void lcd_fill_rect(int x, int y, int w, int h, uint16_t rgb565) {
    if (w <= 0 || h <= 0) {
        return;
    }
    set_window(x, y, w, h);

    uint8_t line[board::kScreenWidth * 2];
    for (int px = 0; px < w; ++px) {
        line[px * 2] = static_cast<uint8_t>(rgb565 >> 8);
        line[px * 2 + 1] = static_cast<uint8_t>(rgb565 & 0xff);
    }

    select();
    set_dc(true);
    for (int row = 0; row < h; ++row) {
        write_bytes(line, static_cast<size_t>(w) * 2);
    }
    wait_idle();
    deselect();
}

}  // namespace

void init() {
    gpio_init(board::kLcdPinCs);
    gpio_init(board::kLcdPinDc);
    gpio_init(board::kLcdPinRst);
    gpio_init(board::kLcdPinRamCs);
    gpio_init(board::kLcdPinMiso);

    gpio_set_dir(board::kLcdPinCs, GPIO_OUT);
    gpio_set_dir(board::kLcdPinDc, GPIO_OUT);
    gpio_set_dir(board::kLcdPinRst, GPIO_OUT);
    gpio_set_dir(board::kLcdPinRamCs, GPIO_OUT);
    gpio_set_dir(board::kLcdPinMiso, GPIO_IN);
    gpio_disable_pulls(board::kLcdPinMiso);

    gpio_put(board::kLcdPinCs, 1);
    gpio_put(board::kLcdPinDc, 1);
    gpio_put(board::kLcdPinRst, 1);
    gpio_put(board::kLcdPinRamCs, 1);

    g_pio_offset = pio_add_program(g_pio, &lcd_spi_min_program);
    lcd_spi_min_program_init(g_pio, g_sm, g_pio_offset, board::kLcdPinMosi,
                             board::kLcdPinSck, kPioClkDiv);

    reset_panel();

    // ST7365P init.  0x3a=0x65 selects RGB565; 0x21 enables the inversion this
    // panel needs.
    static const uint8_t b9[] = {0x02, 0xe0};
    static const uint8_t c0[] = {0x80, 0x06};
    static const uint8_t e8[] = {0x40, 0x8a, 0x00, 0x00, 0x29, 0x19, 0xaa, 0x33};
    static const uint8_t e0[] = {0xf0, 0x06, 0x0f, 0x05, 0x04, 0x20, 0x37, 0x33,
                                 0x4c, 0x37, 0x13, 0x14, 0x2b, 0x31};
    static const uint8_t e1[] = {0xf0, 0x11, 0x1b, 0x11, 0x0f, 0x0a, 0x37, 0x43,
                                 0x4c, 0x37, 0x13, 0x13, 0x2c, 0x32};

    write_command1(0xf0, 0xc3);
    write_command1(0xf0, 0x96);
    write_command1(0x36, 0x48);
    write_command1(0x3a, 0x65);
    write_command1(0xb1, 0xa0);
    write_command1(0xb4, 0x00);
    write_command1(0xb7, 0xc6);
    write_commandn(0xb9, b9, sizeof(b9));
    write_commandn(0xc0, c0, sizeof(c0));
    write_command1(0xc1, 0x15);
    write_command1(0xc2, 0xa7);
    write_command1(0xc5, 0x04);
    write_commandn(0xe8, e8, sizeof(e8));
    write_commandn(0xe0, e0, sizeof(e0));
    write_commandn(0xe1, e1, sizeof(e1));
    write_command1(0xf0, 0x3c);
    write_command1(0xf0, 0x69);
    write_command1(0x35, 0x00);

    write_command(0x11);
    sleep_ms(120);
    write_command(0x21);
    lcd_fill_rect(0, 0, board::kScreenWidth, board::kScreenHeight, 0x0000);
    write_command(0x29);
    sleep_ms(120);
}

void fill_screen(uint16_t rgb565) {
    lcd_fill_rect(0, 0, board::kScreenWidth, board::kScreenHeight, rgb565);
}

void blit_arduboy(const uint8_t* fb, uint16_t fg, uint16_t bg) {
    constexpr int kW = board::kGameWidth;          // 128
    constexpr int kDw = board::kBlitWidth;         // 256
    constexpr int kDh = board::kBlitHeight;        // 128
    const uint8_t fg_hi = static_cast<uint8_t>(fg >> 8);
    const uint8_t fg_lo = static_cast<uint8_t>(fg & 0xff);
    const uint8_t bg_hi = static_cast<uint8_t>(bg >> 8);
    const uint8_t bg_lo = static_cast<uint8_t>(bg & 0xff);

    set_window(board::kBlitOriginX, board::kBlitOriginY, kDw, kDh);
    select();
    set_dc(true);

    static uint8_t row_buf[kDw * 2];
    for (int dy = 0; dy < kDh; ++dy) {
        const int sy = dy >> 1;                     // 2x vertical scale
        const int page = sy >> 3;                   // SSD1306 page
        const uint8_t mask = static_cast<uint8_t>(1u << (sy & 7));
        const uint8_t* page_row = fb + page * kW;
        uint8_t* out = row_buf;
        for (int sx = 0; sx < kW; ++sx) {
            const bool lit = (page_row[sx] & mask) != 0;
            const uint8_t hi = lit ? fg_hi : bg_hi;
            const uint8_t lo = lit ? fg_lo : bg_lo;
            // each source column maps to 2 destination columns
            *out++ = hi;
            *out++ = lo;
            *out++ = hi;
            *out++ = lo;
        }
        write_bytes(row_buf, sizeof(row_buf));
    }
    wait_idle();
    deselect();
}

}  // namespace pcvb::display
