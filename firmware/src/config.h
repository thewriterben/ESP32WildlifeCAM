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

// ===========================
// GPIO PIN ALLOCATION TABLE 
// ===========================
// AI-Thinker ESP32-CAM hardware-fixed pins (CANNOT CHANGE):
// GPIO 0:  Camera XCLK
// GPIO 4:  Camera LED/Flash  
// GPIO 5:  Camera Y2
// GPIO 18: Camera Y3
// GPIO 19: Camera Y4
// GPIO 21: Camera Y5  
// GPIO 22: Camera PCLK
// GPIO 23: Camera HREF
// GPIO 25: Camera VSYNC
// GPIO 26: Camera SIOD (I2C SDA)
// GPIO 27: Camera SIOC (I2C SCL)
// GPIO 32: Camera PWDN
// GPIO 34: Camera Y8 (input only)
// GPIO 35: Camera Y9 (input only)
// GPIO 36: Camera Y6 (input only)
// GPIO 39: Camera Y7 (input only)
//
// Available pins for other functions: 1, 2, 3, 12, 13, 14, 15, 16, 17, 33
// (Note: GPIO 6-11 are flash pins and should be avoided)

// Motion Detection
#define PIR_PIN 1                         // PIR sensor pin - Using available GPIO 1
#define PIR_DEBOUNCE_TIME 2000           // ms - prevent multiple triggers
#define MOTION_DETECTION_ENABLED true    
#define MOTION_SENSITIVITY 50            // 0-100, higher = more sensitive

// Weather Filtering
#define WEATHER_FILTERING_ENABLED true
#define WIND_THRESHOLD 15               // km/h - ignore motion above this wind speed
#define RAIN_THRESHOLD 0.5             // mm/h - ignore motion during rain
#define TEMP_COMP_ENABLED true         // Temperature compensation for PIR

// ===========================
// REALISTIC GPIO PIN ALLOCATION
// ===========================
// AI-Thinker ESP32-CAM has LIMITED available pins due to camera usage.
// This configuration prioritizes core functionality and makes some features optional.
//
// AVAILABLE PINS: 2, 12, 13, 14, 15, 16, 17, 33 (only 8 pins!)
// NEEDED FUNCTIONS: PIR(1) + LoRa(6) + SD(4) + LED(1) + Power(2) + Other(3) = 17 pins
// 
// SOLUTION: Use input-only pins for ADC, make some features optional/shared

// Motion Detection - PRIORITY 1 (gets the last available pin)
#define PIR_PIN 13                        // PIR sensor pin - only remaining available pin
#define PIR_DEBOUNCE_TIME 2000           // ms - prevent multiple triggers
#define MOTION_DETECTION_ENABLED true    
#define MOTION_SENSITIVITY 50            // 0-100, higher = more sensitive

// Weather Filtering
#define WEATHER_FILTERING_ENABLED true
#define WIND_THRESHOLD 15               // km/h - ignore motion above this wind speed
#define RAIN_THRESHOLD 0.5             // mm/h - ignore motion during rain
#define TEMP_COMP_ENABLED true         // Temperature compensation for PIR

// Power Management - Use input-only pins for ADC readings
#define SOLAR_VOLTAGE_PIN 34           // ADC pin - camera Y8 (shared, input-only)
#define BATTERY_VOLTAGE_PIN 35         // ADC pin - camera Y9 (shared, input-only)
#define SOLAR_VOLTAGE_THRESHOLD 3.2    // V - minimum solar voltage for charging
#define BATTERY_LOW_THRESHOLD 3.0      // V - low battery warning
#define BATTERY_CRITICAL_THRESHOLD 2.8 // V - critical battery level
#define DEEP_SLEEP_DURATION 300        // seconds - sleep between checks
#define CHARGING_LED_PIN 2             // Charging indicator LED - only remaining pin

// ===========================
// FINAL PIN ALLOCATION SUMMARY
// ===========================
// Available pins on AI-Thinker ESP32-CAM: 2, 12, 13, 14, 15, 16, 17, 33 (8 pins)
// 
// ASSIGNED PINS (no conflicts):
// GPIO 2:  CHARGING_LED_PIN
// GPIO 12: LORA_MISO  
// GPIO 13: PIR_PIN
// GPIO 14: LORA_SCK
// GPIO 15: LORA_MOSI
// GPIO 16: LORA_CS
// GPIO 17: LORA_RST
// GPIO 33: LORA_DIO0
//
// SHARED PINS (camera + other functions):
// GPIO 34: Camera Y8 + SOLAR_VOLTAGE_PIN (input-only, safe to share)
// GPIO 35: Camera Y9 + BATTERY_VOLTAGE_PIN (input-only, safe to share)
//
// DISABLED FEATURES (due to pin conflicts):
// - SD Card storage (conflicts with LoRa)
// - Vibration sensor (conflicts with LoRa CS)
// - IR LED night vision (conflicts with LoRa DIO0)
// - Satellite communication (conflicts with LoRa and PIR)
//
// ALTERNATIVE: To enable SD card, set LORA_ENABLED to false

// LoRa Mesh Network
#define LORA_ENABLED true
#define LORA_FREQUENCY 915E6           // 433E6, 868E6, 915E6
#define LORA_TX_POWER 20              // dBm (5-20)
#define LORA_SPREADING_FACTOR 7        // 6-12
#define LORA_SIGNAL_BANDWIDTH 125E3    // 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3
#define LORA_CODING_RATE 5            // 5-8
#define LORA_PREAMBLE_LENGTH 8        // 6-65535
#define LORA_SYNC_WORD 0x12           // Network ID

// ===========================
// FINAL GPIO PIN ALLOCATION - CONFLICT FREE
// ===========================
// Available pins: 2, 12, 13, 14, 15, 16, 17, 33 (8 pins total)
// Assignments (no conflicts):

// LoRa Pins (6 pins needed) - CORRECTED TO AVOID ALL CONFLICTS  
#define LORA_SCK 14     // Available (was conflicting with SD CLK)
#define LORA_MISO 12    // Available (was conflicting with SD CS)
#define LORA_MOSI 15    // Available (was conflicting with SD MOSI)
#define LORA_CS 16      // Available 
#define LORA_RST 17     // Available
#define LORA_DIO0 33    // Available

// Network Configuration
#define NODE_ID 1                     // Unique node identifier
#define MAX_MESH_NODES 10            // Maximum nodes in mesh
#define MESH_RETRY_COUNT 3           // Transmission retry attempts
#define MESH_ACK_TIMEOUT 5000        // ms - acknowledgment timeout

// File System - DISABLED DUE TO PIN CONFLICTS
// NOTE: SD card functionality conflicts with LoRa pins on AI-Thinker ESP32-CAM
// Either disable LoRa OR disable SD card, cannot have both with current pin layout
#define SD_CARD_ENABLED false        // Disabled due to insufficient pins
// #define SD_CS_PIN 12                 // CONFLICTS with LORA_MISO
// #define SD_MOSI_PIN 15               // CONFLICTS with LORA_MOSI  
// #define SD_CLK_PIN 14                // CONFLICTS with LORA_SCK
// #define SD_MISO_PIN 2                // Available but only 1 pin not enough
#define IMAGE_FOLDER "/images"       // Folder for captured images (LittleFS)
#define LOG_FOLDER "/logs"          // Folder for system logs (LittleFS)
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

// Additional Sensors - DISABLED DUE TO PIN CONFLICTS
// NOTE: These sensors conflict with LoRa pins on AI-Thinker ESP32-CAM
#define VIBRATION_ENABLED false       // Disabled - would conflict with LORA_CS pin 16
// #define VIBRATION_PIN 16            // CONFLICTS with LORA_CS
#define IR_LED_ENABLED false          // Disabled - would conflict with LORA_DIO0 pin 33
// #define IR_LED_PIN 33               // CONFLICTS with LORA_DIO0  
#define NIGHT_VISION_ENABLED false    // Disabled due to IR LED conflict

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

// ===========================
// TIMING CONFIGURATION
// ===========================

// System Update Intervals
#define VOLTAGE_CHECK_INTERVAL 5000      // ms - interval for reading battery/solar voltages
#define POWER_LOG_INTERVAL 60000         // ms - interval for logging power status
#define SENSOR_READ_INTERVAL 1000        // ms - interval for reading environmental sensors
#define HEARTBEAT_INTERVAL 30000         // ms - interval for sending network heartbeats

// Communication Timeouts and Delays
#define SERIAL_INIT_DELAY 1000           // ms - delay for serial initialization
#define NETWORK_RETRY_DELAY 2000         // ms - delay between network transmission attempts
#define SATELLITE_RESPONSE_DELAY 2000    // ms - delay waiting for satellite module response
#define SATELLITE_WAKEUP_DELAY 100       // ms - delay for satellite module wake up
#define LORA_TRANSMISSION_DELAY 1000     // ms - simulated delay for LoRa transmission
#define COMMUNICATION_SETUP_DELAY 5000   // ms - delay for communication module setup

// ===========================
// HARDWARE CONFIGURATION
// ===========================

// ADC and Voltage Measurement
#define ADC_RESOLUTION 12                // bits - ADC resolution (12-bit = 0-4095)
#define VOLTAGE_DIVIDER_RATIO 2.0        // ratio for voltage divider circuits
#define ADC_REFERENCE_VOLTAGE 3.3        // V - reference voltage for ADC
#define VOLTAGE_CALIBRATION_SAMPLES 10   // number of samples for voltage averaging

// ===========================
// COMMUNICATION CONFIGURATION
// ===========================

// Satellite Communication - DISABLED DUE TO PIN CONFLICTS
// NOTE: Satellite module pins conflict with LoRa on AI-Thinker ESP32-CAM
#define SATELLITE_ENABLED false             // Disabled due to pin conflicts
#define SATELLITE_BAUD_RATE 19200        // baud rate for satellite module communication
#define SATELLITE_RETRY_COUNT 5          // number of retry attempts for satellite commands
#define SATELLITE_MESSAGE_MAX_LENGTH 100 // max bytes per satellite message
// #define SAT_SLEEP_PIN 12                 // CONFLICTS with LORA_MISO
// #define SAT_RING_PIN 13                  // CONFLICTS with PIR_PIN

// LoRa Mesh Network
#define LORA_MESSAGE_QUEUE_SIZE 10       // maximum queued messages
#define LORA_ROUTING_TABLE_SIZE MAX_MESH_NODES // size of routing table

// Network Performance
#define NETWORK_SCORE_EXCELLENT 100      // score for highest priority network
#define CELLULAR_DATA_CHUNK_SIZE 512     // bytes per cellular transmission chunk

// ===========================
// ALGORITHM CONFIGURATION
// ===========================

// Motion Detection
#define MOTION_CONSECUTIVE_THRESHOLD 3   // consecutive motions needed for valid detection
#define DEFAULT_TEMPERATURE 20.0         // °C - default temperature for motion compensation

// Data Compression
#define RLE_MAX_COUNT 255                // maximum run-length encoding count
#define COMPRESSION_MIN_EFFICIENCY 3     // minimum bytes for RLE compression
#define BASE64_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

// Image Processing
#define THUMBNAIL_GENERATION_ENABLED true // enable thumbnail creation
#define IMAGE_METADATA_ENABLED true      // enable image metadata embedding

// ===========================
// SYSTEM CONFIGURATION
// ===========================

// CPU Performance
#define NORMAL_CPU_FREQUENCY 240         // MHz - normal operation frequency
#define POWER_SAVE_CPU_FREQUENCY 80      // MHz - power saving mode frequency

// Memory Management
#define BUFFER_SIZE_SMALL 256            // bytes - small buffer size
#define BUFFER_SIZE_MEDIUM 1024          // bytes - medium buffer size  
#define BUFFER_SIZE_LARGE 4096           // bytes - large buffer size
#define JSON_BUFFER_SMALL 256            // bytes - small JSON document buffer
#define JSON_BUFFER_MEDIUM 512           // bytes - medium JSON document buffer
#define JSON_BUFFER_LARGE 1024           // bytes - large JSON document buffer

// Function-like macros for common operations
#define VOLTAGE_TO_ADC(voltage) ((voltage) * 4095 / ADC_REFERENCE_VOLTAGE)
#define ADC_TO_VOLTAGE(adc) ((adc) * ADC_REFERENCE_VOLTAGE / 4095)
#define PERCENTAGE(value, max_value) ((value) * 100 / (max_value))

#endif // CONFIG_H