# Deployment Guide for LilyGO T-Camera Plus S3

This guide provides comprehensive step-by-step instructions for deploying the LilyGO T-Camera Plus S3 for wildlife monitoring applications. This board is an excellent choice for advanced projects requiring AI processing, high-resolution imaging, and robust connectivity options.

## Prerequisites

### Hardware Requirements
- **LilyGO T-Camera Plus S3 board** (ESP32-S3 based)
- **USB-C cable** for programming and power
- **Computer** with Windows 10/11, macOS, or Linux
- **MicroSD card** (32GB or larger, Class 10 recommended)
- **External antenna** (optional, for improved WiFi range)

### Additional Components (Optional)
- **PIR motion sensor** (HC-SR501 or similar)
- **Environmental sensors** (DHT22, BMP280, etc.)
- **LoRa module** (SX1276/SX1278 for long-range communication)
- **Solar panel** (6V 3W minimum for outdoor deployment)
- **LiPo battery** (3.7V 2000mAh or larger)
- **Weatherproof enclosure** (IP65 rated)

### Software Requirements
- **Arduino IDE 2.0+** or **PlatformIO**
- **ESP32 Arduino Core** (version 2.0.11 or later)
- **Required Libraries:**
  - `ESP32-Camera` library
  - `ArduinoJson` library (version 6.x)
  - `WiFi` library (included with ESP32 core)
  - `SD_MMC` library (for SD card support)

## Step-by-Step Instructions

### 1. Setting Up Arduino IDE

#### Install Arduino IDE
1. Download Arduino IDE 2.0+ from [arduino.cc](https://www.arduino.cc/en/software)
2. Install the IDE following the platform-specific instructions

#### Configure Board Manager URLs
1. Open Arduino IDE
2. Go to `File` > `Preferences`
3. In the **Additional Boards Manager URLs** field, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click `OK` to save preferences

#### Install ESP32 Board Package
1. Go to `Tools` > `Board` > `Boards Manager`
2. Search for **"ESP32"**
3. Install **"esp32 by Espressif Systems"** (version 2.0.11 or later)
4. Wait for installation to complete

### 2. Install Required Libraries

1. Go to `Tools` > `Manage Libraries`
2. Install the following libraries:
   - Search **"ArduinoJson"** → Install version 6.x
   - Search **"AsyncTCP"** → Install by "me-no-dev"
   - Search **"ESPAsyncWebServer"** → Install by "me-no-dev"

> **Note:** The ESP32-Camera library is included with the ESP32 core package.

### 3. Board Configuration

#### Select Board and Settings
1. Go to `Tools` > `Board` > `ESP32 Arduino`
2. Select **"ESP32S3 Dev Module"**
3. Configure the following settings:
   - **CPU Frequency:** 240MHz (WiFi/BT)
   - **Flash Mode:** QIO 80MHz
   - **Flash Size:** 16MB (128Mb)
   - **Partition Scheme:** "16M Flash (3MB APP/9.9MB FATFS)"
   - **PSRAM:** "OPI PSRAM"
   - **Arduino Runs On:** Core 1
   - **Events Run On:** Core 1

### 4. Connecting the Board

#### Physical Connection
1. Connect the LilyGO T-Camera Plus S3 to your computer using a USB-C cable
2. **Important:** Ensure the cable supports both data and power transmission

#### Port Selection
1. In Arduino IDE, go to `Tools` > `Port`
2. Select the appropriate port:
   - **Windows:** `COMx` (where x is a number)
   - **macOS:** `/dev/cu.usbserial-xxxxx`
   - **Linux:** `/dev/ttyUSB0` or `/dev/ttyACM0`

> **Troubleshooting:** If the board is not detected, try a different USB cable or install CP210x drivers from Silicon Labs.

### 5. Writing and Loading the Code

#### Basic Camera Code Example
Create a new sketch and paste the following code:

```cpp
#include "esp_camera.h"
#include "WiFi.h"
#include "SD_MMC.h"
#include "FS.h"

// LilyGO T-Camera Plus S3 Pin Configuration
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    15
#define SIOD_GPIO_NUM    4
#define SIOC_GPIO_NUM    5

#define Y2_GPIO_NUM      11
#define Y3_GPIO_NUM      9
#define Y4_GPIO_NUM      8
#define Y5_GPIO_NUM      10
#define Y6_GPIO_NUM      12
#define Y7_GPIO_NUM      18
#define Y8_GPIO_NUM      17
#define Y9_GPIO_NUM      16

#define VSYNC_GPIO_NUM   6
#define HREF_GPIO_NUM    7
#define PCLK_GPIO_NUM    13

// WiFi Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

void setup() {
  Serial.begin(115200);
  Serial.println("Starting LilyGO T-Camera Plus S3...");
  
  // Initialize SD card
  if (!SD_MMC.begin()) {
    Serial.println("SD Card initialization failed!");
  } else {
    Serial.println("SD Card initialized successfully");
  }
  
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
  
  // High resolution for S3 with PSRAM
  config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
  config.jpeg_quality = 10;
  config.fb_count = 2;
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }
  
  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected successfully");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  Serial.println("Camera ready! Use capture() to take photos.");
}

void loop() {
  // Capture photo every 10 seconds for testing
  capture();
  delay(10000);
}

void capture() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  
  // Save to SD card with timestamp
  String filename = "/photo_" + String(millis()) + ".jpg";
  File file = SD_MMC.open(filename, FILE_WRITE);
  if (file) {
    file.write(fb->buf, fb->len);
    file.close();
    Serial.println("Photo saved: " + filename);
  } else {
    Serial.println("Failed to open file for writing");
  }
  
  esp_camera_fb_return(fb);
}
```

### 6. Compiling the Code

1. Click the **Verify** button (✓) or press `Ctrl+R`
2. Wait for compilation to complete
3. Check for any errors in the output console
4. If errors occur, verify:
   - All libraries are installed correctly
   - Board settings match the configuration above
   - Code syntax is correct

### 7. Uploading the Code

1. Ensure the board is connected via USB-C
2. Click the **Upload** button (→) or press `Ctrl+U`
3. Monitor the upload progress in the console
4. The board will automatically reset after successful upload

> **Note:** The LilyGO T-Camera Plus S3 has auto-reset functionality, so no manual reset is required for uploading.

### 8. Testing the Deployment

1. Open the **Serial Monitor** (`Tools` > `Serial Monitor`)
2. Set baud rate to **115200**
3. Press the reset button on the board (if needed)
4. Verify you see:
   - "Starting LilyGO T-Camera Plus S3..."
   - "SD Card initialized successfully"
   - "WiFi connected successfully"
   - "Camera ready! Use capture() to take photos."
   - "Photo saved: /photo_xxxxx.jpg" messages

## Hardware Specifications

### Board Features
- **Microcontroller:** ESP32-S3 (Xtensa dual-core 32-bit LX7, up to 240MHz)
- **Memory:** 512KB SRAM + 8MB PSRAM
- **Flash:** 16MB SPI Flash
- **Connectivity:** WiFi 802.11 b/g/n, Bluetooth 5.0 LE
- **USB:** USB-C connector with USB 1.1 OTG support
- **Operating Voltage:** 3.3V
- **Input Voltage:** 5V via USB-C or external power

### Camera Specifications
- **Sensor:** OV2640 CMOS sensor
- **Resolution:** Up to 1600x1200 (UXGA)
- **Formats:** JPEG, RGB565, YUV422
- **Frame Rate:** Up to 60fps at lower resolutions
- **Features:** Auto exposure, auto white balance, auto gain control
- **Field of View:** Approximately 66° diagonal

### Memory and Storage
- **SRAM:** 512KB internal
- **PSRAM:** 8MB external (OPI PSRAM)
- **Flash:** 16MB for firmware and file system
- **SD Card:** MicroSD slot supporting up to 32GB (FAT32)

### Connectivity Options
- **WiFi:** 2.4GHz 802.11 b/g/n with external antenna connector
- **Bluetooth:** 5.0 LE for short-range communication
- **USB:** USB-C for programming, debugging, and power
- **GPIO:** 20+ available pins for sensors and peripherals

### Power Requirements
- **Operating Voltage:** 3.3V (regulated)
- **Input Voltage:** 5V ±5% via USB-C
- **Power Consumption:**
  - Active (WiFi + Camera): ~200-300mA
  - Light Sleep: ~20mA
  - Deep Sleep: ~50µA

## Pin Configuration

### GPIO Mapping

#### Camera Interface (Fixed Pins)
```cpp
#define XCLK_GPIO_NUM    15  // Camera clock
#define SIOD_GPIO_NUM    4   // Camera SDA (I2C data)
#define SIOC_GPIO_NUM    5   // Camera SCL (I2C clock)
#define Y2_GPIO_NUM      11  // Camera data bit 2
#define Y3_GPIO_NUM      9   // Camera data bit 3
#define Y4_GPIO_NUM      8   // Camera data bit 4
#define Y5_GPIO_NUM      10  // Camera data bit 5
#define Y6_GPIO_NUM      12  // Camera data bit 6
#define Y7_GPIO_NUM      18  // Camera data bit 7
#define Y8_GPIO_NUM      17  // Camera data bit 8
#define Y9_GPIO_NUM      16  // Camera data bit 9
#define VSYNC_GPIO_NUM   6   // Vertical sync
#define HREF_GPIO_NUM    7   // Horizontal reference
#define PCLK_GPIO_NUM    13  // Pixel clock
```

#### Available GPIO Pins for External Sensors
```cpp
// Available for external sensors and peripherals
#define AVAILABLE_GPIOS {1, 2, 3, 14, 19, 20, 21, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48}

// Recommended pin assignments
#define PIR_SENSOR_PIN   1   // PIR motion sensor
#define LED_STATUS_PIN   2   // Status LED
#define DHT_SENSOR_PIN   3   // Temperature/humidity sensor
#define BUTTON_PIN       14  // User button input
```

#### Special Function Pins
```cpp
// USB and Serial
#define USB_DN_PIN       19  // USB D- (not available for GPIO)
#define USB_DP_PIN       20  // USB D+ (not available for GPIO)

// SPI Flash (Internal - Do not use)
// GPIO 26-32 are used for internal SPI flash

// SD Card Interface
#define SD_CMD_PIN       35  // SD card command
#define SD_CLK_PIN       36  // SD card clock  
#define SD_D0_PIN        37  // SD card data 0
#define SD_D1_PIN        38  // SD card data 1
#define SD_D2_PIN        39  // SD card data 2
#define SD_D3_PIN        40  // SD card data 3
```

### Pin Conflicts and Limitations

#### Camera vs External Sensors
- **Camera pins are dedicated** and cannot be used for other purposes
- **Avoid using pins 4-18** for external sensors as they're used by the camera

#### SD Card vs External SPI Devices
- **SD card uses dedicated pins** (35-40) that cannot be shared
- **External SPI devices** must use alternative pins if SD card is enabled

#### Power Considerations
- **Maximum current per pin:** 40mA
- **Total GPIO current:** 200mA maximum
- **Use level shifters** for 5V sensor compatibility

### Recommended Sensor Connections

#### PIR Motion Sensor (HC-SR501)
```cpp
// PIR Sensor Connection
// VCC -> 3.3V (or 5V if sensor supports it)
// GND -> GND
// OUT -> GPIO 1

#define PIR_SENSOR_PIN 1
void setupPIR() {
  pinMode(PIR_SENSOR_PIN, INPUT);
}

bool motionDetected() {
  return digitalRead(PIR_SENSOR_PIN) == HIGH;
}
```

#### Environmental Sensor (DHT22)
```cpp
// DHT22 Connection
// VCC -> 3.3V
// GND -> GND
// DATA -> GPIO 3 (with 10kΩ pull-up resistor)

#include "DHT.h"
#define DHT_PIN 3
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

void setupDHT() {
  dht.begin();
}

void readEnvironmental() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  if (!isnan(humidity) && !isnan(temperature)) {
    Serial.printf("Temperature: %.1f°C, Humidity: %.1f%%\n", 
                  temperature, humidity);
  }
}
```

#### LoRa Module (SX1276/SX1278)
```cpp
// LoRa Module Connection
// VCC -> 3.3V
// GND -> GND
// SCK -> GPIO 41
// MISO -> GPIO 42
// MOSI -> GPIO 43
// CS -> GPIO 44
// RST -> GPIO 45
// DIO0 -> GPIO 46

#include <SPI.h>
#include <LoRa.h>

#define LORA_CS_PIN    44
#define LORA_RST_PIN   45
#define LORA_DIO0_PIN  46

void setupLoRa() {
  SPI.begin();
  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
  
  if (!LoRa.begin(915E6)) {  // 915MHz for North America
    Serial.println("LoRa initialization failed!");
    return;
  }
  Serial.println("LoRa initialized successfully");
}
```

## Board-Specific Configuration

### PSRAM Configuration
The LilyGO T-Camera Plus S3 includes 8MB of OPI PSRAM that significantly improves performance:

```cpp
// Enable PSRAM in camera configuration
camera_config_t config;
// ... other settings ...

// With PSRAM enabled, use higher quality settings
if (psramFound()) {
  config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
  config.jpeg_quality = 8;             // Higher quality
  config.fb_count = 2;                 // Double buffering
  Serial.println("PSRAM found! Using high-quality settings.");
} else {
  config.frame_size = FRAMESIZE_SVGA;  // 800x600
  config.jpeg_quality = 12;            // Lower quality
  config.fb_count = 1;                 // Single buffer
  Serial.println("PSRAM not found. Using standard settings.");
}
```

### Power Management Settings
Configure optimal power management for outdoor deployment:

```cpp
#include "esp_pm.h"
#include "esp_sleep.h"

void setupPowerManagement() {
  // Configure automatic light sleep
  esp_pm_config_esp32s3_t pm_config = {
    .max_freq_mhz = 240,
    .min_freq_mhz = 80,
    .light_sleep_enable = true
  };
  ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
  
  // Configure deep sleep wakeup sources
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_1, 1);  // PIR sensor wake
  esp_sleep_enable_timer_wakeup(10 * 60 * 1000000ULL);  // 10-minute timer wake
}

void enterDeepSleep() {
  // Power down camera
  esp_camera_deinit();
  
  // Disable WiFi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  // Enter deep sleep
  Serial.println("Entering deep sleep...");
  esp_deep_sleep_start();
}
```

### Camera Initialization Parameters
Optimize camera settings for wildlife photography:

```cpp
void optimizeCameraForWildlife() {
  sensor_t* sensor = esp_camera_sensor_get();
  if (sensor) {
    // Auto exposure settings
    sensor->set_exposure_ctrl(sensor, 1);      // Enable auto exposure
    sensor->set_aec2(sensor, 0);               // Disable AEC2
    sensor->set_ae_level(sensor, 0);           // Normal AE level
    
    // Auto white balance
    sensor->set_whitebal(sensor, 1);           // Enable AWB
    sensor->set_awb_gain(sensor, 1);           // Enable AWB gain
    
    // Image quality settings
    sensor->set_brightness(sensor, 0);         // Normal brightness
    sensor->set_contrast(sensor, 0);           // Normal contrast
    sensor->set_saturation(sensor, 0);         // Normal saturation
    sensor->set_sharpness(sensor, 0);          // Normal sharpness
    
    // Noise reduction
    sensor->set_denoise(sensor, 1);            // Enable noise reduction
    
    // Lens correction
    sensor->set_lenc(sensor, 1);               // Enable lens correction
    
    Serial.println("Camera optimized for wildlife photography");
  }
}
```

## Troubleshooting Section

### Common Issues and Solutions

#### Camera Initialization Failed
**Symptoms:** `Camera init failed with error 0x%x` in serial monitor

**Solutions:**
1. **Check power supply:** Ensure stable 5V power via USB-C
2. **Verify connections:** Confirm USB-C cable supports data transfer
3. **Reset the board:** Press the reset button and try again
4. **Check PSRAM setting:** Ensure "OPI PSRAM" is selected in Arduino IDE
5. **Try different camera settings:**
   ```cpp
   // Use conservative settings for troubleshooting
   config.frame_size = FRAMESIZE_QVGA;  // 320x240
   config.jpeg_quality = 20;
   config.fb_count = 1;
   ```

#### Upload/Connection Problems
**Symptoms:** "Failed to connect to ESP32-S3" or upload timeouts

**Solutions:**
1. **Install drivers:** Download and install CP210x USB drivers
2. **Check cable:** Use a high-quality USB-C cable with data support
3. **Manual reset:** Hold BOOT button while connecting USB cable
4. **Port selection:** Try different USB ports on your computer
5. **Restart Arduino IDE:** Close and reopen the Arduino IDE

#### SD Card Not Detected
**Symptoms:** "SD Card initialization failed!" message

**Solutions:**
1. **Format SD card:** Use FAT32 format with allocation unit size 4096 bytes
2. **Check card compatibility:** Use Class 10 microSD card (32GB max)
3. **Verify connections:** Ensure SD card is properly inserted
4. **Test different card:** Try with a known-good SD card
5. **Check power:** SD cards require stable power supply

#### WiFi Connection Issues
**Symptoms:** "Connecting to WiFi..." repeats indefinitely

**Solutions:**
1. **Check credentials:** Verify SSID and password are correct
2. **Signal strength:** Move closer to WiFi router
3. **Network compatibility:** Ensure 2.4GHz network (5GHz not supported)
4. **Router settings:** Disable MAC filtering temporarily
5. **Antenna connection:** Check external antenna connection if used

#### Poor Image Quality
**Symptoms:** Blurry, dark, or poor quality images

**Solutions:**
1. **Lighting conditions:** Ensure adequate lighting (>2 lux minimum)
2. **Focus adjustment:** Manually adjust camera lens focus
3. **Quality settings:** Increase JPEG quality (lower number = higher quality)
4. **Stability:** Ensure camera is mounted steadily
5. **Lens cleaning:** Clean camera lens with soft cloth

#### Memory Issues
**Symptoms:** Crashes, reboots, or "out of memory" errors

**Solutions:**
1. **Enable PSRAM:** Verify PSRAM is enabled and detected
2. **Reduce frame size:** Use lower resolution settings
3. **Optimize code:** Avoid large static arrays, use dynamic allocation
4. **Buffer management:** Properly release camera frame buffers
5. **Monitor usage:** Use `esp_get_free_heap_size()` to track memory

### Debug Tips

#### Serial Monitor Debugging
```cpp
// Enable verbose debugging
void enableDebugOutput() {
  Serial.setDebugOutput(true);
  
  // Camera debug info
  esp_log_level_set("camera", ESP_LOG_VERBOSE);
  
  // WiFi debug info
  esp_log_level_set("wifi", ESP_LOG_VERBOSE);
  
  // System info
  Serial.printf("ESP32-S3 Chip: %s\n", ESP.getChipModel());
  Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("PSRAM Size: %d bytes\n", ESP.getPsramSize());
  Serial.printf("PSRAM Free: %d bytes\n", ESP.getFreePsram());
}
```

#### Hardware Testing
```cpp
// Test GPIO pins
void testGPIOPins() {
  int testPins[] = {1, 2, 3, 14};  // Available GPIO pins
  
  for (int pin : testPins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);
    Serial.printf("GPIO %d tested\n", pin);
  }
}

// Test camera sensor communication
void testCameraSensor() {
  sensor_t* sensor = esp_camera_sensor_get();
  if (sensor) {
    Serial.printf("Camera sensor ID: 0x%X\n", sensor->id.PID);
    Serial.println("Camera sensor communication OK");
  } else {
    Serial.println("Camera sensor communication failed");
  }
}
```

## Example Code Snippets

### Basic Camera Initialization
```cpp
#include "esp_camera.h"

bool initializeCamera() {
  camera_config_t config;
  
  // Pin configuration for LilyGO T-Camera Plus S3
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 11;
  config.pin_d1 = 9;
  config.pin_d2 = 8;
  config.pin_d3 = 10;
  config.pin_d4 = 12;
  config.pin_d5 = 18;
  config.pin_d6 = 17;
  config.pin_d7 = 16;
  config.pin_xclk = 15;
  config.pin_pclk = 13;
  config.pin_vsync = 6;
  config.pin_href = 7;
  config.pin_sscb_sda = 4;
  config.pin_sscb_scl = 5;
  config.pin_pwdn = -1;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Quality settings
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  return (err == ESP_OK);
}
```

### Motion-Triggered Photography
```cpp
#include "esp_camera.h"

#define PIR_PIN 1
#define LED_PIN 2

int motionCount = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize camera
  if (!initializeCamera()) {
    Serial.println("Camera initialization failed!");
    return;
  }
  
  // Setup PIR sensor
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("Motion-triggered camera ready!");
}

void loop() {
  // Check for motion
  if (digitalRead(PIR_PIN) == HIGH) {
    Serial.println("Motion detected!");
    
    // Indicate activity
    digitalWrite(LED_PIN, HIGH);
    
    // Capture sequence of photos
    captureSequence(3, 1000);  // 3 photos, 1 second apart
    
    motionCount++;
    Serial.printf("Total motion events: %d\n", motionCount);
    
    // LED off
    digitalWrite(LED_PIN, LOW);
    
    // Wait to avoid duplicate triggers
    delay(5000);
  }
  
  delay(100);  // Small delay to prevent excessive polling
}

void captureSequence(int count, int delayMs) {
  for (int i = 0; i < count; i++) {
    captureAndSave(String("/motion_") + motionCount + "_" + i + ".jpg");
    if (i < count - 1) delay(delayMs);
  }
}

void captureAndSave(String filename) {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  
  // Save to SD card
  File file = SD_MMC.open(filename, FILE_WRITE);
  if (file) {
    file.write(fb->buf, fb->len);
    file.close();
    Serial.println("Saved: " + filename);
  }
  
  esp_camera_fb_return(fb);
}
```

### WiFi Web Server for Remote Monitoring
```cpp
#include "esp_camera.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Initialize camera
  initializeCamera();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  Serial.println("WiFi connected");
  Serial.print("Camera server: http://");
  Serial.println(WiFi.localIP());
  
  // Setup web server routes
  setupWebServer();
  
  server.begin();
}

void setupWebServer() {
  // Live camera stream
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><title>Wildlife Camera</title></head>";
    html += "<body><h1>LilyGO T-Camera Plus S3</h1>";
    html += "<img src='/capture' width='800' height='600'>";
    html += "<br><button onclick='location.reload()'>Refresh</button>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });
  
  // Capture single image
  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest *request){
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      request->send(500, "text/plain", "Camera capture failed");
      return;
    }
    
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/jpeg", fb->buf, fb->len);
    response->addHeader("Content-Disposition", "inline; filename=capture.jpg");
    request->send(response);
    
    esp_camera_fb_return(fb);
  });
  
  // System status
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String status = "{";
    status += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
    status += "\"psram\":" + String(ESP.getFreePsram()) + ",";
    status += "\"uptime\":" + String(millis()) + ",";
    status += "\"wifi_rssi\":" + String(WiFi.RSSI());
    status += "}";
    request->send(200, "application/json", status);
  });
}

void loop() {
  // Server runs asynchronously
  delay(1000);
}
```

### Environmental Sensor Integration
```cpp
#include "DHT.h"
#include "ArduinoJson.h"

#define DHT_PIN 3
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

struct EnvironmentalData {
  float temperature;
  float humidity;
  float pressure;    // If using BMP280
  unsigned long timestamp;
};

void setup() {
  Serial.begin(115200);
  
  // Initialize sensors
  dht.begin();
  
  // Initialize camera
  initializeCamera();
  
  Serial.println("Environmental monitoring system ready");
}

void loop() {
  // Read environmental data
  EnvironmentalData envData = readEnvironmentalData();
  
  // Check if conditions are suitable for wildlife activity
  if (isWildlifeActive(envData)) {
    Serial.println("Favorable conditions detected, monitoring for motion...");
    
    // Enhanced motion detection during favorable conditions
    enableEnhancedMonitoring();
  }
  
  // Log environmental data
  logEnvironmentalData(envData);
  
  delay(60000);  // Read every minute
}

EnvironmentalData readEnvironmentalData() {
  EnvironmentalData data;
  
  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();
  data.pressure = 0;  // Add BMP280 reading if available
  data.timestamp = millis();
  
  return data;
}

bool isWildlifeActive(EnvironmentalData data) {
  // Wildlife activity parameters (adjust based on target species)
  const float MIN_TEMP = 5.0;   // Minimum temperature for activity
  const float MAX_TEMP = 35.0;  // Maximum temperature for activity
  const float MIN_HUMIDITY = 30.0;  // Minimum humidity
  
  return (data.temperature >= MIN_TEMP && 
          data.temperature <= MAX_TEMP &&
          data.humidity >= MIN_HUMIDITY &&
          !isnan(data.temperature) &&
          !isnan(data.humidity));
}

void logEnvironmentalData(EnvironmentalData data) {
  // Create JSON log entry
  DynamicJsonDocument doc(200);
  doc["timestamp"] = data.timestamp;
  doc["temperature"] = data.temperature;
  doc["humidity"] = data.humidity;
  doc["pressure"] = data.pressure;
  
  // Save to SD card
  File logFile = SD_MMC.open("/environmental_log.json", FILE_APPEND);
  if (logFile) {
    serializeJson(doc, logFile);
    logFile.println();
    logFile.close();
  }
  
  // Print to serial
  Serial.printf("Temp: %.1f°C, Humidity: %.1f%%\n", 
                data.temperature, data.humidity);
}

void enableEnhancedMonitoring() {
  // Increase motion sensor sensitivity during favorable conditions
  // Reduce sleep intervals
  // Enable additional sensors if available
}
```

---

## Conclusion

The LilyGO T-Camera Plus S3 is an excellent choice for advanced wildlife monitoring applications. With its ESP32-S3 processor, 8MB PSRAM, and comprehensive GPIO options, it provides the performance and flexibility needed for sophisticated monitoring systems.

Key advantages:
- **High Performance:** ESP32-S3 with 8MB PSRAM enables high-resolution imaging and complex processing
- **Rich Connectivity:** WiFi, Bluetooth, and abundant GPIO pins for sensors
- **Easy Development:** Arduino IDE compatibility with extensive library support
- **Expandability:** Multiple connection options for LoRa, environmental sensors, and other peripherals

This guide provides a solid foundation for deploying the T-Camera Plus S3 in various wildlife monitoring scenarios. Adapt the configurations and code examples to match your specific deployment requirements and target species.