//include other files
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "gyro.h"
 
 
//Global constants kept in one place so they are quick to change
 
//Pins and bus
#define GYRO_I2C       i2c0
#define GYRO_SDA_PIN   32      // or GP40 (not GP28/29: GP29 is the LCD's SPI CSn)
#define GYRO_SCL_PIN   33      // or GP41
#define GYRO_I2C_HZ    400000  // MPU-6050 max is 400 kHz (RP2350 max could do 1 MHz)
 
//MPU Registers (names kept the same as the MPU-6050 register map)
#define MPU_ADDR          0x68     // AD0 low. Use 0x69 if AD0 is tied high.
#define REG_SMPLRT_DIV    0x19
#define REG_CONFIG        0x1A
#define REG_GYRO_CONFIG   0x1B
#define REG_ACCEL_CONFIG  0x1C
#define REG_ACCEL_XOUT_H  0x3B     // start of 14-byte block: accel(6) temp(2) gyro(6)
#define REG_PWR_MGMT_1    0x6B
#define REG_WHO_AM_I      0x75
 
//Unit conversions (the datasheet calls these "LSB per unit")
#define ACCEL_COUNTS_PER_G    16384.0f  // +-2 g full scale
#define GYRO_COUNTS_PER_DPS   131.0f    // +-250 deg/s full scale
#define RAD_TO_DEG            57.29578f
#define I2C_TIMEOUT_US        2000      // don't hang forever if a wire falls out
 
//Tuning
#define FILTER_GYRO_WEIGHT  0.98f  // complementary filter: how much to trust the gyro
#define MAX_TILT_DEG        30.0f  // degrees of tilt that counts as "full stick"
#define DEADZONE_DEG        3.0f   // degrees around level that count as "no tilt"
#define LEVEL_CAL_SAMPLES   100    // samples averaged when setting the plane (~1 s)
#define READ_PERIOD_MS      10     // 100 Hz update rate
#define STICK_MAX           127    // largest stick value in either direction
 
//Shared state (extern'd in gyro.h)
gyro_data_t gyro;                         // latest sensor values and tilt
volatile int8_t stick_left_right   = 0;   // -127..127, what the game should use
volatile int8_t stick_forward_back = 0;   // -127..127
 
//Private state
static float rate_x_offset_dps, rate_y_offset_dps, rate_z_offset_dps;  // gyro error measured while still
static float level_roll_deg, level_pitch_deg;          // the "level plane" captured at startup
static float filtered_roll_deg, filtered_pitch_deg;    // filtered absolute angles
static uint64_t last_read_time_us;                     // when gyro_read last ran
 
 
//I2C Helper functions
static bool mpu_write(uint8_t reg_addr, uint8_t value)
{
    uint8_t packet[2] = { reg_addr, value };   // register first, then the data byte
    return i2c_write_timeout_us(GYRO_I2C, MPU_ADDR, packet, 2, false, I2C_TIMEOUT_US) == 2;
}
 
static bool mpu_read(uint8_t reg_addr, uint8_t *read_buf, size_t num_bytes)
{
    // Send the register address, keep the bus (nostop = true -> repeated start),
    // then read num_bytes. The MPU-6050 auto-increments the register address.
    if (i2c_write_timeout_us(GYRO_I2C, MPU_ADDR, &reg_addr, 1, true, I2C_TIMEOUT_US) != 1)
    {
        return false;
    }
    return i2c_read_timeout_us(GYRO_I2C, MPU_ADDR, read_buf, num_bytes, false, I2C_TIMEOUT_US) == (int)num_bytes;
}
 
// Reads all 6 axes in one burst so accel and gyro come from the same instant.
static bool mpu_read_raw(int16_t accel_raw[3], int16_t rate_raw[3])
{
    uint8_t raw_bytes[14];
    if (!mpu_read(REG_ACCEL_XOUT_H, raw_bytes, 14))
    {
        return false;
    }
 
    for (int axis = 0; axis < 3; axis++)
    {
        // Data is big-endian: high byte first
        accel_raw[axis] = (int16_t)((raw_bytes[2 * axis]     << 8) | raw_bytes[2 * axis + 1]);
        rate_raw[axis]  = (int16_t)((raw_bytes[8 + 2 * axis] << 8) | raw_bytes[9 + 2 * axis]);  // skip temp (bytes 6, 7)
    }
 
    return true;
}
 
// Tilt from gravity alone (accurate long-term, noisy short-term)
static void accel_angles(float accel_x_g, float accel_y_g, float accel_z_g, float *roll_deg, float *pitch_deg)
{
    *roll_deg  = atan2f(accel_y_g, accel_z_g) * RAD_TO_DEG;
    *pitch_deg = atan2f(-accel_x_g, sqrtf(accel_y_g * accel_y_g + accel_z_g * accel_z_g)) * RAD_TO_DEG;
}
 
//Setup the GY-521
bool gyro_set(void)
{
    //set pins
    i2c_init(GYRO_I2C, GYRO_I2C_HZ);
    gpio_set_function(GYRO_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(GYRO_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(GYRO_SDA_PIN);
    gpio_pull_up(GYRO_SCL_PIN);
 
    //check for sensor (This is used for testing and debugging)
    uint8_t who_am_i = 0;
 
    if (!mpu_read(REG_WHO_AM_I, &who_am_i, 1))
    {
        printf("GY-521: no response at 0x%02X - check wiring/AD0\n", MPU_ADDR);
        return false;
    }
 
    if (who_am_i != 0x68)
    {
        printf("GY-521: WHO_AM_I = 0x%02X (expected 0x68, may be a clone chip)\n", who_am_i);
    }
 
    //configure
    mpu_write(REG_PWR_MGMT_1, 0x80);    // full device reset
    sleep_ms(100);
    mpu_write(REG_PWR_MGMT_1, 0x01);    // wake up, clock = gyro X PLL
    mpu_write(REG_CONFIG, 0x03);        // digital low-pass ~44 Hz
    mpu_write(REG_SMPLRT_DIV, 9);       // 1 kHz / (1 + 9) = 100 Hz sample rate
    mpu_write(REG_GYRO_CONFIG, 0x00);   // +-250 deg/s
    mpu_write(REG_ACCEL_CONFIG, 0x00);  // +-2 g
    sleep_ms(50);
 
    //set plane
    gyro_set_plane();
    return true;
}
 
//Hold the controller still while this runs (~1 s). That position becomes "level".
void gyro_set_plane(void)
{
    int16_t accel_raw[3], rate_raw[3];
    float rate_sum[3]  = {0};
    float accel_sum[3] = {0};
    int good_samples = 0;
 
    for (int sample = 0; sample < LEVEL_CAL_SAMPLES; sample++)
    {
        if (mpu_read_raw(accel_raw, rate_raw))
        {
            for (int axis = 0; axis < 3; axis++)
            {
                rate_sum[axis]  += rate_raw[axis];
                accel_sum[axis] += accel_raw[axis];
            }
 
            good_samples++;
        }
 
        sleep_ms(READ_PERIOD_MS);
    }
 
    if (good_samples == 0)
    {
        return;
    }
 
    //a still gyro should read 0, so whatever it reads is error to subtract later
    rate_x_offset_dps = rate_sum[0] / good_samples / GYRO_COUNTS_PER_DPS;
    rate_y_offset_dps = rate_sum[1] / good_samples / GYRO_COUNTS_PER_DPS;
    rate_z_offset_dps = rate_sum[2] / good_samples / GYRO_COUNTS_PER_DPS;
 
    accel_angles(accel_sum[0] / good_samples / ACCEL_COUNTS_PER_G,
                 accel_sum[1] / good_samples / ACCEL_COUNTS_PER_G,
                 accel_sum[2] / good_samples / ACCEL_COUNTS_PER_G,
                 &level_roll_deg, &level_pitch_deg);
 
    filtered_roll_deg  = level_roll_deg;
    filtered_pitch_deg = level_pitch_deg;
    last_read_time_us  = time_us_64();
}
 
//Have it read any inputs
bool gyro_read(void)
{
    //get sample
    int16_t accel_raw[3], rate_raw[3];
    if (!mpu_read_raw(accel_raw, rate_raw))
    {
        return false;
    }
 
    uint64_t now_us = time_us_64();
    float elapsed_s = (now_us - last_read_time_us) / 1e6f;
    last_read_time_us = now_us;
    if (elapsed_s > 0.1f) elapsed_s = 0.1f;   // e.g. after a breakpoint, don't jump
 
    //convert raw counts to real units
    gyro.accel_x_g  = accel_raw[0] / ACCEL_COUNTS_PER_G;
    gyro.accel_y_g  = accel_raw[1] / ACCEL_COUNTS_PER_G;
    gyro.accel_z_g  = accel_raw[2] / ACCEL_COUNTS_PER_G;
    gyro.rate_x_dps = rate_raw[0] / GYRO_COUNTS_PER_DPS - rate_x_offset_dps;
    gyro.rate_y_dps = rate_raw[1] / GYRO_COUNTS_PER_DPS - rate_y_offset_dps;
    gyro.rate_z_dps = rate_raw[2] / GYRO_COUNTS_PER_DPS - rate_z_offset_dps;
 
    //update information based on accelerometer
    float accel_roll_deg;
    float accel_pitch_deg;
 
    accel_angles(gyro.accel_x_g, gyro.accel_y_g, gyro.accel_z_g, &accel_roll_deg, &accel_pitch_deg);
 
    //extra, this helps with possible drifting
    filtered_roll_deg  = FILTER_GYRO_WEIGHT * (filtered_roll_deg  + gyro.rate_x_dps * elapsed_s)
                       + (1.0f - FILTER_GYRO_WEIGHT) * accel_roll_deg;
    filtered_pitch_deg = FILTER_GYRO_WEIGHT * (filtered_pitch_deg + gyro.rate_y_dps * elapsed_s)
                       + (1.0f - FILTER_GYRO_WEIGHT) * accel_pitch_deg;
 
    gyro.roll_deg  = filtered_roll_deg  - level_roll_deg;
    gyro.pitch_deg = filtered_pitch_deg - level_pitch_deg;
    return true;
}
 
//helper function for reaction
static int8_t tilt_to_stick(float tilt_deg)
{
    float tilt_size = fabsf(tilt_deg);
 
    if (tilt_size < DEADZONE_DEG)
    {
        return 0;
    }
 
    float stick_fraction = (tilt_size - DEADZONE_DEG) / (MAX_TILT_DEG - DEADZONE_DEG);  // 0..1
 
    if (stick_fraction > 1.0f)
    {
        stick_fraction = 1.0f;
    }
 
    int stick_value = (int)(stick_fraction * STICK_MAX);
    return (int8_t)(tilt_deg < 0 ? -stick_value : stick_value);
}
 
//React to any changes and send information out
void gyro_react(void)
{
    //Have the game react to the updated information
    //(swap or negate these if the board is mounted rotated in the controller)
 
    //left or right
    stick_left_right = tilt_to_stick(gyro.roll_deg);
 
    //forward or back
    stick_forward_back = tilt_to_stick(gyro.pitch_deg);
 
    //TODO: send stick_left_right / stick_forward_back to the game
}
 
//function to have the GY-521 constantly read information while the controller is used.
void gyro_task(void)
{
    absolute_time_t next_read_time = get_absolute_time();
 
    #ifdef GYRO_DEBUG
    int reads_since_print = 0;
    #endif
 
    for (;;)
    {
        if (gyro_read())
        {
            gyro_react();
 
            //debug to help for later
            #ifdef GYRO_DEBUG
            if (++reads_since_print >= 10)   // print ~10 times/second
            {
                reads_since_print = 0;
                printf("roll %6.1f  pitch %6.1f  |  left/right %4d  fwd/back %4d\r\n",
                       gyro.roll_deg, gyro.pitch_deg, stick_left_right, stick_forward_back);
            }
            #endif
        }
 
        next_read_time = delayed_by_ms(next_read_time, READ_PERIOD_MS);
        sleep_until(next_read_time);
    }
}
