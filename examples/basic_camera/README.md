# Basic Camera Example

## Simple ESP32 Camera Capture Test

This example demonstrates basic camera functionality without the complexity of the full wildlife monitoring system. Use this to verify camera hardware and learn the basics.

### Hardware Required

- ESP32-S3-DevKitC-1 or compatible board
- OV2640 camera module
- MicroSD card (optional, for image storage)
- Jumper wires for connections

### Wiring Diagram

```
ESP32-S3-DevKitC-1    OV2640 Camera Module
==================    ====================
3.3V             ──── VCC
GND              ──── GND
GPIO 0           ──── XCLK
GPIO 26          ──── SIOD (I2C Data)
GPIO 27          ──── SIOC (I2C Clock)
GPIO 35          ──── Y9 (Data 9)
GPIO 34          ──── Y8 (Data 8)
GPIO 39          ──── Y7 (Data 7)
GPIO 36          ──── Y6 (Data 6)
GPIO 21          ──── Y5 (Data 5)
GPIO 19          ──── Y4 (Data 4)
GPIO 18          ──── Y3 (Data 3)
GPIO 5           ──── Y2 (Data 2)
GPIO 25          ──── VSYNC
GPIO 23          ──── HREF
GPIO 22          ──── PCLK
GPIO 32          ──── PWDN (Power Down)
```

### Code Files

#### platformio.ini
```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

lib_deps = 
    espressif/esp32-camera@^2.0.4

monitor_speed = 115200
upload_speed = 921600

build_flags = 
    -DCORE_DEBUG_LEVEL=0
    -DBOARD_HAS_PSRAM
    -DARDUINO_RUNNING_CORE=1
```

#### src/main.cpp
```cpp
#include <Arduino.h>
#include <esp_camera.h>
#include <SD_MMC.h>

// Camera pin definitions for AI-Thinker ESP32-CAM
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

// Global variables
bool cameraInitialized = false;
bool sdCardInitialized = false;
int imageCounter = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("ESP32 Camera Basic Example");
    Serial.println("==========================");
    
    // Initialize camera
    if (initCamera()) {
        Serial.println("✓ Camera initialized successfully");
        cameraInitialized = true;
    } else {
        Serial.println("✗ Camera initialization failed");
    }
    
    // Initialize SD card (optional)
    if (initSDCard()) {
        Serial.println("✓ SD card initialized");
        sdCardInitialized = true;
    } else {
        Serial.println("⚠ SD card not available (images will not be saved)");
    }
    
    Serial.println("\nReady! Send any character via serial to capture image.");
}

void loop() {
    // Wait for serial input to trigger capture
    if (Serial.available() > 0) {
        Serial.read(); // Clear the input
        captureAndDisplayImage();
    }
    
    delay(100);
}

bool initCamera() {
    camera_config_t config;
    
    // Camera configuration
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
    config.pixel_format = PIXFORMAT_JPEG;
    
    // Frame size and quality
    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
        config.jpeg_quality = 10;            // High quality
        config.fb_count = 2;
        Serial.println("PSRAM found - using high quality settings");
    } else {
        config.frame_size = FRAMESIZE_SVGA;  // 800x600
        config.jpeg_quality = 12;
        config.fb_count = 1;
        Serial.println("PSRAM not found - using standard settings");
    }
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    // Get camera sensor and configure settings
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor != NULL) {
        // Configure sensor for optimal image quality
        sensor->set_brightness(sensor, 0);      // -2 to 2
        sensor->set_contrast(sensor, 0);        // -2 to 2
        sensor->set_saturation(sensor, 0);      // -2 to 2
        sensor->set_special_effect(sensor, 0);  // 0=No Effect
        sensor->set_whitebal(sensor, 1);        // Enable auto white balance
        sensor->set_awb_gain(sensor, 1);        // Enable AWB gain
        sensor->set_wb_mode(sensor, 0);         // Auto WB mode
        sensor->set_exposure_ctrl(sensor, 1);   // Enable exposure control
        sensor->set_aec2(sensor, 0);            // AEC2 off
        sensor->set_ae_level(sensor, 0);        // AE level 0
        sensor->set_aec_value(sensor, 300);     // AEC value
        sensor->set_gain_ctrl(sensor, 1);       // Enable gain control
        sensor->set_agc_gain(sensor, 0);        // AGC gain 0
        sensor->set_gainceiling(sensor, (gainceiling_t)0);  // Gain ceiling 0
        sensor->set_bpc(sensor, 0);             // BPC off
        sensor->set_wpc(sensor, 1);             // WPC on
        sensor->set_raw_gma(sensor, 1);         // Raw GMA on
        sensor->set_lenc(sensor, 1);            // Lens correction on
        sensor->set_hmirror(sensor, 0);         // H-mirror off
        sensor->set_vflip(sensor, 0);           // V-flip off
        sensor->set_dcw(sensor, 1);             // DCW on
        
        Serial.println("Camera sensor configured");
    }
    
    return true;
}

bool initSDCard() {
    if (!SD_MMC.begin()) {
        return false;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        return false;
    }
    
    // Create images directory
    if (!SD_MMC.exists("/images")) {
        SD_MMC.mkdir("/images");
    }
    
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD card size: %lluMB\n", cardSize);
    
    return true;
}

void captureAndDisplayImage() {
    if (!cameraInitialized) {
        Serial.println("✗ Camera not initialized");
        return;
    }
    
    Serial.println("\nCapturing image...");
    
    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("✗ Camera capture failed");
        return;
    }
    
    Serial.printf("✓ Image captured: %dx%d pixels, %d bytes\n", 
                  fb->width, fb->height, fb->len);
    
    // Save to SD card if available
    if (sdCardInitialized) {
        String filename = saveImageToSD(fb);
        if (filename.length() > 0) {
            Serial.printf("✓ Image saved: %s\n", filename.c_str());
        } else {
            Serial.println("✗ Failed to save image");
        }
    }
    
    // Display image information
    displayImageInfo(fb);
    
    // Return frame buffer
    esp_camera_fb_return(fb);
    
    imageCounter++;
    Serial.printf("\nTotal images captured: %d\n", imageCounter);
    Serial.println("Send any character to capture another image...\n");
}

String saveImageToSD(camera_fb_t* fb) {
    // Generate filename with timestamp
    String filename = "/images/img_" + String(millis()) + "_" + 
                     String(imageCounter, DEC) + ".jpg";
    
    File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    if (!file) {
        return "";
    }
    
    size_t written = file.write(fb->buf, fb->len);
    file.close();
    
    if (written != fb->len) {
        return "";
    }
    
    return filename;
}

void displayImageInfo(camera_fb_t* fb) {
    Serial.println("Image Information:");
    Serial.printf("  Width: %d pixels\n", fb->width);
    Serial.printf("  Height: %d pixels\n", fb->height);
    Serial.printf("  Size: %d bytes\n", fb->len);
    Serial.printf("  Format: %s\n", (fb->format == PIXFORMAT_JPEG) ? "JPEG" : "Other");
    
    // Calculate compression ratio
    int uncompressedSize = fb->width * fb->height * 3; // Assume 24-bit RGB
    float compressionRatio = (float)uncompressedSize / fb->len;
    Serial.printf("  Compression: %.1fx\n", compressionRatio);
    
    // Memory usage
    Serial.printf("  Free heap: %d bytes\n", ESP.getFreeHeap());
    
    // Performance metrics
    static unsigned long lastCaptureTime = 0;
    unsigned long currentTime = millis();
    if (lastCaptureTime > 0) {
        Serial.printf("  Time since last capture: %lu ms\n", currentTime - lastCaptureTime);
    }
    lastCaptureTime = currentTime;
}
```

### Usage Instructions

1. **Hardware Setup:**
   ```bash
   1. Connect camera module to ESP32 according to wiring diagram
   2. Insert formatted microSD card (optional)
   3. Connect ESP32 to computer via USB
   ```

2. **Software Setup:**
   ```bash
   # Install PlatformIO
   pip install platformio
   
   # Build and upload
   pio run --target upload
   
   # Open serial monitor
   pio device monitor
   ```

3. **Testing:**
   ```bash
   # You should see:
   ESP32 Camera Basic Example
   ==========================
   PSRAM found - using high quality settings
   ✓ Camera initialized successfully
   ✓ SD card initialized
   SD card size: 16384MB
   Camera sensor configured
   
   Ready! Send any character via serial to capture image.
   
   # Type any character and press Enter to capture
   ```

4. **Expected Output:**
   ```bash
   Capturing image...
   ✓ Image captured: 1600x1200 pixels, 245760 bytes
   ✓ Image saved: /images/img_12345_0.jpg
   Image Information:
     Width: 1600 pixels
     Height: 1200 pixels
     Size: 245760 bytes
     Format: JPEG
     Compression: 26.2x
     Free heap: 4567890 bytes
   
   Total images captured: 1
   Send any character to capture another image...
   ```

### Configuration Options

#### Image Quality Settings
```cpp
// High quality (larger files)
config.frame_size = FRAMESIZE_UXGA;   // 1600x1200
config.jpeg_quality = 8;              // Higher quality

// Balanced settings
config.frame_size = FRAMESIZE_SVGA;   // 800x600
config.jpeg_quality = 12;             // Good quality

// Low quality (smaller files)
config.frame_size = FRAMESIZE_VGA;    // 640x480
config.jpeg_quality = 20;             // Lower quality
```

#### Camera Sensor Adjustments
```cpp
// Brightness adjustment (-2 to 2)
sensor->set_brightness(sensor, 1);    // Brighter

// Contrast adjustment (-2 to 2)
sensor->set_contrast(sensor, 1);      // Higher contrast

// Saturation adjustment (-2 to 2)
sensor->set_saturation(sensor, -1);   // Less saturated

// Special effects
sensor->set_special_effect(sensor, 2); // Grayscale
// 0=No Effect, 1=Negative, 2=Grayscale, 3=Red Tint, 
// 4=Green Tint, 5=Blue Tint, 6=Sepia
```

### Troubleshooting

#### Camera Not Detected
```bash
# Check connections
1. Verify all camera pins connected correctly
2. Ensure 3.3V power supply to camera
3. Check for loose connections
4. Try a different camera module

# Serial output shows:
Camera init failed with error 0x105
```

#### Poor Image Quality
```bash
# Adjust camera settings
1. Increase JPEG quality (lower number = higher quality)
2. Adjust brightness/contrast for lighting conditions
3. Clean camera lens
4. Ensure stable power supply
```

#### SD Card Issues
```bash
# SD card not detected
1. Format card as FAT32
2. Use Class 10 or faster card
3. Maximum 32GB capacity recommended
4. Check card insertion and contacts
```

#### Memory Issues
```bash
# Out of memory errors
1. Reduce frame size
2. Increase JPEG compression (higher quality number)
3. Use single frame buffer (fb_count = 1)
4. Monitor free heap memory
```

### Next Steps

Once this basic example works:

1. **Explore Advanced Features:**
   - Motion detection integration
   - Time-lapse photography
   - Image processing and analysis

2. **Add Networking:**
   - WiFi connectivity for image transmission
   - LoRa module for remote communication

3. **Power Management:**
   - Battery monitoring
   - Solar charging integration
   - Deep sleep optimization

4. **Study the Full Project:**
   - Return to main firmware for complete wildlife camera system
   - Understand modular architecture
   - Implement additional sensors

---

**This example provides a solid foundation for understanding ESP32 camera operation before tackling the complete wildlife monitoring system.**