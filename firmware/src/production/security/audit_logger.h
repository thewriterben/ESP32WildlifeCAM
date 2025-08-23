#ifndef AUDIT_LOGGER_H
#define AUDIT_LOGGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <vector>
#include <map>
#include "../../config.h"

/**
 * @brief Comprehensive Audit Logging System for Security and Compliance
 * 
 * Provides detailed audit trails for all system activities, security events,
 * and conservation data access to meet regulatory compliance requirements.
 */
class AuditLogger {
public:
    /**
     * @brief Audit event types
     */
    enum class EventType {
        SYSTEM_STARTUP = 0,
        SYSTEM_SHUTDOWN = 1,
        USER_LOGIN = 2,
        USER_LOGOUT = 3,
        DATA_ACCESS = 4,
        DATA_MODIFICATION = 5,
        CONFIGURATION_CHANGE = 6,
        SECURITY_VIOLATION = 7,
        WILDLIFE_DETECTION = 8,
        ALERT_TRIGGERED = 9,
        NETWORK_EVENT = 10,
        FIRMWARE_UPDATE = 11,
        BACKUP_OPERATION = 12,
        MAINTENANCE_ACTION = 13,
        API_ACCESS = 14,
        FILE_OPERATION = 15
    };

    /**
     * @brief Audit event severity levels
     */
    enum class Severity {
        INFO = 0,
        WARNING = 1,
        ERROR = 2,
        CRITICAL = 3,
        SECURITY = 4
    };

    /**
     * @brief Audit log entry structure
     */
    struct AuditEntry {
        String entry_id;
        uint32_t timestamp;
        EventType event_type;
        Severity severity;
        String user_id;
        String session_id;
        String source_ip;
        String resource_accessed;
        String action_performed;
        String result_status;
        String details;
        String device_id;
        String location_id;
        uint32_t file_size_bytes;
        String checksum;
    };

    /**
     * @brief Audit configuration
     */
    struct AuditConfig {
        bool enabled;
        bool log_to_file;
        bool log_to_network;
        bool real_time_monitoring;
        uint32_t max_entries_per_file;
        uint32_t retention_days;
        bool encrypt_logs;
        bool compress_logs;
        String log_directory;
        String network_endpoint;
        std::vector<EventType> monitored_events;
        Severity minimum_severity;
    };

    /**
     * @brief Initialize audit logger
     * @param config Audit logging configuration
     * @return true if initialization successful
     */
    static bool initialize(const AuditConfig& config);

    /**
     * @brief Log audit event
     * @param event_type Type of event
     * @param severity Event severity
     * @param user_id User associated with event
     * @param action Action performed
     * @param resource Resource involved
     * @param result Result of action
     * @param details Additional details
     */
    static void logEvent(EventType event_type, Severity severity, 
                        const String& user_id, const String& action,
                        const String& resource, const String& result,
                        const String& details = "");

    /**
     * @brief Log security event with enhanced details
     * @param event_type Security event type
     * @param user_id User involved
     * @param source_ip Source IP address
     * @param action Action attempted
     * @param result Result of action
     * @param threat_level Threat level assessment
     */
    static void logSecurityEvent(EventType event_type, const String& user_id,
                                const String& source_ip, const String& action,
                                const String& result, Severity threat_level = Severity::SECURITY);

    /**
     * @brief Log wildlife detection event
     * @param species_detected Species detected
     * @param confidence_score Detection confidence
     * @param location Location of detection
     * @param image_path Path to detection image
     * @param conservation_status Conservation status of species
     */
    static void logWildlifeEvent(const String& species_detected, float confidence_score,
                                const String& location, const String& image_path,
                                const String& conservation_status);

    /**
     * @brief Process audit logger tasks (call in main loop)
     */
    static void process();

    /**
     * @brief Query audit log entries
     * @param start_timestamp Start time for query
     * @param end_timestamp End time for query
     * @param event_filter Filter by event type
     * @param user_filter Filter by user ID
     * @param severity_filter Filter by minimum severity
     * @return Vector of matching audit entries
     */
    static std::vector<AuditEntry> queryLog(uint32_t start_timestamp, uint32_t end_timestamp,
                                           EventType event_filter = static_cast<EventType>(-1),
                                           const String& user_filter = "",
                                           Severity severity_filter = Severity::INFO);

    /**
     * @brief Export audit logs for compliance reporting
     * @param format Export format ("json", "csv", "xml")
     * @param start_timestamp Start time for export
     * @param end_timestamp End time for export
     * @param include_sensitive Include sensitive data in export
     * @return Exported audit data
     */
    static String exportLogs(const String& format, uint32_t start_timestamp,
                            uint32_t end_timestamp, bool include_sensitive = false);

    /**
     * @brief Get audit statistics
     * @param days_back Number of days to include in statistics
     * @return JSON object with audit statistics
     */
    static DynamicJsonDocument getAuditStatistics(uint16_t days_back = 30);

    /**
     * @brief Verify audit log integrity
     * @param start_timestamp Start time for verification
     * @param end_timestamp End time for verification
     * @return true if audit log integrity is verified
     */
    static bool verifyLogIntegrity(uint32_t start_timestamp, uint32_t end_timestamp);

    /**
     * @brief Archive old audit logs
     * @param days_old Archive logs older than specified days
     * @return Number of entries archived
     */
    static uint32_t archiveOldLogs(uint16_t days_old);

    /**
     * @brief Set audit monitoring for specific events
     * @param event_types Vector of event types to monitor
     * @param minimum_severity Minimum severity level to log
     */
    static void setEventMonitoring(const std::vector<EventType>& event_types, Severity minimum_severity);

    /**
     * @brief Get current audit configuration
     * @return Current audit configuration
     */
    static AuditConfig getCurrentConfig() { return config_; }

    /**
     * @brief Enable/disable real-time audit monitoring
     * @param enabled Enable real-time monitoring
     */
    static void setRealTimeMonitoring(bool enabled);

private:
    static AuditConfig config_;
    static std::vector<AuditEntry> log_buffer_;
    static uint32_t current_log_file_entries_;
    static String current_log_file_path_;
    static uint32_t last_flush_time_;
    static bool initialized_;
    
    // Internal logging methods
    static void writeLogEntry(const AuditEntry& entry);
    static void flushLogBuffer();
    static bool rotateLogFile();
    static String generateEntryId();
    static String calculateEntryChecksum(const AuditEntry& entry);
    static void encryptLogEntry(AuditEntry& entry);
    static void sendToNetworkEndpoint(const AuditEntry& entry);
    
    // Utility methods
    static String eventTypeToString(EventType type);
    static String severityToString(Severity severity);
    static String formatLogEntry(const AuditEntry& entry, const String& format);
    static bool shouldLogEvent(EventType type, Severity severity);
    static void maintainRetentionPolicy();
    
    // Compliance methods
    static void generateComplianceReport(const String& report_type, uint32_t start_time, uint32_t end_time);
    static bool validateLogEntry(const AuditEntry& entry);
    static void detectAnomalousActivity();
};

#endif // AUDIT_LOGGER_H