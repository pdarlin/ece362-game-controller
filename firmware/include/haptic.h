#ifndef HAPTIC_H
#define HAPTIC_H

#include <stdint.h>
#include "pico/stdlib.h"

// initialize PWM for haptic feedback
void haptic_init(void);

// start vibration at specific intensity (0-100)
void haptic_start(uint8_t intensity);

// stop vibration
void haptic_stop(void);

// check if haptic feedback is currently active
bool haptic_is_active(void);

#endif // HAPTIC_H



