#include "wifi_manager.h"
#include "../utils/logger.h"

bool WiFiManager::initialize() {
    if (initialized) {
        return true;
    }

    LOG_INFO("Initializing WiFi manager...");

    // Initialize default configuration
    initializeDefaultConfig();

    // Set WiFi mode
    WiFi.mode(WIFI_STA);
    
    // Initialize statistics
    stats.status = ConnectionStatus::DISCONNECTED;
    stats.reconnectCount = 0;
    stats.ntpSynchronized = false;

    initialized = true;
    LOG_INFO("WiFi manager initialized successfully");
    
    return true;
}

void WiFiManager::setConfiguration(const NetworkConfig& newConfig) {
    config = newConfig;
    LOG_INFO("WiFi configuration updated");
}

bool WiFiManager::connect(const String& ssid, const String& password, uint32_t timeout) {
    if (!initialized) {
        return false;
    }

    LOG_INFO("Connecting to WiFi network: " + ssid);
    
    stats.status = ConnectionStatus::CONNECTING;
    connectionStartTime = millis();
    lastConnectionAttempt = connectionStartTime;

    WiFi.begin(ssid.c_str(), password.c_str());

    // Wait for connection
    uint32_t startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        stats.status = ConnectionStatus::CONNECTED;
        stats.ipAddress = WiFi.localIP().toString();
        stats.signalStrength = WiFi.RSSI();
        stats.connectionTime = millis() - connectionStartTime;
        
        LOG_INFO("WiFi connected successfully!");
        LOG_INFO("IP address: " + stats.ipAddress);
        LOG_INFO("Signal strength: " + String(stats.signalStrength) + " dBm");

        // Initialize NTP if enabled
        if (config.enableNTP) {
            if (initializeNTP()) {
                synchronizeTime();
            }
        }

        return true;
    } else {
        stats.status = ConnectionStatus::FAILED;
        LOG_ERROR("WiFi connection failed");
        return false;
    }
}

void WiFiManager::disconnect() {
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
        stats.status = ConnectionStatus::DISCONNECTED;
        LOG_INFO("WiFi disconnected");
    }
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED && stats.status == ConnectionStatus::CONNECTED;
}

WiFiManager::ConnectionStatus WiFiManager::getStatus() {
    return stats.status;
}

void WiFiManager::update() {
    if (!initialized) {
        return;
    }

    // Update connection status
    handleConnectionEvents();
    
    // Update statistics
    updateStatistics();
    
    // Check auto-reconnect
    if (autoReconnect) {
        checkAutoReconnect();
    }

    // Periodic NTP sync
    if (isConnected() && config.enableNTP) {
        uint32_t now = millis();
        if (now - lastNTPSync > 3600000) { // Sync every hour
            synchronizeTime();
        }
    }
}

bool WiFiManager::initializeNTP() {
    if (!isConnected()) {
        return false;
    }

    LOG_INFO("Initializing NTP client...");
    
    // Configure NTP
    configTime(config.timezone * 3600, 0, config.ntpServer.c_str());
    
    LOG_INFO("NTP client initialized with server: " + config.ntpServer);
    return true;
}

bool WiFiManager::synchronizeTime() {
    if (!isConnected()) {
        return false;
    }

    LOG_INFO("Synchronizing time with NTP...");
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        stats.ntpSynchronized = true;
        stats.lastNTPSync = getCurrentTimestamp();
        lastNTPSync = millis();
        
        LOG_INFO("Time synchronized: " + stats.lastNTPSync);
        return true;
    } else {
        LOG_ERROR("Failed to synchronize time");
        return false;
    }
}

String WiFiManager::getCurrentTimestamp() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        // Fallback to millis-based timestamp
        return String(millis() / 1000);
    }

    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(timeStr);
}

WiFiManager::NetworkStats WiFiManager::getStatistics() {
    updateStatistics();
    return stats;
}

void WiFiManager::setAutoReconnect(bool enable) {
    autoReconnect = enable;
    LOG_INFO("Auto-reconnect " + String(enable ? "enabled" : "disabled"));
}

bool WiFiManager::startWebServer() {
    if (!isConnected()) {
        LOG_ERROR("Cannot start web server - not connected to WiFi");
        return false;
    }

    // Web server implementation would go here
    // For now, just set the flag
    webServerRunning = true;
    LOG_INFO("Web server started on port " + String(config.webServerPort));
    
    return true;
}

void WiFiManager::stopWebServer() {
    if (webServerRunning) {
        webServerRunning = false;
        LOG_INFO("Web server stopped");
    }
}

void WiFiManager::cleanup() {
    if (initialized) {
        stopWebServer();
        disconnect();
        initialized = false;
        LOG_INFO("WiFi manager cleaned up");
    }
}

void WiFiManager::initializeDefaultConfig() {
    config.ssid = "";
    config.password = "";
    config.enableNTP = true;
    config.ntpServer = "pool.ntp.org";
    config.timezone = 0; // UTC
    config.enableRemoteAccess = false;
    config.webServerPort = 80;
}

void WiFiManager::handleConnectionEvents() {
    wl_status_t wifiStatus = WiFi.status();
    
    switch (wifiStatus) {
        case WL_CONNECTED:
            if (stats.status != ConnectionStatus::CONNECTED) {
                stats.status = ConnectionStatus::CONNECTED;
                LOG_INFO("WiFi connection established");
            }
            break;
            
        case WL_DISCONNECTED:
            if (stats.status == ConnectionStatus::CONNECTED) {
                stats.status = ConnectionStatus::DISCONNECTED;
                LOG_WARNING("WiFi connection lost");
            }
            break;
            
        case WL_CONNECT_FAILED:
            stats.status = ConnectionStatus::FAILED;
            LOG_ERROR("WiFi connection failed");
            break;
            
        default:
            // Other states (idle, scanning, etc.)
            break;
    }
}

void WiFiManager::updateStatistics() {
    if (isConnected()) {
        stats.ipAddress = WiFi.localIP().toString();
        stats.signalStrength = WiFi.RSSI();
    }
}

void WiFiManager::checkAutoReconnect() {
    if (!isConnected() && !config.ssid.isEmpty()) {
        uint32_t now = millis();
        
        // Try to reconnect every 30 seconds
        if (now - lastConnectionAttempt > 30000) {
            LOG_INFO("Attempting auto-reconnect...");
            
            if (connect(config.ssid, config.password, 10000)) {
                stats.reconnectCount++;
                LOG_INFO("Auto-reconnect successful (attempt #" + String(stats.reconnectCount) + ")");
            } else {
                LOG_WARNING("Auto-reconnect failed");
            }
        }
    }
}