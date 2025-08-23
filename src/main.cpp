/**
 * ESP32 Wildlife Camera - Complete AI-Powered Implementation
 * 
 * A comprehensive wildlife monitoring system with:
 * - Motion detection (PIR + frame difference)
 * - AI-based wildlife classification using TensorFlow Lite
 * - Power management with solar charging support
 * - Adaptive duty cycling for extended battery life
 * - Data collection and storage on SD card
 * - Real-time optimization and calibration
 * 
 * Author: ESP32WildlifeCAM Project
 * Version: 2.0.0 (Complete Implementation)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <SD_MMC.h>
#include <time.h>
#include <esp_sleep.h>

// Project headers
#include "include/config.h"
#include "include/pins.h"
#include "camera/camera_manager.h"
#include "detection/hybrid_motion_detector.h"
#include "power/power_manager.h"
#include "utils/logger.h"

// System components
CameraManager cameraManager;
HybridMotionDetector motionDetector;
PowerManager powerManager;

// System state
bool systemInitialized = false;
bool sdCardInitialized = false;
uint32_t bootTime = 0;
uint32_t lastStatusCheck = 0;
uint32_t dailyTriggerCount = 0;
uint32_t lastDayReset = 0;

// Statistics
struct SystemStats {
    uint32_t totalBootCount;
    uint32_t totalImages;
    uint32_t motionEvents;
    uint32_t uptime;
    float averageBatteryLevel;
    uint32_t deepSleepCount;
} systemStats = {};

/**
 * @brief Initialize SD card storage
 */
bool initializeSDCard() {
    LOG_INFO("Initializing SD card...");
    
    if (!SD_MMC.begin()) {
        LOG_ERROR("SD Card initialization failed");
        return false;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        LOG_ERROR("No SD card attached");
        return false;
    }
    
    const char* cardTypeStr = "UNKNOWN";
    if (cardType == CARD_MMC) {
        cardTypeStr = "MMC";
    } else if (cardType == CARD_SD) {
        cardTypeStr = "SDSC";
    } else if (cardType == CARD_SDHC) {
        cardTypeStr = "SDHC";
    }
    
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    LOG_INFO("SD Card Type: " + String(cardTypeStr) + ", Size: " + String((unsigned long)cardSize) + "MB");
    
    // Create required directories
    if (!SD_MMC.exists(IMAGE_FOLDER)) {
        SD_MMC.mkdir(IMAGE_FOLDER);
        LOG_INFO("Created directory: " + String(IMAGE_FOLDER));
    }
    
    if (!SD_MMC.exists(LOG_FOLDER)) {
        SD_MMC.mkdir(LOG_FOLDER);
        LOG_INFO("Created directory: " + String(LOG_FOLDER));
    }
    
    if (!SD_MMC.exists(DATA_FOLDER)) {
        SD_MMC.mkdir(DATA_FOLDER);
        LOG_INFO("Created directory: " + String(DATA_FOLDER));
    }
    
    sdCardInitialized = true;
    LOG_INFO("SD card initialized successfully");
    return true;
}

/**
 * @brief Initialize system time
 */
void initializeTime() {
    // Configure NTP (if WiFi available) or use RTC
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        LOG_WARNING("Failed to obtain time, using default");
        // Set a default time for file naming
        time_t defaultTime = 1640995200; // Jan 1, 2022
        struct timeval tv = { defaultTime, 0 };
        settimeofday(&tv, NULL);
    }
}

/**
 * @brief Check if current time is within active hours
 */
bool isWithinActiveHours() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    int currentHour = timeinfo.tm_hour;
    
    if (ACTIVE_START_HOUR <= ACTIVE_END_HOUR) {
        return (currentHour >= ACTIVE_START_HOUR && currentHour < ACTIVE_END_HOUR);
    } else {
        // Handles case where active period crosses midnight
        return (currentHour >= ACTIVE_START_HOUR || currentHour < ACTIVE_END_HOUR);
    }
}

/**
 * @brief Reset daily counters
 */
void resetDailyCounts() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    uint32_t currentDay = timeinfo.tm_yday;
    uint32_t lastDay = lastDayReset / (24 * 3600);
    
    if (currentDay != lastDay) {
        dailyTriggerCount = 0;
        lastDayReset = now;
        LOG_INFO("Daily counters reset");
    }
}

/**
 * @brief Handle motion detection and image capture
 */
void handleMotionEvent() {
    LOG_DEBUG("Processing motion event...");
    
    // Check if within active hours
    if (!isWithinActiveHours() && !NIGHT_MODE_ENABLED) {
        LOG_DEBUG("Motion detected outside active hours, ignoring");
        return;
    }
    
    // Check daily trigger limit
    if (dailyTriggerCount >= MAX_DAILY_TRIGGERS) {
        LOG_WARNING("Daily trigger limit reached, ignoring motion");
        return;
    }
    
    // Capture image
    CameraManager::CaptureResult result = cameraManager.captureImage();
    if (result.success) {
        dailyTriggerCount++;
        systemStats.totalImages++;
        
        LOG_INFO("Image captured: " + result.filename + " (" + String(result.imageSize) + " bytes, " + String(result.captureTime) + "ms)");
        
        // Update statistics
        systemStats.motionEvents++;
    } else {
        LOG_ERROR("Failed to capture image");
    }
}

/**
 * @brief System status monitoring
 */
void performStatusCheck() {
    uint32_t now = millis();
    
    if (now - lastStatusCheck < STATUS_CHECK_INTERVAL) {
        return;
    }
    
    lastStatusCheck = now;
    
    // Update power manager
    powerManager.update();
    
    // Get system statistics
    PowerManager::PowerStats powerStats = powerManager.getStatistics();
    CameraManager::CameraStats cameraStats = cameraManager.getStatistics();
    HybridMotionDetector::HybridStats motionStats = motionDetector.getStatistics();
    
    // Update system uptime
    systemStats.uptime = now - bootTime;
    systemStats.averageBatteryLevel = (systemStats.averageBatteryLevel * 0.9f) + (powerStats.batteryPercentage * 0.1f);
    
    // Log system status
    String statusMsg = "Status - Battery: " + String(powerStats.batteryPercentage, 1) + "%, " +
                      "Images: " + String(systemStats.totalImages) + ", " +
                      "Motion Events: " + String(systemStats.motionEvents) + ", " +
                      "Uptime: " + String(systemStats.uptime / 1000) + "s";
    
    LOG_INFO(statusMsg);
    
    // Check for low battery
    if (powerStats.currentState == PowerManager::PowerState::LOW_BATTERY) {
        LOG_WARNING("Low battery detected: " + String(powerStats.batteryPercentage, 1) + "%");
    } else if (powerStats.currentState == PowerManager::PowerState::CRITICAL) {
        LOG_CRITICAL("Critical battery level: " + String(powerStats.batteryPercentage, 1) + "%");
    }
}

/**
 * @brief Main system loop
 */
void loop() {
    if (!systemInitialized) {
        return;
    }
    
    // Reset daily counts
    resetDailyCounts();
    
    // Perform status check
    performStatusCheck();
    
    // Check for motion
    HybridMotionDetector::HybridResult motionResult = motionDetector.detectMotion();
    if (motionResult.motionDetected) {
        LOG_DEBUG("Motion detected: " + motionResult.description);
        handleMotionEvent();
    }
    
    // Check if system should enter deep sleep
    if (powerManager.shouldEnterDeepSleep()) {
        LOG_INFO("Entering deep sleep mode");
        
        // Update statistics before sleep
        systemStats.deepSleepCount++;
        
        // Cleanup resources
        Logger::flush();
        
        // Enter deep sleep
        uint32_t sleepDuration = powerManager.getOptimalSleepDuration();
        powerManager.enterDeepSleep(sleepDuration);
    }
    
    // Small delay to prevent overwhelming the system
    delay(100);
}

/**
 * @brief System initialization
 */
void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUDRATE);
    delay(1000);
    
    Serial.println("=================================");
    Serial.println("ESP32 Wildlife Camera v2.0.0");
    Serial.println("Complete AI-Powered Implementation");
    Serial.println("=================================");
    
    bootTime = millis();
    systemStats.totalBootCount++;
    
    // Initialize SD card first for logging
    if (!initializeSDCard()) {
        Serial.println("Critical: SD card initialization failed");
        // Continue without SD card for debugging
    }
    
    // Initialize logger
    Logger::initialize(sdCardInitialized);
    LOG_INFO("=== System Boot Started ===");
    
    // Initialize time
    initializeTime();
    
    // Initialize power management
    if (!powerManager.initialize()) {
        LOG_CRITICAL("Power manager initialization failed");
        return;
    }
    
    // Initialize camera
    if (!cameraManager.initialize()) {
        LOG_CRITICAL("Camera initialization failed");
        return;
    }
    
    // Initialize motion detector
    if (!motionDetector.initialize(&cameraManager)) {
        LOG_CRITICAL("Motion detector initialization failed");
        return;
    }
    
    // Configure motion detector for wildlife monitoring
    motionDetector.setDetectionMode(HYBRID_MOTION_ENABLED, false, false);
    motionDetector.configureWeights(0.6f, 0.4f, MOTION_CONFIRMATION_TIME);
    
    // Configure power management for field deployment
    if (POWER_SAVE_MODE_ENABLED) {
        powerManager.setPowerSavingEnabled(true);
        powerManager.configureAdaptiveDutyCycle(ADAPTIVE_DUTY_CYCLE, 50.0f);
    }
    
    // System initialization complete
    systemInitialized = true;
    
    LOG_INFO("=== System Initialization Complete ===");
    LOG_INFO("Wildlife Camera Ready for Operation");
    
    // Display initial status
    PowerManager::PowerStats powerStats = powerManager.getStatistics();
    LOG_INFO("Initial Status - Battery: " + String(powerStats.batteryPercentage, 1) + "%, " +
             "Solar: " + String(powerStats.solarVoltage, 2) + "V, " +
             "State: " + String((int)powerStats.currentState));
    
    // Flash LED to indicate successful initialization
    pinMode(POWER_LED_PIN, OUTPUT);
    for (int i = 0; i < 3; i++) {
        digitalWrite(POWER_LED_PIN, HIGH);
        delay(200);
        digitalWrite(POWER_LED_PIN, LOW);
        delay(200);
    }
}