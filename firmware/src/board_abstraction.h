#ifndef BOARD_ABSTRACTION_H
#define BOARD_ABSTRACTION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Board type enumeration
typedef enum {
    BOARD_ESP32_CAM,
    BOARD_ESP32_S3_CAM,
    BOARD_ESP_EYE,
    BOARD_M5_TIMER_CAM,
    BOARD_TTGO_T_CAMERA,
    BOARD_XIAO_ESP32S3_SENSE,
    BOARD_CUSTOM,
    BOARD_UNKNOWN
} board_type_t;

// Camera sensor enumeration
typedef enum {
    SENSOR_OV2640,
    SENSOR_OV3660,
    SENSOR_OV5640,
    SENSOR_GC032A,
    SENSOR_OV7670,
    SENSOR_HM01B0,
    SENSOR_UNKNOWN
} camera_sensor_t;

// Power profile enumeration
typedef enum {
    POWER_PROFILE_LOW,      // Ultra-low power for battery operation
    POWER_PROFILE_NORMAL,   // Balanced power consumption
    POWER_PROFILE_HIGH      // High performance mode
} power_profile_t;

// Connectivity options
typedef struct {
    bool wifi_enabled;
    bool cellular_enabled;
    bool satellite_enabled;
    bool lora_enabled;
} connectivity_options_t;

// GPIO pin configuration
typedef struct {
    int8_t camera_sda;
    int8_t camera_scl;
    int8_t camera_d0;
    int8_t camera_d1;
    int8_t camera_d2;
    int8_t camera_d3;
    int8_t camera_d4;
    int8_t camera_d5;
    int8_t camera_d6;
    int8_t camera_d7;
    int8_t camera_xclk;
    int8_t camera_pclk;
    int8_t camera_vsync;
    int8_t camera_href;
    int8_t camera_pwdn;
    int8_t camera_reset;
    int8_t led_flash;
    int8_t sd_cs;
    int8_t pir_sensor;
} gpio_config_t;

// Board configuration structure
typedef struct {
    board_type_t board_type;
    camera_sensor_t sensor_type;
    power_profile_t power_profile;
    connectivity_options_t connectivity;
    gpio_config_t gpio;
    const char* board_name;
    uint32_t max_resolution_width;
    uint32_t max_resolution_height;
    bool has_flash_led;
    bool has_pir_sensor;
    bool has_sd_card;
    bool has_battery;
} board_config_t;

// Function prototypes
board_type_t board_detect(void);
bool board_configure(board_type_t board_type);
const board_config_t* board_get_config(void);
const char* board_get_name(board_type_t board_type);
bool board_supports_feature(const char* feature);

#ifdef __cplusplus
}
#endif

#endif // BOARD_ABSTRACTION_H