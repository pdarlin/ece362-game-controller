#include <stdio.h>
#include "pico/stdlib.h"
#include "gyro.h"

int main(void) {
    stdio_init_all();
    sleep_ms(2000);

    printf("Starting MPU-6050 gyro test...\n");
    printf("Keep gyro still during calibration.\n");

    // Initialize and calibrate gyro
    if (!gyro_set()) {
        printf("Gyro not detected!\n");
        while (true) {
            sleep_ms(1000);
        }
    }

    printf("Gyro initialized and calibrated!\n");

    while (true) {
        if (gyro_read()) {
            // Convert tilt into joystick values
            gyro_react();

            printf("Roll: %.2f | Pitch: %.2f | X: %d | Y: %d\n",
                gyro.roll_deg,
                gyro.pitch_deg,
                stick_left_right,
                stick_forward_back);
        } else {
            printf("Gyro read failed!\n");
        }

        sleep_ms(100);
    }
}
