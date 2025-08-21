#include "board_detector.h"
#include "../src/board_abstraction.h"
#include <esp_system.h>
#include <esp_chip_info.h>
#include <driver/gpio.h>
#include <esp_log.h>

static const char* TAG = "BOARD_DETECTOR";

// Structure to hold board detection criteria
typedef struct {
    board_type_t board_type;
    const char* board_name;
    uint32_t chip_features;  // Expected ESP32 chip features
    int test_gpio_pin;       // GPIO pin to test for board identification
    bool expected_gpio_state; // Expected state of test GPIO
    const char* detection_method;
} board_detection_criteria_t;

// Board detection criteria table
static const board_detection_criteria_t detection_table[] = {
    {
        .board_type = BOARD_ESP32_CAM,
        .board_name = "ESP32-CAM (AI-Thinker)",
        .chip_features = CHIP_FEATURE_WIFI_BGN | CHIP_FEATURE_BT,
        .test_gpio_pin = 33,  // Flash LED pin for AI-Thinker ESP32-CAM
        .expected_gpio_state = false, // Should be pulled low initially
        .detection_method = "GPIO state test"
    },
    {
        .board_type = BOARD_ESP32_S3_CAM,
        .board_name = "ESP32-S3-CAM",
        .chip_features = CHIP_FEATURE_WIFI_BGN,
        .test_gpio_pin = 48,  // Flash LED pin for ESP32-S3-CAM
        .expected_gpio_state = false,
        .detection_method = "Chip model + GPIO test"
    },
    {
        .board_type = BOARD_ESP_EYE,
        .board_name = "ESP-EYE (Espressif)",
        .chip_features = CHIP_FEATURE_WIFI_BGN | CHIP_FEATURE_BT,
        .test_gpio_pin = 22,  // Specific to ESP-EYE
        .expected_gpio_state = true,
        .detection_method = "Espressif signature + GPIO"
    },
    {
        .board_type = BOARD_M5_TIMER_CAM,
        .board_name = "M5Stack Timer Camera",
        .chip_features = CHIP_FEATURE_WIFI_BGN | CHIP_FEATURE_BT,
        .test_gpio_pin = 2,   // M5Stack LED pin
        .expected_gpio_state = false,
        .detection_method = "M5Stack signature"
    },
    {
        .board_type = BOARD_TTGO_T_CAMERA,
        .board_name = "TTGO T-Camera",
        .chip_features = CHIP_FEATURE_WIFI_BGN | CHIP_FEATURE_BT,
        .test_gpio_pin = 4,   // TTGO flash LED pin
        .expected_gpio_state = false,
        .detection_method = "TTGO GPIO pattern"
    },
    {
        .board_type = BOARD_XIAO_ESP32S3_SENSE,
        .board_name = "XIAO ESP32S3 Sense",
        .chip_features = CHIP_FEATURE_WIFI_BGN,
        .test_gpio_pin = 21,  // XIAO specific pin
        .expected_gpio_state = true,
        .detection_method = "XIAO form factor detection"
    }
};

static const size_t detection_table_size = sizeof(detection_table) / sizeof(detection_table[0]);

// Internal helper functions
static bool test_gpio_pin(int pin, bool expected_state);
static bool check_chip_features(uint32_t expected_features);
static board_type_t detect_by_chip_model(void);
static board_type_t detect_by_gpio_signature(void);
static void log_detection_info(void);

board_type_t board_auto_detect(void) {
    ESP_LOGI(TAG, "Starting automatic board detection...");
    
    // Log system information for debugging
    log_detection_info();
    
    // Try detection by chip model first
    board_type_t detected_board = detect_by_chip_model();
    if (detected_board != BOARD_UNKNOWN) {
        ESP_LOGI(TAG, "Board detected by chip model: %s", 
                 board_get_name(detected_board));
        return detected_board;
    }
    
    // Fall back to GPIO signature detection
    detected_board = detect_by_gpio_signature();
    if (detected_board != BOARD_UNKNOWN) {
        ESP_LOGI(TAG, "Board detected by GPIO signature: %s", 
                 board_get_name(detected_board));
        return detected_board;
    }
    
    // If no specific board detected, default to ESP32-CAM
    ESP_LOGW(TAG, "Could not detect specific board, defaulting to ESP32-CAM");
    return BOARD_ESP32_CAM;
}

bool board_validate_detection(board_type_t detected_board) {
    if (detected_board == BOARD_UNKNOWN) {
        return false;
    }
    
    // Find the detection criteria for this board
    const board_detection_criteria_t* criteria = NULL;
    for (size_t i = 0; i < detection_table_size; i++) {
        if (detection_table[i].board_type == detected_board) {
            criteria = &detection_table[i];
            break;
        }
    }
    
    if (!criteria) {
        ESP_LOGW(TAG, "No validation criteria for board type %d", detected_board);
        return true; // Assume valid if no criteria
    }
    
    // Validate chip features
    if (!check_chip_features(criteria->chip_features)) {
        ESP_LOGE(TAG, "Chip features don't match expected for %s", criteria->board_name);
        return false;
    }
    
    // Validate GPIO signature
    if (!test_gpio_pin(criteria->test_gpio_pin, criteria->expected_gpio_state)) {
        ESP_LOGW(TAG, "GPIO signature test failed for %s", criteria->board_name);
        // Don't fail validation on GPIO test as it might be inconclusive
    }
    
    ESP_LOGI(TAG, "Board validation successful for %s", criteria->board_name);
    return true;
}

const char* board_get_detection_info(board_type_t board_type) {
    for (size_t i = 0; i < detection_table_size; i++) {
        if (detection_table[i].board_type == board_type) {
            return detection_table[i].detection_method;
        }
    }
    return "Unknown detection method";
}

// Internal helper function implementations
static bool test_gpio_pin(int pin, bool expected_state) {
    if (pin < 0) {
        return true; // Skip test for invalid pins
    }
    
    // Configure pin as input with pullup
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << pin),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to configure GPIO %d for testing", pin);
        return false;
    }
    
    // Read pin state
    int pin_state = gpio_get_level((gpio_num_t)pin);
    bool state_matches = (pin_state == 1) == expected_state;
    
    ESP_LOGD(TAG, "GPIO %d test: expected=%d, actual=%d, match=%s", 
             pin, expected_state, pin_state, state_matches ? "YES" : "NO");
    
    return state_matches;
}

static bool check_chip_features(uint32_t expected_features) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    // Check if all expected features are present
    bool features_match = (chip_info.features & expected_features) == expected_features;
    
    ESP_LOGD(TAG, "Chip features: expected=0x%08X, actual=0x%08X, match=%s",
             expected_features, chip_info.features, features_match ? "YES" : "NO");
    
    return features_match;
}

static board_type_t detect_by_chip_model(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    ESP_LOGD(TAG, "Chip model: %d, revision: %d, cores: %d", 
             chip_info.model, chip_info.revision, chip_info.cores);
    
    // ESP32-S3 based boards
    if (chip_info.model == CHIP_ESP32S3) {
        // Could be ESP32-S3-CAM or XIAO ESP32S3 Sense
        // Need additional GPIO testing to distinguish
        if (test_gpio_pin(48, false)) {
            return BOARD_ESP32_S3_CAM;
        } else if (test_gpio_pin(21, true)) {
            return BOARD_XIAO_ESP32S3_SENSE;
        }
        return BOARD_ESP32_S3_CAM; // Default to ESP32-S3-CAM
    }
    
    // ESP32 classic based boards
    if (chip_info.model == CHIP_ESP32) {
        // Most boards use ESP32 classic, need GPIO testing
        return BOARD_UNKNOWN; // Let GPIO detection handle it
    }
    
    return BOARD_UNKNOWN;
}

static board_type_t detect_by_gpio_signature(void) {
    // Test each board's GPIO signature
    for (size_t i = 0; i < detection_table_size; i++) {
        const board_detection_criteria_t* criteria = &detection_table[i];
        
        // Skip if chip features don't match
        if (!check_chip_features(criteria->chip_features)) {
            continue;
        }
        
        // Test GPIO signature
        if (test_gpio_pin(criteria->test_gpio_pin, criteria->expected_gpio_state)) {
            ESP_LOGI(TAG, "GPIO signature match found for %s", criteria->board_name);
            return criteria->board_type;
        }
    }
    
    return BOARD_UNKNOWN;
}

static void log_detection_info(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    ESP_LOGI(TAG, "=== Board Detection Info ===");
    ESP_LOGI(TAG, "Chip: ESP32 model %d", chip_info.model);
    ESP_LOGI(TAG, "Cores: %d", chip_info.cores);
    ESP_LOGI(TAG, "Revision: %d", chip_info.revision);
    ESP_LOGI(TAG, "Features: 0x%08X", chip_info.features);
    
    if (chip_info.features & CHIP_FEATURE_WIFI_BGN) {
        ESP_LOGI(TAG, "  - WiFi 2.4GHz");
    }
    if (chip_info.features & CHIP_FEATURE_BT) {
        ESP_LOGI(TAG, "  - Bluetooth Classic");
    }
    if (chip_info.features & CHIP_FEATURE_BLE) {
        ESP_LOGI(TAG, "  - Bluetooth LE");
    }
    
    uint32_t flash_size;
    esp_flash_get_size(NULL, &flash_size);
    ESP_LOGI(TAG, "Flash size: %d MB", flash_size / (1024 * 1024));
    ESP_LOGI(TAG, "============================");
}