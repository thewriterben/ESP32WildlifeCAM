#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <vector>
#include <map>
#include "../../config.h"

/**
 * @brief Centralized Configuration Management for Production Deployment
 * 
 * Manages configuration versioning, distribution, and validation across
 * wildlife camera networks with support for remote configuration updates.
 */
class ConfigManager {
public:
    /**
     * @brief Configuration section types
     */
    enum class ConfigSection {
        CAMERA = 0,
        MOTION_DETECTION = 1,
        POWER_MANAGEMENT = 2,
        NETWORK = 3,
        AI_PROCESSING = 4,
        DEPLOYMENT = 5,
        SECURITY = 6,
        CUSTOM = 7
    };

    /**
     * @brief Configuration validation result
     */
    struct ValidationResult {
        bool valid;
        String error_message;
        std::vector<String> warnings;
        float compatibility_score;
    };

    /**
     * @brief Configuration version information
     */
    struct ConfigVersion {
        String version;
        String timestamp;
        String author;
        String description;
        String checksum;
        std::map<String, String> changes;
    };

    /**
     * @brief Configuration profile for different deployment scenarios
     */
    struct ConfigProfile {
        String name;
        String description;
        ConfigSection primary_section;
        std::map<String, String> parameters;
        bool requires_restart;
        float priority;
    };

    /**
     * @brief Initialize configuration manager
     * @param enable_remote_sync Enable remote configuration synchronization
     * @return true if initialization successful
     */
    static bool initialize(bool enable_remote_sync = true);

    /**
     * @brief Load configuration from storage
     * @param config_path Path to configuration file
     * @return true if loaded successfully
     */
    static bool loadConfiguration(const String& config_path = "/config.json");

    /**
     * @brief Save current configuration to storage
     * @param config_path Path to save configuration
     * @param create_backup Create backup of existing configuration
     * @return true if saved successfully
     */
    static bool saveConfiguration(const String& config_path = "/config.json", bool create_backup = true);

    /**
     * @brief Get configuration parameter value
     * @param section Configuration section
     * @param key Parameter key
     * @param default_value Default value if not found
     * @return Parameter value as string
     */
    static String getParameter(ConfigSection section, const String& key, const String& default_value = "");

    /**
     * @brief Set configuration parameter value
     * @param section Configuration section
     * @param key Parameter key
     * @param value Parameter value
     * @param immediate_save Save configuration immediately
     * @return true if set successfully
     */
    static bool setParameter(ConfigSection section, const String& key, const String& value, bool immediate_save = false);

    /**
     * @brief Get integer configuration parameter
     * @param section Configuration section
     * @param key Parameter key
     * @param default_value Default value if not found
     * @return Parameter value as integer
     */
    static int getIntParameter(ConfigSection section, const String& key, int default_value = 0);

    /**
     * @brief Get float configuration parameter
     * @param section Configuration section
     * @param key Parameter key
     * @param default_value Default value if not found
     * @return Parameter value as float
     */
    static float getFloatParameter(ConfigSection section, const String& key, float default_value = 0.0f);

    /**
     * @brief Get boolean configuration parameter
     * @param section Configuration section
     * @param key Parameter key
     * @param default_value Default value if not found
     * @return Parameter value as boolean
     */
    static bool getBoolParameter(ConfigSection section, const String& key, bool default_value = false);

    /**
     * @brief Validate configuration parameters
     * @param config_json Configuration JSON to validate
     * @return Validation result with errors and warnings
     */
    static ValidationResult validateConfiguration(const DynamicJsonDocument& config_json);

    /**
     * @brief Apply configuration profile
     * @param profile_name Name of profile to apply
     * @return true if applied successfully
     */
    static bool applyProfile(const String& profile_name);

    /**
     * @brief Create custom configuration profile
     * @param profile Configuration profile to create
     * @return true if created successfully
     */
    static bool createProfile(const ConfigProfile& profile);

    /**
     * @brief Get list of available configuration profiles
     * @return Vector of available profile names
     */
    static std::vector<String> getAvailableProfiles();

    /**
     * @brief Sync configuration with remote server
     * @param server_url Remote configuration server URL
     * @return true if sync successful
     */
    static bool syncWithRemote(const String& server_url = "");

    /**
     * @brief Check for configuration updates from remote server
     * @return true if updates available
     */
    static bool checkForUpdates();

    /**
     * @brief Download and apply remote configuration
     * @param version_id Specific version to download (empty for latest)
     * @return true if download and apply successful
     */
    static bool downloadRemoteConfig(const String& version_id = "");

    /**
     * @brief Get configuration version history
     * @return Vector of configuration versions
     */
    static std::vector<ConfigVersion> getVersionHistory();

    /**
     * @brief Rollback to previous configuration version
     * @param steps Number of versions to rollback (default: 1)
     * @return true if rollback successful
     */
    static bool rollbackConfiguration(uint8_t steps = 1);

    /**
     * @brief Export configuration as JSON string
     * @param include_metadata Include version and metadata
     * @return Configuration as formatted JSON string
     */
    static String exportConfiguration(bool include_metadata = true);

    /**
     * @brief Import configuration from JSON string
     * @param config_json JSON configuration string
     * @param validate_before_import Validate before importing
     * @return true if import successful
     */
    static bool importConfiguration(const String& config_json, bool validate_before_import = true);

    /**
     * @brief Get configuration statistics
     * @return JSON object with configuration statistics
     */
    static DynamicJsonDocument getStatistics();

    /**
     * @brief Reset configuration to factory defaults
     * @param section Specific section to reset (or all if not specified)
     * @return true if reset successful
     */
    static bool resetToDefaults(ConfigSection section = static_cast<ConfigSection>(-1));

    /**
     * @brief Register configuration change callback
     * @param callback Function to call when configuration changes
     */
    static void registerChangeCallback(void (*callback)(ConfigSection section, const String& key, const String& old_value, const String& new_value));

    /**
     * @brief Enable/disable automatic configuration backup
     * @param enabled Enable automatic backup
     * @param interval_hours Backup interval in hours
     */
    static void setAutoBackup(bool enabled, uint8_t interval_hours = 24);

    /**
     * @brief Process configuration manager tasks (call in main loop)
     */
    static void process();

private:
    static DynamicJsonDocument config_data_;
    static String current_config_path_;
    static bool remote_sync_enabled_;
    static uint32_t last_sync_time_;
    static uint32_t last_backup_time_;
    static bool auto_backup_enabled_;
    static uint8_t backup_interval_hours_;
    static void (*change_callback_)(ConfigSection, const String&, const String&, const String&);
    static bool initialized_;

    // Helper methods
    static String sectionToString(ConfigSection section);
    static ConfigSection stringToSection(const String& section_str);
    static String generateConfigChecksum(const DynamicJsonDocument& config);
    static bool createConfigBackup(const String& suffix = "");
    static bool restoreConfigBackup(const String& backup_file);
    static std::vector<String> listConfigBackups();
    static void notifyConfigChange(ConfigSection section, const String& key, const String& old_value, const String& new_value);
    static bool validateParameterValue(ConfigSection section, const String& key, const String& value);
    static DynamicJsonDocument getDefaultConfiguration();
    static bool downloadFile(const String& url, const String& local_path);
    static String generateDeviceFingerprint();
};

#endif // CONFIG_MANAGER_H