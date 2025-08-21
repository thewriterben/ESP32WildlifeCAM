#include <Arduino.h>
#include "../../firmware/src/board_abstraction.h"
#include "../../firmware/src/camera_universal.h"
#include "../../firmware/boards/board_detector.h"

// Board detection example for ESP32 Wildlife Camera
// This example demonstrates automatic board detection and configuration

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 Wildlife Camera - Board Detection Example ===");
    
    delay(2000); // Allow serial connection to stabilize
    
    // Step 1: Automatic board detection
    Serial.println("\n1. Starting automatic board detection...");
    board_type_t detected_board = board_auto_detect();
    
    if (detected_board == BOARD_UNKNOWN) {
        Serial.println("❌ Failed to detect board type");
        Serial.println("Defaulting to ESP32-CAM configuration");
        detected_board = BOARD_ESP32_CAM;
    } else {
        Serial.printf("✅ Detected board: %s\n", board_get_name(detected_board));
        Serial.printf("   Detection method: %s\n", board_get_detection_info(detected_board));
    }
    
    // Step 2: Validate detection
    Serial.println("\n2. Validating board detection...");
    if (board_validate_detection(detected_board)) {
        Serial.println("✅ Board detection validation passed");
    } else {
        Serial.println("⚠️  Board detection validation failed, but continuing...");
    }
    
    // Step 3: Configure board
    Serial.println("\n3. Configuring board...");
    if (board_configure(detected_board)) {
        Serial.println("✅ Board configuration successful");
    } else {
        Serial.println("❌ Board configuration failed");
        return;
    }
    
    // Step 4: Display board information
    const board_config_t* config = board_get_config();
    if (config) {
        display_board_info(config);
    }
    
    // Step 5: Initialize camera
    Serial.println("\n5. Initializing camera...");
    if (init_camera_for_wildlife()) {
        Serial.println("✅ Camera initialization successful");
    } else {
        Serial.println("❌ Camera initialization failed");
        return;
    }
    
    Serial.println("\n🎉 Board detection and setup complete!");
    Serial.println("Ready for wildlife monitoring operations.\n");
}

void loop() {
    static unsigned long last_info_time = 0;
    static int capture_count = 0;
    
    // Display board info every 30 seconds
    if (millis() - last_info_time > 30000) {
        display_status_info(capture_count);
        last_info_time = millis();
    }
    
    // Test camera capture every 10 seconds
    static unsigned long last_capture_time = 0;
    if (millis() - last_capture_time > 10000) {
        test_camera_capture();
        capture_count++;
        last_capture_time = millis();
    }
    
    delay(100);
}

void display_board_info(const board_config_t* config) {
    Serial.println("\n4. Board Configuration Details:");
    Serial.printf("   Board: %s\n", config->board_name);
    Serial.printf("   Sensor: %s\n", get_sensor_name(config->sensor_type));
    Serial.printf("   Max Resolution: %dx%d\n", 
                  config->max_resolution_width, config->max_resolution_height);
    Serial.printf("   Power Profile: %s\n", get_power_profile_name(config->power_profile));
    
    // Display features
    Serial.println("   Features:");
    Serial.printf("     Flash LED: %s\n", config->has_flash_led ? "Yes" : "No");
    Serial.printf("     PIR Sensor: %s\n", config->has_pir_sensor ? "Yes" : "No");
    Serial.printf("     SD Card: %s\n", config->has_sd_card ? "Yes" : "No");
    Serial.printf("     Battery: %s\n", config->has_battery ? "Yes" : "No");
    
    // Display connectivity
    Serial.println("   Connectivity:");
    Serial.printf("     WiFi: %s\n", config->connectivity.wifi_enabled ? "Yes" : "No");
    Serial.printf("     Cellular: %s\n", config->connectivity.cellular_enabled ? "Yes" : "No");
    Serial.printf("     Satellite: %s\n", config->connectivity.satellite_enabled ? "Yes" : "No");
    Serial.printf("     LoRa: %s\n", config->connectivity.lora_enabled ? "Yes" : "No");
    
    // Display GPIO configuration
    Serial.println("   GPIO Configuration:");
    Serial.printf("     Camera I2C: SDA=%d, SCL=%d\n", 
                  config->gpio.camera_sda, config->gpio.camera_scl);
    Serial.printf("     Camera Control: XCLK=%d, PCLK=%d, VSYNC=%d, HREF=%d\n",
                  config->gpio.camera_xclk, config->gpio.camera_pclk, 
                  config->gpio.camera_vsync, config->gpio.camera_href);
    Serial.printf("     Camera Power: PWDN=%d, RESET=%d\n",
                  config->gpio.camera_pwdn, config->gpio.camera_reset);
    
    if (config->has_flash_led && config->gpio.led_flash >= 0) {
        Serial.printf("     Flash LED: GPIO%d\n", config->gpio.led_flash);
    }
    if (config->has_pir_sensor && config->gpio.pir_sensor >= 0) {
        Serial.printf("     PIR Sensor: GPIO%d\n", config->gpio.pir_sensor);
    }
    if (config->has_sd_card && config->gpio.sd_cs >= 0) {
        Serial.printf("     SD Card CS: GPIO%d\n", config->gpio.sd_cs);
    }
}

bool init_camera_for_wildlife() {
    // Get board configuration to optimize camera settings
    const board_config_t* board_config = board_get_config();
    
    // Configure camera based on board capabilities
    camera_config_t cam_config = {
        .framesize = FRAMESIZE_VGA,        // Start with VGA for testing
        .pixel_format = PIXFORMAT_JPEG,     // JPEG for wildlife monitoring
        .jpeg_quality = 12,                 // Good quality
        .flip_horizontally = false,
        .flip_vertically = false,
        .brightness = 0,                    // Neutral
        .contrast = 0,                      // Neutral
        .saturation = 0,                    // Neutral
        .auto_exposure = true,              // Auto exposure for varying light
        .exposure_value = 300,
        .auto_white_balance = true,         // Auto white balance
        .white_balance_mode = 0
    };
    
    // Adjust configuration based on board capabilities
    if (board_config->max_resolution_width >= 1600) {
        cam_config.framesize = FRAMESIZE_UXGA; // Use maximum resolution if supported
    } else if (board_config->max_resolution_width >= 1280) {
        cam_config.framesize = FRAMESIZE_SXGA;
    }
    
    return camera_universal_init(&cam_config);
}

void test_camera_capture() {
    Serial.printf("\n📸 Testing camera capture...");
    
    camera_fb_t* fb = camera_universal_capture();
    if (fb) {
        Serial.printf(" Success!\n");
        Serial.printf("   Image size: %d bytes\n", fb->len);
        Serial.printf("   Resolution: %dx%d\n", fb->width, fb->height);
        Serial.printf("   Format: %s\n", pixformat_to_string(fb->format));
        Serial.printf("   Timestamp: %llu\n", fb->timestamp);
        
        // Return frame buffer
        camera_universal_return_fb(fb);
    } else {
        Serial.printf(" Failed!\n");
    }
}

void display_status_info(int capture_count) {
    Serial.printf("\n📊 Status Update (Captures: %d):\n", capture_count);
    Serial.printf("   Board: %s\n", board_get_name(board_get_config()->board_type));
    Serial.printf("   Uptime: %lu seconds\n", millis() / 1000);
    Serial.printf("   Free heap: %d bytes\n", ESP.getFreeHeap());
    
    // Test board features
    Serial.println("   Feature tests:");
    Serial.printf("     Flash LED support: %s\n", 
                  board_supports_feature("flash_led") ? "Available" : "Not available");
    Serial.printf("     PIR sensor support: %s\n", 
                  board_supports_feature("pir_sensor") ? "Available" : "Not available");
    Serial.printf("     SD card support: %s\n", 
                  board_supports_feature("sd_card") ? "Available" : "Not available");
    Serial.printf("     WiFi support: %s\n", 
                  board_supports_feature("wifi") ? "Available" : "Not available");
}

const char* get_sensor_name(camera_sensor_t sensor) {
    switch (sensor) {
        case SENSOR_OV2640: return "OV2640 (2MP)";
        case SENSOR_OV3660: return "OV3660 (3MP)";
        case SENSOR_OV5640: return "OV5640 (5MP)";
        case SENSOR_GC032A: return "GC032A (VGA)";
        case SENSOR_OV7670: return "OV7670 (VGA)";
        case SENSOR_HM01B0: return "HM01B0 (QVGA)";
        default: return "Unknown";
    }
}

const char* get_power_profile_name(power_profile_t profile) {
    switch (profile) {
        case POWER_PROFILE_LOW: return "Low Power";
        case POWER_PROFILE_NORMAL: return "Normal";
        case POWER_PROFILE_HIGH: return "High Performance";
        default: return "Unknown";
    }
}