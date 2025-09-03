/**
 * @file web_server.cpp
 * @brief Simple web server implementation for wildlife camera
 * @author thewriterben
 * @date 2025-09-03
 */

#include "web_server.h"
#include "../core/system_manager.h"
#include "../utils/logger.h"
#include <SD_MMC.h>

WildlifeCameraWebServer::WildlifeCameraWebServer(SystemManager* systemMgr, int port) 
    : systemManager(systemMgr), server(nullptr), serverPort(port), running(false) {
}

WildlifeCameraWebServer::~WildlifeCameraWebServer() {
    stop();
    if (server) {
        delete server;
        server = nullptr;
    }
}

bool WildlifeCameraWebServer::init() {
    if (server) {
        delete server;
    }
    
    server = new WebServer(serverPort);
    
    // Set up route handlers
    server->on("/", [this]() { handleRoot(); });
    server->on("/capture", [this]() { handleCapture(); });
    server->on("/stream", [this]() { handleStream(); });
    server->on("/status", [this]() { handleStatus(); });
    server->on("/images", [this]() { handleImages(); });
    server->on("/image", [this]() { handleImageFile(); });
    server->onNotFound([this]() { handleNotFound(); });
    
    Serial.println("Web server initialized");
    return true;
}

bool WildlifeCameraWebServer::start() {
    if (!server) {
        Serial.println("Server not initialized");
        return false;
    }
    
    server->begin();
    running = true;
    Serial.printf("Web server started on port %d\n", serverPort);
    Serial.printf("Access at: http://%s/\n", WiFi.localIP().toString().c_str());
    
    return true;
}

void WildlifeCameraWebServer::stop() {
    if (server && running) {
        server->stop();
        running = false;
        Serial.println("Web server stopped");
    }
}

void WildlifeCameraWebServer::handleClient() {
    if (server && running) {
        server->handleClient();
    }
}

void WildlifeCameraWebServer::handleRoot() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>ESP32 Wildlife Camera</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #2e7d32; color: white; }";
    html += ".container { max-width: 800px; margin: 0 auto; background-color: #4caf50; padding: 20px; border-radius: 10px; }";
    html += ".button { background-color: #1b5e20; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; margin: 5px; display: inline-block; }";
    html += ".button:hover { background-color: #2e7d32; }";
    html += ".status { background-color: #1b5e20; padding: 15px; border-radius: 5px; margin: 10px 0; }";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h1>🦌 ESP32 Wildlife Camera</h1>";
    html += "<div class='status'>";
    html += "<h3>System Status</h3>";
    html += "<p><strong>Status:</strong> " + String(systemManager ? "Online" : "Offline") + "</p>";
    html += "<p><strong>Camera:</strong> " + String(systemManager && systemManager->isCameraReady() ? "Ready" : "Not Ready") + "</p>";
    html += "<p><strong>Storage:</strong> " + String(systemManager && systemManager->isStorageReady() ? "Ready" : "Not Ready") + "</p>";
    html += "<p><strong>Free Heap:</strong> " + String(ESP.getFreeHeap()) + " bytes</p>";
    html += "<p><strong>Uptime:</strong> " + String(millis() / 1000) + " seconds</p>";
    html += "</div>";
    html += "<h3>Actions</h3>";
    html += "<a href='/capture' class='button'>📸 Capture Image</a>";
    html += "<a href='/images' class='button'>📁 View Images</a>";
    html += "<a href='/status' class='button'>📊 System Status</a>";
    html += "<a href='/stream' class='button'>📹 Live Stream</a>";
    html += "<h3>Wildlife Camera Features</h3>";
    html += "<ul>";
    html += "<li>Automatic motion detection and capture</li>";
    html += "<li>SD card storage with timestamp filenames</li>";
    html += "<li>Remote web access for image viewing</li>";
    html += "<li>System status monitoring</li>";
    html += "</ul>";
    html += "</div></body></html>";
    
    server->send(200, "text/html", html);
}

void WildlifeCameraWebServer::handleCapture() {
    if (!systemManager || !systemManager->isCameraReady()) {
        server->send(503, "text/plain", "Camera not available");
        return;
    }
    
    // Capture image
    camera_fb_t* fb = systemManager->captureImage();
    if (!fb) {
        server->send(500, "text/plain", "Failed to capture image");
        return;
    }
    
    // Save image if storage is available
    String savedPath = "";
    if (systemManager->isStorageReady()) {
        savedPath = systemManager->saveImage(fb);
    }
    
    // Send image to browser
    server->sendHeader("Content-Disposition", "inline; filename=capture.jpg");
    server->send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);
    
    // Clean up
    systemManager->releaseFrameBuffer(fb);
    
    if (!savedPath.isEmpty()) {
        Serial.printf("Image captured and saved: %s\n", savedPath.c_str());
    }
}

void WildlifeCameraWebServer::handleStream() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Wildlife Camera Stream</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 0; background-color: #000; color: white; text-align: center; }";
    html += "img { max-width: 100%; height: auto; border: 2px solid #4caf50; }";
    html += ".controls { padding: 20px; background-color: #2e7d32; }";
    html += ".button { background-color: #1b5e20; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; margin: 5px; }";
    html += "</style>";
    html += "<script>";
    html += "function refreshImage() { document.getElementById('stream').src = '/capture?' + new Date().getTime(); }";
    html += "setInterval(refreshImage, 2000);"; // Refresh every 2 seconds
    html += "</script></head><body>";
    html += "<div class='controls'>";
    html += "<h2>🦌 Wildlife Camera Live View</h2>";
    html += "<a href='/' class='button'>🏠 Home</a>";
    html += "<a href='javascript:refreshImage()' class='button'>🔄 Refresh</a>";
    html += "</div>";
    html += "<img id='stream' src='/capture' alt='Wildlife Camera Feed'>";
    html += "</body></html>";
    
    server->send(200, "text/html", html);
}

void WildlifeCameraWebServer::handleStatus() {
    server->send(200, "text/html", generateStatusPage());
}

void WildlifeCameraWebServer::handleImages() {
    server->send(200, "text/html", generateImagesPage());
}

void WildlifeCameraWebServer::handleImageFile() {
    if (!server->hasArg("name")) {
        server->send(400, "text/plain", "Missing image name parameter");
        return;
    }
    
    String imageName = server->arg("name");
    String imagePath = "/images/" + imageName;
    
    if (!SD_MMC.exists(imagePath.c_str())) {
        server->send(404, "text/plain", "Image not found");
        return;
    }
    
    File imageFile = SD_MMC.open(imagePath.c_str(), FILE_READ);
    if (!imageFile) {
        server->send(500, "text/plain", "Failed to open image file");
        return;
    }
    
    server->streamFile(imageFile, "image/jpeg");
    imageFile.close();
}

void WildlifeCameraWebServer::handleNotFound() {
    server->send(404, "text/plain", "Not Found");
}

String WildlifeCameraWebServer::generateStatusPage() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Wildlife Camera Status</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #2e7d32; color: white; }";
    html += ".container { max-width: 800px; margin: 0 auto; background-color: #4caf50; padding: 20px; border-radius: 10px; }";
    html += ".status-item { background-color: #1b5e20; padding: 10px; margin: 5px 0; border-radius: 5px; }";
    html += ".button { background-color: #1b5e20; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; margin: 5px; }";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h1>📊 System Status</h1>";
    
    // System information
    html += "<div class='status-item'>";
    html += "<h3>Hardware Status</h3>";
    html += "<p><strong>Camera:</strong> " + String(systemManager && systemManager->isCameraReady() ? "✅ Ready" : "❌ Not Ready") + "</p>";
    html += "<p><strong>Storage:</strong> " + String(systemManager && systemManager->isStorageReady() ? "✅ Ready" : "❌ Not Ready") + "</p>";
    html += "<p><strong>WiFi:</strong> " + String(WiFi.status() == WL_CONNECTED ? "✅ Connected" : "❌ Disconnected") + "</p>";
    html += "</div>";
    
    // Memory information
    html += "<div class='status-item'>";
    html += "<h3>Memory Status</h3>";
    html += "<p><strong>Free Heap:</strong> " + String(ESP.getFreeHeap()) + " bytes</p>";
    html += "<p><strong>Total Heap:</strong> " + String(ESP.getHeapSize()) + " bytes</p>";
    if (psramFound()) {
        html += "<p><strong>PSRAM:</strong> " + String(ESP.getPsramSize()) + " bytes</p>";
        html += "<p><strong>Free PSRAM:</strong> " + String(ESP.getFreePsram()) + " bytes</p>";
    }
    html += "</div>";
    
    // Storage information
    if (systemManager && systemManager->isStorageReady()) {
        uint64_t totalMB, usedMB, freeMB;
        if (systemManager->getStorageStats(totalMB, usedMB, freeMB)) {
            html += "<div class='status-item'>";
            html += "<h3>Storage Status</h3>";
            html += "<p><strong>Total:</strong> " + String((unsigned long)totalMB) + " MB</p>";
            html += "<p><strong>Used:</strong> " + String((unsigned long)usedMB) + " MB</p>";
            html += "<p><strong>Free:</strong> " + String((unsigned long)freeMB) + " MB</p>";
            html += "</div>";
        }
    }
    
    // System uptime
    html += "<div class='status-item'>";
    html += "<h3>System Information</h3>";
    html += "<p><strong>Uptime:</strong> " + String(millis() / 1000) + " seconds</p>";
    html += "<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>";
    html += "<p><strong>MAC Address:</strong> " + WiFi.macAddress() + "</p>";
    html += "</div>";
    
    html += "<a href='/' class='button'>🏠 Home</a>";
    html += "</div></body></html>";
    
    return html;
}

String WildlifeCameraWebServer::generateImagesPage() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Wildlife Camera Images</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #2e7d32; color: white; }";
    html += ".container { max-width: 1000px; margin: 0 auto; background-color: #4caf50; padding: 20px; border-radius: 10px; }";
    html += ".image-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(200px, 1fr)); gap: 10px; margin: 20px 0; }";
    html += ".image-item { background-color: #1b5e20; padding: 10px; border-radius: 5px; text-align: center; }";
    html += ".image-item img { max-width: 100%; height: auto; border-radius: 5px; cursor: pointer; }";
    html += ".button { background-color: #1b5e20; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; margin: 5px; }";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h1>📁 Wildlife Camera Images</h1>";
    
    if (!systemManager || !systemManager->isStorageReady()) {
        html += "<p>Storage not available</p>";
    } else {
        // List images from SD card
        File root = SD_MMC.open("/images");
        if (!root || !root.isDirectory()) {
            html += "<p>No images directory found</p>";
        } else {
            html += "<div class='image-grid'>";
            File file = root.openNextFile();
            int imageCount = 0;
            
            while (file && imageCount < 20) { // Limit to 20 images for performance
                if (!file.isDirectory()) {
                    String fileName = file.name();
                    if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg")) {
                        html += "<div class='image-item'>";
                        html += "<img src='/image?name=" + fileName + "' alt='" + fileName + "' onclick='window.open(this.src)'>";
                        html += "<p>" + fileName + "</p>";
                        html += "</div>";
                        imageCount++;
                    }
                }
                file = root.openNextFile();
            }
            html += "</div>";
            
            if (imageCount == 0) {
                html += "<p>No images found. Capture some images first!</p>";
            } else {
                html += "<p>Showing " + String(imageCount) + " images (click to view full size)</p>";
            }
        }
    }
    
    html += "<a href='/' class='button'>🏠 Home</a>";
    html += "<a href='/capture' class='button'>📸 Capture New Image</a>";
    html += "</div></body></html>";
    
    return html;
}

String WildlifeCameraWebServer::getContentType(const String& filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
    else if (filename.endsWith(".png")) return "image/png";
    else if (filename.endsWith(".gif")) return "image/gif";
    return "text/plain";
}