# TTGO T-Camera Deployment Guide

This comprehensive guide provides step-by-step instructions for deploying the TTGO T-Camera for wildlife monitoring applications. The TTGO T-Camera features an integrated OLED display and comprehensive GPIO options, making it ideal for standalone monitoring systems that require local status display and user interaction capabilities.

## Prerequisites

### Hardware Requirements
- **TTGO T-Camera board** (with integrated OLED display)
- **USB-C cable** for programming and power
- **Computer** with Windows, macOS, or Linux
- **MicroSD card** (16-64GB, Class 10 recommended)
- **PIR motion sensor** (HC-SR501 or similar)
- **External power supply** (5V 2A recommended)
- **18650 battery** (optional, for portable operation)

### Software Requirements
- **Arduino IDE 2.0+** or **PlatformIO**
- **ESP32 board package** for Arduino IDE
- **Required libraries:**
  - ESP32 Camera Driver
  - WiFi library (built-in)
  - SD library (built-in)
  - ArduinoJson (for configuration)
  - U8g2 library (for OLED display)
  - TFT_eSPI (alternative display library)

### Additional Components
- **Breadboard or PCB** for prototype circuits
- **Jumper wires** for sensor connections
- **Weatherproof enclosure** (IP65 rated minimum)
- **Solar panel and battery** for field deployment
- **External sensors** (temperature, humidity, light)

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
     - **U8g2** by oliver (for OLED display)
     - **ArduinoJson** by Benoit Blanchon

### 2. Connecting the Board

1. **Connect via USB-C:**
   - Use a quality USB-C cable to connect the TTGO T-Camera to your computer
   - The board should power on (OLED display will light up)

2. **Select board and port in Arduino IDE:**
   - Go to `Tools` > `Board` > `ESP32 Arduino`
   - Select **"TTGO T1"** or **"ESP32 Dev Module"**
   - Go to `Tools` > `Port` and select the appropriate COM port
   - Set **Upload Speed** to **921600**

3. **Configure board settings:**
   - **CPU Frequency:** 240MHz
   - **Flash Mode:** QIO
   - **Flash Size:** 4MB
   - **Partition Scheme:** Default 4MB with SPIFFS
   - **PSRAM:** Enabled

### 3. Writing the Code

1. **Create a new sketch** in Arduino IDE

2. **Basic camera and display initialization code:**
   ```cpp
   #include "esp_camera.h"
   #include "WiFi.h"
   #include "SD_MMC.h"
   #include <U8g2lib.h>
   #include <Wire.h>
   
   // TTGO T-Camera pin definitions
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
   
   #define LED_GPIO_NUM      2  // Status LED
   #define FLASH_GPIO_NUM    4  // Flash LED
   
   // Display pins (I2C)
   #define OLED_SDA         21
   #define OLED_SCL         22
   #define OLED_RST         -1  // Reset pin (or -1 if sharing Arduino reset pin)
   
   // Initialize OLED display
   U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, OLED_RST, OLED_SCL, OLED_SDA);
   
   void setup() {
     Serial.begin(115200);
     
     // Initialize display
     u8g2.begin();
     u8g2.clearBuffer();
     u8g2.setFont(u8g2_font_ncenB08_tr);
     u8g2.drawStr(0, 15, "T-Camera Starting...");
     u8g2.sendBuffer();
     
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
     config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
     config.jpeg_quality = 12;            // Good quality
     config.fb_count = 2;                 // Dual frame buffers
     config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
     
     // Initialize camera
     esp_err_t err = esp_camera_init(&config);
     if (err != ESP_OK) {
       Serial.printf("Camera init failed with error 0x%x", err);
       u8g2.clearBuffer();
       u8g2.drawStr(0, 15, "Camera FAILED!");
       u8g2.sendBuffer();
       return;
     }
     
     u8g2.clearBuffer();
     u8g2.drawStr(0, 15, "T-Camera Ready");
     u8g2.sendBuffer();
     
     Serial.println("TTGO T-Camera initialized successfully");
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

> **⚠️ Warning:** Ensure PSRAM is enabled for optimal performance with display and camera

### 5. Uploading the Code

1. **Click the upload button** (arrow) in Arduino IDE
2. **Monitor the output console** for upload progress
3. **Wait for "Hard resetting via RTS pin"** message
4. **The board will automatically reset** and the OLED should show status

### 6. Testing the Deployment

1. **Check OLED display** for startup messages
2. **Open Serial Monitor** (`Tools` > `Serial Monitor`)
3. **Set baud rate to 115200**
4. **Press the Reset button** on the board
5. **Verify both serial and display output**

## Hardware Specifications

### Board Specifications
- **Microcontroller:** ESP32-WROVER-B (Dual-core Xtensa LX6, 240MHz)
- **Flash Memory:** 4MB SPI Flash
- **PSRAM:** 8MB QSPI PSRAM
- **WiFi:** 802.11 b/g/n (2.4GHz)
- **Bluetooth:** Bluetooth 4.2 Classic and BLE
- **USB:** USB-C connector for programming and power
- **Battery:** 18650 battery holder (battery not included)
- **Operating Voltage:** 3.3V
- **Input Voltage:** 5V (via USB-C) or 3.7V (via battery)
- **Dimensions:** 54mm × 54mm × 14mm

### Camera Specifications
- **Sensor:** OV2640 2-megapixel CMOS sensor
- **Resolution:** Up to 1600×1200 (UXGA)
- **Image Formats:** JPEG, RGB565, YUV422
- **Frame Rates:** Up to 60fps at VGA, 15fps at UXGA
- **Features:** Auto exposure, auto white balance, auto gain control
- **Field of View:** ~66° diagonal
- **Low Light Performance:** Good with flash LED support

### Display Specifications
- **Type:** 0.96" OLED SSD1306
- **Resolution:** 128×64 pixels
- **Colors:** Monochrome (white)
- **Interface:** I2C (SDA: GPIO21, SCL: GPIO22)
- **Brightness:** High contrast, readable in daylight
- **Power Consumption:** Low power consumption

### Memory and Storage
- **RAM:** 520KB SRAM + 8MB PSRAM
- **Flash:** 4MB for program storage
- **External Storage:** MicroSD card slot (up to 64GB)
- **Display Buffer:** ~1KB for OLED frame buffer

### Connectivity Options
- **WiFi:** Built-in with PCB antenna
- **USB:** USB-C for programming and serial communication
- **GPIO:** Multiple GPIO pins available for sensors
- **I2C:** Hardware I2C bus (shared with display)
- **SPI:** Hardware SPI bus available
- **Battery Management:** Built-in charging circuit for 18650

### Power Requirements
- **Operating Current:** 95mA (active), 6mA (light sleep)
- **Camera Current:** 160mA during capture
- **Display Current:** 20mA (active), 0.5mA (sleep)
- **Deep Sleep Current:** 6µA
- **Battery Life:** 10-15 hours with 3000mAh 18650

## Pin Configuration

### Camera Pin Mapping
```cpp
// TTGO T-Camera Camera Pins
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
#define LED_GPIO_NUM      2    // Status LED
#define FLASH_GPIO_NUM    4    // Flash LED (shared with Y4)
```

### Display and Interface Pins
```cpp
// OLED Display pins (I2C)
#define OLED_SDA         21    // I2C data
#define OLED_SCL         22    // I2C clock
#define OLED_RST         -1    // Reset (shared with ESP32 reset)

// User interface pins
#define BUTTON_0         35    // User button 0
#define BUTTON_1         34    // User button 1 (if available)

// Battery monitoring
#define BAT_ADC_PIN      33    // Battery voltage monitoring
```

### Available GPIO for External Sensors
```cpp
// Available pins for external connections
#define AVAILABLE_GPIO_PINS {0, 16, 17, 33, 34, 35}

// Recommended sensor pin assignments
#define PIR_SENSOR_PIN    0    // PIR motion sensor
#define DHT_SENSOR_PIN   16    // DHT22 temperature/humidity
#define LIGHT_SENSOR_PIN 33    // Light sensor (analog)
#define EXT_BUTTON_1     34    // External button 1 (input only)
#define EXT_BUTTON_2     35    // External button 2 (input only)
#define I2C_EXT_SDA      17    // External I2C SDA (if not using built-in)
#define I2C_EXT_SCL      16    // External I2C SCL (if not using built-in)
```

### Pin Conflicts and Limitations
```cpp
// Critical pin usage for TTGO T-Camera
struct PinConflicts {
    // Camera pins - DO NOT USE for other purposes
    int camera_pins[] = {4, 5, 12, 13, 14, 15, 18, 19, 23, 25, 26, 27, 32, 36, 39};
    
    // Display pins - DO NOT USE for other purposes
    int display_pins[] = {21, 22};  // I2C SDA, SCL
    
    // Input-only pins
    int input_only_pins[] = {34, 35, 36, 39};
    
    // Flash/Y4 pin conflict
    bool flash_y4_conflict = true;  // GPIO 4 shared between flash and camera Y4
    
    // Available for external use
    int available_pins[] = {0, 16, 17, 33, 34, 35};
    
    // Pin recommendations
    struct {
        int pir_sensor = 0;        // Motion detection
        int temperature = 16;      // DHT22 or similar
        int light_sensor = 33;     // Analog light sensor
        int button_1 = 34;         // User button (input only)
        int button_2 = 35;         // User button (input only)
    } recommended;
};
```

## Board-Specific Configuration

### OLED Display Integration
The TTGO T-Camera's integrated display enables rich status information:

```cpp
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ -1, /* clock=*/ 22, /* data=*/ 21);

class DisplayManager {
private:
    unsigned long last_update;
    int current_screen;
    const unsigned long UPDATE_INTERVAL = 1000;  // 1 second
    
public:
    enum ScreenType {
        SCREEN_STATUS,
        SCREEN_CAMERA,
        SCREEN_BATTERY,
        SCREEN_WIFI,
        SCREEN_COUNT
    };
    
    void begin() {
        u8g2.begin();
        u8g2.setFont(u8g2_font_6x10_tf);
        current_screen = SCREEN_STATUS;
        last_update = 0;
        
        showSplashScreen();
    }
    
    void update() {
        if (millis() - last_update > UPDATE_INTERVAL) {
            updateCurrentScreen();
            last_update = millis();
        }
    }
    
    void nextScreen() {
        current_screen = (current_screen + 1) % SCREEN_COUNT;
        updateCurrentScreen();
    }
    
private:
    void showSplashScreen() {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(15, 20, "TTGO T-Camera");
        u8g2.drawStr(20, 35, "Wildlife Monitor");
        u8g2.drawStr(30, 50, "Starting...");
        u8g2.sendBuffer();
        delay(2000);
    }
    
    void updateCurrentScreen() {
        switch (current_screen) {
            case SCREEN_STATUS:
                showStatusScreen();
                break;
            case SCREEN_CAMERA:
                showCameraScreen();
                break;
            case SCREEN_BATTERY:
                showBatteryScreen();
                break;
            case SCREEN_WIFI:
                showWiFiScreen();
                break;
        }
    }
    
    void showStatusScreen() {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        
        // Title
        u8g2.drawStr(0, 10, "Wildlife Monitor");
        u8g2.drawLine(0, 12, 128, 12);
        
        // Status information
        static int photo_count = 0;
        static unsigned long uptime_hours = millis() / 3600000;
        
        char buffer[32];
        sprintf(buffer, "Photos: %d", photo_count);
        u8g2.drawStr(0, 25, buffer);
        
        sprintf(buffer, "Uptime: %luh", uptime_hours);
        u8g2.drawStr(0, 35, buffer);
        
        // Motion status
        bool motion_detected = digitalRead(0) == HIGH;  // Assuming PIR on GPIO 0
        u8g2.drawStr(0, 45, motion_detected ? "Motion: YES" : "Motion: NO");
        
        // Battery percentage
        int battery_percent = getBatteryPercentage();
        sprintf(buffer, "Batt: %d%%", battery_percent);
        u8g2.drawStr(0, 55, buffer);
        
        u8g2.sendBuffer();
    }
    
    void showCameraScreen() {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        
        u8g2.drawStr(0, 10, "Camera Status");
        u8g2.drawLine(0, 12, 128, 12);
        
        // Camera information
        sensor_t * s = esp_camera_sensor_get();
        if (s) {
            sensor_id_t id = s->id;
            char buffer[32];
            
            sprintf(buffer, "Sensor: OV%04X", id.PID);
            u8g2.drawStr(0, 25, buffer);
            
            framesize_t framesize = s->status.framesize;
            const char* size_str = "";
            switch (framesize) {
                case FRAMESIZE_UXGA: size_str = "UXGA"; break;
                case FRAMESIZE_SVGA: size_str = "SVGA"; break;
                case FRAMESIZE_VGA:  size_str = "VGA"; break;
                default: size_str = "Other"; break;
            }
            
            sprintf(buffer, "Size: %s", size_str);
            u8g2.drawStr(0, 35, buffer);
            
            sprintf(buffer, "Quality: %d", s->status.quality);
            u8g2.drawStr(0, 45, buffer);
            
            sprintf(buffer, "AWB: %s", s->status.awb ? "ON" : "OFF");
            u8g2.drawStr(0, 55, buffer);
        } else {
            u8g2.drawStr(0, 25, "Camera: ERROR");
        }
        
        u8g2.sendBuffer();
    }
    
    void showBatteryScreen() {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        
        u8g2.drawStr(0, 10, "Power Status");
        u8g2.drawLine(0, 12, 128, 12);
        
        float voltage = readBatteryVoltage();
        int percentage = getBatteryPercentage();
        
        char buffer[32];
        sprintf(buffer, "Voltage: %.2fV", voltage);
        u8g2.drawStr(0, 25, buffer);
        
        sprintf(buffer, "Level: %d%%", percentage);
        u8g2.drawStr(0, 35, buffer);
        
        // Battery bar
        int bar_width = (percentage * 100) / 100;  // Scale to 100 pixels
        u8g2.drawFrame(0, 45, 102, 8);
        u8g2.drawBox(1, 46, bar_width, 6);
        
        // Power source
        bool usb_powered = (voltage > 4.5);  // Rough USB detection
        u8g2.drawStr(0, 63, usb_powered ? "Source: USB" : "Source: Battery");
        
        u8g2.sendBuffer();
    }
    
    void showWiFiScreen() {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        
        u8g2.drawStr(0, 10, "WiFi Status");
        u8g2.drawLine(0, 12, 128, 12);
        
        if (WiFi.status() == WL_CONNECTED) {
            u8g2.drawStr(0, 25, "Connected");
            
            String ssid = WiFi.SSID();
            if (ssid.length() > 16) ssid = ssid.substring(0, 16);
            u8g2.drawStr(0, 35, ssid.c_str());
            
            char buffer[32];
            sprintf(buffer, "RSSI: %ddBm", WiFi.RSSI());
            u8g2.drawStr(0, 45, buffer);
            
            // IP address (truncated)
            String ip = WiFi.localIP().toString();
            if (ip.length() > 16) ip = ip.substring(0, 16);
            u8g2.drawStr(0, 55, ip.c_str());
        } else {
            u8g2.drawStr(0, 25, "Disconnected");
            u8g2.drawStr(0, 35, "Searching...");
        }
        
        u8g2.sendBuffer();
    }
};

DisplayManager display;
```

### User Interface with Buttons
Implement user interaction through buttons and display:

```cpp
class UserInterface {
private:
    unsigned long last_button_press;
    bool button_pressed;
    const unsigned long BUTTON_DEBOUNCE = 200;
    
public:
    void begin() {
        pinMode(35, INPUT);  // Button pin (input only)
        last_button_press = 0;
        button_pressed = false;
    }
    
    void update() {
        // Check for button press
        if (digitalRead(35) == LOW && !button_pressed) {
            if (millis() - last_button_press > BUTTON_DEBOUNCE) {
                handleButtonPress();
                last_button_press = millis();
                button_pressed = true;
            }
        } else if (digitalRead(35) == HIGH) {
            button_pressed = false;
        }
    }
    
private:
    void handleButtonPress() {
        // Cycle through display screens
        display.nextScreen();
        
        // Optional: Add other button functions
        Serial.println("Button pressed - screen changed");
    }
};

UserInterface ui;
```

### Wildlife Monitoring Integration
Combine camera, display, and sensors for comprehensive monitoring:

```cpp
class TCameraWildlifeMonitor {
private:
    unsigned long last_check;
    int total_captures;
    bool motion_active;
    
public:
    void begin() {
        display.begin();
        ui.begin();
        setupSensors();
        
        last_check = millis();
        total_captures = 0;
        motion_active = true;
        
        // Show startup complete on display
        showStartupMessage();
    }
    
    void update() {
        // Update display and UI
        display.update();
        ui.update();
        
        // Check for motion
        if (motion_active && digitalRead(0) == HIGH) {  // PIR on GPIO 0
            handleMotionDetection();
        }
        
        // Periodic system checks
        if (millis() - last_check > 30000) {  // Every 30 seconds
            performSystemCheck();
            last_check = millis();
        }
    }
    
private:
    void setupSensors() {
        pinMode(0, INPUT);   // PIR sensor
        pinMode(16, INPUT);  // DHT22 or other sensor
        pinMode(2, OUTPUT);  // Status LED
        
        Serial.println("Sensors configured for wildlife monitoring");
    }
    
    void showStartupMessage() {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(10, 20, "Wildlife Monitor");
        u8g2.drawStr(25, 35, "READY");
        u8g2.drawStr(15, 50, "Press button for");
        u8g2.drawStr(25, 62, "status info");
        u8g2.sendBuffer();
        delay(3000);
    }
    
    void handleMotionDetection() {
        Serial.println("Wildlife motion detected!");
        
        // Flash status LED
        digitalWrite(2, HIGH);
        
        // Show motion detected on display
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(20, 25, "MOTION");
        u8g2.drawStr(15, 40, "DETECTED!");
        u8g2.sendBuffer();
        
        // Capture photo
        camera_fb_t * fb = esp_camera_fb_get();
        if (fb) {
            String filename = "/wildlife_" + String(total_captures) + ".jpg";
            savePhotoToSD(fb, filename);
            total_captures++;
            
            // Show capture success
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_6x10_tf);
            u8g2.drawStr(10, 20, "Photo Captured!");
            char buffer[32];
            sprintf(buffer, "Total: %d", total_captures);
            u8g2.drawStr(30, 35, buffer);
            u8g2.sendBuffer();
            
            esp_camera_fb_return(fb);
        } else {
            // Show capture error
            u8g2.clearBuffer();
            u8g2.drawStr(15, 30, "Capture FAILED");
            u8g2.sendBuffer();
        }
        
        digitalWrite(2, LOW);
        delay(2000);  // Show message for 2 seconds
    }
    
    void savePhotoToSD(camera_fb_t* fb, String filename) {
        File file = SD_MMC.open(filename, FILE_WRITE);
        if (file) {
            file.write(fb->buf, fb->len);
            file.close();
            Serial.printf("Photo saved: %s (%d bytes)\n", filename.c_str(), fb->len);
        } else {
            Serial.println("Failed to save photo to SD card");
        }
    }
    
    void performSystemCheck() {
        // Check battery
        int battery_percent = getBatteryPercentage();
        if (battery_percent < 20) {
            Serial.println("Low battery warning");
            showLowBatteryWarning();
        }
        
        // Check SD card space
        uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
        uint64_t usedBytes = SD_MMC.usedBytes() / (1024 * 1024);
        float usage_percent = (float)usedBytes / cardSize * 100;
        
        if (usage_percent > 90) {
            Serial.println("SD card nearly full");
            showSDWarning();
        }
        
        // Update capture count display
        Serial.printf("System check: %d photos, %d%% battery, %.1f%% SD usage\n",
                     total_captures, battery_percent, usage_percent);
    }
    
    void showLowBatteryWarning() {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(25, 20, "WARNING");
        u8g2.drawStr(15, 35, "Low Battery!");
        u8g2.drawStr(10, 50, "Connect Charger");
        u8g2.sendBuffer();
        delay(3000);
    }
    
    void showSDWarning() {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(25, 20, "WARNING");
        u8g2.drawStr(20, 35, "SD Card Full");
        u8g2.drawStr(15, 50, "Replace Soon");
        u8g2.sendBuffer();
        delay(3000);
    }
};

TCameraWildlifeMonitor wildlife_monitor;
```

## Troubleshooting Section

### Common Issues and Solutions

#### Display Not Working
**Symptoms:**
- OLED display remains blank
- Corrupted display output
- Display flickering

**Solutions:**
1. **Check I2C connections:**
   ```cpp
   void testI2CDisplay() {
       Wire.begin(21, 22);  // SDA, SCL
       Wire.beginTransmission(0x3C);  // SSD1306 address
       uint8_t error = Wire.endTransmission();
       
       if (error == 0) {
           Serial.println("Display I2C communication OK");
       } else {
           Serial.printf("Display I2C error: %d\n", error);
           Serial.println("Check SDA/SCL connections");
       }
   }
   ```

2. **Display initialization debug:**
   ```cpp
   void debugDisplay() {
       Serial.println("Initializing display...");
       
       if (!u8g2.begin()) {
           Serial.println("Display initialization failed");
           return;
       }
       
       // Test pattern
       u8g2.clearBuffer();
       u8g2.drawFrame(0, 0, 128, 64);
       u8g2.drawStr(10, 30, "Test Pattern");
       u8g2.sendBuffer();
       
       Serial.println("Display test complete");
   }
   ```

#### Battery and Power Issues
**Symptoms:**
- Inaccurate battery readings
- Unexpected shutdowns
- Charging issues

**Solutions:**
1. **Battery voltage calibration:**
   ```cpp
   float calibrateBatteryReading() {
       // Take multiple readings for accuracy
       int total = 0;
       for (int i = 0; i < 10; i++) {
           total += analogRead(33);  // Battery ADC pin
           delay(10);
       }
       
       int average = total / 10;
       
       // Convert to voltage with calibration factor
       // This factor may need adjustment based on your specific board
       float voltage = (average * 3.3 / 4095.0) * 2.0 * 1.1;  // 1.1 is calibration
       
       Serial.printf("Raw ADC: %d, Calculated voltage: %.2fV\n", average, voltage);
       return voltage;
   }
   ```

2. **Power consumption analysis:**
   ```cpp
   void analyzePowerConsumption() {
       Serial.println("=== Power Analysis ===");
       
       unsigned long start_time = millis();
       
       // Measure different states
       Serial.println("Active mode (display on, camera ready)");
       delay(5000);
       
       u8g2.setPowerSave(1);  // Turn off display
       Serial.println("Display off mode");
       delay(5000);
       
       // Put camera in low power mode
       digitalWrite(26, HIGH);  // PWDN
       Serial.println("Camera power down mode");
       delay(5000);
       
       // Restore normal operation
       digitalWrite(26, LOW);
       u8g2.setPowerSave(0);
       
       Serial.println("=== Analysis Complete ===");
   }
   ```

### Debug Tips

#### Comprehensive System Debug
```cpp
void performSystemDiagnostics() {
    Serial.println("=== TTGO T-Camera Diagnostics ===");
    
    // Memory check
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    if (psramFound()) {
        Serial.printf("PSRAM: %d bytes free\n", ESP.getFreePsram());
    } else {
        Serial.println("PSRAM: Not found");
    }
    
    // Camera check
    sensor_t * s = esp_camera_sensor_get();
    if (s) {
        Serial.printf("Camera sensor PID: 0x%02X\n", s->id.PID);
    } else {
        Serial.println("Camera: Not detected");
    }
    
    // Display check
    testI2CDisplay();
    
    // Battery check
    float voltage = calibrateBatteryReading();
    Serial.printf("Battery voltage: %.2fV\n", voltage);
    
    // SD card check
    if (SD_MMC.begin()) {
        uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
        Serial.printf("SD card: %lluMB\n", cardSize);
    } else {
        Serial.println("SD card: Not detected");
    }
    
    // WiFi check
    Serial.printf("WiFi status: %d\n", WiFi.status());
    
    Serial.println("=== Diagnostics Complete ===");
}
```

## Example Code Snippets

### Complete Wildlife Monitoring System
```cpp
#include "esp_camera.h"
#include "WiFi.h"
#include "SD_MMC.h"
#include <U8g2lib.h>

// Initialize all components
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, -1, 22, 21);
DisplayManager display;
UserInterface ui;
TCameraWildlifeMonitor wildlife_monitor;

// Utility functions
float readBatteryVoltage() {
    int reading = analogRead(33);
    return (reading * 3.3 / 4095.0) * 2.0 * 1.1;  // Calibration factor
}

int getBatteryPercentage() {
    float voltage = readBatteryVoltage();
    if (voltage >= 4.1) return 100;
    if (voltage >= 3.9) return 75;
    if (voltage >= 3.7) return 50;
    if (voltage >= 3.5) return 25;
    return voltage >= 3.3 ? 10 : 0;
}

bool initializeCamera() {
    camera_config_t config;
    // [Camera configuration code from step 3 above]
    
    esp_err_t err = esp_camera_init(&config);
    return (err == ESP_OK);
}

void setup() {
    Serial.begin(115200);
    
    // Initialize SD card
    if (!SD_MMC.begin()) {
        Serial.println("SD card initialization failed!");
        return;
    }
    
    // Initialize camera
    if (!initializeCamera()) {
        Serial.println("Camera initialization failed!");
        return;
    }
    
    // Initialize wildlife monitoring system
    wildlife_monitor.begin();
    
    Serial.println("TTGO T-Camera Wildlife Monitor Ready");
}

void loop() {
    wildlife_monitor.update();
    delay(100);
}
```

---

*The TTGO T-Camera provides an excellent balance of features for wildlife monitoring with its integrated display, comprehensive GPIO options, and compact form factor, making it ideal for standalone monitoring systems that require local status display and user interaction capabilities.*