#include "board_abstraction.h"
#include <string.h>

// Static configuration storage
static board_config_t current_config;
static bool config_initialized = false;

// Board configurations for supported ESP32 camera boards
static const board_config_t board_configs[] = {
    // ESP32-CAM (AI-Thinker)
    {
        .board_type = BOARD_ESP32_CAM,
        .sensor_type = SENSOR_OV2640,
        .power_profile = POWER_PROFILE_NORMAL,
        .connectivity = {.wifi_enabled = true, .cellular_enabled = true, .satellite_enabled = true, .lora_enabled = true},
        .gpio = {
            .camera_sda = 21, .camera_scl = 22,
            .camera_d0 = 5, .camera_d1 = 18, .camera_d2 = 19, .camera_d3 = 21,
            .camera_d4 = 36, .camera_d5 = 39, .camera_d6 = 34, .camera_d7 = 35,
            .camera_xclk = 0, .camera_pclk = 22, .camera_vsync = 25, .camera_href = 23,
            .camera_pwdn = 32, .camera_reset = -1, .led_flash = 4, .sd_cs = -1, .pir_sensor = -1
        },
        .board_name = "ESP32-CAM (AI-Thinker)",
        .max_resolution_width = 1600, .max_resolution_height = 1200,
        .has_flash_led = true, .has_pir_sensor = false, .has_sd_card = false, .has_battery = false
    },
    
    // ESP32-S3-CAM
    {
        .board_type = BOARD_ESP32_S3_CAM,
        .sensor_type = SENSOR_OV2640,
        .power_profile = POWER_PROFILE_HIGH,
        .connectivity = {.wifi_enabled = true, .cellular_enabled = true, .satellite_enabled = true, .lora_enabled = true},
        .gpio = {
            .camera_sda = 4, .camera_scl = 5,
            .camera_d0 = 11, .camera_d1 = 9, .camera_d2 = 8, .camera_d3 = 10,
            .camera_d4 = 12, .camera_d5 = 18, .camera_d6 = 17, .camera_d7 = 16,
            .camera_xclk = 15, .camera_pclk = 13, .camera_vsync = 6, .camera_href = 7,
            .camera_pwdn = -1, .camera_reset = -1, .led_flash = 48, .sd_cs = 21, .pir_sensor = -1
        },
        .board_name = "ESP32-S3-CAM",
        .max_resolution_width = 1600, .max_resolution_height = 1200,
        .has_flash_led = true, .has_pir_sensor = false, .has_sd_card = true, .has_battery = false
    },
    
    // ESP-EYE (Espressif Official)
    {
        .board_type = BOARD_ESP_EYE,
        .sensor_type = SENSOR_OV2640,
        .power_profile = POWER_PROFILE_HIGH,
        .connectivity = {.wifi_enabled = true, .cellular_enabled = true, .satellite_enabled = true, .lora_enabled = true},
        .gpio = {
            .camera_sda = 18, .camera_scl = 23,
            .camera_d0 = 4, .camera_d1 = 5, .camera_d2 = 18, .camera_d3 = 19,
            .camera_d4 = 36, .camera_d5 = 39, .camera_d6 = 34, .camera_d7 = 35,
            .camera_xclk = 4, .camera_pclk = 25, .camera_vsync = 25, .camera_href = 26,
            .camera_pwdn = -1, .camera_reset = -1, .led_flash = 22, .sd_cs = 13, .pir_sensor = -1
        },
        .board_name = "ESP-EYE (Espressif)",
        .max_resolution_width = 1600, .max_resolution_height = 1200,
        .has_flash_led = true, .has_pir_sensor = false, .has_sd_card = true, .has_battery = false
    },
    
    // M5Stack Timer Camera
    {
        .board_type = BOARD_M5_TIMER_CAM,
        .sensor_type = SENSOR_OV3660,
        .power_profile = POWER_PROFILE_LOW,
        .connectivity = {.wifi_enabled = true, .cellular_enabled = true, .satellite_enabled = true, .lora_enabled = true},
        .gpio = {
            .camera_sda = 12, .camera_scl = 14,
            .camera_d0 = 17, .camera_d1 = 35, .camera_d2 = 34, .camera_d3 = 5,
            .camera_d4 = 39, .camera_d5 = 18, .camera_d6 = 36, .camera_d7 = 19,
            .camera_xclk = 27, .camera_pclk = 21, .camera_vsync = 22, .camera_href = 26,
            .camera_pwdn = -1, .camera_reset = 15, .led_flash = 2, .sd_cs = 4, .pir_sensor = 33
        },
        .board_name = "M5Stack Timer Camera",
        .max_resolution_width = 2048, .max_resolution_height = 1536,
        .has_flash_led = true, .has_pir_sensor = true, .has_sd_card = true, .has_battery = true
    },
    
    // TTGO T-Camera
    {
        .board_type = BOARD_TTGO_T_CAMERA,
        .sensor_type = SENSOR_OV2640,
        .power_profile = POWER_PROFILE_NORMAL,
        .connectivity = {.wifi_enabled = true, .cellular_enabled = true, .satellite_enabled = true, .lora_enabled = true},
        .gpio = {
            .camera_sda = 21, .camera_scl = 22,
            .camera_d0 = 34, .camera_d1 = 13, .camera_d2 = 14, .camera_d3 = 35,
            .camera_d4 = 39, .camera_d5 = 38, .camera_d6 = 37, .camera_d7 = 36,
            .camera_xclk = 32, .camera_pclk = 19, .camera_vsync = 27, .camera_href = 25,
            .camera_pwdn = 26, .camera_reset = -1, .led_flash = 4, .sd_cs = 0, .pir_sensor = 33
        },
        .board_name = "TTGO T-Camera",
        .max_resolution_width = 1600, .max_resolution_height = 1200,
        .has_flash_led = true, .has_pir_sensor = true, .has_sd_card = true, .has_battery = true
    },
    
    // XIAO ESP32S3 Sense
    {
        .board_type = BOARD_XIAO_ESP32S3_SENSE,
        .sensor_type = SENSOR_OV2640,
        .power_profile = POWER_PROFILE_LOW,
        .connectivity = {.wifi_enabled = true, .cellular_enabled = true, .satellite_enabled = false, .lora_enabled = true},
        .gpio = {
            .camera_sda = 5, .camera_scl = 6,
            .camera_d0 = 15, .camera_d1 = 17, .camera_d2 = 18, .camera_d3 = 16,
            .camera_d4 = 14, .camera_d5 = 12, .camera_d6 = 11, .camera_d7 = 48,
            .camera_xclk = 10, .camera_pclk = 13, .camera_vsync = 38, .camera_href = 47,
            .camera_pwdn = -1, .camera_reset = -1, .led_flash = -1, .sd_cs = 21, .pir_sensor = -1
        },
        .board_name = "XIAO ESP32S3 Sense",
        .max_resolution_width = 1600, .max_resolution_height = 1200,
        .has_flash_led = false, .has_pir_sensor = false, .has_sd_card = true, .has_battery = false
    }
};

static const size_t num_board_configs = sizeof(board_configs) / sizeof(board_configs[0]);

board_type_t board_detect(void) {
    // Basic board detection logic - in real implementation this would
    // check GPIO states, read chip ID, detect connected sensors, etc.
    // For now, return ESP32-CAM as default
    return BOARD_ESP32_CAM;
}

bool board_configure(board_type_t board_type) {
    // Find the configuration for the specified board type
    for (size_t i = 0; i < num_board_configs; i++) {
        if (board_configs[i].board_type == board_type) {
            memcpy(&current_config, &board_configs[i], sizeof(board_config_t));
            config_initialized = true;
            return true;
        }
    }
    return false;
}

const board_config_t* board_get_config(void) {
    if (!config_initialized) {
        // Auto-detect and configure if not already done
        board_type_t detected = board_detect();
        board_configure(detected);
    }
    return &current_config;
}

const char* board_get_name(board_type_t board_type) {
    for (size_t i = 0; i < num_board_configs; i++) {
        if (board_configs[i].board_type == board_type) {
            return board_configs[i].board_name;
        }
    }
    return "Unknown Board";
}

bool board_supports_feature(const char* feature) {
    if (!config_initialized) {
        board_get_config(); // Initialize if needed
    }
    
    if (strcmp(feature, "flash_led") == 0) {
        return current_config.has_flash_led;
    } else if (strcmp(feature, "pir_sensor") == 0) {
        return current_config.has_pir_sensor;
    } else if (strcmp(feature, "sd_card") == 0) {
        return current_config.has_sd_card;
    } else if (strcmp(feature, "battery") == 0) {
        return current_config.has_battery;
    } else if (strcmp(feature, "wifi") == 0) {
        return current_config.connectivity.wifi_enabled;
    } else if (strcmp(feature, "cellular") == 0) {
        return current_config.connectivity.cellular_enabled;
    } else if (strcmp(feature, "satellite") == 0) {
        return current_config.connectivity.satellite_enabled;
    } else if (strcmp(feature, "lora") == 0) {
        return current_config.connectivity.lora_enabled;
    }
    
    return false;
}