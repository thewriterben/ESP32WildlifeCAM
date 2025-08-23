# ESP32-S3-CAM Deployment Guide

This comprehensive guide provides step-by-step instructions for deploying the ESP32-S3-CAM board for wildlife monitoring applications. The ESP32-S3-CAM offers enhanced performance with dual-core processing, larger memory, and improved camera capabilities, making it ideal for high-resolution monitoring and AI-enhanced wildlife detection.

## Prerequisites

### Hardware Requirements
- **ESP32-S3-CAM board** (with built-in USB connector)
- **USB-C cable** for programming and power
- **Computer** with Windows, macOS, or Linux
- **MicroSD card** (16-128GB, Class 10 recommended)
- **External antenna** (optional, for improved WiFi range)
- **PIR motion sensor** (HC-SR501 or similar)
- **Power supply** (5V 2A recommended for stable operation)

### Software Requirements
- **Arduino IDE 2.0+** or **PlatformIO**
- **ESP32 board package 2.0.5+** for ESP32-S3 support
- **Required libraries:**
  - ESP32 Camera Driver
  - WiFi library (built-in)
  - SD library (built-in)
  - ArduinoJson (for configuration)
  - AsyncWebServer (optional, for web interface)

### Additional Components
- **Breadboard or PCB** for prototype circuits
- **Jumper wires** for sensor connections
- **Weatherproof enclosure** (IP65 rated minimum)
- **Solar panel and battery** for field deployment
- **External flash LED** (optional, for night photography)

## Step-by-Step Instructions

### 1. Setting Up Arduino IDE

1. **Download and install Arduino IDE** from [arduino.cc](https://www.arduino.cc/en/software)

2. **Add ESP32 board support:**
   - Open Arduino IDE
   - Go to `File` > `Preferences`
   - In **Additional Boards Manager URLs**, add:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Click **OK**

3. **Install ESP32 board package:**
   - Go to `Tools` > `Board` > `Boards Manager`
   - Search for **"ESP32"**
   - Install **"ESP32 by Espressif Systems"** (version 2.0.5 or later)

4. **Install required libraries:**
   - Go to `Tools` > `Manage Libraries`
   - Install the following libraries:
     - **ArduinoJson** by Benoit Blanchon
     - **ESPAsyncWebServer** by lacamera (optional)

### 2. Connecting the Board

> **✅ Advantage:** The ESP32-S3-CAM typically has a built-in USB connector, making programming much easier than older ESP32-CAM boards.

1. **Connect via USB-C:**
   - Use a quality USB-C cable to connect the board to your computer
   - The board should power on and be recognized by your system

2. **Select board and port in Arduino IDE:**
   - Go to `Tools` > `Board` > `ESP32 Arduino`
   - Select **"ESP32S3 Dev Module"** or **"ESP32-S3-CAM"** if available
   - Go to `Tools` > `Port` and select the appropriate COM port
   - Set **Upload Speed** to **921600** for faster uploads

3. **Configure board settings:**
   - **CPU Frequency:** 240MHz
   - **Flash Mode:** QIO
   - **Flash Size:** 8MB (usually)
   - **Partition Scheme:** 8M Flash (3MB APP/5MB SPIFFS)
   - **PSRAM:** Enabled

### 3. Writing the Code

1. **Create a new sketch** in Arduino IDE

2. **Basic camera initialization code:**
   ```cpp
   #include "esp_camera.h"
   #include "WiFi.h"
   #include "SD_MMC.h"
   
   // ESP32-S3-CAM pin definitions
   #define PWDN_GPIO_NUM    -1  // Not connected
   #define RESET_GPIO_NUM   -1  // Not connected
   #define XCLK_GPIO_NUM    40
   #define SIOD_GPIO_NUM    17  // SDA
   #define SIOC_GPIO_NUM    18  // SCL
   
   #define Y9_GPIO_NUM      39
   #define Y8_GPIO_NUM      41
   #define Y7_GPIO_NUM      42
   #define Y6_GPIO_NUM      12
   #define Y5_GPIO_NUM       3
   #define Y4_GPIO_NUM      14
   #define Y3_GPIO_NUM      47
   #define Y2_GPIO_NUM      13
   #define VSYNC_GPIO_NUM   21
   #define HREF_GPIO_NUM    38
   #define PCLK_GPIO_NUM    11
   
   #define LED_GPIO_NUM     48  // Built-in LED/Flash
   
   void setup() {
     Serial.begin(115200);
     
     // Camera configuration
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
     config.pixel_format = PIXFORMAT_JPEG;
     config.frame_size = FRAMESIZE_QXGA;  // 2048x1536 (high resolution)
     config.jpeg_quality = 10;            // High quality
     config.fb_count = 3;                 // Multiple frame buffers
     config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
     
     // Initialize camera
     esp_err_t err = esp_camera_init(&config);
     if (err != ESP_OK) {
       Serial.printf("Camera init failed with error 0x%x", err);
       return;
     }
     
     Serial.println("ESP32-S3-CAM initialized successfully");
   }
   
   void loop() {
     // Your main code here
   }
   ```

3. **Save your sketch** with a descriptive name

### 4. Compiling the Code

1. **Verify board configuration** in `Tools` menu
2. **Click the checkmark button** (Verify) to compile
3. **Check for errors** in the output console
4. **Fix any compilation errors** before proceeding

> **⚠️ Warning:** Ensure PSRAM is enabled in board settings for high-resolution image capture

### 5. Uploading the Code

1. **Click the upload button** (arrow) in Arduino IDE
2. **Monitor the output console** for upload progress
3. **Wait for "Hard resetting via RTS pin"** message
4. **The board will automatically reset** and start running your code

### 6. Testing the Deployment

1. **Open Serial Monitor** (`Tools` > `Serial Monitor`)
2. **Set baud rate to 115200**
3. **Press the Reset button** on the board (if needed)
4. **Verify initialization messages** appear in serial output
5. **Test camera functionality** by taking a test photo

## Hardware Specifications

### Board Specifications
- **Microcontroller:** ESP32-S3-WROOM-1 (Dual-core Xtensa LX7, 240MHz)
- **Flash Memory:** 8MB SPI Flash (typical)
- **PSRAM:** 8MB QSPI PSRAM
- **WiFi:** 802.11 b/g/n (2.4GHz)
- **Bluetooth:** Bluetooth 5.0 LE
- **USB:** Built-in USB-C connector for programming
- **Operating Voltage:** 3.3V
- **Input Voltage:** 5V (via USB-C)
- **Dimensions:** 50mm × 25mm × 10mm (typical)

### Camera Specifications
- **Sensor:** OV2640 or OV5640 (model dependent)
- **Resolution:** 
  - OV2640: Up to 1600×1200 (UXGA)
  - OV5640: Up to 2592×1944 (5MP)
- **Image Formats:** JPEG, RGB565, YUV422
- **Frame Rates:** Up to 60fps at lower resolutions
- **Features:** Auto exposure, auto white balance, auto gain control
- **Field of View:** ~66° diagonal
- **Low Light Performance:** Improved sensitivity vs standard ESP32-CAM

### Memory and Storage
- **RAM:** 512KB SRAM + 8MB PSRAM
- **Flash:** 8MB for program storage and file system
- **External Storage:** MicroSD card slot (up to 128GB)
- **Partition Scheme:** Configurable (recommended: 3MB APP + 5MB SPIFFS)

### Connectivity Options
- **WiFi:** Built-in with external antenna connector
- **USB:** USB-C for programming and serial communication
- **Serial:** Additional UART pins available
- **I2C:** Hardware I2C bus available (separate from camera I2C)
- **SPI:** Hardware SPI bus available

### Power Requirements
- **Operating Current:** 100mA (active), 7mA (light sleep)
- **Camera Current:** 180mA during capture
- **Deep Sleep Current:** 7µA
- **Input Voltage Range:** 3.0V - 3.6V
- **Recommended Power Supply:** 5V 2A for stable operation

## Pin Configuration

### Camera Pin Mapping
```cpp
// ESP32-S3-CAM Camera Pins
#define PWDN_GPIO_NUM    -1    // Power down (not connected)
#define RESET_GPIO_NUM   -1    // Reset (not connected)
#define XCLK_GPIO_NUM    40    // Camera clock
#define SIOD_GPIO_NUM    17    // Camera I2C data (SDA)
#define SIOC_GPIO_NUM    18    // Camera I2C clock (SCL)

// Camera data pins
#define Y9_GPIO_NUM      39    // Data bit 9
#define Y8_GPIO_NUM      41    // Data bit 8
#define Y7_GPIO_NUM      42    // Data bit 7
#define Y6_GPIO_NUM      12    // Data bit 6
#define Y5_GPIO_NUM       3    // Data bit 5
#define Y4_GPIO_NUM      14    // Data bit 4
#define Y3_GPIO_NUM      47    // Data bit 3
#define Y2_GPIO_NUM      13    // Data bit 2
#define VSYNC_GPIO_NUM   21    // Vertical sync
#define HREF_GPIO_NUM    38    // Horizontal reference
#define PCLK_GPIO_NUM    11    // Pixel clock

// Built-in peripherals
#define LED_GPIO_NUM     48    // Built-in LED/Flash
```

### Available GPIO for External Sensors
```cpp
// Available pins for external connections (ESP32-S3 specific)
#define AVAILABLE_GPIO_PINS {1, 2, 4, 5, 6, 7, 8, 9, 10, 15, 16, 43, 44, 45, 46}

// Recommended sensor pin assignments
#define PIR_SENSOR_PIN    1    // PIR motion sensor
#define STATUS_LED_PIN    2    // External status LED
#define BUTTON_PIN        4    // User button
#define ANALOG_SENSOR_1   5    // Analog sensor input
#define ANALOG_SENSOR_2   6    // Analog sensor input
#define I2C_SDA_EXT       8    // External I2C SDA
#define I2C_SCL_EXT       9    // External I2C SCL
#define SPI_CS_EXT       10    // External SPI CS
#define UART_TX_EXT      43    // External UART TX
#define UART_RX_EXT      44    // External UART RX
```

### Pin Conflicts and Limitations
```cpp
// Critical pin usage warnings for ESP32-S3-CAM
struct PinConflicts {
    // Camera pins - DO NOT USE for other purposes
    int camera_pins[] = {3, 11, 12, 13, 14, 17, 18, 21, 38, 39, 40, 41, 42, 47, 48};
    
    // USB pins - avoid using for external connections
    int usb_pins[] = {19, 20};  // USB D- and D+
    
    // Strapping pins - use with caution
    int strapping_pins[] = {0, 3, 45, 46};  // Can affect boot process
    
    // SD card pins (if SD card is used)
    int sd_pins[] = {36, 37, 35, 34, 33, 32};  // SD_D0-3, SD_CLK, SD_CMD
    
    // Available for external use without conflicts
    int safe_pins[] = {1, 2, 4, 5, 6, 7, 8, 9, 10, 15, 16, 43, 44};
    
    // Pin recommendations for common peripherals
    struct {
        int pir_sensor = 1;        // Safe, no conflicts
        int external_led = 2;      // Safe, no conflicts
        int button = 4;            // Safe, no conflicts
        int i2c_sda = 8;          // Dedicated external I2C
        int i2c_scl = 9;          // Dedicated external I2C
        int spi_cs = 10;          // SPI chip select
        int uart_tx = 43;         // External UART
        int uart_rx = 44;         // External UART
    } recommended;
};
```

## Board-Specific Configuration

### High-Resolution Camera Configuration
The ESP32-S3-CAM supports higher resolutions and better image quality:

```cpp
void configureHighResolutionCamera() {
    sensor_t * s = esp_camera_sensor_get();
    
    // Configure for high-resolution wildlife photography
    s->set_framesize(s, FRAMESIZE_QXGA);    // 2048x1536 resolution
    s->set_quality(s, 8);                   // High quality (lower number = better quality)
    
    // Optimize for wildlife photography
    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 1);       // Slightly increased contrast
    s->set_saturation(s, 0);     // Natural colors
    s->set_special_effect(s, 0); // No special effects
    s->set_whitebal(s, 1);       // Auto white balance
    s->set_awb_gain(s, 1);       // Auto white balance gain
    s->set_wb_mode(s, 0);        // Auto white balance mode
    s->set_exposure_ctrl(s, 1);  // Auto exposure
    s->set_aec2(s, 1);           // Automatic exposure control
    s->set_ae_level(s, 0);       // Auto exposure level
    s->set_aec_value(s, 300);    // Exposure value
    s->set_gain_ctrl(s, 1);      // Auto gain
    s->set_agc_gain(s, 0);       // Auto gain control
    s->set_gainceiling(s, (gainceiling_t)2); // Gain ceiling
    s->set_bpc(s, 1);            // Black pixel correction
    s->set_wpc(s, 1);            // White pixel correction
    s->set_raw_gma(s, 1);        // Raw gamma
    s->set_lenc(s, 1);           // Lens correction
    s->set_hmirror(s, 0);        // Horizontal mirror
    s->set_vflip(s, 0);          // Vertical flip
    s->set_dcw(s, 1);            // DCW (Downsize EN)
    s->set_colorbar(s, 0);       // Disable color bar test pattern
}
```

### Advanced Power Management
ESP32-S3 offers improved power management features:

```cpp
#include "esp_pm.h"
#include "esp_sleep.h"

void configureAdvancedPowerManagement() {
    // Configure CPU frequency scaling for ESP32-S3
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 10,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);
    
    // Configure deep sleep wake sources
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_1, 1);  // PIR sensor wake
    esp_sleep_enable_timer_wakeup(10 * 60 * 1000000);  // 10 minute timer
    
    // Enable PSRAM power down in sleep
    esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_OFF);
    
    Serial.println("Advanced power management configured");
}

void enterOptimizedDeepSleep() {
    // Flush all pending operations
    Serial.flush();
    
    // Power down camera if needed
    // (ESP32-S3-CAM may not have dedicated power down pin)
    
    // Disconnect WiFi and Bluetooth
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    btStop();
    
    // Enter deep sleep with optimized settings
    esp_deep_sleep_start();
}
```

### Memory Optimization for Large Images
Take advantage of the larger memory in ESP32-S3:

```cpp
struct ESP32S3MemoryConfig {
    // Larger partition scheme for ESP32-S3
    String partition_scheme = "default";       // Use default 8MB partition
    uint32_t app_partition_size = 3072;        // KB - larger application space
    uint32_t spiffs_partition_size = 5120;     // KB - more file storage
    
    // Frame buffer optimization
    uint8_t fb_count = 3;                      // Multiple frame buffers
    bool enable_psram = true;                  // Always enable PSRAM
    bool psram_for_fb = true;                  // Use PSRAM for frame buffers
    
    // High-resolution image settings
    framesize_t max_framesize = FRAMESIZE_QXGA; // 2048x1536
    uint8_t jpeg_quality_high = 6;             // Very high quality
    uint8_t jpeg_quality_balanced = 10;        // Good quality
    uint8_t jpeg_quality_fast = 15;            // Faster processing
    
    // Memory allocation
    bool enable_spiram_cache = true;           // Enable SPIRAM cache
    size_t spiram_heap_size = 6 * 1024 * 1024; // 6MB SPIRAM heap
};

void optimizeMemoryForHighRes() {
    // Check PSRAM availability
    if (!psramFound()) {
        Serial.println("ERROR: PSRAM not found! High-resolution mode requires PSRAM");
        return;
    }
    
    size_t psram_size = ESP.getPsramSize();
    size_t free_psram = ESP.getFreePsram();
    
    Serial.printf("PSRAM Total: %d bytes\n", psram_size);
    Serial.printf("PSRAM Free: %d bytes\n", free_psram);
    
    // Configure camera for high resolution if enough PSRAM
    if (free_psram > 2 * 1024 * 1024) {  // At least 2MB free
        configureHighResolutionCamera();
        Serial.println("High-resolution mode enabled");
    } else {
        Serial.println("Insufficient PSRAM for high-resolution mode");
    }
}
```

## Troubleshooting Section

### Common Issues and Solutions

#### Camera Initialization Failed
**Symptoms:**
- "Camera init failed with error 0x[hex]" in serial output
- PSRAM allocation errors
- High-resolution capture failures

**Solutions:**
1. **Check PSRAM configuration:**
   ```cpp
   void troubleshootPSRAM() {
       if (!psramFound()) {
           Serial.println("ERROR: PSRAM not detected");
           Serial.println("Check board settings:");
           Serial.println("- Tools > PSRAM: Enabled");
           Serial.println("- Verify ESP32-S3 variant with PSRAM");
           return;
       }
       
       size_t psram_size = ESP.getPsramSize();
       size_t free_psram = ESP.getFreePsram();
       
       Serial.printf("PSRAM Size: %d bytes\n", psram_size);
       Serial.printf("Free PSRAM: %d bytes\n", free_psram);
       
       if (free_psram < 1024 * 1024) {  // Less than 1MB
           Serial.println("WARNING: Low PSRAM available");
           Serial.println("Try reducing frame buffer count or resolution");
       }
   }
   ```

2. **Verify camera sensor communication:**
   ```cpp
   void testCameraSensor() {
       Wire.begin(SIOD_GPIO_NUM, SIOC_GPIO_NUM);
       
       // Test OV2640 (address 0x30)
       Wire.beginTransmission(0x30);
       uint8_t error = Wire.endTransmission();
       
       if (error == 0) {
           Serial.println("OV2640 sensor detected");
       } else {
           // Test OV5640 (address 0x3C)
           Wire.beginTransmission(0x3C);
           error = Wire.endTransmission();
           if (error == 0) {
               Serial.println("OV5640 sensor detected");
           } else {
               Serial.println("No camera sensor detected");
           }
       }
   }
   ```

#### USB Connection Issues
**Symptoms:**
- Board not recognized by computer
- Upload failures
- Serial monitor not working

**Solutions:**
1. **Check USB-C connection:**
   ```
   USB Connection Troubleshooting:
   
   1. Use a quality USB-C data cable (not charge-only)
   2. Try different USB ports on your computer
   3. Install ESP32-S3 USB drivers if needed
   4. Check Device Manager (Windows) for proper recognition
   5. Try holding Boot button while connecting USB
   ```

2. **USB CDC mode configuration:**
   ```cpp
   // In Arduino IDE board settings:
   // - USB CDC On Boot: Enabled
   // - Upload Mode: UART0 / Hardware CDC
   
   void setupUSBSerial() {
       Serial.begin(115200);
       
       // Wait for USB serial connection
       while (!Serial && millis() < 5000) {
           delay(100);
       }
       
       if (Serial) {
           Serial.println("USB Serial connected");
       } else {
           // Fall back to hardware serial if USB fails
           Serial0.begin(115200);
           Serial0.println("Using hardware serial");
       }
   }
   ```

#### High-Resolution Image Issues
**Symptoms:**
- Out of memory errors during capture
- Image corruption or incomplete captures
- Slow capture performance

**Solutions:**
1. **Optimize capture settings:**
   ```cpp
   void optimizeHighResCapture() {
       sensor_t * s = esp_camera_sensor_get();
       
       // Check available memory before capture
       size_t free_heap = ESP.getFreeHeap();
       size_t free_psram = ESP.getFreePsram();
       
       if (free_psram < 500000) {  // Less than 500KB PSRAM
           Serial.println("Low PSRAM - reducing quality/resolution");
           s->set_framesize(s, FRAMESIZE_UXGA);  // Lower resolution
           s->set_quality(s, 12);                // Lower quality
       }
       
       // Capture with error handling
       camera_fb_t * fb = esp_camera_fb_get();
       if (!fb) {
           Serial.println("Camera capture failed");
           // Try again with lower settings
           s->set_framesize(s, FRAMESIZE_SVGA);
           fb = esp_camera_fb_get();
       }
       
       if (fb) {
           Serial.printf("Capture successful: %dx%d, %d bytes\n", 
                        fb->width, fb->height, fb->len);
           esp_camera_fb_return(fb);
       }
   }
   ```

### Debug Tips

#### Memory Monitoring for ESP32-S3
```cpp
void printDetailedMemoryInfo() {
    Serial.println("=== ESP32-S3 Memory Status ===");
    
    // Heap memory
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Largest free block: %d bytes\n", 
                 heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    Serial.printf("Min free heap: %d bytes\n", ESP.getMinFreeHeap());
    
    // PSRAM memory
    if (psramFound()) {
        Serial.printf("PSRAM total: %d bytes\n", ESP.getPsramSize());
        Serial.printf("PSRAM free: %d bytes\n", ESP.getFreePsram());
        Serial.printf("PSRAM used: %d bytes\n", 
                     ESP.getPsramSize() - ESP.getFreePsram());
    }
    
    // Flash memory
    Serial.printf("Flash size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("Free sketch space: %d bytes\n", ESP.getFreeSketchSpace());
    
    Serial.println("==============================");
}
```

## Example Code Snippets

### High-Resolution Camera Initialization
```cpp
#include "esp_camera.h"

bool initializeHighResCamera() {
    camera_config_t config;
    
    // ESP32-S3-CAM pin configuration
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 13;
    config.pin_d1 = 47;
    config.pin_d2 = 14;
    config.pin_d3 = 3;
    config.pin_d4 = 12;
    config.pin_d5 = 42;
    config.pin_d6 = 41;
    config.pin_d7 = 39;
    config.pin_xclk = 40;
    config.pin_pclk = 11;
    config.pin_vsync = 21;
    config.pin_href = 38;
    config.pin_sscb_sda = 17;
    config.pin_sscb_scl = 18;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    
    // High-resolution settings
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QXGA;      // 2048x1536
    config.jpeg_quality = 8;                 // High quality
    config.fb_count = 3;                     // Multiple buffers for smooth capture
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM; // Use PSRAM for frame buffers
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed: 0x%x\n", err);
        return false;
    }
    
    configureHighResolutionCamera();
    return true;
}
```

### Advanced PIR Motion Detection
```cpp
#define PIR_PIN 1
#define LED_PIN 48

volatile bool motionDetected = false;
volatile unsigned long motionTime = 0;
const unsigned long MOTION_COOLDOWN = 15000;  // 15 second cooldown

void IRAM_ATTR pirInterrupt() {
    unsigned long currentTime = millis();
    if (currentTime - motionTime > MOTION_COOLDOWN) {
        motionDetected = true;
        motionTime = currentTime;
    }
}

void setupAdvancedPIR() {
    pinMode(PIR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    
    // Configure interrupt
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirInterrupt, RISING);
    
    // Brief LED flash to indicate setup
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("Advanced PIR sensor configured");
}

void handleAdvancedMotionDetection() {
    if (motionDetected) {
        Serial.println("Wildlife motion detected!");
        
        // Turn on LED during capture sequence
        digitalWrite(LED_PIN, HIGH);
        
        // Capture sequence with different resolutions/qualities
        captureWildlifeSequence();
        
        digitalWrite(LED_PIN, LOW);
        motionDetected = false;
    }
}

void captureWildlifeSequence() {
    sensor_t * s = esp_camera_sensor_get();
    
    // Capture 1: High resolution for detail
    s->set_framesize(s, FRAMESIZE_QXGA);
    s->set_quality(s, 6);
    camera_fb_t * fb_high = esp_camera_fb_get();
    if (fb_high) {
        saveImageWithTimestamp(fb_high, "high");
        esp_camera_fb_return(fb_high);
    }
    
    delay(2000);
    
    // Capture 2: Medium resolution for quick processing
    s->set_framesize(s, FRAMESIZE_UXGA);
    s->set_quality(s, 10);
    camera_fb_t * fb_med = esp_camera_fb_get();
    if (fb_med) {
        saveImageWithTimestamp(fb_med, "med");
        esp_camera_fb_return(fb_med);
    }
    
    delay(2000);
    
    // Capture 3: Low resolution for transmission
    s->set_framesize(s, FRAMESIZE_SVGA);
    s->set_quality(s, 15);
    camera_fb_t * fb_low = esp_camera_fb_get();
    if (fb_low) {
        saveImageWithTimestamp(fb_low, "low");
        esp_camera_fb_return(fb_low);
    }
}
```

### WiFi with Fallback AP Mode
```cpp
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);
bool apMode = false;

void setupWiFiWithFallback() {
    // Try to connect to saved WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin("your_ssid", "your_password");
    
    Serial.print("Connecting to WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(1000);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.printf("Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        setupWebServer();
    } else {
        Serial.println();
        Serial.println("WiFi connection failed, starting AP mode");
        startAPMode();
    }
}

void startAPMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32S3-Wildlife-Cam", "wildlife123");
    apMode = true;
    
    Serial.printf("AP Mode started: ESP32S3-Wildlife-Cam\n");
    Serial.printf("IP address: %s\n", WiFi.softAPIP().toString().c_str());
    
    setupWebServer();
}

void setupWebServer() {
    server.on("/", handleRoot);
    server.on("/capture", handleCapture);
    server.on("/status", handleStatus);
    server.begin();
    Serial.println("Web server started");
}

void handleRoot() {
    String html = R"(
<!DOCTYPE html>
<html>
<head><title>ESP32-S3 Wildlife Camera</title></head>
<body>
<h1>Wildlife Camera Control</h1>
<p><a href="/capture">Capture Photo</a></p>
<p><a href="/status">System Status</a></p>
</body>
</html>
    )";
    server.send(200, "text/html", html);
}

void handleCapture() {
    camera_fb_t * fb = esp_camera_fb_get();
    if (fb) {
        server.sendHeader("Content-Type", "image/jpeg");
        server.sendHeader("Content-Length", String(fb->len));
        server.send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);
        esp_camera_fb_return(fb);
    } else {
        server.send(500, "text/plain", "Camera capture failed");
    }
}

void handleStatus() {
    String status = "System Status:\n";
    status += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    status += "Free PSRAM: " + String(ESP.getFreePsram()) + " bytes\n";
    status += "WiFi Mode: " + String(apMode ? "AP" : "STA") + "\n";
    status += "Signal Strength: " + String(WiFi.RSSI()) + " dBm\n";
    
    server.send(200, "text/plain", status);
}
```

---

*The ESP32-S3-CAM provides enhanced performance and capabilities for advanced wildlife monitoring applications, with its dual-core processing, larger memory, and improved camera support making it ideal for high-resolution monitoring and AI-enhanced detection systems.*