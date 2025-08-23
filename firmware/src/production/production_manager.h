#ifndef PRODUCTION_MANAGER_H
#define PRODUCTION_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>

// Production subsystem includes
#include "deployment/ota_manager.h"
#include "deployment/config_manager.h"
#include "field_testing/environmental_adapter.h"
#include "monitoring/species_detector.h"
#include "enterprise/cloud_integrator.h"
#include "security/data_protector.h"

/**
 * @brief Central Production Manager for Phase 4 Wildlife Camera Deployment
 * 
 * Coordinates all production-ready features including deployment, field testing,
 * monitoring, enterprise integration, and security across the wildlife camera network.
 */
class ProductionManager {
public:
    /**
     * @brief Production deployment phases
     */
    enum class DeploymentPhase {
        INITIALIZATION = 0,
        DEVICE_PROVISIONING = 1,
        FIELD_TESTING = 2,
        PRODUCTION_DEPLOYMENT = 3,
        MONITORING_ACTIVE = 4,
        MAINTENANCE_MODE = 5,
        DECOMMISSIONING = 6
    };

    /**
     * @brief System health status levels
     */
    enum class HealthStatus {
        EXCELLENT = 0,  // All systems optimal
        GOOD = 1,       // Minor issues, fully functional
        WARNING = 2,    // Some degradation, may need attention
        CRITICAL = 3,   // Significant issues, limited functionality
        FAILURE = 4     // System failure, intervention required
    };

    /**
     * @brief Production system statistics
     */
    struct SystemStatistics {
        DeploymentPhase current_phase;
        HealthStatus overall_health;
        uint32_t uptime_seconds;
        uint32_t total_detections;
        uint32_t successful_transmissions;
        uint32_t failed_transmissions;
        float battery_level;
        float system_efficiency;
        uint32_t last_maintenance_timestamp;
        
        // Subsystem health
        struct {
            bool ota_manager_healthy;
            bool config_manager_healthy;
            bool environmental_adapter_healthy;
            bool species_detector_healthy;
            bool cloud_integrator_healthy;
            bool data_protector_healthy;
        } subsystem_health;
    };

    /**
     * @brief Production configuration profile
     */
    struct ProductionProfile {
        String profile_name;
        String deployment_scenario; // research, conservation, urban, etc.
        DeploymentPhase target_phase;
        
        // Feature enablement
        bool enable_ota_updates;
        bool enable_cloud_sync;
        bool enable_environmental_adaptation;
        bool enable_advanced_species_detection;
        bool enable_data_encryption;
        bool enable_audit_logging;
        
        // Performance parameters
        float detection_confidence_threshold;
        uint32_t sync_interval_seconds;
        uint32_t adaptation_interval_seconds;
        uint8_t power_optimization_level; // 0-10
        
        // Conservation parameters
        bool prioritize_endangered_species;
        bool enable_conservation_alerts;
        bool enable_anti_poaching_mode;
        float location_privacy_level;
    };

    /**
     * @brief Initialize production manager with specified profile
     * @param profile Production profile configuration
     * @return true if initialization successful
     */
    static bool initialize(const ProductionProfile& profile);

    /**
     * @brief Start production deployment process
     * @param target_phase Target deployment phase to achieve
     * @return true if deployment started successfully
     */
    static bool startDeployment(DeploymentPhase target_phase = DeploymentPhase::PRODUCTION_DEPLOYMENT);

    /**
     * @brief Process all production subsystems (call in main loop)
     */
    static void process();

    /**
     * @brief Get current system statistics
     * @return Current production system statistics
     */
    static SystemStatistics getSystemStatistics();

    /**
     * @brief Get current deployment phase
     * @return Current deployment phase
     */
    static DeploymentPhase getCurrentPhase() { return current_phase_; }

    /**
     * @brief Transition to next deployment phase
     * @return true if phase transition successful
     */
    static bool advanceToNextPhase();

    /**
     * @brief Perform system health check
     * @return Overall system health status
     */
    static HealthStatus performHealthCheck();

    /**
     * @brief Enable/disable production feature
     * @param feature_name Name of feature to control
     * @param enabled Enable/disable the feature
     * @return true if feature control successful
     */
    static bool setFeatureEnabled(const String& feature_name, bool enabled);

    /**
     * @brief Get list of available production profiles
     * @return Vector of available profile names
     */
    static std::vector<String> getAvailableProfiles();

    /**
     * @brief Load production profile
     * @param profile_name Name of profile to load
     * @return true if profile loaded successfully
     */
    static bool loadProfile(const String& profile_name);

    /**
     * @brief Save current configuration as profile
     * @param profile_name Name for the new profile
     * @return true if profile saved successfully
     */
    static bool saveProfile(const String& profile_name);

    /**
     * @brief Trigger emergency shutdown
     * @param reason Reason for emergency shutdown
     * @return true if shutdown initiated successfully
     */
    static bool emergencyShutdown(const String& reason);

    /**
     * @brief Generate production deployment report
     * @param include_detailed_logs Include detailed subsystem logs
     * @return JSON report with deployment status and statistics
     */
    static DynamicJsonDocument generateDeploymentReport(bool include_detailed_logs = false);

    /**
     * @brief Perform system maintenance tasks
     * @param maintenance_type Type of maintenance to perform
     * @return true if maintenance completed successfully
     */
    static bool performMaintenance(const String& maintenance_type = "routine");

    /**
     * @brief Get system alerts and warnings
     * @return Vector of current system alerts
     */
    static std::vector<String> getSystemAlerts();

    /**
     * @brief Clear system alert
     * @param alert_id ID of alert to clear
     * @return true if alert cleared successfully
     */
    static bool clearAlert(const String& alert_id);

    /**
     * @brief Set production parameters
     * @param parameters JSON object with parameter updates
     * @return true if parameters updated successfully
     */
    static bool setProductionParameters(const DynamicJsonDocument& parameters);

    /**
     * @brief Get production parameters
     * @return Current production parameters as JSON
     */
    static DynamicJsonDocument getProductionParameters();

    /**
     * @brief Test all production subsystems
     * @return JSON object with test results for each subsystem
     */
    static DynamicJsonDocument runSystemTests();

    /**
     * @brief Export production logs
     * @param hours_back Hours of logs to export
     * @param subsystem Specific subsystem logs (empty for all)
     * @return JSON array with log entries
     */
    static DynamicJsonDocument exportLogs(uint16_t hours_back = 24, const String& subsystem = "");

    /**
     * @brief Register for production alerts
     * @param alert_types Vector of alert types to monitor
     * @param callback Callback function for alerts
     */
    static void registerAlertCallback(const std::vector<String>& alert_types, void (*callback)(const String& alert_type, const String& message));

    /**
     * @brief Get subsystem status
     * @param subsystem_name Name of subsystem to check
     * @return JSON object with subsystem status details
     */
    static DynamicJsonDocument getSubsystemStatus(const String& subsystem_name);

    /**
     * @brief Restart subsystem
     * @param subsystem_name Name of subsystem to restart
     * @return true if subsystem restarted successfully
     */
    static bool restartSubsystem(const String& subsystem_name);

private:
    static ProductionProfile current_profile_;
    static DeploymentPhase current_phase_;
    static HealthStatus current_health_;
    static SystemStatistics statistics_;
    static uint32_t initialization_time_;
    static uint32_t last_health_check_;
    static uint32_t last_maintenance_;
    static bool initialized_;
    static std::vector<String> system_alerts_;
    static void (*alert_callback_)(const String& alert_type, const String& message);
    
    // Phase management
    static bool initializePhase();
    static bool executeDeviceProvisioning();
    static bool executeFieldTesting();
    static bool executeProductionDeployment();
    static bool executeMonitoringActive();
    static bool executeMaintenanceMode();
    static bool executeDecommissioning();
    
    // Subsystem management
    static bool initializeSubsystems();
    static void processSubsystems();
    static bool checkSubsystemHealth(const String& subsystem_name);
    static void updateStatistics();
    
    // Health monitoring
    static HealthStatus calculateOverallHealth();
    static void checkForAlerts();
    static void addAlert(const String& alert_type, const String& message);
    static void removeExpiredAlerts();
    
    // Profile management
    static bool loadDefaultProfiles();
    static ProductionProfile createDefaultProfile(const String& scenario);
    static bool validateProfile(const ProductionProfile& profile);
    
    // Utility methods
    static String phaseToString(DeploymentPhase phase);
    static String healthToString(HealthStatus health);
    static void logProductionEvent(const String& event, const String& details);
    static uint32_t getUptime();
    static float calculateSystemEfficiency();
};

// Global production manager instance
extern ProductionManager* g_productionManager;

#endif // PRODUCTION_MANAGER_H