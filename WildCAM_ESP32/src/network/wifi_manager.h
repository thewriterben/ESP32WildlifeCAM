/**
 * @file wifi_manager.h
 * @brief WiFi connectivity management
 * @author thewriterben
 * @date 2025-08-31
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

/**
 * @brief WiFi management class for remote monitoring
 * 
 * Handles WiFi connection, reconnection, and basic networking
 * for the wildlife camera system.
 */
class WiFiManager {
public:
    /**
     * @brief WiFi connection status
     */
    enum Status {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        CONNECTION_FAILED,
        AP_MODE
    };

    /**
     * @brief WiFi configuration
     */
    struct Config {
        String ssid;
        String password;
        String hostname;
        bool enableAP;
        String apSSID;
        String apPassword;
        uint32_t connectTimeout;
        uint32_t reconnectInterval;
    };

    /**
     * @brief Initialize WiFi manager
     * @param config WiFi configuration
     * @return true if initialization successful
     */
    bool initialize(const Config& config);

    /**
     * @brief Update WiFi status (call regularly)
     */
    void update();

    /**
     * @brief Connect to WiFi network
     * @return true if connection initiated
     */
    bool connect();

    /**
     * @brief Disconnect from WiFi
     */
    void disconnect();

    /**
     * @brief Get current connection status
     * @return Current status
     */
    Status getStatus() const { return m_status; }

    /**
     * @brief Check if connected to WiFi
     * @return true if connected
     */
    bool isConnected() const { return m_status == CONNECTED; }

    /**
     * @brief Get local IP address
     * @return IP address as string
     */
    String getLocalIP() const;

    /**
     * @brief Get signal strength (RSSI)
     * @return RSSI in dBm
     */
    int32_t getRSSI() const;

    /**
     * @brief Enable or disable automatic reconnection
     * @param enable Enable auto-reconnect
     */
    void setAutoReconnect(bool enable) { m_autoReconnect = enable; }

    /**
     * @brief Get configuration
     * @return Current configuration
     */
    const Config& getConfig() const { return m_config; }

    /**
     * @brief Update configuration
     * @param config New configuration
     */
    void setConfig(const Config& config);

    /**
     * @brief Cleanup WiFi resources
     */
    void cleanup();

private:
    bool m_initialized = false;
    Status m_status = DISCONNECTED;
    Config m_config;
    
    uint32_t m_lastConnectAttempt = 0;
    uint32_t m_lastStatusCheck = 0;
    uint32_t m_connectStartTime = 0;
    bool m_autoReconnect = true;
    
    /**
     * @brief Attempt WiFi connection
     * @return true if connection started
     */
    bool attemptConnection();

    /**
     * @brief Start Access Point mode
     * @return true if AP started successfully
     */
    bool startAccessPoint();

    /**
     * @brief Update connection status
     */
    void updateStatus();

    /**
     * @brief Handle reconnection logic
     */
    void handleReconnection();
};

#endif // WIFI_MANAGER_H