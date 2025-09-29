/**
 * @file motion_coordinator.h
 * @brief Motion detection coordination and management
 * @author thewriterben
 * @date 2025-08-31
 */

#ifndef MOTION_COORDINATOR_H
#define MOTION_COORDINATOR_H

#include <Arduino.h>
#include <functional>

/**
 * @brief Motion detection coordinator class
 * 
 * Manages PIR sensor detection, environmental filtering,
 * and camera trigger coordination.
 */
class MotionCoordinator {
public:
    /**
     * @brief Environmental conditions for motion detection
     */
    struct EnvironmentalConditions {
        float temperature = 20.0f;
        float humidity = 50.0f;
        float lightLevel = 50.0f;
        float windSpeed = 0.0f;
        bool isDaytime = true;
    };

    /**
     * @brief Motion detection callback function type
     */
    using MotionCallback = std::function<void(float confidence)>;

    /**
     * @brief Initialize motion detection system
     * @param pirPin PIR sensor pin number
     * @param callback Function to call when motion is detected
     * @return true if initialization successful
     */
    bool initialize(int pirPin, MotionCallback callback);

    /**
     * @brief Update motion detection (call regularly)
     */
    void update();

    /**
     * @brief Set environmental conditions for filtering
     * @param conditions Current environmental conditions
     */
    void setEnvironmentalConditions(const EnvironmentalConditions& conditions);

    /**
     * @brief Enable or disable motion detection
     * @param enable Enable motion detection
     */
    void setEnabled(bool enable);

    /**
     * @brief Check if motion detection is enabled
     * @return true if enabled
     */
    bool isEnabled() const { return m_enabled; }

    /**
     * @brief Get current motion sensitivity
     * @return Sensitivity value (0.0-1.0)
     */
    float getSensitivity() const { return m_sensitivity; }

    /**
     * @brief Set motion sensitivity
     * @param sensitivity Sensitivity value (0.0-1.0)
     */
    void setSensitivity(float sensitivity);

    /**
     * @brief Get motion detection statistics
     */
    struct MotionStats {
        uint32_t totalDetections = 0;
        uint32_t filteredDetections = 0;
        uint32_t falsePositives = 0;
        float averageConfidence = 0.0f;
        uint32_t lastDetectionTime = 0;
    };

    /**
     * @brief Get motion detection statistics
     * @return Current statistics
     */
    MotionStats getStatistics() const { return m_stats; }

    /**
     * @brief Reset statistics
     */
    void resetStatistics();

private:
    bool m_initialized = false;
    bool m_enabled = true;
    int m_pirPin = -1;
    float m_sensitivity = 0.5f;
    
    MotionCallback m_callback;
    EnvironmentalConditions m_conditions;
    MotionStats m_stats;
    
    uint32_t m_lastMotionTime = 0;
    uint32_t m_debounceTime = 2000; // 2 seconds
    bool m_lastPirState = false;

    /**
     * @brief Check PIR sensor for motion
     * @return true if motion detected
     */
    bool checkPIRMotion();

    /**
     * @brief Apply environmental filtering
     * @param rawDetection Raw motion detection result
     * @return Filtered confidence (0.0-1.0)
     */
    float applyEnvironmentalFiltering(bool rawDetection);

    /**
     * @brief Calculate motion confidence based on conditions
     * @param detection Raw detection state
     * @return Confidence value (0.0-1.0)
     */
    float calculateConfidence(bool detection);
};

#endif // MOTION_COORDINATOR_H