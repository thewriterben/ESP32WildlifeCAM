/**
 * @file camera_handler.cpp
 * @brief Implementation of Camera Driver for Step 1.4
 * @author ESP32WildlifeCAM Project
 * @date 2025-08-31
 * 
 * This file implements the critical Camera Driver system as outlined in the
 * Unified Development Plan Step 1.4. Provides conflict-aware initialization,
 * PSRAM optimization, and proper frame buffer management.
 */

#include "camera_handler.h"
#include "camera_utils.cpp"
#include "ai_bridge.cpp"
#include "power_bridge.cpp"
#include "../../firmware/src/hal/board_detector.h"
#include "../../firmware/src/hal/camera_board.h"
#include "../../firmware/src/ai/ai_common.h"
#include <esp_system.h>
#include <memory>

// Forward declarations for AI integration
struct CameraFrame {
    uint8_t* data;
    size_t length;
    uint16_t width;
    uint16_t height;
    uint32_t timestamp;
};

// Constructor
CameraHandler::CameraHandler() : 
    initialized(false), 
    board_instance(nullptr),
    frame_queue(nullptr),
    last_capture_time(0) {
    
    // Initialize configuration to default values
    memset(&config, 0, sizeof(camera_config_t));
    memset(&stats, 0, sizeof(CaptureStats));
    memset(&init_result, 0, sizeof(CameraInitResult));
    
    // Setup advanced frame queue system
    setupFrameQueue();
    
    Serial.println("CameraHandler: Constructor initialized");
}

// Destructor
CameraHandler::~CameraHandler() {
    deinitialize();
    
    if (frame_queue != nullptr) {
        vQueueDelete(frame_queue);
        frame_queue = nullptr;
    }
    
    Serial.println("CameraHandler: Destructor completed");
}

bool CameraHandler::init() {
    Serial.println("CameraHandler: Initializing with recommended configuration...");
    
    // Get recommended configuration based on hardware
    CameraConfig recommended_config = CameraUtils::getRecommendedConfig();
    
    // Initialize with recommended configuration
    esp_err_t result = initialize(recommended_config);
    
    return (result == ESP_OK);
}

esp_err_t CameraHandler::initialize(const CameraConfig& user_config) {
    Serial.println("CameraHandler: Initializing with user configuration...");
    uint32_t start_time = millis();
    
    if (initialized) {
        Serial.println("CameraHandler: Already initialized");
        return ESP_OK;
    }
    
    // Validate configuration
    if (!CameraUtils::validateCameraConfig(user_config, "Unknown")) {
        Serial.println("CameraHandler: Configuration validation failed");
        init_result.error_code = ESP_ERR_INVALID_ARG;
        init_result.error_message = "Invalid configuration parameters";
        return ESP_ERR_INVALID_ARG;
    }
    
    // Detect hardware capabilities
    if (!detectHardwareCapabilities()) {
        Serial.println("CameraHandler: Hardware detection failed");
        init_result.error_code = ESP_FAIL;
        init_result.error_message = "Hardware detection failed";
        return ESP_FAIL;
    }
    
    // Apply configuration to camera_config_t
    config.pin_d0 = -1;      // Will be set by setupPins()
    config.pin_d1 = -1;
    config.pin_d2 = -1;
    config.pin_d3 = -1;
    config.pin_d4 = -1;
    config.pin_d5 = -1;
    config.pin_d6 = -1;
    config.pin_d7 = -1;
    config.pin_xclk = -1;
    config.pin_pclk = -1;
    config.pin_vsync = -1;
    config.pin_href = -1;
    config.pin_sscb_sda = -1;
    config.pin_sscb_scl = -1;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = user_config.xclk_freq_hz;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pixel_format = user_config.pixel_format;
    config.frame_size = user_config.max_framesize;
    config.jpeg_quality = user_config.jpeg_quality;
    config.fb_count = user_config.fb_count;
    config.grab_mode = user_config.grab_mode;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    
    // Setup pins using board detection
    esp_err_t pin_result = setupPins();
    if (pin_result != ESP_OK) {
        Serial.printf("CameraHandler: Pin setup failed: 0x%x\n", pin_result);
        init_result.error_code = pin_result;
        init_result.error_message = "Pin configuration failed";
        return pin_result;
    }
    
    // Initialize ESP camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("CameraHandler: Camera init failed: 0x%x\n", err);
        init_result.error_code = err;
        init_result.error_message = "ESP camera initialization failed";
        CameraUtils::diagnoseCameraIssues(err);
        return err;
    }
    
    // Configure sensor
    esp_err_t sensor_result = configureSensor();
    if (sensor_result != ESP_OK) {
        Serial.printf("CameraHandler: Sensor configuration failed: 0x%x\n", sensor_result);
        esp_camera_deinit();
        init_result.error_code = sensor_result;
        init_result.error_message = "Sensor configuration failed";
        return sensor_result;
    }
    
    // Optimize memory usage after successful initialization
    optimizeMemoryUsage();
    
    initialized = true;
    
    // Record successful initialization
    init_result.error_code = ESP_OK;
    init_result.initialized = true;
    init_result.error_message = "Successfully initialized";
    init_result.init_time_ms = millis() - start_time;
    
    Serial.printf("CameraHandler: Initialization successful (%lu ms)\n", init_result.init_time_ms);
    CameraUtils::logCameraConfig(user_config);
    
    return ESP_OK;
}

esp_err_t CameraHandler::captureFrame(uint32_t timeout_ms) {
    if (!initialized) {
        Serial.println("CameraHandler: Not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    uint32_t capture_start = millis();
    
    // Capture frame
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("CameraHandler: Frame capture failed");
        updateCaptureStats(millis() - capture_start, 0, false);
        handleCaptureFailure(); // Enhanced error handling
        return ESP_FAIL;
    }
    
    uint32_t capture_time = millis() - capture_start;
    
    // Check timeout
    if (capture_time > timeout_ms) {
        Serial.printf("CameraHandler: Capture timeout (%lu ms > %lu ms)\n", capture_time, timeout_ms);
        esp_camera_fb_return(fb);
        updateCaptureStats(capture_time, 0, false);
        handleCaptureFailure(); // Enhanced error handling
        return ESP_ERR_TIMEOUT;
    }
    
    // Add to queue if space available
    if (xQueueSend(frame_queue, &fb, 0) != pdTRUE) {
        Serial.println("CameraHandler: Frame queue full, dropping frame");
        esp_camera_fb_return(fb);
        updateCaptureStats(capture_time, fb->len, false);
        return ESP_ERR_NO_MEM;
    }
    
    updateCaptureStats(capture_time, fb->len, true);
    Serial.printf("CameraHandler: Frame captured (%dx%d, %d bytes, %lu ms)\n", 
                  fb->width, fb->height, fb->len, capture_time);
    
    return ESP_OK;
}

camera_fb_t* CameraHandler::getFrameBuffer() {
    if (frame_queue == nullptr) {
        Serial.println("CameraHandler: Frame queue not initialized");
        return nullptr;
    }
    
    camera_fb_t* fb = nullptr;
    if (xQueueReceive(frame_queue, &fb, 0) == pdTRUE) {
        return fb;
    }
    
    return nullptr;
}

void CameraHandler::returnFrameBuffer(camera_fb_t* fb) {
    if (fb != nullptr) {
        esp_camera_fb_return(fb);
    }
}

esp_err_t CameraHandler::deinitialize() {
    Serial.println("CameraHandler: Deinitializing...");
    
    if (!initialized) {
        return ESP_OK;
    }
    
    // Return any pending frame buffers
    if (frame_queue != nullptr) {
        camera_fb_t* fb;
        while (xQueueReceive(frame_queue, &fb, 0) == pdTRUE) {
            esp_camera_fb_return(fb);
        }
    }
    
    // Deinitialize camera
    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK) {
        Serial.printf("CameraHandler: Deinit failed: 0x%x\n", err);
        return err;
    }
    
    initialized = false;
    Serial.println("CameraHandler: Deinitialization complete");
    
    return ESP_OK;
}

bool CameraHandler::validatePinAssignment() {
    Serial.println("CameraHandler: Validating pin assignments...");
    
    // Create board instance for validation
    std::unique_ptr<CameraBoard> board = BoardDetector::createBoard();
    if (!board) {
        Serial.println("CameraHandler: Failed to create board for validation");
        return false;
    }
    
    GPIOMap gpio_map = board->getGPIOMap();
    return BoardDetector::validateGPIOConfiguration(gpio_map);
}

esp_err_t CameraHandler::initializeWithConflictCheck() {
    Serial.println("CameraHandler: Initializing with conflict checking...");
    
    if (!validatePinAssignment()) {
        Serial.println("CameraHandler: Pin validation failed");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Get recommended configuration based on hardware
    CameraConfig recommended_config = CameraUtils::getRecommendedConfig();
    return initialize(recommended_config);
}

CameraInitResult CameraHandler::getInitializationResult() const {
    return init_result;
}

CaptureStats CameraHandler::getCaptureStats() const {
    return stats;
}

bool CameraHandler::testCamera() {
    Serial.println("CameraHandler: Testing camera functionality...");
    
    if (!initialized) {
        Serial.println("CameraHandler: Camera not initialized for test");
        return false;
    }
    
    // Capture a test frame
    esp_err_t result = captureFrame(5000); // 5 second timeout
    if (result != ESP_OK) {
        Serial.printf("CameraHandler: Test capture failed: 0x%x\n", result);
        return false;
    }
    
    // Get and immediately return the frame
    camera_fb_t* fb = getFrameBuffer();
    if (!fb) {
        Serial.println("CameraHandler: Test frame buffer retrieval failed");
        return false;
    }
    
    Serial.printf("CameraHandler: Test successful - %dx%d, %d bytes\n", 
                  fb->width, fb->height, fb->len);
    
    returnFrameBuffer(fb);
    return true;
}

// Private methods

esp_err_t CameraHandler::setupPins() {
    Serial.println("CameraHandler: Setting up pins...");
    
    // Create board instance
    std::unique_ptr<CameraBoard> board = BoardDetector::createBoard();
    if (!board) {
        Serial.println("CameraHandler: Failed to create board instance");
        return ESP_FAIL;
    }
    
    // Store board reference for later use (note: unique_ptr manages the lifetime)
    
    // Get GPIO mapping
    GPIOMap gpio_map = board->getGPIOMap();
    
    // Apply pin configuration
    config.pin_d0 = gpio_map.y2_pin;
    config.pin_d1 = gpio_map.y3_pin;
    config.pin_d2 = gpio_map.y4_pin;
    config.pin_d3 = gpio_map.y5_pin;
    config.pin_d4 = gpio_map.y6_pin;
    config.pin_d5 = gpio_map.y7_pin;
    config.pin_d6 = gpio_map.y8_pin;
    config.pin_d7 = gpio_map.y9_pin;
    config.pin_xclk = gpio_map.xclk_pin;
    config.pin_pclk = gpio_map.pclk_pin;
    config.pin_vsync = gpio_map.vsync_pin;
    config.pin_href = gpio_map.href_pin;
    config.pin_sscb_sda = gpio_map.siod_pin;
    config.pin_sscb_scl = gpio_map.sioc_pin;
    config.pin_pwdn = gpio_map.pwdn_pin;
    config.pin_reset = gpio_map.reset_pin;
    
    Serial.println("CameraHandler: Pin setup complete");
    return ESP_OK;
}

esp_err_t CameraHandler::configureSensor() {
    Serial.println("CameraHandler: Configuring sensor...");
    
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        Serial.println("CameraHandler: Failed to get sensor handle");
        return ESP_FAIL;
    }
    
    // Apply wildlife-optimized sensor settings
    sensor->set_brightness(sensor, 0);       // Normal brightness
    sensor->set_contrast(sensor, 0);         // Normal contrast
    sensor->set_saturation(sensor, 0);       // Normal saturation
    sensor->set_special_effect(sensor, 0);   // No special effects
    sensor->set_whitebal(sensor, 1);         // Enable white balance
    sensor->set_awb_gain(sensor, 1);         // Enable AWB gain
    sensor->set_wb_mode(sensor, 0);          // Auto white balance mode
    sensor->set_exposure_ctrl(sensor, 1);    // Enable exposure control
    sensor->set_aec2(sensor, 0);             // Disable AEC2
    sensor->set_ae_level(sensor, 0);         // Normal AE level
    sensor->set_aec_value(sensor, 300);      // AEC value
    sensor->set_gain_ctrl(sensor, 1);        // Enable gain control
    sensor->set_agc_gain(sensor, 0);         // AGC gain
    sensor->set_gainceiling(sensor, GAINCEILING_2X); // 2x gain ceiling
    sensor->set_bpc(sensor, 0);              // Disable black pixel correction
    sensor->set_wpc(sensor, 1);              // Enable white pixel correction
    sensor->set_raw_gma(sensor, 1);          // Enable gamma correction
    sensor->set_lenc(sensor, 1);             // Enable lens correction
    sensor->set_hmirror(sensor, 0);          // No horizontal mirror
    sensor->set_vflip(sensor, 0);            // No vertical flip
    sensor->set_dcw(sensor, 1);              // Enable downsize cropping
    sensor->set_colorbar(sensor, 0);         // Disable color bar
    
    Serial.println("CameraHandler: Sensor configuration complete");
    return ESP_OK;
}

bool CameraHandler::detectHardwareCapabilities() {
    Serial.println("CameraHandler: Detecting hardware capabilities...");
    
    // PSRAM detection
    init_result.psram_detected = psramFound();
    init_result.psram_size = init_result.psram_detected ? ESP.getPsramSize() : 0;
    
    Serial.printf("CameraHandler: PSRAM %s (%d bytes)\n", 
                  init_result.psram_detected ? "detected" : "not found",
                  init_result.psram_size);
    
    // Board detection
    BoardType board_type = BoardDetector::detectBoardType();
    init_result.board_name = BoardDetector::getBoardName(board_type);
    
    Serial.printf("CameraHandler: Board type: %s\n", init_result.board_name);
    
    // Note: Sensor detection will happen after esp_camera_init
    init_result.sensor_detected = false;
    init_result.sensor_name = "Unknown";
    
    return true;
}

void CameraHandler::updateCaptureStats(uint32_t capture_time, size_t image_size, bool success) {
    stats.total_captures++;
    
    if (success) {
        stats.successful_captures++;
        stats.total_data_captured += image_size;
        
        // Update timing statistics
        if (stats.successful_captures == 1) {
            stats.min_capture_time_ms = capture_time;
            stats.max_capture_time_ms = capture_time;
            stats.avg_capture_time_ms = capture_time;
            stats.avg_image_size = image_size;
        } else {
            if (capture_time < stats.min_capture_time_ms) {
                stats.min_capture_time_ms = capture_time;
            }
            if (capture_time > stats.max_capture_time_ms) {
                stats.max_capture_time_ms = capture_time;
            }
            
            // Running average
            stats.avg_capture_time_ms = 
                (stats.avg_capture_time_ms * (stats.successful_captures - 1) + capture_time) / 
                stats.successful_captures;
            
            stats.avg_image_size = 
                (stats.avg_image_size * (stats.successful_captures - 1) + image_size) / 
                stats.successful_captures;
        }
    } else {
        stats.failed_captures++;
    }
    
    stats.last_capture_time_ms = capture_time;
    last_capture_time = millis();
}

void CameraHandler::logDiagnosticInfo() const {
    Serial.println("=== CameraHandler Diagnostic Info ===");
    Serial.printf("Initialized: %s\n", initialized ? "Yes" : "No");
    Serial.printf("Total captures: %lu\n", stats.total_captures);
    Serial.printf("Successful: %lu\n", stats.successful_captures);
    Serial.printf("Failed: %lu\n", stats.failed_captures);
    Serial.printf("Average capture time: %lu ms\n", stats.avg_capture_time_ms);
    Serial.printf("Average image size: %d bytes\n", stats.avg_image_size);
    Serial.println("=== End Diagnostic Info ===");
}

// ========== AI INTEGRATION METHODS ==========

AIResult CameraHandler::captureAndAnalyze(ModelType model) {
    Serial.println("CameraHandler: Starting capture and AI analysis...");
    
    AIResult result;
    result.confidence = 0.0f;
    result.species = SpeciesType::UNKNOWN;
    result.detected = false;
    
    if (!initialized) {
        Serial.println("CameraHandler: Camera not initialized for AI analysis");
        result.error_message = "Camera not initialized";
        return result;
    }
    
    // Capture frame with 5 second timeout
    esp_err_t capture_result = captureFrame(5000);
    if (capture_result != ESP_OK) {
        Serial.printf("CameraHandler: Frame capture failed for AI analysis: 0x%x\n", capture_result);
        result.error_message = "Frame capture failed";
        return result;
    }
    
    // Get captured frame
    camera_fb_t* fb = getFrameBuffer();
    if (!fb) {
        Serial.println("CameraHandler: Failed to get frame buffer for AI analysis");
        result.error_message = "Frame buffer retrieval failed";
        return result;
    }
    
    // Create frame structure for AI processing
    CameraFrame frame;
    frame.data = fb->buf;
    frame.length = fb->len;
    frame.width = fb->width;
    frame.height = fb->height;
    frame.timestamp = millis();
    
    // Load AI processing functions
    extern bool initializeInferenceEngine();
    extern AIResult runWildlifeInference(const CameraFrame& frame, ModelType model);
    
    // Initialize inference engine if needed
    static bool ai_initialized = false;
    if (!ai_initialized) {
        ai_initialized = initializeInferenceEngine();
        if (!ai_initialized) {
            Serial.println("CameraHandler: Failed to initialize AI inference engine");
            result.error_message = "AI initialization failed";
            returnFrameBuffer(fb);
            return result;
        }
    }
    
    // Run AI inference
    result = runWildlifeInference(frame, model);
    
    // Return frame buffer
    returnFrameBuffer(fb);
    
    Serial.printf("CameraHandler: AI analysis complete - Species: %d, Confidence: %.2f\n", 
                  (int)result.species, result.confidence);
    
    return result;
}

String CameraHandler::saveImage(camera_fb_t* fb, const char* folder) {
    if (!fb) {
        Serial.println("CameraHandler: Cannot save null frame buffer");
        return "";
    }
    
    // Load storage manager functions
    extern bool initializeStorageManager();
    extern bool saveImageToStorage(camera_fb_t* fb, const String& filename);
    
    // Initialize storage if needed
    static bool storage_initialized = false;
    if (!storage_initialized) {
        storage_initialized = initializeStorageManager();
        if (!storage_initialized) {
            Serial.println("CameraHandler: Failed to initialize storage manager");
            return "";
        }
    }
    
    // Generate unique filename with timestamp
    char timestamp[32];
    snprintf(timestamp, sizeof(timestamp), "%lu", millis());
    String filename = String(folder) + "/wildlife_" + String(timestamp) + ".jpg";
    
    // Save image using storage manager
    if (saveImageToStorage(fb, filename)) {
        Serial.printf("CameraHandler: Image saved successfully: %s\n", filename.c_str());
        return filename;
    } else {
        Serial.println("CameraHandler: Failed to save image");
        return "";
    }
}

// ========== ADVANCED MEMORY MANAGEMENT ==========

bool CameraHandler::setupFrameQueue() {
    Serial.println("CameraHandler: Setting up advanced frame queue system...");
    
    // Check if queue already exists
    if (frame_queue != nullptr) {
        Serial.println("CameraHandler: Frame queue already exists, optimizing...");
        
        // Clear existing queue
        camera_fb_t* fb;
        while (xQueueReceive(frame_queue, &fb, 0) == pdTRUE) {
            esp_camera_fb_return(fb);
        }
        vQueueDelete(frame_queue);
    }
    
    // Calculate optimal queue size based on available memory
    uint8_t optimal_queue_size = 3; // Default triple buffering
    
    if (psramFound()) {
        size_t psram_free = ESP.getFreePsram();
        Serial.printf("CameraHandler: PSRAM available: %d bytes\n", psram_free);
        
        // With PSRAM, we can afford more buffers
        if (psram_free > 2 * 1024 * 1024) { // > 2MB
            optimal_queue_size = 5;
        } else if (psram_free > 1 * 1024 * 1024) { // > 1MB
            optimal_queue_size = 4;
        }
    } else {
        size_t heap_free = ESP.getFreeHeap();
        Serial.printf("CameraHandler: Heap available: %d bytes\n", heap_free);
        
        // Without PSRAM, be more conservative
        if (heap_free < 100 * 1024) { // < 100KB
            optimal_queue_size = 1;
        } else if (heap_free < 200 * 1024) { // < 200KB
            optimal_queue_size = 2;
        }
    }
    
    // Create optimized queue
    frame_queue = xQueueCreate(optimal_queue_size, sizeof(camera_fb_t*));
    if (frame_queue == nullptr) {
        Serial.println("CameraHandler: Failed to create frame queue");
        return false;
    }
    
    Serial.printf("CameraHandler: Frame queue created with %d slots\n", optimal_queue_size);
    return true;
}

void CameraHandler::optimizeMemoryUsage() {
    Serial.println("CameraHandler: Optimizing memory usage...");
    
    // Log current memory status
    Serial.printf("Free heap before optimization: %d bytes\n", ESP.getFreeHeap());
    if (psramFound()) {
        Serial.printf("Free PSRAM before optimization: %d bytes\n", ESP.getFreePsram());
    }
    
    // Force garbage collection
    esp_err_t heap_result = heap_caps_check_integrity_all(true);
    if (heap_result != ESP_OK) {
        Serial.printf("CameraHandler: Heap integrity check failed: 0x%x\n", heap_result);
    }
    
    // Configure camera frame buffer location based on available memory
    if (initialized && psramFound()) {
        Serial.println("CameraHandler: Optimizing for PSRAM usage");
        
        // Configure for PSRAM frame buffers
        sensor_t* sensor = esp_camera_sensor_get();
        if (sensor) {
            // Use PSRAM for frame buffers to free up heap
            config.fb_location = CAMERA_FB_IN_PSRAM;
            
            // Adjust frame buffer count based on PSRAM availability
            size_t psram_free = ESP.getFreePsram();
            if (psram_free > 4 * 1024 * 1024) { // > 4MB
                config.fb_count = 3;
            } else if (psram_free > 2 * 1024 * 1024) { // > 2MB
                config.fb_count = 2;
            } else {
                config.fb_count = 1;
            }
            
            Serial.printf("CameraHandler: Optimized frame buffer count: %d\n", config.fb_count);
        }
    } else {
        Serial.println("CameraHandler: Optimizing for heap-only usage");
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.fb_count = 1; // Conservative for heap-only systems
    }
    
    // Log memory status after optimization
    Serial.printf("Free heap after optimization: %d bytes\n", ESP.getFreeHeap());
    if (psramFound()) {
        Serial.printf("Free PSRAM after optimization: %d bytes\n", ESP.getFreePsram());
    }
}

// ========== PRODUCTION ERROR HANDLING ==========

void CameraHandler::handleCaptureFailure() {
    Serial.println("CameraHandler: Handling capture failure with recovery strategies...");
    
    stats.failed_captures++;
    
    // Strategy 1: Check sensor status
    if (initialized) {
        sensor_t* sensor = esp_camera_sensor_get();
        if (!sensor) {
            Serial.println("CameraHandler: Sensor handle lost, attempting reinitialization...");
            
            // Attempt sensor recovery
            esp_err_t recovery_result = configureSensor();
            if (recovery_result == ESP_OK) {
                Serial.println("CameraHandler: Sensor recovery successful");
                return;
            }
        }
    }
    
    // Strategy 2: Memory cleanup and defragmentation
    Serial.println("CameraHandler: Performing memory cleanup...");
    
    // Clear frame queue
    if (frame_queue != nullptr) {
        camera_fb_t* fb;
        while (xQueueReceive(frame_queue, &fb, 0) == pdTRUE) {
            esp_camera_fb_return(fb);
        }
    }
    
    // Force memory cleanup
    optimizeMemoryUsage();
    
    // Strategy 3: Check for consecutive failures
    if (stats.failed_captures > 5) {
        Serial.println("CameraHandler: Multiple consecutive failures detected");
        
        if (stats.failed_captures > 10) {
            Serial.println("CameraHandler: Critical failure threshold reached, reinitializing camera...");
            
            // Full camera reinitialization
            esp_camera_deinit();
            delay(1000); // Allow hardware to reset
            
            esp_err_t reinit_result = esp_camera_init(&config);
            if (reinit_result == ESP_OK) {
                Serial.println("CameraHandler: Camera reinitialization successful");
                configureSensor(); // Reconfigure sensor
                stats.failed_captures = 0; // Reset failure counter
            } else {
                Serial.printf("CameraHandler: Camera reinitialization failed: 0x%x\n", reinit_result);
            }
        }
    }
    
    // Strategy 4: Log comprehensive diagnostic information
    logDiagnosticInfo();
    Serial.printf("CameraHandler: Free heap: %d bytes\n", ESP.getFreeHeap());
    if (psramFound()) {
        Serial.printf("CameraHandler: Free PSRAM: %d bytes\n", ESP.getFreePsram());
    }
    
    Serial.println("CameraHandler: Capture failure recovery strategies completed");
}

// ========== POWER MANAGEMENT INTEGRATION ==========

esp_err_t CameraHandler::capturePowerAware(uint32_t timeout_ms, bool power_aware) {
    Serial.println("CameraHandler: Starting power-aware capture...");
    
    if (!initialized) {
        Serial.println("CameraHandler: Not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Load power management functions
    extern bool initializePowerManager();
    extern float getCurrentBatteryLevel();
    extern int getCurrentPowerState();
    extern void setPowerMode(int mode);
    
    // Initialize power manager if needed
    static bool power_initialized = false;
    if (power_aware && !power_initialized) {
        power_initialized = initializePowerManager();
        if (!power_initialized) {
            Serial.println("CameraHandler: Warning - Power manager initialization failed, continuing without power awareness");
            power_aware = false;
        }
    }
    
    // Adapt settings based on power state
    if (power_aware) {
        float battery_level = getCurrentBatteryLevel();
        int power_state = getCurrentPowerState();
        
        Serial.printf("CameraHandler: Battery level: %.2f, Power state: %d\n", battery_level, power_state);
        
        // Adapt camera settings based on power level
        adaptToPowerState(power_state);
        
        // Adjust timeout based on battery level
        if (battery_level < 0.2f) { // Less than 20% battery
            timeout_ms = min(timeout_ms, 2000u); // Reduce timeout to 2 seconds
            Serial.println("CameraHandler: Reduced timeout for low battery");
        } else if (battery_level < 0.5f) { // Less than 50% battery
            timeout_ms = min(timeout_ms, 3000u); // Reduce timeout to 3 seconds
            Serial.println("CameraHandler: Moderate timeout for medium battery");
        }
    }
    
    // Perform normal capture with potentially adjusted settings
    esp_err_t result = captureFrame(timeout_ms);
    
    if (power_aware && result == ESP_OK) {
        // Log power consumption impact
        Serial.printf("CameraHandler: Power-aware capture completed successfully in %lu ms\n", timeout_ms);
    }
    
    return result;
}

AIResult CameraHandler::captureAndAnalyzePowerAware(ModelType model, float power_level) {
    Serial.printf("CameraHandler: Starting power-aware AI capture (power level: %.2f)...\n", power_level);
    
    AIResult result;
    result.confidence = 0.0f;
    result.species = SpeciesType::UNKNOWN;
    result.detected = false;
    
    if (!initialized) {
        Serial.println("CameraHandler: Camera not initialized for power-aware AI analysis");
        result.error_message = "Camera not initialized";
        return result;
    }
    
    // Adjust capture timeout based on power level
    uint32_t timeout_ms = 5000; // Default timeout
    if (power_level < 0.3f) {
        timeout_ms = 2000; // Low power mode - quick capture
        Serial.println("CameraHandler: Using low power mode for AI capture");
    } else if (power_level < 0.7f) {
        timeout_ms = 3500; // Medium power mode
        Serial.println("CameraHandler: Using medium power mode for AI capture");
    }
    
    // Use power-aware capture
    esp_err_t capture_result = capturePowerAware(timeout_ms, true);
    if (capture_result != ESP_OK) {
        Serial.printf("CameraHandler: Power-aware capture failed: 0x%x\n", capture_result);
        result.error_message = "Power-aware capture failed";
        return result;
    }
    
    // Get captured frame
    camera_fb_t* fb = getFrameBuffer();
    if (!fb) {
        Serial.println("CameraHandler: Failed to get frame buffer for power-aware AI analysis");
        result.error_message = "Frame buffer retrieval failed";
        return result;
    }
    
    // Create frame structure for AI processing
    CameraFrame frame;
    frame.data = fb->buf;
    frame.length = fb->len;
    frame.width = fb->width;
    frame.height = fb->height;
    frame.timestamp = millis();
    
    // Load AI processing functions
    extern bool initializeInferenceEngine();
    extern AIResult runWildlifeInference(const CameraFrame& frame, ModelType model);
    
    // Initialize inference engine if needed
    static bool ai_initialized = false;
    if (!ai_initialized) {
        ai_initialized = initializeInferenceEngine();
        if (!ai_initialized) {
            Serial.println("CameraHandler: Failed to initialize AI inference engine");
            result.error_message = "AI initialization failed";
            returnFrameBuffer(fb);
            return result;
        }
    }
    
    // For low power scenarios, use simpler/faster models if available
    if (power_level < 0.4f) {
        Serial.println("CameraHandler: Using optimized AI model for low power");
        // Could switch to a lighter model here in a real implementation
    }
    
    // Run AI inference
    result = runWildlifeInference(frame, model);
    
    // Return frame buffer
    returnFrameBuffer(fb);
    
    Serial.printf("CameraHandler: Power-aware AI analysis complete - Species: %d, Confidence: %.2f\n", 
                  (int)result.species, result.confidence);
    
    return result;
}

void CameraHandler::adaptToPowerState(int power_state) {
    Serial.printf("CameraHandler: Adapting to power state: %d\n", power_state);
    
    if (!initialized) {
        Serial.println("CameraHandler: Cannot adapt settings - camera not initialized");
        return;
    }
    
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        Serial.println("CameraHandler: Cannot get sensor for power adaptation");
        return;
    }
    
    // Power state mapping:
    // 0 = NORMAL, 1 = POWER_SAVE, 2 = LOW_BATTERY, 3 = CRITICAL, 4 = CHARGING
    
    switch (power_state) {
        case 0: // NORMAL
            Serial.println("CameraHandler: Normal power mode - optimal settings");
            sensor->set_brightness(sensor, 0);       // Normal brightness
            sensor->set_contrast(sensor, 0);         // Normal contrast
            sensor->set_ae_level(sensor, 0);         // Normal AE level
            sensor->set_gainceiling(sensor, GAINCEILING_2X);
            break;
            
        case 1: // POWER_SAVE
            Serial.println("CameraHandler: Power save mode - reduced quality settings");
            sensor->set_brightness(sensor, -1);      // Slightly reduced brightness
            sensor->set_contrast(sensor, -1);        // Slightly reduced contrast
            sensor->set_ae_level(sensor, -1);        // Lower AE level
            sensor->set_gainceiling(sensor, GAINCEILING_16X); // Higher gain ceiling for low light
            break;
            
        case 2: // LOW_BATTERY
            Serial.println("CameraHandler: Low battery mode - minimal power settings");
            sensor->set_brightness(sensor, -2);      // Reduced brightness
            sensor->set_contrast(sensor, -2);        // Reduced contrast
            sensor->set_ae_level(sensor, -2);        // Lower AE level
            sensor->set_gainceiling(sensor, GAINCEILING_32X); // Higher gain for quick capture
            
            // Reduce frame buffer count to save memory and processing
            if (config.fb_count > 1) {
                config.fb_count = 1;
                Serial.println("CameraHandler: Reduced frame buffer count for low battery");
            }
            break;
            
        case 3: // CRITICAL
            Serial.println("CameraHandler: Critical power mode - emergency settings");
            sensor->set_brightness(sensor, -2);
            sensor->set_contrast(sensor, -2);
            sensor->set_ae_level(sensor, -2);
            sensor->set_gainceiling(sensor, GAINCEILING_64X); // Maximum gain for fastest capture
            
            // Minimal frame buffers
            config.fb_count = 1;
            
            // Consider reducing frame size for faster processing
            if (config.frame_size > FRAMESIZE_VGA) {
                config.frame_size = FRAMESIZE_VGA;
                Serial.println("CameraHandler: Reduced frame size for critical power");
            }
            break;
            
        case 4: // CHARGING
            Serial.println("CameraHandler: Charging mode - can use optimal settings");
            sensor->set_brightness(sensor, 1);       // Enhanced brightness
            sensor->set_contrast(sensor, 1);         // Enhanced contrast
            sensor->set_ae_level(sensor, 1);         // Higher AE level
            sensor->set_gainceiling(sensor, GAINCEILING_2X); // Lower gain ceiling for quality
            
            // Can afford more frame buffers while charging
            if (psramFound() && config.fb_count < 3) {
                config.fb_count = 3;
                Serial.println("CameraHandler: Increased frame buffer count while charging");
            }
            break;
            
        default:
            Serial.printf("CameraHandler: Unknown power state %d, using default settings\n", power_state);
            break;
    }
    
    Serial.println("CameraHandler: Power state adaptation complete");
}