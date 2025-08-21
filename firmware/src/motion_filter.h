#ifndef MOTION_FILTER_H
#define MOTION_FILTER_H

#include <Arduino.h>

// Motion detection statistics structure
struct MotionStats {
    unsigned long lastMotionTime;
    int consecutiveCount;
    float windSpeed;
    float rainfall;
    float temperature;
    bool filteringEnabled;
};

// Motion filter status structure
struct MotionFilterStatus {
    bool initialized;
    unsigned long lastMotionTime;
    bool currentlyDetecting;
    bool weatherSensorActive;
    bool filteringActive;
    int consecutiveMotions;
};

namespace MotionFilter {
    /**
     * Initialize motion detection system
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * Check if motion is currently detected
     * @return true if motion detected, false otherwise
     */
    bool isMotionDetected();
    
    /**
     * Validate motion against weather conditions and other filters
     * @return true if motion is valid, false if filtered out
     */
    bool isValidMotion();
    
    /**
     * Get current motion detection statistics
     * @return Motion statistics structure
     */
    MotionStats getMotionStats();
    
    /**
     * Set motion sensitivity (0-100)
     * @param sensitivity Sensitivity level from 0 (least sensitive) to 100 (most sensitive)
     */
    void setMotionSensitivity(int sensitivity);
    
    /**
     * Enable or disable weather filtering
     * @param enabled true to enable weather filtering, false to disable
     */
    void setWeatherFiltering(bool enabled);
    
    /**
     * Get detailed motion filter status
     * @return Motion filter status structure
     */
    MotionFilterStatus getStatus();
    
    /**
     * Reset motion detection statistics
     */
    void resetStats();
    
    /**
     * Cleanup motion filter resources
     */
    void cleanup();
}

#endif // MOTION_FILTER_H