#ifndef PINS_H
#define PINS_H

// ===========================
// ESP32-CAM PIN DEFINITIONS
// AI-Thinker ESP32-CAM Module
// ===========================

// ===========================
// CAMERA PINS (OV2640)
// ===========================
#define PWDN_GPIO_NUM     32  // Power down pin
#define RESET_GPIO_NUM    -1  // Reset pin (not connected)
#define XCLK_GPIO_NUM      0  // External clock
#define SIOD_GPIO_NUM     26  // SDA pin
#define SIOC_GPIO_NUM     27  // SCL pin

#define Y9_GPIO_NUM       35  // Camera data pins
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25  // Vertical sync
#define HREF_GPIO_NUM     23  // Horizontal reference
#define PCLK_GPIO_NUM     22  // Pixel clock

// ===========================
// MOTION DETECTION PINS
// ===========================
#define PIR_PIN           13  // PIR motion sensor pin
#define PIR_POWER_PIN     12  // PIR sensor power control

// ===========================
// POWER MANAGEMENT PINS
// ===========================
#define BATTERY_VOLTAGE_PIN    33  // Battery voltage monitoring (ADC1_CH5)
#define SOLAR_VOLTAGE_PIN      32  // Solar panel voltage monitoring
#define CHARGING_CONTROL_PIN   14  // Charging control pin
#define POWER_LED_PIN          2   // Power status LED (built-in)
#define CHARGING_LED_PIN       15  // Charging status LED

// ===========================
// STORAGE PINS
// ===========================
// SD Card pins are fixed on ESP32-CAM:
// CMD  = GPIO 15
// CLK  = GPIO 14
// D0   = GPIO 2
// D1   = GPIO 4
// D2   = GPIO 12
// D3   = GPIO 13
// Note: GPIO 4 has built-in LED, GPIO 33 can be used for flash

#define SD_MMC_CMD    15  // SD card command pin
#define SD_MMC_CLK    14  // SD card clock pin
#define SD_MMC_D0      2  // SD card data 0
#define SD_MMC_D1      4  // SD card data 1 (also built-in LED)
#define SD_MMC_D2     12  // SD card data 2
#define SD_MMC_D3     13  // SD card data 3

// ===========================
// SENSOR PINS
// ===========================
#define BME280_SDA_PIN    26  // Environmental sensor SDA (shared with camera)
#define BME280_SCL_PIN    27  // Environmental sensor SCL (shared with camera)
#define BME280_ADDRESS    0x76 // BME280 I2C address

// ===========================
// NIGHT VISION PINS
// ===========================
#define IR_LED_PIN        16  // IR LED control pin (spare GPIO)
#define LIGHT_SENSOR_PIN  33  // Light sensor ADC pin (shared with battery monitoring)

// ===========================
// COMMUNICATION PINS (Optional)
// ===========================
#define UART_TX_PIN       1   // UART TX (USB serial)
#define UART_RX_PIN       3   // UART RX (USB serial)

// Optional LoRa pins (for future expansion)
#define LORA_SCK_PIN      18  // LoRa SPI clock (shared with camera)
#define LORA_MISO_PIN     19  // LoRa SPI MISO (shared with camera)
#define LORA_MOSI_PIN     23  // LoRa SPI MOSI (shared with camera)
#define LORA_CS_PIN       16  // LoRa chip select
#define LORA_RST_PIN      17  // LoRa reset
#define LORA_DIO0_PIN     26  // LoRa DIO0 interrupt

// ===========================
// SPARE/MULTIPURPOSE PINS
// ===========================
#define SPARE_GPIO_1      16  // Spare GPIO for expansion
#define SPARE_GPIO_2      17  // Spare GPIO for expansion

// ===========================
// PIN USAGE NOTES
// ===========================
/*
ESP32-CAM Pin Usage Summary:
- GPIO 0: Camera XCLK (can be used for boot mode selection)
- GPIO 1: UART TX (USB serial)
- GPIO 2: Built-in LED / SD D0
- GPIO 3: UART RX (USB serial)
- GPIO 4: Built-in Flash LED / SD D1
- GPIO 5: Camera Y2
- GPIO 12: SD D2 / PIR Power (careful with boot strapping)
- GPIO 13: PIR Sensor / SD D3
- GPIO 14: Charging Control / SD CLK
- GPIO 15: Charging LED / SD CMD
- GPIO 16: Available for expansion
- GPIO 17: Available for expansion
- GPIO 18: Camera Y3
- GPIO 19: Camera Y4
- GPIO 21: Camera Y5
- GPIO 22: Camera PCLK
- GPIO 23: Camera HREF
- GPIO 25: Camera VSYNC
- GPIO 26: Camera SIOD (I2C SDA)
- GPIO 27: Camera SIOC (I2C SCL)
- GPIO 32: Camera PWDN / Solar Voltage
- GPIO 33: Battery Voltage (ADC only)
- GPIO 34: Camera Y8 (Input only)
- GPIO 35: Camera Y9 (Input only)
- GPIO 36: Camera Y6 (Input only)
- GPIO 39: Camera Y7 (Input only)

Constraints:
- GPIO 34, 35, 36, 39 are input-only
- GPIO 6-11 are connected to SPI flash (do not use)
- GPIO 12 has internal pull-down (affects boot if HIGH)
- GPIO 2 and 15 have internal pull-ups (affect boot mode)
*/

// ===========================
// VOLTAGE DIVIDER RATIOS
// ===========================
#define BATTERY_VOLTAGE_DIVIDER 2.0f  // 2:1 voltage divider for battery
#define SOLAR_VOLTAGE_DIVIDER   3.0f  // 3:1 voltage divider for solar panel

// ===========================
// ADC CONFIGURATION
// ===========================
#define ADC_RESOLUTION 12             // ADC resolution (12-bit = 0-4095)
#define ADC_VREF 3.3f                // ADC reference voltage
#define ADC_SAMPLES 16               // Number of samples for averaging

#endif // PINS_H