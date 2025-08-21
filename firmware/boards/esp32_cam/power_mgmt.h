#ifndef ESP32_CAM_POWER_MGMT_H
#define ESP32_CAM_POWER_MGMT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Power profile enumeration
typedef enum {
    POWER_PROFILE_LOW,      // Ultra-low power for battery operation
    POWER_PROFILE_NORMAL,   // Balanced power consumption  
    POWER_PROFILE_HIGH      // High performance mode
} power_profile_t;

// Power management functions for ESP32-CAM
bool esp32_cam_power_init(power_profile_t profile);
void esp32_cam_enter_deep_sleep(uint64_t sleep_duration_us);
void esp32_cam_enter_light_sleep(uint64_t sleep_duration_us);
bool esp32_cam_set_cpu_frequency(uint32_t freq_mhz);
void esp32_cam_disable_unused_peripherals(void);
uint32_t esp32_cam_get_battery_voltage_mv(void);
float esp32_cam_get_power_consumption_mw(void);
bool esp32_cam_is_external_power_available(void);
void esp32_cam_optimize_for_solar(void);
power_profile_t esp32_cam_get_current_profile(void);

#ifdef __cplusplus
}
#endif

#endif // ESP32_CAM_POWER_MGMT_H