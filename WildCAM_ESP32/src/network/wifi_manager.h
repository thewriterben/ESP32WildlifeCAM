/**
 * @file wifi_manager.h
 * @brief Simple WiFi management for wildlife camera
 * @author thewriterben
 * @date 2025-09-03
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

/**
 * @brief Simple WiFi manager for wildlife camera
 */
class WiFiManager {
public:
    WiFiManager();
    ~WiFiManager();
    
    /**
     * @brief Initialize WiFi connection
     * @param ssid WiFi network name
     * @param password WiFi password
     * @param timeout_ms Connection timeout in milliseconds
     * @return true if connected successfully, false otherwise
     */
    bool connect(const char* ssid, const char* password, unsigned long timeout_ms = 10000);
    
    /**
     * @brief Start WiFi Access Point mode
     * @param ap_ssid Access Point name
     * @param ap_password Access Point password (empty for open)
     * @return true if AP started successfully, false otherwise
     */
    bool startAccessPoint(const char* ap_ssid, const char* ap_password = "");
    
    /**
     * @brief Check if WiFi is connected
     * @return true if connected, false otherwise
     */
    bool isConnected() const;
    
    /**
     * @brief Get current IP address
     * @return IP address as string
     */
    String getIPAddress() const;
    
    /**
     * @brief Get WiFi signal strength
     * @return RSSI value in dBm
     */
    int getSignalStrength() const;
    
    /**
     * @brief Disconnect from WiFi
     */
    void disconnect();
    
    /**
     * @brief Update WiFi status (call in loop)
     */
    void update();

private:
    bool connected;
    bool apMode;
    unsigned long lastCheck;
    
    /**
     * @brief Print WiFi connection information
     */
    void printConnectionInfo();
};

#endif // WIFI_MANAGER_H