#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "../include/config.h"

/**
 * @brief Runtime configuration management for wildlife camera
 * 
 * Manages configuration persistence, runtime parameter adjustment,
 * field deployment settings, and validation for the system.
 */
class ConfigManager {
public:
    /**
     * @brief Configuration categories
     */
    enum class ConfigCategory {
        POWER,
        MOTION,
        CAMERA,
        STORAGE,
        NETWORK,
        SYSTEM,
        DEPLOYMENT
    };

    /**
     * @brief Configuration validation result
     */
    struct ValidationResult {
        bool isValid;
        String errorMessage;
        String warnings;
    };

    /**
     * @brief Power configuration parameters
     */
    struct PowerConfig {
        float batteryLowThreshold;
        float batteryCriticalThreshold;
        uint32_t deepSleepDuration;
        bool adaptiveDutyCycle;
        bool solarChargingEnabled;
        uint8_t maxCpuFreq;
        uint8_t minCpuFreq;
    };

    /**
     * @brief Motion detection configuration
     */
    struct MotionConfig {
        uint8_t pirSensitivity;
        float cameraThreshold;
        uint32_t cooldownPeriod;
        bool enableFalsePositiveFilter;
        float confidenceThreshold;
        uint8_t detectionMethod; // 0=PIR, 1=Camera, 2=Both
    };

    /**
     * @brief Camera configuration
     */
    struct CameraConfig {
        uint8_t frameSize;      // FRAMESIZE enum
        uint8_t jpegQuality;    // 0-63
        uint8_t brightness;     // -2 to 2
        uint8_t contrast;       // -2 to 2
        uint8_t saturation;     // -2 to 2
        bool autoExposure;
        uint16_t exposureValue;
    };

    /**
     * @brief Storage configuration
     */
    struct StorageConfig {
        uint8_t compressionLevel;
        uint32_t maxFileSize;
        uint8_t cleanupThreshold;  // Storage % to trigger cleanup
        uint8_t retentionDays;
        bool enableMetadata;
        bool enableLogging;
    };

    /**
     * @brief Network configuration
     */
    struct NetworkConfig {
        String wifiSSID;
        String wifiPassword;
        bool enableNTP;
        String ntpServer;
        int8_t timezone;
        bool enableRemoteAccess;
        uint16_t webServerPort;
    };

    /**
     * @brief Deployment configuration
     */
    struct DeploymentConfig {
        String locationName;
        float latitude;
        float longitude;
        String deploymentId;
        uint32_t deploymentDate;
        String researcherContact;
        String notes;
    };

    /**
     * @brief Initialize configuration manager
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Load all configurations from storage
     * @return true if load successful
     */
    bool loadConfiguration();

    /**
     * @brief Save all configurations to storage
     * @return true if save successful
     */
    bool saveConfiguration();

    /**
     * @brief Reset all configurations to defaults
     */
    void resetToDefaults();

    /**
     * @brief Get power configuration
     * @return Current power configuration
     */
    PowerConfig getPowerConfig() const { return powerConfig; }

    /**
     * @brief Set power configuration
     * @param config New power configuration
     * @param saveImmediate Save to storage immediately
     * @return true if set successfully
     */
    bool setPowerConfig(const PowerConfig& config, bool saveImmediate = false);

    /**
     * @brief Get motion configuration
     * @return Current motion configuration
     */
    MotionConfig getMotionConfig() const { return motionConfig; }

    /**
     * @brief Set motion configuration
     * @param config New motion configuration
     * @param saveImmediate Save to storage immediately
     * @return true if set successfully
     */
    bool setMotionConfig(const MotionConfig& config, bool saveImmediate = false);

    /**
     * @brief Get camera configuration
     * @return Current camera configuration
     */
    CameraConfig getCameraConfig() const { return cameraConfig; }

    /**
     * @brief Set camera configuration
     * @param config New camera configuration
     * @param saveImmediate Save to storage immediately
     * @return true if set successfully
     */
    bool setCameraConfig(const CameraConfig& config, bool saveImmediate = false);

    /**
     * @brief Get storage configuration
     * @return Current storage configuration
     */
    StorageConfig getStorageConfig() const { return storageConfig; }

    /**
     * @brief Set storage configuration
     * @param config New storage configuration
     * @param saveImmediate Save to storage immediately
     * @return true if set successfully
     */
    bool setStorageConfig(const StorageConfig& config, bool saveImmediate = false);

    /**
     * @brief Get network configuration
     * @return Current network configuration
     */
    NetworkConfig getNetworkConfig() const { return networkConfig; }

    /**
     * @brief Set network configuration
     * @param config New network configuration
     * @param saveImmediate Save to storage immediately
     * @return true if set successfully
     */
    bool setNetworkConfig(const NetworkConfig& config, bool saveImmediate = false);

    /**
     * @brief Get deployment configuration
     * @return Current deployment configuration
     */
    DeploymentConfig getDeploymentConfig() const { return deploymentConfig; }

    /**
     * @brief Set deployment configuration
     * @param config New deployment configuration
     * @param saveImmediate Save to storage immediately
     * @return true if set successfully
     */
    bool setDeploymentConfig(const DeploymentConfig& config, bool saveImmediate = false);

    /**
     * @brief Validate configuration
     * @param category Configuration category to validate
     * @return Validation result
     */
    ValidationResult validateConfig(ConfigCategory category);

    /**
     * @brief Validate all configurations
     * @return Overall validation result
     */
    ValidationResult validateAllConfigs();

    /**
     * @brief Get configuration as JSON string
     * @param category Configuration category
     * @return JSON string representation
     */
    String getConfigAsJSON(ConfigCategory category);

    /**
     * @brief Set configuration from JSON string
     * @param category Configuration category
     * @param jsonString JSON configuration
     * @return true if parsing and setting successful
     */
    bool setConfigFromJSON(ConfigCategory category, const String& jsonString);

    /**
     * @brief Check if configuration has changed since last save
     * @return true if configuration is dirty
     */
    bool isDirty() const { return configDirty; }

    /**
     * @brief Mark configuration as clean (saved)
     */
    void markClean() { configDirty = false; }

    /**
     * @brief Check if initialized
     * @return true if initialized
     */
    bool isInitialized() const { return initialized; }

    /**
     * @brief Cleanup configuration manager
     */
    void cleanup();

private:
    bool initialized = false;
    bool configDirty = false;
    Preferences preferences;
    
    // Configuration structures
    PowerConfig powerConfig;
    MotionConfig motionConfig;
    CameraConfig cameraConfig;
    StorageConfig storageConfig;
    NetworkConfig networkConfig;
    DeploymentConfig deploymentConfig;
    
    /**
     * @brief Initialize default configurations
     */
    void initializeDefaults();

    /**
     * @brief Load specific configuration category
     * @param category Configuration category
     * @return true if load successful
     */
    bool loadConfigCategory(ConfigCategory category);

    /**
     * @brief Save specific configuration category
     * @param category Configuration category
     * @return true if save successful
     */
    bool saveConfigCategory(ConfigCategory category);

    /**
     * @brief Get preference namespace for category
     * @param category Configuration category
     * @return Namespace string
     */
    String getNamespace(ConfigCategory category);

    /**
     * @brief Validate power configuration
     * @return Validation result
     */
    ValidationResult validatePowerConfig();

    /**
     * @brief Validate motion configuration
     * @return Validation result
     */
    ValidationResult validateMotionConfig();

    /**
     * @brief Validate camera configuration
     * @return Validation result
     */
    ValidationResult validateCameraConfig();

    /**
     * @brief Validate storage configuration
     * @return Validation result
     */
    ValidationResult validateStorageConfig();

    /**
     * @brief Validate network configuration
     * @return Validation result
     */
    ValidationResult validateNetworkConfig();

    /**
     * @brief Validate deployment configuration
     * @return Validation result
     */
    ValidationResult validateDeploymentConfig();
};

#endif // CONFIG_MANAGER_H