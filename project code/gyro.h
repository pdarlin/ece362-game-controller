#ifndef GYRO_H
#define GYRO_H
 
#include <stdint.h>
#include <stdbool.h>
 
// Uncomment to print roll/pitch/stick values to the Serial Monitor
// #define GYRO_DEBUG
 
typedef struct {
    float accel_x_g, accel_y_g, accel_z_g;      // acceleration in g
    float rate_x_dps, rate_y_dps, rate_z_dps;   // rotation speed in deg/s (offset removed)
    float roll_deg;                             // left/right tilt from level, in degrees
    float pitch_deg;                            // forward/back tilt from level, in degrees
} gyro_data_t;
 
extern gyro_data_t gyro;
extern volatile int8_t stick_left_right;     // -127 .. 127
extern volatile int8_t stick_forward_back;   // -127 .. 127
 
bool gyro_set(void);        // pins + sensor setup + level calibration
void gyro_set_plane(void);  // re-zero "level" any time (e.g. on a button press)
bool gyro_read(void);       // read one sample, update gyro.roll_deg / gyro.pitch_deg
void gyro_react(void);      // convert tilt into stick_left_right / stick_forward_back
void gyro_task(void);       // infinite 100 Hz read/react loop
 
#endif
