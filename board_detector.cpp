/**
 * @file board_detector.cpp
 * @brief Board detection implementation
 * @author thewriterben
 * @date 2025-08-31
 */

#include "board_detector.h"
#include "../utils/logger.h"
#include <soc/efuse_reg.h>

// Static member initialization
BoardDetector::BoardType BoardDetector::s_detectedBoard = BoardDetector::UNKNOWN;
std::map<BoardDetector::BoardType, BoardDetector::PinConfig> BoardDetector::s_boardConfigs;
std::map<BoardDetector::BoardType, std::vector<BoardDetector::Feature>> BoardDetector::s_boardFeatures;

void BoardDetector::initializeBoardConfigs() {
    // AI-Thinker ESP32-CAM configuration
    PinConfig aiThinkerConfig;
    aiThinkerConfig.cam_pwdn = 32;
    aiThinkerConfig.cam_reset = -1;
    aiThinkerConfig.cam_xclk = 0;
    aiThinkerConfig.cam_siod = 26;
    aiThinkerConfig.cam_sioc = 27;
    aiThinkerConfig.cam_y9 = 35;
    aiThinkerConfig.cam_y8 = 34;
    aiThinkerConfig.cam_y7 = 39;
    aiThinkerConfig.cam_y6 = 36;
    aiThinkerConfig.cam_y5 = 21;
    aiThinkerConfig.cam_y4 = 19;
    aiThinkerConfig.cam_y3 = 18;
    aiThinkerConfig.cam_y2 = 5;
    aiThinkerConfig.cam_vsync = 25;
    aiThinkerConfig.cam_href = 23;
    aiThinkerConfig.cam_pclk = 22;
    
    // SD card (shares SPI with LoRa - needs careful management)
    aiThinkerConfig.sd_cs = 13;
    aiThinkerConfig.sd_mosi = 15;
    aiThinkerConfig.sd_miso = 2;
    aiThinkerConfig.sd_clk = 14;
    
    // LoRa module
    aiThinkerConfig.lora_cs = 12;
    aiThinkerConfig.lora_rst = 16;
    aiThinkerConfig.lora_irq = 4;
    aiThinkerConfig.lora_mosi = 15; // Shared with SD
    aiThinkerConfig.lora_miso = 2;  // Shared with SD
    aiThinkerConfig.lora_sck = 14;  // Shared with SD
    
    // Other peripherals
    aiThinkerConfig.pir_pin = 33;
    aiThinkerConfig.led_pin = 4;
    aiThinkerConfig.battery_pin = 35; // ADC1_CH7
    aiThinkerConfig.i2c_sda = 21;
    aiThinkerConfig.i2c_scl = 22;
    
    s_boardConfigs[AI_THINKER_CAM] = aiThinkerConfig;
    
    // AI-Thinker features
    s_boardFeatures[AI_THINKER_CAM] = {
        FEATURE_CAMERA,
        FEATURE_SD_CARD,
        FEATURE_PSRAM,
        FEATURE_WIFI,
        FEATURE_BLUETOOTH
    };
    
    // TODO: Add ESP32-S3-CAM configuration
    // TODO: Add TTGO T-Camera configuration
}

BoardDetector::BoardType BoardDetector::detectBoard() {
    if (s_detectedBoard != UNKNOWN) {
        return s_detectedBoard;
    }
    
    initializeBoardConfigs();
    
    // Try detection methods in order
    s_detectedBoard = detectByChipModel();
    if (s_detectedBoard != UNKNOWN) {
        return s_detectedBoard;
    }
    
    s_detectedBoard = detectByPSRAM();
    if (s_detectedBoard != UNKNOWN) {
        return s_detectedBoard;
    }
    
    s_detectedBoard = detectByGPIO();
    
    return s_detectedBoard;
}

BoardDetector::BoardType BoardDetector::detectByChipModel() {
    uint32_t chip_model = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_VER_PKG);
    uint32_t chip_revision = ESP.getChipRevision();
    
    Logger::debug("Chip model: 0x%X, revision: %d", chip_model, chip_revision);
    
    // Check for specific board identifiers
    #ifdef BOARD_AI_THINKER_CAM
        return AI_THINKER_CAM;
    #elif defined(BOARD_ESP32S3_CAM)
        return ESP32_S3_CAM;
    #elif defined(BOARD_TTGO_T_CAMERA)
        return TTGO_T_CAMERA;
    #endif
    
    return UNKNOWN;
}

BoardDetector::BoardType BoardDetector::detectByPSRAM() {
    if (psramFound()) {
        size_t psram_size = ESP.getPsramSize();
        Logger::debug("PSRAM found: %d bytes", psram_size);
        
        // AI-Thinker typically has 4MB PSRAM
        if (psram_size >= 4 * 1024 * 1024) {
            return AI_THINKER_CAM;
        }
    }
    
    return UNKNOWN;
}

BoardDetector::BoardType BoardDetector::detectByGPIO() {
    // Try to detect based on GPIO states
    // This is less reliable but can work as fallback
    
    // Check if camera pins are accessible
    pinMode(0, INPUT);  // CAM_XCLK on AI-Thinker
    pinMode(26, INPUT); // CAM_SIOD on AI-Thinker
    pinMode(27, INPUT); // CAM_SIOC on AI-Thinker
    
    // AI-Thinker specific: GPIO4 has the flash LED
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);
    
    return AI_THINKER_CAM; // Default assumption
}

const char* BoardDetector::getBoardName(BoardType board) {
    switch (board) {
        case AI_THINKER_CAM: return "AI-Thinker ESP32-CAM";
        case ESP32_S3_CAM: return "ESP32-S3-CAM";
        case TTGO_T_CAMERA: return "TTGO T-Camera";
        case CUSTOM: return "Custom Board";
        default: return "Unknown Board";
    }
}

bool BoardDetector::hasFeature(BoardType board, Feature feature) {
    auto it = s_boardFeatures.find(board);
    if (it != s_boardFeatures.end()) {
        const auto& features = it->second;
        return std::find(features.begin(), features.end(), feature) != features.end();
    }
    return false;
}

bool BoardDetector::hasFeature(Feature feature) {
    if (s_detectedBoard == UNKNOWN) {
        detectBoard();
    }
    return hasFeature(s_detectedBoard, feature);
}

BoardDetector::PinConfig BoardDetector::getPinConfig(BoardType board) {
    auto it = s_boardConfigs.find(board);
    if (it != s_boardConfigs.end()) {
        return it->second;
    }
    return PinConfig(); // Return default config
}

BoardDetector::PinConfig BoardDetector::getPinConfig() {
    if (s_detectedBoard == UNKNOWN) {
        detectBoard();
    }
    return getPinConfig(s_detectedBoard);
}

std::vector<int8_t> BoardDetector::getUsedPins(BoardType board) {
    std::vector<int8_t> used_pins;
    PinConfig config = getPinConfig(board);
    
    // Add all configured pins to the used list
    if (config.cam_pwdn >= 0) used_pins.push_back(config.cam_pwdn);
    if (config.cam_xclk >= 0) used_pins.push_back(config.cam_xclk);
    if (config.cam_siod >= 0) used_pins.push_back(config.cam_siod);
    if (config.cam_sioc >= 0) used_pins.push_back(config.cam_sioc);
    // ... add all other pins
    
    // Remove duplicates
    std::sort(used_pins.begin(), used_pins.end());
    used_pins.erase(std::unique(used_pins.begin(), used_pins.end()), used_pins.end());
    
    return used_pins;
}

std::vector<int8_t> BoardDetector::getAvailablePins(BoardType board) {
    std::vector<int8_t> all_pins;
    std::vector<int8_t> available_pins;
    
    // Define available GPIO pins for ESP32
    for (int i = 0; i <= 39; i++) {
        // Skip pins that are not available
        if (i == 20 || i == 24 || (i >= 28 && i <= 31)) continue;
        // Skip input-only pins for general use
        if (i >= 34 && i <= 39) continue;
        all_pins.push_back(i);
    }
    
    std::vector<int8_t> used = getUsedPins(board);
    
    // Find pins not in the used list
    for (int8_t pin : all_pins) {
        if (std::find(used.begin(), used.end(), pin) == used.end()) {
            available_pins.push_back(pin);
        }
    }
    
    return available_pins;
}