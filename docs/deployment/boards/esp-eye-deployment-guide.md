# ESP-EYE Deployment Guide

This guide provides comprehensive step-by-step instructions for deploying the ESP-EYE development board. The ESP-EYE is Espressif's official AI camera development board, featuring advanced computer vision capabilities and optimized for machine learning applications.

## Overview

The ESP-EYE is a professional-grade ESP32 camera development board with enhanced features for AI and computer vision applications. It includes built-in accelerometer, microphone support, and advanced processing capabilities, making it ideal for sophisticated wildlife monitoring systems with voice recognition, motion analysis, and real-time AI classification.

## Prerequisites

### Hardware Requirements
- ESP-EYE development board (Espressif official)
- Micro USB cable for programming and power
- Computer with Arduino IDE installed
- MicroSD card (up to 32GB, FAT32 formatted)
- External sensors (PIR, environmental sensors)
- I2S microphone or audio input device (optional)
- External antennas (WiFi, optional for extended range)
- Power bank or solar charging system for field deployment

### Software Requirements
- Latest version of Arduino IDE (2.0+ recommended)
- ESP32 board package for Arduino IDE (version 2.0.0+)
- Required libraries:
  - ESP32 Camera library
  - ESP-DL library (for AI/ML features)
  - AsyncWebServer library
  - ArduinoJson library
  - ESP8266Audio library (for audio features)
  - MPU6050 library (for accelerometer)

### Additional Components Needed
- Breadboard and jumper wires for sensor connections
- Environmental sensors: BME280, SHT30 (I2C interface)
- Audio components: MAX98357A I2S audio amplifier
- Motion sensors: PIR sensors, accelerometer (built-in MPU6050)
- GPS module: NEO-8M for location tracking
- LoRa module: SX1276 for long-range communication
- Weather-resistant enclosure rated IP65 or higher
- Solar panel (15W recommended) and charge controller
- High-capacity battery: 3.7V Li-ion with protection circuit

## Step-by-Step Instructions

### 1. Setting Up the Arduino IDE

1. **Download and Install Arduino IDE**
   - Download the latest Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)
   - Install version 2.0+ for best ESP32 support and ESP-DL integration

2. **Add ESP32 Board Support**
   - Open Arduino IDE
   - Go to `File` > `Preferences`
   - Add the following URL to **Additional Boards Manager URLs**:
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Click "OK" to save

3. **Install ESP32 Board Package**
   - Navigate to `Tools` > `Board` > `Boards Manager`
   - Search for "ESP32"
   - Install **ESP32 by Espressif Systems** package (ensure version 2.0.0+)
   - Wait for installation to complete
   - Restart Arduino IDE

### 2. Board Manager Installation Steps

1. **Select the Correct Board**
   - Go to `Tools` > `Board` > `ESP32 Arduino`
   - Select **ESP32 Wrover Module** (ESP-EYE is based on ESP32-WROVER-B)

2. **Configure Board Settings for ESP-EYE**
   - **Upload Speed**: 921600 (fast upload)
   - **CPU Frequency**: 240MHz (maximum performance)
   - **Flash Frequency**: 80MHz
   - **Flash Mode**: QIO
   - **Flash Size**: 4MB (32Mb)
   - **PSRAM**: Enabled (ESP-EYE has 8MB PSRAM)
   - **Partition Scheme**: Huge APP (3MB No OTA/1MB SPIFFS)
   - **Core Debug Level**: None (or Info for debugging)

### 3. Connecting the Board

#### Micro USB Connection
```
ESP-EYE    →    Computer
--------        ---------
Micro USB  →    USB Port
```

**ESP-EYE Connection Features:**
- Built-in USB-to-serial converter (CH340G)
- Automatic programming mode detection
- Power and programming through single USB cable
- Reset and boot buttons for manual control

1. **Physical Connection Steps**
   - Connect Micro USB cable to ESP-EYE
   - Connect other end to computer
   - ESP-EYE should power on (check LED indicators)
   - Windows/Mac should detect USB serial device

2. **Verify Connection**
   - Check Device Manager (Windows) or System Information (Mac)
   - Look for CH340 or USB-SERIAL device
   - Select correct COM port in `Tools` > `Port`

### 4. Writing/Loading the Code

1. **Install Required Libraries**
   - Go to `Tools` > `Manage Libraries`
   - Install these libraries:
     - **ESP32Cam** (search for "esp32 camera")
     - **ESPAsyncWebServer** (search for "ESP Async WebServer")
     - **ArduinoJson** (search for "ArduinoJson")
     - **MPU6050_light** (for accelerometer)

2. **ESP-EYE Pin Configuration**
   ```cpp
   // ESP-EYE specific pin definitions
   #define PWDN_GPIO_NUM     -1  // Not connected
   #define RESET_GPIO_NUM    -1  // Not connected (software reset)
   #define XCLK_GPIO_NUM     4   // External clock
   #define SIOD_GPIO_NUM     18  // I2C SDA
   #define SIOC_GPIO_NUM     23  // I2C SCL
   
   // Camera data pins
   #define Y9_GPIO_NUM       36  // D9
   #define Y8_GPIO_NUM       37  // D8
   #define Y7_GPIO_NUM       38  // D7
   #define Y6_GPIO_NUM       39  // D6
   #define Y5_GPIO_NUM       35  // D5
   #define Y4_GPIO_NUM       14  // D4
   #define Y3_GPIO_NUM       13  // D3
   #define Y2_GPIO_NUM       34  // D2
   #define VSYNC_GPIO_NUM    5   // Vertical sync
   #define HREF_GPIO_NUM     27  // Horizontal reference
   #define PCLK_GPIO_NUM     25  // Pixel clock
   
   // Built-in peripherals
   #define LED_GPIO_NUM      21  // Built-in LED
   #define FLASH_LED_GPIO    22  // Flash LED
   #define BOOT_BUTTON       0   // Boot button
   #define USER_BUTTON       15  // User button
   ```

### 5. Compiling the Code

1. **Verify Code Syntax**
   - Click the checkmark button (✓) to verify/compile
   - Check for compilation errors in the bottom panel
   - Ensure PSRAM is enabled in board settings

### 6. Uploading the Code

1. **Standard Upload Process**
   - Connect ESP-EYE via Micro USB
   - Select correct COM port
   - Click upload button (→) in Arduino IDE
   - ESP-EYE handles boot mode automatically

### 7. Testing the Deployment

1. **Open Serial Monitor**
   - Go to `Tools` > `Serial Monitor`
   - Set baud rate to 115200
   - Press RESET button on ESP-EYE

2. **Verify System Initialization**
   - Check for ESP32 chip identification
   - PSRAM detection (should show 8MB)
   - Camera initialization success
   - MPU6050 accelerometer initialization
   - WiFi connection status

## Hardware Specifications

### Board Features
- **Microcontroller**: ESP32-WROVER-B (240MHz dual-core)
- **Flash Memory**: 4MB SPI Flash
- **SRAM**: 520KB internal SRAM
- **PSRAM**: 8MB external PSRAM
- **Camera Interface**: DVP 8-bit parallel
- **Motion Sensor**: MPU6050 6-axis accelerometer/gyroscope
- **Connectivity**: WiFi 802.11 b/g/n, Bluetooth 4.2 LE
- **USB**: Micro USB with CH340G converter
- **Operating Voltage**: 3.3V
- **Dimensions**: 90mm × 40mm × 12mm

### Camera Specifications
- **Sensor**: OV2640 CMOS (enhanced for AI)
- **Resolution**: Up to 1600×1200 (UXGA)
- **Formats**: JPEG, RGB565, YUV422, Grayscale
- **Frame Rate**: Up to 60fps at lower resolutions
- **Field of View**: ~66° diagonal
- **Low Light**: Enhanced sensitivity to 0.5 lux

## Pin Configuration

### Available Pins for External Sensors
```cpp
// Safe GPIO pins for external use
#define PIR_SENSOR_PIN    2   // PIR motion sensor
#define EXTERNAL_LED_PIN  16  // External status LED
#define RELAY_CONTROL_PIN 17  // Relay or actuator control
#define GPIO_EXT_1        12  // General purpose I/O
#define TEMPERATURE_PIN   1   // OneWire temperature sensor
```

### Pin Conflicts and Limitations
- Camera interface uses pins 4, 5, 13, 14, 18, 23, 25, 27, 34-39
- MPU6050 shares I2C bus with camera (pins 18, 23)
- I2S audio interface available on pins 26, 32, 33
- Limited external GPIO available due to comprehensive built-in features

## Board-Specific Configuration

### MPU6050 Accelerometer Integration
```cpp
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

void initializeMPU6050() {
    Wire.begin(18, 23);  // SDA, SCL (shared with camera)
    
    if (mpu.begin() != 0) {
        Serial.println("MPU6050 initialization failed");
    } else {
        Serial.println("MPU6050 initialized successfully");
        mpu.calcOffsets();  // Calculate gyro and accelerometer offsets
    }
}

void readMotionData() {
    mpu.update();
    
    float accelX = mpu.getAccX();
    float accelY = mpu.getAccY();
    float accelZ = mpu.getAccZ();
    
    float gyroX = mpu.getGyroX();
    float gyroY = mpu.getGyroY();
    float gyroZ = mpu.getGyroZ();
    
    // Detect significant motion
    float totalAccel = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
    if (totalAccel > 1.5) {  // Threshold for motion detection
        Serial.println("Significant motion detected!");
    }
}
```

## Troubleshooting Section

### Common Issues and Solutions

**Problem: Camera initialization fails**
- **Solution**: Verify PSRAM is enabled in board settings
- **Solution**: Check pin connections and power supply
- **Solution**: Ensure MPU6050 is not conflicting with camera I2C

**Problem: MPU6050 not detected**
- **Solution**: Check I2C wiring (pins 18 and 23)
- **Solution**: Verify I2C address (usually 0x68)
- **Solution**: Ensure sufficient power supply

**Problem: Upload fails**
- **Solution**: Check USB cable and driver installation
- **Solution**: Try manual boot mode (hold BOOT, press RESET)
- **Solution**: Verify board selection and port

## Example Code Snippets

### Basic ESP-EYE Setup with Motion Detection
```cpp
#include "esp_camera.h"
#include <WiFi.h>
#include <MPU6050_light.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

MPU6050 mpu(Wire);

void setup() {
    Serial.begin(115200);
    
    // Initialize I2C for MPU6050
    Wire.begin(18, 23);
    
    // Initialize camera (ESP-EYE configuration)
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 34;
    config.pin_d1 = 13;
    config.pin_d2 = 14;
    config.pin_d3 = 35;
    config.pin_d4 = 39;
    config.pin_d5 = 38;
    config.pin_d6 = 37;
    config.pin_d7 = 36;
    config.pin_xclk = 4;
    config.pin_pclk = 25;
    config.pin_vsync = 5;
    config.pin_href = 27;
    config.pin_sscb_sda = 18;
    config.pin_sscb_scl = 23;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    
    if(psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.fb_location = CAMERA_FB_IN_PSRAM;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    
    // Initialize MPU6050
    if (mpu.begin() != 0) {
        Serial.println("MPU6050 initialization failed");
    } else {
        Serial.println("MPU6050 initialized successfully");
        mpu.calcOffsets();
    }
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
    
    Serial.println("ESP-EYE Wildlife Monitor ready!");
}

void loop() {
    mpu.update();
    
    // Check for motion
    float totalAccel = sqrt(mpu.getAccX()*mpu.getAccX() + 
                           mpu.getAccY()*mpu.getAccY() + 
                           mpu.getAccZ()*mpu.getAccZ());
    
    if (totalAccel > 1.5) {
        Serial.println("Motion detected! Capturing image...");
        
        camera_fb_t* fb = esp_camera_fb_get();
        if (fb) {
            Serial.printf("Image captured: %d bytes\n", fb->len);
            // Process or save image here
            esp_camera_fb_return(fb);
        }
    }
    
    delay(100);
}
```

---

*This ESP-EYE deployment guide provides comprehensive instructions tailored for Espressif's official AI camera development board. The guide emphasizes the unique features of ESP-EYE including the built-in MPU6050 accelerometer, enhanced AI capabilities, and professional-grade development features. It serves as both a standalone deployment reference and demonstrates advanced ESP32 camera board capabilities for sophisticated wildlife monitoring applications.*