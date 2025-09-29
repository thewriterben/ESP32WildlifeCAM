#ifndef MOTION_DETECTOR_H
#define MOTION_DETECTOR_H

#include <Arduino.h>
#include "esp_camera.h"
#include "motion_detection.h"
#include "pir_sensor.h"
#include "../include/config.h"
#include "../include/pins.h"

/**
 * @brief Unified motion detection system
 * 
 * Combines PIR sensor and camera-based motion detection
 * for comprehensive wildlife monitoring with false positive filtering.
 */
class MotionDetector {
public:
    /**
     * @brief Motion detection method
     */
    enum class DetectionMethod {
        PIR_ONLY,           // PIR sensor only
        CAMERA_ONLY,        // Camera frame analysis only
        PIR_AND_CAMERA,     // PIR confirms camera motion
        PIR_OR_CAMERA,     // Either PIR or camera triggers
        ADAPTIVE           // Adaptive based on conditions
    };

    /**
     * @brief Motion detection result
     */
    struct MotionResult {
        bool motionDetected;
        bool pirTriggered;
        bool cameraMotion;
        float confidence;           // Combined confidence score
        float pirConfidence;        // PIR confidence
        float cameraConfidence;     // Camera motion confidence
        uint32_t detectionTime;     // Time of detection
        DetectionMethod methodUsed;
        MotionDetection::MotionResult cameraDetails;
    };

    /**
     * @brief Motion detection configuration
     */
    struct MotionConfig {
        DetectionMethod method;
        float pirSensitivity;
        float cameraSensitivity;
        uint32_t cooldownPeriod;    // Minimum time between detections
        bool enableFalsePositiveFilter;
        float confidenceThreshold;
        bool enableAdaptiveMode;
    };

    /**
     * @brief Initialize motion detection system
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Detect motion using current configuration
     * @return Motion detection result
     */
    bool detectMotion();

    /**
     * @brief Get detailed motion detection result
     * @return Complete motion result structure
     */
    MotionResult getMotionResult();

    /**
     * @brief Configure PIR sensor parameters
     */
    void configurePIRSensor();

    /**
     * @brief Get motion confidence score
     * @return Confidence level (0.0-1.0)
     */
    float getMotionConfidence();

    /**
     * @brief Enable motion interrupt on PIR sensor
     */
    void enableMotionInterrupt();

    /**
     * @brief Disable motion interrupt
     */
    void disableMotionInterrupt();

    /**
     * @brief Set detection method
     * @param method Detection method to use
     */
    void setDetectionMethod(DetectionMethod method);

    /**
     * @brief Set motion configuration
     * @param config Configuration parameters
     */
    void setConfiguration(const MotionConfig& config);

    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    MotionConfig getConfiguration() const { return config; }

    /**
     * @brief Update motion detector (call regularly)
     */
    void update();

    /**
     * @brief Check if in cooldown period
     * @return true if in cooldown
     */
    bool isInCooldown();

    /**
     * @brief Reset motion detection statistics
     */
    void resetStatistics();

    /**
     * @brief Enable or disable false positive filtering
     * @param enable Enable filtering
     */
    void setFalsePositiveFiltering(bool enable);

    /**
     * @brief PIR interrupt handler (static for ISR)
     */
    static void IRAM_ATTR pirInterruptHandler();

    /**
     * @brief Check if initialized
     * @return true if initialized
     */
    bool isInitialized() const { return initialized; }

    /**
     * @brief Cleanup resources
     */
    void cleanup();

private:
    bool initialized = false;
    MotionConfig config;
    MotionResult lastResult = {};
    
    // Component instances
    MotionDetection* cameraMotion = nullptr;
    PIRSensor* pirSensor = nullptr;
    
    // Timing and filtering
    uint32_t lastDetectionTime = 0;
    uint32_t lastCameraFrame = 0;
    uint32_t falsePositiveCount = 0;
    uint32_t totalDetections = 0;
    
    // Interrupt handling
    static volatile bool pirInterruptFlag;
    static MotionDetector* instance;
    
    /**
     * @brief Filter false positives using multiple criteria
     * @param result Motion result to filter
     * @return true if motion is likely genuine
     */
    bool filterFalsePositives(const MotionResult& result);

    /**
     * @brief Combine PIR and camera confidence scores
     * @param pirConf PIR confidence
     * @param camConf Camera confidence
     * @return Combined confidence
     */
    float combineConfidence(float pirConf, float camConf);

    /**
     * @brief Initialize default configuration
     */
    void initializeDefaultConfig();

    /**
     * @brief Handle PIR interrupt (non-static)
     */
    void handlePIRInterrupt();

    /**
     * @brief Adaptive method selection based on conditions
     * @return Recommended detection method
     */
    DetectionMethod selectAdaptiveMethod();
};

#endif // MOTION_DETECTOR_H