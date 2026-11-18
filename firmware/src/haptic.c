#include "haptic.h"
#include "hardware/pwm.h"

#define HAPTIC_PIN 10

static bool haptic_active = false;
static uint slice_num;

void haptic_init(void) {
    slice_num = pwm_gpio_to_slice_num(HAPTIC_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 150.0f); // Set clock divider for 1kHz
    pwm_config_set_wrap(&config, 999); // Set TOP value for 1kHz
    gpio_set_function(HAPTIC_PIN, GPIO_FUNC_PWM);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(HAPTIC_PIN, 0); // Start with 0% duty cycle
} 

void haptic_start(uint8_t intensity) {
    if (intensity > 100) {
        intensity = 100; // Set intensity at 100%
    }
    
    uint16_t level = (intensity * 1000) / 100; // Convert intensity to PWM level (0-1000)
    pwm_set_gpio_level(HAPTIC_PIN, level);

    if (intensity > 0) {
        haptic_active = true;
    } else {
        haptic_active = false;
    }
}

void haptic_stop(void) {
    pwm_set_gpio_level(HAPTIC_PIN, 0); // Set duty cycle to 0%
    haptic_active = false;
}

bool haptic_is_active(void) {
    return haptic_active;
}