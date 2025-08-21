#ifndef ESP32_CAM_PINOUT_H
#define ESP32_CAM_PINOUT_H

// ESP32-CAM (AI-Thinker) Pin Definitions
// This is the most popular and affordable ESP32 camera board

// Camera I2C pins
#define CAMERA_SDA_PIN    21
#define CAMERA_SCL_PIN    22

// Camera data pins
#define CAMERA_D0_PIN     5
#define CAMERA_D1_PIN     18
#define CAMERA_D2_PIN     19
#define CAMERA_D3_PIN     21
#define CAMERA_D4_PIN     36
#define CAMERA_D5_PIN     39
#define CAMERA_D6_PIN     34
#define CAMERA_D7_PIN     35

// Camera control pins
#define CAMERA_XCLK_PIN   0
#define CAMERA_PCLK_PIN   22
#define CAMERA_VSYNC_PIN  25
#define CAMERA_HREF_PIN   23
#define CAMERA_PWDN_PIN   32
#define CAMERA_RESET_PIN  -1  // Not connected

// Additional pins
#define LED_FLASH_PIN     4   // Built-in flash LED
#define SD_CS_PIN         -1  // No built-in SD card
#define PIR_SENSOR_PIN    -1  // No built-in PIR

// Available GPIO pins for external peripherals
#define GPIO_PIN_1        1   // TX0 (use with caution)
#define GPIO_PIN_3        3   // RX0 (use with caution)
#define GPIO_PIN_12       12  // Available for external sensors
#define GPIO_PIN_13       13  // Available for external sensors
#define GPIO_PIN_14       14  // Available for external sensors
#define GPIO_PIN_15       15  // Available for external sensors
#define GPIO_PIN_16       16  // Available for external sensors

// Power and voltage pins
#define BATTERY_ADC_PIN   -1  // No built-in battery monitoring
#define POWER_EN_PIN      -1  // No power enable control

// Board identification
#define BOARD_NAME        "ESP32-CAM"
#define BOARD_VENDOR      "AI-Thinker"
#define DEFAULT_SENSOR    "OV2640"

#endif // ESP32_CAM_PINOUT_H