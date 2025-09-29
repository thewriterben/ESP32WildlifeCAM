/**
 * @file main.cpp
 * @brief ESP32WildlifeCAM Main Entry Point - Version 3.2.0
 * @author thewriterben
 * @date 2025-01-01
 * @version 3.2.0
 * 
 * Version 3.2.0 - Advanced Wildlife Monitoring Platform
 * 
 * New Features:
 * 1. Blockchain-based Data Verification
 * 2. Real-time Collaborative Research Platform
 * 3. Advanced Audio Detection and Classification
 * 4. Federated Learning Across Camera Networks
 */

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "config.h"
#include "hardware/board_detector.h"
#include "core/system_manager.h"
#include "utils/logger.h"

// Version 3.2.0 Component Includes
#include "src/blockchain/SmartContractEngine.h"
#include "src/blockchain/BlockchainIntegration.h"
#include "src/research/CollaborativeResearchPlatform.h"
#include "src/audio/wildlife_audio_classifier.h"
#include "src/ai/FederatedLearningCoordinator.h"

// Global system manager instance
SystemManager* g_system = nullptr;

// Version 3.2.0 Global component instances
SmartContractEngine* g_smartContractEngine = nullptr;
CollaborativeResearchPlatform* g_researchPlatform = nullptr;
WildlifeAudioClassifier* g_audioClassifier = nullptr;
FederatedLearningCoordinator* g_federatedCoordinator = nullptr;

/**
 * Version 3.2.0 Enhanced Safe Mode Implementation
 * Provides comprehensive error handling and safe operation when initialization fails
 * Now includes blockchain integrity checks and research platform status
 */
void enterSafeMode() {
    Logger::error("=== ENTERING SAFE MODE v3.2.0 ===");
    
    // Initialize basic hardware for safe mode operation
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Initialize watchdog timer for safety
    esp_task_wdt_init(30, true); // 30 second timeout
    esp_task_wdt_add(NULL);
    
    // Log system state for debugging
    Logger::error("System failed to initialize properly");
    Logger::error("Free heap: %d bytes", ESP.getFreeHeap());
    if (psramFound()) {
        Logger::error("PSRAM: %d bytes", ESP.getPsramSize());
    }
    
    // Version 3.2.0: Check component states
    if (g_smartContractEngine) {
        Logger::error("Smart Contract Engine: %s", 
                     g_smartContractEngine->isInitialized() ? "Initialized" : "Failed");
    }
    if (g_researchPlatform) {
        Logger::error("Research Platform: %s", 
                     g_researchPlatform->isInitialized() ? "Initialized" : "Failed");
    }
    if (g_federatedCoordinator) {
        Logger::error("Federated Learning: %s", 
                     g_federatedCoordinator->isInitialized() ? "Initialized" : "Failed");
    }
    
    // Shutdown non-essential peripherals for safety
    // Disable WiFi if it was initialized
    WiFi.mode(WIFI_OFF);
    
    // Enter safe mode loop with enhanced functionality
    unsigned long lastErrorBlink = 0;
    unsigned long lastStatusReport = 0;
    unsigned long lastWatchdogReset = 0;
    int blinkPhase = 0;
    int resetAttempts = 0;
    const int MAX_RESET_ATTEMPTS = 3;
    
    Logger::error("Safe mode active - SOS LED pattern will indicate system error");
    Logger::error("Send 'r' via serial to attempt system reset");
    Logger::error("Send 's' via serial to view system status");
    
    while (true) {
        unsigned long now = millis();
        
        // Reset watchdog timer regularly
        if (now - lastWatchdogReset > 10000) { // Every 10 seconds
            esp_task_wdt_reset();
            lastWatchdogReset = now;
        }
        
        // SOS LED pattern (... --- ...)
        if (now - lastErrorBlink > 200) {
            switch (blinkPhase) {
                case 0: case 2: case 4: // Short blinks (.)
                    digitalWrite(LED_BUILTIN, HIGH);
                    lastErrorBlink = now;
                    blinkPhase++;
                    break;
                case 1: case 3: case 5: // Short pauses
                    digitalWrite(LED_BUILTIN, LOW);
                    lastErrorBlink = now;
                    blinkPhase++;
                    break;
                case 6: case 8: case 10: // Long blinks (-)
                    digitalWrite(LED_BUILTIN, HIGH);
                    if (now - lastErrorBlink > 600) {
                        lastErrorBlink = now;
                        blinkPhase++;
                    }
                    break;
                case 7: case 9: case 11: // Short pauses
                    digitalWrite(LED_BUILTIN, LOW);
                    lastErrorBlink = now;
                    blinkPhase++;
                    break;
                case 12: case 14: case 16: // Short blinks (.)
                    digitalWrite(LED_BUILTIN, HIGH);
                    lastErrorBlink = now;
                    blinkPhase++;
                    break;
                case 13: case 15: case 17: // Short pauses
                    digitalWrite(LED_BUILTIN, LOW);
                    lastErrorBlink = now;
                    blinkPhase++;
                    break;
                default: // Long pause before repeating
                    digitalWrite(LED_BUILTIN, LOW);
                    if (now - lastErrorBlink > 2000) {
                        blinkPhase = 0;
                        lastErrorBlink = now;
                    }
                    break;
            }
        }
        
        // Periodic status report
        if (now - lastStatusReport > 30000) { // Every 30 seconds
            Logger::error("Safe mode status - Free heap: %d bytes, Uptime: %lu ms", 
                         ESP.getFreeHeap(), now);
            lastStatusReport = now;
        }
        
        // Check for serial commands
        if (Serial.available()) {
            String command = Serial.readStringUntil('\n');
            command.trim();
            command.toLowerCase();
            
            if (command == "r" || command == "reset") {
                if (resetAttempts < MAX_RESET_ATTEMPTS) {
                    resetAttempts++;
                    Logger::error("Attempting system reset (attempt %d/%d)", resetAttempts, MAX_RESET_ATTEMPTS);
                    
                    // Clean up system manager if it exists
                    if (g_system) {
                        delete g_system;
                        g_system = nullptr;
                    }
                    
                    // Attempt to restart system
                    delay(1000);
                    ESP.restart();
                } else {
                    Logger::error("Maximum reset attempts reached - staying in safe mode");
                }
            } else if (command == "s" || command == "status") {
                Logger::error("=== SAFE MODE STATUS ===");
                Logger::error("  System State: SAFE_MODE");
                Logger::error("  Free Heap: %d bytes", ESP.getFreeHeap());
                Logger::error("  PSRAM: %s", psramFound() ? "Available" : "Not found");
                Logger::error("  Reset Attempts: %d/%d", resetAttempts, MAX_RESET_ATTEMPTS);
                Logger::error("  Uptime: %lu ms", now);
                if (g_system) {
                    Logger::error("  Last Error: %s", g_system->getLastError());
                }
                Logger::error("========================");
            } else if (command == "h" || command == "help") {
                Logger::error("=== SAFE MODE COMMANDS ===");
                Logger::error("  r, reset - Attempt system reset");
                Logger::error("  s, status - Show system status");
                Logger::error("  h, help - Show this help");
                Logger::error("==========================");
            }
        }
        
        // Small delay to prevent excessive CPU usage
        delay(50);
    }
}

/**
 * Version 3.2.0 Enhanced System Initialization
 * Initializes all Version 3.2.0 components in correct order
 */
bool initializeVersion32Components() {
    Logger::info("Initializing Version 3.2.0 components...");
    
    // 1. Initialize Smart Contract Engine
    g_smartContractEngine = new SmartContractEngine();
    if (!g_smartContractEngine->initialize()) {
        Logger::error("Failed to initialize Smart Contract Engine");
        return false;
    }
    Logger::info("✓ Smart Contract Engine initialized");
    
    // 2. Initialize Collaborative Research Platform
    g_researchPlatform = new CollaborativeResearchPlatform();
    if (!g_researchPlatform->initialize(81)) { // WebSocket on port 81
        Logger::error("Failed to initialize Collaborative Research Platform");
        return false;
    }
    Logger::info("✓ Collaborative Research Platform initialized");
    
    // 3. Initialize Wildlife Audio Classifier
    g_audioClassifier = new WildlifeAudioClassifier();
    if (!g_audioClassifier->initialize()) {
        Logger::error("Failed to initialize Wildlife Audio Classifier");
        return false;
    }
    Logger::info("✓ Wildlife Audio Classifier initialized");
    
    // 4. Initialize Federated Learning Coordinator
    g_federatedCoordinator = new FederatedLearningCoordinator();
    if (!g_federatedCoordinator->initialize()) {
        Logger::error("Failed to initialize Federated Learning Coordinator");
        return false;
    }
    Logger::info("✓ Federated Learning Coordinator initialized");
    
    Logger::info("All Version 3.2.0 components initialized successfully!");
    return true;
}

/**
 * Version 3.2.0 Component Cleanup
 * Properly shuts down all Version 3.2.0 components
 */
void cleanupVersion32Components() {
    Logger::info("Cleaning up Version 3.2.0 components...");
    
    if (g_federatedCoordinator) {
        g_federatedCoordinator->shutdown();
        delete g_federatedCoordinator;
        g_federatedCoordinator = nullptr;
    }
    
    if (g_audioClassifier) {
        delete g_audioClassifier;
        g_audioClassifier = nullptr;
    }
    
    if (g_researchPlatform) {
        g_researchPlatform->shutdown();
        delete g_researchPlatform;
        g_researchPlatform = nullptr;
    }
    
    if (g_smartContractEngine) {
        g_smartContractEngine->shutdown();
        delete g_smartContractEngine;
        g_smartContractEngine = nullptr;
    }
    
    Logger::info("Version 3.2.0 components cleanup complete");
}

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        ; // Wait for serial port to connect
    }
    
    // Print startup banner
    Logger::info("===================================");
    Logger::info("ESP32WildlifeCAM v3.2.0");
    Logger::info("Advanced Wildlife Monitoring Platform");
    Logger::info("Build: " __DATE__ " " __TIME__);
    Logger::info("Author: thewriterben");
    Logger::info("===================================");
    Logger::info("New in v3.2.0:");
    Logger::info("- Blockchain-based Data Verification");
    Logger::info("- Real-time Collaborative Research Platform");
    Logger::info("- Advanced Audio Detection & Classification");
    Logger::info("- Federated Learning Across Camera Networks");
    Logger::info("===================================");
    
    // Detect board type
    BoardDetector::BoardType board = BoardDetector::detectBoard();
    Logger::info("Detected board: %s", BoardDetector::getBoardName(board));
    
    // Create and initialize system manager
    g_system = new SystemManager(board);
    if (!g_system->initialize()) {
        Logger::error("Failed to initialize system!");
        Logger::error("Last error: %s", g_system->getLastError());
        Logger::error("Entering safe mode...");
        
        // Enter comprehensive safe mode
        enterSafeMode();
    }
    
    // Initialize Version 3.2.0 components
    if (!initializeVersion32Components()) {
        Logger::error("Failed to initialize Version 3.2.0 components!");
        Logger::error("Entering safe mode...");
        
        // Cleanup what we can
        cleanupVersion32Components();
        
        // Enter safe mode
        enterSafeMode();
    }
    
    Logger::info("System initialization complete!");
    Logger::info("Free heap: %d bytes", ESP.getFreeHeap());
    if (psramFound()) {
        Logger::info("PSRAM: %d bytes", ESP.getPsramSize());
    }
    
    Logger::info("Version 3.2.0 Wildlife Monitoring Platform ready!");
}

void loop() {
    // Main system loop
    if (g_system) {
        g_system->update();
    }
    
    // Version 3.2.0 component updates
    if (g_researchPlatform && g_researchPlatform->isInitialized()) {
        g_researchPlatform->update();
    }
    
    if (g_federatedCoordinator && g_federatedCoordinator->isInitialized()) {
        g_federatedCoordinator->update();
    }
    
    // Yield to other tasks
    yield();
}