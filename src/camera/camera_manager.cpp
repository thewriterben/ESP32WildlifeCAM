#include "camera_manager.h"
#include <SD_MMC.h>
#include <FS.h>
#include <time.h>

bool CameraManager::initialize() {
    if (initialized) {
        return true;
    }

    Serial.println("Initializing camera...");

    // Get camera configuration
    camera_config_t config = getCameraConfig();

    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    // Get camera sensor for configuration
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
        Serial.println("Failed to get camera sensor");
        return false;
    }

    // Configure initial sensor settings
    configureSensor();

    // Optimize for wildlife photography
    optimizeForWildlife();

    initialized = true;
    resetStatistics();

    Serial.println("Camera initialized successfully");
    Serial.printf("Camera configuration: %s\n", getConfiguration().c_str());

    return true;
}

CameraManager::CaptureResult CameraManager::captureImage(const String& folder) {
    CaptureResult result = {};
    result.success = false;

    if (!initialized) {
        Serial.println("Camera not initialized");
        return result;
    }

    uint32_t startTime = millis();

    // Capture image to frame buffer
    camera_fb_t* fb = captureToBuffer();
    if (fb == nullptr) {
        Serial.println("Camera capture failed");
        updateStats(false, millis() - startTime, 0);
        return result;
    }

    // Save to SD card
    String filename = saveFrameBuffer(fb, folder);
    if (filename.length() > 0) {
        result.success = true;
        result.filename = filename;
        result.imageSize = fb->len;
        result.captureTime = millis() - startTime;
        result.frameBuffer = fb;

        Serial.printf("Image captured: %s (%d bytes, %d ms)\n", 
                     filename.c_str(), fb->len, result.captureTime);
    }

    updateStats(result.success, result.captureTime, result.imageSize);
    return result;
}

camera_fb_t* CameraManager::captureToBuffer() {
    if (!initialized) {
        return nullptr;
    }

    // Capture frame
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb == nullptr) {
        Serial.println("Camera capture failed");
        return nullptr;
    }

    // Validate frame buffer
    if (fb->len == 0 || fb->buf == nullptr) {
        Serial.println("Invalid frame buffer");
        esp_camera_fb_return(fb);
        return nullptr;
    }

    return fb;
}

void CameraManager::returnFrameBuffer(camera_fb_t* fb) {
    if (fb != nullptr) {
        esp_camera_fb_return(fb);
    }
}

String CameraManager::saveFrameBuffer(camera_fb_t* fb, const String& folder, const String& filename) {
    if (fb == nullptr || fb->len == 0) {
        return "";
    }

    // Ensure directory exists
    if (!ensureDirectory(folder)) {
        Serial.printf("Failed to create directory: %s\n", folder.c_str());
        return "";
    }

    // Generate filename if not provided
    String saveFilename = filename;
    if (saveFilename.length() == 0) {
        saveFilename = generateFilename(folder);
    } else if (!saveFilename.startsWith("/")) {
        saveFilename = folder + "/" + saveFilename;
    }

    // Save to SD card
    File file = SD_MMC.open(saveFilename.c_str(), FILE_WRITE);
    if (!file) {
        Serial.printf("Failed to create file: %s\n", saveFilename.c_str());
        return "";
    }

    size_t written = file.write(fb->buf, fb->len);
    file.close();

    if (written != fb->len) {
        Serial.printf("Write error: %d/%d bytes written\n", written, fb->len);
        return "";
    }

    return saveFilename;
}

void CameraManager::configureSensor(int brightness, int contrast, int saturation) {
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
        return;
    }

    // Configure sensor settings
    sensor->set_brightness(sensor, brightness);   // -2 to 2
    sensor->set_contrast(sensor, contrast);       // -2 to 2
    sensor->set_saturation(sensor, saturation);   // -2 to 2
    
    // Wildlife-optimized settings
    sensor->set_sharpness(sensor, CAMERA_SHARPNESS_DEFAULT);
    sensor->set_denoise(sensor, CAMERA_DENOISE_DEFAULT);
    sensor->set_gain_ctrl(sensor, 1);             // Enable auto gain
    sensor->set_exposure_ctrl(sensor, 1);         // Enable auto exposure
    sensor->set_awb_gain(sensor, 1);              // Enable auto white balance gain
    sensor->set_wb_mode(sensor, CAMERA_WB_MODE_DEFAULT);
}

void CameraManager::optimizeForWildlife(int lightLevel, int motionLevel) {
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
        return;
    }

    // Adjust settings based on light level
    if (lightLevel < 30) {  // Low light conditions
        sensor->set_aec_value(sensor, 600);       // Longer exposure
        sensor->set_agc_gain(sensor, 10);         // Higher gain
        sensor->set_gainceiling(sensor, GAINCEILING_16X);
    } else if (lightLevel > 70) {  // Bright conditions
        sensor->set_aec_value(sensor, 200);       // Shorter exposure
        sensor->set_agc_gain(sensor, 0);          // Lower gain
        sensor->set_gainceiling(sensor, GAINCEILING_2X);
    } else {  // Normal conditions
        sensor->set_aec_value(sensor, CAMERA_AEC_VALUE_DEFAULT);
        sensor->set_agc_gain(sensor, CAMERA_AGC_GAIN_DEFAULT);
        sensor->set_gainceiling(sensor, CAMERA_GAIN_CEILING_DEFAULT);
    }

    // Adjust for motion level
    if (motionLevel > 50) {  // High motion expected
        sensor->set_aec_value(sensor, 150);       // Faster shutter for motion blur reduction
    }

    // Enable lens correction for wildlife photography
    if (LENS_CORRECTION_ENABLED) {
        sensor->set_lenc(sensor, 1);
    }
}

void CameraManager::setNightMode(bool enable) {
    nightModeEnabled = enable;
    
    if (enable) {
        // Night mode optimizations
        optimizeForWildlife(10, 25);  // Low light, low motion
        configureSensor(1, 1, 0);     // Increased brightness and contrast
    } else {
        // Day mode optimizations
        optimizeForWildlife(50, 50);  // Normal conditions
        configureSensor(0, 0, 0);     // Default settings
    }
}

void CameraManager::resetStatistics() {
    stats = {};
    captureCounter = 0;
}

String CameraManager::getConfiguration() const {
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
        return "Camera sensor not available";
    }

    String config = "Frame Size: ";
    switch (sensor->status.framesize) {
        case FRAMESIZE_UXGA: config += "UXGA(1600x1200)"; break;
        case FRAMESIZE_SXGA: config += "SXGA(1280x1024)"; break;
        case FRAMESIZE_XGA:  config += "XGA(1024x768)"; break;
        case FRAMESIZE_SVGA: config += "SVGA(800x600)"; break;
        case FRAMESIZE_VGA:  config += "VGA(640x480)"; break;
        default: config += "Unknown"; break;
    }
    
    config += ", Quality: " + String(sensor->status.quality);
    config += ", Night Mode: " + String(nightModeEnabled ? "ON" : "OFF");
    
    return config;
}

void CameraManager::cleanup() {
    if (initialized) {
        esp_camera_deinit();
        initialized = false;
        Serial.println("Camera cleaned up");
    }
}

String CameraManager::generateFilename(const String& folder, const String& extension) {
    // Get current time
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    // Generate filename with timestamp and counter
    char filename[100];
    snprintf(filename, sizeof(filename), "%s/IMG_%04d%02d%02d_%02d%02d%02d_%04d%s",
             folder.c_str(),
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_sec,
             captureCounter++,
             extension.c_str());

    return String(filename);
}

bool CameraManager::ensureDirectory(const String& path) {
    if (SD_MMC.exists(path.c_str())) {
        return true;
    }

    return SD_MMC.mkdir(path.c_str());
}

void CameraManager::updateStats(bool success, uint32_t captureTime, size_t imageSize) {
    stats.totalCaptures++;
    
    if (success) {
        stats.successfulCaptures++;
        
        // Update running averages
        stats.averageCaptureTime = ((stats.averageCaptureTime * (stats.successfulCaptures - 1)) + captureTime) / stats.successfulCaptures;
        stats.averageImageSize = ((stats.averageImageSize * (stats.successfulCaptures - 1)) + imageSize) / stats.successfulCaptures;
    } else {
        stats.failedCaptures++;
    }
    
    stats.successRate = (float)stats.successfulCaptures / stats.totalCaptures;
}

camera_config_t CameraManager::getCameraConfig() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = CAMERA_PIXEL_FORMAT;
    config.frame_size = CAMERA_FRAME_SIZE;
    config.jpeg_quality = CAMERA_JPEG_QUALITY;
    config.fb_count = CAMERA_FB_COUNT;
    config.grab_mode = CAMERA_GRAB_MODE;

    return config;
}