/**
 * @file web_server.h
 * @brief Simple web server for wildlife camera remote access
 * @author thewriterben
 * @date 2025-09-03
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_camera.h>

// Forward declarations
class SystemManager;

/**
 * @brief Simple web server for wildlife camera
 */
class WildlifeCameraWebServer {
public:
    WildlifeCameraWebServer(SystemManager* systemMgr, int port = 80);
    ~WildlifeCameraWebServer();
    
    /**
     * @brief Initialize web server
     * @return true if successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Start web server
     * @return true if successful, false otherwise
     */
    bool start();
    
    /**
     * @brief Stop web server
     */
    void stop();
    
    /**
     * @brief Handle client requests (call in loop)
     */
    void handleClient();
    
    /**
     * @brief Check if server is running
     * @return true if running, false otherwise
     */
    bool isRunning() const { return running; }

private:
    SystemManager* systemManager;
    WebServer* server;
    int serverPort;
    bool running;
    
    // Web page handlers
    void handleRoot();
    void handleCapture();
    void handleStream();
    void handleStatus();
    void handleImages();
    void handleImageFile();
    void handleNotFound();
    
    // Helper methods
    String generateStatusPage();
    String generateImagesPage();
    String getContentType(const String& filename);
    void sendCameraImage();
};

#endif // WEB_SERVER_H