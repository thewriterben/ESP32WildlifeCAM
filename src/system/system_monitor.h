#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <Arduino.h>
#include "../include/config.h"

/**
 * @brief System health monitoring and diagnostics
 * 
 * Monitors component health, performance metrics, temperature,
 * and provides diagnostic reporting for the wildlife camera system.
 */
class SystemMonitor {
public:
    /**
     * @brief Component health status
     */
    enum class HealthStatus {
        HEALTHY,
        WARNING,
        CRITICAL,
        FAILED,
        UNKNOWN
    };

    /**
     * @brief System component types
     */
    enum class Component {
        POWER_MANAGER,
        STORAGE_MANAGER,
        MOTION_DETECTOR,
        CAMERA,
        WIFI_MANAGER,
        SENSORS,
        SYSTEM_OVERALL
    };

    /**
     * @brief Component health information
     */
    struct ComponentHealth {
        Component component;
        HealthStatus status;
        String statusMessage;
        float healthScore;      // 0.0-1.0
        uint32_t lastCheck;
        uint32_t errorCount;
    };

    /**
     * @brief System performance metrics
     */
    struct PerformanceMetrics {
        uint32_t uptime;
        uint32_t freeHeap;
        uint32_t freePsram;
        uint32_t minFreeHeap;
        float cpuTemperature;
        uint16_t resetCount;
        uint32_t watchdogResets;
        float averageLoopTime;
        uint32_t maxLoopTime;
    };

    /**
     * @brief System diagnostics report
     */
    struct DiagnosticReport {
        uint32_t timestamp;
        PerformanceMetrics performance;
        ComponentHealth components[7]; // One for each component type
        String summary;
        HealthStatus overallHealth;
        String recommendations;
    };

    /**
     * @brief Initialize system monitor
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Update system monitor (call regularly)
     */
    void update();

    /**
     * @brief Check health of specific component
     * @param component Component to check
     * @return Component health information
     */
    ComponentHealth checkComponentHealth(Component component);

    /**
     * @brief Check health of all components
     */
    void checkAllComponentHealth();

    /**
     * @brief Get performance metrics
     * @return Current performance metrics
     */
    PerformanceMetrics getPerformanceMetrics();

    /**
     * @brief Generate diagnostic report
     * @return Complete diagnostic report
     */
    DiagnosticReport generateDiagnosticReport();

    /**
     * @brief Monitor system temperature
     * @return Temperature in Celsius
     */
    float monitorTemperature();

    /**
     * @brief Register component for monitoring
     * @param component Component type
     * @param healthCheckCallback Optional health check function
     */
    void registerComponent(Component component, bool (*healthCheckCallback)() = nullptr);

    /**
     * @brief Report component error
     * @param component Component that had an error
     * @param errorMessage Error description
     */
    void reportComponentError(Component component, const String& errorMessage);

    /**
     * @brief Get overall system health status
     * @return Overall health status
     */
    HealthStatus getOverallHealth();

    /**
     * @brief Enable or disable automatic health checks
     * @param enable Enable automatic checks
     */
    void setAutomaticHealthChecks(bool enable);

    /**
     * @brief Set health check interval
     * @param intervalMs Interval in milliseconds
     */
    void setHealthCheckInterval(uint32_t intervalMs);

    /**
     * @brief Reset component error count
     * @param component Component to reset
     */
    void resetComponentErrors(Component component);

    /**
     * @brief Get component name string
     * @param component Component type
     * @return Component name
     */
    static String getComponentName(Component component);

    /**
     * @brief Get health status string
     * @param status Health status
     * @return Status string
     */
    static String getHealthStatusString(HealthStatus status);

    /**
     * @brief Check if initialized
     * @return true if initialized
     */
    bool isInitialized() const { return initialized; }

    /**
     * @brief Cleanup system monitor
     */
    void cleanup();

private:
    bool initialized = false;
    bool automaticHealthChecks = true;
    uint32_t healthCheckInterval = 60000; // 1 minute
    uint32_t lastHealthCheck = 0;
    uint32_t lastPerformanceUpdate = 0;
    uint32_t bootTime = 0;
    
    ComponentHealth componentHealths[7];
    PerformanceMetrics performance = {};
    
    // Performance tracking
    uint32_t loopStartTime = 0;
    uint32_t totalLoopTime = 0;
    uint32_t loopCount = 0;
    
    /**
     * @brief Update performance metrics
     */
    void updatePerformanceMetrics();

    /**
     * @brief Initialize component health structures
     */
    void initializeComponentHealth();

    /**
     * @brief Calculate component health score
     * @param component Component to evaluate
     * @return Health score (0.0-1.0)
     */
    float calculateHealthScore(Component component);

    /**
     * @brief Determine overall health from components
     * @return Overall health status
     */
    HealthStatus calculateOverallHealth();

    /**
     * @brief Generate health recommendations
     * @return Recommendations string
     */
    String generateRecommendations();

    /**
     * @brief Check individual component health
     */
    void checkPowerManagerHealth();
    void checkStorageManagerHealth();
    void checkMotionDetectorHealth();
    void checkCameraHealth();
    void checkWiFiManagerHealth();
    void checkSensorsHealth();
};

#endif // SYSTEM_MONITOR_H