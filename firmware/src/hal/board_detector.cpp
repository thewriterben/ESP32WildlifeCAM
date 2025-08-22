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
#include <vector>

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
            // ESP-EYE implementation not yet completed, falling back to ESP32-CAM
            DEBUG_PRINTLN("ESP-EYE not yet implemented, falling back to ESP32-CAM");
            return std::make_unique<ESP32CAM>();
            
        case BOARD_M5STACK_TIMER_CAM:
            // M5Stack Timer Camera implementation not yet completed, falling back to ESP32-CAM
            DEBUG_PRINTLN("M5Stack Timer Camera not yet implemented, falling back to ESP32-CAM");
            return std::make_unique<ESP32CAM>();
            
        case BOARD_TTGO_T_CAMERA:
            // TTGO T-Camera implementation not yet completed, falling back to ESP32-CAM
            DEBUG_PRINTLN("TTGO T-Camera not yet implemented, falling back to ESP32-CAM");
            return std::make_unique<ESP32CAM>();
            
        case BOARD_XIAO_ESP32S3_SENSE:
            // XIAO ESP32S3 Sense implementation not yet completed, falling back to ESP32-CAM
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
    // Comprehensive GPIO pin testing
    if (pin < 0 || pin > 39) {
        DEBUG_PRINTF("GPIO pin %d out of range (0-39)\n", pin);
        return false;
    }
    
    // Skip testing input-only pins that cannot be configured as outputs
    if (pin >= 34 && pin <= 39) {
        // These are input-only pins, just check if they can be read
        pinMode(pin, INPUT);
        delay(1);
        digitalRead(pin); // Test read capability
        return true;
    }
    
    // Test if pin can be configured and responds correctly
    uint8_t original_mode = digitalRead(pin);
    
    // Try to configure pin as output and test
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    delay(1);
    bool high_state = digitalRead(pin);
    
    digitalWrite(pin, LOW);
    delay(1);
    bool low_state = digitalRead(pin);
    
    // Restore original configuration
    pinMode(pin, INPUT);
    
    // Pin is working if we can set high/low states
    bool pin_functional = (high_state == HIGH && low_state == LOW);
    
    if (!pin_functional) {
        DEBUG_PRINTF("GPIO pin %d failed functionality test\n", pin);
    }
    
    return pin_functional;
}

BoardType BoardDetector::detectByI2CDevices() {
    // Initialize I2C with default pins first
    Wire.begin();
    delay(100); // Allow I2C to stabilize
    
    DEBUG_PRINTLN("Scanning for I2C devices...");
    bool deviceFound = false;
    
    // Scan for I2C devices that might indicate specific boards
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
            deviceFound = true;
            DEBUG_PRINTF("I2C device found at address 0x%02X\n", address);
            
            // Check for specific devices that indicate board types
            switch (address) {
                case 0x76:
                case 0x77:
                    // BME280 or BMP280 sensor - common on ESP32-S3-CAM boards
                    DEBUG_PRINTLN("BME280/BMP280 sensor detected");
                    break;
                case 0x3C:
                case 0x3D:
                    // OLED display - common on ESP-EYE boards
                    DEBUG_PRINTLN("OLED display detected - possible ESP-EYE board");
                    return BOARD_ESP_EYE;
                case 0x68:
                    // MPU6050 accelerometer - might indicate M5Stack boards
                    DEBUG_PRINTLN("MPU6050 accelerometer detected");
                    break;
                case 0x21:
                    // Camera sensor OV2640 address
                    DEBUG_PRINTLN("Camera sensor I2C detected");
                    break;
                default:
                    DEBUG_PRINTF("Unknown I2C device at 0x%02X\n", address);
                    break;
            }
        }
    }
    
    if (!deviceFound) {
        DEBUG_PRINTLN("No I2C devices found");
    }
    
    // For now, return unknown - more sophisticated board detection by I2C device combinations would be implemented here
    return BOARD_UNKNOWN;
}

bool BoardDetector::validateGPIOConfiguration(const GPIOMap& gpio_map) {
    // Check if GPIO configuration is valid for the current chip
    // Comprehensive validation of pin numbers against ESP32 capabilities
    
    // Helper function to validate a single pin
    auto validatePin = [](int pin, const char* pin_name) -> bool {
        if (pin < 0) return true; // Negative pins are "not used" indicators
        if (pin > 39) {
            DEBUG_PRINTF("ERROR: %s pin %d exceeds maximum GPIO (39)\n", pin_name, pin);
            return false;
        }
        // Additional ESP32-specific validations
        if (pin == 6 || pin == 7 || pin == 8 || pin == 9 || pin == 10 || pin == 11) {
            DEBUG_PRINTF("WARNING: %s pin %d is connected to flash and may cause issues\n", pin_name, pin);
            return false; // These pins are typically used for flash memory
        }
        return true;
    };
    
    // Validate all pins in the GPIO map
    bool valid = true;
    valid &= validatePin(gpio_map.pwdn_pin, "PWDN");
    valid &= validatePin(gpio_map.reset_pin, "RESET");
    valid &= validatePin(gpio_map.xclk_pin, "XCLK");
    valid &= validatePin(gpio_map.siod_pin, "SIOD");
    valid &= validatePin(gpio_map.sioc_pin, "SIOC");
    valid &= validatePin(gpio_map.led_pin, "LED");
    valid &= validatePin(gpio_map.flash_pin, "FLASH");
    
    // Validate camera data pins
    valid &= validatePin(gpio_map.y9_pin, "Y9");
    valid &= validatePin(gpio_map.y8_pin, "Y8");
    valid &= validatePin(gpio_map.y7_pin, "Y7");
    valid &= validatePin(gpio_map.y6_pin, "Y6");
    valid &= validatePin(gpio_map.y5_pin, "Y5");
    valid &= validatePin(gpio_map.y4_pin, "Y4");
    valid &= validatePin(gpio_map.y3_pin, "Y3");
    valid &= validatePin(gpio_map.y2_pin, "Y2");
    valid &= validatePin(gpio_map.vsync_pin, "VSYNC");
    valid &= validatePin(gpio_map.href_pin, "HREF");
    valid &= validatePin(gpio_map.pclk_pin, "PCLK");
    
    // Check for pin conflicts (same pin used for multiple functions)
    std::vector<int> used_pins;
    auto addPin = [&used_pins](int pin, const char* name) -> bool {
        if (pin < 0) return true; // Skip unused pins
        for (int used_pin : used_pins) {
            if (used_pin == pin) {
                DEBUG_PRINTF("ERROR: Pin conflict - GPIO %d used for multiple functions including %s\n", pin, name);
                return false;
            }
        }
        used_pins.push_back(pin);
        return true;
    };
    
    valid &= addPin(gpio_map.pwdn_pin, "PWDN");
    valid &= addPin(gpio_map.reset_pin, "RESET");
    valid &= addPin(gpio_map.xclk_pin, "XCLK");
    valid &= addPin(gpio_map.siod_pin, "SIOD");
    valid &= addPin(gpio_map.sioc_pin, "SIOC");
    valid &= addPin(gpio_map.led_pin, "LED");
    // Note: flash_pin might be same as led_pin on some boards, so don't check conflicts for it
    
    if (valid) {
        DEBUG_PRINTLN("GPIO configuration validation passed");
    } else {
        DEBUG_PRINTLN("GPIO configuration validation failed");
    }
    
    return valid;
}