#ifndef CONFIG_H
#define CONFIG_H

// ===========================
// WILDLIFE CAMERA CONFIG
// ===========================

// Camera Configuration
#define CAMERA_MODEL_AI_THINKER  // AI-Thinker ESP32-CAM
// #define CAMERA_MODEL_WROVER_KIT
// #define CAMERA_MODEL_ESP_EYE
// #define CAMERA_MODEL_M5STACK_PSRAM
// #define CAMERA_MODEL_M5STACK_V2_PSRAM
// #define CAMERA_MODEL_M5STACK_WIDE
// #define CAMERA_MODEL_M5STACK_ESP32CAM
// #define CAMERA_MODEL_TTGO_T_JOURNAL

// Camera Settings
#define CAMERA_FRAME_SIZE FRAMESIZE_UXGA  // 1600x1200
#define CAMERA_JPEG_QUALITY 12            // 10-63 lower means higher quality
#define CAMERA_FB_COUNT 2                 // Frame buffer count

// Motion Detection
#define PIR_PIN 13                        // PIR sensor pin
#define PIR_DEBOUNCE_TIME 2000           // ms - prevent multiple triggers
#define MOTION_DETECTION_ENABLED true    
#define MOTION_SENSITIVITY 50            // 0-100, higher = more sensitive

// Weather Filtering
#define WEATHER_FILTERING_ENABLED true
#define WIND_THRESHOLD 15               // km/h - ignore motion above this wind speed
#define RAIN_THRESHOLD 0.5             // mm/h - ignore motion during rain
#define TEMP_COMP_ENABLED true         // Temperature compensation for PIR

// Power Management
#define SOLAR_VOLTAGE_PIN 34           // ADC pin for solar voltage monitoring
#define BATTERY_VOLTAGE_PIN 35         // ADC pin for battery voltage monitoring
#define SOLAR_VOLTAGE_THRESHOLD 3.2    // V - minimum solar voltage for charging
#define BATTERY_LOW_THRESHOLD 3.0      // V - low battery warning
#define BATTERY_CRITICAL_THRESHOLD 2.8 // V - critical battery level
#define DEEP_SLEEP_DURATION 300        // seconds - sleep between checks
#define CHARGING_LED_PIN 2             // Charging indicator LED

// LoRa Mesh Network
#define LORA_ENABLED true
#define LORA_FREQUENCY 915E6           // 433E6, 868E6, 915E6
#define LORA_TX_POWER 20              // dBm (5-20)
#define LORA_SPREADING_FACTOR 7        // 6-12
#define LORA_SIGNAL_BANDWIDTH 125E3    // 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3
#define LORA_CODING_RATE 5            // 5-8
#define LORA_PREAMBLE_LENGTH 8        // 6-65535
#define LORA_SYNC_WORD 0x12           // Network ID

// LoRa Pins (SX1276/SX1262)
#define LORA_SCK 18
#define LORA_MISO 19
#define LORA_MOSI 23
#define LORA_CS 5
#define LORA_RST 14
#define LORA_DIO0 26

// Network Configuration
#define NODE_ID 1                     // Unique node identifier
#define MAX_MESH_NODES 10            // Maximum nodes in mesh
#define MESH_RETRY_COUNT 3           // Transmission retry attempts
#define MESH_ACK_TIMEOUT 5000        // ms - acknowledgment timeout

// File System
#define SD_CS_PIN 12                 // SD card chip select
#define SD_MOSI_PIN 15
#define SD_CLK_PIN 14
#define SD_MISO_PIN 2
#define IMAGE_FOLDER "/images"       // Folder for captured images
#define LOG_FOLDER "/logs"          // Folder for system logs
#define MAX_FILES_PER_DAY 100       // Maximum images per day

// Time Management
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 0            // GMT offset in seconds
#define DAYLIGHT_OFFSET_SEC 3600    // Daylight saving offset

// Environmental Sensors (BME280)
#define BME280_ENABLED false        // Enable weather sensor
#define BME280_ADDRESS 0x76         // I2C address
#define BME280_SDA 21              // I2C data pin
#define BME280_SCL 22              // I2C clock pin

// ADXL345 Accelerometer
#define ADXL345_ENABLED false       // Enable accelerometer
#define ADXL345_ADDRESS 0x53        // I2C address (same bus as BME280)

// Additional Sensors
#define VIBRATION_PIN 16            // SW-420 vibration sensor pin
#define IR_LED_PIN 33               // 12V IR LED ring control (MOSFET gate)
#define NIGHT_VISION_ENABLED true   // Enable IR LEDs for night mode

// Trigger Settings
#define TRIGGER_ACTIVE_HOURS_START 6   // Hour (24h format) - start active period
#define TRIGGER_ACTIVE_HOURS_END 20    // Hour (24h format) - end active period
#define NIGHT_MODE_ENABLED false       // Enable night photography
#define MAX_DAILY_TRIGGERS 50          // Maximum triggers per day

// Image Processing
#define IMAGE_TIMESTAMP_ENABLED true   // Add timestamp to images
#define IMAGE_COMPRESSION_ENABLED true // Enable image compression for transmission
#define THUMBNAIL_ENABLED true         // Generate thumbnails for quick preview
#define THUMBNAIL_SIZE 160             // Thumbnail width in pixels

// Debug and Logging
#define DEBUG_ENABLED true
#define SERIAL_BAUD_RATE 115200
#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3
#define CURRENT_LOG_LEVEL LOG_LEVEL_INFO

// Firmware Information
#define FIRMWARE_VERSION "1.0.0"
#define HARDWARE_VERSION "1.0"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

// Pin Definitions for AI-Thinker ESP32-CAM
#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define CAMERA_LED_PIN     4  // Built-in LED
#endif

// Utility Macros
#define DEBUG_PRINT(x) if(DEBUG_ENABLED) Serial.print(x)
#define DEBUG_PRINTLN(x) if(DEBUG_ENABLED) Serial.println(x)
#define DEBUG_PRINTF(format, ...) if(DEBUG_ENABLED) Serial.printf(format, ##__VA_ARGS__)

// Function-like macros for common operations
#define VOLTAGE_TO_ADC(voltage) ((voltage) * 4095 / 3.3)
#define ADC_TO_VOLTAGE(adc) ((adc) * 3.3 / 4095)
#define PERCENTAGE(value, max_value) ((value) * 100 / (max_value))

#endif // CONFIG_H