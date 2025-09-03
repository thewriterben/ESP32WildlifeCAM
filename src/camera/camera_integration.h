/**
 * @file camera_integration.h
 * @brief Minimal camera integration for SystemManager
 * @author thewriterben
 * @date 2025-09-03
 */

#ifndef CAMERA_INTEGRATION_H
#define CAMERA_INTEGRATION_H

#include <Arduino.h>
#include <esp_camera.h>

/**
 * @brief Minimal camera configuration for AI-Thinker ESP32-CAM
 */
class CameraIntegration {
public:
    CameraIntegration();
    ~CameraIntegration();
    
    /**
     * @brief Initialize camera with AI-Thinker ESP32-CAM default configuration
     * @return true if successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Check if camera is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized; }
    
    /**
     * @brief Capture a single image
     * @return Camera frame buffer pointer, or nullptr on failure
     */
    camera_fb_t* captureImage();
    
    /**
     * @brief Release camera frame buffer
     * @param fb Frame buffer to release
     */
    void releaseFrameBuffer(camera_fb_t* fb);
    
    /**
     * @brief Get camera configuration
     * @return Camera configuration structure
     */
    camera_config_t getCameraConfig() const { return camera_config; }

private:
    bool initialized;
    camera_config_t camera_config;
    
    /**
     * @brief Configure camera for AI-Thinker ESP32-CAM
     */
    void configureAIThinkerESP32CAM();
    
    /**
     * @brief Configure sensor settings for wildlife photography
     */
    void configureSensorSettings();
};

#endif // CAMERA_INTEGRATION_H