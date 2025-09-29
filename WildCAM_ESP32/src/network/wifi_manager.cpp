/**
 * @file wifi_manager.cpp
 * @brief WiFi connectivity management implementation
 * @author thewriterben
 * @date 2025-08-31
 */

#include "wifi_manager.h"
#include "../utils/logger.h"

bool WiFiManager::initialize(const Config& config) {
    if (m_initialized) {
        return true;
    }

    m_config = config;
    m_status = DISCONNECTED;

    // Configure WiFi mode
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(m_config.hostname.c_str());

    // Set auto-reconnect
    WiFi.setAutoReconnect(m_autoReconnect);

    Logger::info("WiFi Manager initialized");
    Logger::info("  Hostname: %s", m_config.hostname.c_str());
    Logger::info("  Auto-reconnect: %s", m_autoReconnect ? "enabled" : "disabled");

    m_initialized = true;
    return true;
}

void WiFiManager::update() {
    if (!m_initialized) {
        return;
    }

    uint32_t now = millis();

    // Update status every second
    if (now - m_lastStatusCheck >= 1000) {
        updateStatus();
        m_lastStatusCheck = now;
    }

    // Handle reconnection logic
    if (m_autoReconnect) {
        handleReconnection();
    }
}

bool WiFiManager::connect() {
    if (!m_initialized) {
        Logger::error("WiFi manager not initialized");
        return false;
    }

    if (m_config.ssid.isEmpty()) {
        Logger::error("No SSID configured");
        return false;
    }

    Logger::info("Connecting to WiFi: %s", m_config.ssid.c_str());
    
    return attemptConnection();
}

void WiFiManager::disconnect() {
    if (m_status == CONNECTED || m_status == CONNECTING) {
        WiFi.disconnect();
        m_status = DISCONNECTED;
        Logger::info("WiFi disconnected");
    }
}

String WiFiManager::getLocalIP() const {
    if (m_status == CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

int32_t WiFiManager::getRSSI() const {
    if (m_status == CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}

void WiFiManager::setConfig(const Config& config) {
    m_config = config;
    
    // If currently connected to different network, disconnect
    if (m_status == CONNECTED && WiFi.SSID() != config.ssid) {
        disconnect();
    }

    Logger::info("WiFi configuration updated");
}

void WiFiManager::cleanup() {
    if (m_initialized) {
        disconnect();
        WiFi.mode(WIFI_OFF);
        m_initialized = false;
        Logger::info("WiFi manager cleaned up");
    }
}

bool WiFiManager::attemptConnection() {
    if (m_status == CONNECTING) {
        return false; // Already attempting connection
    }

    m_status = CONNECTING;
    m_connectStartTime = millis();
    m_lastConnectAttempt = m_connectStartTime;

    // Begin WiFi connection
    WiFi.begin(m_config.ssid.c_str(), m_config.password.c_str());

    Logger::info("WiFi connection attempt started");
    return true;
}

bool WiFiManager::startAccessPoint() {
    if (m_config.apSSID.isEmpty()) {
        Logger::error("No AP SSID configured");
        return false;
    }

    WiFi.mode(WIFI_AP);
    
    bool success;
    if (m_config.apPassword.isEmpty()) {
        success = WiFi.softAP(m_config.apSSID.c_str());
    } else {
        success = WiFi.softAP(m_config.apSSID.c_str(), m_config.apPassword.c_str());
    }

    if (success) {
        m_status = AP_MODE;
        Logger::info("Access Point started: %s", m_config.apSSID.c_str());
        Logger::info("AP IP: %s", WiFi.softAPIP().toString().c_str());
        return true;
    } else {
        Logger::error("Failed to start Access Point");
        return false;
    }
}

void WiFiManager::updateStatus() {
    wl_status_t wifiStatus = WiFi.status();
    
    switch (wifiStatus) {
        case WL_CONNECTED:
            if (m_status != CONNECTED) {
                m_status = CONNECTED;
                uint32_t connectTime = millis() - m_connectStartTime;
                Logger::info("WiFi connected in %d ms", connectTime);
                Logger::info("  IP: %s", getLocalIP().c_str());
                Logger::info("  RSSI: %d dBm", getRSSI());
            }
            break;
            
        case WL_CONNECT_FAILED:
        case WL_WRONG_PASSWORD:
        case WL_NO_SSID_AVAIL:
            if (m_status == CONNECTING) {
                m_status = CONNECTION_FAILED;
                Logger::error("WiFi connection failed (status: %d)", wifiStatus);
            }
            break;
            
        case WL_DISCONNECTED:
            if (m_status == CONNECTED) {
                m_status = DISCONNECTED;
                Logger::warning("WiFi disconnected");
            } else if (m_status == CONNECTING) {
                // Check timeout
                uint32_t elapsed = millis() - m_connectStartTime;
                if (elapsed > m_config.connectTimeout) {
                    m_status = CONNECTION_FAILED;
                    Logger::error("WiFi connection timeout after %d ms", elapsed);
                }
            }
            break;
            
        default:
            // Keep current status for other states (IDLE_STATUS, etc.)
            break;
    }
}

void WiFiManager::handleReconnection() {
    if (!m_autoReconnect) {
        return;
    }

    uint32_t now = millis();
    
    // Attempt reconnection if disconnected or failed
    if ((m_status == DISCONNECTED || m_status == CONNECTION_FAILED) && 
        !m_config.ssid.isEmpty()) {
        
        // Check if enough time has passed since last attempt
        if (now - m_lastConnectAttempt >= m_config.reconnectInterval) {
            Logger::info("Attempting WiFi reconnection...");
            attemptConnection();
        }
    }
}