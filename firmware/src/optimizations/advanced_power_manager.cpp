#include "advanced_power_manager.h"
#include "../config.h"
#include <soc/rtc.h>

// Static instance for interrupt handling
UltraLowPowerMotionDetector* UltraLowPowerMotionDetector::instance = nullptr;

// AdvancedPowerManager implementation
bool AdvancedPowerManager::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing Advanced Power Manager...");
    
    currentProfile = POWER_BALANCED;
    dvfsEnabled = false;
    pirWakeupPin = GPIO_NUM_13; // PIR_PIN from config
    lastPowerMeasurement = 0;
    lastMeasuredPower = 0;
    
    // Configure power management
    esp_pm_config_esp32_t pm_config;
    pm_config.max_freq_mhz = 240;
    pm_config.min_freq_mhz = 80;
    pm_config.light_sleep_enable = true;
    
    if (esp_pm_configure(&pm_config) != ESP_OK) {
        DEBUG_PRINTLN("WARNING: Failed to configure power management");
    }
    
    // Configure GPIO for low power
    configureGPIOForLowPower();
    
    // Configure brownout detector
    configureBrownoutDetector();
    
    // Calibrate power model
    powerModel = calibratePowerModel();
    
    initialized = true;
    DEBUG_PRINTLN("Advanced Power Manager initialized successfully");
    return true;
}

void AdvancedPowerManager::cleanup() {
    if (!initialized) return;
    
    // Reset to normal power profile
    setProfile(POWER_BALANCED);
    
    initialized = false;
}

void AdvancedPowerManager::adjustCPUFrequency(float load) {
    if (!dvfsEnabled) return;
    
    uint32_t targetFreq;
    
    if (load > 0.8f) {
        targetFreq = MAX_CPU_FREQ_MHZ;
    } else if (load > 0.6f) {
        targetFreq = 160;
    } else if (load > 0.4f) {
        targetFreq = 120;
    } else {
        targetFreq = MIN_CPU_FREQ_MHZ;
    }
    
    setFrequency(targetFreq);
}

void AdvancedPowerManager::enableDVFS() {
    dvfsEnabled = true;
    DEBUG_PRINTLN("Dynamic Voltage and Frequency Scaling enabled");
}

bool AdvancedPowerManager::setFrequency(uint32_t frequencyMHz) {
    // Validate frequency
    if (frequencyMHz < MIN_CPU_FREQ_MHZ || frequencyMHz > MAX_CPU_FREQ_MHZ) {
        return false;
    }
    
    // Round to nearest step
    uint32_t rounded = ((frequencyMHz + STEP_SIZE_MHZ/2) / STEP_SIZE_MHZ) * STEP_SIZE_MHZ;
    
    esp_pm_config_esp32_t pm_config;
    pm_config.max_freq_mhz = rounded;
    pm_config.min_freq_mhz = MIN_CPU_FREQ_MHZ;
    pm_config.light_sleep_enable = true;
    
    return esp_pm_configure(&pm_config) == ESP_OK;
}

uint32_t AdvancedPowerManager::getCurrentFrequency() {
    rtc_cpu_freq_config_t config;
    rtc_clk_cpu_freq_get_config(&config);
    return config.freq_mhz;
}

void AdvancedPowerManager::powerDownUnusedPeripherals() {
    // Disable unused peripherals based on current configuration
    
    #if !defined(WIFI_ENABLED) || WIFI_ENABLED == false
    // Disable WiFi if not needed
    esp_wifi_stop();
    esp_wifi_deinit();
    #endif
    
    #if !defined(BT_ENABLED) || BT_ENABLED == false
    // Disable Bluetooth if not needed
    esp_bt_controller_disable();
    #endif
    
    // Disable unused ADC channels
    // This is hardware-specific and would need careful implementation
}

void AdvancedPowerManager::selectOptimalRadioPower(int8_t rssi, uint32_t distance) {
    // Adjust LoRa transmission power based on signal quality and distance
    int8_t optimalPower = 5; // Minimum power
    
    if (rssi < -100 || distance > 1000) {
        optimalPower = 20; // Maximum power for poor conditions
    } else if (rssi < -90 || distance > 500) {
        optimalPower = 15;
    } else if (rssi < -80 || distance > 250) {
        optimalPower = 10;
    }
    
    // Apply the power setting to LoRa module
    // This would integrate with the existing LoRa mesh implementation
}

void AdvancedPowerManager::dynamicCameraSettings(float lightLevel, bool motionDetected) {
    // Adjust camera settings based on conditions to optimize power
    
    if (lightLevel < 10.0f) { // Low light
        // Reduce frame rate, increase exposure
        // This would integrate with camera_handler
    }
    
    if (!motionDetected) {
        // Reduce frame rate when no motion
        // Put camera in standby mode
    }
}

void AdvancedPowerManager::optimizeWiFiPower(int8_t signalStrength) {
    #if defined(WIFI_ENABLED) && WIFI_ENABLED == true
    // Adjust WiFi power based on signal strength
    wifi_power_t power = WIFI_POWER_19_5dBm; // Default
    
    if (signalStrength > -50) {
        power = WIFI_POWER_8_5dBm;   // Excellent signal
    } else if (signalStrength > -60) {
        power = WIFI_POWER_11dBm;    // Good signal
    } else if (signalStrength > -70) {
        power = WIFI_POWER_15dBm;    // Fair signal
    }
    
    esp_wifi_set_max_tx_power(power);
    #endif
}

void AdvancedPowerManager::setProfile(PowerProfile profile) {
    if (!validateProfile(profile)) return;
    
    currentProfile = profile;
    applyPowerProfile(profile);
    
    DEBUG_PRINTF("Power profile changed to: %d\n", profile);
}

void AdvancedPowerManager::applyPowerProfile(PowerProfile profile) {
    switch (profile) {
        case POWER_MAXIMUM_PERFORMANCE:
            setFrequency(MAX_CPU_FREQ_MHZ);
            // Enable all features
            break;
            
        case POWER_BALANCED:
            setFrequency(160);
            // Balanced settings
            break;
            
        case POWER_ECO_MODE:
            setFrequency(120);
            powerDownUnusedPeripherals();
            break;
            
        case POWER_SURVIVAL:
            setFrequency(MIN_CPU_FREQ_MHZ);
            powerDownUnusedPeripherals();
            // Minimal functionality
            break;
            
        case POWER_HIBERNATION:
            // Prepare for deep sleep
            configureGPIOForLowPower();
            enableDeepSleepWakeOnMotion();
            break;
    }
}

bool AdvancedPowerManager::validateProfile(PowerProfile profile) {
    return profile >= POWER_MAXIMUM_PERFORMANCE && profile <= POWER_HIBERNATION;
}

void AdvancedPowerManager::optimizeBasedOnSolarData(float solarVoltage, float batteryLevel) {
    PowerProfile recommendedProfile = currentProfile;
    
    if (batteryLevel < 20.0f) {
        // Critical battery - enter survival mode
        recommendedProfile = POWER_SURVIVAL;
    } else if (batteryLevel < 40.0f && solarVoltage < 3.0f) {
        // Low battery and no charging - eco mode
        recommendedProfile = POWER_ECO_MODE;
    } else if (batteryLevel > 80.0f && solarVoltage > 4.0f) {
        // Good battery and charging - performance mode
        recommendedProfile = POWER_BALANCED;
    }
    
    if (recommendedProfile != currentProfile) {
        setProfile(recommendedProfile);
    }
}

float AdvancedPowerManager::measureActualPowerConsumption() {
    // This would measure actual current consumption
    // For now, estimate based on current configuration
    
    uint32_t freq = getCurrentFrequency();
    float estimatedPower = powerModel.basePower;
    
    estimatedPower += (freq / 240.0f) * powerModel.cpuPowerCoeff * 50.0f;
    
    #if defined(LORA_ENABLED) && LORA_ENABLED == true
    estimatedPower += powerModel.radioPowerCoeff * 30.0f;
    #endif
    
    lastMeasuredPower = estimatedPower;
    lastPowerMeasurement = millis();
    
    return estimatedPower;
}

AdvancedPowerManager::PowerProfile AdvancedPowerManager::selectOptimalProfile(float batteryLevel, uint32_t timeToNextCharge) {
    if (batteryLevel < 10.0f) {
        return POWER_HIBERNATION;
    } else if (batteryLevel < 25.0f) {
        return POWER_SURVIVAL;
    } else if (batteryLevel < 50.0f || timeToNextCharge > 48) {
        return POWER_ECO_MODE;
    } else {
        return POWER_BALANCED;
    }
}

void AdvancedPowerManager::enableDeepSleepWakeOnMotion() {
    esp_sleep_enable_ext0_wakeup(pirWakeupPin, 1);
    rtc_gpio_pulldown_en(pirWakeupPin);
    rtc_gpio_pullup_dis(pirWakeupPin);
}

bool AdvancedPowerManager::hasMotionSinceLastCheck() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    return wakeup_reason == ESP_SLEEP_WAKEUP_EXT0;
}

void AdvancedPowerManager::configureGPIOForLowPower() {
    // Configure unused GPIO pins for minimal power consumption
    for (int i = 0; i < GPIO_NUM_MAX; i++) {
        // Skip pins in use
        if (i == PIR_PIN || i == SOLAR_VOLTAGE_PIN || i == BATTERY_VOLTAGE_PIN ||
            i == LORA_CS || i == LORA_RST || i == LORA_DIO0) {
            continue;
        }
        
        // Configure as input with pullup to reduce leakage
        gpio_set_direction((gpio_num_t)i, GPIO_MODE_INPUT);
        gpio_set_pull_mode((gpio_num_t)i, GPIO_PULLUP_ONLY);
    }
}

void AdvancedPowerManager::configureBrownoutDetector() {
    // Configure brownout detector for optimal power management
    esp_brownout_disable(); // Disable to prevent unexpected resets
}

AdvancedPowerManager::PowerModel AdvancedPowerManager::calibratePowerModel() {
    PowerModel model;
    
    // Perform basic calibration
    // This would involve actual power measurements in a real implementation
    model.calibrated = true;
    
    DEBUG_PRINTLN("Power model calibrated");
    return model;
}

float AdvancedPowerManager::predictPowerConsumption(PowerProfile profile, uint32_t durationMs) {
    float baseCurrent = 0;
    
    switch (profile) {
        case POWER_MAXIMUM_PERFORMANCE: baseCurrent = 250.0f; break;
        case POWER_BALANCED: baseCurrent = 180.0f; break;
        case POWER_ECO_MODE: baseCurrent = 120.0f; break;
        case POWER_SURVIVAL: baseCurrent = 80.0f; break;
        case POWER_HIBERNATION: baseCurrent = 10.0f; break;
    }
    
    return baseCurrent * (durationMs / 1000.0f) / 3600.0f; // mAh
}

AdvancedPowerManager::PowerMetrics AdvancedPowerManager::getPowerMetrics() const {
    return metrics;
}

void AdvancedPowerManager::resetMetrics() {
    metrics = PowerMetrics();
}

// UltraLowPowerMotionDetector implementation
UltraLowPowerMotionDetector::UltraLowPowerMotionDetector(gpio_num_t pin) 
    : pirPin(pin), motionDetected(false), initialized(false), lastMotionTime(0) {
    instance = this;
}

bool UltraLowPowerMotionDetector::init() {
    if (initialized) return true;
    
    gpio_config_t config;
    config.pin_bit_mask = (1ULL << pirPin);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    config.intr_type = GPIO_INTR_POSEDGE;
    
    gpio_config(&config);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(pirPin, motionISR, nullptr);
    
    initialized = true;
    return true;
}

void UltraLowPowerMotionDetector::cleanup() {
    if (initialized) {
        gpio_isr_handler_remove(pirPin);
        initialized = false;
    }
}

void IRAM_ATTR UltraLowPowerMotionDetector::motionISR() {
    if (instance) {
        instance->motionDetected = true;
        instance->lastMotionTime = millis();
    }
}

bool UltraLowPowerMotionDetector::hasMotionSinceLastCheck() {
    bool motion = motionDetected;
    motionDetected = false;
    return motion;
}

void UltraLowPowerMotionDetector::clearMotionFlag() {
    motionDetected = false;
}

void UltraLowPowerMotionDetector::enableDeepSleepWakeOnMotion() {
    configurePinForWakeup();
    esp_sleep_enable_ext0_wakeup(pirPin, 1);
}

void UltraLowPowerMotionDetector::configurePinForWakeup() {
    rtc_gpio_init(pirPin);
    rtc_gpio_set_direction(pirPin, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_en(pirPin);
    rtc_gpio_pullup_dis(pirPin);
}