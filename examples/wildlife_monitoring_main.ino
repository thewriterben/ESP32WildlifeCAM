#include <Arduino.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include <esp_timer.h>
#include "firmware/src/board_abstraction.h"
#include "firmware/src/camera_universal.h"
#include "firmware/boards/board_detector.h"

// Wildlife monitoring configuration
#define CAPTURE_INTERVAL_MS     300000    // 5 minutes between captures
#define MOTION_CHECK_INTERVAL   10000     // Check for motion every 10 seconds
#define WIFI_TIMEOUT_MS         15000     // WiFi connection timeout
#define MAX_DAILY_CAPTURES      288       // Max captures per day (5 min intervals)

// Wildlife monitoring state
typedef struct {
    board_type_t board_type;
    camera_sensor_t sensor_type;
    bool motion_detection_enabled;
    bool wifi_enabled;
    bool low_power_mode;
    uint32_t capture_count;
    uint32_t motion_detections;
    uint64_t last_capture_time;
    uint64_t last_motion_time;
} wildlife_monitoring_state_t;

static wildlife_monitoring_state_t monitoring_state;

// Function prototypes
void setup_wildlife_monitoring();
void wildlife_monitoring_loop();
bool capture_wildlife_image();
bool detect_motion();
void transmit_image_data(camera_fb_t* fb);
void enter_power_save_mode();
void log_monitoring_status();
bool setup_connectivity();
void cleanup_and_sleep();

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n🦌 ESP32 Wildlife Camera System Starting...");
    Serial.println("===========================================");
    
    // Initialize wildlife monitoring system
    setup_wildlife_monitoring();
    
    Serial.println("🎉 Wildlife monitoring system ready!");
    Serial.println("Starting continuous monitoring...\n");
}

void loop() {
    wildlife_monitoring_loop();
}

void setup_wildlife_monitoring() {
    // Step 1: Detect and configure board
    Serial.println("📡 Detecting ESP32 camera board...");
    monitoring_state.board_type = board_auto_detect();
    
    if (!board_configure(monitoring_state.board_type)) {
        Serial.println("❌ Failed to configure board");
        ESP.restart();
    }
    
    const board_config_t* board_config = board_get_config();
    monitoring_state.sensor_type = board_config->sensor_type;
    
    Serial.printf("✅ Board: %s\n", board_config->board_name);
    Serial.printf("   Sensor: %s\n", get_sensor_name(monitoring_state.sensor_type));
    
    // Step 2: Configure camera for wildlife monitoring
    Serial.println("\n📸 Initializing camera for wildlife monitoring...");
    camera_config_t cam_config = {
        .framesize = FRAMESIZE_UXGA,        // High resolution for wildlife detail
        .pixel_format = PIXFORMAT_JPEG,      // Compressed for efficient storage/transmission
        .jpeg_quality = 10,                  // Good quality for species identification
        .flip_horizontally = false,
        .flip_vertically = false,
        .brightness = 0,                     // Neutral brightness
        .contrast = 1,                       // Slightly enhanced contrast for wildlife
        .saturation = 0,                     // Natural colors
        .auto_exposure = true,               // Handle varying light conditions
        .exposure_value = 300,
        .auto_white_balance = true,          // Adapt to different lighting
        .white_balance_mode = 0
    };
    
    // Adjust settings based on board capabilities
    if (board_config->max_resolution_width < 1600) {
        cam_config.framesize = FRAMESIZE_SXGA; // Use best available resolution
    }
    
    if (!camera_universal_init(&cam_config)) {
        Serial.println("❌ Camera initialization failed");
        ESP.restart();
    }
    
    Serial.println("✅ Camera initialized successfully");
    
    // Step 3: Configure monitoring features
    monitoring_state.motion_detection_enabled = board_supports_feature("pir_sensor");
    monitoring_state.wifi_enabled = board_supports_feature("wifi");
    monitoring_state.low_power_mode = (board_config->power_profile == POWER_PROFILE_LOW);
    
    Serial.println("\n🔧 Wildlife monitoring features:");
    Serial.printf("   Motion detection: %s\n", 
                  monitoring_state.motion_detection_enabled ? "Enabled (PIR)" : "Software-based");
    Serial.printf("   WiFi connectivity: %s\n", 
                  monitoring_state.wifi_enabled ? "Available" : "Disabled");
    Serial.printf("   Power mode: %s\n", 
                  monitoring_state.low_power_mode ? "Low power" : "Normal");
    
    // Step 4: Setup connectivity if available
    if (monitoring_state.wifi_enabled) {
        setup_connectivity();
    }
    
    // Initialize monitoring state
    monitoring_state.capture_count = 0;
    monitoring_state.motion_detections = 0;
    monitoring_state.last_capture_time = 0;
    monitoring_state.last_motion_time = 0;
    
    // Initial capture to test system
    Serial.println("\n📷 Taking initial test capture...");
    if (capture_wildlife_image()) {
        Serial.println("✅ Test capture successful");
    } else {
        Serial.println("⚠️  Test capture failed, but continuing...");
    }
}

void wildlife_monitoring_loop() {
    uint64_t current_time = esp_timer_get_time() / 1000; // Convert to milliseconds
    
    // Check for motion detection
    if (monitoring_state.motion_detection_enabled) {
        if (current_time - monitoring_state.last_motion_time > MOTION_CHECK_INTERVAL) {
            if (detect_motion()) {
                Serial.println("🎯 Motion detected! Capturing image...");
                if (capture_wildlife_image()) {
                    monitoring_state.motion_detections++;
                }
                monitoring_state.last_motion_time = current_time;
            }
        }
    }
    
    // Regular interval capture
    if (current_time - monitoring_state.last_capture_time > CAPTURE_INTERVAL_MS) {
        Serial.println("⏰ Scheduled capture time");
        if (capture_wildlife_image()) {
            monitoring_state.last_capture_time = current_time;
        }
    }
    
    // Log status every hour
    static uint64_t last_status_time = 0;
    if (current_time - last_status_time > 3600000) { // 1 hour
        log_monitoring_status();
        last_status_time = current_time;
    }
    
    // Power management
    if (monitoring_state.low_power_mode) {
        // In low power mode, sleep between checks
        delay(5000); // 5 second sleep
    } else {
        delay(1000);  // 1 second delay in normal mode
    }
    
    // Check if we should enter deep sleep (for very low power operation)
    if (monitoring_state.capture_count >= MAX_DAILY_CAPTURES) {
        cleanup_and_sleep();
    }
}

bool capture_wildlife_image() {
    Serial.printf("📸 Capturing wildlife image #%d...", monitoring_state.capture_count + 1);
    
    camera_fb_t* fb = camera_universal_capture();
    if (!fb) {
        Serial.println(" Failed!");
        return false;
    }
    
    Serial.printf(" Success! (%d bytes)\n", fb->len);
    Serial.printf("   Resolution: %dx%d\n", fb->width, fb->height);
    Serial.printf("   Format: %s\n", pixformat_to_string(fb->format));
    
    // Transmit image data if connectivity available
    if (monitoring_state.wifi_enabled && WiFi.status() == WL_CONNECTED) {
        transmit_image_data(fb);
    } else {
        Serial.println("   📦 Image stored locally (no connectivity)");
    }
    
    // Return frame buffer
    camera_universal_return_fb(fb);
    
    monitoring_state.capture_count++;
    return true;
}

bool detect_motion() {
    // If hardware PIR sensor available, read it
    const board_config_t* config = board_get_config();
    if (config->has_pir_sensor && config->gpio.pir_sensor >= 0) {
        return digitalRead(config->gpio.pir_sensor) == HIGH;
    }
    
    // Otherwise, use software-based motion detection
    // This is a simplified implementation - real motion detection would
    // compare consecutive frames
    static uint32_t last_motion_check = 0;
    uint32_t current_time = millis();
    
    if (current_time - last_motion_check > 30000) { // Simulate motion every 30 seconds
        last_motion_check = current_time;
        return true; // Simulate detected motion
    }
    
    return false;
}

void transmit_image_data(camera_fb_t* fb) {
    Serial.println("   📡 Transmitting image data...");
    
    // This is where you would implement actual data transmission
    // Examples:
    // - HTTP POST to cloud server
    // - MQTT publish to broker  
    // - LoRa transmission
    // - Cellular upload via SIM800L
    
    // For demonstration, just log the transmission
    Serial.printf("   📤 Transmitted %d bytes to wildlife monitoring server\n", fb->len);
    
    // In real implementation:
    // - Compress/resize image if needed
    // - Add metadata (timestamp, location, etc.)
    // - Implement retry logic for failed transmissions
    // - Queue images for batch transmission to save power
}

void enter_power_save_mode() {
    Serial.println("💤 Entering power save mode...");
    
    // Configure wakeup timer
    esp_sleep_enable_timer_wakeup(CAPTURE_INTERVAL_MS * 1000); // Convert to microseconds
    
    // If PIR sensor available, configure as wakeup source
    const board_config_t* config = board_get_config();
    if (config->has_pir_sensor && config->gpio.pir_sensor >= 0) {
        esp_sleep_enable_ext0_wakeup((gpio_num_t)config->gpio.pir_sensor, 1);
        Serial.println("   PIR sensor configured as wakeup source");
    }
    
    // Enter light sleep (preserves RAM, faster wakeup)
    esp_light_sleep_start();
    
    Serial.println("🌅 Woke up from power save mode");
}

void log_monitoring_status() {
    Serial.println("\n📊 Wildlife Monitoring Status Report");
    Serial.println("====================================");
    Serial.printf("Board: %s\n", board_get_name(monitoring_state.board_type));
    Serial.printf("Uptime: %lu minutes\n", millis() / 60000);
    Serial.printf("Total captures: %d\n", monitoring_state.capture_count);
    Serial.printf("Motion detections: %d\n", monitoring_state.motion_detections);
    Serial.printf("Free memory: %d bytes\n", ESP.getFreeHeap());
    
    if (monitoring_state.wifi_enabled) {
        Serial.printf("WiFi status: %s\n", 
                      WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());
        }
    }
    
    Serial.println("====================================\n");
}

bool setup_connectivity() {
    Serial.println("\n🌐 Setting up WiFi connectivity...");
    
    // In a real implementation, WiFi credentials would be configured
    // via web interface, stored in EEPROM, or set via constants
    const char* ssid = "Wildlife_Network";
    const char* password = "wildlife123";
    
    WiFi.begin(ssid, password);
    
    uint32_t start_time = millis();
    while (WiFi.status() != WL_CONNECTED && 
           millis() - start_time < WIFI_TIMEOUT_MS) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n✅ WiFi connected to %s\n", ssid);
        Serial.printf("   IP address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("   Signal strength: %d dBm\n", WiFi.RSSI());
        return true;
    } else {
        Serial.println("\n⚠️  WiFi connection failed, continuing in offline mode");
        return false;
    }
}

void cleanup_and_sleep() {
    Serial.println("\n💤 Daily capture limit reached, entering deep sleep...");
    
    // Save monitoring state to RTC memory or EEPROM for persistence
    // In real implementation, you would save:
    // - Daily capture count
    // - Motion detection statistics  
    // - Any queued images for transmission
    
    // Configure wakeup for next day (24 hours)
    esp_sleep_enable_timer_wakeup(24ULL * 3600ULL * 1000000ULL); // 24 hours in microseconds
    
    // Deep sleep (lowest power consumption)
    esp_deep_sleep_start();
}

// Utility function for sensor name display
const char* get_sensor_name(camera_sensor_t sensor) {
    switch (sensor) {
        case SENSOR_OV2640: return "OV2640 (2MP)";
        case SENSOR_OV3660: return "OV3660 (3MP)";
        case SENSOR_OV5640: return "OV5640 (5MP)";
        case SENSOR_GC032A: return "GC032A (VGA)";
        case SENSOR_OV7670: return "OV7670 (VGA)";
        case SENSOR_HM01B0: return "HM01B0 (QVGA)";
        default: return "Unknown Sensor";
    }
}