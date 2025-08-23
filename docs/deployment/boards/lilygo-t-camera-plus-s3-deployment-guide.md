# Deployment Guide for LilyGO T-Camera Plus S3

This guide provides comprehensive step-by-step instructions for deploying the LilyGO T-Camera Plus S3. It can also serve as a template for deploying other similar ESP32 camera boards.

> ⚠️ **Important**: This board features the ESP32-S3 chip with advanced capabilities including enhanced AI processing and improved camera performance.

## Prerequisites

### Hardware Requirements
- **LilyGO T-Camera Plus S3 board**
- **USB-C cable** (for programming and power)
- **Computer** with Arduino IDE installed
- **MicroSD card** (optional, Class 10 or higher recommended)
- **External antenna** (optional, for improved WiFi/LoRa range)

### Software Requirements
- **Arduino IDE** (version 2.0+ recommended)
- **ESP32 board package** for Arduino IDE
- **Required Libraries:**
  - ESP32 Camera Library
  - AsyncWebServer (for web interface)
  - ArduinoJson (for configuration)
  - WiFi library (built-in)

### Additional Components (Optional)
- **PIR motion sensor** (for motion detection)
- **Environmental sensors** (DHT22, BME280)
- **LoRa module** (SX1276/SX1278 for long-range communication)
- **Solar panel and battery** (for remote deployment)

## Step-by-Step Instructions

### 1. Setting Up the Arduino IDE

1. **Download and Install Arduino IDE**
   - Download from [arduino.cc](https://www.arduino.cc/en/software)
   - Install version 2.0 or newer for best ESP32-S3 support

2. **Add ESP32 Board Support**
   - Open Arduino IDE
   - Go to `File` > `Preferences`
   - Add this URL to **Additional Boards Manager URLs**:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Click **OK** to save preferences

3. **Install ESP32 Board Package**
   - Navigate to `Tools` > `Board` > `Boards Manager`
   - Search for **"esp32"**
   - Install **"ESP32 by Espressif Systems"** (version 2.0.0 or newer)
   - Wait for installation to complete

### 2. Connecting the Board

1. **Physical Connection**
   - Connect the LilyGO T-Camera Plus S3 to your computer using USB-C cable
   - The board should power up automatically (LED indicators will light up)
   - No additional programming mode setup required (USB-to-serial built-in)

2. **Board Selection in Arduino IDE**
   - Go to `Tools` > `Board` > `ESP32 Arduino`
   - Select **"ESP32S3 Dev Module"** or **"LilyGO T-Camera S3"** if available
   - Set **Port** to the detected COM/USB port
   - Configure board settings:
     - **Upload Speed**: 921600
     - **Flash Mode**: QIO
     - **Flash Size**: 16MB
     - **Partition Scheme**: Huge APP (3MB No OTA/1MB SPIFFS)
     - **PSRAM**: OPI PSRAM

### 3. Installing Required Libraries

1. **Open Library Manager**
   - Go to `Tools` > `Manage Libraries`

2. **Install Core Libraries**
   - Search and install:
     - **ESP32 Camera** (by Espressif)
     - **ESP Async WebServer** (by lacamera)
     - **ArduinoJson** (by Benoit Blanchon)
     - **WiFi** (built-in, no installation needed)

3. **Verify Installation**
   - Check that all libraries appear in `Sketch` > `Include Library`

### 4. Writing the Code

1. **Create New Sketch**
   - Open new sketch: `File` > `New`
   - Save with descriptive name: `LilyGO_T_Camera_S3_Wildlife`

2. **Basic Code Structure**
   ```cpp
   #include "esp_camera.h"
   #include <WiFi.h>
   #include <AsyncWebServer.h>
   #include <ArduinoJson.h>
   
   // Camera pin definitions (T-Camera Plus S3 specific)
   #define PWDN_GPIO_NUM     -1
   #define RESET_GPIO_NUM    -1
   #define XCLK_GPIO_NUM     40
   #define SIOD_GPIO_NUM     17
   #define SIOC_GPIO_NUM     18
   
   // Data pins
   #define Y9_GPIO_NUM       39
   #define Y8_GPIO_NUM       41
   #define Y7_GPIO_NUM       42
   #define Y6_GPIO_NUM       12
   #define Y5_GPIO_NUM       3
   #define Y4_GPIO_NUM       14
   #define Y3_GPIO_NUM       47
   #define Y2_GPIO_NUM       13
   #define VSYNC_GPIO_NUM    21
   #define HREF_GPIO_NUM     38
   #define PCLK_GPIO_NUM     11
   
   void setup() {
     Serial.begin(115200);
     // Initialize camera and WiFi
   }
   
   void loop() {
     // Main program logic
   }
   ```

### 5. Compiling the Code

1. **Verify Syntax**
   - Click the checkmark button (✓) to compile
   - Arduino IDE will check for syntax errors

2. **Review Output**
   - Check the black console area for compilation results
   - Green text = success, red text = errors
   - Fix any errors before proceeding

3. **Memory Usage Check**
   - Note flash and RAM usage in console output
   - Ensure usage is below 80% for stable operation

### 6. Uploading the Code

1. **Prepare for Upload**
   - Ensure board is connected via USB-C
   - Verify correct port selection in `Tools` > `Port`
   - No boot button pressing required (auto-reset circuit)

2. **Upload Process**
   - Click the right arrow button (→) to upload
   - Monitor progress in console window
   - Upload typically takes 30-60 seconds

3. **Upload Verification**
   - Look for "Hard resetting via RTS pin..." message
   - Console should show "Done uploading"

### 7. Testing the Deployment

1. **Open Serial Monitor**
   - Go to `Tools` > `Serial Monitor`
   - Set baud rate to **115200**
   - Press **Reset** button on board

2. **Monitor Boot Sequence**
   - Check for successful camera initialization
   - Verify WiFi connection (if configured)
   - Note any error messages

3. **Verify Functionality**
   ```
   Expected Serial Output:
   ✓ Camera init: OK
   ✓ WiFi connected: [IP Address]
   ✓ Web server started
   ✓ System ready
   ```

## Hardware Specifications

### Microcontroller Features
- **Chip**: ESP32-S3 dual-core Xtensa LX7 @ 240MHz
- **Memory**: 512KB SRAM + 8MB PSRAM (OPI)
- **Flash**: 16MB SPI Flash
- **WiFi**: 802.11 b/g/n (2.4 GHz)
- **Bluetooth**: Bluetooth 5.0 LE
- **AI Acceleration**: Vector instructions for AI/ML workloads

### Camera Specifications
- **Sensor**: OV2640 CMOS image sensor
- **Resolution**: Up to 1600×1200 (UXGA)
- **Formats**: JPEG, RGB565, YUV422, Grayscale
- **Frame Rate**: Up to 60fps (VGA), 30fps (SVGA), 15fps (UXGA)
- **Features**: Auto exposure, auto white balance, auto gain control
- **Field of View**: Approximately 66° diagonal

### Memory and Storage
- **RAM**: 512KB SRAM + 8MB PSRAM for large frame buffers
- **Flash**: 16MB for firmware and file storage
- **MicroSD**: Slot for additional storage (up to 32GB)
- **Partitioning**: Flexible partition schemes supported

### Connectivity Options
- **USB**: USB-C connector for programming and power
- **WiFi**: Built-in antenna + U.FL connector for external antenna
- **GPIO**: 14 available GPIO pins
- **I2C**: Hardware I2C for sensor communication
- **SPI**: Hardware SPI for SD card and additional peripherals

### Power Requirements
- **Operating Voltage**: 3.3V (regulated from USB 5V)
- **Current Consumption**: 
  - Active (camera + WiFi): ~200mA
  - Light sleep: ~10mA
  - Deep sleep: ~150µA
- **Power Input**: USB-C (5V) or battery (3.7V Li-ion)

## Pin Configuration

### Camera Interface Pins
```cpp
// LilyGO T-Camera Plus S3 Camera Pin Mapping
#define PWDN_GPIO_NUM     -1   // Power down (not used)
#define RESET_GPIO_NUM    -1   // Reset (not used)
#define XCLK_GPIO_NUM     40   // External clock
#define SIOD_GPIO_NUM     17   // I2C data (SDA)
#define SIOC_GPIO_NUM     18   // I2C clock (SCL)

// 8-bit parallel camera data
#define Y9_GPIO_NUM       39   // Data bit 9 (MSB)
#define Y8_GPIO_NUM       41   // Data bit 8
#define Y7_GPIO_NUM       42   // Data bit 7
#define Y6_GPIO_NUM       12   // Data bit 6
#define Y5_GPIO_NUM        3   // Data bit 5
#define Y4_GPIO_NUM       14   // Data bit 4
#define Y3_GPIO_NUM       47   // Data bit 3
#define Y2_GPIO_NUM       13   // Data bit 2 (LSB)

// Camera control signals
#define VSYNC_GPIO_NUM    21   // Vertical sync
#define HREF_GPIO_NUM     38   // Horizontal reference
#define PCLK_GPIO_NUM     11   // Pixel clock
```

### Available GPIO for External Sensors
```cpp
// Available pins for external sensors and peripherals
#define AVAILABLE_GPIOS {1, 2, 4, 5, 6, 7, 8, 9, 10, 15, 16, 19, 20, 35, 36, 37, 45, 46, 48}

// Recommended sensor assignments
#define PIR_SENSOR_PIN     1   // PIR motion sensor
#define DHT_SENSOR_PIN     2   // Temperature/humidity sensor
#define LORA_CS_PIN        5   // LoRa chip select
#define LORA_RST_PIN       6   // LoRa reset
#define LORA_DIO0_PIN      7   // LoRa interrupt
#define LED_STATUS_PIN    48   // Status LED
```

### Pin Conflicts and Limitations
```cpp
// Critical pin reservations
struct PinReservations {
    // USB/Serial communication (do not use)
    int usb_dm_pin = 19;        // USB D- (reserved)
    int usb_dp_pin = 20;        // USB D+ (reserved)
    
    // Flash memory interface (do not use)
    int flash_pins[] = {26, 27, 28, 29, 30, 31, 32};
    
    // PSRAM interface (do not use)  
    int psram_pins[] = {33, 34, 35, 36, 37};
    
    // Strapping pins (use with caution)
    int strap_pin_0 = 0;        // Download mode
    int strap_pin_3 = 3;        // JTAG enable
    int strap_pin_45 = 45;      // VDD_SPI voltage
    int strap_pin_46 = 46;      // ROM message printing
};
```

### Recommended Pin Assignments for Common Sensors
```cpp
// PIR Motion Sensor
#define PIR_VCC_PIN        -1   // Connect to 3.3V
#define PIR_GND_PIN        -1   // Connect to GND  
#define PIR_OUT_PIN         1   // Digital output to GPIO 1

// DHT22 Temperature/Humidity Sensor
#define DHT_VCC_PIN        -1   // Connect to 3.3V
#define DHT_GND_PIN        -1   // Connect to GND
#define DHT_DATA_PIN        2   // Data line to GPIO 2

// LoRa SX1276/1278 Module
#define LORA_VCC_PIN       -1   // Connect to 3.3V
#define LORA_GND_PIN       -1   // Connect to GND
#define LORA_SCK_PIN      SCK   // SPI clock (shared)
#define LORA_MISO_PIN    MISO   // SPI MISO (shared)
#define LORA_MOSI_PIN    MOSI   // SPI MOSI (shared)
#define LORA_CS_PIN         5   // Chip select
#define LORA_RST_PIN        6   // Reset pin
#define LORA_DIO0_PIN       7   // Interrupt pin
```

## Board-Specific Configuration

### ESP32-S3 Specific Settings
```cpp
// Optimal configuration for LilyGO T-Camera Plus S3
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
config.xclk_freq_hz = 20000000;       // 20MHz clock for optimal performance
config.frame_size = FRAMESIZE_UXGA;   // Maximum resolution
config.pixel_format = PIXFORMAT_JPEG; // JPEG format for efficiency
config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
config.fb_location = CAMERA_FB_IN_PSRAM; // Use PSRAM for frame buffer
config.jpeg_quality = 12;             // Good quality/size balance
config.fb_count = 1;                  // Single frame buffer
```

### PSRAM Configuration
```cpp
// Enable and configure PSRAM for large frame buffers
void configurePSRAM() {
    if (psramFound()) {
        Serial.println("PSRAM found and enabled");
        // Use PSRAM for camera frame buffer
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.fb_count = 2;  // Dual buffer for smoother operation
    } else {
        Serial.println("PSRAM not found, using internal RAM");
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.fb_count = 1;  // Single buffer to save RAM
        config.frame_size = FRAMESIZE_SVGA; // Lower resolution for RAM
    }
}
```

### Camera Initialization Parameters
```cpp
// Initialize camera with optimal settings
esp_err_t camera_init() {
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return err;
    }
    
    // Fine-tune camera settings
    sensor_t *s = esp_camera_sensor_get();
    s->set_brightness(s, 0);     // Brightness: -2 to 2
    s->set_contrast(s, 0);       // Contrast: -2 to 2  
    s->set_saturation(s, 0);     // Saturation: -2 to 2
    s->set_special_effect(s, 0); // Special effects: 0-6
    s->set_whitebal(s, 1);       // White balance: enable
    s->set_awb_gain(s, 1);       // Auto white balance gain: enable
    s->set_wb_mode(s, 0);        // White balance mode: 0-4
    s->set_exposure_ctrl(s, 1);  // Exposure control: enable
    s->set_aec2(s, 0);           // AEC algorithm: 0 or 1
    s->set_ae_level(s, 0);       // AE level: -2 to 2
    s->set_aec_value(s, 300);    // AEC value: 0-1200
    s->set_gain_ctrl(s, 1);      // Gain control: enable
    s->set_agc_gain(s, 0);       // AGC gain: 0-30
    s->set_gainceiling(s, (gainceiling_t)0); // Gain ceiling: 0-6
    
    return ESP_OK;
}
```

### Power Management Settings
```cpp
// Configure power management for battery operation
void configurePowerManagement() {
    // Enable automatic light sleep
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = 240,      // Maximum CPU frequency
        .min_freq_mhz = 10,       // Minimum CPU frequency
        .light_sleep_enable = true // Enable automatic light sleep
    };
    esp_pm_configure(&pm_config);
    
    // Configure wake-up sources
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_1, 1); // PIR sensor wake-up
    esp_sleep_enable_timer_wakeup(30 * 60 * 1000000); // 30-minute timer
}
```

## Troubleshooting Section

### Common Issues and Solutions

#### Upload/Programming Issues

| Problem | Symptoms | Solution |
|---------|----------|----------|
| **Upload fails** | "Failed to connect to ESP32" | • Check USB-C cable connection<br>• Try different USB port<br>• Press and hold RESET while clicking upload |
| **Board not detected** | No COM port visible | • Install USB-to-serial drivers<br>• Try different USB cable<br>• Check Device Manager (Windows) |
| **Compilation errors** | Red text in console | • Verify board selection (ESP32S3 Dev Module)<br>• Check library installations<br>• Update ESP32 board package |

#### Camera Initialization Issues

| Problem | Symptoms | Solution |
|---------|----------|----------|
| **Camera init failed** | Error 0x105 in serial | • Check camera module connection<br>• Verify pin definitions<br>• Ensure adequate power supply |
| **Poor image quality** | Blurry or dark images | • Adjust JPEG quality setting<br>• Tune brightness/contrast<br>• Check lens focus |
| **Frame buffer errors** | Memory allocation fails | • Enable PSRAM in board settings<br>• Reduce frame size<br>• Use single frame buffer |

#### WiFi Connection Problems

```cpp
// WiFi debugging code
void debugWiFiConnection() {
    Serial.println("WiFi Status Codes:");
    Serial.println("0: WL_IDLE_STATUS - changing between statuses");
    Serial.println("1: WL_NO_SSID_AVAIL - no SSID available");
    Serial.println("2: WL_SCAN_COMPLETED - scan completed");
    Serial.println("3: WL_CONNECTED - connected to WiFi");
    Serial.println("4: WL_CONNECT_FAILED - connection failed");
    Serial.println("5: WL_CONNECTION_LOST - connection lost");
    Serial.println("6: WL_DISCONNECTED - disconnected");
    
    Serial.printf("Current status: %d\n", WiFi.status());
    Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
}
```

#### Memory Issues

```cpp
// Memory debugging utilities
void printMemoryInfo() {
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    Serial.printf("Largest free block: %d bytes\n", ESP.getMaxAllocHeap());
    Serial.printf("Min free heap: %d bytes\n", ESP.getMinFreeHeap());
}
```

#### Power Supply Issues

| Problem | Symptoms | Solution |
|---------|----------|----------|
| **Brownout resets** | Random reboots during operation | • Use external 5V/2A power supply<br>• Add capacitors for power filtering<br>• Check USB cable quality |
| **Poor WiFi performance** | Weak signal, disconnections | • Ensure adequate power supply<br>• Use external antenna<br>• Check for interference |

### Debug Tips

1. **Enable Debug Output**
   ```cpp
   // In Arduino IDE, set Tools > Core Debug Level to "Verbose"
   Serial.setDebugOutput(true);
   ```

2. **Monitor Heap Memory**
   ```cpp
   void setup() {
       Serial.begin(115200);
       printMemoryInfo(); // Check initial memory
   }
   
   void loop() {
       // Check for memory leaks periodically
       static unsigned long lastCheck = 0;
       if (millis() - lastCheck > 10000) {
           printMemoryInfo();
           lastCheck = millis();
       }
   }
   ```

3. **Camera Debug Mode**
   ```cpp
   // Enable camera debug for initialization issues
   #define CAMERA_DEBUG 1
   ```

## Example Code Snippets

### Basic Camera Initialization
```cpp
#include "esp_camera.h"

// Camera pin definitions for LilyGO T-Camera Plus S3
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     40
#define SIOD_GPIO_NUM     17
#define SIOC_GPIO_NUM     18
#define Y9_GPIO_NUM       39
#define Y8_GPIO_NUM       41
#define Y7_GPIO_NUM       42
#define Y6_GPIO_NUM       12
#define Y5_GPIO_NUM        3
#define Y4_GPIO_NUM       14
#define Y3_GPIO_NUM       47
#define Y2_GPIO_NUM       13
#define VSYNC_GPIO_NUM    21
#define HREF_GPIO_NUM     38
#define PCLK_GPIO_NUM     11

void setupCamera() {
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
    
    // Use PSRAM for frame buffer if available
    if(psramFound()){
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    
    Serial.println("Camera initialized successfully");
}
```

### PIR Motion Sensor Integration
```cpp
#define PIR_PIN 1
#define LED_PIN 48

volatile bool motionDetected = false;

void IRAM_ATTR motionISR() {
    motionDetected = true;
}

void setupMotionSensor() {
    pinMode(PIR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionISR, RISING);
    Serial.println("PIR motion sensor initialized");
}

void checkMotion() {
    if (motionDetected) {
        Serial.println("Motion detected!");
        digitalWrite(LED_PIN, HIGH);
        
        // Capture photo when motion detected
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) {
            Serial.printf("Image captured: %d bytes\n", fb->len);
            // Process or save image here
            esp_camera_fb_return(fb);
        }
        
        motionDetected = false;
        digitalWrite(LED_PIN, LOW);
    }
}
```

### WiFi Configuration Example
```cpp
#include <WiFi.h>
#include <AsyncWebServer.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

AsyncWebServer server(80);

void setupWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("WiFi connected! IP address: ");
        Serial.println(WiFi.localIP());
        
        setupWebServer();
    } else {
        Serial.println("WiFi connection failed!");
    }
}

void setupWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", "<h1>LilyGO T-Camera Plus S3</h1><p>Camera server running</p>");
    });
    
    server.on("/capture", HTTP_GET, [](AsyncWebServerRequest *request){
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            request->send(500, "text/plain", "Camera capture failed");
            return;
        }
        
        AsyncWebServerResponse *response = request->beginResponse_P(200, "image/jpeg", fb->buf, fb->len);
        response->addHeader("Content-Disposition", "inline; filename=capture.jpg");
        request->send(response);
        
        esp_camera_fb_return(fb);
    });
    
    server.begin();
    Serial.println("Web server started");
}
```

### Basic Network Configuration
```cpp
#include <WiFiManager.h>

void setupNetworkConfiguration() {
    WiFiManager wifiManager;
    
    // Reset settings for testing
    // wifiManager.resetSettings();
    
    // Set custom IP for portal
    wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
    
    // Set configuration portal timeout
    wifiManager.setConfigPortalTimeout(300); // 5 minutes
    
    // Try to connect to saved WiFi or start config portal
    if (!wifiManager.autoConnect("LilyGO-T-Camera-S3", "wildlife123")) {
        Serial.println("Failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
    }
    
    Serial.println("WiFi connected successfully");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}
```

### Environmental Sensor Integration
```cpp
#include <DHT.h>

#define DHT_PIN 2
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

void setupEnvironmentalSensor() {
    dht.begin();
    Serial.println("DHT22 sensor initialized");
}

void readEnvironmentalData() {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }
    
    Serial.printf("Temperature: %.2f°C, Humidity: %.2f%%\n", temperature, humidity);
    
    // Add environmental data to image metadata
    // This can be embedded in JPEG EXIF data or saved separately
}
```

## Conclusion

Congratulations! You have successfully deployed the LilyGO T-Camera Plus S3. This advanced ESP32-S3 based camera board offers excellent performance for wildlife monitoring applications with its enhanced processing power, large PSRAM, and comprehensive GPIO options.

### Key Advantages of T-Camera Plus S3:
- **High Performance**: ESP32-S3 with AI acceleration capabilities
- **Large Memory**: 8MB PSRAM for high-resolution image processing
- **Easy Programming**: Built-in USB-to-serial converter
- **Expandable**: Multiple GPIO pins for sensors and peripherals
- **Future-Ready**: Support for advanced AI/ML applications

### Next Steps:
1. **Customize for your application**: Modify code for specific wildlife monitoring needs
2. **Add sensors**: Integrate PIR, environmental, or LoRa modules
3. **Optimize power**: Implement deep sleep for battery operation
4. **Scale deployment**: Use this guide as template for multiple units

You can adapt these instructions for other ESP32 camera boards by replacing specific pin definitions, board selections, and hardware specifications as needed.