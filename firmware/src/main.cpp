/**
 * ESP32 Wildlife Camera - Main Application
 * 
 * A solar-powered wildlife trail camera with LoRa mesh networking,
 * intelligent motion detection, and weather filtering.
 * 
 * Features:
 * - ESP32-S3 with camera module
 * - Solar power management
 * - LoRa mesh networking
 * - Weather-aware motion filtering
 * - Deep sleep power optimization
 * 
 * Author: ESP32WildlifeCAM Project
 * Version: 1.0.0
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_camera.h>
#include <SD_MMC.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>

#include "config.h"
#include "camera_handler.h"
#include "motion_filter.h"
#include "solar_manager.h"
#include "lora_mesh.h"

// Global variables
bool cameraInitialized = false;
bool sdCardInitialized = false;
bool loraInitialized = false;
unsigned long lastMotionTime = 0;
unsigned long bootTime = 0;
int dailyTriggerCount = 0;

// Function prototypes
void initializeSystem();
void initializeCamera();
void initializeSDCard();
void initializeFileSystem();
void handleMotionDetection();
void enterDeepSleep();
void logSystemStatus();
void handleLowPower();
bool isWithinActiveHours();
void resetDailyCounts();

/**
 * Arduino Setup Function
 * Initializes all system components and prepares for operation
 */
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(SERIAL_INIT_DELAY);  // Allow serial to initialize
    
    DEBUG_PRINTLN("\n========================================");
    DEBUG_PRINTLN("ESP32 Wildlife Camera Starting...");
    DEBUG_PRINTF("Firmware Version: %s\n", FIRMWARE_VERSION);
    DEBUG_PRINTF("Build Date: %s %s\n", BUILD_DATE, BUILD_TIME);
    DEBUG_PRINTLN("========================================\n");
    
    bootTime = millis();
    
    // Initialize all system components
    initializeSystem();
    
    // Log initial system status
    logSystemStatus();
    
    DEBUG_PRINTLN("System initialization complete!");
    DEBUG_PRINTLN("Wildlife camera ready for operation.\n");
}

/**
 * Arduino Main Loop
 * Handles motion detection, power management, and system monitoring
 */
void loop() {
    static unsigned long lastStatusCheck = 0;
    unsigned long currentTime = millis();
    
    // Check system status every 30 seconds
    if (currentTime - lastStatusCheck > 30000) {
        logSystemStatus();
        lastStatusCheck = currentTime;
        
        // Reset daily counters at midnight
        resetDailyCounts();
        
        // Handle low power situations
        if (SolarManager::getBatteryVoltage() < BATTERY_CRITICAL_THRESHOLD) {
            handleLowPower();
            return;
        }
    }
    
    // Check for motion detection
    if (MotionFilter::isMotionDetected()) {
        DEBUG_PRINTLN("Motion detected!");
        
        // Apply weather filtering
        if (MotionFilter::isValidMotion()) {
            DEBUG_PRINTLN("Valid motion after filtering");
            handleMotionDetection();
        } else {
            DEBUG_PRINTLN("Motion filtered out (weather conditions)");
        }
    }
    
    // Handle LoRa mesh communication
    if (loraInitialized) {
        LoraMesh::processMessages();
    }
    
    // Update solar manager
    SolarManager::update();
    
    // Small delay to prevent excessive CPU usage
    delay(100);
    
    // Enter deep sleep if no activity and low power
    if (currentTime - lastMotionTime > DEEP_SLEEP_DURATION * 1000 && 
        SolarManager::getBatteryVoltage() < SOLAR_VOLTAGE_THRESHOLD) {
        enterDeepSleep();
    }
}

/**
 * Initialize all system components
 */
void initializeSystem() {
    DEBUG_PRINTLN("Initializing system components...");
    
    // Initialize file system first
    initializeFileSystem();
    
    // Initialize power management
    SolarManager::init();
    
    // Initialize motion detection
    MotionFilter::init();
    
    // Initialize camera
    initializeCamera();
    
    // Initialize SD card storage
    initializeSDCard();
    
    // Initialize LoRa mesh networking
    if (LORA_ENABLED) {
        loraInitialized = LoraMesh::init();
        if (loraInitialized) {
            DEBUG_PRINTLN("LoRa mesh network initialized");
        } else {
            DEBUG_PRINTLN("Warning: LoRa initialization failed");
        }
    }
    
    // Set up PIR interrupt
    pinMode(PIR_PIN, INPUT);
    
    // Initialize status LED
    pinMode(CHARGING_LED_PIN, OUTPUT);
    digitalWrite(CHARGING_LED_PIN, LOW);
}

/**
 * Initialize camera module
 */
void initializeCamera() {
    DEBUG_PRINTLN("Initializing camera...");
    
    cameraInitialized = CameraHandler::init();
    
    if (cameraInitialized) {
        DEBUG_PRINTLN("Camera initialized successfully");
        
        // Test camera by taking a sample photo
        camera_fb_t* fb = CameraHandler::captureImage();
        if (fb) {
            DEBUG_PRINTF("Test image captured: %dx%d, %d bytes\n", 
                        fb->width, fb->height, fb->len);
            esp_camera_fb_return(fb);
        } else {
            DEBUG_PRINTLN("Warning: Test image capture failed");
        }
    } else {
        DEBUG_PRINTLN("Error: Camera initialization failed!");
    }
}

/**
 * Initialize SD card storage
 */
void initializeSDCard() {
    DEBUG_PRINTLN("Initializing SD card...");
    
    if (!SD_MMC.begin()) {
        DEBUG_PRINTLN("Warning: SD card initialization failed");
        sdCardInitialized = false;
        return;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        DEBUG_PRINTLN("Warning: No SD card attached");
        sdCardInitialized = false;
        return;
    }
    
    // Create necessary directories
    if (!SD_MMC.exists(IMAGE_FOLDER)) {
        SD_MMC.mkdir(IMAGE_FOLDER);
        DEBUG_PRINTF("Created directory: %s\n", IMAGE_FOLDER);
    }
    
    if (!SD_MMC.exists(LOG_FOLDER)) {
        SD_MMC.mkdir(LOG_FOLDER);
        DEBUG_PRINTF("Created directory: %s\n", LOG_FOLDER);
    }
    
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    DEBUG_PRINTF("SD card initialized: %lluMB\n", cardSize);
    
    sdCardInitialized = true;
}

/**
 * Initialize LittleFS file system
 */
void initializeFileSystem() {
    DEBUG_PRINTLN("Initializing file system...");
    
    if (!LittleFS.begin()) {
        DEBUG_PRINTLN("LittleFS mount failed, formatting...");
        if (LittleFS.format()) {
            DEBUG_PRINTLN("LittleFS formatted successfully");
            if (!LittleFS.begin()) {
                DEBUG_PRINTLN("Error: LittleFS mount failed after format");
                return;
            }
        } else {
            DEBUG_PRINTLN("Error: LittleFS format failed");
            return;
        }
    }
    
    DEBUG_PRINTLN("File system initialized");
}

/**
 * Handle motion detection event
 */
void handleMotionDetection() {
    lastMotionTime = millis();
    
    // Check if within active hours
    if (!isWithinActiveHours()) {
        DEBUG_PRINTLN("Motion detected outside active hours");
        return;
    }
    
    // Check daily trigger limit
    if (dailyTriggerCount >= MAX_DAILY_TRIGGERS) {
        DEBUG_PRINTLN("Daily trigger limit reached");
        return;
    }
    
    // Capture image
    if (cameraInitialized) {
        camera_fb_t* fb = CameraHandler::captureImage();
        if (fb) {
            dailyTriggerCount++;
            
            // Save image to SD card
            if (sdCardInitialized) {
                String filename = CameraHandler::saveImage(fb, IMAGE_FOLDER);
                DEBUG_PRINTF("Image saved: %s\n", filename.c_str());
                
                // Transmit via LoRa if enabled
                if (loraInitialized && IMAGE_COMPRESSION_ENABLED) {
                    LoraMesh::transmitImage(fb, filename);
                }
            }
            
            esp_camera_fb_return(fb);
        } else {
            DEBUG_PRINTLN("Error: Failed to capture image");
        }
    }
    
    // Flash LED to indicate capture
    digitalWrite(CHARGING_LED_PIN, HIGH);
    delay(100);
    digitalWrite(CHARGING_LED_PIN, LOW);
}

/**
 * Enter deep sleep mode to conserve power
 */
void enterDeepSleep() {
    DEBUG_PRINTLN("Entering deep sleep...");
    
    // Configure wake-up source (PIR motion sensor)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);  // PIR_PIN
    
    // Also wake up after a timeout
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION * 1000000ULL);
    
    // Turn off unnecessary peripherals
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    delay(100);
    esp_deep_sleep_start();
}

/**
 * Log current system status
 */
void logSystemStatus() {
    float batteryVoltage = SolarManager::getBatteryVoltage();
    float solarVoltage = SolarManager::getSolarVoltage();
    bool isCharging = SolarManager::isCharging();
    
    DEBUG_PRINTLN("=== System Status ===");
    DEBUG_PRINTF("Uptime: %lu seconds\n", (millis() - bootTime) / 1000);
    DEBUG_PRINTF("Battery: %.2fV %s\n", batteryVoltage, 
                 isCharging ? "(Charging)" : "(Discharging)");
    DEBUG_PRINTF("Solar: %.2fV\n", solarVoltage);
    DEBUG_PRINTF("Daily triggers: %d/%d\n", dailyTriggerCount, MAX_DAILY_TRIGGERS);
    DEBUG_PRINTF("Camera: %s\n", cameraInitialized ? "OK" : "Error");
    DEBUG_PRINTF("SD Card: %s\n", sdCardInitialized ? "OK" : "Error");
    DEBUG_PRINTF("LoRa: %s\n", loraInitialized ? "OK" : "Disabled");
    DEBUG_PRINTF("Free heap: %d bytes\n", ESP.getFreeHeap());
    DEBUG_PRINTLN("====================\n");
}

/**
 * Handle critical low power situation
 */
void handleLowPower() {
    DEBUG_PRINTLN("CRITICAL: Low battery detected!");
    DEBUG_PRINTLN("Entering emergency deep sleep...");
    
    // Disable all non-essential systems
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    // Extended sleep time in low power mode
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION * 2 * 1000000ULL);
    esp_deep_sleep_start();
}

/**
 * Check if current time is within active hours
 */
bool isWithinActiveHours() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        // If time is not available, assume it's within active hours
        return true;
    }
    
    int currentHour = timeinfo.tm_hour;
    return (currentHour >= TRIGGER_ACTIVE_HOURS_START && 
            currentHour < TRIGGER_ACTIVE_HOURS_END);
}

/**
 * Reset daily counters at midnight
 */
void resetDailyCounts() {
    static int lastDay = -1;
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        if (lastDay != -1 && timeinfo.tm_mday != lastDay) {
            DEBUG_PRINTLN("New day detected - resetting daily counters");
            dailyTriggerCount = 0;
        }
        lastDay = timeinfo.tm_mday;
    }
}