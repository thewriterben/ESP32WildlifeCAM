/**
 * @file board_profiles.h
 * @brief Board-specific configuration profiles
 */

#ifndef BOARD_PROFILES_H
#define BOARD_PROFILES_H

#include "../hal/camera_board.h"

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

// ESP32-S3-CAM Profile (placeholder)
const GPIOMap ESP32_S3_CAM_GPIO_MAP = {
    .pwdn_pin = -1,
    .reset_pin = -1,
    .xclk_pin = 40,
    .siod_pin = 17,
    .sioc_pin = 18,
    .y9_pin = 39,
    .y8_pin = 41,
    .y7_pin = 42,
    .y6_pin = 12,
    .y5_pin = 3,
    .y4_pin = 14,
    .y3_pin = 47,
    .y2_pin = 13,
    .vsync_pin = 21,
    .href_pin = 38,
    .pclk_pin = 11,
    .led_pin = 48,
    .flash_pin = 48
};

const PowerProfile ESP32_S3_CAM_POWER_PROFILE = {
    .sleep_current_ua = 7000,
    .active_current_ma = 100,
    .camera_current_ma = 180,
    .has_external_power = false,
    .supports_deep_sleep = true,
    .min_voltage = 3.0,
    .max_voltage = 3.6
};

const CameraConfig ESP32_S3_CAM_CAMERA_CONFIG = {
    .max_framesize = FRAMESIZE_QXGA,
    .pixel_format = PIXFORMAT_JPEG,
    .jpeg_quality = 10,
    .fb_count = 3,
    .xclk_freq_hz = 20000000,
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

#endif // BOARD_PROFILES_H