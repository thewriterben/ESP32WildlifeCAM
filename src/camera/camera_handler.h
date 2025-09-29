/**
 * @file camera_handler.h
 * @brief Main camera handler interface for ESP32WildlifeCAM Step 1.4
 * @author ESP32WildlifeCAM Project
 * @date 2025-08-31
 * 
 * This file provides the Camera Driver interface as specified in the
 * Unified Development Plan Step 1.4. It includes conflict-aware initialization
 * and integration with the GPIO manager and board detection system.
 */

#ifndef SRC_CAMERA_HANDLER_H
#define SRC_CAMERA_HANDLER_H

#include <Arduino.h>
#include <esp_camera.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "camera_config.h"

// Forward declarations for camera system
class CameraBoard;
struct GPIOMap;

// Forward declarations for AI integration
struct AIResult;
enum class ModelType;
class InferenceEngine;

// Forward declarations for power management
class PowerManager;
enum class PowerState;

/**
 * @class CameraHandler
 * @brief Critical Camera Driver implementation for Step 1.4
 * 
 * This class implements the camera driver system as outlined in the Unified
 * Development Plan. It provides conflict-aware initialization, PSRAM optimization,
 * and proper frame buffer management to prevent memory leaks.
 */
class CameraHandler {
private:
    camera_config_t config;
    QueueHandle_t frame_queue;
    bool initialized;
    CameraBoard* board_instance;
    
public:
    /**
     * @brief Initialize camera with default configuration (convenience method)
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Initialize camera with user configuration
     * @param user_config Camera configuration parameters
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initialize(const CameraConfig& user_config);
    
    /**
     * @brief Capture frame with timeout
     * @param timeout_ms Maximum time to wait for capture in milliseconds
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t captureFrame(uint32_t timeout_ms);
    
    /**
     * @brief Get frame buffer from queue
     * @return Pointer to frame buffer or nullptr if none available
     */
    camera_fb_t* getFrameBuffer();
    
    /**
     * @brief Return frame buffer to system
     * @param fb Frame buffer to return
     */
    void returnFrameBuffer(camera_fb_t* fb);
    
    /**
     * @brief Deinitialize camera and cleanup resources
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t deinitialize();
    
    /**
     * @brief Validate pin assignment for conflict-free operation
     * @return true if pin assignments are valid, false otherwise
     */
    bool validatePinAssignment();
    
    /**
     * @brief Initialize camera with conflict checking
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initializeWithConflictCheck();
    
    /**
     * @brief Check if camera is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized; }
    
    /**
     * @brief Get initialization result with diagnostic information
     * @return CameraInitResult structure with detailed status
     */
    CameraInitResult getInitializationResult() const;
    
    /**
     * @brief Get capture statistics
     * @return CaptureStats structure with performance metrics
     */
    CaptureStats getCaptureStats() const;
    
    /**
     * @brief Test camera functionality
     * @return true if camera test passed, false otherwise
     */
    bool testCamera();
    
    /**
     * @brief Capture and analyze frame using AI inference
     * @param model Model type to use for analysis
     * @return AI analysis result with species detection and confidence
     */
    AIResult captureAndAnalyze(ModelType model);
    
    /**
     * @brief Save captured image to storage
     * @param fb Frame buffer to save
     * @param folder Destination folder path
     * @return Filename of saved image or empty string on failure
     */
    String saveImage(camera_fb_t* fb, const char* folder = "/wildlife_images");
    
    /**
     * @brief Setup advanced frame buffer queue system
     * @return true if setup successful, false otherwise
     */
    bool setupFrameQueue();
    
    /**
     * @brief Optimize memory usage for PSRAM and heap
     */
    void optimizeMemoryUsage();
    
    /**
     * @brief Handle capture failure with recovery strategies
     */
    void handleCaptureFailure();
    
    /**
     * @brief Capture frame with power management awareness
     * @param timeout_ms Maximum time to wait for capture
     * @param power_aware Enable power-conscious settings
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t capturePowerAware(uint32_t timeout_ms = 5000, bool power_aware = true);
    
    /**
     * @brief Capture and analyze with power management integration
     * @param model Model type to use for analysis
     * @param power_level Current power level (0.0-1.0)
     * @return AI analysis result optimized for power consumption
     */
    AIResult captureAndAnalyzePowerAware(ModelType model, float power_level = 1.0f);
    
    /**
     * @brief Adapt camera settings based on power state
     * @param power_state Current power state from PowerManager
     */
    void adaptToPowerState(int power_state);
    
    /**
     * @brief Constructor
     */
    CameraHandler();
    
    /**
     * @brief Destructor
     */
    ~CameraHandler();
    
private:
    // Internal state tracking
    CaptureStats stats;
    CameraInitResult init_result;
    uint32_t last_capture_time;
    
    // Private methods
    esp_err_t setupPins();
    esp_err_t configureSensor();
    bool detectHardwareCapabilities();
    void updateCaptureStats(uint32_t capture_time, size_t image_size, bool success);
    void logDiagnosticInfo() const;
};

#endif // SRC_CAMERA_HANDLER_H