#ifndef CONFIG_H
#define CONFIG_H

// ===========================
// ESP32 WILDLIFE CAMERA CONFIGURATION
// Centralized configuration for all subsystems
// ===========================

// ===========================
// CAMERA CONFIGURATION
// ===========================

// Camera Board Selection (choose one)
#define CAMERA_MODEL_AI_THINKER  // AI-Thinker ESP32-CAM
// #define CAMERA_MODEL_WROVER_KIT
// #define CAMERA_MODEL_ESP_EYE
// #define CAMERA_MODEL_M5STACK_PSRAM
// #define CAMERA_MODEL_M5STACK_V2_PSRAM
// #define CAMERA_MODEL_M5STACK_WIDE
// #define CAMERA_MODEL_M5STACK_ESP32CAM
// #define CAMERA_MODEL_TTGO_T_JOURNAL

// Camera Capture Settings
#define CAMERA_FRAME_SIZE FRAMESIZE_UXGA  // Image resolution: 1600x1200
#define CAMERA_JPEG_QUALITY 12            // JPEG quality: 10-63 (lower = higher quality)
#define CAMERA_FB_COUNT 2                 // Frame buffer count for smooth capture
#define CAMERA_PIXEL_FORMAT PIXFORMAT_JPEG // Image format
#define CAMERA_GRAB_MODE CAMERA_GRAB_LATEST // Frame grab mode

// Camera Sensor Configuration
#define CAMERA_BRIGHTNESS_DEFAULT 0       // Brightness: -2 to 2
#define CAMERA_CONTRAST_DEFAULT 0         // Contrast: -2 to 2  
#define CAMERA_SATURATION_DEFAULT 0       // Saturation: -2 to 2
#define CAMERA_SHARPNESS_DEFAULT 0        // Sharpness: -2 to 2
#define CAMERA_DENOISE_DEFAULT 0          // Denoise: 0 = disable, 1 = enable
#define CAMERA_AEC_VALUE_DEFAULT 300      // Auto exposure control value
#define CAMERA_AGC_GAIN_DEFAULT 0         // Auto gain control value
#define CAMERA_AWB_GAIN_DEFAULT 1         // Auto white balance gain
#define CAMERA_WB_MODE_DEFAULT 0          // White balance mode
#define CAMERA_AE_LEVEL_DEFAULT 0         // Auto exposure level
#define CAMERA_GAIN_CEILING_DEFAULT GAINCEILING_2X // Maximum sensor gain

// Camera Feature Enables
#define AUTO_EXPOSURE_ENABLED true        // Enable automatic exposure control
#define AUTO_WHITE_BALANCE_ENABLED true   // Enable automatic white balance
#define LENS_CORRECTION_ENABLED true      // Enable lens distortion correction

// ===========================
// MOTION DETECTION CONFIGURATION
// ===========================

// PIR Sensor Settings
#define PIR_PIN 13                        // PIR sensor GPIO pin
#define PIR_DEBOUNCE_TIME 2000           // ms - prevent multiple triggers
#define PIR_TRIGGER_MODE RISING          // Interrupt trigger mode
#define MOTION_DETECTION_ENABLED true    // Enable/disable motion detection
#define MOTION_SENSITIVITY 50            // 0-100, higher = more sensitive
#define MOTION_TIMEOUT 30000             // ms - motion detection timeout
#define MOTION_CONSECUTIVE_THRESHOLD 3   // Number of consecutive motions to confirm

// Weather Filtering Settings
#define WEATHER_FILTERING_ENABLED true   // Enable weather-based motion filtering
#define WIND_THRESHOLD 15.0              // km/h - ignore motion above this wind speed
#define RAIN_THRESHOLD 0.5               // mm/h - ignore motion during rain
#define TEMP_COMP_ENABLED true           // Temperature compensation for PIR
#define TEMP_STABILITY_THRESHOLD 2.0     // °C - temperature change threshold
#define WEATHER_READING_INTERVAL 60000   // ms - how often to read weather sensors

// ===========================
// POWER MANAGEMENT CONFIGURATION
// ===========================

// ADC and Voltage Monitoring
#define SOLAR_VOLTAGE_PIN 34             // ADC pin for solar voltage monitoring
#define BATTERY_VOLTAGE_PIN 35           // ADC pin for battery voltage monitoring
#define ADC_RESOLUTION 12                // ADC resolution in bits (12-bit = 0-4095)
#define ADC_REFERENCE_VOLTAGE 3.3        // ADC reference voltage
#define VOLTAGE_DIVIDER_RATIO 2.0        // Voltage divider ratio for scaling
#define VOLTAGE_READINGS_COUNT 10        // Number of readings to average
#define VOLTAGE_READING_DELAY 10         // ms - delay between voltage readings

// Battery Thresholds (in volts)
#define BATTERY_FULL_VOLTAGE 4.2         // Maximum battery voltage
#define BATTERY_NORMAL_VOLTAGE 3.8       // Normal operation threshold
#define BATTERY_GOOD_VOLTAGE 3.4         // Good battery level
#define BATTERY_LOW_THRESHOLD 3.0        // Low battery warning threshold
#define BATTERY_CRITICAL_THRESHOLD 2.8   // Critical battery level
#define BATTERY_SHUTDOWN_VOLTAGE 2.5     // Emergency shutdown voltage

// Solar Charging Configuration  
#define SOLAR_VOLTAGE_THRESHOLD 3.2      // V - minimum solar voltage for charging
#define SOLAR_CHARGING_VOLTAGE_MIN 4.0   // V - minimum solar voltage to start charging
#define SOLAR_OPTIMAL_VOLTAGE 5.0        // V - optimal solar voltage

// Power Management Timings
#define VOLTAGE_CHECK_INTERVAL 5000      // ms - how often to check voltages
#define POWER_LOG_INTERVAL 60000         // ms - how often to log power status
#define DEEP_SLEEP_DURATION 300          // seconds - sleep between checks
#define LOW_POWER_CPU_FREQ 80            // MHz - reduced CPU frequency for power saving

// Power Indicator LED
#define CHARGING_LED_PIN 2               // Charging indicator LED pin

// ===========================
// LORA MESH NETWORK CONFIGURATION
// ===========================

// LoRa Module Enable/Disable
#define LORA_ENABLED true                // Enable/disable LoRa functionality

// LoRa Radio Configuration
#define LORA_FREQUENCY 915E6             // Frequency: 433E6, 868E6, 915E6 Hz
#define LORA_TX_POWER 20                 // TX Power: 5-20 dBm
#define LORA_SPREADING_FACTOR 7          // Spreading Factor: 6-12
#define LORA_SIGNAL_BANDWIDTH 125E3      // Bandwidth: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3
#define LORA_CODING_RATE 5               // Coding Rate: 5-8
#define LORA_PREAMBLE_LENGTH 8           // Preamble Length: 6-65535
#define LORA_SYNC_WORD 0x12              // Network sync word (network ID)
#define LORA_CRC_ENABLED true            // Enable CRC checking

// LoRa GPIO Pin Configuration
#define LORA_SCK 18                      // SPI clock pin
#define LORA_MISO 19                     // SPI MISO pin
#define LORA_MOSI 23                     // SPI MOSI pin
#define LORA_CS 5                        // Chip select pin
#define LORA_RST 14                      // Reset pin
#define LORA_DIO0 26                     // DIO0 interrupt pin

// Mesh Network Configuration
#define NODE_ID 1                        // Unique node identifier (1-255)
#define MAX_MESH_NODES 10                // Maximum nodes in mesh network
#define MESH_RETRY_COUNT 3               // Transmission retry attempts
#define MESH_ACK_TIMEOUT 5000            // ms - acknowledgment timeout
#define MESH_ROUTE_TIMEOUT 30000         // ms - route discovery timeout
#define MESH_HEARTBEAT_INTERVAL 60000    // ms - heartbeat message interval
#define MESH_MAX_HOPS 5                  // Maximum hops for message forwarding

// Message Configuration
#define LORA_MAX_PACKET_SIZE 255         // Maximum LoRa packet size
#define LORA_MESSAGE_QUEUE_SIZE 10       // Maximum queued messages
#define LORA_TRANSMISSION_TIMEOUT 10000  // ms - transmission timeout

// ===========================
// WIFI CONFIGURATION
// ===========================

#define WIFI_ENABLED false               // Enable/disable WiFi functionality
#define WIFI_SSID ""                     // WiFi network name
#define WIFI_PASSWORD ""                 // WiFi network password
#define WIFI_TIMEOUT 10000               // ms - WiFi connection timeout
#define WIFI_RETRY_COUNT 3               // WiFi connection retry attempts
#define WIFI_SLEEP_MODE WIFI_PS_MIN_MODEM // WiFi power save mode

// ===========================
// FILE SYSTEM CONFIGURATION
// ===========================

// SD Card Configuration
#define SD_CS_PIN 12                     // SD card chip select pin
#define SD_MOSI_PIN 15                   // SD card MOSI pin
#define SD_CLK_PIN 14                    // SD card clock pin
#define SD_MISO_PIN 2                    // SD card MISO pin
#define SD_SPI_FREQ 40000000            // SD card SPI frequency (40MHz)

// Storage Paths and Limits
#define IMAGE_FOLDER "/images"           // Folder for captured images
#define LOG_FOLDER "/logs"               // Folder for system logs
#define CONFIG_FOLDER "/config"          // Folder for configuration files
#define MAX_FILES_PER_DAY 100            // Maximum images per day
#define MAX_FILE_SIZE 1048576            // Maximum file size (1MB)
#define FILENAME_TIMESTAMP_FORMAT "%Y%m%d_%H%M%S" // Timestamp format for filenames

// ===========================
// TIME MANAGEMENT CONFIGURATION
// ===========================

// NTP Configuration
#define NTP_ENABLED false                // Enable NTP time synchronization
#define NTP_SERVER "pool.ntp.org"        // NTP server address
#define GMT_OFFSET_SEC 0                 // GMT offset in seconds
#define DAYLIGHT_OFFSET_SEC 3600         // Daylight saving offset in seconds
#define NTP_UPDATE_INTERVAL 86400000     // ms - NTP update interval (24 hours)

// RTC Configuration
#define RTC_ENABLED false                // Enable external RTC module
#define RTC_SDA_PIN 21                   // RTC I2C data pin
#define RTC_SCL_PIN 22                   // RTC I2C clock pin

// ===========================
// ENVIRONMENTAL SENSORS CONFIGURATION
// ===========================

// BME280 Weather Sensor
#define BME280_ENABLED false             // Enable BME280 weather sensor
#define BME280_ADDRESS 0x76              // I2C address (0x76 or 0x77)
#define BME280_SDA 21                    // I2C data pin
#define BME280_SCL 22                    // I2C clock pin
#define BME280_READING_INTERVAL 30000    // ms - sensor reading interval

// Additional Environmental Sensors
#define LIGHT_SENSOR_ENABLED false       // Enable light level sensor
#define LIGHT_SENSOR_PIN 36              // ADC pin for light sensor
#define WIND_SENSOR_ENABLED false        // Enable wind speed sensor
#define WIND_SENSOR_PIN 39               // GPIO pin for wind sensor

// ===========================
// TRIGGER AND TIMING CONFIGURATION
// ===========================

// Active Hours Configuration
#define TRIGGER_ACTIVE_HOURS_START 6     // Hour (24h format) - start active period
#define TRIGGER_ACTIVE_HOURS_END 20      // Hour (24h format) - end active period
#define NIGHT_MODE_ENABLED false         // Enable night photography
#define MAX_DAILY_TRIGGERS 50            // Maximum triggers per day
#define TRIGGER_COOLDOWN_PERIOD 5000     // ms - minimum time between triggers

// Schedule Configuration
#define DAILY_RESET_HOUR 0               // Hour to reset daily counters
#define MAINTENANCE_HOUR 3               // Hour for maintenance tasks
#define STATUS_REPORT_INTERVAL 300000    // ms - system status reporting interval (5 min)

// ===========================
// IMAGE PROCESSING CONFIGURATION
// ===========================

// Image Enhancement
#define IMAGE_TIMESTAMP_ENABLED true     // Add timestamp overlay to images
#define IMAGE_COMPRESSION_ENABLED true   // Enable image compression for transmission
#define THUMBNAIL_ENABLED true           // Generate thumbnails for quick preview
#define THUMBNAIL_SIZE 160               // Thumbnail width in pixels
#define WATERMARK_ENABLED false          // Add watermark to images

// Image Processing Settings
#define AUTO_EXPOSURE_ENABLED true       // Enable automatic exposure adjustment
#define AUTO_WHITE_BALANCE_ENABLED true  // Enable automatic white balance
#define NOISE_REDUCTION_ENABLED true     // Enable noise reduction
#define LENS_CORRECTION_ENABLED true     // Enable lens distortion correction

// ===========================
// DEBUG AND LOGGING CONFIGURATION
// ===========================

// Debug Settings
#define DEBUG_ENABLED true               // Enable/disable debug output
#define SERIAL_BAUD_RATE 115200          // Serial communication baud rate
#define DEBUG_BUFFER_SIZE 256            // Debug message buffer size

// Logging Levels
#define LOG_LEVEL_ERROR 0                // Error messages only
#define LOG_LEVEL_WARN 1                 // Warning and error messages
#define LOG_LEVEL_INFO 2                 // Info, warning, and error messages
#define LOG_LEVEL_DEBUG 3                // All messages including debug
#define CURRENT_LOG_LEVEL LOG_LEVEL_INFO // Current logging level

// Logging Configuration
#define LOG_TO_SERIAL true               // Enable serial logging
#define LOG_TO_FILE false                // Enable file logging
#define LOG_FILE_MAX_SIZE 1048576        // Maximum log file size (1MB)
#define LOG_ROTATION_ENABLED false       // Enable log file rotation

// ===========================
// MULTI-BOARD COMMUNICATION CONFIGURATION
// ===========================

// Multi-Board System Configuration
#define MULTIBOARD_ENABLED true          // Enable multi-board coordination system
#define MULTIBOARD_NODE_ID 1             // Default node ID (should be unique per device)
#define MULTIBOARD_PREFERRED_ROLE ROLE_NODE // Default preferred role
#define MULTIBOARD_DISCOVERY_TIMEOUT 120000 // ms - Discovery timeout (2 minutes)
#define MULTIBOARD_ROLE_CHANGE_TIMEOUT 60000 // ms - Role change timeout (1 minute)
#define MULTIBOARD_HEARTBEAT_INTERVAL 60000 // ms - Heartbeat interval (1 minute)
#define MULTIBOARD_TASK_TIMEOUT 300000   // ms - Task execution timeout (5 minutes)
#define MULTIBOARD_COORDINATOR_TIMEOUT 600000 // ms - Coordinator availability timeout (10 minutes)

// Multi-Board Features
#define MULTIBOARD_AUTO_ROLE_SELECTION true // Enable automatic role selection based on capabilities
#define MULTIBOARD_STANDALONE_FALLBACK true // Enable standalone mode when coordinator unavailable
#define MULTIBOARD_TASK_EXECUTION true   // Enable task execution on nodes
#define MULTIBOARD_LOAD_BALANCING true   // Enable intelligent task distribution
#define MULTIBOARD_TOPOLOGY_MAPPING true // Enable network topology mapping

// Integration Settings
#define MULTIBOARD_LORA_INTEGRATION true // Integrate with existing LoRa mesh system
#define MULTIBOARD_NETWORK_SELECTOR_INTEGRATION true // Integrate with network selector

// ===========================
// FIRMWARE INFORMATION
// ===========================

#define FIRMWARE_VERSION "1.1.0"         // Firmware version string (updated for multi-board support)
#define HARDWARE_VERSION "1.0"           // Hardware version string
#define BUILD_DATE __DATE__               // Build date (automatically set)
#define BUILD_TIME __TIME__               // Build time (automatically set)
#define DEVICE_NAME "WildlifeCam"         // Device identification name

// ===========================
// PIN DEFINITIONS BY CAMERA MODEL
// ===========================

// AI-Thinker ESP32-CAM Pin Definitions
#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32             // Power down pin
#define RESET_GPIO_NUM    -1             // Reset pin (not connected)
#define XCLK_GPIO_NUM      0             // External clock pin
#define SIOD_GPIO_NUM     26             // I2C data pin for camera
#define SIOC_GPIO_NUM     27             // I2C clock pin for camera

// Camera Data Pins
#define Y9_GPIO_NUM       35             // Camera data bit 9
#define Y8_GPIO_NUM       34             // Camera data bit 8
#define Y7_GPIO_NUM       39             // Camera data bit 7
#define Y6_GPIO_NUM       36             // Camera data bit 6
#define Y5_GPIO_NUM       21             // Camera data bit 5
#define Y4_GPIO_NUM       19             // Camera data bit 4
#define Y3_GPIO_NUM       18             // Camera data bit 3
#define Y2_GPIO_NUM        5             // Camera data bit 2
#define VSYNC_GPIO_NUM    25             // Vertical sync pin
#define HREF_GPIO_NUM     23             // Horizontal reference pin
#define PCLK_GPIO_NUM     22             // Pixel clock pin

#define CAMERA_LED_PIN     4             // Built-in camera LED pin
#endif

// TODO: Add pin definitions for other camera models as needed

// ===========================
// UTILITY MACROS AND FUNCTIONS
// ===========================

// Debug and logging macros
#define DEBUG_PRINT(x) if(DEBUG_ENABLED) Serial.print(x)
#define DEBUG_PRINTLN(x) if(DEBUG_ENABLED) Serial.println(x)
#define DEBUG_PRINTF(format, ...) if(DEBUG_ENABLED) Serial.printf(format, ##__VA_ARGS__)

// Voltage and ADC conversion macros
#define VOLTAGE_TO_ADC(voltage) ((voltage) * 4095 / ADC_REFERENCE_VOLTAGE)
#define ADC_TO_VOLTAGE(adc) ((adc) * ADC_REFERENCE_VOLTAGE / 4095)
#define PERCENTAGE(value, max_value) ((value) * 100 / (max_value))

// Time conversion macros
#define SECONDS_TO_MS(sec) ((sec) * 1000)
#define MINUTES_TO_MS(min) ((min) * 60 * 1000)
#define HOURS_TO_MS(hour) ((hour) * 60 * 60 * 1000)

// Memory and size macros
#define KB_TO_BYTES(kb) ((kb) * 1024)
#define MB_TO_BYTES(mb) ((mb) * 1024 * 1024)

// Configuration validation macros
#define VALIDATE_RANGE(value, min, max) ((value) >= (min) && (value) <= (max))
#define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

#endif // CONFIG_H