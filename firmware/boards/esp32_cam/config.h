#ifndef ESP32_CAM_CONFIG_H
#define ESP32_CAM_CONFIG_H

#include "pinout.h"

// ESP32-CAM Configuration Settings
// Optimized for wildlife monitoring applications

// Camera Configuration
#define DEFAULT_FRAMESIZE         FRAMESIZE_UXGA  // 1600x1200 for wildlife detail
#define DEFAULT_JPEG_QUALITY      10              // Good quality vs file size
#define DEFAULT_BRIGHTNESS        0               // Neutral brightness
#define DEFAULT_CONTRAST          0               // Neutral contrast
#define DEFAULT_SATURATION        0               // Neutral saturation

// Power Management
#define SLEEP_MODE_ENABLED        true
#define DEEP_SLEEP_DURATION_US    30000000        // 30 seconds between captures
#define LOW_POWER_MODE            true            // Enable power optimizations
#define FLASH_LED_ENABLED         true            // Use flash for night captures

// Motion Detection Settings
#define PIR_ENABLED               false           // No built-in PIR sensor
#define MOTION_THRESHOLD          50              // Software motion detection threshold
#define MOTION_DETECTION_ENABLED  true            // Enable software motion detection

// Storage Configuration
#define SD_CARD_ENABLED           false           // No built-in SD card
#define WIFI_ENABLED              true            // Enable WiFi for data transmission
#define BUFFER_SIZE               32768           // Image buffer size

// Wildlife Monitoring Features
#define TIMESTAMP_ENABLED         true            // Add timestamp to images
#define GEOLOCATION_ENABLED       false           // No GPS on basic ESP32-CAM
#define TEMPERATURE_SENSOR        false           // No built-in temperature sensor
#define BATTERY_MONITORING        false           // No built-in battery monitoring

// Connectivity Options
#define WIFI_TIMEOUT_MS           10000           // WiFi connection timeout
#define MAX_RETRY_ATTEMPTS        3               // Max connection retries
#define DATA_TRANSMISSION_MODE    "BATCH"         // BATCH or REALTIME

// Image Processing
#define AUTO_EXPOSURE             true
#define AUTO_WHITE_BALANCE        true
#define NIGHT_MODE_ENABLED        true            // Use flash in low light
#define IMAGE_FLIP_H              false           // Horizontal flip
#define IMAGE_FLIP_V              false           // Vertical flip

// Wildlife Specific Settings
#define CAPTURE_INTERVAL_MS       60000           // 1 minute between regular captures
#define MOTION_CAPTURE_ENABLED    true            // Capture on motion detection
#define CONSECUTIVE_SHOTS         3               // Number of shots per trigger
#define SHOT_INTERVAL_MS          1000            // Time between consecutive shots

// Memory Management
#define FRAME_BUFFER_COUNT        2               // Double buffering
#define MAX_IMAGE_SIZE            1048576         // 1MB max image size
#define COMPRESSION_LEVEL         80              // JPEG compression level

#endif // ESP32_CAM_CONFIG_H