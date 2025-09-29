/**
 * @file power_bridge.cpp
 * @brief Power management integration bridge for camera handler
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 * 
 * This file provides bridge functions to connect the camera handler
 * with the power management system.
 */

#include "../power/power_manager.h"
#include <Arduino.h>

// Global power manager instance
static PowerManager* g_power_manager = nullptr;

/**
 * Initialize the power management system
 */
bool initializePowerManager() {
    if (g_power_manager != nullptr) {
        return true; // Already initialized
    }
    
    Serial.println("Power Bridge: Initializing power manager...");
    
    g_power_manager = new PowerManager();
    if (!g_power_manager) {
        Serial.println("Power Bridge: Failed to create power manager");
        return false;
    }
    
    if (!g_power_manager->initialize()) {
        Serial.println("Power Bridge: Failed to initialize power manager");
        delete g_power_manager;
        g_power_manager = nullptr;
        return false;
    }
    
    Serial.println("Power Bridge: Power manager initialized successfully");
    return true;
}

/**
 * Get current battery level (0.0 - 1.0)
 */
float getCurrentBatteryLevel() {
    if (!g_power_manager) {
        Serial.println("Power Bridge: Power manager not initialized, assuming full battery");
        return 1.0f; // Assume full battery if power manager not available
    }
    
    float percentage = g_power_manager->getBatteryPercentage();
    return percentage / 100.0f; // Convert percentage to 0.0-1.0 range
}

/**
 * Get current power state as integer
 */
int getCurrentPowerState() {
    if (!g_power_manager) {
        Serial.println("Power Bridge: Power manager not initialized, assuming normal state");
        return 0; // NORMAL state
    }
    
    PowerManager::PowerState state = g_power_manager->getPowerState();
    return static_cast<int>(state);
}

/**
 * Set power mode
 */
void setPowerMode(int mode) {
    if (!g_power_manager) {
        Serial.println("Power Bridge: Power manager not initialized, cannot set power mode");
        return;
    }
    
    PowerManager::PowerMode power_mode = static_cast<PowerManager::PowerMode>(mode);
    g_power_manager->setPowerMode(power_mode);
    
    Serial.printf("Power Bridge: Set power mode to %d\n", mode);
}

/**
 * Get current power mode
 */
int getCurrentPowerMode() {
    if (!g_power_manager) {
        Serial.println("Power Bridge: Power manager not initialized, assuming balanced mode");
        return 1; // BALANCED mode
    }
    
    PowerManager::PowerMode mode = g_power_manager->getPowerMode();
    return static_cast<int>(mode);
}

/**
 * Check if system should enter deep sleep
 */
bool shouldEnterDeepSleep() {
    if (!g_power_manager) {
        return false; // Don't sleep if power manager not available
    }
    
    return g_power_manager->shouldEnterDeepSleep();
}

/**
 * Update power system (should be called regularly)
 */
void updatePowerSystem() {
    if (g_power_manager) {
        g_power_manager->update();
    }
}

/**
 * Prepare for deep sleep
 */
void prepareForDeepSleep(uint32_t sleep_duration) {
    if (g_power_manager) {
        g_power_manager->prepareForDeepSleep(sleep_duration);
        Serial.printf("Power Bridge: Prepared for deep sleep (%lu ms)\n", sleep_duration);
    }
}

/**
 * Get battery voltage
 */
float getBatteryVoltage() {
    if (!g_power_manager) {
        return 4.2f; // Assume full Li-ion voltage if not available
    }
    
    return g_power_manager->getBatteryVoltage();
}

/**
 * Cleanup power bridge resources
 */
void cleanupPowerBridge() {
    if (g_power_manager) {
        delete g_power_manager;
        g_power_manager = nullptr;
        Serial.println("Power Bridge: Power manager cleaned up");
    }
}