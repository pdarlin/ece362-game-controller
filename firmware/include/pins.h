
#ifndef PINS_H
#define PINS_H

// ========================================
// MPU-9250 Motion Sensor (I2C)
// ========================================
#define MPU_SDA_PIN         4
#define MPU_SCL_PIN         5
#define MPU_INT_PIN         6

// ========================================
// Left Joystick (KY-023)
// ========================================
#define JOYSTICK_L_X_PIN    40
#define JOYSTICK_L_Y_PIN    41
#define JOYSTICK_L_SW_PIN   26

// ========================================
// Right Joystick (KY-023)
// ========================================
#define JOYSTICK_R_X_PIN    45
#define JOYSTICK_R_Y_PIN    47
#define JOYSTICK_R_SW_PIN   3

// ========================================
// TFT Display (SPI1)
// ========================================
#define TFT_CS_PIN          15
#define TFT_RST_PIN         14
#define TFT_DC_PIN          12
#define TFT_MOSI_PIN        11
#define TFT_SCK_PIN         10

// ========================================
// microSD Card (SPI0)
// ========================================
#define SD_CS_PIN           22
#define SD_MOSI_PIN         19
#define SD_MISO_PIN         0
#define SD_SCK_PIN          18

// ========================================
// Haptic Motors (PWM)
// ========================================
#define HAPTIC_LEFT_PIN     25
#define HAPTIC_RIGHT_PIN    21

// ========================================
// Passive Buzzer (PWM)
// ========================================
#define BUZZER_PIN          23

// ========================================
// D-Pad Buttons
// ========================================
#define DPAD_SW1_PIN        30
#define DPAD_SW2_PIN        29
#define DPAD_SW3_PIN        31
#define DPAD_SW4_PIN        1

#endif // PINS_H
