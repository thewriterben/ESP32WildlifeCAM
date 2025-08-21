#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include <Arduino.h>
#include <esp_camera.h>
#include "hal/camera_board.h"
#include <memory>

// Lighting condition enumeration
enum LightingCondition {
    LIGHTING_BRIGHT = 0,
    LIGHTING_NORMAL = 1,
    LIGHTING_DIM = 2,
    LIGHTING_DARK = 3
};

// Camera status structure
struct CameraStatus {
    bool initialized;
    bool sensorDetected;
    int imageCount;
    esp_err_t lastError;
    BoardType boardType;
    SensorType sensorType;
    const char* boardName;
    const char* sensorName;
};

namespace CameraHandler {
    /**
     * Initialize camera with optimal settings for wildlife photography
     * Uses HAL for automatic board detection and configuration
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * Initialize camera with specific board type
     * @param boardType Specific board type to use
     * @return true if initialization successful, false otherwise
     */
    bool init(BoardType boardType);
    
    /**
     * Get the current camera board instance
     * @return Pointer to camera board, or nullptr if not initialized
     */
    CameraBoard* getBoard();
    
    /**
     * Configure camera sensor settings optimized for wildlife detection
     * @param sensor Pointer to camera sensor
     */
    void configureSensorSettings(sensor_t* sensor);
    
    /**
     * Capture a single image
     * @return Camera frame buffer pointer, or nullptr on failure
     */
    camera_fb_t* captureImage();
    
    /**
     * Save image to SD card with timestamp and metadata
     * @param fb Camera frame buffer
     * @param folder Target folder path
     * @return Filename of saved image, or empty string on failure
     */
    String saveImage(camera_fb_t* fb, const char* folder);
    
    /**
     * Generate timestamped filename
     * @param folder Target folder path
     * @return Generated filename
     */
    String generateFilename(const char* folder);
    
    /**
     * Save image metadata as JSON
     * @param imageFilename Path to image file
     * @param fb Camera frame buffer
     */
    void saveImageMetadata(const String& imageFilename, camera_fb_t* fb);
    
    /**
     * Configure camera sensor settings optimized for wildlife detection
     * (Legacy function for backward compatibility)
     * @param sensor Pointer to camera sensor
     */
    void configureSensorSettings(sensor_t* sensor);
    
    /**
     * Get camera status information
     * @return Camera status structure
     */
    CameraStatus getStatus();
    
    /**
     * Take a test image and return basic info
     * @return true if test successful, false otherwise
     */
    bool testCamera();
    
    /**
     * Adjust camera settings for different lighting conditions
     * @param condition Lighting condition to optimize for
     */
    void adjustForLighting(LightingCondition condition);
    
    /**
     * Flash camera LED briefly
     */
    void flashLED();
    
    /**
     * Cleanup camera resources
     */
    void cleanup();
}

#endif // CAMERA_HANDLER_H