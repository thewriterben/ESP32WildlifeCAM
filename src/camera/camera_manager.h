#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <Arduino.h>
#include <esp_camera.h>
#include "../include/config.h"
#include "../include/pins.h"

/**
 * @brief Camera management class for ESP32-CAM
 * 
 * Handles camera initialization, configuration, image capture,
 * and optimization for wildlife monitoring applications.
 */
class CameraManager {
public:
    /**
     * @brief Camera capture result structure
     */
    struct CaptureResult {
        bool success;
        size_t imageSize;
        String filename;
        uint32_t captureTime;
        camera_fb_t* frameBuffer;
    };

    /**
     * @brief Camera statistics structure
     */
    struct CameraStats {
        uint32_t totalCaptures;
        uint32_t successfulCaptures;
        uint32_t failedCaptures;
        uint32_t averageCaptureTime;
        size_t averageImageSize;
        float successRate;
    };

    /**
     * @brief Initialize camera system
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Capture image and save to SD card
     * @param folder Target folder path
     * @return Capture result with details
     */
    CaptureResult captureImage(const String& folder = IMAGE_FOLDER);

    /**
     * @brief Capture image to frame buffer (no save)
     * @return Frame buffer pointer (must be returned with returnFrameBuffer)
     */
    camera_fb_t* captureToBuffer();

    /**
     * @brief Return frame buffer to free memory
     * @param fb Frame buffer to return
     */
    void returnFrameBuffer(camera_fb_t* fb);

    /**
     * @brief Save frame buffer to file
     * @param fb Frame buffer to save
     * @param folder Target folder
     * @param filename Custom filename (optional)
     * @return Saved filename or empty string on failure
     */
    String saveFrameBuffer(camera_fb_t* fb, const String& folder, const String& filename = "");

    /**
     * @brief Configure camera settings for specific conditions
     * @param brightness Brightness adjustment (-2 to 2)
     * @param contrast Contrast adjustment (-2 to 2)
     * @param saturation Saturation adjustment (-2 to 2)
     */
    void configureSensor(int brightness = 0, int contrast = 0, int saturation = 0);

    /**
     * @brief Optimize camera settings for wildlife photography
     * @param lightLevel Current light level (0-100)
     * @param motionLevel Expected motion level (0-100)
     */
    void optimizeForWildlife(int lightLevel = 50, int motionLevel = 50);

    /**
     * @brief Enable/disable night mode optimizations
     * @param enable Enable night mode
     */
    void setNightMode(bool enable);

    /**
     * @brief Check if camera is initialized and ready
     * @return true if camera ready
     */
    bool isReady() const { return initialized; }

    /**
     * @brief Get camera statistics
     * @return Current camera statistics
     */
    CameraStats getStatistics() const { return stats; }

    /**
     * @brief Reset camera statistics
     */
    void resetStatistics();

    /**
     * @brief Get current camera configuration
     * @return Camera configuration string
     */
    String getConfiguration() const;

    /**
     * @brief Cleanup camera resources
     */
    void cleanup();

private:
    bool initialized = false;
    CameraStats stats = {};
    uint32_t captureCounter = 0;
    bool nightModeEnabled = false;

    /**
     * @brief Generate unique filename for image
     * @param folder Target folder
     * @param extension File extension
     * @return Generated filename
     */
    String generateFilename(const String& folder, const String& extension = ".jpg");

    /**
     * @brief Create directory if it doesn't exist
     * @param path Directory path
     * @return true if directory exists or was created
     */
    bool ensureDirectory(const String& path);

    /**
     * @brief Update capture statistics
     * @param success Whether capture was successful
     * @param captureTime Time taken for capture
     * @param imageSize Size of captured image
     */
    void updateStats(bool success, uint32_t captureTime, size_t imageSize);

    /**
     * @brief Configure camera pins for ESP32-CAM
     * @return Camera configuration structure
     */
    camera_config_t getCameraConfig();
};

#endif // CAMERA_MANAGER_H