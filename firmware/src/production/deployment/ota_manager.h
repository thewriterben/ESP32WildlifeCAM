#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "../../config.h"

/**
 * @brief Over-the-Air (OTA) Update Manager for Production Deployment
 * 
 * Provides secure, reliable firmware updates for wildlife camera networks.
 * Features include version management, rollback capabilities, and staged deployments.
 */
class OTAManager {
public:
    /**
     * @brief OTA update status codes
     */
    enum class UpdateStatus {
        IDLE = 0,
        CHECKING = 1,
        DOWNLOADING = 2,
        INSTALLING = 3,
        COMPLETE = 4,
        FAILED = 5,
        ROLLBACK = 6
    };

    /**
     * @brief Firmware version information
     */
    struct FirmwareVersion {
        String version;
        String build_date;
        String checksum;
        uint32_t size_bytes;
        bool mandatory;
        String changelog;
        float compatibility_score;
    };

    /**
     * @brief OTA configuration settings
     */
    struct OTAConfig {
        String update_server_url = "https://updates.wildlifecam.org/firmware/";
        String device_type = "ESP32_WILDLIFE_CAM";
        String current_version = WILDLIFE_CAM_VERSION;
        uint32_t check_interval_hours = 24;
        bool auto_update_enabled = false;
        bool staged_deployment = true;
        uint8_t deployment_group = 0; // 0=beta, 1=stable, 2=critical
        float battery_threshold = 3.2; // Minimum battery for updates
        bool require_manual_approval = true;
    };

    /**
     * @brief Update progress callback function type
     */
    typedef void (*ProgressCallback)(uint8_t progress, const String& status);

    /**
     * @brief Initialize OTA manager
     * @param config OTA configuration settings
     * @return true if initialization successful
     */
    static bool initialize(const OTAConfig& config = OTAConfig());

    /**
     * @brief Check for available firmware updates
     * @param force_check Force check even if within interval
     * @return true if updates available
     */
    static bool checkForUpdates(bool force_check = false);

    /**
     * @brief Download available firmware update
     * @param version_info Version to download (empty for latest)
     * @return true if download successful
     */
    static bool downloadUpdate(const FirmwareVersion& version_info = FirmwareVersion());

    /**
     * @brief Install downloaded firmware update
     * @param restart_after_install Automatically restart after installation
     * @return true if installation successful
     */
    static bool installUpdate(bool restart_after_install = true);

    /**
     * @brief Rollback to previous firmware version
     * @return true if rollback successful
     */
    static bool rollbackToPrevious();

    /**
     * @brief Get current update status
     * @return Current update status
     */
    static UpdateStatus getStatus() { return status_; }

    /**
     * @brief Get available firmware versions
     * @return Vector of available versions
     */
    static std::vector<FirmwareVersion> getAvailableVersions();

    /**
     * @brief Get current firmware version info
     * @return Current version information
     */
    static FirmwareVersion getCurrentVersion();

    /**
     * @brief Set progress callback for update operations
     * @param callback Function to call with progress updates
     */
    static void setProgressCallback(ProgressCallback callback);

    /**
     * @brief Process OTA manager tasks (call in main loop)
     */
    static void process();

    /**
     * @brief Validate firmware integrity
     * @param firmware_path Path to firmware file
     * @param expected_checksum Expected SHA256 checksum
     * @return true if firmware is valid
     */
    static bool validateFirmware(const String& firmware_path, const String& expected_checksum);

    /**
     * @brief Get update statistics
     * @return JSON object with update statistics
     */
    static DynamicJsonDocument getUpdateStatistics();

    /**
     * @brief Enable/disable automatic updates
     * @param enabled Enable automatic updates
     */
    static void setAutoUpdateEnabled(bool enabled);

    /**
     * @brief Set deployment group for staged updates
     * @param group Deployment group (0=beta, 1=stable, 2=critical)
     */
    static void setDeploymentGroup(uint8_t group);

    /**
     * @brief Schedule update for specific time
     * @param timestamp Unix timestamp for scheduled update
     * @return true if scheduled successfully
     */
    static bool scheduleUpdate(uint32_t timestamp);

    /**
     * @brief Cancel scheduled update
     */
    static void cancelScheduledUpdate();

private:
    static OTAConfig config_;
    static UpdateStatus status_;
    static ProgressCallback progress_callback_;
    static FirmwareVersion pending_version_;
    static uint32_t last_check_time_;
    static uint32_t scheduled_update_time_;
    static bool initialized_;

    // Internal helper methods
    static bool connectToUpdateServer();
    static String generateDeviceFingerprint();
    static bool verifyUpdateSignature(const String& firmware_data, const String& signature);
    static void updateProgress(uint8_t progress, const String& status);
    static bool createBackup();
    static bool restoreBackup();
    static void saveUpdateState();
    static void loadUpdateState();
    static bool checkBatteryLevel();
    static bool checkStorageSpace(uint32_t required_bytes);
};

#endif // OTA_MANAGER_H