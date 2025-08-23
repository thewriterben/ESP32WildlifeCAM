# Deployment Guide for LilyGO T-Camera Plus S3

This guide provides comprehensive step-by-step instructions for deploying the LilyGO T-Camera Plus S3 board for wildlife monitoring applications.

## Prerequisites

### Hardware Requirements
- **LilyGO T-Camera Plus S3 board**
- **USB-C cable** for programming and power
- **Computer** with Windows, macOS, or Linux
- **MicroSD card** (16-128GB, Class 10 recommended) 
- **External antenna** (optional, for improved WiFi range)
- **PIR motion sensor** (optional, HC-SR501 or similar)
- **Power bank or solar panel** for field deployment

### Software Requirements
- **Arduino IDE 2.0+** or **PlatformIO**
- **ESP32 board package** for Arduino IDE
- **Required libraries:**
  - ESP32 Camera Driver
  - WiFi library (built-in)
  - SD library (built-in)
  - ArduinoJson (for configuration)
  - TFT_eSPI (for display support)

### Additional Components
- **Weatherproof enclosure** (IP65 rated minimum)
- **Mounting hardware** (screws, brackets, straps)
- **Jumper wires** for sensor connections
- **Breadboard or PCB** for prototype circuits

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
   - Install **"ESP32 by Espressif Systems"**

4. **Install required libraries:**
   - Go to `Tools` > `Manage Libraries`
   - Install the following libraries:
     - **ArduinoJson** by Benoit Blanchon
     - **TFT_eSPI** by Bodmer

### 2. Connecting the Board

1. **Connect via USB-C:**
   - Use a quality USB-C cable to connect the board to your computer
   - The board should power on (LED indicators will light up)

2. **Select board and port:**
   - Go to `Tools` > `Board` > `ESP32 Arduino`
   - Select **"ESP32S3 Dev Module"** or **"LilyGO T-Camera S3"** if available
   - Go to `Tools` > `Port` and select the appropriate COM port (Windows) or /dev/tty* (macOS/Linux)

3. **Configure board settings:**
   - **CPU Frequency:** 240MHz
   - **Flash Mode:** QIO
   - **Flash Size:** 16MB
   - **Partition Scheme:** 16M Flash (3MB APP/9.9MB FATFS)
   - **PSRAM:** Enabled

### 3. Writing the Code

1. **Create a new sketch** in Arduino IDE

2. **Basic camera initialization code:**
   ```cpp
   #include "esp_camera.h"
   #include "WiFi.h"
   #include "SD_MMC.h"
   
   // Camera pin definitions for LilyGO T-Camera Plus S3
   #define PWDN_GPIO_NUM    26
   #define RESET_GPIO_NUM   -1
   #define XCLK_GPIO_NUM    32
   #define SIOD_GPIO_NUM    13  // SDA
   #define SIOC_GPIO_NUM    12  // SCL
   
   #define Y9_GPIO_NUM      39
   #define Y8_GPIO_NUM      36
   #define Y7_GPIO_NUM      23
   #define Y6_GPIO_NUM      18
   #define Y5_GPIO_NUM      15
   #define Y4_GPIO_NUM       4
   #define Y3_GPIO_NUM      14
   #define Y2_GPIO_NUM       5
   #define VSYNC_GPIO_NUM   27
   #define HREF_GPIO_NUM    25
   #define PCLK_GPIO_NUM    19
   
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
     config.frame_size = FRAMESIZE_UXGA;
     config.jpeg_quality = 10;
     config.fb_count = 3;
     
     // Initialize camera
     esp_err_t err = esp_camera_init(&config);
     if (err != ESP_OK) {
       Serial.printf("Camera init failed with error 0x%x", err);
       return;
     }
     
     Serial.println("Camera initialized successfully");
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

> **⚠️ Warning:** Ensure PSRAM is enabled in board settings for proper operation

### 5. Uploading the Code

1. **Press and hold** the **Boot button** on the board
2. **Click the upload button** (arrow) in Arduino IDE
3. **Release the Boot button** when upload begins
4. **Monitor the output console** for upload progress
5. **Wait for "Hard resetting via RTS pin"** message

### 6. Testing the Deployment

1. **Open Serial Monitor** (`Tools` > `Serial Monitor`)
2. **Set baud rate to 115200**
3. **Press the Reset button** on the board
4. **Verify initialization messages** appear
5. **Test camera functionality** by taking a test photo

## Hardware Specifications

### Board Specifications
- **Microcontroller:** ESP32-S3-WROOM-1 (Dual-core Xtensa LX7, 240MHz)
- **Flash Memory:** 16MB SPI Flash
- **PSRAM:** 8MB QSPI PSRAM
- **WiFi:** 802.11 b/g/n (2.4GHz)
- **Bluetooth:** Bluetooth 5.0 LE
- **Operating Voltage:** 3.3V
- **Input Voltage:** 5V (via USB-C)
- **Dimensions:** 54mm × 54mm × 14mm

### Camera Specifications
- **Sensor:** OV2640 2-megapixel CMOS sensor
- **Resolution:** Up to 1600×1200 (UXGA)
- **Image Formats:** JPEG, RGB565, YUV422
- **Frame Rates:** Up to 60fps at VGA, 15fps at UXGA
- **Features:** Auto exposure, auto white balance, auto gain control
- **Field of View:** 66° diagonal

### Memory and Storage
- **RAM:** 512KB SRAM + 8MB PSRAM
- **Flash:** 16MB for program storage
- **External Storage:** MicroSD card slot (up to 128GB)
- **Partition Scheme:** Configurable (recommended: 3MB APP + 9.9MB FATFS)

### Connectivity Options
- **WiFi:** Built-in with external antenna connector
- **USB:** USB-C for programming and power
- **Serial:** UART pins available for external communication
- **I2C:** Hardware I2C bus available
- **SPI:** Hardware SPI bus available

### Power Requirements
- **Operating Current:** 90mA (active), 5mA (sleep)
- **Camera Current:** 150mA during capture
- **Deep Sleep Current:** 5µA
- **Input Voltage Range:** 3.0V - 4.2V
- **Recommended Battery:** Li-Po 3.7V 2000mAh+

### Display
- **Type:** 1.14" ST7789 Color TFT LCD
- **Resolution:** 240×135 pixels
- **Interface:** SPI
- **Colors:** 65K colors (16-bit)
- **Backlight:** Controllable brightness

## Pin Configuration

### Camera Pin Mapping
```cpp
// LilyGO T-Camera Plus S3 Camera Pins
#define PWDN_GPIO_NUM    26    // Camera power down
#define RESET_GPIO_NUM   -1    // Camera reset (not connected)
#define XCLK_GPIO_NUM    32    // Camera clock
#define SIOD_GPIO_NUM    13    // Camera I2C data (SDA)
#define SIOC_GPIO_NUM    12    // Camera I2C clock (SCL)

// Camera data pins
#define Y9_GPIO_NUM      39    // Data bit 9
#define Y8_GPIO_NUM      36    // Data bit 8
#define Y7_GPIO_NUM      23    // Data bit 7
#define Y6_GPIO_NUM      18    // Data bit 6
#define Y5_GPIO_NUM      15    // Data bit 5
#define Y4_GPIO_NUM       4    // Data bit 4
#define Y3_GPIO_NUM      14    // Data bit 3
#define Y2_GPIO_NUM       5    // Data bit 2
#define VSYNC_GPIO_NUM   27    // Vertical sync
#define HREF_GPIO_NUM    25    // Horizontal reference
#define PCLK_GPIO_NUM    19    // Pixel clock

// Built-in peripherals
#define LED_GPIO_NUM      2    // Built-in LED
#define FLASH_GPIO_NUM    4    // Camera flash LED
```

### Display Pin Mapping
```cpp
// ST7789 TFT Display Pins
#define TFT_CS_PIN       12    // Chip select
#define TFT_DC_PIN       13    // Data/Command
#define TFT_RST_PIN      14    // Reset
#define TFT_BL_PIN       15    // Backlight
#define TFT_MOSI_PIN     21    // SPI data
#define TFT_SCLK_PIN     22    // SPI clock
```

### Available GPIO for External Sensors
```cpp
// Available pins for external connections
#define AVAILABLE_GPIO_PINS {0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 20, 21}

// Recommended sensor pin assignments
#define PIR_SENSOR_PIN    0    // PIR motion sensor
#define EXTERNAL_LED_PIN  1    // External status LED
#define BUTTON_PIN        3    // User button
#define ANALOG_SENSOR_1   8    // Analog sensor input
#define ANALOG_SENSOR_2   9    // Analog sensor input
#define I2C_SDA_EXT      10    // External I2C SDA
#define I2C_SCL_EXT      11    // External I2C SCL
```

### Pin Conflicts and Limitations
```cpp
// Critical pin usage warnings
struct PinLimitations {
    // Camera pins - DO NOT USE for other purposes
    int camera_pins[] = {4, 5, 12, 13, 14, 15, 18, 19, 23, 25, 26, 27, 32, 36, 39};
    
    // Display pins - conflicts with external SPI devices
    int display_pins[] = {12, 13, 14, 15, 21, 22};
    
    // SD card pins (if used)
    int sd_pins[] = {38, 39, 40, 41, 42};  // When SD card is enabled
    
    // Boot-sensitive pins (use with caution)
    int boot_pins[] = {0, 46};  // Can affect boot process
    
    // Input-only pins
    int input_only[] = {36, 39};  // Cannot be used as outputs
};
```

## Board-Specific Configuration

### Display Configuration
The T-Camera Plus S3 includes a built-in 1.14" TFT display that requires specific configuration:

```cpp
#include <TFT_eSPI.h>

// Display initialization
TFT_eSPI tft = TFT_eSPI();

void setupDisplay() {
    tft.init();
    tft.setRotation(1);  // Landscape orientation
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Wildlife Cam");
}
```

### Camera Sensor Configuration
Optimize camera settings for wildlife photography:

```cpp
void configureCameraForWildlife() {
    sensor_t * s = esp_camera_sensor_get();
    
    // Optimize for wildlife photography
    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_special_effect(s, 0); // 0 to 6 (0=None)
    s->set_whitebal(s, 1);       // 0 = disable, 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable, 1 = enable
    s->set_wb_mode(s, 0);        // 0 to 4
    s->set_exposure_ctrl(s, 1);  // 0 = disable, 1 = enable
    s->set_aec2(s, 0);           // 0 = disable, 1 = enable
    s->set_ae_level(s, 0);       // -2 to 2
    s->set_aec_value(s, 300);    // 0 to 1200
    s->set_gain_ctrl(s, 1);      // 0 = disable, 1 = enable
    s->set_agc_gain(s, 0);       // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
    s->set_bpc(s, 0);            // 0 = disable, 1 = enable
    s->set_wpc(s, 1);            // 0 = disable, 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable, 1 = enable
    s->set_lenc(s, 1);           // 0 = disable, 1 = enable
    s->set_hmirror(s, 0);        // 0 = disable, 1 = enable
    s->set_vflip(s, 0);          // 0 = disable, 1 = enable
    s->set_dcw(s, 1);            // 0 = disable, 1 = enable
    s->set_colorbar(s, 0);       // 0 = disable, 1 = enable
}
```

### Power Management Settings
Configure optimal power management for battery operation:

```cpp
#include "esp_pm.h"
#include "esp_sleep.h"

void configurePowerManagement() {
    // Enable automatic light sleep
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 10,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);
    
    // Configure deep sleep wake sources
    esp_sleep_enable_ext0_wakeup(PIR_SENSOR_PIN, 1);  // Wake on PIR
    esp_sleep_enable_timer_wakeup(5 * 60 * 1000000);  // Wake every 5 minutes
    
    // Disable WiFi power save for faster response (optional)
    esp_wifi_set_ps(WIFI_PS_NONE);
}
```

### Bootloader Configuration
For field deployment, configure watchdog and brownout detection:

```cpp
#include "esp_system.h"

void configureBootloader() {
    // Disable brownout detector for battery operation
    // WARNING: Use only with stable power supply
    // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    
    // Configure watchdog timer
    esp_task_wdt_init(30, true);  // 30 second timeout
    esp_task_wdt_add(NULL);       // Add current task to watchdog
}
```

## Troubleshooting Section

### Common Issues and Solutions

#### Camera Initialization Failed
**Symptoms:**
- "Camera init failed with error 0x[hex]" in serial output
- Black images or no image capture

**Solutions:**
1. **Check power supply:**
   ```cpp
   float voltage = analogRead(A0) * 3.3 / 4095.0 * 2.0;
   if (voltage < 3.2) {
       Serial.println("ERROR: Voltage too low for camera");
   }
   ```

2. **Verify pin connections:**
   - Ensure no external devices conflict with camera pins
   - Check for loose connections on camera module

3. **Reset camera module:**
   ```cpp
   digitalWrite(PWDN_GPIO_NUM, HIGH);
   delay(100);
   digitalWrite(PWDN_GPIO_NUM, LOW);
   delay(100);
   ```

#### Upload Failures
**Symptoms:**
- "Failed to connect to ESP32" error
- Upload process hangs or times out

**Solutions:**
1. **Enter download mode manually:**
   - Hold Boot button, press Reset button, release Reset, release Boot
   - Try upload immediately after entering download mode

2. **Check USB cable and port:**
   - Use a data-capable USB-C cable (not charge-only)
   - Try different USB ports or computers
   - Install ESP32 USB drivers if needed

3. **Verify board settings:**
   - Correct board type: ESP32S3 Dev Module
   - Correct COM port selected
   - Upload speed: 115200 or 460800

#### Display Issues
**Symptoms:**
- Blank or corrupted display
- Display not responding

**Solutions:**
1. **Check TFT_eSPI configuration:**
   ```cpp
   // In User_Setup.h or User_Setup_Select.h
   #define ST7789_DRIVER
   #define TFT_WIDTH  240
   #define TFT_HEIGHT 240
   #define TFT_MOSI 21
   #define TFT_SCLK 22
   #define TFT_CS   12
   #define TFT_DC   13
   #define TFT_RST  14
   ```

2. **Test display with simple code:**
   ```cpp
   tft.fillScreen(TFT_RED);
   delay(1000);
   tft.fillScreen(TFT_GREEN);
   delay(1000);
   tft.fillScreen(TFT_BLUE);
   ```

#### WiFi Connection Problems
**Symptoms:**
- Cannot connect to WiFi network
- Frequent disconnections

**Solutions:**
1. **Check WiFi credentials:**
   ```cpp
   WiFi.begin("your_ssid", "your_password");
   int attempts = 0;
   while (WiFi.status() != WL_CONNECTED && attempts < 20) {
       delay(500);
       Serial.print(".");
       attempts++;
   }
   if (WiFi.status() == WL_CONNECTED) {
       Serial.println("WiFi connected");
   } else {
       Serial.println("WiFi connection failed");
   }
   ```

2. **Improve signal strength:**
   - Connect external antenna to U.FL connector
   - Position board closer to WiFi router
   - Check for 2.4GHz network (ESP32 doesn't support 5GHz)

#### Memory Issues
**Symptoms:**
- "Guru Meditation Error" crashes
- Out of memory errors during operation

**Solutions:**
1. **Enable PSRAM:**
   - Verify PSRAM is enabled in Arduino IDE board settings
   - Check PSRAM initialization:
   ```cpp
   if (psramFound()) {
       Serial.println("PSRAM found and enabled");
   } else {
       Serial.println("PSRAM not found");
   }
   ```

2. **Optimize memory usage:**
   ```cpp
   // Use smaller frame buffers
   config.fb_count = 1;  // Reduce from 3 to 1
   config.jpeg_quality = 12;  // Increase compression
   config.frame_size = FRAMESIZE_SVGA;  // Reduce resolution
   ```

### Debug Tips

#### Enable Debug Output
```cpp
// Add to setup() for detailed debugging
Serial.setDebugOutput(true);
esp_log_level_set("*", ESP_LOG_VERBOSE);
esp_log_level_set("camera", ESP_LOG_VERBOSE);
```

#### Memory Monitoring
```cpp
void printMemoryInfo() {
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    Serial.printf("Used PSRAM: %d bytes\n", ESP.getPsramSize() - ESP.getFreePsram());
}
```

#### Camera Status Check
```cpp
void checkCameraStatus() {
    sensor_t * s = esp_camera_sensor_get();
    if (s != NULL) {
        sensor_id_t id = s->id;
        Serial.printf("Camera sensor: %s\n", 
                     (id.PID == OV2640_PID) ? "OV2640" : "Unknown");
    } else {
        Serial.println("Camera sensor not detected");
    }
}
```

## Example Code Snippets

### Basic Camera Initialization with Error Handling
```cpp
#include "esp_camera.h"

bool initializeCamera() {
    camera_config_t config;
    
    // Pin configuration for LilyGO T-Camera Plus S3
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 14;
    config.pin_d2 = 4;
    config.pin_d3 = 15;
    config.pin_d4 = 18;
    config.pin_d5 = 23;
    config.pin_d6 = 36;
    config.pin_d7 = 39;
    config.pin_xclk = 32;
    config.pin_pclk = 19;
    config.pin_vsync = 27;
    config.pin_href = 25;
    config.pin_sscb_sda = 13;
    config.pin_sscb_scl = 12;
    config.pin_pwdn = 26;
    config.pin_reset = -1;
    
    // Camera settings
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 3;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    // Configure camera sensor for wildlife photography
    configureCameraForWildlife();
    
    Serial.println("Camera initialized successfully");
    return true;
}
```

### PIR Motion Sensor Integration
```cpp
#define PIR_PIN 0  // GPIO 0 for PIR sensor

volatile bool motionDetected = false;
unsigned long lastMotionTime = 0;
const unsigned long motionCooldown = 5000;  // 5 second cooldown

void IRAM_ATTR pirInterrupt() {
    if (millis() - lastMotionTime > motionCooldown) {
        motionDetected = true;
        lastMotionTime = millis();
    }
}

void setupPIRSensor() {
    pinMode(PIR_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirInterrupt, RISING);
    Serial.println("PIR sensor initialized");
}

void handleMotionDetection() {
    if (motionDetected) {
        Serial.println("Motion detected! Taking photo...");
        
        camera_fb_t * fb = esp_camera_fb_get();
        if (fb) {
            // Save photo to SD card or transmit via WiFi
            String filename = "/photo_" + String(millis()) + ".jpg";
            savePhotoToSD(fb, filename);
            esp_camera_fb_return(fb);
        }
        
        motionDetected = false;
    }
}
```

### WiFi Configuration with Captive Portal
```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

WebServer server(80);
DNSServer dnsServer;
const char* ap_ssid = "WildlifeCam_Setup";

bool connectToWiFi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(1000);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.printf("Connected to %s\n", ssid);
        Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
        return true;
    } else {
        Serial.println();
        Serial.println("WiFi connection failed");
        return false;
    }
}

void startConfigPortal() {
    WiFi.softAP(ap_ssid);
    Serial.printf("Config portal started: %s\n", ap_ssid);
    Serial.printf("Connect to configure WiFi at: %s\n", WiFi.softAPIP().toString().c_str());
    
    dnsServer.start(53, "*", WiFi.softAPIP());
    
    server.on("/", handleConfigPage);
    server.on("/save", handleConfigSave);
    server.begin();
}

void handleConfigPage() {
    String html = R"(
<!DOCTYPE html>
<html>
<head><title>Wildlife Camera Config</title></head>
<body>
<h1>WiFi Configuration</h1>
<form action="/save" method="post">
    <label>WiFi SSID:</label><br>
    <input type="text" name="ssid" required><br><br>
    <label>WiFi Password:</label><br>
    <input type="password" name="password"><br><br>
    <input type="submit" value="Save Configuration">
</form>
</body>
</html>
    )";
    server.send(200, "text/html", html);
}

void handleConfigSave() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    // Save configuration to preferences/EEPROM
    // Then restart and connect to WiFi
    
    server.send(200, "text/html", "Configuration saved! Device will restart...");
    delay(2000);
    ESP.restart();
}
```

### SD Card Photo Storage with Rotation
```cpp
#include "SD_MMC.h"
#include "FS.h"

bool initializeSDCard() {
    if (!SD_MMC.begin()) {
        Serial.println("SD Card Mount Failed");
        return false;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }
    
    Serial.printf("SD Card Type: %s\n", 
                 (cardType == CARD_MMC) ? "MMC" : 
                 (cardType == CARD_SD) ? "SDSC" : 
                 (cardType == CARD_SDHC) ? "SDHC" : "UNKNOWN");
    
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    
    return true;
}

void savePhotoToSD(camera_fb_t* fb, String filename) {
    File file = SD_MMC.open(filename, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    
    if (file.write(fb->buf, fb->len) == fb->len) {
        Serial.printf("Photo saved: %s (%d bytes)\n", filename.c_str(), fb->len);
    } else {
        Serial.println("Failed to write photo data");
    }
    
    file.close();
    
    // Check if we need to rotate old files
    rotateOldPhotos();
}

void rotateOldPhotos() {
    const int MAX_PHOTOS = 1000;  // Keep maximum 1000 photos
    
    File root = SD_MMC.open("/");
    int photoCount = 0;
    
    // Count existing photos
    File file = root.openNextFile();
    while (file) {
        if (String(file.name()).endsWith(".jpg")) {
            photoCount++;
        }
        file = root.openNextFile();
    }
    
    // Delete oldest photos if over limit
    if (photoCount > MAX_PHOTOS) {
        Serial.printf("Photo count (%d) exceeds limit, cleaning up...\n", photoCount);
        // Implementation to delete oldest files would go here
    }
}
```