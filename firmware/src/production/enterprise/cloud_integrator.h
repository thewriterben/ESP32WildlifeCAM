#ifndef CLOUD_INTEGRATOR_H
#define CLOUD_INTEGRATOR_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <queue>
#include "../../config.h"

/**
 * @brief Cloud Platform Integration for Enterprise Wildlife Monitoring
 * 
 * Provides seamless integration with cloud platforms for data synchronization,
 * analytics, and large-scale wildlife monitoring network management.
 */
class CloudIntegrator {
public:
    /**
     * @brief Supported cloud platforms
     */
    enum class CloudPlatform {
        AWS_IOT = 0,
        AZURE_IOT = 1,
        GOOGLE_CLOUD = 2,
        CUSTOM_API = 3,
        MULTIPLE = 4
    };

    /**
     * @brief Data synchronization modes
     */
    enum class SyncMode {
        REAL_TIME = 0,      // Immediate sync
        BATCHED = 1,        // Periodic batch sync
        SMART = 2,          // Adaptive based on conditions
        MANUAL = 3          // Manual sync only
    };

    /**
     * @brief Cloud connection status
     */
    enum class ConnectionStatus {
        DISCONNECTED = 0,
        CONNECTING = 1,
        CONNECTED = 2,
        SYNCING = 3,
        ERROR = 4,
        THROTTLED = 5
    };

    /**
     * @brief Data types for cloud synchronization
     */
    enum class DataType {
        SPECIES_DETECTION = 0,
        ENVIRONMENTAL_DATA = 1,
        SYSTEM_TELEMETRY = 2,
        IMAGES = 3,
        AUDIO = 4,
        CONFIGURATION = 5,
        ALERTS = 6,
        ANALYTICS = 7
    };

    /**
     * @brief Cloud platform configuration
     */
    struct CloudConfig {
        CloudPlatform platform;
        String endpoint_url;
        String device_id;
        String api_key;
        String secret_key;
        String region;
        String bucket_name;
        
        // Authentication
        String certificate_path;
        String private_key_path;
        bool use_ssl;
        uint16_t port;
        
        // Sync settings
        SyncMode sync_mode;
        uint32_t batch_interval_seconds;
        uint32_t max_batch_size;
        bool compress_data;
        bool encrypt_data;
        
        // Retry settings
        uint8_t max_retries;
        uint32_t retry_delay_ms;
        bool exponential_backoff;
    };

    /**
     * @brief Data synchronization item
     */
    struct SyncItem {
        String item_id;
        DataType data_type;
        String data_payload;
        uint32_t timestamp;
        uint8_t retry_count;
        uint32_t size_bytes;
        bool high_priority;
        String metadata;
    };

    /**
     * @brief Synchronization statistics
     */
    struct SyncStatistics {
        uint32_t total_items_synced;
        uint32_t successful_syncs;
        uint32_t failed_syncs;
        uint32_t pending_items;
        uint64_t total_bytes_synced;
        uint32_t last_sync_timestamp;
        float sync_success_rate;
        uint32_t average_sync_time_ms;
        ConnectionStatus current_status;
    };

    /**
     * @brief Cloud analytics response
     */
    struct AnalyticsResponse {
        bool success;
        String analysis_id;
        DynamicJsonDocument results;
        std::vector<String> insights;
        std::vector<String> recommendations;
        float confidence_score;
        uint32_t processing_time_ms;
    };

    /**
     * @brief Initialize cloud integrator
     * @param config Cloud platform configuration
     * @return true if initialization successful
     */
    static bool initialize(const CloudConfig& config);

    /**
     * @brief Connect to cloud platform
     * @param force_reconnect Force reconnection even if already connected
     * @return true if connection successful
     */
    static bool connect(bool force_reconnect = false);

    /**
     * @brief Disconnect from cloud platform
     */
    static void disconnect();

    /**
     * @brief Add data item to synchronization queue
     * @param data_type Type of data to sync
     * @param data_payload JSON data payload
     * @param high_priority Mark as high priority for immediate sync
     * @param metadata Additional metadata for the item
     * @return Unique item ID for tracking
     */
    static String queueForSync(DataType data_type, const String& data_payload, bool high_priority = false, const String& metadata = "");

    /**
     * @brief Synchronize all pending data with cloud
     * @param force_sync Force sync even if not in sync window
     * @return Number of items successfully synced
     */
    static uint32_t syncPendingData(bool force_sync = false);

    /**
     * @brief Upload image to cloud storage
     * @param image_data Pointer to image data
     * @param image_size Size of image data
     * @param filename Filename for cloud storage
     * @param metadata Image metadata
     * @return Cloud URL of uploaded image
     */
    static String uploadImage(const uint8_t* image_data, size_t image_size, const String& filename, const String& metadata = "");

    /**
     * @brief Download configuration from cloud
     * @param config_version Specific version to download (empty for latest)
     * @return Configuration JSON string
     */
    static String downloadConfiguration(const String& config_version = "");

    /**
     * @brief Send real-time alert to cloud
     * @param alert_type Type of alert
     * @param alert_data Alert data payload
     * @param urgency_level Urgency level (0-10)
     * @return true if alert sent successfully
     */
    static bool sendAlert(const String& alert_type, const String& alert_data, uint8_t urgency_level = 5);

    /**
     * @brief Request cloud analytics processing
     * @param analysis_type Type of analysis to perform
     * @param data_payload Data for analysis
     * @param parameters Analysis parameters
     * @return Analytics response with results
     */
    static AnalyticsResponse requestAnalytics(const String& analysis_type, const String& data_payload, const DynamicJsonDocument& parameters = DynamicJsonDocument(0));

    /**
     * @brief Get current connection status
     * @return Current cloud connection status
     */
    static ConnectionStatus getConnectionStatus() { return current_status_; }

    /**
     * @brief Get synchronization statistics
     * @return Current sync statistics
     */
    static SyncStatistics getSyncStatistics();

    /**
     * @brief Set synchronization mode
     * @param mode New synchronization mode
     * @param batch_interval Batch interval for batched mode (seconds)
     */
    static void setSyncMode(SyncMode mode, uint32_t batch_interval = 300);

    /**
     * @brief Enable/disable specific data type synchronization
     * @param data_type Data type to configure
     * @param enabled Enable/disable sync for this data type
     */
    static void setDataTypeSyncEnabled(DataType data_type, bool enabled);

    /**
     * @brief Process cloud integration tasks (call in main loop)
     */
    static void process();

    /**
     * @brief Get pending sync queue size
     * @return Number of items in sync queue
     */
    static uint32_t getPendingSyncCount();

    /**
     * @brief Clear sync queue
     * @param data_type Specific data type to clear (or all if not specified)
     */
    static void clearSyncQueue(DataType data_type = static_cast<DataType>(-1));

    /**
     * @brief Test cloud connectivity
     * @return true if cloud is reachable and responsive
     */
    static bool testConnectivity();

    /**
     * @brief Get cloud time synchronization
     * @return Current cloud timestamp
     */
    static uint32_t getCloudTimestamp();

    /**
     * @brief Register for cloud-based firmware updates
     * @param device_type Device type identifier
     * @param current_version Current firmware version
     * @return true if registration successful
     */
    static bool registerForUpdates(const String& device_type, const String& current_version);

    /**
     * @brief Subscribe to cloud notifications
     * @param notification_types Vector of notification types to subscribe to
     * @return true if subscription successful
     */
    static bool subscribeToNotifications(const std::vector<String>& notification_types);

    /**
     * @brief Export cloud integration logs
     * @param hours_back Hours of logs to export
     * @return JSON array with log entries
     */
    static DynamicJsonDocument exportLogs(uint16_t hours_back = 24);

private:
    static CloudConfig config_;
    static ConnectionStatus current_status_;
    static std::queue<SyncItem> sync_queue_;
    static SyncStatistics statistics_;
    static uint32_t last_sync_time_;
    static uint32_t last_connection_attempt_;
    static bool initialized_;
    static std::map<DataType, bool> data_type_enabled_;
    
    // Platform-specific methods
    static bool connectAWS();
    static bool connectAzure();
    static bool connectGoogleCloud();
    static bool connectCustomAPI();
    
    // Synchronization methods
    static bool syncSingleItem(const SyncItem& item);
    static bool syncBatchItems(const std::vector<SyncItem>& items);
    static String formatPayloadForPlatform(const SyncItem& item);
    static bool validateSyncResponse(const String& response);
    
    // Authentication and security
    static String generateAuthHeader();
    static bool loadCertificates();
    static String encryptPayload(const String& payload);
    static String compressPayload(const String& payload);
    
    // Utility methods
    static String dataTypeToString(DataType type);
    static String generateItemId();
    static void updateStatistics(bool success, uint32_t bytes_transferred, uint32_t duration_ms);
    static bool shouldRetrySync(const SyncItem& item);
    static uint32_t calculateRetryDelay(uint8_t retry_count);
    static void logCloudOperation(const String& operation, const String& details, bool success);
    
    // Error handling
    static void handleConnectionError(const String& error_message);
    static void handleSyncError(const SyncItem& item, const String& error_message);
    static bool isTemporaryError(int http_code);
};

#endif // CLOUD_INTEGRATOR_H