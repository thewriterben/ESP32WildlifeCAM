# M5Stack Timer Camera Deployment Guide

This comprehensive guide provides step-by-step instructions for deploying the M5Stack Timer Camera for wildlife monitoring applications. The M5Stack Timer Camera is a compact, portable camera module with built-in battery and timer functionality, making it ideal for remote wildlife monitoring, time-lapse photography, and mobile research deployments.

## Prerequisites

### Hardware Requirements
- **M5Stack Timer Camera** (with built-in battery)
- **USB-C cable** for programming and charging
- **Computer** with Windows, macOS, or Linux
- **MicroSD card** (16-64GB, Class 10 recommended)
- **PIR motion sensor** (HC-SR501 or similar)
- **External power source** (optional, for extended operation)

### Software Requirements
- **Arduino IDE 2.0+** or **PlatformIO**
- **ESP32 board package** for Arduino IDE
- **Required libraries:**
  - ESP32 Camera Driver
  - WiFi library (built-in)
  - SD library (built-in)
  - ArduinoJson (for configuration)
  - M5Stack library (for hardware integration)

### Additional Components
- **Protective case** (for harsh environments)
- **Tripod or mounting hardware**
- **External battery pack** (for extended deployments)
- **Solar panel** (optional, for sustainable power)

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
     - **M5Stack** by M5Stack
     - **ArduinoJson** by Benoit Blanchon

### 2. Connecting the Board

1. **Connect via USB-C:**
   - Use the included USB-C cable to connect the Timer Camera to your computer
   - The device will power on automatically when connected

2. **Select board and port in Arduino IDE:**
   - Go to `Tools` > `Board` > `ESP32 Arduino`
   - Select **"M5Stack-Timer-CAM"** or **"ESP32 Dev Module"**
   - Go to `Tools` > `Port` and select the appropriate COM port
   - Set **Upload Speed** to **921600**

3. **Configure board settings:**
   - **CPU Frequency:** 240MHz
   - **Flash Mode:** QIO
   - **Flash Size:** 4MB
   - **Partition Scheme:** Default 4MB with SPIFFS
   - **PSRAM:** Disabled (not available on this model)

### 3. Writing the Code

1. **Create a new sketch** in Arduino IDE

2. **Basic camera initialization code:**
   ```cpp
   #include "esp_camera.h"
   #include "WiFi.h"
   #include "SD_MMC.h"
   #include "M5TimerCAM.h"
   
   // M5Stack Timer Camera pin definitions
   #define PWDN_GPIO_NUM    -1  // Not connected
   #define RESET_GPIO_NUM   15
   #define XCLK_GPIO_NUM    27
   #define SIOD_GPIO_NUM    25  // SDA
   #define SIOC_GPIO_NUM    23  // SCL
   
   #define Y9_GPIO_NUM      19
   #define Y8_GPIO_NUM      36
   #define Y7_GPIO_NUM      18
   #define Y6_GPIO_NUM      39
   #define Y5_GPIO_NUM       5
   #define Y4_GPIO_NUM      34
   #define Y3_GPIO_NUM      35
   #define Y2_GPIO_NUM      32
   #define VSYNC_GPIO_NUM   22
   #define HREF_GPIO_NUM    26
   #define PCLK_GPIO_NUM    21
   
   #define LED_GPIO_NUM      2  // Status LED
   #define FLASH_GPIO_NUM   14  // Flash LED
   
   void setup() {
     Serial.begin(115200);
     
     // Initialize M5Stack Timer Camera
     TimerCAM.begin();
     
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
     config.fb_count = 1;                 // Single frame buffer
     config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
     
     // Initialize camera
     esp_err_t err = esp_camera_init(&config);
     if (err != ESP_OK) {
       Serial.printf("Camera init failed with error 0x%x", err);
       return;
     }
     
     Serial.println("M5Stack Timer Camera initialized successfully");
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

> **⚠️ Note:** The Timer Camera does not have PSRAM, so avoid high frame buffer counts

### 5. Uploading the Code

1. **Click the upload button** (arrow) in Arduino IDE
2. **Monitor the output console** for upload progress
3. **Wait for "Hard resetting via RTS pin"** message
4. **The device will automatically reset** and start running your code

### 6. Testing the Deployment

1. **Open Serial Monitor** (`Tools` > `Serial Monitor`)
2. **Set baud rate to 115200**
3. **Press the reset button** on the Timer Camera
4. **Verify initialization messages** appear
5. **Test camera functionality** by taking a test photo

## Hardware Specifications

### Board Specifications
- **Microcontroller:** ESP32-DOWDQ6 (Dual-core Xtensa LX6, 240MHz)
- **Flash Memory:** 4MB SPI Flash
- **PSRAM:** Not available
- **WiFi:** 802.11 b/g/n (2.4GHz)
- **Bluetooth:** Bluetooth 4.2 Classic and BLE
- **USB:** USB-C connector for programming and charging
- **Battery:** Built-in 120mAh Li-Po battery
- **Operating Voltage:** 3.7V (battery) / 5V (USB)
- **Dimensions:** 48mm × 24mm × 12mm
- **Weight:** 15g

### Camera Specifications
- **Sensor:** OV3660 3-megapixel CMOS sensor
- **Resolution:** Up to 2048×1536 (QXGA)
- **Image Formats:** JPEG, RGB565, YUV422
- **Frame Rates:** Up to 60fps at VGA, 15fps at QXGA
- **Features:** Auto exposure, auto white balance, auto gain control
- **Field of View:** ~66° diagonal
- **Low Light Performance:** Good performance with built-in flash

### Memory and Storage
- **RAM:** 520KB SRAM (no PSRAM)
- **Flash:** 4MB for program storage
- **External Storage:** MicroSD card slot (up to 64GB)
- **Limitations:** Limited RAM requires careful memory management

### Connectivity Options
- **WiFi:** Built-in with PCB antenna
- **USB:** USB-C for programming and charging
- **GPIO:** Limited GPIO pins available due to compact design
- **I2C:** Available for external sensors
- **Battery Management:** Built-in charging circuit

### Power Requirements
- **Operating Current:** 90mA (active), 5mA (sleep)
- **Camera Current:** 150mA during capture
- **Battery Life:** 2-4 hours active use, 20+ hours standby
- **Charging Current:** 200mA via USB-C
- **Solar Compatibility:** 5V solar panel via USB-C

## Pin Configuration

### Camera Pin Mapping
```cpp
// M5Stack Timer Camera Pins
#define PWDN_GPIO_NUM    -1    // Power down (not connected)
#define RESET_GPIO_NUM   15    // Camera reset
#define XCLK_GPIO_NUM    27    // Camera clock
#define SIOD_GPIO_NUM    25    // Camera I2C data (SDA)
#define SIOC_GPIO_NUM    23    // Camera I2C clock (SCL)

// Camera data pins
#define Y9_GPIO_NUM      19    // Data bit 9
#define Y8_GPIO_NUM      36    // Data bit 8
#define Y7_GPIO_NUM      18    // Data bit 7
#define Y6_GPIO_NUM      39    // Data bit 6
#define Y5_GPIO_NUM       5    // Data bit 5
#define Y4_GPIO_NUM      34    // Data bit 4
#define Y3_GPIO_NUM      35    // Data bit 3
#define Y2_GPIO_NUM      32    // Data bit 2
#define VSYNC_GPIO_NUM   22    // Vertical sync
#define HREF_GPIO_NUM    26    // Horizontal reference
#define PCLK_GPIO_NUM    21    // Pixel clock

// Built-in peripherals
#define LED_GPIO_NUM      2    // Status LED
#define FLASH_GPIO_NUM   14    // Flash LED
#define BUTTON_PIN        0    // Built-in button
#define BAT_ADC_PIN      38    // Battery voltage monitoring
```

### Available GPIO for External Sensors
```cpp
// Limited available pins due to compact design
#define AVAILABLE_GPIO_PINS {0, 4, 12, 13, 16, 17}

// Recommended sensor pin assignments
#define PIR_SENSOR_PIN    4    // PIR motion sensor
#define EXT_LED_PIN      12    // External status LED
#define I2C_SDA_EXT      13    // External I2C SDA
#define I2C_SCL_EXT      16    // External I2C SCL
#define ANALOG_SENSOR    17    // Analog sensor input
```

### Pin Conflicts and Limitations
```cpp
// Critical pin usage for M5Stack Timer Camera
struct PinConflicts {
    // Camera pins - DO NOT USE for other purposes
    int camera_pins[] = {5, 15, 18, 19, 21, 22, 23, 25, 26, 27, 32, 34, 35, 36, 39};
    
    // Built-in peripherals
    int builtin_pins[] = {0, 2, 14, 38};  // Button, LED, Flash, Battery ADC
    
    // Available for external use
    int available_pins[] = {4, 12, 13, 16, 17};
    
    // Input-only pins
    int input_only_pins[] = {34, 35, 36, 38, 39};
    
    // Limited GPIO warning
    bool gpio_limited = true;  // Very few pins available
    
    // Recommendations for minimal setup
    struct {
        int pir_sensor = 4;        // Motion detection
        int external_led = 12;     // Status indicator
        int i2c_sda = 13;         // I2C sensors
        int i2c_scl = 16;         // I2C sensors
    } minimal_config;
};
```

## Board-Specific Configuration

### Built-in Battery Management
The Timer Camera includes battery management features:

```cpp
#include "driver/adc.h"

void setupBatteryMonitoring() {
    // Configure ADC for battery voltage reading
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_2, ADC_ATTEN_DB_11);  // GPIO 38
    
    Serial.println("Battery monitoring configured");
}

float readBatteryVoltage() {
    int raw_reading = adc1_get_raw(ADC1_CHANNEL_2);  // GPIO 38
    
    // Convert to voltage (calibration may vary by unit)
    float voltage = (raw_reading * 3.3 / 4095.0) * 2.0;  // Voltage divider factor
    
    return voltage;
}

int getBatteryPercentage() {
    float voltage = readBatteryVoltage();
    
    // Li-Po battery voltage curve (approximate)
    if (voltage >= 4.1) return 100;
    if (voltage >= 3.9) return 75;
    if (voltage >= 3.7) return 50;
    if (voltage >= 3.5) return 25;
    if (voltage >= 3.3) return 10;
    return 0;
}

void checkBatteryStatus() {
    float voltage = readBatteryVoltage();
    int percentage = getBatteryPercentage();
    
    Serial.printf("Battery: %.2fV (%d%%)\n", voltage, percentage);
    
    if (percentage < 20) {
        Serial.println("WARNING: Low battery!");
        // Flash LED to indicate low battery
        for (int i = 0; i < 5; i++) {
            digitalWrite(LED_GPIO_NUM, HIGH);
            delay(100);
            digitalWrite(LED_GPIO_NUM, LOW);
            delay(100);
        }
    }
}
```

### Timer and Sleep Functions
Optimize for battery life with timer functions:

```cpp
#include "esp_sleep.h"
#include "esp_timer.h"

// Timer callback for periodic capture
void timerCallback(void* arg) {
    static bool capture_flag = false;
    capture_flag = true;
    
    // Signal main loop to take photo
    xTaskNotifyGive((TaskHandle_t)arg);
}

void setupTimerCapture(uint32_t interval_seconds) {
    // Create periodic timer
    esp_timer_create_args_t timer_args = {
        .callback = timerCallback,
        .arg = xTaskGetCurrentTaskHandle(),
        .name = "capture_timer"
    };
    
    esp_timer_handle_t timer_handle;
    esp_timer_create(&timer_args, &timer_handle);
    
    // Start timer
    esp_timer_start_periodic(timer_handle, interval_seconds * 1000000);  // microseconds
    
    Serial.printf("Timer capture set for every %d seconds\n", interval_seconds);
}

void enterLowPowerMode() {
    // Configure wake-up sources
    esp_sleep_enable_timer_wakeup(60 * 1000000);  // Wake every minute
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);  // Wake on button press
    
    // Power down peripherals
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    Serial.println("Entering low power mode...");
    Serial.flush();
    
    // Enter light sleep (maintains timer functionality)
    esp_light_sleep_start();
}

void smartPowerManagement() {
    static unsigned long last_activity = millis();
    const unsigned long IDLE_TIMEOUT = 30000;  // 30 seconds
    
    if (millis() - last_activity > IDLE_TIMEOUT) {
        int battery_level = getBatteryPercentage();
        
        if (battery_level < 30) {
            // Low battery - enter deep sleep
            Serial.println("Low battery - entering deep sleep");
            esp_deep_sleep_start();
        } else {
            // Normal battery - light sleep
            enterLowPowerMode();
            last_activity = millis();
        }
    }
}
```

### Flash and LED Control
Control built-in LEDs for status indication:

```cpp
void setupLEDs() {
    pinMode(LED_GPIO_NUM, OUTPUT);
    pinMode(FLASH_GPIO_NUM, OUTPUT);
    
    // Initial LED test
    digitalWrite(LED_GPIO_NUM, HIGH);
    delay(500);
    digitalWrite(LED_GPIO_NUM, LOW);
    
    Serial.println("LEDs configured");
}

void flashLED(int count, int duration_ms = 200) {
    for (int i = 0; i < count; i++) {
        digitalWrite(FLASH_GPIO_NUM, HIGH);
        delay(duration_ms);
        digitalWrite(FLASH_GPIO_NUM, LOW);
        delay(duration_ms);
    }
}

void statusBlink(int pattern) {
    switch (pattern) {
        case 1:  // Single blink - normal operation
            digitalWrite(LED_GPIO_NUM, HIGH);
            delay(100);
            digitalWrite(LED_GPIO_NUM, LOW);
            break;
            
        case 2:  // Double blink - photo captured
            for (int i = 0; i < 2; i++) {
                digitalWrite(LED_GPIO_NUM, HIGH);
                delay(100);
                digitalWrite(LED_GPIO_NUM, LOW);
                delay(100);
            }
            break;
            
        case 3:  // Triple blink - error condition
            for (int i = 0; i < 3; i++) {
                digitalWrite(LED_GPIO_NUM, HIGH);
                delay(200);
                digitalWrite(LED_GPIO_NUM, LOW);
                delay(200);
            }
            break;
    }
}
```

## Troubleshooting Section

### Common Issues and Solutions

#### Battery and Power Issues
**Symptoms:**
- Device not powering on
- Rapid battery drain
- Charging not working

**Solutions:**
1. **Battery diagnostics:**
   ```cpp
   void diagnoseBattery() {
       float voltage = readBatteryVoltage();
       int percentage = getBatteryPercentage();
       
       Serial.println("=== Battery Diagnostics ===");
       Serial.printf("Voltage: %.2fV\n", voltage);
       Serial.printf("Percentage: %d%%\n", percentage);
       
       if (voltage < 3.0) {
           Serial.println("ERROR: Battery critically low or disconnected");
       } else if (voltage < 3.5) {
           Serial.println("WARNING: Battery low - charge required");
       } else {
           Serial.println("Battery OK");
       }
       
       // Test charging detection
       // (Implementation depends on charging circuit design)
       Serial.println("===========================");
   }
   ```

2. **Power optimization:**
   ```cpp
   void optimizePowerConsumption() {
       // Reduce CPU frequency
       setCpuFrequencyMhz(80);  // Reduce from 240MHz
       
       // Disable unused peripherals
       esp_wifi_stop();
       esp_bt_controller_disable();
       
       // Use light sleep between operations
       esp_sleep_enable_timer_wakeup(1000000);  // 1 second
       
       Serial.println("Power optimization applied");
   }
   ```

#### Memory Limitations
**Symptoms:**
- Out of memory errors
- Camera initialization failures
- Image capture failures

**Solutions:**
1. **Memory optimization:**
   ```cpp
   void optimizeMemoryUsage() {
       // Check available memory
       Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
       Serial.printf("Largest free block: %d bytes\n", 
                    heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
       
       // Configure camera for low memory usage
       sensor_t * s = esp_camera_sensor_get();
       if (s) {
           // Reduce frame size if memory is low
           if (ESP.getFreeHeap() < 100000) {  // Less than 100KB
               s->set_framesize(s, FRAMESIZE_SVGA);  // 800x600
               Serial.println("Reduced frame size due to low memory");
           }
           
           // Optimize quality vs memory usage
           s->set_quality(s, 15);  // Higher compression
       }
   }
   ```

#### Camera Issues Specific to Timer Camera
**Symptoms:**
- Poor image quality
- Capture failures
- Flash not working

**Solutions:**
1. **Camera optimization:**
   ```cpp
   void optimizeTimerCamera() {
       sensor_t * s = esp_camera_sensor_get();
       
       // OV3660-specific optimizations
       s->set_brightness(s, 0);     // Neutral brightness
       s->set_contrast(s, 1);       // Slight contrast boost
       s->set_saturation(s, 0);     // Natural colors
       s->set_special_effect(s, 0); // No effects
       s->set_whitebal(s, 1);       // Auto white balance
       s->set_awb_gain(s, 1);       // Auto white balance gain
       s->set_wb_mode(s, 0);        // Auto white balance mode
       s->set_exposure_ctrl(s, 1);  // Auto exposure
       s->set_aec2(s, 1);           // AEC sensor
       s->set_ae_level(s, 0);       // Auto exposure level
       s->set_aec_value(s, 300);    // Exposure value
       s->set_gain_ctrl(s, 1);      // Auto gain
       s->set_agc_gain(s, 0);       // Auto gain value
       s->set_gainceiling(s, (gainceiling_t)2); // Gain ceiling
       s->set_bpc(s, 0);            // Black pixel correction
       s->set_wpc(s, 1);            // White pixel correction
       s->set_raw_gma(s, 1);        // Raw gamma
       s->set_lenc(s, 1);           // Lens correction
       s->set_hmirror(s, 0);        // Horizontal mirror
       s->set_vflip(s, 0);          // Vertical flip
       s->set_dcw(s, 1);            // DCW enable
       s->set_colorbar(s, 0);       // Disable color bar
       
       Serial.println("Timer Camera optimizations applied");
   }
   ```

### Debug Tips

#### Timer Camera Specific Debugging
```cpp
void debugTimerCamera() {
    Serial.println("=== M5Stack Timer Camera Debug ===");
    
    // Check camera sensor
    sensor_t * s = esp_camera_sensor_get();
    if (s) {
        sensor_id_t id = s->id;
        Serial.printf("Sensor PID: 0x%02X\n", id.PID);
        
        if (id.PID == OV3660_PID) {
            Serial.println("OV3660 sensor detected (correct)");
        } else {
            Serial.printf("Unexpected sensor PID: 0x%02X\n", id.PID);
        }
    } else {
        Serial.println("Camera sensor not detected");
    }
    
    // Check memory
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    
    // Check battery
    checkBatteryStatus();
    
    // Test LEDs
    Serial.println("Testing LEDs...");
    statusBlink(1);
    flashLED(1);
    
    Serial.println("===================================");
}
```

## Example Code Snippets

### Time-Lapse Wildlife Photography
```cpp
#include "M5TimerCAM.h"
#include "time.h"

class TimeLapseController {
private:
    unsigned long last_capture;
    unsigned long capture_interval;
    int photo_count;
    bool time_lapse_active;
    
public:
    TimeLapseController(unsigned long interval_ms) : 
        capture_interval(interval_ms), 
        last_capture(0), 
        photo_count(0), 
        time_lapse_active(false) {}
    
    void start() {
        time_lapse_active = true;
        last_capture = millis();
        photo_count = 0;
        
        Serial.printf("Time-lapse started: %lu ms interval\n", capture_interval);
        statusBlink(2);  // Double blink to indicate start
    }
    
    void stop() {
        time_lapse_active = false;
        Serial.printf("Time-lapse stopped: %d photos captured\n", photo_count);
        statusBlink(3);  // Triple blink to indicate stop
    }
    
    void update() {
        if (!time_lapse_active) return;
        
        unsigned long current_time = millis();
        if (current_time - last_capture >= capture_interval) {
            captureTimeLapsePhoto();
            last_capture = current_time;
        }
        
        // Check battery and stop if too low
        if (getBatteryPercentage() < 15) {
            Serial.println("Battery too low for time-lapse");
            stop();
        }
    }
    
private:
    void captureTimeLapsePhoto() {
        // Flash LED briefly to indicate capture
        digitalWrite(LED_GPIO_NUM, HIGH);
        
        camera_fb_t * fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Time-lapse capture failed");
            digitalWrite(LED_GPIO_NUM, LOW);
            return;
        }
        
        // Generate filename with timestamp
        String filename = "/timelapse_" + String(photo_count) + "_" + String(millis()) + ".jpg";
        
        // Save to SD card
        File file = SD_MMC.open(filename, FILE_WRITE);
        if (file) {
            file.write(fb->buf, fb->len);
            file.close();
            
            photo_count++;
            Serial.printf("Time-lapse photo %d saved: %s\n", photo_count, filename.c_str());
        } else {
            Serial.println("Failed to save time-lapse photo");
        }
        
        esp_camera_fb_return(fb);
        digitalWrite(LED_GPIO_NUM, LOW);
        
        // Brief status LED blink
        statusBlink(1);
    }
};

TimeLapseController timelapse(30000);  // 30 second interval

void setup() {
    Serial.begin(115200);
    TimerCAM.begin();
    
    if (!initializeCamera()) {
        Serial.println("Camera initialization failed!");
        return;
    }
    
    if (!SD_MMC.begin()) {
        Serial.println("SD card initialization failed!");
        return;
    }
    
    setupBatteryMonitoring();
    setupLEDs();
    
    // Start time-lapse after 5 second delay
    delay(5000);
    timelapse.start();
}

void loop() {
    timelapse.update();
    
    // Check for button press to stop/start
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(50);  // debounce
        if (digitalRead(BUTTON_PIN) == LOW) {
            // Toggle time-lapse
            static bool toggle = false;
            if (toggle) {
                timelapse.stop();
            } else {
                timelapse.start();
            }
            toggle = !toggle;
            
            // Wait for button release
            while (digitalRead(BUTTON_PIN) == LOW) {
                delay(10);
            }
        }
    }
    
    // Power management
    smartPowerManagement();
    
    delay(100);
}
```

### Motion-Triggered Portable Camera
```cpp
#include "M5TimerCAM.h"

#define PIR_PIN 4

class PortableWildlifeCamera {
private:
    bool motion_mode_active;
    unsigned long last_motion;
    const unsigned long MOTION_COOLDOWN = 10000;  // 10 seconds
    int captures_today;
    
public:
    PortableWildlifeCamera() : 
        motion_mode_active(false), 
        last_motion(0), 
        captures_today(0) {}
    
    void begin() {
        pinMode(PIR_PIN, INPUT);
        pinMode(BUTTON_PIN, INPUT_PULLUP);
        
        // Configure interrupt for motion detection
        attachInterrupt(digitalPinToInterrupt(PIR_PIN), []() {
            // Simple interrupt handler - set flag in main loop
        }, RISING);
        
        Serial.println("Portable wildlife camera ready");
        statusBlink(1);
    }
    
    void update() {
        // Check for motion
        if (digitalRead(PIR_PIN) == HIGH && motion_mode_active) {
            unsigned long current_time = millis();
            if (current_time - last_motion > MOTION_COOLDOWN) {
                handleMotionDetection();
                last_motion = current_time;
            }
        }
        
        // Check button for mode toggle
        if (digitalRead(BUTTON_PIN) == LOW) {
            delay(50);
            if (digitalRead(BUTTON_PIN) == LOW) {
                toggleMotionMode();
                while (digitalRead(BUTTON_PIN) == LOW) delay(10);
            }
        }
        
        // Battery conservation
        if (getBatteryPercentage() < 20) {
            if (motion_mode_active) {
                Serial.println("Low battery - disabling motion mode");
                motion_mode_active = false;
                statusBlink(3);
            }
        }
    }
    
private:
    void toggleMotionMode() {
        motion_mode_active = !motion_mode_active;
        
        if (motion_mode_active) {
            Serial.println("Motion detection mode ON");
            statusBlink(2);  // Double blink for ON
        } else {
            Serial.println("Motion detection mode OFF");
            statusBlink(1);  // Single blink for OFF
        }
    }
    
    void handleMotionDetection() {
        Serial.println("Motion detected! Capturing wildlife...");
        
        // Use flash if available (check light conditions first)
        bool use_flash = shouldUseFlash();
        
        if (use_flash) {
            digitalWrite(FLASH_GPIO_NUM, HIGH);
            delay(100);  // Pre-flash delay
        }
        
        // Capture sequence
        for (int i = 0; i < 3; i++) {
            camera_fb_t * fb = esp_camera_fb_get();
            if (fb) {
                String filename = "/motion_" + String(captures_today) + "_" + 
                                String(i) + "_" + String(millis()) + ".jpg";
                
                File file = SD_MMC.open(filename, FILE_WRITE);
                if (file) {
                    file.write(fb->buf, fb->len);
                    file.close();
                    Serial.printf("Motion photo saved: %s\n", filename.c_str());
                }
                
                esp_camera_fb_return(fb);
            }
            
            if (i < 2) delay(1000);  // 1 second between shots
        }
        
        if (use_flash) {
            digitalWrite(FLASH_GPIO_NUM, LOW);
        }
        
        captures_today++;
        statusBlink(2);  // Indicate successful capture
        
        // Log event with battery status
        Serial.printf("Motion event %d complete. Battery: %d%%\n", 
                     captures_today, getBatteryPercentage());
    }
    
    bool shouldUseFlash() {
        // Simple light detection using camera
        sensor_t * s = esp_camera_sensor_get();
        if (!s) return false;
        
        // Take a quick test shot to check brightness
        s->set_framesize(s, FRAMESIZE_QQVGA);  // Very small for speed
        camera_fb_t * fb = esp_camera_fb_get();
        
        if (!fb) return false;
        
        // Calculate average brightness
        uint32_t total_brightness = 0;
        for (int i = 0; i < fb->len; i += 10) {
            total_brightness += fb->buf[i];
        }
        uint32_t avg_brightness = total_brightness / (fb->len / 10);
        
        esp_camera_fb_return(fb);
        
        // Restore normal frame size
        s->set_framesize(s, FRAMESIZE_UXGA);
        
        // Use flash if average brightness is low
        return avg_brightness < 50;
    }
};

PortableWildlifeCamera wildlife_cam;

void setup() {
    Serial.begin(115200);
    TimerCAM.begin();
    
    if (!initializeCamera()) {
        Serial.println("Camera initialization failed!");
        return;
    }
    
    if (!SD_MMC.begin()) {
        Serial.println("SD card initialization failed!");
        return;
    }
    
    setupBatteryMonitoring();
    setupLEDs();
    optimizeTimerCamera();
    
    wildlife_cam.begin();
    
    Serial.printf("System ready. Battery: %d%%\n", getBatteryPercentage());
}

void loop() {
    wildlife_cam.update();
    
    // Check battery every minute
    static unsigned long last_battery_check = 0;
    if (millis() - last_battery_check > 60000) {
        checkBatteryStatus();
        last_battery_check = millis();
    }
    
    delay(100);
}
```

---

*The M5Stack Timer Camera provides a compact, portable solution for wildlife monitoring with built-in battery management and timer functionality, making it ideal for remote deployments and mobile research applications where size and power efficiency are critical.*