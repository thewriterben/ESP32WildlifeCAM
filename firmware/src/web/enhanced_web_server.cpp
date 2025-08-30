/**
 * @file enhanced_web_server.cpp
 * @brief Enhanced Web Server Implementation
 * 
 * Implementation of the enhanced web server with WebSocket support
 * and real-time dashboard functionality.
 */

#include "enhanced_web_server.h"
#include <esp_log.h>

// Mobile API integration
#include "../../mobile_app/firmware/src/mobile/mobile_api.h"
#include "../../mobile_app/firmware/src/mobile/mobile_websocket.h"

static const char* TAG = "EnhancedWebServer";

// Global instance
EnhancedWebServer* g_enhancedWebServer = nullptr;

EnhancedWebServer::EnhancedWebServer() : 
    server_(80), 
    webSocket_("/ws"),
    running_(false),
    lastSystemUpdate_(0),
    lastHeartbeat_(0) {
}

EnhancedWebServer::~EnhancedWebServer() {
    end();
}

bool EnhancedWebServer::begin(const EnhancedWebConfig& config) {
    if (running_) {
        ESP_LOGW(TAG, "Server already running");
        return true;
    }
    
    config_ = config;
    server_ = AsyncWebServer(config_.port);
    
    // Setup components
    setupWebSocket();
    setupAPIEndpoints();
    setupStaticFiles();
    setupRoutes();
    
    if (config_.enableCORS) {
        setupCORS();
    }
    
    // Initialize metrics
    updateSystemMetrics();
    
    // Start server
    server_.begin();
    running_ = true;
    
    ESP_LOGI(TAG, "Enhanced web server started on port %d", config_.port);
    ESP_LOGI(TAG, "Dashboard available at: http://%s%s", 
             WiFi.localIP().toString().c_str(), config_.dashboardPath.c_str());
    ESP_LOGI(TAG, "WebSocket endpoint: ws://%s%s", 
             WiFi.localIP().toString().c_str(), config_.wsPath.c_str());
    
    return true;
}

void EnhancedWebServer::end() {
    if (!running_) return;
    
    server_.end();
    running_ = false;
    
    ESP_LOGI(TAG, "Enhanced web server stopped");
}

void EnhancedWebServer::setupWebSocket() {
    webSocket_.onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, 
                             AwsEventType type, void* arg, uint8_t* data, size_t len) {
        this->onWSEvent(server, client, type, arg, data, len);
    });
    
    server_.addHandler(&webSocket_);
    ESP_LOGI(TAG, "WebSocket handler configured");
}

void EnhancedWebServer::setupAPIEndpoints() {
    // System status and metrics
    server_.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIStatus(request);
    });
    
    server_.on("/api/metrics", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPISystemMetrics(request);
    });
    
    // Mobile-optimized API endpoints
    server_.on("/api/mobile/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileStatus(request);
        } else {
            this->handleAPIStatus(request);
        }
    });
    
    server_.on("/api/mobile/capture", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileCapture(request);
        } else {
            this->handleAPICapture(request);
        }
    });
    
    server_.on("/api/mobile/preview", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobilePreview(request);
        } else {
            request->send(501, "application/json", "{\"error\":\"Mobile preview not available\"}");
        }
    });
    
    server_.on("/api/mobile/settings", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileSettings(request);
        } else {
            this->handleAPIConfig(request);
        }
    });
    
    server_.on("/api/mobile/settings", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileSettings(request);
        } else {
            this->handleAPIConfigUpdate(request);
        }
    });
    
    server_.on("/api/mobile/notifications", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileNotifications(request);
        } else {
            request->send(200, "application/json", "{\"notifications_enabled\":true}");
        }
    });
    
    server_.on("/api/mobile/notifications", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileNotifications(request);
        } else {
            request->send(200, "application/json", "{\"success\":true}");
        }
    });
    
    server_.on("/api/mobile/images", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileImageList(request);
        } else {
            this->handleAPIImageList(request);
        }
    });
    
    server_.on("/api/mobile/thumbnail", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileThumbnail(request);
        } else {
            this->handleAPIImageThumbnail(request);
        }
    });
    
    server_.on("/api/mobile/burst", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileBurstMode(request);
        } else {
            request->send(501, "application/json", "{\"error\":\"Burst mode not available\"}");
        }
    });
    
    server_.on("/api/mobile/burst", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileBurstMode(request);
        } else {
            request->send(501, "application/json", "{\"error\":\"Burst mode not available\"}");
        }
    });
    
    // Image management
    server_.on("/api/images", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIImageList(request);
    });
    
    server_.on("/api/thumbnail", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIImageThumbnail(request);
    });
    
    // Camera control
    server_.on("/api/capture", HTTP_POST, [this](AsyncWebServerRequest* request) {
        this->handleAPICapture(request);
    });
    
    server_.on("/api/stream", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIStream(request);
    });
    
    // Configuration
    server_.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIConfig(request);
    });
    
    server_.on("/api/config", HTTP_POST, [this](AsyncWebServerRequest* request) {
        this->handleAPIConfigUpdate(request);
    });
    
    // Storage and power stats
    server_.on("/api/storage", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIStorageStats(request);
    });
    
    server_.on("/api/power", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIPowerStats(request);
    });
    
    // Wildlife detection log
    server_.on("/api/wildlife", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIWildlifeLog(request);
    });
    
    ESP_LOGI(TAG, "API endpoints configured");
}

void EnhancedWebServer::setupStaticFiles() {
    // Serve dashboard assets
    server_.serveStatic("/assets/", SD_MMC, "/web/assets/");
    
    ESP_LOGI(TAG, "Static file serving configured");
}

void EnhancedWebServer::setupRoutes() {
    // Main dashboard
    server_.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleDashboard(request);
    });
    
    // Catch-all for SPA routing
    server_.onNotFound([this](AsyncWebServerRequest* request) {
        if (request->url().startsWith("/api/")) {
            request->send(404, "application/json", "{\"error\":\"API endpoint not found\"}");
        } else {
            // Serve dashboard for client-side routing
            this->handleDashboard(request);
        }
    });
    
    ESP_LOGI(TAG, "Routes configured");
}

void EnhancedWebServer::setupCORS() {
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    
    ESP_LOGI(TAG, "CORS headers configured");
}

void EnhancedWebServer::onWSEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                                 AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            ESP_LOGI(TAG, "WebSocket client connected: %u", client->id());
            // Send initial system status
            broadcastSystemStatus();
            break;
            
        case WS_EVT_DISCONNECT:
            ESP_LOGI(TAG, "WebSocket client disconnected: %u", client->id());
            break;
            
        case WS_EVT_DATA: {
            String message = String((char*)data, len);
            handleWSMessage(client, message);
            break;
        }
        
        case WS_EVT_ERROR:
            ESP_LOGW(TAG, "WebSocket error for client %u", client->id());
            break;
            
        default:
            break;
    }
}

void EnhancedWebServer::handleWSMessage(AsyncWebSocketClient* client, const String& message) {
    DynamicJsonDocument doc(512);
    
    if (deserializeJson(doc, message) != DeserializationError::Ok) {
        ESP_LOGW(TAG, "Invalid JSON in WebSocket message");
        return;
    }
    
    String action = doc["action"];
    
    if (action == "ping") {
        // Respond to ping with pong
        DynamicJsonDocument response(128);
        response["type"] = "pong";
        response["timestamp"] = millis();
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
    } else if (action == "request_update") {
        // Client requesting immediate update
        updateSystemMetrics();
        broadcastSystemStatus();
    }
}

void EnhancedWebServer::sendWSMessage(WSMessageType type, const JsonDocument& data) {
    DynamicJsonDocument message(1024);
    message["type"] = static_cast<int>(type);
    message["timestamp"] = millis();
    message["data"] = data;
    
    String messageStr;
    serializeJson(message, messageStr);
    
    webSocket_.textAll(messageStr);
}

void EnhancedWebServer::sendHeartbeat() {
    if (millis() - lastHeartbeat_ < config_.wsHeartbeatInterval) {
        return;
    }
    
    DynamicJsonDocument heartbeat(128);
    heartbeat["uptime"] = millis();
    heartbeat["clients"] = webSocket_.count();
    
    sendWSMessage(WSMessageType::HEARTBEAT, heartbeat);
    lastHeartbeat_ = millis();
}

void EnhancedWebServer::updateSystemMetrics() {
    updatePowerMetrics();
    updateStorageMetrics();
    updateNetworkMetrics();
    updateCameraMetrics();
    
    // System metrics
    metrics_.freeHeap = ESP.getFreeHeap();
    metrics_.totalHeap = ESP.getHeapSize();
    metrics_.uptime = millis();
    
    // Temperature (if available)
    #ifdef CONFIG_IDF_TARGET_ESP32S3
    metrics_.temperature = temperatureRead();
    #endif
    
    lastSystemUpdate_ = millis();
}

void EnhancedWebServer::updatePowerMetrics() {
    // Integration with existing power manager
    // This would need to be adapted based on actual PowerManager implementation
    PowerSystemStatus powerStatus;
    
    // Mock implementation - replace with actual power manager calls
    metrics_.batteryVoltage = 3.8; // powerManager.getBatteryVoltage();
    metrics_.solarVoltage = 4.2;   // powerManager.getSolarVoltage();
    metrics_.batteryPercentage = 85; // powerManager.getBatteryPercentage();
    metrics_.isCharging = true;    // powerManager.isCharging();
    metrics_.powerState = POWER_NORMAL; // powerManager.getPowerState();
}

void EnhancedWebServer::updateStorageMetrics() {
    // Integration with existing storage manager
    #ifdef SD_MMC_SUPPORTED
    if (SD_MMC.cardType() != CARD_NONE) {
        metrics_.totalStorage = SD_MMC.cardSize();
        metrics_.usedStorage = SD_MMC.usedBytes();
        metrics_.freeStorage = metrics_.totalStorage - metrics_.usedStorage;
        
        // Count image files
        File root = SD_MMC.open("/");
        uint32_t imageCount = 0;
        while (File file = root.openNextFile()) {
            String filename = file.name();
            if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
                imageCount++;
            }
        }
        metrics_.imageCount = imageCount;
    }
    #endif
}

void EnhancedWebServer::updateNetworkMetrics() {
    metrics_.wifiConnected = WiFi.status() == WL_CONNECTED;
    if (metrics_.wifiConnected) {
        metrics_.wifiSignalStrength = WiFi.RSSI();
        metrics_.ipAddress = WiFi.localIP().toString();
    }
}

void EnhancedWebServer::updateCameraMetrics() {
    // Mock implementation - replace with actual camera status
    metrics_.cameraReady = true;
    metrics_.monitoringActive = true;
    metrics_.dailyCaptures = 12;
    metrics_.totalCaptures = 156;
    metrics_.lastDetectedSpecies = "deer";
    metrics_.lastDetectionConfidence = 0.92;
}

// Real-time broadcasting methods
void EnhancedWebServer::broadcastSystemStatus() {
    DynamicJsonDocument status(1024);
    
    // Power data
    status["battery"]["voltage"] = metrics_.batteryVoltage;
    status["battery"]["percentage"] = metrics_.batteryPercentage;
    status["battery"]["charging"] = metrics_.isCharging;
    status["solar"]["voltage"] = metrics_.solarVoltage;
    status["power"]["state"] = static_cast<int>(metrics_.powerState);
    
    // Memory and system
    status["memory"]["free"] = metrics_.freeHeap;
    status["memory"]["total"] = metrics_.totalHeap;
    status["system"]["uptime"] = metrics_.uptime;
    status["system"]["temperature"] = metrics_.temperature;
    
    // Storage
    status["storage"]["total"] = metrics_.totalStorage;
    status["storage"]["used"] = metrics_.usedStorage;
    status["storage"]["free"] = metrics_.freeStorage;
    status["storage"]["images"] = metrics_.imageCount;
    
    // Network
    status["network"]["connected"] = metrics_.wifiConnected;
    status["network"]["signal"] = metrics_.wifiSignalStrength;
    status["network"]["ip"] = metrics_.ipAddress;
    
    // Camera and AI
    status["camera"]["ready"] = metrics_.cameraReady;
    status["camera"]["monitoring"] = metrics_.monitoringActive;
    status["captures"]["daily"] = metrics_.dailyCaptures;
    status["captures"]["total"] = metrics_.totalCaptures;
    status["wildlife"]["lastSpecies"] = metrics_.lastDetectedSpecies;
    status["wildlife"]["lastConfidence"] = metrics_.lastDetectionConfidence;
    
    sendWSMessage(WSMessageType::SYSTEM_STATUS, status);
}

void EnhancedWebServer::broadcastWildlifeDetection(const String& species, float confidence, const String& imagePath) {
    DynamicJsonDocument detection(512);
    detection["species"] = species;
    detection["confidence"] = confidence;
    detection["imagePath"] = imagePath;
    detection["timestamp"] = millis();
    
    sendWSMessage(WSMessageType::WILDLIFE_DETECTION, detection);
    
    // Update metrics
    metrics_.lastDetectedSpecies = species;
    metrics_.lastDetectionConfidence = confidence;
    metrics_.totalCaptures++;
    
    logMessage("Wildlife detected: " + species + " (confidence: " + String(confidence) + ")");
}

void EnhancedWebServer::broadcastPowerUpdate() {
    updatePowerMetrics();
    
    DynamicJsonDocument power(256);
    power["battery"]["voltage"] = metrics_.batteryVoltage;
    power["battery"]["percentage"] = metrics_.batteryPercentage;
    power["battery"]["charging"] = metrics_.isCharging;
    power["solar"]["voltage"] = metrics_.solarVoltage;
    power["state"] = static_cast<int>(metrics_.powerState);
    
    sendWSMessage(WSMessageType::POWER_UPDATE, power);
}

void EnhancedWebServer::broadcastStorageUpdate() {
    updateStorageMetrics();
    
    DynamicJsonDocument storage(256);
    storage["total"] = metrics_.totalStorage;
    storage["used"] = metrics_.usedStorage;
    storage["free"] = metrics_.freeStorage;
    storage["images"] = metrics_.imageCount;
    
    sendWSMessage(WSMessageType::STORAGE_UPDATE, storage);
}

void EnhancedWebServer::broadcastCameraStatus() {
    DynamicJsonDocument camera(256);
    camera["ready"] = metrics_.cameraReady;
    camera["monitoring"] = metrics_.monitoringActive;
    camera["dailyCaptures"] = metrics_.dailyCaptures;
    camera["totalCaptures"] = metrics_.totalCaptures;
    
    sendWSMessage(WSMessageType::CAMERA_STATUS, camera);
}

void EnhancedWebServer::broadcastErrorAlert(const String& error) {
    DynamicJsonDocument alert(256);
    alert["message"] = error;
    alert["severity"] = "error";
    
    sendWSMessage(WSMessageType::ERROR_ALERT, alert);
    
    logMessage("Error alert: " + error);
}

uint32_t EnhancedWebServer::getConnectedClients() const {
    return webSocket_.count();
}

void EnhancedWebServer::enableAuthentication(bool enable, const String& username, const String& password) {
    config_.enableAuth = enable;
    if (!username.isEmpty()) config_.username = username;
    if (!password.isEmpty()) config_.password = password;
}

bool EnhancedWebServer::authenticateRequest(AsyncWebServerRequest* request) {
    if (!config_.enableAuth) return true;
    
    // Simple basic auth check
    if (!request->authenticate(config_.username.c_str(), config_.password.c_str())) {
        return false;
    }
    
    return true;
}

void EnhancedWebServer::logMessage(const String& message) {
    ESP_LOGI(TAG, "%s", message.c_str());
    if (logCallback_) {
        logCallback_(message);
    }
}

// API Handler implementations
void EnhancedWebServer::handleDashboard(AsyncWebServerRequest* request) {
    // For now, serve a simple dashboard HTML
    // In a full implementation, this would load from SD card or embedded assets
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Wildlife Camera Dashboard</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<link rel='stylesheet' href='/assets/dashboard.css'>";
    html += "</head><body>";
    html += "<div id='dashboard'>Loading dashboard...</div>";
    html += "<script src='/assets/dashboard.js'></script>";
    html += "</body></html>";
    
    request->send(200, "text/html", html);
}

void EnhancedWebServer::handleAPIStatus(AsyncWebServerRequest* request) {
    request->send(200, "application/json", generateSystemStatusJSON());
}

void EnhancedWebServer::handleAPISystemMetrics(AsyncWebServerRequest* request) {
    updateSystemMetrics();
    request->send(200, "application/json", generateSystemStatusJSON());
}

void EnhancedWebServer::handleAPIImageList(AsyncWebServerRequest* request) {
    int page = request->hasParam("page") ? request->getParam("page")->value().toInt() : 0;
    int pageSize = request->hasParam("size") ? request->getParam("size")->value().toInt() : 20;
    
    request->send(200, "application/json", generateImageListJSON(page, pageSize));
}

void EnhancedWebServer::handleAPIImageThumbnail(AsyncWebServerRequest* request) {
    if (!request->hasParam("file")) {
        request->send(400, "application/json", "{\"error\":\"Missing file parameter\"}");
        return;
    }
    
    String filename = request->getParam("file")->value();
    
    // For now, just redirect to the full image
    // In a full implementation, this would generate/serve thumbnails
    request->redirect("/images/" + filename);
}

void EnhancedWebServer::handleAPICapture(AsyncWebServerRequest* request) {
    // Mock capture implementation
    DynamicJsonDocument response(256);
    response["success"] = true;
    response["filename"] = "wildlife_" + String(millis()) + ".jpg";
    response["timestamp"] = millis();
    
    String responseStr;
    serializeJson(response, responseStr);
    
    request->send(200, "application/json", responseStr);
    
    // Trigger updates
    metrics_.totalCaptures++;
    metrics_.dailyCaptures++;
    broadcastCameraStatus();
}

void EnhancedWebServer::handleAPIConfig(AsyncWebServerRequest* request) {
    request->send(200, "application/json", generateConfigJSON());
}

void EnhancedWebServer::handleAPIConfigUpdate(AsyncWebServerRequest* request) {
    request->send(200, "application/json", "{\"success\":true,\"message\":\"Configuration updated\"}");
}

void EnhancedWebServer::handleAPIStream(AsyncWebServerRequest* request) {
    request->send(501, "application/json", "{\"error\":\"Camera streaming not yet implemented\"}");
}

void EnhancedWebServer::handleAPIStorageStats(AsyncWebServerRequest* request) {
    request->send(200, "application/json", generateStorageStatsJSON());
}

void EnhancedWebServer::handleAPIPowerStats(AsyncWebServerRequest* request) {
    request->send(200, "application/json", generatePowerStatsJSON());
}

void EnhancedWebServer::handleAPIWildlifeLog(AsyncWebServerRequest* request) {
    int limit = request->hasParam("limit") ? request->getParam("limit")->value().toInt() : 50;
    request->send(200, "application/json", generateWildlifeLogJSON(limit));
}

// JSON generation methods
String EnhancedWebServer::generateSystemStatusJSON() {
    DynamicJsonDocument doc(1024);
    
    doc["battery"]["voltage"] = metrics_.batteryVoltage;
    doc["battery"]["percentage"] = metrics_.batteryPercentage;
    doc["battery"]["charging"] = metrics_.isCharging;
    doc["solar"]["voltage"] = metrics_.solarVoltage;
    doc["power"]["state"] = static_cast<int>(metrics_.powerState);
    
    doc["memory"]["free"] = metrics_.freeHeap;
    doc["memory"]["total"] = metrics_.totalHeap;
    doc["system"]["uptime"] = metrics_.uptime;
    doc["system"]["temperature"] = metrics_.temperature;
    
    doc["storage"]["total"] = metrics_.totalStorage;
    doc["storage"]["used"] = metrics_.usedStorage;
    doc["storage"]["free"] = metrics_.freeStorage;
    doc["storage"]["images"] = metrics_.imageCount;
    
    doc["network"]["connected"] = metrics_.wifiConnected;
    doc["network"]["signal"] = metrics_.wifiSignalStrength;
    doc["network"]["ip"] = metrics_.ipAddress;
    
    doc["camera"]["ready"] = metrics_.cameraReady;
    doc["camera"]["monitoring"] = metrics_.monitoringActive;
    doc["captures"]["daily"] = metrics_.dailyCaptures;
    doc["captures"]["total"] = metrics_.totalCaptures;
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateImageListJSON(int page, int pageSize) {
    DynamicJsonDocument doc(2048);
    JsonArray images = doc.createNestedArray("images");
    
    // Mock image list - in real implementation, scan SD card
    for (int i = 0; i < pageSize && i < 10; i++) {
        JsonObject img = images.createNestedObject();
        img["filename"] = "wildlife_" + String(millis() - i * 60000) + ".jpg";
        img["timestamp"] = millis() - i * 60000;
        img["size"] = 156789 + i * 1000;
        img["species"] = (i % 3 == 0) ? "deer" : (i % 3 == 1) ? "fox" : "unknown";
    }
    
    doc["page"] = page;
    doc["pageSize"] = pageSize;
    doc["total"] = metrics_.imageCount;
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateConfigJSON() {
    DynamicJsonDocument doc(512);
    
    doc["captureInterval"] = 30;
    doc["motionSensitivity"] = 75;
    doc["imageQuality"] = 80;
    doc["powerSaveMode"] = true;
    doc["nightMode"] = "auto";
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateStorageStatsJSON() {
    DynamicJsonDocument doc(256);
    
    doc["total"] = metrics_.totalStorage;
    doc["used"] = metrics_.usedStorage;
    doc["free"] = metrics_.freeStorage;
    doc["images"] = metrics_.imageCount;
    doc["usage_percent"] = metrics_.totalStorage > 0 ? 
        (metrics_.usedStorage * 100 / metrics_.totalStorage) : 0;
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generatePowerStatsJSON() {
    DynamicJsonDocument doc(256);
    
    doc["battery"]["voltage"] = metrics_.batteryVoltage;
    doc["battery"]["percentage"] = metrics_.batteryPercentage;
    doc["battery"]["charging"] = metrics_.isCharging;
    doc["solar"]["voltage"] = metrics_.solarVoltage;
    doc["state"] = static_cast<int>(metrics_.powerState);
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateWildlifeLogJSON(int limit) {
    DynamicJsonDocument doc(1024);
    JsonArray log = doc.createNestedArray("log");
    
    // Mock wildlife detection log
    for (int i = 0; i < limit && i < 10; i++) {
        JsonObject entry = log.createNestedObject();
        entry["timestamp"] = millis() - i * 3600000; // 1 hour intervals
        entry["species"] = (i % 4 == 0) ? "deer" : 
                          (i % 4 == 1) ? "fox" : 
                          (i % 4 == 2) ? "raccoon" : "unknown";
        entry["confidence"] = 0.7 + (i % 3) * 0.1;
        entry["filename"] = "wildlife_" + String(millis() - i * 3600000) + ".jpg";
    }
    
    doc["total"] = 10;
    doc["limit"] = limit;
    
    String result;
    serializeJson(doc, result);
    return result;
}

// Utility functions
bool startEnhancedWebServer(const EnhancedWebConfig& config) {
    if (g_enhancedWebServer == nullptr) {
        g_enhancedWebServer = new EnhancedWebServer();
    }
    
    return g_enhancedWebServer->begin(config);
}

void stopEnhancedWebServer() {
    if (g_enhancedWebServer != nullptr) {
        g_enhancedWebServer->end();
        delete g_enhancedWebServer;
        g_enhancedWebServer = nullptr;
    }
}

bool isEnhancedWebServerRunning() {
    return g_enhancedWebServer != nullptr && g_enhancedWebServer->isRunning();
}

void updateWebDashboard() {
    if (g_enhancedWebServer != nullptr) {
        g_enhancedWebServer->triggerSystemUpdate();
        g_enhancedWebServer->sendHeartbeat();
    }
}

void notifyWildlifeDetection(const String& species, float confidence, const String& imagePath) {
    if (g_enhancedWebServer != nullptr) {
        g_enhancedWebServer->broadcastWildlifeDetection(species, confidence, imagePath);
    }
}

void notifyPowerStateChange() {
    if (g_enhancedWebServer != nullptr) {
        g_enhancedWebServer->broadcastPowerUpdate();
    }
}

void notifyStorageChange() {
    if (g_enhancedWebServer != nullptr) {
        g_enhancedWebServer->broadcastStorageUpdate();
    }
}