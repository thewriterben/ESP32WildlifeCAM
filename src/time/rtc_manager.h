#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include <time.h>
#include "../include/config.h"

/**
 * @brief Real-Time Clock management for wildlife camera
 * 
 * Manages RTC functionality, NTP synchronization, timestamp generation,
 * and time-based operations for the wildlife monitoring system.
 */
class RTCManager {
public:
    /**
     * @brief Time synchronization status
     */
    enum class SyncStatus {
        NOT_SYNCHRONIZED,
        NTP_SYNCED,
        RTC_SYNCED,
        MANUAL_SET,
        SYNC_FAILED
    };

    /**
     * @brief Time configuration
     */
    struct TimeConfig {
        String ntpServer;
        int timezone;
        bool enableDST;
        uint32_t syncInterval;  // NTP sync interval in seconds
        bool useRTCBackup;
    };

    /**
     * @brief Time information structure
     */
    struct TimeInfo {
        uint32_t timestamp;     // Unix timestamp
        String isoString;       // ISO 8601 format
        String filename;        // Filename-safe format
        String readable;        // Human-readable format
        SyncStatus syncStatus;
        uint32_t lastSync;
    };

    /**
     * @brief Initialize RTC manager
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Set time configuration
     * @param config Time configuration
     */
    void setConfiguration(const TimeConfig& config);

    /**
     * @brief Synchronize time with NTP server
     * @return true if synchronization successful
     */
    bool synchronizeWithNTP();

    /**
     * @brief Set time manually
     * @param timestamp Unix timestamp
     * @return true if time set successfully
     */
    bool setTime(uint32_t timestamp);

    /**
     * @brief Get current timestamp
     * @return Unix timestamp
     */
    uint32_t getCurrentTimestamp();

    /**
     * @brief Get formatted timestamp for images
     * @return Filename-safe timestamp string
     */
    String getImageTimestamp();

    /**
     * @brief Get complete time information
     * @return Time information structure
     */
    TimeInfo getTimeInfo();

    /**
     * @brief Check if time is synchronized
     * @return true if time is synchronized
     */
    bool isSynchronized();

    /**
     * @brief Update RTC manager (call regularly)
     */
    void update();

    /**
     * @brief Schedule automatic NTP sync
     * @param intervalSeconds Sync interval in seconds
     */
    void scheduleNTPSync(uint32_t intervalSeconds = 3600);

    /**
     * @brief Get days since epoch
     * @return Number of days since Jan 1, 1970
     */
    uint32_t getDaysSinceEpoch();

    /**
     * @brief Check if it's daytime
     * @param sunriseHour Sunrise hour (0-23)
     * @param sunsetHour Sunset hour (0-23)
     * @return true if current time is between sunrise and sunset
     */
    bool isDaytime(uint8_t sunriseHour = 6, uint8_t sunsetHour = 18);

    /**
     * @brief Get time until next event
     * @param targetHour Target hour (0-23)
     * @param targetMinute Target minute (0-59)
     * @return Seconds until target time
     */
    uint32_t getSecondsUntil(uint8_t targetHour, uint8_t targetMinute = 0);

    /**
     * @brief Format timestamp to string
     * @param timestamp Unix timestamp
     * @param format Format string (strftime style)
     * @return Formatted time string
     */
    String formatTime(uint32_t timestamp, const String& format = "%Y-%m-%d %H:%M:%S");

    /**
     * @brief Get synchronization status
     * @return Current sync status
     */
    SyncStatus getSyncStatus() const { return syncStatus; }

    /**
     * @brief Check if initialized
     * @return true if initialized
     */
    bool isInitialized() const { return initialized; }

    /**
     * @brief Cleanup RTC manager
     */
    void cleanup();

private:
    bool initialized = false;
    SyncStatus syncStatus = SyncStatus::NOT_SYNCHRONIZED;
    TimeConfig config;
    
    uint32_t lastNTPSync = 0;
    uint32_t ntpSyncInterval = 3600; // 1 hour default
    uint32_t timezoneOffset = 0;
    
    /**
     * @brief Initialize default configuration
     */
    void initializeDefaultConfig();

    /**
     * @brief Convert timezone to offset seconds
     * @param timezone Timezone hours from UTC
     * @return Offset in seconds
     */
    int32_t getTimezoneOffset(int timezone);

    /**
     * @brief Check if NTP sync is due
     * @return true if sync should be performed
     */
    bool isNTPSyncDue();

    /**
     * @brief Perform NTP synchronization
     * @return true if successful
     */
    bool performNTPSync();
};

#endif // RTC_MANAGER_H