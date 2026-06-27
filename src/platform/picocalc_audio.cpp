/*
 * Picocalc BVWC Volleyball - PicoCalc port.
 * SPDX-License-Identifier: MIT
 */

#include "platform/picocalc_audio.h"

#include "config/board_config.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

namespace pcvb::audio {
namespace {

uint g_slice = 0;
uint g_chan_l = 0;
uint g_chan_r = 0;

struct Segment {
    uint16_t freq;
    uint16_t dur;
};

Segment g_queue[2];
int g_seg_count = 0;
int g_seg_index = 0;
uint32_t g_seg_deadline_ms = 0;
bool g_playing = false;

inline uint32_t now_ms() {
    return to_ms_since_boot(get_absolute_time());
}

// Drive a 50% square wave at `freq` by picking a clkdiv/wrap whose product is
// sysclk/freq.  GP26/GP27 share one slice, so both channels get the same level.
void set_freq(uint16_t freq) {
    if (freq == 0) {
        pwm_set_chan_level(g_slice, g_chan_l, 0);
        pwm_set_chan_level(g_slice, g_chan_r, 0);
        return;
    }

    const uint32_t fclk = clock_get_hz(clk_sys);
    uint32_t total = fclk / freq;  // counts per period before division
    float div = 1.0f;
    while (total / static_cast<uint32_t>(div) > 65535u) {
        div += 1.0f;
    }
    uint16_t wrap = static_cast<uint16_t>(total / static_cast<uint32_t>(div));
    if (wrap < 2) {
        wrap = 2;
    }

    pwm_set_clkdiv(g_slice, div);
    pwm_set_wrap(g_slice, wrap);
    const uint16_t level = static_cast<uint16_t>(wrap / 2);  // 50% duty
    pwm_set_chan_level(g_slice, g_chan_l, level);
    pwm_set_chan_level(g_slice, g_chan_r, level);
    pwm_set_enabled(g_slice, true);
}

void start_segment(int index) {
    g_seg_index = index;
    g_seg_deadline_ms = now_ms() + g_queue[index].dur;
    set_freq(g_queue[index].freq);
    g_playing = true;
}

}  // namespace

void init() {
    gpio_set_function(board::kAudioPwmLeft, GPIO_FUNC_PWM);
    gpio_set_function(board::kAudioPwmRight, GPIO_FUNC_PWM);
    g_slice = pwm_gpio_to_slice_num(board::kAudioPwmLeft);
    g_chan_l = pwm_gpio_to_channel(board::kAudioPwmLeft);
    g_chan_r = pwm_gpio_to_channel(board::kAudioPwmRight);

    pwm_config config = pwm_get_default_config();
    pwm_init(g_slice, &config, false);
    pwm_set_chan_level(g_slice, g_chan_l, 0);
    pwm_set_chan_level(g_slice, g_chan_r, 0);
}

void stop() {
    g_playing = false;
    g_seg_count = 0;
    pwm_set_chan_level(g_slice, g_chan_l, 0);
    pwm_set_chan_level(g_slice, g_chan_r, 0);
    pwm_set_enabled(g_slice, false);
}

void tone(uint16_t frequency_hz, uint16_t duration_ms) {
    g_queue[0] = {frequency_hz, duration_ms};
    g_seg_count = 1;
    start_segment(0);
}

void tone2(uint16_t f1, uint16_t d1, uint16_t f2, uint16_t d2) {
    g_queue[0] = {f1, d1};
    g_queue[1] = {f2, d2};
    g_seg_count = 2;
    start_segment(0);
}

void service() {
    if (!g_playing) {
        return;
    }
    if (static_cast<int32_t>(now_ms() - g_seg_deadline_ms) < 0) {
        return;
    }
    const int next = g_seg_index + 1;
    if (next < g_seg_count) {
        start_segment(next);
    } else {
        stop();
    }
}

}  // namespace pcvb::audio
