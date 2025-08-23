# Deployment Guide for LilyGO T-Camera Plus S3

This guide provides comprehensive step-by-step instructions for deploying the LilyGO T-Camera Plus S3 for wildlife monitoring applications. This board features ESP32-S3, OV2640 camera, and an integrated ST7789 TFT display, making it ideal for standalone monitoring with real-time feedback.

## Prerequisites

### Hardware Requirements
- **LilyGO T-Camera Plus S3 board** (ESP32-S3 based)
- **USB-C cable** for programming and power
- **Computer** with Arduino IDE installed (Windows, macOS, or Linux)
- **MicroSD card** (8GB-32GB, Class 10 recommended) for image storage
- **Additional components** (optional):
  - PIR motion sensor for motion detection
  - External antenna for improved WiFi/Bluetooth range
  - Weatherproof enclosure for outdoor deployment
  - Solar panel and battery for remote operation

### Software Requirements
- **Arduino IDE** version 2.0 or later (or Arduino IDE 1.8.19+)
- **ESP32 board package** for Arduino IDE
- **Required libraries**:
  - ESP32 Camera library
  - TFT_eSPI for display support
  - ArduinoJson for configuration management
  - WiFi library (included with ESP32 package)

> ⚠️ **Important**: Ensure your computer has drivers for USB-C to serial communication. Most modern systems include these drivers automatically.

## Step-by-Step Instructions

### 1. Setting Up the Arduino IDE

1. **Install Arduino IDE**
   - Download the latest Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)
   - Install following the platform-specific instructions

2. **Add ESP32 Board Package**
   - Open Arduino IDE
   - Go to `File` > `Preferences`
   - Add the following URL to **Additional Boards Manager URLs**:
     ```
     https://espressif.github.io/arduino-esp32/package_esp32_index.json
     ```
   - Click **OK** to save preferences

3. **Install ESP32 Board Support**
   - Navigate to `Tools` > `Board` > `Boards Manager`
   - Search for **"ESP32"** by Espressif Systems
   - Click **Install** on the latest version (2.0.0 or higher recommended)
   - Wait for installation to complete

4. **Install Required Libraries**
   - Go to `Tools` > `Manage Libraries`
   - Install the following libraries:
     - **TFT_eSPI** by Bodmer (for display support)
     - **ArduinoJson** by Benoit Blanchon (for configuration)
     - **ESP32-CAM** libraries (if available)

### 2. Connecting the Board

1. **Physical Connection**
   - Connect the LilyGO T-Camera Plus S3 to your computer using a USB-C cable
   - Ensure the cable supports data transfer (not power-only)
   - The board should power on, indicated by the display activating

2. **Board Selection in Arduino IDE**
   - Go to `Tools` > `Board` > `ESP32 Arduino`
   - Select **"ESP32S3 Dev Module"** or **"LilyGO T-Camera-S3"** if available
   - If LilyGO T-Camera-S3 is not listed, use ESP32S3 Dev Module with these settings:
     - USB CDC On Boot: "Enabled"
     - CPU Frequency: "240MHz (WiFi/BT)"
     - Flash Size: "4MB (32Mb)"
     - Partition Scheme: "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"
     - PSRAM: "QSPI PSRAM"

3. **Port Selection**
   - Go to `Tools` > `Port`
   - Select the COM port (Windows) or /dev/tty.usbmodem (macOS/Linux) that appears when board is connected
   - If no port appears, check drivers and cable connection

### 3. Writing the Code

1. **Create New Sketch**
   - Click `File` > `New Sketch`
   - Save the sketch with a descriptive name (e.g., "WildlifeCAM_LilyGO_S3")

2. **Basic Camera Test Code**
   - Copy and paste the basic camera initialization code (see Example Code Snippets section)
   - Modify configuration parameters as needed for your deployment

3. **Configure Display and Storage**
   - Add display initialization code for real-time feedback
   - Configure SD card support for image storage
   - Set up network connectivity for remote monitoring

### 4. Compiling the Code

1. **Initial Compilation**
   - Click the checkmark button (✓) in the Arduino IDE to compile
   - Check the output console for any errors or warnings

2. **Common Compilation Issues**
   - **Library not found**: Install missing libraries via Library Manager
   - **Board not selected**: Verify correct board and partition scheme selection
   - **Memory issues**: Adjust partition scheme or reduce code size

3. **Compilation Success**
   - Successful compilation shows memory usage statistics
   - Note the flash memory and RAM usage percentages

### 5. Uploading the Code

1. **Prepare for Upload**
   - Ensure the board is connected and recognized
   - Hold the **BOOT** button on the board while clicking upload (if required)
   - Some T-Camera Plus S3 boards have automatic download mode

2. **Upload Process**
   - Click the right arrow button (→) to upload
   - Monitor the progress in the output console
   - Upload typically takes 10-30 seconds depending on code size

3. **Upload Success Indicators**
   - Console shows "Hard resetting via RTS pin..."
   - Board automatically resets and starts running the new code
   - Display should show initialization messages

### 6. Testing the Deployment

1. **Initial Testing**
   - Open `Tools` > `Serial Monitor` and set baud rate to 115200
   - Reset the board if necessary using the **RST** button
   - Monitor serial output for initialization messages

2. **Camera Function Test**
   - Verify camera initialization success messages
   - Test image capture functionality
   - Check display for live preview or status information

3. **Storage and Network Testing**
   - Verify SD card detection and write capability
   - Test WiFi connection if network features are enabled
   - Validate image storage and file naming

4. **Sensor Integration Testing**
   - Test motion detection if PIR sensor is connected
   - Verify external sensor readings
   - Check power consumption and battery life estimates

> ✅ **Success Indicators**: Camera initializes, images save to SD card, display shows status, serial monitor shows normal operation logs.

## Hardware Specifications

### Board Overview
- **Microcontroller**: ESP32-S3 (Xtensa dual-core 32-bit LX7, up to 240MHz)
- **Camera**: OV2640 2MP CMOS sensor with auto-focus lens
- **Display**: 1.14" ST7789 TFT LCD (240x240 pixels, 65K colors)
- **Memory**: 512KB SRAM + 8MB PSRAM + 4MB Flash
- **Connectivity**: WiFi 802.11 b/g/n, Bluetooth 5.0 LE
- **Dimensions**: 52mm x 25mm x 12mm
- **Weight**: Approximately 15g

### Camera Specifications
- **Sensor**: OV2640 CMOS image sensor
- **Resolution**: Up to 1600x1200 (UXGA), optimal at 800x600 (SVGA)
- **Formats**: JPEG, RGB565, YUV422
- **Features**: Auto exposure, auto white balance, auto gain control
- **Frame Rate**: Up to 60fps at lower resolutions, 15fps at UXGA
- **Field of View**: ~62° diagonal viewing angle
- **Focus**: Fixed focus optimized for 1m to infinity

### Memory and Storage
- **SRAM**: 512KB internal SRAM
- **PSRAM**: 8MB external PSRAM (quad SPI)
- **Flash**: 4MB SPI Flash memory
- **SD Card**: MicroSD slot supporting up to 32GB FAT32
- **Image Storage**: ~2000-4000 images per GB depending on compression

### Connectivity Options
- **WiFi**: 802.11 b/g/n (2.4GHz), WPA/WPA2 security
- **Bluetooth**: 5.0 LE with mesh support
- **USB**: USB-C connector for programming and power
- **GPIO**: 15+ available GPIO pins for sensors and peripherals
- **I2C**: Hardware I2C support for sensor integration

### Power Requirements
- **Operating Voltage**: 3.3V-5V (USB-C or battery)
- **Active Current**: ~90mA (camera and WiFi active)
- **Sleep Current**: ~5μA (deep sleep mode)
- **Battery Support**: Built-in Li-Po battery connector and charging
- **Power Management**: Automatic power switching, deep sleep capable

## Pin Configuration

### GPIO Mapping
```cpp
// LilyGO T-Camera Plus S3 GPIO Definitions
#define PWDN_GPIO_NUM     26    // Camera power down
#define RESET_GPIO_NUM    -1    // Camera reset (not connected)
#define XCLK_GPIO_NUM     32    // Camera clock
#define SIOD_GPIO_NUM     13    // Camera SDA (I2C data)
#define SIOC_GPIO_NUM     12    // Camera SCL (I2C clock)

// Camera data pins (parallel interface)
#define Y9_GPIO_NUM       39    // Camera data bit 9
#define Y8_GPIO_NUM       36    // Camera data bit 8  
#define Y7_GPIO_NUM       23    // Camera data bit 7
#define Y6_GPIO_NUM       18    // Camera data bit 6
#define Y5_GPIO_NUM       15    // Camera data bit 5
#define Y4_GPIO_NUM        4    // Camera data bit 4
#define Y3_GPIO_NUM       14    // Camera data bit 3
#define Y2_GPIO_NUM        5    // Camera data bit 2
#define VSYNC_GPIO_NUM    27    // Camera vertical sync
#define HREF_GPIO_NUM     25    // Camera horizontal reference
#define PCLK_GPIO_NUM     19    // Camera pixel clock

// Built-in peripherals
#define LED_GPIO_NUM       2    // Built-in LED/Flash
#define FLASH_GPIO_NUM     4    // Camera flash LED

// Display pins (ST7789 TFT)
#define TFT_CS_PIN        12    // Display chip select (shared with camera I2C)
#define TFT_DC_PIN        13    // Display data/command (shared with camera I2C)  
#define TFT_RST_PIN       14    // Display reset
#define TFT_BL_PIN        15    // Display backlight
#define TFT_MOSI_PIN      11    // Display SPI data
#define TFT_SCLK_PIN      10    // Display SPI clock
```

### Available GPIO for External Sensors
```cpp
// Available pins for external sensors and peripherals
// Note: Some pins are shared with display and may require careful configuration

// Safe GPIO pins for sensors
#define AVAILABLE_GPIOS {16, 17, 18, 21, 22, 33, 34, 35}

// GPIO pin assignments for common sensors
#define PIR_SENSOR_PIN     16    // PIR motion sensor
#define DHT_SENSOR_PIN     17    // Temperature/humidity sensor
#define BUZZER_PIN         21    // Audio alert buzzer
#define STATUS_LED_PIN     22    // External status LED

// I2C pins for additional sensors
#define EXT_SDA_PIN        33    // External I2C data
#define EXT_SCL_PIN        34    // External I2C clock
```

### Pin Conflicts and Limitations

> ⚠️ **Critical Pin Conflicts**: Several pins are shared between camera, display, and external peripherals. Careful configuration is required.

> 🚨 **Display vs. I2C Conflict**: Pins 12 and 13 are shared between display control and camera I2C. External I2C sensors must use alternative pins (33, 34).

```cpp
// Pin conflict analysis for LilyGO T-Camera Plus S3
struct PinConflicts {
    // Display vs External I2C conflicts
    bool display_enabled = true;        // Display uses pins 12, 13, 14, 15
    bool external_i2c_enabled = false; // Cannot use standard I2C pins
    
    // Alternative I2C configuration
    int alt_sda_pin = 33;              // Alternative I2C SDA
    int alt_scl_pin = 34;              // Alternative I2C SCL
    
    // Camera flash vs external LED
    bool camera_flash_enabled = true;  // Built-in flash on GPIO 4
    int external_flash_pin = 21;       // Alternative flash control
    
    // SD card vs SPI conflicts
    bool sdcard_enabled = true;        // SD card uses SPI bus
    bool external_spi_enabled = false; // Cannot use SPI for other devices
};
```

### Recommended Pin Assignments

```cpp
// Recommended pin configuration for wildlife monitoring
struct RecommendedPinout {
    // Motion detection
    int pir_sensor_pin = 16;           // PIR motion sensor
    int microwave_sensor_pin = 17;     // Alternative motion sensor
    
    // Environmental monitoring
    int dht22_pin = 21;                // Temperature/humidity
    int light_sensor_pin = 35;         // Analog light level (ADC)
    
    // Status and alerts
    int status_led_pin = 22;           // External status LED
    int buzzer_pin = 33;               // Audio alert
    
    // Power management
    int battery_voltage_pin = 36;      // Battery level monitoring (ADC)
    int solar_voltage_pin = 39;        // Solar panel voltage (ADC)
    
    // Communication
    // WiFi and Bluetooth use internal pins
    // LoRa module would require external SPI (conflicts with SD card)
};
```

## Board-Specific Configuration

### Camera Initialization Parameters
```cpp
// Optimized camera configuration for LilyGO T-Camera Plus S3
camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    
    .xclk_freq_hz = 20000000,          // 20MHz camera clock
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,     // JPEG format for storage efficiency
    .frame_size = FRAMESIZE_UXGA,       // Maximum resolution (1600x1200)
    .jpeg_quality = 10,                 // High quality (lower number = better quality)
    .fb_count = 3,                      // Frame buffer count (use PSRAM)
    .fb_location = CAMERA_FB_IN_PSRAM,  // Store frame buffers in PSRAM
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY // Optimize for wildlife monitoring
};
```

### Display Configuration
```cpp
// ST7789 TFT display configuration for T-Camera Plus S3
#include <TFT_eSPI.h>

// TFT_eSPI configuration (add to User_Setup.h)
#define ST7789_DRIVER      // Define display driver
#define TFT_WIDTH  240     // Display width
#define TFT_HEIGHT 240     // Display height
#define TFT_MOSI   11      // SPI data pin
#define TFT_SCLK   10      // SPI clock pin
#define TFT_CS     12      // Chip select
#define TFT_DC     13      // Data/Command
#define TFT_RST    14      // Reset
#define TFT_BL     15      // Backlight

// Initialize display
TFT_eSPI tft = TFT_eSPI();

void initDisplay() {
    tft.init();
    tft.setRotation(2);                // Adjust rotation as needed
    tft.fillScreen(TFT_BLACK);
    
    // Enable backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    // Display startup message
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.drawString("Wildlife CAM", 20, 100);
    tft.drawString("Initializing...", 20, 130);
}
```

### Power Management Settings
```cpp
// Power optimization configuration for extended battery life
#include "esp_pm.h"
#include "esp_sleep.h"

void configurePowerManagement() {
    // Enable automatic light sleep
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = 240,          // Maximum CPU frequency
        .min_freq_mhz = 80,           // Minimum CPU frequency  
        .light_sleep_enable = true    // Enable automatic light sleep
    };
    esp_pm_configure(&pm_config);
    
    // Configure deep sleep wakeup sources
    esp_sleep_enable_timer_wakeup(5 * 60 * 1000000); // 5 minutes
    esp_sleep_enable_ext0_wakeup(PIR_SENSOR_PIN, 1); // PIR motion detection
    
    // Configure wake stub for faster wake up
    esp_sleep_enable_wake_stub();
}

void enterDeepSleep() {
    // Turn off display backlight
    digitalWrite(TFT_BL, LOW);
    
    // Power down camera
    digitalWrite(PWDN_GPIO_NUM, HIGH);
    
    // Enter deep sleep
    esp_deep_sleep_start();
}
```

### SD Card and Storage Configuration
```cpp
// SD card configuration for image storage
#include "SD.h"
#include "FS.h"

bool initSDCard() {
    if (!SD.begin()) {
        Serial.println("SD Card initialization failed!");
        return false;
    }
    
    // Check SD card type and capacity
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }
    
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    
    // Create directory structure
    if (!SD.exists("/wildlife")) {
        SD.mkdir("/wildlife");
    }
    if (!SD.exists("/wildlife/images")) {
        SD.mkdir("/wildlife/images");
    }
    if (!SD.exists("/wildlife/config")) {
        SD.mkdir("/wildlife/config");
    }
    
    return true;
}

String generateImageFilename() {
    // Generate unique filename with timestamp
    return "/wildlife/images/IMG_" + String(millis()) + ".jpg";
}
```

## Troubleshooting Section

### Common Issues and Solutions

#### 1. Camera Initialization Failures

**Symptom**: Camera fails to initialize, error messages in serial monitor
```
Camera init failed with error 0x20001
```

**Solutions**:
```cpp
// Check camera power and connections
void troubleshootCamera() {
    // Verify power down pin state
    digitalWrite(PWDN_GPIO_NUM, LOW);  // Ensure camera is powered on
    delay(100);
    
    // Test I2C communication
    Wire.begin(SIOD_GPIO_NUM, SIOC_GPIO_NUM);
    Wire.beginTransmission(0x30);  // OV2640 I2C address
    if (Wire.endTransmission() == 0) {
        Serial.println("Camera I2C communication OK");
    } else {
        Serial.println("Camera I2C communication failed");
        // Check pin connections and power supply
    }
}
```

**Common causes**:
- Insufficient power supply (use quality USB-C cable)
- Pin conflicts with display pins
- PSRAM not properly enabled
- Incorrect camera configuration

#### 2. Display Not Working

**Symptom**: Display remains black or shows corrupted data

**Solutions**:
- Verify TFT_eSPI library configuration in User_Setup.h
- Check backlight pin (GPIO 15) is set HIGH
- Ensure no pin conflicts with camera pins

```cpp
// Display diagnostic code
void testDisplay() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);  // Enable backlight
    
    tft.init();
    tft.fillScreen(TFT_RED);     // Should show red screen
    delay(1000);
    tft.fillScreen(TFT_GREEN);   // Should show green screen  
    delay(1000);
    tft.fillScreen(TFT_BLUE);    // Should show blue screen
}
```

#### 3. Upload and Connection Problems

**Symptom**: Board not detected or upload fails

**Solutions**:
1. **Check USB-C cable**: Ensure cable supports data transfer
2. **Install drivers**: ESP32-S3 usually uses built-in drivers
3. **Boot mode**: Hold BOOT button while connecting if needed
4. **Port selection**: Verify correct COM port in Arduino IDE

```bash
# Check device detection (Windows)
# Device Manager > Ports (COM & LPT)
# Should show "USB Serial Device" or similar

# Check device detection (macOS/Linux)
ls /dev/tty.usbmodem* # macOS
ls /dev/ttyACM* # Linux
```

#### 4. Memory and Storage Issues

**Symptom**: Out of memory errors, SD card problems

**Solutions**:
- Reduce JPEG quality or image size
- Enable PSRAM in board configuration
- Use appropriate partition scheme
- Check SD card format (FAT32) and capacity

```cpp
// Memory diagnostic
void checkMemory() {
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("PSRAM size: %d bytes\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    
    // Check SD card
    if (SD.cardType() != CARD_NONE) {
        Serial.printf("SD card size: %lluMB\n", SD.cardSize() / (1024 * 1024));
        Serial.printf("SD card used: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    }
}
```

#### 5. Power and Battery Issues

**Symptom**: Board resets, insufficient power, poor battery life

**Solutions**:
- Use quality power supply (5V/1A minimum)
- Implement proper power management
- Monitor battery voltage
- Optimize sleep modes

```cpp
// Power monitoring
void monitorPower() {
    // Read battery voltage (if connected)
    int battery_reading = analogRead(36);  // Assuming voltage divider
    float battery_voltage = (battery_reading * 3.3) / 4095.0 * 2.0;
    
    Serial.printf("Battery voltage: %.2fV\n", battery_voltage);
    
    if (battery_voltage < 3.3) {
        Serial.println("WARNING: Low battery voltage");
        // Implement low power mode or shutdown
    }
}
```

#### 6. Network Connectivity Issues

**Symptom**: WiFi connection fails, poor range

**Solutions**:
- Check WiFi credentials and network availability
- Verify antenna connection
- Implement connection retry logic
- Monitor signal strength

```cpp
// WiFi diagnostic
void testWiFi() {
    WiFi.begin("your_ssid", "your_password");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nWiFi connected: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());
    } else {
        Serial.println("\nWiFi connection failed");
    }
}
```

### Debug Tips

1. **Serial Monitor**: Always use 115200 baud rate for ESP32-S3
2. **Verbose Output**: Enable debug output in board configuration
3. **Step-by-Step Testing**: Test each component individually
4. **Hardware Check**: Use multimeter to verify power and connections
5. **Library Versions**: Ensure compatible library versions

## Example Code Snippets

### Basic Camera Initialization
```cpp
#include "esp_camera.h"
#include "SD.h"
#include "FS.h"

// Pin definitions (see Pin Configuration section above)
// ... pin definitions ...

void setup() {
    Serial.begin(115200);
    
    // Initialize camera
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
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    
    Serial.println("Camera initialized successfully");
}

void loop() {
    // Capture image
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }
    
    Serial.printf("Picture taken! Size: %d bytes\n", fb->len);
    
    // Return frame buffer
    esp_camera_fb_return(fb);
    
    delay(5000);  // Wait 5 seconds between captures
}
```

### PIR Motion Sensor Integration
```cpp
#include "esp_camera.h"
#include "esp_sleep.h"

#define PIR_PIN 16

volatile bool motionDetected = false;

void IRAM_ATTR motionISR() {
    motionDetected = true;
}

void setup() {
    Serial.begin(115200);
    
    // Initialize camera (see basic example above)
    // ... camera initialization code ...
    
    // Setup PIR sensor
    pinMode(PIR_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionISR, RISING);
    
    // Configure deep sleep wakeup
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_16, 1);
    
    Serial.println("Wildlife camera ready - waiting for motion");
}

void loop() {
    if (motionDetected) {
        Serial.println("Motion detected! Capturing image...");
        
        // Capture and save image
        captureAndSaveImage();
        
        // Reset motion flag
        motionDetected = false;
        
        // Wait a bit to avoid multiple triggers
        delay(2000);
    }
    
    // Enter deep sleep after 30 seconds of no activity
    if (millis() > 30000) {
        Serial.println("Entering deep sleep...");
        esp_deep_sleep_start();
    }
    
    delay(100);
}

void captureAndSaveImage() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }
    
    // Generate filename with timestamp
    String filename = "/wildlife/IMG_" + String(millis()) + ".jpg";
    
    // Save to SD card
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
        file.write(fb->buf, fb->len);
        file.close();
        Serial.printf("Image saved: %s (%d bytes)\n", filename.c_str(), fb->len);
    } else {
        Serial.println("Failed to save image to SD card");
    }
    
    esp_camera_fb_return(fb);
}
```

### Display Status and Preview
```cpp
#include <TFT_eSPI.h>
#include "esp_camera.h"

TFT_eSPI tft = TFT_eSPI();

void setup() {
    Serial.begin(115200);
    
    // Initialize display
    tft.init();
    tft.setRotation(2);
    tft.fillScreen(TFT_BLACK);
    
    // Enable backlight
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);
    
    // Initialize camera (see basic example)
    // ... camera initialization code ...
    
    displayStatus("Camera Ready");
}

void loop() {
    // Capture image
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb) {
        // Display image info
        displayImageInfo(fb->len);
        
        // Could display thumbnail here (requires JPEG decoding)
        
        esp_camera_fb_return(fb);
    }
    
    delay(5000);
}

void displayStatus(String message) {
    tft.fillRect(0, 0, 240, 30, TFT_BLUE);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.drawString(message, 10, 10);
}

void displayImageInfo(size_t imageSize) {
    tft.fillRect(0, 50, 240, 100, TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(1);
    
    tft.drawString("Last Capture:", 10, 60);
    tft.drawString(String(imageSize) + " bytes", 10, 80);
    tft.drawString(String(millis()/1000) + "s uptime", 10, 100);
}
```

### Network Configuration and Remote Monitoring
```cpp
#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

WebServer server(80);

void setup() {
    Serial.begin(115200);
    
    // Initialize camera (see basic example)
    // ... camera initialization code ...
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    
    Serial.println("WiFi connected");
    Serial.print("Camera stream ready at: http://");
    Serial.println(WiFi.localIP());
    
    // Setup web server routes
    server.on("/", handleRoot);
    server.on("/capture", handleCapture);
    server.on("/stream", handleStream);
    
    server.begin();
}

void loop() {
    server.handleClient();
}

void handleRoot() {
    String html = "<html><body>";
    html += "<h1>Wildlife Camera</h1>";
    html += "<p><a href='/capture'>Capture Image</a></p>";
    html += "<p><a href='/stream'>Live Stream</a></p>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
}

void handleCapture() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        server.send(500, "text/plain", "Camera capture failed");
        return;
    }
    
    server.sendHeader("Content-Type", "image/jpeg");
    server.sendHeader("Content-Length", String(fb->len));
    server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);
    
    esp_camera_fb_return(fb);
}

void handleStream() {
    // Implement MJPEG streaming for live preview
    // This is a simplified version - full implementation would be more complex
    server.send(200, "text/plain", "Streaming not implemented in this example");
}
```

> 💡 **Pro Tips**: 
> - Use PSRAM for better performance with high-resolution images
> - Implement proper error handling and recovery mechanisms
> - Consider power consumption in your deployment scenarios
> - Test thoroughly in your target environment before field deployment

---

## Related Deployment Guides

For additional ESP32 camera board options, see:
- **[AI-Thinker ESP32-CAM](ai-thinker-esp32-cam.md)** - Budget-friendly option with extensive documentation
- **[Complete Board Compatibility Guide](../../board_compatibility.md)** - Comprehensive board comparison and selection guide
- **[Main Deployment Guide](../README.md)** - Overview of all supported boards and deployment scenarios

> 💡 **Board Selection Tip**: The LilyGO T-Camera Plus S3 is ideal when you need:
> - Built-in display for status monitoring
> - ESP32-S3 performance for advanced processing
> - USB-C convenience for programming
> - Integrated battery management
> 
> Consider the [AI-Thinker ESP32-CAM](ai-thinker-esp32-cam.md) for budget deployments or when external display is not needed.

---

*This comprehensive guide ensures successful deployment of the LilyGO T-Camera Plus S3 for wildlife monitoring applications. The board's ESP32-S3 processor, integrated display, and robust feature set make it an excellent choice for advanced monitoring scenarios requiring real-time feedback and user interaction.*