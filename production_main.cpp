/**
 * @file production_main.cpp
 * @brief Production-ready main loop for ESP32WildlifeCAM
 * @author thewriterben
 * @date 2025-01-01
 * @version 1.0.0
 * 
 * Production Wildlife Camera System - Core Functionality
 * 
 * This implements the essential motion → capture → storage workflow
 * for a production-ready wildlife monitoring system.
 */

#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include "esp_sleep.h"
#include "config.h"
#include "src/power/power_manager.h"
#include "src/data/storage_manager.h"
#include "src/detection/motion_detector.h"
#include "src/network/wifi_manager.h"
#include "src/system/system_monitor.h"
#include "src/utils/logger.h"

// Global component instances
PowerManager* g_powerManager = nullptr;
MotionDetector* g_motionDetector = nullptr;
WiFiManager* g_wifiManager = nullptr;
SystemMonitor* g_systemMonitor = nullptr;

// System state
bool g_systemInitialized = false;
uint32_t g_lastHealthCheck = 0;
uint32_t g_imageCounter = 0;
uint32_t g_motionEvents = 0;

/**
 * @brief Initialize camera system
 * @return true if initialization successful
 */
bool initializeCamera() {
    Logger::info("Initializing camera...");

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = DEFAULT_FRAMESIZE;
    config.jpeg_quality = DEFAULT_JPEG_QUALITY;
    config.fb_count = CAMERA_FB_COUNT;

    if (psramFound()) {
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Logger::error("Camera initialization failed with error: " + String(err));
        return false;
    }

    Logger::info("Camera initialized successfully");
    return true;
}

/**
 * @brief Capture and save wildlife image
 * @param motionResult Motion detection result for metadata
 * @return true if capture successful
 */
bool captureWildlifeImage(const MotionDetector::MotionResult& motionResult) {
    Logger::info("Capturing wildlife image...");

    // Take photo
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Logger::error("Failed to capture image");
        return false;
    }

    // Generate filename with timestamp
    String timestamp = String(millis() / 1000); // Simple timestamp
    String filename = "wildlife_" + timestamp + "_" + String(g_imageCounter) + ".jpg";

    // Save image
    bool saveSuccess = StorageManager::saveImage(fb, filename);
    
    if (saveSuccess) {
        // Create and save metadata
        StorageManager::ImageMetadata metadata;
        metadata.timestamp = timestamp;
        metadata.batteryLevel = g_powerManager->getBatteryPercentage();
        metadata.temperature = 25.0; // Would come from environmental sensor
        metadata.humidity = 60.0;    // Would come from environmental sensor
        metadata.aiDetection = "wildlife_detected";
        metadata.confidence = motionResult.confidence;
        metadata.location = "camera_station_001";

        StorageManager::saveMetadata(metadata);
        
        g_imageCounter++;
        Logger::info("Image captured and saved: " + filename);
        Logger::info("Image size: " + String(fb->len) + " bytes");
    }

    esp_camera_fb_return(fb);
    return saveSuccess;
}

/**
 * @brief Perform system health check
 */
void performHealthCheck() {
    uint32_t now = millis();
    if (now - g_lastHealthCheck < 60000) { // Check every minute
        return;
    }

    Logger::info("=== System Health Check ===");
    
    // Power system health
    if (g_powerManager) {
        float batteryLevel = g_powerManager->getBatteryPercentage();
        Logger::info("Battery: " + String(batteryLevel, 1) + "%");
        
        if (batteryLevel < 20.0f) {
            Logger::warning("Low battery warning: " + String(batteryLevel, 1) + "%");
        }
        
        if (batteryLevel < 10.0f) {
            Logger::error("Critical battery level: " + String(batteryLevel, 1) + "%");
            // Could trigger emergency mode here
        }
    }

    // Storage health
    StorageManager::StorageStats stats = StorageManager::getStatistics();
    Logger::info("Storage: " + String(stats.usagePercentage, 1) + "% used");
    Logger::info("Images stored: " + String(stats.imageFiles));
    
    if (stats.usagePercentage > 85.0f) {
        Logger::warning("Storage nearly full: " + String(stats.usagePercentage, 1) + "%");
        StorageManager::cleanupOldFiles();
    }

    // Check SD card health
    if (!StorageManager::isSDCardHealthy()) {
        Logger::error("SD card health check failed");
    }

    // Motion detection statistics
    Logger::info("Motion events: " + String(g_motionEvents));
    Logger::info("Images captured: " + String(g_imageCounter));
    
    // Memory statistics
    Logger::info("Free heap: " + String(ESP.getFreeHeap()) + " bytes");
    if (psramFound()) {
        Logger::info("Free PSRAM: " + String(ESP.getFreePsram()) + " bytes");
    }

    Logger::info("=== Health Check Complete ===");
    g_lastHealthCheck = now;
}

/**
 * @brief Production setup - initialize all core components
 */
void productionSetup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        ; // Wait for serial port to connect
    }

    Logger::info("===================================");
    Logger::info("ESP32WildlifeCAM Production System");
    Logger::info("Version: " + String(VERSION));
    Logger::info("Build: " __DATE__ " " __TIME__);
    Logger::info("===================================");

    // Initialize logger
    if (!Logger::initialize()) {
        Serial.println("Logger initialization failed");
    }

    // Initialize power management
    Logger::info("Initializing power management...");
    g_powerManager = new PowerManager();
    if (!g_powerManager->initialize()) {
        Logger::error("Power manager initialization failed");
        return;
    }
    
    // Initialize storage management
    Logger::info("Initializing storage management...");
    if (!StorageManager::initialize()) {
        Logger::error("Storage manager initialization failed");
        return;
    }

    // Initialize camera
    if (!initializeCamera()) {
        Logger::error("Camera initialization failed");
        return;
    }

    // Initialize motion detection
    Logger::info("Initializing motion detection...");
    g_motionDetector = new MotionDetector();
    if (!g_motionDetector->initialize()) {
        Logger::error("Motion detector initialization failed");
        return;
    }

    // Initialize system monitor
    Logger::info("Initializing system monitor...");
    g_systemMonitor = new SystemMonitor();
    if (g_systemMonitor->initialize()) {
        Logger::info("System monitor initialized");
    } else {
        Logger::warning("System monitor initialization failed");
    }

    // Initialize network management (optional)
    Logger::info("Initializing network management...");
    g_wifiManager = new WiFiManager();
    if (g_wifiManager->initialize()) {
        Logger::info("WiFi manager ready (not connected)");
    } else {
        Logger::warning("WiFi manager initialization failed");
    }

    // Configure motion detection for wildlife monitoring
    MotionDetector::MotionConfig motionConfig = g_motionDetector->getConfiguration();
    motionConfig.method = MotionDetector::DetectionMethod::PIR_OR_CAMERA;
    motionConfig.confidenceThreshold = 0.3f;
    motionConfig.cooldownPeriod = PIR_COOLDOWN_MS;
    motionConfig.enableFalsePositiveFilter = true;
    g_motionDetector->setConfiguration(motionConfig);

    // System initialization complete
    g_systemInitialized = true;
    Logger::info("Production system initialization complete!");
    Logger::info("Ready for wildlife monitoring...");

    // Initial health check
    performHealthCheck();
}

/**
 * @brief Production main loop - core wildlife monitoring workflow
 */
void productionLoop() {
    if (!g_systemInitialized) {
        delay(1000);
        return;
    }

    // Update all components
    if (g_powerManager) {
        g_powerManager->update();
    }
    
    if (g_motionDetector) {
        g_motionDetector->update();
    }

    if (g_wifiManager) {
        g_wifiManager->update();
    }

    if (g_systemMonitor) {
        g_systemMonitor->update();
    }

    // Main workflow: Motion Detection → Image Capture → Storage
    bool motionDetected = false;
    if (g_motionDetector) {
        motionDetected = g_motionDetector->detectMotion();
    }

    if (motionDetected) {
        g_motionEvents++;
        Logger::info("Motion detected! Event #" + String(g_motionEvents));
        
        // Get motion details
        MotionDetector::MotionResult motionResult = g_motionDetector->getMotionResult();
        Logger::info("Motion confidence: " + String(motionResult.confidence, 3));
        Logger::info("PIR triggered: " + String(motionResult.pirTriggered ? "Yes" : "No"));
        Logger::info("Camera motion: " + String(motionResult.cameraMotion ? "Yes" : "No"));

        // Capture wildlife image
        if (!captureWildlifeImage(motionResult)) {
            Logger::error("Failed to capture wildlife image");
        }

        // Optimize power consumption after activity
        if (g_powerManager) {
            g_powerManager->optimizePowerConsumption();
        }
    }

    // Perform periodic health checks
    performHealthCheck();

    // Check if system should enter deep sleep
    if (g_powerManager && g_powerManager->shouldEnterDeepSleep()) {
        Logger::info("Entering deep sleep to conserve power...");
        
        // Cleanup before sleep
        Logger::flush();
        
        // Enable motion interrupt for wake-up
        if (g_motionDetector) {
            g_motionDetector->enableMotionInterrupt();
        }
        
        // Enter deep sleep
        uint32_t sleepDuration = g_powerManager->getOptimalSleepDuration();
        g_powerManager->enterDeepSleep(sleepDuration);
    }

    // Small delay to prevent excessive CPU usage
    delay(100);
}

// Note: To use this production main loop instead of the existing main.cpp,
// either rename this file to main.cpp or include these functions in main.cpp
// and call productionSetup() and productionLoop() from setup() and loop()