/**
 * @file camera_integration.cpp
 * @brief Minimal camera integration implementation for SystemManager
 * @author thewriterben
 * @date 2025-09-03
 */

#include "camera_integration.h"

// AI-Thinker ESP32-CAM pin definitions
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

CameraIntegration::CameraIntegration() : initialized(false) {
    // Initialize camera configuration to default values
    memset(&camera_config, 0, sizeof(camera_config_t));
    configureAIThinkerESP32CAM();
}

CameraIntegration::~CameraIntegration() {
    if (initialized) {
        esp_camera_deinit();
        initialized = false;
    }
}

bool CameraIntegration::init() {
    if (initialized) {
        return true; // Already initialized
    }
    
    Serial.println("Initializing ESP32-CAM...");
    
    // Configure camera power down pin
    if (PWDN_GPIO_NUM >= 0) {
        pinMode(PWDN_GPIO_NUM, OUTPUT);
        digitalWrite(PWDN_GPIO_NUM, LOW); // Camera enabled (active low)
    }
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    // Get camera sensor
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == NULL) {
        Serial.println("Failed to get camera sensor");
        esp_camera_deinit();
        return false;
    }
    
    // Configure sensor settings for wildlife photography
    configureSensorSettings();
    
    initialized = true;
    Serial.println("Camera initialized successfully");
    
    return true;
}

camera_fb_t* CameraIntegration::captureImage() {
    if (!initialized) {
        Serial.println("Camera not initialized");
        return nullptr;
    }
    
    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return nullptr;
    }
    
    Serial.printf("Image captured: %dx%d, %d bytes\n", fb->width, fb->height, fb->len);
    return fb;
}

void CameraIntegration::releaseFrameBuffer(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}

void CameraIntegration::configureAIThinkerESP32CAM() {
    camera_config.pin_pwdn = PWDN_GPIO_NUM;
    camera_config.pin_reset = RESET_GPIO_NUM;
    camera_config.pin_xclk = XCLK_GPIO_NUM;
    camera_config.pin_sccb_sda = SIOD_GPIO_NUM;
    camera_config.pin_sccb_scl = SIOC_GPIO_NUM;
    
    camera_config.pin_d7 = Y9_GPIO_NUM;
    camera_config.pin_d6 = Y8_GPIO_NUM;
    camera_config.pin_d5 = Y7_GPIO_NUM;
    camera_config.pin_d4 = Y6_GPIO_NUM;
    camera_config.pin_d3 = Y5_GPIO_NUM;
    camera_config.pin_d2 = Y4_GPIO_NUM;
    camera_config.pin_d1 = Y3_GPIO_NUM;
    camera_config.pin_d0 = Y2_GPIO_NUM;
    camera_config.pin_vsync = VSYNC_GPIO_NUM;
    camera_config.pin_href = HREF_GPIO_NUM;
    camera_config.pin_pclk = PCLK_GPIO_NUM;
    
    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    camera_config.xclk_freq_hz = 20000000;
    camera_config.ledc_timer = LEDC_TIMER_0;
    camera_config.ledc_channel = LEDC_CHANNEL_0;
    
    camera_config.pixel_format = PIXFORMAT_JPEG; // YUV422, GRAYSCALE, RGB565, JPEG
    camera_config.frame_size = FRAMESIZE_UXGA;   // QVGA, CIF, VGA, SVGA, XGA, SXGA, UXGA
    
    camera_config.jpeg_quality = 10; // 0-63, lower means higher quality
    camera_config.fb_count = 1;      // When jpeg mode is used, if fb_count is more than one, the driver will work in continuous mode.
    
    // For higher resolution, reduce frame rate and quality
    if (psramFound()) {
        camera_config.jpeg_quality = 10;
        camera_config.fb_count = 2;
        camera_config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
        Serial.println("PSRAM found - using high quality settings");
    } else {
        camera_config.frame_size = FRAMESIZE_SVGA;
        camera_config.jpeg_quality = 12;
        camera_config.fb_count = 1;
        camera_config.grab_mode = CAMERA_GRAB_LATEST;
        Serial.println("PSRAM not found - using conservative settings");
    }
}

void CameraIntegration::configureSensorSettings() {
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        return;
    }
    
    // Wildlife photography optimizations
    sensor->set_brightness(sensor, 0);     // -2 to 2
    sensor->set_contrast(sensor, 0);       // -2 to 2
    sensor->set_saturation(sensor, 0);     // -2 to 2
    sensor->set_special_effect(sensor, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    sensor->set_whitebal(sensor, 1);       // 0 = disable , 1 = enable
    sensor->set_awb_gain(sensor, 1);       // 0 = disable , 1 = enable
    sensor->set_wb_mode(sensor, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    sensor->set_exposure_ctrl(sensor, 1);  // 0 = disable , 1 = enable
    sensor->set_aec2(sensor, 0);           // 0 = disable , 1 = enable
    sensor->set_ae_level(sensor, 0);       // -2 to 2
    sensor->set_aec_value(sensor, 300);    // 0 to 1200
    sensor->set_gain_ctrl(sensor, 1);      // 0 = disable , 1 = enable
    sensor->set_agc_gain(sensor, 0);       // 0 to 30
    sensor->set_gainceiling(sensor, (gainceiling_t)0);  // 0 to 6
    sensor->set_bpc(sensor, 0);            // 0 = disable , 1 = enable
    sensor->set_wpc(sensor, 1);            // 0 = disable , 1 = enable
    sensor->set_raw_gma(sensor, 1);        // 0 = disable , 1 = enable
    sensor->set_lenc(sensor, 1);           // 0 = disable , 1 = enable
    sensor->set_hmirror(sensor, 0);        // 0 = disable , 1 = enable
    sensor->set_vflip(sensor, 0);          // 0 = disable , 1 = enable
    sensor->set_dcw(sensor, 1);            // 0 = disable , 1 = enable
    sensor->set_colorbar(sensor, 0);       // 0 = disable , 1 = enable
    
    Serial.println("Camera sensor configured for wildlife photography");
}