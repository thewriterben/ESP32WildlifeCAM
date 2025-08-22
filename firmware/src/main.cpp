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
#include "power_manager.h"
#include "wifi_manager.h"
#include "lora_mesh.h"

// AI/ML Integration (conditionally compiled)
#ifdef ESP32_AI_ENABLED
#include "ai/ai_wildlife_system.h"
#include "ai/ai_common.h"
#endif

/**
 * @class SystemManager
 * @brief Central coordinator for all wildlife camera subsystems with AI/ML integration
 * 
 * This class manages the initialization, coordination, and lifecycle
 * of all camera subsystems, providing a clean interface for the main loop.
 * Includes comprehensive AI/ML capabilities when enabled.
 */
class SystemManager {
public:
    SystemManager();
    ~SystemManager();
    
    bool init();
    void update();
    void handleMotionDetection();
    void enterDeepSleep();
    void handleLowPower();
    void logSystemStatus();
    void cleanup();
    
    // Subsystem access methods
    CameraHandler& getCamera() { return cameraHandler; }
    MotionFilter& getMotionFilter() { return motionFilter; }
    PowerManager& getPowerManager() { return powerManager; }
    WiFiManager& getWiFiManager() { return wifiManager; }
    
    // Status methods
    bool isCameraInitialized() const { return cameraHandler.isInitialized(); }
    bool isSDCardInitialized() const { return sdCardInitialized; }
    bool isLoRaInitialized() const { return loraInitialized; }
    int getDailyTriggerCount() const { return dailyTriggerCount; }
    unsigned long getBootTime() const { return bootTime; }
    
#ifdef ESP32_AI_ENABLED
    // AI-specific methods
    bool isAISystemInitialized() const { return aiSystemInitialized; }
    void handleAIAnalysis();
    void handleIntelligentCapture();
    void saveAIAnalysisMetadata(const String& filename, const WildlifeAnalysisResult& analysis);
#endif

private:
    // Subsystem instances
    CameraHandler cameraHandler;
    MotionFilter motionFilter;
    PowerManager powerManager;
    WiFiManager wifiManager;
    
    // System state
    bool sdCardInitialized;
    bool loraInitialized;
    unsigned long lastMotionTime;
    unsigned long bootTime;
    int dailyTriggerCount;
    unsigned long lastStatusCheck;
    
#ifdef ESP32_AI_ENABLED
    // AI-related state
    bool aiSystemInitialized;
    unsigned long lastAIAnalysis;
    WildlifeAnalysisResult lastAnalysisResult;
#endif
    
    // Private methods
    bool initializeFileSystem();
    bool initializeSDCard();
    bool initializeLoRa();
    bool isWithinActiveHours();
    void resetDailyCounts();
    String createImageFilename();
    
#ifdef ESP32_AI_ENABLED
    bool initializeAISystem();
#endif
};

// Global system manager instance
SystemManager systemManager;

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
    
    // Initialize system manager
    if (!systemManager.init()) {
        DEBUG_PRINTLN("CRITICAL: System initialization failed!");
        DEBUG_PRINTLN("Entering emergency deep sleep...");
        esp_deep_sleep(60 * 1000000); // Sleep for 1 minute and retry
    }
    
    // Log initial system status
    systemManager.logSystemStatus();
    
    DEBUG_PRINTLN("System initialization complete!");
    DEBUG_PRINTLN("Wildlife camera ready for operation.\n");
}

/**
 * Arduino Main Loop
 * Handles motion detection, power management, and system monitoring
 */
void loop() {
    // Update all subsystems
    systemManager.update();
    
    // Small delay to prevent excessive CPU usage
    delay(100);
}

// SystemManager implementation

/**
 * SystemManager Constructor
 */
SystemManager::SystemManager() 
    : sdCardInitialized(false), loraInitialized(false), lastMotionTime(0),
      bootTime(0), dailyTriggerCount(0), lastStatusCheck(0)
#ifdef ESP32_AI_ENABLED
    , aiSystemInitialized(false), lastAIAnalysis(0)
#endif
{
}

/**
 * SystemManager Destructor
 */
SystemManager::~SystemManager() {
    cleanup();
}

/**
 * Initialize all system components
 */
bool SystemManager::init() {
    DEBUG_PRINTLN("Initializing system components...");
    
    bootTime = millis();
    
    // Initialize file system first
    if (!initializeFileSystem()) {
        DEBUG_PRINTLN("File system initialization failed");
        return false;
    }
    
    // Initialize power management
    if (!powerManager.init()) {
        DEBUG_PRINTLN("Power management initialization failed");
        return false;
    }
    
    // Initialize motion detection
    if (!motionFilter.init()) {
        DEBUG_PRINTLN("Motion filter initialization failed");
        return false;
    }
    
    // Initialize camera
    if (!cameraHandler.init()) {
        DEBUG_PRINTLN("Camera initialization failed");
        return false;
    }
    
    // Initialize SD card storage
    if (!initializeSDCard()) {
        DEBUG_PRINTLN("Warning: SD card initialization failed");
        // Not critical - system can continue without SD card
    }
    
    // Initialize WiFi if enabled
    if (WIFI_ENABLED) {
        if (!wifiManager.init()) {
            DEBUG_PRINTLN("Warning: WiFi initialization failed");
            // Not critical - system can continue without WiFi
        }
    }
    
    // Initialize LoRa mesh networking if enabled
    if (LORA_ENABLED) {
        loraInitialized = initializeLoRa();
        if (loraInitialized) {
            DEBUG_PRINTLN("LoRa mesh network initialized");
        } else {
            DEBUG_PRINTLN("Warning: LoRa initialization failed");
        }
    }
    
#ifdef ESP32_AI_ENABLED
    // Initialize AI system if enabled
    if (initializeAISystem()) {
        aiSystemInitialized = true;
        DEBUG_PRINTLN("AI Wildlife System initialized successfully");
    } else {
        aiSystemInitialized = false;
        DEBUG_PRINTLN("Warning: AI system initialization failed - running without AI");
    }
#endif
    
    return true;
}

/**
 * Update all subsystems
 */
void SystemManager::update() {
    unsigned long currentTime = millis();
    
    // Update power management
    powerManager.update();
    
    // Update WiFi if enabled
    if (WIFI_ENABLED) {
        wifiManager.update();
    }
    
    // Check system status every 30 seconds
    if (currentTime - lastStatusCheck > STATUS_REPORT_INTERVAL) {
        logSystemStatus();
        lastStatusCheck = currentTime;
        
        // Reset daily counters at midnight
        resetDailyCounts();
        
        // Handle low power situations
        if (powerManager.getBatteryVoltage() < BATTERY_CRITICAL_THRESHOLD) {
            handleLowPower();
            return;
        }
    }
    
    // Check for motion detection
    if (motionFilter.isMotionDetected()) {
        DEBUG_PRINTLN("Motion detected!");
        
        // Apply weather filtering
        if (motionFilter.isValidMotion()) {
            DEBUG_PRINTLN("Valid motion after filtering");
            
#ifdef ESP32_AI_ENABLED
            // Use AI-enhanced motion analysis if available
            if (aiSystemInitialized) {
                handleAIAnalysis();
            } else {
                handleMotionDetection();
            }
#else
            handleMotionDetection();
#endif
        } else {
            DEBUG_PRINTLN("Motion filtered out (weather conditions)");
        }
    }
    
#ifdef ESP32_AI_ENABLED
    // Perform intelligent capture check (AI-driven)
    if (aiSystemInitialized && g_aiSystem && g_aiSystem->shouldTriggerCapture()) {
        DEBUG_PRINTLN("AI triggered intelligent capture");
        handleIntelligentCapture();
    }
#endif
    
    // Handle LoRa mesh communication
    if (loraInitialized) {
        LoraMesh::processMessages();
    }
    
#ifdef ESP32_AI_ENABLED
    // AI-optimized deep sleep decision
    if (aiSystemInitialized && g_aiSystem && g_aiSystem->shouldEnterDeepSleep()) {
        DEBUG_PRINTLN("AI recommends deep sleep");
        enterDeepSleep();
        return;
    }
#endif
    
    // Enter deep sleep if no activity and low power
    if (currentTime - lastMotionTime > DEEP_SLEEP_DURATION * 1000 && 
        powerManager.shouldEnterLowPower()) {
        enterDeepSleep();
    }
}

/**
 * Handle motion detection event
 */
void SystemManager::handleMotionDetection() {
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
    if (cameraHandler.isInitialized()) {
        camera_fb_t* fb = cameraHandler.captureImage();
        if (fb) {
            dailyTriggerCount++;
            
            // Save image to SD card
            if (sdCardInitialized) {
                String filename = cameraHandler.saveImage(fb, IMAGE_FOLDER);
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
void SystemManager::enterDeepSleep() {
    DEBUG_PRINTLN("Entering deep sleep mode...");
    
    // Cleanup all subsystems
    cleanup();
    
    // Configure wake-up sources
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION * 1000000); // Convert to microseconds
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1); // PIR sensor wake-up
    
    DEBUG_PRINTF("Sleeping for %d seconds\n", DEEP_SLEEP_DURATION);
    delay(100); // Allow debug message to be sent
    
    esp_deep_sleep_start();
}

/**
 * Handle critical low power situation
 */
void SystemManager::handleLowPower() {
    DEBUG_PRINTLN("Critical low power - entering emergency mode");
    
    // Enter power saving mode
    powerManager.enterPowerSaving();
    
    // Disable non-essential systems
    if (wifiManager.isConnected()) {
        wifiManager.disconnect();
    }
    
    // Reduce camera quality if possible
    // This would require additional camera configuration methods
    
    // Enter deep sleep for longer period
    DEBUG_PRINTLN("Entering extended deep sleep...");
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION * 2 * 1000000); // Double sleep time
    esp_deep_sleep_start();
}

/**
 * Log current system status
 */
void SystemManager::logSystemStatus() {
    PowerStats powerStats = powerManager.getPowerStats();
    MotionFilterStatus motionStatus = motionFilter.getStatus();
    CameraStatus cameraStatus = cameraHandler.getStatus();
    WiFiStatusInfo wifiStatus = wifiManager.getStatus();
    
    DEBUG_PRINTLN("=== System Status ===");
    DEBUG_PRINTF("Uptime: %lu seconds\n", (millis() - bootTime) / 1000);
    DEBUG_PRINTF("Battery: %.2fV (%d%%) %s\n", 
                 powerStats.batteryVoltage, powerStats.batteryPercentage,
                 powerStats.isCharging ? "(Charging)" : "(Discharging)");
    DEBUG_PRINTF("Solar: %.2fV\n", powerStats.solarVoltage);
    DEBUG_PRINTF("Daily triggers: %d/%d\n", dailyTriggerCount, MAX_DAILY_TRIGGERS);
    DEBUG_PRINTF("Camera: %s (%s)\n", 
                 cameraStatus.initialized ? "OK" : "Error",
                 cameraStatus.boardName);
    DEBUG_PRINTF("SD Card: %s\n", sdCardInitialized ? "OK" : "Error");
    DEBUG_PRINTF("LoRa: %s\n", loraInitialized ? "OK" : "Disabled");
    DEBUG_PRINTF("WiFi: %s", wifiStatus.initialized ? "Enabled" : "Disabled");
    if (wifiStatus.status == WIFI_STATUS_CONNECTED) {
        DEBUG_PRINTF(" (Connected to %s, IP: %s, RSSI: %d dBm)\n", 
                     wifiStatus.ssid.c_str(), wifiStatus.ipAddress.c_str(), wifiStatus.rssi);
    } else {
        DEBUG_PRINTLN(" (Disconnected)");
    }
    DEBUG_PRINTF("Motion Filter: %s\n", motionStatus.initialized ? "Active" : "Inactive");
    DEBUG_PRINTF("Free heap: %d bytes\n", ESP.getFreeHeap());
    DEBUG_PRINTLN("====================\n");
}

/**
 * Cleanup all system resources
 */
void SystemManager::cleanup() {
    DEBUG_PRINTLN("Cleaning up system resources...");
    
    cameraHandler.cleanup();
    motionFilter.cleanup();
    powerManager.cleanup();
    wifiManager.cleanup();
    
    if (loraInitialized) {
        LoraMesh::cleanup();
        loraInitialized = false;
    }
}

/**
 * Initialize LittleFS file system
 */
bool SystemManager::initializeFileSystem() {
    DEBUG_PRINTLN("Initializing file system...");
    
    if (!LittleFS.begin()) {
        DEBUG_PRINTLN("LittleFS mount failed, formatting...");
        if (LittleFS.format()) {
            DEBUG_PRINTLN("LittleFS formatted successfully");
            if (!LittleFS.begin()) {
                DEBUG_PRINTLN("Error: LittleFS mount failed after format");
                return false;
            }
        } else {
            DEBUG_PRINTLN("Error: LittleFS format failed");
            return false;
        }
    }
    
    DEBUG_PRINTLN("File system initialized");
    return true;
}

/**
 * Initialize SD card storage
 */
bool SystemManager::initializeSDCard() {
    DEBUG_PRINTLN("Initializing SD card...");
    
    if (!SD_MMC.begin()) {
        DEBUG_PRINTLN("SD Card initialization failed");
        return false;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        DEBUG_PRINTLN("No SD card attached");
        return false;
    }
    
    DEBUG_PRINTF("SD Card Type: ");
    if (cardType == CARD_MMC) {
        DEBUG_PRINTLN("MMC");
    } else if (cardType == CARD_SD) {
        DEBUG_PRINTLN("SDSC");
    } else if (cardType == CARD_SDHC) {
        DEBUG_PRINTLN("SDHC");
    } else {
        DEBUG_PRINTLN("UNKNOWN");
    }
    
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    DEBUG_PRINTF("SD Card Size: %lluMB\n", cardSize);
    
    // Create required directories
    if (!SD_MMC.exists(IMAGE_FOLDER)) {
        SD_MMC.mkdir(IMAGE_FOLDER);
        DEBUG_PRINTF("Created directory: %s\n", IMAGE_FOLDER);
    }
    
    if (!SD_MMC.exists(LOG_FOLDER)) {
        SD_MMC.mkdir(LOG_FOLDER);
        DEBUG_PRINTF("Created directory: %s\n", LOG_FOLDER);
    }
    
    sdCardInitialized = true;
    DEBUG_PRINTLN("SD card initialized successfully");
    return true;
}

/**
 * Initialize LoRa mesh networking
 */
bool SystemManager::initializeLoRa() {
    DEBUG_PRINTLN("Initializing LoRa mesh networking...");
    return LoraMesh::init();
}

/**
 * Check if current time is within active hours
 */
bool SystemManager::isWithinActiveHours() {
    if (!NTP_ENABLED) {
        return true; // Always active if no time reference
    }
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return true; // Default to active if time not available
    }
    
    int currentHour = timeinfo.tm_hour;
    
    if (TRIGGER_ACTIVE_HOURS_START <= TRIGGER_ACTIVE_HOURS_END) {
        // Normal case: start < end (e.g., 6 AM to 8 PM)
        return (currentHour >= TRIGGER_ACTIVE_HOURS_START && 
                currentHour < TRIGGER_ACTIVE_HOURS_END);
    } else {
        // Overnight case: start > end (e.g., 8 PM to 6 AM)
        return (currentHour >= TRIGGER_ACTIVE_HOURS_START || 
                currentHour < TRIGGER_ACTIVE_HOURS_END);
    }
}

/**
 * Reset daily counters at midnight
 */
void SystemManager::resetDailyCounts() {
    if (!NTP_ENABLED) return;
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;
    
    static int lastDay = -1;
    if (lastDay != timeinfo.tm_mday) {
        if (lastDay != -1) { // Not the first check
            DEBUG_PRINTLN("Resetting daily counters");
            dailyTriggerCount = 0;
            
#ifdef ESP32_AI_ENABLED
            // Reset AI system daily metrics if available
            if (aiSystemInitialized && g_aiSystem) {
                DEBUG_PRINTLN("Resetting AI daily metrics");
                // This would reset daily AI metrics if implemented
            }
#endif
            powerManager.resetStats();
            motionFilter.resetStats();
        }
        lastDay = timeinfo.tm_mday;
    }
}

#ifdef ESP32_AI_ENABLED
/**
 * Initialize AI Wildlife System
 */
bool SystemManager::initializeAISystem() {
    DEBUG_PRINTLN("Initializing AI Wildlife System...");
    
    // Initialize the global AI system
    if (::initializeAISystem()) {
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
        return true;
    } else {
        DEBUG_PRINTLN("Warning: AI system initialization failed - running without AI");
        return false;
    }
}

/**
 * Handle AI-enhanced motion analysis
 */
void SystemManager::handleAIAnalysis() {
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
    if (cameraHandler.isInitialized()) {
        camera_fb_t* fb = cameraHandler.captureImage();
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
                
                String filename = cameraHandler.saveImage(fb, IMAGE_FOLDER);
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
void SystemManager::handleIntelligentCapture() {
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
void SystemManager::saveAIAnalysisMetadata(const String& filename, const WildlifeAnalysisResult& analysis) {
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
    PowerStats powerStats = powerManager.getPowerStats();
    environment["battery_voltage"] = powerStats.batteryVoltage;
    environment["solar_voltage"] = powerStats.solarVoltage;
    environment["is_charging"] = powerStats.isCharging;
    
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
#endif
