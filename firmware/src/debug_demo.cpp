/**
 * ESP32 Wildlife Camera - Enhanced Debug System Demonstration
 * 
 * This file demonstrates the capabilities of the new enhanced debug system,
 * showing how it improves upon the basic DEBUG_PRINT macros with:
 * - Categorized logging
 * - Timestamped output
 * - Performance timing
 * - Memory monitoring
 * - Runtime configuration
 * 
 * Author: ESP32WildlifeCAM Project
 * Version: 2.1.0 (Enhanced Debug)
 */

#include "debug_utils.h"
#include "debug_config.h"

/**
 * Demonstrate the enhanced debug system features
 */
void demonstrateEnhancedDebugging() {
    // Initialize the debug system
    EnhancedDebugSystem::init();
    
    DEBUG_SYSTEM_INFO("=== Enhanced Debug System Demo ===");
    
    // 1. Demonstrate categorized logging
    DEBUG_SYSTEM_INFO("1. Categorized Logging:");
    DEBUG_CAMERA_INFO("Camera module initialized successfully");
    DEBUG_MOTION_WARN("Motion sensor calibration needed");
    DEBUG_POWER_ERROR("Battery voltage critically low: 3.1V");
    DEBUG_WIFI_DEBUG("WiFi scan found 5 networks");
    DEBUG_LORA_INFO("LoRa mesh network joined successfully");
    DEBUG_AI_INFO("AI model loaded: wildlife_classifier_v2.tflite");
    
    // 2. Demonstrate performance timing
    DEBUG_SYSTEM_INFO("2. Performance Timing:");
    DEBUG_TIMER_START("image_capture");
    delay(100); // Simulate image capture time
    DEBUG_TIMER_END("image_capture");
    
    DEBUG_TIMER_START("ai_inference");
    delay(50); // Simulate AI processing time
    DEBUG_TIMER_END("ai_inference");
    
    DEBUG_TIMER_START("lora_transmission");
    delay(200); // Simulate LoRa transmission time
    DEBUG_TIMER_END("lora_transmission");
    
    // Print timing results
    DEBUG_TIMER_PRINT();
    
    // 3. Demonstrate memory monitoring
    DEBUG_SYSTEM_INFO("3. Memory Monitoring:");
    DEBUG_PRINT_MEMORY();
    
    // 4. Demonstrate system information
    DEBUG_SYSTEM_INFO("4. System Information:");
    DEBUG_PRINT_SYSTEM();
    
    // 5. Demonstrate runtime configuration
    DEBUG_SYSTEM_INFO("5. Runtime Configuration:");
    DEBUG_SYSTEM_INFO("Available debug presets:");
    DEBUG_SYSTEM_INFO("  - 'debug preset dev' - Development mode (verbose)");
    DEBUG_SYSTEM_INFO("  - 'debug preset prod' - Production mode (minimal)");
    DEBUG_SYSTEM_INFO("  - 'debug preset field' - Field deployment mode");
    DEBUG_SYSTEM_INFO("  - 'debug preset perf' - Performance testing mode");
    DEBUG_SYSTEM_INFO("  - 'debug preset silent' - Emergency mode only");
    
    DEBUG_SYSTEM_INFO("Available debug commands:");
    DEBUG_SYSTEM_INFO("  - 'debug level cam 4' - Set camera debug to level 4");
    DEBUG_SYSTEM_INFO("  - 'debug global 2' - Set all categories to level 2");
    DEBUG_SYSTEM_INFO("  - 'debug status' - Show current configuration");
    DEBUG_SYSTEM_INFO("  - 'debug memory' - Show memory information");
    DEBUG_SYSTEM_INFO("  - 'debug timers' - Show performance timers");
    
    // 6. Demonstrate different debug levels
    DEBUG_SYSTEM_INFO("6. Debug Levels (current category settings):");
    for (int i = 0; i < DEBUG_COUNT; i++) {
        DebugCategory cat = static_cast<DebugCategory>(i);
        DEBUG_SYSTEM_INFO("  %s: %s", 
                         EnhancedDebugSystem::categoryToString(cat),
                         EnhancedDebugSystem::levelToString(EnhancedDebugSystem::getLevel(cat)));
    }
    
    DEBUG_SYSTEM_INFO("=== Demo Complete ===");
}

/**
 * Demonstrate backward compatibility with existing DEBUG macros
 */
void demonstrateBackwardCompatibility() {
    DEBUG_SYSTEM_INFO("=== Backward Compatibility Demo ===");
    
    // These existing macros still work but now use the enhanced system
    DEBUG_PRINT("Old DEBUG_PRINT still works");
    DEBUG_PRINTLN("Old DEBUG_PRINTLN still works");
    DEBUG_PRINTF("Old DEBUG_PRINTF still works: %d\n", 42);
    
    DEBUG_SYSTEM_INFO("All existing DEBUG macros continue to work!");
    DEBUG_SYSTEM_INFO("They now benefit from enhanced features like timestamps");
    DEBUG_SYSTEM_INFO("=== Compatibility Demo Complete ===");
}

/**
 * Demonstrate wildlife camera specific debug scenarios
 */
void demonstrateWildlifeCameraDebugging() {
    DEBUG_SYSTEM_INFO("=== Wildlife Camera Debug Scenarios ===");
    
    // Scenario 1: Motion detection and image capture
    DEBUG_MOTION_INFO("PIR sensor triggered");
    DEBUG_TIMER_START("motion_processing");
    
    DEBUG_MOTION_DEBUG("Checking weather conditions...");
    DEBUG_MOTION_DEBUG("Wind speed: 5 km/h (acceptable)");
    DEBUG_MOTION_DEBUG("Light level: 450 lux (sufficient)");
    DEBUG_MOTION_INFO("Motion validated - proceeding with capture");
    
    DEBUG_CAMERA_INFO("Configuring camera for capture");
    DEBUG_TIMER_START("camera_capture");
    // Simulate camera capture
    delay(50);
    DEBUG_TIMER_END("camera_capture");
    DEBUG_CAMERA_INFO("Image captured: 1920x1080, 245KB");
    
    DEBUG_TIMER_END("motion_processing");
    
    // Scenario 2: AI analysis
    DEBUG_AI_INFO("Starting AI analysis of captured image");
    DEBUG_TIMER_START("ai_analysis");
    
    DEBUG_AI_DEBUG("Loading species classification model");
    DEBUG_AI_DEBUG("Preprocessing image: resize to 224x224");
    DEBUG_AI_DEBUG("Running inference...");
    // Simulate AI processing
    delay(100);
    DEBUG_AI_INFO("Species detected: White-tailed Deer (confidence: 87.3%)");
    DEBUG_AI_INFO("Behavior: Feeding (confidence: 92.1%)");
    
    DEBUG_TIMER_END("ai_analysis");
    
    // Scenario 3: Power management
    DEBUG_POWER_INFO("Checking power status");
    DEBUG_POWER_INFO("Battery: 3.7V (78%)");
    DEBUG_POWER_INFO("Solar panel: 4.2V (charging)");
    DEBUG_POWER_DEBUG("Estimated runtime: 14 hours");
    
    // Scenario 4: LoRa transmission
    DEBUG_LORA_INFO("Transmitting capture data via LoRa mesh");
    DEBUG_TIMER_START("lora_tx");
    
    DEBUG_LORA_DEBUG("Compressing image data: 245KB -> 89KB");
    DEBUG_LORA_DEBUG("Creating mesh packet with metadata");
    DEBUG_LORA_DEBUG("Searching for mesh coordinator...");
    DEBUG_LORA_INFO("Mesh coordinator found: Node ID 1 (RSSI: -67dBm)");
    // Simulate transmission
    delay(150);
    DEBUG_LORA_INFO("Transmission complete: 89KB sent");
    
    DEBUG_TIMER_END("lora_tx");
    
    // Show final performance summary
    DEBUG_SYSTEM_INFO("=== Performance Summary ===");
    DEBUG_TIMER_PRINT();
    DEBUG_PRINT_MEMORY();
    
    DEBUG_SYSTEM_INFO("=== Wildlife Camera Debug Demo Complete ===");
}

/**
 * Main demonstration function
 */
void runDebugSystemDemo() {
    // Run all demonstrations
    demonstrateEnhancedDebugging();
    delay(1000);
    
    demonstrateBackwardCompatibility();
    delay(1000);
    
    demonstrateWildlifeCameraDebugging();
    
    DEBUG_SYSTEM_INFO("Enhanced Debug System demonstration complete!");
    DEBUG_SYSTEM_INFO("Use serial commands to interact with the debug system at runtime.");
}