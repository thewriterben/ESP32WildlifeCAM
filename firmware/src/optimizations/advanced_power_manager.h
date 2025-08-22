#ifndef ADVANCED_POWER_MANAGER_H
#define ADVANCED_POWER_MANAGER_H

#include <Arduino.h>
#include <driver/rtc_io.h>
#include <esp_sleep.h>
#include <esp_pm.h>
#include <vector>

/**
 * Advanced Power Manager - Enhanced power optimization for ESP32WildlifeCAM
 * 
 * Implements dynamic frequency scaling, predictive power scheduling,
 * and network-coordinated power management for maximum battery life.
 */

enum PowerProfile {
    POWER_MAXIMUM_PERFORMANCE = 0,
    POWER_BALANCED = 1,
    POWER_ECO_MODE = 2,
    POWER_SURVIVAL = 3,
    POWER_HIBERNATION = 4
};

class AdvancedPowerManager {
public:
    // Dynamic frequency scaling
    void adjustCPUFrequency(float load);
    void enableDVFS(); // Dynamic Voltage and Frequency Scaling
    bool setFrequency(uint32_t frequencyMHz);
    uint32_t getCurrentFrequency();
    
    // Intelligent component power management
    void powerDownUnusedPeripherals();
    void selectOptimalRadioPower(int8_t rssi, uint32_t distance);
    void dynamicCameraSettings(float lightLevel, bool motionDetected);
    void optimizeWiFiPower(int8_t signalStrength);
    
    // Predictive power scheduling
    struct PowerSchedule {
        uint32_t wakeTime;
        uint32_t activeTime;
        PowerProfile profile;
        String description;
        bool enabled;
        
        PowerSchedule() : wakeTime(0), activeTime(0), profile(POWER_BALANCED), 
                         description(""), enabled(true) {}
    };
    
    void schedulePowerEvents(const std::vector<PowerSchedule>& schedule);
    void optimizeBasedOnSolarData(float solarVoltage, float batteryLevel);
    void calculateOptimalDutyCycle(float batteryLevel, uint32_t missionDurationHours);
    
    // Network-coordinated power management
    void coordinateNetworkSleep(const std::vector<String>& nodeIds);
    void synchronizePowerState(PowerProfile targetProfile);
    
    // Real-time power optimization
    float measureActualPowerConsumption();
    void adaptPowerProfileBasedOnMeasurement();
    PowerProfile selectOptimalProfile(float batteryLevel, uint32_t timeToNextCharge);
    
    // Ultra-low power motion detection
    void enableDeepSleepWakeOnMotion();
    bool hasMotionSinceLastCheck();
    void configurePIRWakeup(gpio_num_t pirPin);
    
    // Initialization and management
    bool init();
    void cleanup();
    PowerProfile getCurrentProfile() const { return currentProfile; }
    void setProfile(PowerProfile profile);
    
    // Performance monitoring
    struct PowerMetrics {
        float averagePowerConsumption;
        uint32_t totalSleepTime;
        uint32_t totalActiveTime;
        float efficiencyRating;
        uint32_t wakeupCount;
        
        PowerMetrics() : averagePowerConsumption(0), totalSleepTime(0),
                        totalActiveTime(0), efficiencyRating(0), wakeupCount(0) {}
    };
    
    PowerMetrics getPowerMetrics() const;
    void resetMetrics();

private:
    // Power measurement and prediction
    struct PowerModel {
        float cpuPowerCoeff;
        float radioPowerCoeff;
        float cameraPowerCoeff;
        float basePower;
        bool calibrated;
        
        PowerModel() : cpuPowerCoeff(1.2f), radioPowerCoeff(0.8f), 
                      cameraPowerCoeff(2.5f), basePower(80.0f), calibrated(false) {}
    };
    
    PowerModel calibratePowerModel();
    float predictPowerConsumption(PowerProfile profile, uint32_t durationMs);
    
    // Hardware-specific optimizations
    void configureGPIOForLowPower();
    void optimizeFlashAccess();
    void configureBrownoutDetector();
    void disableUnusedPeripherals();
    
    // Profile management
    void applyPowerProfile(PowerProfile profile);
    bool validateProfile(PowerProfile profile);
    
    // State variables
    PowerProfile currentProfile;
    PowerModel powerModel;
    PowerMetrics metrics;
    std::vector<PowerSchedule> powerSchedule;
    bool initialized;
    bool dvfsEnabled;
    gpio_num_t pirWakeupPin;
    unsigned long lastPowerMeasurement;
    float lastMeasuredPower;
    
    // Constants
    static const uint32_t POWER_MEASUREMENT_INTERVAL = 10000; // 10 seconds
    static const uint32_t MIN_CPU_FREQ_MHZ = 80;
    static const uint32_t MAX_CPU_FREQ_MHZ = 240;
    static const uint32_t STEP_SIZE_MHZ = 40;
};

// Ultra-low power motion detector for deep sleep wake
class UltraLowPowerMotionDetector {
    gpio_num_t pirPin;
    bool motionDetected;
    bool initialized;
    unsigned long lastMotionTime;
    
public:
    UltraLowPowerMotionDetector(gpio_num_t pin);
    bool init();
    void enableDeepSleepWakeOnMotion();
    void cleanup();
    static void IRAM_ATTR motionISR();
    bool hasMotionSinceLastCheck();
    void clearMotionFlag();
    
private:
    static UltraLowPowerMotionDetector* instance;
    void configurePinForWakeup();
};

#endif // ADVANCED_POWER_MANAGER_H