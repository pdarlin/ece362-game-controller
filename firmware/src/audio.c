#include "audio.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define AUDIO_PIN 12
#define AUDIO_CLKDIV 125.0f

static uint audio_slice;
static uint64_t stop_time = 0;

void audio_init(void) {
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    audio_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    pwm_set_clkdiv(audio_slice, AUDIO_CLKDIV);
    pwm_set_gpio_level(AUDIO_PIN, 0);
    pwm_set_enabled(audio_slice, true);
}

void audio_stop(void) {
    pwm_set_gpio_level(AUDIO_PIN, 0);
    stop_time = 0;
}

void audio_play_tone(uint32_t frequency,
                     uint32_t duration_ms) {
    if (frequency == 0 || duration_ms == 0) {
        audio_stop();
        return;
    }

    uint32_t ticks =
        (uint32_t)(clock_get_hz(clk_sys) /
                   AUDIO_CLKDIV / frequency);

    if (ticks < 2 || ticks > 65536) {
        audio_stop();
        return;
    }

    pwm_set_wrap(audio_slice, ticks - 1);
    pwm_set_gpio_level(AUDIO_PIN, ticks / 2);

    stop_time = time_us_64() +
                (uint64_t)duration_ms * 1000;
}

void audio_update(void) {
    if (stop_time != 0 && time_us_64() >= stop_time) {
        audio_stop();
    }
}
