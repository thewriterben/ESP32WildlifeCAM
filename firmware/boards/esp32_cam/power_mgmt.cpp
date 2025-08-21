#include "power_mgmt.h"
#include "../board_abstraction.h"
#include <driver/gpio.h>
#include <esp_sleep.h>
#include <esp_pm.h>

// Power management for ESP32-CAM
// Optimized for solar-powered wildlife monitoring

static bool power_mgmt_initialized = false;
static power_profile_t current_profile = POWER_PROFILE_NORMAL;

bool esp32_cam_power_init(power_profile_t profile) {
    if (power_mgmt_initialized) {
        return true;
    }
    
    current_profile = profile;
    
    // Configure unused pins as input with pullup to reduce power consumption
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << GPIO_NUM_1) | (1ULL << GPIO_NUM_3),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf);
    
    // Set up power management based on profile
    esp_pm_config_esp32_t pm_config;
    
    switch (profile) {
        case POWER_PROFILE_LOW:
            // Ultra-low power for battery operation
            pm_config.max_freq_mhz = 80;   // Reduce CPU frequency
            pm_config.min_freq_mhz = 10;   // Lower minimum frequency
            pm_config.light_sleep_enable = true;
            break;
            
        case POWER_PROFILE_NORMAL:
            // Balanced power consumption
            pm_config.max_freq_mhz = 160;  // Normal CPU frequency
            pm_config.min_freq_mhz = 80;   // Moderate minimum frequency
            pm_config.light_sleep_enable = true;
            break;
            
        case POWER_PROFILE_HIGH:
            // High performance mode
            pm_config.max_freq_mhz = 240;  // Maximum CPU frequency
            pm_config.min_freq_mhz = 160;  // High minimum frequency
            pm_config.light_sleep_enable = false;
            break;
    }
    
    esp_err_t ret = esp_pm_configure(&pm_config);
    if (ret != ESP_OK) {
        return false;
    }
    
    power_mgmt_initialized = true;
    return true;
}

void esp32_cam_enter_deep_sleep(uint64_t sleep_duration_us) {
    // Configure wakeup timer
    esp_sleep_enable_timer_wakeup(sleep_duration_us);
    
    // Optionally configure external wakeup (for PIR sensor if connected)
    // esp_sleep_enable_ext0_wakeup(PIR_GPIO, 1);
    
    // Enter deep sleep
    esp_deep_sleep_start();
}

void esp32_cam_enter_light_sleep(uint64_t sleep_duration_us) {
    // Configure wakeup timer for light sleep
    esp_sleep_enable_timer_wakeup(sleep_duration_us);
    
    // Enter light sleep (maintains RAM, faster wake-up)
    esp_light_sleep_start();
}

bool esp32_cam_set_cpu_frequency(uint32_t freq_mhz) {
    esp_pm_config_esp32_t pm_config;
    
    // Get current configuration
    esp_pm_get_configuration(&pm_config);
    
    // Update frequency
    pm_config.max_freq_mhz = freq_mhz;
    pm_config.min_freq_mhz = freq_mhz / 2; // Set minimum to half of maximum
    
    return esp_pm_configure(&pm_config) == ESP_OK;
}

void esp32_cam_disable_unused_peripherals(void) {
    // Disable unused peripherals to save power
    // This is board-specific optimization for ESP32-CAM
    
    // Disable Bluetooth if not needed
    esp_bt_controller_disable();
    
    // Note: We keep WiFi enabled as it's primary communication method
    // for wildlife monitoring data transmission
}

uint32_t esp32_cam_get_battery_voltage_mv(void) {
    // ESP32-CAM doesn't have built-in battery monitoring
    // Return 0 to indicate not available
    return 0;
}

float esp32_cam_get_power_consumption_mw(void) {
    // Estimated power consumption based on current profile
    switch (current_profile) {
        case POWER_PROFILE_LOW:
            return 150.0f;    // ~150mW in low power mode
        case POWER_PROFILE_NORMAL:
            return 300.0f;    // ~300mW in normal mode
        case POWER_PROFILE_HIGH:
            return 500.0f;    // ~500mW in high performance mode
        default:
            return 300.0f;
    }
}

bool esp32_cam_is_external_power_available(void) {
    // ESP32-CAM typically powered via USB or external 5V
    // For solar applications, check if sufficient voltage is available
    // This would require additional hardware for voltage monitoring
    return true; // Assume external power is available
}

void esp32_cam_optimize_for_solar(void) {
    // Specific optimizations for solar-powered operation
    
    // Set to low power profile
    esp32_cam_power_init(POWER_PROFILE_LOW);
    
    // Disable unused peripherals
    esp32_cam_disable_unused_peripherals();
    
    // Configure for deep sleep between captures
    // This would be called from main application logic
}

power_profile_t esp32_cam_get_current_profile(void) {
    return current_profile;
}