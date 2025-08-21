#ifndef SOLAR_MANAGER_H
#define SOLAR_MANAGER_H

#include <Arduino.h>

// Power state enumeration
enum PowerState {
    POWER_CRITICAL = 0,  // Battery critically low
    POWER_LOW = 1,       // Battery low
    POWER_GOOD = 2,      // Battery good
    POWER_NORMAL = 3     // Battery normal/full
};

// Power statistics structure
struct PowerStats {
    float batteryVoltage;
    float solarVoltage;
    int batteryPercentage;
    bool isCharging;
    PowerState powerState;
    unsigned long totalChargingTime;
    float estimatedRuntime;
};

// Power system status structure
struct PowerSystemStatus {
    bool initialized;
    float batteryVoltage;
    float solarVoltage;
    int batteryPercentage;
    bool isCharging;
    PowerState powerState;
    float estimatedRuntime;
    unsigned long totalChargingTime;
    bool lowPowerMode;
};

namespace SolarManager {
    /**
     * Initialize solar power management system
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * Update power system status (call regularly)
     */
    void update();
    
    /**
     * Get current battery voltage
     * @return Battery voltage in volts
     */
    float getBatteryVoltage();
    
    /**
     * Get current solar panel voltage
     * @return Solar panel voltage in volts
     */
    float getSolarVoltage();
    
    /**
     * Check if battery is currently charging
     * @return true if charging, false otherwise
     */
    bool isCharging();
    
    /**
     * Get current power state
     * @return Current power state enum
     */
    PowerState getPowerState();
    
    /**
     * Get battery level as percentage (0-100)
     * @return Battery charge percentage
     */
    int getBatteryPercentage();
    
    /**
     * Get estimated remaining runtime in hours
     * @return Estimated runtime in hours
     */
    float getEstimatedRuntime();
    
    /**
     * Get power system statistics
     * @return Power statistics structure
     */
    PowerStats getPowerStats();
    
    /**
     * Check if system should enter low power mode
     * @return true if low power mode recommended
     */
    bool shouldEnterLowPower();
    
    /**
     * Enter power saving mode
     */
    void enterPowerSaving();
    
    /**
     * Exit power saving mode
     */
    void exitPowerSaving();
    
    /**
     * Calibrate voltage readings with known values
     * @param actualBatteryVoltage Actual battery voltage measured externally
     * @param actualSolarVoltage Actual solar voltage measured externally
     */
    void calibrateVoltage(float actualBatteryVoltage, float actualSolarVoltage);
    
    /**
     * Get detailed power system status
     * @return Power system status structure
     */
    PowerSystemStatus getSystemStatus();
    
    /**
     * Reset power statistics
     */
    void resetStats();
    
    /**
     * Cleanup solar manager resources
     */
    void cleanup();
}

#endif // SOLAR_MANAGER_H