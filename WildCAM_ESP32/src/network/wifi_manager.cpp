/**
 * @file wifi_manager.cpp
 * @brief Simple WiFi management implementation
 * @author thewriterben
 * @date 2025-09-03
 */

#include "wifi_manager.h"
#include "../utils/logger.h"

WiFiManager::WiFiManager() : connected(false), apMode(false), lastCheck(0) {
}

WiFiManager::~WiFiManager() {
    disconnect();
}

bool WiFiManager::connect(const char* ssid, const char* password, unsigned long timeout_ms) {
    Serial.printf("Connecting to WiFi: %s\n", ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout_ms) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        connected = true;
        apMode = false;
        printConnectionInfo();
        return true;
    } else {
        Serial.printf("WiFi connection failed. Status: %d\n", WiFi.status());
        return false;
    }
}

bool WiFiManager::startAccessPoint(const char* ap_ssid, const char* ap_password) {
    Serial.printf("Starting Access Point: %s\n", ap_ssid);
    
    WiFi.mode(WIFI_AP);
    
    bool result;
    if (strlen(ap_password) > 0) {
        result = WiFi.softAP(ap_ssid, ap_password);
    } else {
        result = WiFi.softAP(ap_ssid);
    }
    
    if (result) {
        apMode = true;
        connected = true; // Consider AP mode as "connected" for our purposes
        
        Serial.printf("Access Point started successfully\n");
        Serial.printf("AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
        Serial.printf("Connect to '%s' and visit http://%s\n", ap_ssid, WiFi.softAPIP().toString().c_str());
        
        return true;
    } else {
        Serial.println("Failed to start Access Point");
        return false;
    }
}

bool WiFiManager::isConnected() const {
    if (apMode) {
        return WiFi.softAPgetStationNum() > 0 || connected; // Return true if AP is running
    } else {
        return WiFi.status() == WL_CONNECTED;
    }
}

String WiFiManager::getIPAddress() const {
    if (apMode) {
        return WiFi.softAPIP().toString();
    } else {
        return WiFi.localIP().toString();
    }
}

int WiFiManager::getSignalStrength() const {
    if (apMode) {
        return 0; // Signal strength not applicable in AP mode
    } else {
        return WiFi.RSSI();
    }
}

void WiFiManager::disconnect() {
    if (connected) {
        WiFi.disconnect();
        connected = false;
        apMode = false;
        Serial.println("WiFi disconnected");
    }
}

void WiFiManager::update() {
    unsigned long now = millis();
    
    // Check connection status every 10 seconds
    if (now - lastCheck > 10000) {
        lastCheck = now;
        
        if (!apMode) {
            // Check station mode connection
            bool currentlyConnected = (WiFi.status() == WL_CONNECTED);
            if (connected != currentlyConnected) {
                connected = currentlyConnected;
                if (connected) {
                    Serial.println("WiFi reconnected");
                    printConnectionInfo();
                } else {
                    Serial.println("WiFi connection lost");
                }
            }
        }
    }
}

void WiFiManager::printConnectionInfo() {
    if (apMode) {
        Serial.println("=== WiFi Access Point Info ===");
        Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
        Serial.printf("Connected stations: %d\n", WiFi.softAPgetStationNum());
    } else {
        Serial.println("=== WiFi Connection Info ===");
        Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
        Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());
        Serial.printf("Signal: %d dBm\n", WiFi.RSSI());
        Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
    }
    Serial.println("===============================");
}