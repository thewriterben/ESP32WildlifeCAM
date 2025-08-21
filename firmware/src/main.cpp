/**
 * ESP32 Wildlife Camera - Main Application with AI/ML Integration
 * 
 * A solar-powered wildlife trail camera with LoRa mesh networking,
 * intelligent motion detection, weather filtering, and comprehensive
 * AI-powered wildlife monitoring capabilities.
 * 
 * Features:
 * - ESP32-S3 with camera module and AI acceleration
 * - Solar power management with AI optimization
 * - LoRa mesh networking
 * - Weather-aware motion filtering
 * - Deep sleep power optimization
 * - AI-powered species classification and behavior analysis
 * - TensorFlow Lite Micro integration
 * - Edge Impulse platform support
 * - MicroPython AI scripting
 * - Intelligent triggering and power management
 * 
 * Author: ESP32WildlifeCAM Project
 * Version: 2.0.0 (AI Enhanced)
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

// AI/ML Integration
#include "ai/ai_wildlife_system.h"
#include "ai/ai_common.h"

// Global variables
bool cameraInitialized = false;
bool sdCardInitialized = false;
bool loraInitialized = false;
bool aiSystemInitialized = false;
unsigned long lastMotionTime = 0;
unsigned long bootTime = 0;
int dailyTriggerCount = 0;

// AI-related global variables
unsigned long lastAIAnalysis = 0;
WildlifeAnalysisResult lastAnalysisResult;

// Function prototypes
void initializeSystem();
void initializeCamera();
void initializeSDCard();
void initializeFileSystem();
void initializeAISystem();
void handleMotionDetection();
void handleAIAnalysis();
void handleIntelligentCapture();
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
    delay(1000);  // Allow serial to initialize
    
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
    
    // Check for motion detection with AI enhancement
    if (MotionFilter::isMotionDetected()) {
        DEBUG_PRINTLN("Motion detected!");
        
        // Apply weather filtering
        if (MotionFilter::isValidMotion()) {
            DEBUG_PRINTLN("Valid motion after filtering");
            
            // Use AI-enhanced motion analysis if available
            if (aiSystemInitialized) {
                handleAIAnalysis();
            } else {
                handleMotionDetection();
            }
        } else {
            DEBUG_PRINTLN("Motion filtered out (weather conditions)");
        }
    }
    
    // Perform intelligent capture check (AI-driven)
    if (aiSystemInitialized && g_aiSystem && g_aiSystem->shouldTriggerCapture()) {
        DEBUG_PRINTLN("AI triggered intelligent capture");
        handleIntelligentCapture();
    }
    
    // Handle LoRa mesh communication
    if (loraInitialized) {
        LoraMesh::processMessages();
    }
    
    // Update solar manager with AI power optimization
    SolarManager::update();
    if (aiSystemInitialized && g_aiSystem) {
        g_aiSystem->updatePowerState(SolarManager::getBatteryVoltage(), SolarManager::isCharging());
    }
    
    // Small delay to prevent excessive CPU usage
    delay(100);
    
    // AI-optimized deep sleep decision
    if (aiSystemInitialized && g_aiSystem && g_aiSystem->shouldEnterDeepSleep()) {
        DEBUG_PRINTLN("AI recommends deep sleep");
        enterDeepSleep();
    }
    // Fallback to traditional deep sleep logic
    else if (currentTime - lastMotionTime > DEEP_SLEEP_DURATION * 1000 && 
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
    
    // Initialize AI system
    initializeAISystem();
    
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
            
            // Reset AI system daily metrics if available
            if (aiSystemInitialized && g_aiSystem) {
                // This would reset daily AI metrics
                DEBUG_PRINTLN("Resetting AI daily metrics");
            }
        }
        lastDay = timeinfo.tm_mday;
    }
}

/**
 * Initialize AI Wildlife System
 */
void initializeAISystem() {
    DEBUG_PRINTLN("Initializing AI Wildlife System...");
    
    // Initialize the global AI system
    if (initializeAISystem()) {
        aiSystemInitialized = true;
        DEBUG_PRINTLN("AI Wildlife System initialized successfully");
        
        // Load AI models if available
        if (g_aiSystem) {
            // Try to load models from SD card or flash
            bool modelsLoaded = g_aiSystem->loadModels("/models");
            if (modelsLoaded) {
                DEBUG_PRINTLN("AI models loaded successfully");
            } else {
                DEBUG_PRINTLN("Warning: Some AI models failed to load");
            }
            
            // Configure AI system for wildlife monitoring
            AIProcessingConfig aiConfig;
            aiConfig.enableSpeciesClassification = true;
            aiConfig.enableBehaviorAnalysis = true;
            aiConfig.enableMotionDetection = true;
            aiConfig.enableHumanDetection = true;
            aiConfig.powerOptimized = true;
            aiConfig.confidenceThreshold = 0.7f;
            aiConfig.processingInterval = 3000; // 3 seconds between AI analyses
            
            g_aiSystem->configure(aiConfig);
            
            // Enable intelligent scheduling for power optimization
            g_aiSystem->enableIntelligentScheduling(true);
            g_aiSystem->enableContinuousLearning(true);
        }
    } else {
        aiSystemInitialized = false;
        DEBUG_PRINTLN("Warning: AI system initialization failed - running without AI");
    }
}

/**
 * Handle AI-enhanced motion analysis
 */
void handleAIAnalysis() {
    if (!aiSystemInitialized || !g_aiSystem) {
        // Fallback to traditional motion handling
        handleMotionDetection();
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Check if enough time has passed since last AI analysis
    if (currentTime - lastAIAnalysis < 2000) { // Minimum 2 seconds between analyses
        return;
    }
    
    lastAIAnalysis = currentTime;
    lastMotionTime = currentTime;
    
    DEBUG_PRINTLN("Starting AI-enhanced motion analysis...");
    
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
    
    // Capture image for AI analysis
    if (cameraInitialized) {
        camera_fb_t* fb = CameraHandler::captureImage();
        if (fb) {
            // Convert to CameraFrame for AI processing
            CameraFrame frame(fb);
            
            // Perform AI analysis
            WildlifeAnalysisResult analysis = g_aiSystem->analyzeFrame(frame);
            lastAnalysisResult = analysis;
            
            // Log analysis results
            if (analysis.species.species != SpeciesType::UNKNOWN) {
                DEBUG_PRINTF("AI Analysis - Species: %s, Confidence: %.2f%%\n",
                           speciesTypeToString(analysis.species.species),
                           analysis.species.confidence * 100);
                
                if (analysis.behavior.primaryBehavior != BehaviorType::UNKNOWN) {
                    DEBUG_PRINTF("AI Analysis - Behavior: %s, Confidence: %.2f%%\n",
                               behaviorTypeToString(analysis.behavior.primaryBehavior),
                               analysis.behavior.confidence * 100);
                }
            }
            
            // Decide whether to save the image based on AI analysis
            bool shouldSave = false;
            
            if (analysis.overallConfidence >= 0.6f) {
                shouldSave = true;
                DEBUG_PRINTLN("High confidence detection - saving image");
            } else if (analysis.species.species != SpeciesType::UNKNOWN && 
                      analysis.species.confidence >= 0.4f) {
                shouldSave = true;
                DEBUG_PRINTLN("Potential wildlife detected - saving image");
            } else if (analysis.humanPresenceDetected) {
                shouldSave = true;
                DEBUG_PRINTLN("Human presence detected - saving image");
            } else if (analysis.threatDetected) {
                shouldSave = true;
                DEBUG_PRINTLN("Threat detected - saving image");
            }
            
            if (shouldSave && sdCardInitialized) {
                dailyTriggerCount++;
                
                String filename = CameraHandler::saveImage(fb, IMAGE_FOLDER);
                DEBUG_PRINTF("AI-triggered image saved: %s\n", filename.c_str());
                
                // Save AI analysis metadata
                String metadataFilename = filename;
                metadataFilename.replace(".jpg", "_ai.json");
                saveAIAnalysisMetadata(metadataFilename, analysis);
                
                // Notify AI system of image capture
                g_aiSystem->onImageCaptured(frame, filename);
                
                // Transmit via LoRa if enabled and high priority
                if (loraInitialized && IMAGE_COMPRESSION_ENABLED && 
                    analysis.overallConfidence >= 0.8f) {
                    LoraMesh::transmitImage(fb, filename);
                }
            } else {
                DEBUG_PRINTLN("AI analysis below threshold - not saving image");
            }
            
            esp_camera_fb_return(fb);
        } else {
            DEBUG_PRINTLN("Error: Failed to capture image for AI analysis");
        }
    }
}

/**
 * Handle intelligent capture (AI-triggered without motion)
 */
void handleIntelligentCapture() {
    DEBUG_PRINTLN("Processing intelligent capture trigger...");
    
    // Check basic conditions
    if (!isWithinActiveHours()) {
        DEBUG_PRINTLN("Intelligent capture outside active hours");
        return;
    }
    
    if (dailyTriggerCount >= MAX_DAILY_TRIGGERS) {
        DEBUG_PRINTLN("Daily trigger limit reached for intelligent capture");
        return;
    }
    
    // Perform the AI analysis and capture
    handleAIAnalysis();
}

/**
 * Save AI analysis metadata to JSON file
 */
void saveAIAnalysisMetadata(const String& filename, const WildlifeAnalysisResult& analysis) {
    if (!sdCardInitialized) return;
    
    DynamicJsonDocument doc(1024);
    
    // Species information
    if (analysis.species.species != SpeciesType::UNKNOWN) {
        JsonObject species = doc.createNestedObject("species");
        species["type"] = speciesTypeToString(analysis.species.species);
        species["confidence"] = analysis.species.confidence;
        species["confidence_level"] = confidenceLevelToString(analysis.species.confidenceLevel);
        species["count"] = analysis.species.animalCount;
    }
    
    // Behavior information
    if (analysis.behavior.primaryBehavior != BehaviorType::UNKNOWN) {
        JsonObject behavior = doc.createNestedObject("behavior");
        behavior["primary"] = behaviorTypeToString(analysis.behavior.primaryBehavior);
        behavior["confidence"] = analysis.behavior.confidence;
        behavior["activity_level"] = analysis.behavior.activity_level;
        behavior["stress_level"] = analysis.behavior.stress_level;
    }
    
    // Analysis metadata
    doc["overall_confidence"] = analysis.overallConfidence;
    doc["motion_detected"] = analysis.motionDetected;
    doc["threat_detected"] = analysis.threatDetected;
    doc["human_detected"] = analysis.humanPresenceDetected;
    doc["processing_time"] = analysis.processingTime;
    doc["timestamp"] = analysis.timestamp;
    
    // Environmental data
    JsonObject environment = doc.createNestedObject("environment");
    environment["battery_voltage"] = SolarManager::getBatteryVoltage();
    environment["solar_voltage"] = SolarManager::getSolarVoltage();
    environment["is_charging"] = SolarManager::isCharging();
    
    // Save to file
    File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    if (file) {
        serializeJson(doc, file);
        file.close();
        DEBUG_PRINTF("AI metadata saved: %s\n", filename.c_str());
    } else {
        DEBUG_PRINTF("Error: Failed to save AI metadata: %s\n", filename.c_str());
    }
}