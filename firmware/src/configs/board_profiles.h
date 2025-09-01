/**
 * @file board_profiles.h
 * @brief Board-specific configuration profiles
 */

#ifndef BOARD_PROFILES_H
#define BOARD_PROFILES_H

#include "../hal/camera_board.h"

// Default display profile for boards without built-in display
const DisplayProfile NO_DISPLAY_PROFILE = {
    .has_display = false,
    .display_type = 0,
    .display_sda_pin = -1,
    .display_scl_pin = -1,
    .display_cs_pin = -1,
    .display_dc_pin = -1,
    .display_rst_pin = -1,
    .display_bl_pin = -1,
    .display_i2c_addr = 0,
    .display_i2c_freq = 0,
    .display_spi_freq = 0,
    .display_width = 0,
    .display_height = 0
};

// AI-Thinker ESP32-CAM Profile
const GPIOMap AI_THINKER_GPIO_MAP = {
    .pwdn_pin = 32,
    .reset_pin = -1,
    .xclk_pin = 0,
    .siod_pin = 26,
    .sioc_pin = 27,
    .y9_pin = 35,
    .y8_pin = 34,
    .y7_pin = 39,
    .y6_pin = 36,
    .y5_pin = 21,
    .y4_pin = 19,
    .y3_pin = 18,
    .y2_pin = 5,
    .vsync_pin = 25,
    .href_pin = 23,
    .pclk_pin = 22,
    .led_pin = 4,
    .flash_pin = 4
};

const PowerProfile AI_THINKER_POWER_PROFILE = {
    .sleep_current_ua = 10000,
    .active_current_ma = 120,
    .camera_current_ma = 200,
    .has_external_power = false,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 3.6
};

const CameraConfig AI_THINKER_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_UXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 12,
    .fb_count = 2,
    .xclk_freq_hz = 20000000,
    .psram_required = false,
    .has_flash = true
};

// ESP32-S3-CAM Profile - Optimized pin mapping for ESP32-S3-CAM boards
const GPIOMap ESP32_S3_CAM_GPIO_MAP = {
    .pwdn_pin = 38,         // Power down pin
    .reset_pin = -1,        // Reset pin (often not used)
    .xclk_pin = 15,         // Clock signal
    .siod_pin = 4,          // I2C data (SDA)
    .sioc_pin = 5,          // I2C clock (SCL)
    .y9_pin = 16,           // Data pin 7 (MSB)
    .y8_pin = 17,           // Data pin 6
    .y7_pin = 18,           // Data pin 5
    .y6_pin = 12,           // Data pin 4
    .y5_pin = 10,           // Data pin 3
    .y4_pin = 8,            // Data pin 2
    .y3_pin = 9,            // Data pin 1
    .y2_pin = 11,           // Data pin 0 (LSB)
    .vsync_pin = 6,         // Vertical sync
    .href_pin = 7,          // Horizontal reference
    .pclk_pin = 13,         // Pixel clock
    .led_pin = 21,          // Built-in LED
    .flash_pin = 21         // Flash LED (same as built-in LED)
};

const PowerProfile ESP32_S3_CAM_POWER_PROFILE = {
    .sleep_current_ua = 5000,       // 5mA in light sleep (improved vs ESP32)
    .active_current_ma = 160,       // 160mA active (higher performance)
    .camera_current_ma = 250,       // 250mA with camera (enhanced sensors)
    .has_external_power = false,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 3.6
};

const CameraConfig ESP32_S3_CAM_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_QXGA,    // 2592x1944 - supports up to 5MP sensors
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 8,                   // Higher quality for ESP32-S3's processing power
    .fb_count = 3,                      // More frame buffers for smoother operation
    .xclk_freq_hz = 20000000,           // 20MHz clock
    .psram_required = true,             // PSRAM recommended for ESP32-S3-CAM
    .has_flash = true
};
    .psram_required = true,
    .has_flash = true
};

// ESP-EYE Profile (placeholder)
const GPIOMap ESP_EYE_GPIO_MAP = {
    .pwdn_pin = -1,
    .reset_pin = -1,
    .xclk_pin = 4,
    .siod_pin = 18,
    .sioc_pin = 23,
    .y9_pin = 36,
    .y8_pin = 37,
    .y7_pin = 38,
    .y6_pin = 39,
    .y5_pin = 35,
    .y4_pin = 14,
    .y3_pin = 13,
    .y2_pin = 34,
    .vsync_pin = 5,
    .href_pin = 27,
    .pclk_pin = 25,
    .led_pin = 21,
    .flash_pin = 21
};

const PowerProfile ESP_EYE_POWER_PROFILE = {
    .sleep_current_ua = 8000,
    .active_current_ma = 110,
    .camera_current_ma = 190,
    .has_external_power = true,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 5.0
};

const CameraConfig ESP_EYE_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_UXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 10,
    .fb_count = 2,
    .xclk_freq_hz = 20000000,
    .psram_required = true,
    .has_flash = true
};

// M5Stack Timer Camera Profile (placeholder)
const GPIOMap M5STACK_TIMER_CAM_GPIO_MAP = {
    .pwdn_pin = -1,
    .reset_pin = 15,
    .xclk_pin = 27,
    .siod_pin = 25,
    .sioc_pin = 23,
    .y9_pin = 19,
    .y8_pin = 36,
    .y7_pin = 18,
    .y6_pin = 39,
    .y5_pin = 5,
    .y4_pin = 34,
    .y3_pin = 35,
    .y2_pin = 32,
    .vsync_pin = 22,
    .href_pin = 26,
    .pclk_pin = 21,
    .led_pin = 2,
    .flash_pin = 14
};

const PowerProfile M5STACK_TIMER_CAM_POWER_PROFILE = {
    .sleep_current_ua = 5000,
    .active_current_ma = 90,
    .camera_current_ma = 150,
    .has_external_power = true,
    .supports_deep_sleep = true,
    .min_voltage = 3.7,
    .max_voltage = 4.2
};

const CameraConfig M5STACK_TIMER_CAM_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_UXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 12,
    .fb_count = 1,
    .xclk_freq_hz = 20000000,
    .psram_required = false,
    .has_flash = true
};

// TTGO T-Camera Profile
const GPIOMap TTGO_T_CAMERA_GPIO_MAP = {
    .pwdn_pin = 26,
    .reset_pin = -1,
    .xclk_pin = 32,
    .siod_pin = 13,
    .sioc_pin = 12,
    .y9_pin = 39,
    .y8_pin = 36,
    .y7_pin = 23,
    .y6_pin = 18,
    .y5_pin = 15,
    .y4_pin = 4,
    .y3_pin = 14,
    .y2_pin = 5,
    .vsync_pin = 27,
    .href_pin = 25,
    .pclk_pin = 19,
    .led_pin = 2,
    .flash_pin = 4
};

const PowerProfile TTGO_T_CAMERA_POWER_PROFILE = {
    .sleep_current_ua = 6000,
    .active_current_ma = 95,
    .camera_current_ma = 160,
    .has_external_power = false,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 4.2
};

const CameraConfig TTGO_T_CAMERA_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_UXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 12,
    .fb_count = 2,
    .xclk_freq_hz = 20000000,
    .psram_required = true,
    .has_flash = true
};

const DisplayProfile TTGO_T_CAMERA_DISPLAY_PROFILE = {
    .has_display = true,
    .display_type = 1,              // SSD1306 OLED
    .display_sda_pin = 21,
    .display_scl_pin = 22,
    .display_cs_pin = -1,
    .display_dc_pin = -1,
    .display_rst_pin = 16,
    .display_bl_pin = -1,
    .display_i2c_addr = 0x3C,
    .display_i2c_freq = 400000,
    .display_spi_freq = 0,
    .display_width = 128,
    .display_height = 64
};

// XIAO ESP32S3 Sense Profile  
const GPIOMap XIAO_ESP32S3_SENSE_GPIO_MAP = {
    .pwdn_pin = -1,
    .reset_pin = -1,
    .xclk_pin = 10,
    .siod_pin = 40,
    .sioc_pin = 39,
    .y9_pin = 48,
    .y8_pin = 11,
    .y7_pin = 12,
    .y6_pin = 14,
    .y5_pin = 16,
    .y4_pin = 18,
    .y3_pin = 17,
    .y2_pin = 15,
    .vsync_pin = 38,
    .href_pin = 47,
    .pclk_pin = 13,
    .led_pin = -1,
    .flash_pin = -1
};

const PowerProfile XIAO_ESP32S3_SENSE_POWER_PROFILE = {
    .sleep_current_ua = 4000,
    .active_current_ma = 80,
    .camera_current_ma = 140,
    .has_external_power = false,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 3.6
};

const CameraConfig XIAO_ESP32S3_SENSE_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_QXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 10,
    .fb_count = 3,
    .xclk_freq_hz = 20000000,
    .psram_required = true,
    .has_flash = false
};

// FireBeetle ESP32-CAM Profile
const GPIOMap FIREBEETLE_ESP32_CAM_GPIO_MAP = {
    .pwdn_pin = 32,
    .reset_pin = -1,
    .xclk_pin = 0,
    .siod_pin = 26,
    .sioc_pin = 27,
    .y9_pin = 35,
    .y8_pin = 34,
    .y7_pin = 39,
    .y6_pin = 36,
    .y5_pin = 21,
    .y4_pin = 19,
    .y3_pin = 18,
    .y2_pin = 5,
    .vsync_pin = 25,
    .href_pin = 23,
    .pclk_pin = 22,
    .led_pin = 4,
    .flash_pin = 4
};

const PowerProfile FIREBEETLE_ESP32_CAM_POWER_PROFILE = {
    .sleep_current_ua = 3000,
    .active_current_ma = 70,
    .camera_current_ma = 130,
    .has_external_power = true,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 4.2
};

const CameraConfig FIREBEETLE_ESP32_CAM_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_UXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 12,
    .fb_count = 2,
    .xclk_freq_hz = 20000000,
    .psram_required = false,
    .has_flash = true
};

// ESP32-S3-EYE Profile
const GPIOMap ESP32_S3_EYE_GPIO_MAP = {
    .pwdn_pin = -1,
    .reset_pin = -1,
    .xclk_pin = 15,
    .siod_pin = 4,
    .sioc_pin = 5,
    .y9_pin = 16,
    .y8_pin = 17,
    .y7_pin = 18,
    .y6_pin = 12,
    .y5_pin = 10,
    .y4_pin = 8,
    .y3_pin = 9,
    .y2_pin = 11,
    .vsync_pin = 6,
    .href_pin = 7,
    .pclk_pin = 13,
    .led_pin = 48,
    .flash_pin = 48
};

const PowerProfile ESP32_S3_EYE_POWER_PROFILE = {
    .sleep_current_ua = 5000,
    .active_current_ma = 100,
    .camera_current_ma = 170,
    .has_external_power = true,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 5.0
};

const CameraConfig ESP32_S3_EYE_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_QXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 10,
    .fb_count = 3,
    .xclk_freq_hz = 20000000,
    .psram_required = true,
    .has_flash = true
};

// Freenove ESP32-WROVER-CAM Profile
const GPIOMap FREENOVE_ESP32_WROVER_CAM_GPIO_MAP = {
    .pwdn_pin = 32,
    .reset_pin = -1,
    .xclk_pin = 0,
    .siod_pin = 26,
    .sioc_pin = 27,
    .y9_pin = 35,
    .y8_pin = 34,
    .y7_pin = 39,
    .y6_pin = 36,
    .y5_pin = 21,
    .y4_pin = 19,
    .y3_pin = 18,
    .y2_pin = 5,
    .vsync_pin = 25,
    .href_pin = 23,
    .pclk_pin = 22,
    .led_pin = 4,
    .flash_pin = 4
};

const PowerProfile FREENOVE_ESP32_WROVER_CAM_POWER_PROFILE = {
    .sleep_current_ua = 8000,
    .active_current_ma = 110,
    .camera_current_ma = 190,
    .has_external_power = false,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 3.6
};

const CameraConfig FREENOVE_ESP32_WROVER_CAM_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_UXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 12,
    .fb_count = 2,
    .xclk_freq_hz = 20000000,
    .psram_required = true,
    .has_flash = true
};

// M5Stack ESP32CAM Profile
const GPIOMap M5STACK_ESP32CAM_GPIO_MAP = {
    .pwdn_pin = -1,
    .reset_pin = 15,
    .xclk_pin = 27,
    .siod_pin = 25,
    .sioc_pin = 23,
    .y9_pin = 19,
    .y8_pin = 36,
    .y7_pin = 18,
    .y6_pin = 39,
    .y5_pin = 5,
    .y4_pin = 34,
    .y3_pin = 35,
    .y2_pin = 32,
    .vsync_pin = 22,
    .href_pin = 26,
    .pclk_pin = 21,
    .led_pin = 2,
    .flash_pin = 14
};

const PowerProfile M5STACK_ESP32CAM_POWER_PROFILE = {
    .sleep_current_ua = 6000,
    .active_current_ma = 95,
    .camera_current_ma = 160,
    .has_external_power = true,
    .supports_deep_sleep = true,
    .min_voltage = 3.7,
    .max_voltage = 4.2
};

const CameraConfig M5STACK_ESP32CAM_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_UXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 12,
    .fb_count = 2,
    .xclk_freq_hz = 20000000,
    .psram_required = false,
    .has_flash = true
};

// TTGO T-Journal Profile
const GPIOMap TTGO_T_JOURNAL_GPIO_MAP = {
    .pwdn_pin = 26,
    .reset_pin = -1,
    .xclk_pin = 32,
    .siod_pin = 13,
    .sioc_pin = 12,
    .y9_pin = 39,
    .y8_pin = 36,
    .y7_pin = 23,
    .y6_pin = 18,
    .y5_pin = 15,
    .y4_pin = 4,
    .y3_pin = 14,
    .y2_pin = 5,
    .vsync_pin = 27,
    .href_pin = 25,
    .pclk_pin = 19,
    .led_pin = 2,
    .flash_pin = 4
};

const PowerProfile TTGO_T_JOURNAL_POWER_PROFILE = {
    .sleep_current_ua = 8000,
    .active_current_ma = 120,
    .camera_current_ma = 180,
    .has_external_power = false,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 4.2
};

const CameraConfig TTGO_T_JOURNAL_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_UXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 12,
    .fb_count = 2,
    .xclk_freq_hz = 20000000,
    .psram_required = true,
    .has_flash = true
};

// LilyGO T-Camera Plus Profile
const GPIOMap LILYGO_T_CAMERA_PLUS_GPIO_MAP = {
    .pwdn_pin = 26,
    .reset_pin = -1,
    .xclk_pin = 32,
    .siod_pin = 13,
    .sioc_pin = 12,
    .y9_pin = 39,
    .y8_pin = 36,
    .y7_pin = 23,
    .y6_pin = 18,
    .y5_pin = 15,
    .y4_pin = 4,
    .y3_pin = 14,
    .y2_pin = 5,
    .vsync_pin = 27,
    .href_pin = 25,
    .pclk_pin = 19,
    .led_pin = 2,
    .flash_pin = 4
};

const PowerProfile LILYGO_T_CAMERA_PLUS_POWER_PROFILE = {
    .sleep_current_ua = 5000,
    .active_current_ma = 90,
    .camera_current_ma = 150,
    .has_external_power = false,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 4.2
};

const CameraConfig LILYGO_T_CAMERA_PLUS_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_UXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 10,
    .fb_count = 3,
    .xclk_freq_hz = 20000000,
    .psram_required = true,
    .has_flash = true
};

const DisplayProfile LILYGO_T_CAMERA_PLUS_DISPLAY_PROFILE = {
    .has_display = true,
    .display_type = 2,              // ST7789 TFT
    .display_sda_pin = -1,
    .display_scl_pin = -1,
    .display_cs_pin = 12,
    .display_dc_pin = 13,
    .display_rst_pin = 14,
    .display_bl_pin = 15,
    .display_i2c_addr = 0,
    .display_i2c_freq = 0,
    .display_spi_freq = 27000000,
    .display_width = 240,
    .display_height = 240
};

// Espressif ESP32-CAM-MB Profile
const GPIOMap ESPRESSIF_ESP32_CAM_MB_GPIO_MAP = {
    .pwdn_pin = 32,
    .reset_pin = -1,
    .xclk_pin = 0,
    .siod_pin = 26,
    .sioc_pin = 27,
    .y9_pin = 35,
    .y8_pin = 34,
    .y7_pin = 39,
    .y6_pin = 36,
    .y5_pin = 21,
    .y4_pin = 19,
    .y3_pin = 18,
    .y2_pin = 5,
    .vsync_pin = 25,
    .href_pin = 23,
    .pclk_pin = 22,
    .led_pin = 4,
    .flash_pin = 4
};

const PowerProfile ESPRESSIF_ESP32_CAM_MB_POWER_PROFILE = {
    .sleep_current_ua = 9000,
    .active_current_ma = 115,
    .camera_current_ma = 185,
    .has_external_power = true,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 5.0
};

const CameraConfig ESPRESSIF_ESP32_CAM_MB_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_UXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 12,
    .fb_count = 2,
    .xclk_freq_hz = 20000000,
    .psram_required = false,
    .has_flash = true
};

// M5Stack UnitCAM Profile
const GPIOMap M5STACK_UNITCAM_GPIO_MAP = {
    .pwdn_pin = -1,
    .reset_pin = 15,
    .xclk_pin = 27,
    .siod_pin = 25,
    .sioc_pin = 23,
    .y9_pin = 19,
    .y8_pin = 36,
    .y7_pin = 18,
    .y6_pin = 39,
    .y5_pin = 5,
    .y4_pin = 34,
    .y3_pin = 35,
    .y2_pin = 32,
    .vsync_pin = 22,
    .href_pin = 26,
    .pclk_pin = 21,
    .led_pin = 2,
    .flash_pin = 14
};

const PowerProfile M5STACK_UNITCAM_POWER_PROFILE = {
    .sleep_current_ua = 3000,
    .active_current_ma = 70,
    .camera_current_ma = 120,
    .has_external_power = false,
    .supports_deep_sleep = true,
    .min_voltage = 3.7,
    .max_voltage = 4.2
};

const CameraConfig M5STACK_UNITCAM_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_SVGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 12,
    .fb_count = 1,
    .xclk_freq_hz = 20000000,
    .psram_required = false,
    .has_flash = true
};

// TTGO T-Camera V1.7 Profile
const GPIOMap TTGO_T_CAMERA_V17_GPIO_MAP = {
    .pwdn_pin = 26,
    .reset_pin = -1,
    .xclk_pin = 32,
    .siod_pin = 13,
    .sioc_pin = 12,
    .y9_pin = 39,
    .y8_pin = 36,
    .y7_pin = 23,
    .y6_pin = 18,
    .y5_pin = 15,
    .y4_pin = 4,
    .y3_pin = 14,
    .y2_pin = 5,
    .vsync_pin = 27,
    .href_pin = 25,
    .pclk_pin = 19,
    .led_pin = 2,
    .flash_pin = 4
};

const PowerProfile TTGO_T_CAMERA_V17_POWER_PROFILE = {
    .sleep_current_ua = 4000,
    .active_current_ma = 85,
    .camera_current_ma = 145,
    .has_external_power = false,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 4.2
};

const CameraConfig TTGO_T_CAMERA_V17_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_UXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 10,
    .fb_count = 3,
    .xclk_freq_hz = 20000000,
    .psram_required = true,
    .has_flash = true
};

#endif // BOARD_PROFILES_H