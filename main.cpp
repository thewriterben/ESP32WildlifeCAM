/**
 * @file main.cpp
 * @brief ESP32WildlifeCAM Main Entry Point
 * @author thewriterben
 * @date 2025-08-31
 * @version 0.1.0
 */

#include <Arduino.h>
#include "config.h"
#include "hardware/board_detector.h"
#include "core/system_manager.h"
#include "utils/logger.h"

// Global system manager instance
SystemManager* g_system = nullptr;

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        ; // Wait for serial port to connect
    }
    
    // Print startup banner
    Logger::info("===================================");
    Logger::info("ESP32WildlifeCAM v" VERSION);
    Logger::info("Build: " __DATE__ " " __TIME__);
    Logger::info("Author: thewriterben");
    Logger::info("===================================");
    
    // Detect board type
    BoardDetector::BoardType board = BoardDetector::detectBoard();
    Logger::info("Detected board: %s", BoardDetector::getBoardName(board));
    
    // Create and initialize system manager
    g_system = new SystemManager(board);
    if (!g_system->initialize()) {
        Logger::error("Failed to initialize system!");
        Logger::error("Entering safe mode...");
        // TODO: Implement safe mode
        while (1) {
            delay(1000);
        }
    }
    
    Logger::info("System initialization complete!");
    Logger::info("Free heap: %d bytes", ESP.getFreeHeap());
    if (psramFound()) {
        Logger::info("PSRAM: %d bytes", ESP.getPsramSize());
    }
}

void loop() {
    // Main system loop
    if (g_system) {
        g_system->update();
    }
    
    // Yield to other tasks
    yield();
}