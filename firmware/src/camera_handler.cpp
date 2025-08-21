/**
 * Camera Handler Module
 * 
 * Manages ESP32 camera initialization, configuration, and image capture
 * for the wildlife monitoring system using Hardware Abstraction Layer (HAL).
 */

#include "camera_handler.h"
#include "config.h"
#include "hal/board_detector.h"
#include "hal/esp32_cam.h"
#include "configs/sensor_configs.h"
#include <esp_camera.h>
#include <SD_MMC.h>
#include <ArduinoJson.h>

namespace CameraHandler {

// Static variables
static camera_config_t camera_config;
static bool initialized = false;
static int imageCounter = 0;
static std::unique_ptr<CameraBoard> board;

// Internal function declarations
bool initializeCamera();

/**
 * Initialize camera with automatic board detection
 */
bool init() {
    DEBUG_PRINTLN("Initializing camera with automatic board detection...");
    
    // Create board instance using detection
    board = BoardDetector::createBoard();
    if (!board) {
        DEBUG_PRINTLN("Failed to create board instance");
        return false;
    }
    
    DEBUG_PRINTF("Detected board: %s\n", board->getBoardName());
    
    // Initialize the specific board
    if (!board->init()) {
        DEBUG_PRINTLN("Board initialization failed");
        return false;
    }
    
    return initializeCamera();
}

/**
 * Initialize camera with specific board type
 */
bool init(BoardType boardType) {
    DEBUG_PRINTF("Initializing camera with specific board type: %s\n", 
                BoardDetector::getBoardName(boardType));
    
    // Create board instance for specific type
    board = BoardDetector::createBoard(boardType);
    if (!board) {
        DEBUG_PRINTLN("Failed to create board instance");
        return false;
    }
    
    // Initialize the specific board
    if (!board->init()) {
        DEBUG_PRINTLN("Board initialization failed");
        return false;
    }
    
    return initializeCamera();
}

/**
 * Get the current camera board instance
 */
CameraBoard* getBoard() {
    return board.get();
}

/**
 * Internal function to initialize camera with current board configuration
 */
bool initializeCamera() {
bool initializeCamera() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Configuring camera with board-specific settings...");
    
    // Get board-specific configuration
    GPIOMap gpio_map = board->getGPIOMap();
    CameraConfig cam_config = board->getCameraConfig();
    
    // Configure camera_config with board-specific pins
    camera_config.ledc_channel = LEDC_CHANNEL_0;
    camera_config.ledc_timer = LEDC_TIMER_0;
    camera_config.pin_d0 = gpio_map.y2_pin;
    camera_config.pin_d1 = gpio_map.y3_pin;
    camera_config.pin_d2 = gpio_map.y4_pin;
    camera_config.pin_d3 = gpio_map.y5_pin;
    camera_config.pin_d4 = gpio_map.y6_pin;
    camera_config.pin_d5 = gpio_map.y7_pin;
    camera_config.pin_d6 = gpio_map.y8_pin;
    camera_config.pin_d7 = gpio_map.y9_pin;
    camera_config.pin_xclk = gpio_map.xclk_pin;
    camera_config.pin_pclk = gpio_map.pclk_pin;
    camera_config.pin_vsync = gpio_map.vsync_pin;
    camera_config.pin_href = gpio_map.href_pin;
    camera_config.pin_sscb_sda = gpio_map.siod_pin;
    camera_config.pin_sscb_scl = gpio_map.sioc_pin;
    camera_config.pin_pwdn = gpio_map.pwdn_pin;
    camera_config.pin_reset = gpio_map.reset_pin;
    camera_config.xclk_freq_hz = cam_config.xclk_freq_hz;
    camera_config.pixel_format = cam_config.pixel_format;
    
    // Frame size and quality settings based on board capabilities
    if (board->hasPSRAM() && !cam_config.psram_required) {
        camera_config.frame_size = cam_config.max_framesize;
        camera_config.jpeg_quality = cam_config.jpeg_quality;
        camera_config.fb_count = cam_config.fb_count;
        DEBUG_PRINTLN("PSRAM found - using high quality settings");
    } else if (board->hasPSRAM() && cam_config.psram_required) {
        camera_config.frame_size = cam_config.max_framesize;
        camera_config.jpeg_quality = cam_config.jpeg_quality;
        camera_config.fb_count = cam_config.fb_count;
        DEBUG_PRINTLN("PSRAM required and found - using optimal settings");
    } else if (!board->hasPSRAM() && cam_config.psram_required) {
        DEBUG_PRINTLN("PSRAM required but not found - using conservative settings");
        camera_config.frame_size = FRAMESIZE_SVGA;
        camera_config.jpeg_quality = 15;
        camera_config.fb_count = 1;
    } else {
        camera_config.frame_size = FRAMESIZE_SVGA;
        camera_config.jpeg_quality = 15;
        camera_config.fb_count = 1;
        DEBUG_PRINTLN("PSRAM not found - using conservative settings");
    }
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        DEBUG_PRINTF("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    // Get camera sensor
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == NULL) {
        DEBUG_PRINTLN("Failed to get camera sensor");
        return false;
    }
    
    // Configure sensor settings using board-specific implementation
    if (!board->configureSensor(sensor)) {
        DEBUG_PRINTLN("Failed to configure sensor");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("Camera initialized successfully");
    
    return true;
}

/**
 * Configure camera sensor settings optimized for wildlife detection
 */
void configureSensorSettings(sensor_t* sensor) {
    DEBUG_PRINTLN("Configuring camera sensor settings...");
    
    // Basic settings
    sensor->set_brightness(sensor, 0);     // -2 to 2
    sensor->set_contrast(sensor, 0);       // -2 to 2
    sensor->set_saturation(sensor, 0);     // -2 to 2
    sensor->set_special_effect(sensor, 0); // 0 to 6 (0=No Effect, 1=Negative, 2=Grayscale, 3=Red Tint, 4=Green Tint, 5=Blue Tint, 6=Sepia)
    sensor->set_whitebal(sensor, 1);       // 0 = disable , 1 = enable
    sensor->set_awb_gain(sensor, 1);       // 0 = disable , 1 = enable
    sensor->set_wb_mode(sensor, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    
    // Exposure and gain settings for outdoor conditions
    sensor->set_exposure_ctrl(sensor, 1);  // 0 = disable , 1 = enable
    sensor->set_aec2(sensor, 0);           // 0 = disable , 1 = enable
    sensor->set_ae_level(sensor, 0);       // -2 to 2
    sensor->set_aec_value(sensor, 300);    // 0 to 1200
    sensor->set_gain_ctrl(sensor, 1);      // 0 = disable , 1 = enable
    sensor->set_agc_gain(sensor, 0);       // 0 to 30
    sensor->set_gainceiling(sensor, (gainceiling_t)0);  // 0 to 6
    
    // Image enhancement
    sensor->set_bpc(sensor, 0);            // 0 = disable , 1 = enable
    sensor->set_wpc(sensor, 1);            // 0 = disable , 1 = enable
    sensor->set_raw_gma(sensor, 1);        // 0 = disable , 1 = enable
    sensor->set_lenc(sensor, 1);           // 0 = disable , 1 = enable
    
    // Motion detection optimizations
    sensor->set_hmirror(sensor, 0);        // 0 = disable , 1 = enable
    sensor->set_vflip(sensor, 0);          // 0 = disable , 1 = enable
    sensor->set_dcw(sensor, 1);            // 0 = disable , 1 = enable
    sensor->set_colorbar(sensor, 0);       // 0 = disable , 1 = enable
    
    DEBUG_PRINTLN("Camera sensor configured for wildlife photography");
}

/**
 * Capture a single image
 */
camera_fb_t* captureImage() {
    if (!initialized) {
        DEBUG_PRINTLN("Error: Camera not initialized");
        return nullptr;
    }
    
    DEBUG_PRINTLN("Capturing image...");
    
    // Flash the LED briefly to indicate capture
    flashLED();
    
    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        DEBUG_PRINTLN("Error: Camera capture failed");
        return nullptr;
    }
    
    DEBUG_PRINTF("Image captured: %dx%d, %d bytes, format: %d\n", 
                 fb->width, fb->height, fb->len, fb->format);
    
    imageCounter++;
    return fb;
}

/**
 * Save image to SD card with timestamp and metadata
 */
String saveImage(camera_fb_t* fb, const char* folder) {
    if (!fb) {
        DEBUG_PRINTLN("Error: No image buffer to save");
        return "";
    }
    
    // Create filename with timestamp
    String filename = generateFilename(folder);
    
    // Save image file
    File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    if (!file) {
        DEBUG_PRINTF("Error: Failed to create file %s\n", filename.c_str());
        return "";
    }
    
    size_t written = file.write(fb->buf, fb->len);
    file.close();
    
    if (written != fb->len) {
        DEBUG_PRINTF("Error: Failed to write complete image (%d/%d bytes)\n", written, fb->len);
        return "";
    }
    
    // Save metadata
    if (IMAGE_TIMESTAMP_ENABLED) {
        saveImageMetadata(filename, fb);
    }
    
    DEBUG_PRINTF("Image saved successfully: %s (%d bytes)\n", filename.c_str(), written);
    return filename;
}

/**
 * Generate timestamped filename
 */
String generateFilename(const char* folder) {
    struct tm timeinfo;
    char filename[100];
    
    if (getLocalTime(&timeinfo)) {
        snprintf(filename, sizeof(filename), "%s/%04d%02d%02d_%02d%02d%02d_%04d.jpg",
                folder,
                timeinfo.tm_year + 1900,
                timeinfo.tm_mon + 1,
                timeinfo.tm_mday,
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec,
                imageCounter);
    } else {
        // Fallback if time is not available
        snprintf(filename, sizeof(filename), "%s/img_%08lu_%04d.jpg",
                folder, millis(), imageCounter);
    }
    
    return String(filename);
}

/**
 * Save image metadata as JSON
 */
void saveImageMetadata(const String& imageFilename, camera_fb_t* fb) {
    String metaFilename = imageFilename;
    metaFilename.replace(".jpg", ".json");
    
    // Create metadata JSON
    DynamicJsonDocument doc(512);
    
    doc["timestamp"] = millis();
    doc["image_file"] = imageFilename;
    doc["width"] = fb->width;
    doc["height"] = fb->height;
    doc["size_bytes"] = fb->len;
    doc["format"] = fb->format;
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["node_id"] = NODE_ID;
    
    // Add time information if available
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[32];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        doc["datetime"] = timeStr;
    }
    
    // Save metadata file
    File metaFile = SD_MMC.open(metaFilename.c_str(), FILE_WRITE);
    if (metaFile) {
        serializeJson(doc, metaFile);
        metaFile.close();
        DEBUG_PRINTF("Metadata saved: %s\n", metaFilename.c_str());
    } else {
        DEBUG_PRINTF("Warning: Failed to save metadata %s\n", metaFilename.c_str());
    }
}

/**
 * Get camera status information
 */
CameraStatus getStatus() {
    CameraStatus status;
    status.initialized = initialized;
    status.imageCount = imageCounter;
    status.lastError = ESP_OK;  // Would need to track actual errors
    
    if (board) {
        status.boardType = board->getBoardType();
        status.sensorType = board->getSensorType();
        status.boardName = board->getBoardName();
        const SensorCapabilities* sensor_caps = getSensorCapabilities(board->getSensorType());
        status.sensorName = sensor_caps ? sensor_caps->name : "Unknown";
    } else {
        status.boardType = BOARD_UNKNOWN;
        status.sensorType = SENSOR_UNKNOWN;
        status.boardName = "Not Detected";
        status.sensorName = "Not Detected";
    }
    
    if (initialized) {
        sensor_t* sensor = esp_camera_sensor_get();
        if (sensor) {
            status.sensorDetected = true;
            // Could add more sensor-specific status here
        }
    }
    
    return status;
}

/**
 * Take a test image and return basic info
 */
bool testCamera() {
    DEBUG_PRINTLN("Testing camera...");
    
    camera_fb_t* fb = captureImage();
    if (!fb) {
        DEBUG_PRINTLN("Camera test failed - no image captured");
        return false;
    }
    
    DEBUG_PRINTF("Camera test successful: %dx%d, %d bytes\n", 
                 fb->width, fb->height, fb->len);
    
    esp_camera_fb_return(fb);
    return true;
}

/**
 * Adjust camera settings for different lighting conditions
 */
void adjustForLighting(LightingCondition condition) {
    if (!initialized) return;
    
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) return;
    
    DEBUG_PRINTF("Adjusting camera for lighting condition: %d\n", condition);
    
    switch (condition) {
        case LIGHTING_BRIGHT:
            sensor->set_ae_level(sensor, -1);      // Reduce exposure
            sensor->set_agc_gain(sensor, 0);       // Low gain
            sensor->set_brightness(sensor, -1);    // Reduce brightness
            break;
            
        case LIGHTING_NORMAL:
            sensor->set_ae_level(sensor, 0);       // Normal exposure
            sensor->set_agc_gain(sensor, 5);       // Medium gain
            sensor->set_brightness(sensor, 0);     // Normal brightness
            break;
            
        case LIGHTING_DIM:
            sensor->set_ae_level(sensor, 1);       // Increase exposure
            sensor->set_agc_gain(sensor, 15);      // Higher gain
            sensor->set_brightness(sensor, 1);     // Increase brightness
            break;
            
        case LIGHTING_DARK:
            sensor->set_ae_level(sensor, 2);       // Maximum exposure
            sensor->set_agc_gain(sensor, 25);      // High gain
            sensor->set_brightness(sensor, 2);     // Maximum brightness
            break;
    }
}

/**
 * Flash camera LED briefly
 */
void flashLED() {
    #ifdef CAMERA_LED_PIN
    pinMode(CAMERA_LED_PIN, OUTPUT);
    digitalWrite(CAMERA_LED_PIN, HIGH);
    delay(50);  // Brief flash
    digitalWrite(CAMERA_LED_PIN, LOW);
    #endif
}

/**
 * Cleanup camera resources
 */
void cleanup() {
    if (initialized) {
        esp_camera_deinit();
        initialized = false;
        DEBUG_PRINTLN("Camera deinitialized");
    }
}

} // namespace CameraHandler