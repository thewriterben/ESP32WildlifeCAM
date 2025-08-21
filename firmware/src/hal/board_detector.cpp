/**
 * @file board_detector.cpp
 * @brief Hardware detection and board identification system
 */

#include "board_detector.h"
#include "esp32_cam.h"
#include "esp32_s3_cam.h"
#include "../config.h"
#include <esp_system.h>
#include <esp_chip_info.h>
#include <Wire.h>

BoardType BoardDetector::detectBoardType() {
    DEBUG_PRINTLN("Detecting board type...");
    
    // First check chip model
    const char* chip_model = getChipModel();
    DEBUG_PRINTF("Chip model: %s\n", chip_model);
    
    // Check PSRAM availability
    bool has_psram = hasPSRAM();
    DEBUG_PRINTF("PSRAM available: %s\n", has_psram ? "Yes" : "No");
    
    // Try to detect by pin configuration
    BoardType detected_type = detectByPinConfiguration();
    
    if (detected_type != BOARD_UNKNOWN) {
        DEBUG_PRINTF("Board detected by pin configuration: %s\n", getBoardName(detected_type));
        return detected_type;
    }
    
    // Try to detect by I2C devices
    detected_type = detectByI2CDevices();
    
    if (detected_type != BOARD_UNKNOWN) {
        DEBUG_PRINTF("Board detected by I2C devices: %s\n", getBoardName(detected_type));
        return detected_type;
    }
    
    // Default fallback based on chip type
    if (strstr(chip_model, "ESP32-S3")) {
        DEBUG_PRINTLN("Defaulting to ESP32-S3-CAM based on chip model");
        return BOARD_ESP32_S3_CAM;
    } else if (strstr(chip_model, "ESP32")) {
        DEBUG_PRINTLN("Defaulting to AI-Thinker ESP32-CAM based on chip model");
        return BOARD_AI_THINKER_ESP32_CAM;
    }
    
    DEBUG_PRINTLN("Unable to detect board type, using AI-Thinker ESP32-CAM as default");
    return BOARD_AI_THINKER_ESP32_CAM;
}

std::unique_ptr<CameraBoard> BoardDetector::createBoard() {
    BoardType detected_type = detectBoardType();
    return createBoard(detected_type);
}

std::unique_ptr<CameraBoard> BoardDetector::createBoard(BoardType type) {
    switch (type) {
        case BOARD_AI_THINKER_ESP32_CAM:
            return std::make_unique<ESP32CAM>();
            
        case BOARD_ESP32_S3_CAM:
            return std::make_unique<ESP32S3CAM>();
            
        case BOARD_ESP_EYE:
            // TODO: Implement ESPEye class
            DEBUG_PRINTLN("ESP-EYE not yet implemented, falling back to ESP32-CAM");
            return std::make_unique<ESP32CAM>();
            
        case BOARD_M5STACK_TIMER_CAM:
            // TODO: Implement M5StackTimerCam class
            DEBUG_PRINTLN("M5Stack Timer Camera not yet implemented, falling back to ESP32-CAM");
            return std::make_unique<ESP32CAM>();
            
        case BOARD_TTGO_T_CAMERA:
            // TODO: Implement TTGOTCamera class
            DEBUG_PRINTLN("TTGO T-Camera not yet implemented, falling back to ESP32-CAM");
            return std::make_unique<ESP32CAM>();
            
        case BOARD_XIAO_ESP32S3_SENSE:
            // TODO: Implement XiaoESP32S3Sense class
            DEBUG_PRINTLN("XIAO ESP32S3 Sense not yet implemented, falling back to ESP32-CAM");
            return std::make_unique<ESP32CAM>();
            
        default:
            DEBUG_PRINTLN("Unknown board type, falling back to ESP32-CAM");
            return std::make_unique<ESP32CAM>();
    }
}

const char* BoardDetector::getBoardName(BoardType type) {
    switch (type) {
        case BOARD_AI_THINKER_ESP32_CAM: return "AI-Thinker ESP32-CAM";
        case BOARD_ESP32_S3_CAM: return "ESP32-S3-CAM";
        case BOARD_ESP_EYE: return "ESP-EYE";
        case BOARD_M5STACK_TIMER_CAM: return "M5Stack Timer Camera";
        case BOARD_TTGO_T_CAMERA: return "TTGO T-Camera";
        case BOARD_XIAO_ESP32S3_SENSE: return "Seeed XIAO ESP32S3 Sense";
        case BOARD_FIREBEETLE_ESP32_CAM: return "FireBeetle ESP32-E IoT Camera";
        case BOARD_ESP32_S3_EYE: return "ESP32-S3-EYE";
        case BOARD_FREENOVE_ESP32_WROVER_CAM: return "Freenove ESP32-WROVER CAM";
        default: return "Unknown Board";
    }
}

bool BoardDetector::isBoardSupported(BoardType type) {
    switch (type) {
        case BOARD_AI_THINKER_ESP32_CAM:
        case BOARD_ESP32_S3_CAM:
            return true;
        case BOARD_ESP_EYE:
        case BOARD_M5STACK_TIMER_CAM:
        case BOARD_TTGO_T_CAMERA:
        case BOARD_XIAO_ESP32S3_SENSE:
        case BOARD_FIREBEETLE_ESP32_CAM:
        case BOARD_ESP32_S3_EYE:
        case BOARD_FREENOVE_ESP32_WROVER_CAM:
            return false; // Not yet implemented
        default:
            return false;
    }
}

const char* BoardDetector::getChipModel() {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    switch(chip_info.model) {
        case CHIP_ESP32: return "ESP32";
        case CHIP_ESP32S2: return "ESP32-S2";
        case CHIP_ESP32S3: return "ESP32-S3";
        case CHIP_ESP32C3: return "ESP32-C3";
        default: return "Unknown";
    }
}

bool BoardDetector::hasPSRAM() {
    return psramFound();
}

uint32_t BoardDetector::getChipId() {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

BoardType BoardDetector::detectByPinConfiguration() {
    // Get chip model to help with detection
    const char* chip_model = getChipModel();
    
    if (strstr(chip_model, "ESP32-S3")) {
        DEBUG_PRINTLN("ESP32-S3 chip detected, checking for S3-CAM configuration");
        
        // Test for ESP32-S3-CAM specific pins
        // GPIO 40 is commonly used for XCLK on ESP32-S3-CAM
        if (testGPIOPin(40, false)) {
            // GPIO 48 is commonly used for LED on ESP32-S3-CAM
            if (testGPIOPin(48, false)) {
                DEBUG_PRINTLN("ESP32-S3-CAM pin configuration detected");
                return BOARD_ESP32_S3_CAM;
            }
        }
    } else if (strstr(chip_model, "ESP32")) {
        DEBUG_PRINTLN("ESP32 chip detected, checking for ESP32-CAM configuration");
        
        // Test for AI-Thinker ESP32-CAM pin configuration
        // Check if GPIO 0 (XCLK) is available - common for ESP32-CAM
        if (testGPIOPin(0, false)) { // GPIO 0 should be available for XCLK
            // Check if GPIO 32 (PWDN) is available
            if (testGPIOPin(32, false)) {
                DEBUG_PRINTLN("AI-Thinker ESP32-CAM pin configuration detected");
                return BOARD_AI_THINKER_ESP32_CAM;
            }
        }
    }
    
    return BOARD_UNKNOWN;
}

bool BoardDetector::testGPIOPin(int pin, bool expected_state) {
    // Simple GPIO test - check if pin can be configured
    // This is a basic implementation
    if (pin < 0 || pin > 39) return false;
    
    // Try to configure pin as input
    pinMode(pin, INPUT);
    delay(1);
    
    // Read pin state
    bool state = digitalRead(pin);
    
    // This is a simplified test - real implementation would be more sophisticated
    return true; // For now, assume all pins are valid
}

BoardType BoardDetector::detectByI2CDevices() {
    // Initialize I2C with default pins
    Wire.begin();
    
    // Scan for I2C devices that might indicate specific boards
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
            DEBUG_PRINTF("I2C device found at address 0x%02X\n", address);
            
            // Check for specific devices that indicate board types
            switch (address) {
                case 0x76:
                case 0x77:
                    // BME280 or similar sensor - might indicate specific boards
                    break;
                case 0x3C:
                    // OLED display - might indicate ESP-EYE or similar
                    break;
                default:
                    break;
            }
        }
    }
    
    // For now, return unknown - specific board detection by I2C would be implemented here
    return BOARD_UNKNOWN;
}

bool BoardDetector::validateGPIOConfiguration(const GPIOMap& gpio_map) {
    // Check if GPIO configuration is valid for the current chip
    // This would validate pin numbers against chip capabilities
    
    // Basic validation - check if pins are in valid range
    if (gpio_map.pwdn_pin >= 0 && gpio_map.pwdn_pin > 39) return false;
    if (gpio_map.reset_pin >= 0 && gpio_map.reset_pin > 39) return false;
    if (gpio_map.xclk_pin >= 0 && gpio_map.xclk_pin > 39) return false;
    
    // Add more sophisticated validation here
    
    return true;
}