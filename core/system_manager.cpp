/**
 * @file system_manager.cpp
 * @brief System manager implementation
 * @author thewriterben
 * @date 2025-08-31
 */

#include "system_manager.h"
#include "../utils/logger.h"
#include "../config.h"
#include <esp_system.h>
#include <LittleFS.h>
#include <SD_MMC.h>

SystemManager::SystemManager(BoardDetector::BoardType board) 
    : m_boardType(board), 
      m_state(STATE_UNINITIALIZED),
      m_initialized(false), 
      m_lastUpdate(0),
      m_cameraReady(false),
      m_storageReady(false),
      m_networkReady(false),
      m_sensorsReady(false),
      m_errorCount(0) {
    
    m_pinConfig = BoardDetector::getPinConfig(board);
    memset(m_lastError, 0, sizeof(m_lastError));
}

SystemManager::~SystemManager() {
    // Cleanup resources
    if (m_storageReady) {
        SD_MMC.end();
        LittleFS.end();
    }
}

bool SystemManager::initialize() {
    m_state = STATE_INITIALIZING;
    
    Logger::info("=== ESP32WildlifeCAM System Initialization ===");
    Logger::info("Board: %s", BoardDetector::getBoardName(m_boardType));
    
    // Validate hardware configuration first
    if (!validateHardwareConfiguration()) {
        setError("Hardware validation failed");
        enterSafeMode();
        return false;
    }
    
    // Initialize core hardware
    if (!initializeHardware()) {
        setError("Hardware initialization failed");
        enterSafeMode();
        return false;
    }
    
    // Initialize storage (critical for logging)
    if (!initializeStorage()) {
        Logger::warning("Storage initialization failed - limited functionality");
        // Continue without storage for basic operation
    }
    
    // Initialize camera (core functionality)
    if (!initializeCamera()) {
        setError("Camera initialization failed");
        enterSafeMode();
        return false;
    }
    
    // Initialize sensors
    if (!initializeSensors()) {
        Logger::warning("Sensor initialization failed - limited sensing capability");
        // Continue without sensors
    }
    
    // Initialize power management
    if (!initializePowerManagement()) {
        Logger::warning("Power management initialization failed");
        // Continue with basic power monitoring
    }
    
    // Initialize network (optional)
    if (!initializeNetwork()) {
        Logger::warning("Network initialization failed - running in standalone mode");
        // Continue in offline mode
    }
    
    // Initialize tasks
    if (!initializeTasks()) {
        setError("Task initialization failed");
        enterSafeMode();
        return false;
    }
    
    m_initialized = true;
    m_state = STATE_RUNNING;
    
    printSystemInfo();
    Logger::info("=== System initialization complete ===");
    
    return true;
}

bool SystemManager::initializeHardware() {
    Logger::info("Initializing hardware...");
    
    // Configure GPIO pins based on board type
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    // Initialize watchdog timer
    esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
    esp_task_wdt_add(NULL);
    
    // Configure power pins if available
    if (m_pinConfig.battery_pin >= 0) {
        pinMode(m_pinConfig.battery_pin, INPUT);
    }
    
    // Reset watchdog
    esp_task_wdt_reset();
    
    Logger::info("Hardware initialization complete");
    return true;
}

bool SystemManager::initializeCamera() {
    Logger::info("Initializing camera...");
    
    // Camera initialization would go here
    // For now, just validate pins are available
    if (m_pinConfig.cam_xclk < 0 || m_pinConfig.cam_siod < 0) {
        Logger::error("Camera pins not properly configured");
        return false;
    }
    
    // TODO: Implement actual camera initialization
    // This would include configuring the camera module (OV2640)
    // and setting up frame buffers
    
    m_cameraReady = true;
    Logger::info("Camera initialization complete");
    return true;
}

bool SystemManager::initializeStorage() {
    Logger::info("Initializing storage...");
    
    bool success = false;
    
    // Try SD card first
    if (SD_MMC.begin()) {
        uint8_t cardType = SD_MMC.cardType();
        if (cardType != CARD_NONE) {
            uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
            Logger::info("SD Card initialized: %lluMB", cardSize);
            
            // Create required directories
            SD_MMC.mkdir("/wildlife");
            SD_MMC.mkdir("/wildlife/images");
            SD_MMC.mkdir("/wildlife/logs");
            
            success = true;
        } else {
            Logger::warning("SD Card detected but not accessible");
            SD_MMC.end();
        }
    } else {
        Logger::warning("SD Card initialization failed");
    }
    
    // Initialize LittleFS as fallback
    if (!LittleFS.begin()) {
        Logger::warning("LittleFS mount failed, formatting...");
        if (LittleFS.format()) {
            if (LittleFS.begin()) {
                Logger::info("LittleFS formatted and mounted as fallback storage");
                success = true;
            } else {
                Logger::error("LittleFS mount failed after format");
            }
        } else {
            Logger::error("LittleFS format failed");
        }
    } else {
        Logger::info("LittleFS mounted successfully");
        success = true;
    }
    
    m_storageReady = success;
    return success;
}

bool SystemManager::initializeSensors() {
    Logger::info("Initializing sensors...");
    
    // Initialize PIR sensor if available
    if (m_pinConfig.pir_pin >= 0) {
        pinMode(m_pinConfig.pir_pin, INPUT);
        Logger::info("PIR sensor configured on GPIO %d", m_pinConfig.pir_pin);
    }
    
    // TODO: Initialize I2C sensors (BME280, etc.)
    // This would include scanning I2C bus and configuring sensors
    
    m_sensorsReady = true;
    Logger::info("Sensor initialization complete");
    return true;
}

bool SystemManager::initializePowerManagement() {
    Logger::info("Initializing power management...");
    
    // Configure battery monitoring
    if (m_pinConfig.battery_pin >= 0) {
        // Configure ADC for battery monitoring
        analogReadResolution(12); // 12-bit resolution
        Logger::info("Battery monitoring on GPIO %d", m_pinConfig.battery_pin);
    }
    
    // TODO: Initialize power saving features
    // This would include CPU frequency scaling and sleep modes
    
    Logger::info("Power management initialization complete");
    return true;
}

bool SystemManager::initializeNetwork() {
    Logger::info("Initializing network...");
    
    // WiFi initialization would depend on board capabilities
    if (BoardDetector::hasFeature(m_boardType, BoardDetector::FEATURE_WIFI)) {
        // TODO: Initialize WiFi
        Logger::info("WiFi capability detected");
    }
    
    // LoRa initialization (if supported)
    if (BoardDetector::hasFeature(m_boardType, BoardDetector::FEATURE_LORA)) {
        // TODO: Initialize LoRa
        Logger::info("LoRa capability detected");
    }
    
    m_networkReady = false; // Set to true when actually implemented
    return true; // Return true for now to allow offline operation
}

bool SystemManager::initializeTasks() {
    Logger::info("Initializing tasks...");
    
    // TODO: Create FreeRTOS tasks for:
    // - Camera capture
    // - Motion detection
    // - Data logging
    // - Network communication
    // - Power monitoring
    
    Logger::info("Task initialization complete");
    return true;
}

void SystemManager::update() {
    if (!m_initialized || m_state != STATE_RUNNING) {
        return;
    }
    
    unsigned long now = millis();
    
    if (now - m_lastUpdate >= 1000) {
        // Update once per second
        m_lastUpdate = now;
        
        // Reset watchdog
        esp_task_wdt_reset();
        
        // TODO: System health checks
        // TODO: Process motion detection
        // TODO: Update telemetry
        // TODO: Check for low power conditions
        
        // Blink LED to show system is alive
        static bool ledState = false;
        digitalWrite(LED_BUILTIN, ledState);
        ledState = !ledState;
    }
}

void SystemManager::enterSafeMode() {
    m_state = STATE_SAFE_MODE;
    Logger::error("=== ENTERING SAFE MODE ===");
    Logger::error("Reason: %s", m_lastError);
    
    // Minimal system operation
    // Keep only essential functions running
    m_cameraReady = false;
    m_networkReady = false;
    
    // Flash LED in error pattern
    for (int i = 0; i < 10; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
}

bool SystemManager::validateHardwareConfiguration() {
    Logger::info("Validating hardware configuration...");
    
    // Check for pin conflicts
    std::vector<int8_t> usedPins = BoardDetector::getUsedPins(m_boardType);
    
    // Basic validation - ensure camera pins are available
    if (m_pinConfig.cam_xclk < 0 || m_pinConfig.cam_siod < 0 || m_pinConfig.cam_sioc < 0) {
        Logger::error("Critical camera pins not configured");
        return false;
    }
    
    // Check PSRAM if required
    if (BoardDetector::hasFeature(m_boardType, BoardDetector::FEATURE_PSRAM)) {
        if (!psramFound()) {
            Logger::error("PSRAM required but not found");
            return false;
        }
        Logger::info("PSRAM validated: %d bytes", ESP.getPsramSize());
    }
    
    Logger::info("Hardware configuration validated");
    return true;
}

void SystemManager::printSystemInfo() {
    Logger::info("=== System Information ===");
    Logger::info("ESP32 Chip: %s", ESP.getChipModel());
    Logger::info("Chip Revision: %d", ESP.getChipRevision());
    Logger::info("Flash Size: %d bytes", ESP.getFlashChipSize());
    Logger::info("Free Heap: %d bytes", ESP.getFreeHeap());
    
    if (psramFound()) {
        Logger::info("PSRAM Size: %d bytes", ESP.getPsramSize());
        Logger::info("Free PSRAM: %d bytes", ESP.getFreePsram());
    }
    
    Logger::info("Components Status:");
    Logger::info("  Camera: %s", m_cameraReady ? "Ready" : "Not Ready");
    Logger::info("  Storage: %s", m_storageReady ? "Ready" : "Not Ready");
    Logger::info("  Network: %s", m_networkReady ? "Ready" : "Not Ready");
    Logger::info("  Sensors: %s", m_sensorsReady ? "Ready" : "Not Ready");
}

void SystemManager::setError(const char* error) {
    strncpy(m_lastError, error, sizeof(m_lastError) - 1);
    m_lastError[sizeof(m_lastError) - 1] = '\0';
    m_errorCount++;
    m_state = STATE_ERROR;
    Logger::error("System error: %s", error);
}