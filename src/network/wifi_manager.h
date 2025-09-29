#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "../include/config.h"

/**
 * @brief WiFi and network management for wildlife camera
 * 
 * Handles WiFi connectivity, NTP synchronization, and remote monitoring
 * capabilities for the wildlife monitoring system.
 */
class WiFiManager {
public:
    /**
     * @brief Connection status
     */
    enum class ConnectionStatus {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        FAILED,
        TIMEOUT
    };

    /**
     * @brief Network configuration
     */
    struct NetworkConfig {
        String ssid;
        String password;
        bool enableNTP;
        String ntpServer;
        int timezone;
        bool enableRemoteAccess;
        uint16_t webServerPort;
    };

    /**
     * @brief Network statistics
     */
    struct NetworkStats {
        ConnectionStatus status;
        String ipAddress;
        int signalStrength;
        uint32_t connectionTime;
        uint32_t reconnectCount;
        bool ntpSynchronized;
        String lastNTPSync;
    };

    /**
     * @brief Initialize WiFi manager
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Configure network settings
     * @param config Network configuration
     */
    void setConfiguration(const NetworkConfig& config);

    /**
     * @brief Connect to WiFi network
     * @param ssid Network SSID
     * @param password Network password
     * @param timeout Connection timeout in milliseconds
     * @return true if connection successful
     */
    bool connect(const String& ssid, const String& password, uint32_t timeout = 30000);

    /**
     * @brief Disconnect from WiFi
     */
    void disconnect();

    /**
     * @brief Check if connected to WiFi
     * @return true if connected
     */
    bool isConnected();

    /**
     * @brief Get connection status
     * @return Current connection status
     */
    ConnectionStatus getStatus();

    /**
     * @brief Update WiFi manager (call regularly)
     */
    void update();

    /**
     * @brief Initialize NTP client for time synchronization
     * @return true if NTP initialization successful
     */
    bool initializeNTP();

    /**
     * @brief Synchronize time with NTP server
     * @return true if synchronization successful
     */
    bool synchronizeTime();

    /**
     * @brief Get current timestamp string
     * @return Formatted timestamp
     */
    String getCurrentTimestamp();

    /**
     * @brief Get network statistics
     * @return Network statistics structure
     */
    NetworkStats getStatistics();

    /**
     * @brief Enable or disable automatic reconnection
     * @param enable Enable auto-reconnect
     */
    void setAutoReconnect(bool enable);

    /**
     * @brief Start web server for remote access
     * @return true if server started successfully
     */
    bool startWebServer();

    /**
     * @brief Stop web server
     */
    void stopWebServer();

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
    bool autoReconnect = true;
    bool webServerRunning = false;
    NetworkConfig config;
    NetworkStats stats = {};
    
    uint32_t lastConnectionAttempt = 0;
    uint32_t lastNTPSync = 0;
    uint32_t connectionStartTime = 0;
    
    /**
     * @brief Initialize default configuration
     */
    void initializeDefaultConfig();

    /**
     * @brief Handle connection events
     */
    void handleConnectionEvents();

    /**
     * @brief Update network statistics
     */
    void updateStatistics();

    /**
     * @brief Check and handle auto-reconnection
     */
    void checkAutoReconnect();
};

#endif // WIFI_MANAGER_H