# Deployment Guide for LilyGO T-Camera Plus S3

This guide provides comprehensive step-by-step instructions for deploying the LilyGO T-Camera Plus S3. It can also serve as a template for deploying other similar boards.

## Prerequisites

### Hardware Requirements
- **LilyGO T-Camera Plus S3 board** with ESP32-S3 chip and built-in display
- **USB-C cable** for programming and power (NOT micro-USB)
- **Computer** with Windows, macOS, or Linux
- **MicroSD Card** (32GB or smaller, Class 10 recommended) for image storage
- **Optional external antenna** (2.4GHz) for improved WiFi/Bluetooth range

### Software Requirements
- **Arduino IDE** (version 2.0 or newer recommended)
- **ESP32 Board Package** (version 2.0.11 or newer for ESP32-S3 support)
- **Required Libraries:**
  - ESP32 Camera library (included with ESP32 board package)
  - TFT_eSPI library (for display management)
  - WiFi library (included with ESP32 board package)
  - SD library (for microSD card operations)
  - ArduinoJson library (for configuration management)

### Additional Components (Optional)
- **PIR Motion Sensor** (HC-SR501) for motion detection
- **Environmental Sensors** (DHT22, BME280) for environmental monitoring
- **LoRa Module** (SX1276/SX1278) for long-range communication
- **External GPS Module** for location tracking
- **Power Supply Components:**
  - 6W Solar panel (higher capacity for display power requirements)
  - Li-ion battery pack (18650 x2 for extended operation)
  - Battery management system (BMS)

## Step-by-Step Instructions

### 1. Setting Up the Arduino IDE

#### Install or Update Arduino IDE
1. Download the latest Arduino IDE 2.x from [arduino.cc](https://www.arduino.cc/en/software)
2. Install following the platform-specific instructions
3. Launch Arduino IDE

#### Add ESP32-S3 Board Support
1. Open Arduino IDE
2. Go to **File** > **Preferences**
3. In the **Additional Boards Manager URLs** field, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click **OK** to save preferences
5. Go to **Tools** > **Board** > **Boards Manager**
6. Search for **"esp32"**
7. Install **"ESP32 by Espressif Systems"** (version 2.0.11 or newer)
8. Wait for installation to complete

#### Select LilyGO T-Camera Plus S3 Board
1. Go to **Tools** > **Board** > **ESP32 Arduino**
2. Select **"ESP32S3 Dev Module"**
3. Configure the following board settings:
   - **Upload Speed**: 921600
   - **CPU Frequency**: 240MHz (WiFi)
   - **Flash Mode**: QIO
   - **Flash Size**: 16MB (128Mb)
   - **Partition Scheme**: 16M Flash (3MB APP/9.9MB FATFS)
   - **Core Debug Level**: None
   - **PSRAM**: OPI PSRAM
   - **Arduino Runs On**: Core 1
   - **Events Run On**: Core 1

#### Install Required Libraries
1. Go to **Tools** > **Manage Libraries**
2. Install the following libraries:
   - **TFT_eSPI** by Bodmer (for display control)
   - **ArduinoJson** by Benoit Blanchon (for JSON handling)
   - **OneWire** and **DallasTemperature** (if using temperature sensors)

### 2. Connecting the Board

> ✅ **Advantage**: The LilyGO T-Camera Plus S3 has a built-in USB-C connector for easy programming!

#### Direct USB Connection
1. **Connect** the LilyGO T-Camera Plus S3 to your computer using a USB-C cable
2. **Wait** for the device to be recognized by your operating system
3. In Arduino IDE, go to **Tools** > **Port** and select the correct port:
   - **Windows**: Usually COM3, COM4, etc.
   - **macOS**: Usually /dev/cu.usbmodem* or /dev/cu.SLAB_USBtoUART
   - **Linux**: Usually /dev/ttyUSB0 or /dev/ttyACM0

#### Verify Connection
1. Open **Tools** > **Serial Monitor**
2. Set baud rate to **115200**
3. Press the **Reset button** on the T-Camera Plus S3
4. You should see boot messages and the built-in display should show activity

### 3. Writing the Code

#### Basic Camera and Display Test Code
1. Create a new sketch in Arduino IDE (**File** > **New**)
2. Replace the default code with this comprehensive test:

```cpp
#include "esp_camera.h"
#include "WiFi.h"
#include "SD_MMC.h"
#include <TFT_eSPI.h>

// LilyGO T-Camera Plus S3 pin definitions
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     40
#define SIOD_GPIO_NUM     17  // SDA
#define SIOC_GPIO_NUM     18  // SCL
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

// Display and other pins
#define TFT_CS_PIN        12
#define TFT_DC_PIN        13
#define TFT_RST_PIN       14
#define TFT_BL_PIN        15
#define SD_CS_PIN         10

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  Serial.println("LilyGO T-Camera Plus S3 Test");
  
  // Initialize display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.drawString("T-Camera Plus S3", 10, 10);
  tft.drawString("Initializing...", 10, 40);
  
  // Initialize SD card
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD Card Mount Failed");
    tft.setTextColor(TFT_RED);
    tft.drawString("SD Failed", 10, 70);
  } else {
    Serial.println("SD Card initialized");
    tft.setTextColor(TFT_GREEN);
    tft.drawString("SD OK", 10, 70);
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
  config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
  config.jpeg_quality = 10;
  config.fb_count = 2;
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    tft.setTextColor(TFT_RED);
    tft.drawString("Camera Failed", 10, 100);
    return;
  }
  
  Serial.println("Camera initialized successfully!");
  tft.setTextColor(TFT_GREEN);
  tft.drawString("Camera OK", 10, 100);
  
  // Display system info
  tft.setTextSize(1);
  tft.drawString("ESP32-S3 240MHz", 10, 130);
  tft.drawString("16MB Flash", 10, 150);
  tft.drawString("8MB PSRAM", 10, 170);
  tft.drawString("Ready for capture", 10, 190);
}

void loop() {
  // Take a picture
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  
  Serial.printf("Picture taken! Size: %d bytes\n", fb->len);
  
  // Save to SD card
  String filename = "/capture_" + String(millis()) + ".jpg";
  File file = SD_MMC.open(filename, FILE_WRITE);
  if (file) {
    file.write(fb->buf, fb->len);
    file.close();
    Serial.println("Image saved to SD card: " + filename);
    
    // Update display
    tft.fillRect(10, 210, 220, 20, TFT_BLACK);
    tft.setTextColor(TFT_CYAN);
    tft.drawString("Saved: " + filename.substring(1), 10, 210);
  } else {
    Serial.println("Failed to save image");
  }
  
  esp_camera_fb_return(fb);
  
  delay(10000); // Wait 10 seconds before next picture
}
```

3. Save the sketch with a descriptive name (e.g., "LilyGO_T_Camera_Plus_S3_Test")

### 4. Compiling the Code

1. Click the **Verify** button (✓) in the Arduino IDE toolbar
2. Wait for compilation to complete (may take 1-2 minutes for first compile)
3. Check for any error messages in the output window
4. If compilation fails:
   - Verify ESP32-S3 board selection and settings
   - Ensure all required libraries are installed
   - Check that partition scheme allows sufficient space

### 5. Uploading the Code

> ⚠️ **Note**: If upload fails, hold the BOOT button while clicking upload, then release when upload begins

1. **Verify all connections** and board settings
2. **Click Upload** button (→) in Arduino IDE
3. **Monitor upload progress** in the output window
4. **If upload stalls**: Hold BOOT button and press RESET, then try upload again
5. **Wait for "Hard resetting via RTS pin..." message**

#### Troubleshooting Upload Issues
- **"Failed to connect"**: Hold BOOT button during upload
- **"Timed out waiting for packet header"**: Check USB cable and port selection
- **"Hash of data verified"**: Upload successful, board will restart automatically

### 6. Testing the Deployment

#### Verify Basic Functionality
1. **Display Test**: The built-in TFT display should show initialization status
2. **Serial Monitor**: Open Serial Monitor (115200 baud) to see detailed output
3. **SD Card Test**: Verify SD card is detected and accessible
4. **Camera Test**: Confirm camera initialization and image capture

#### Expected Output
- **Display**: Should show "T-Camera Plus S3", initialization status, and system info
- **Serial Monitor**: Should show successful initialization messages and image capture confirmations
- **SD Card**: Should contain captured JPEG images with timestamps

#### Advanced Testing
1. **Image Quality**: Check captured images on SD card for proper exposure and focus
2. **Display Functionality**: Verify TFT display updates and color rendering
3. **Memory Usage**: Monitor free heap and PSRAM usage
4. **WiFi Test**: Add WiFi connection code for network connectivity testing
5. **Power Consumption**: Test with battery power to verify power management

## Hardware Specifications

### Core Hardware Features
- **Microcontroller**: ESP32-S3 (240MHz dual-core Xtensa LX7)
- **Camera**: OV2640 2MP camera with adjustable focus lens
- **Memory**: 
  - 512KB SRAM
  - 8MB PSRAM (OPI PSRAM for high-speed access)
  - 16MB Flash memory
- **Connectivity**: 
  - WiFi 802.11 b/g/n (2.4GHz)
  - Bluetooth 5.0 LE
  - USB-C for programming and power
- **Display**: 1.14" TFT LCD (ST7789) - 240x240 pixels
- **Storage**: MicroSD card slot (supports up to 32GB)
- **Power**: 5V via USB-C or 3.7V battery connector
- **Dimensions**: 48mm x 24mm x 12mm
- **Operating Temperature**: -10°C to +65°C

### Camera Specifications
- **Sensor**: OV2640 CMOS sensor with auto-focus lens
- **Resolution**: Up to 1600x1200 (UXGA)
- **Formats**: JPEG, RGB565, YUV422, Grayscale
- **Features**: 
  - Auto exposure control
  - Auto white balance
  - Auto gain control
  - Manual focus adjustment
- **Frame Rate**: Up to 60fps at lower resolutions (QVGA)
- **Field of View**: ~66° diagonal
- **Minimum Focus Distance**: ~10cm
- **Low Light Performance**: Better than standard ESP32-CAM due to improved lens

### Memory and Storage Details
- **ESP32-S3 Internal Memory**:
  - 512KB SRAM for program execution
  - 8MB PSRAM for frame buffers and large data structures
  - 16MB Flash for firmware and file system
- **MicroSD Card Storage**:
  - Supports FAT32 file system
  - Maximum tested capacity: 32GB
  - Recommended: Class 10 or UHS-I for fast image writing
  - Interface: 4-bit SDMMC for high-speed access

### Connectivity Options
- **WiFi Capabilities**:
  - IEEE 802.11 b/g/n (2.4GHz)
  - WPA/WPA2/WPA3 security support
  - Station mode and Access Point mode
  - Maximum range: ~100m in open area
  - Data throughput: Up to 150Mbps
- **Bluetooth Features**:
  - Bluetooth 5.0 with LE support
  - Range: ~10m indoor, ~50m outdoor
  - Low energy consumption for IoT applications
- **USB-C Interface**:
  - USB 2.0 Full Speed (12Mbps)
  - Power delivery up to 5V 2A
  - Automatic programming mode detection

### Power Requirements
- **Operating Voltage**: 3.0V - 3.6V (internal), 5V via USB-C
- **Current Consumption**:
  - Active (camera + WiFi + display): ~280mA
  - Active (camera only): ~180mA
  - WiFi connected (idle): ~80mA
  - Deep sleep: ~15µA
  - Display backlight: ~50mA (adjustable)
- **Power Sources**:
  - USB-C connector (5V input with onboard regulator)
  - JST-PH 2.0 connector for 3.7V Li-ion battery
  - Solar panel input via charge controller (recommended 6W+)
- **Battery Life** (3000mAh Li-ion):
  - Continuous operation: 8-12 hours
  - Motion-triggered mode: 7-14 days
  - Deep sleep with periodic wake: 30-60 days

## Pin Configuration

### Standard GPIO Map
```cpp
// LilyGO T-Camera Plus S3 Pin Definitions
#define PWDN_GPIO_NUM     -1   // Not connected
#define RESET_GPIO_NUM    -1   // Not connected
#define XCLK_GPIO_NUM     40   // Camera master clock
#define SIOD_GPIO_NUM     17   // Camera SDA (I2C data)
#define SIOC_GPIO_NUM     18   // Camera SCL (I2C clock)

// Camera data pins (8-bit parallel interface)
#define Y9_GPIO_NUM       39   // Camera data bit 7 (MSB)
#define Y8_GPIO_NUM       41   // Camera data bit 6
#define Y7_GPIO_NUM       42   // Camera data bit 5
#define Y6_GPIO_NUM       12   // Camera data bit 4
#define Y5_GPIO_NUM       3    // Camera data bit 3
#define Y4_GPIO_NUM       14   // Camera data bit 2
#define Y3_GPIO_NUM       47   // Camera data bit 1
#define Y2_GPIO_NUM       13   // Camera data bit 0 (LSB)
#define VSYNC_GPIO_NUM    21   // Vertical sync
#define HREF_GPIO_NUM     38   // Horizontal reference
#define PCLK_GPIO_NUM     11   // Pixel clock

// Built-in peripherals
#define TFT_CS_PIN        12   // Display chip select
#define TFT_DC_PIN        13   // Display data/command
#define TFT_RST_PIN       14   // Display reset
#define TFT_BL_PIN        15   // Display backlight control
#define SD_CS_PIN         10   // SD card chip select
#define BATTERY_ADC_PIN   1    // Battery voltage monitoring
```

### Available GPIO for External Sensors
```cpp
// Free GPIO pins for external sensors and peripherals
#define AVAILABLE_GPIOS {2, 4, 5, 6, 7, 8, 9, 16, 35, 36, 37, 45, 46, 48}

// Recommended pin assignments for common sensors
#define PIR_SENSOR_PIN    2    // PIR motion sensor input
#define DHT_SENSOR_PIN    4    // DHT22 temperature/humidity sensor
#define SOIL_SENSOR_PIN   5    // Soil moisture sensor (analog)
#define LIGHT_SENSOR_PIN  6    // Light level sensor (analog)
#define BUZZER_PIN        7    // Buzzer/speaker output
#define STATUS_LED_PIN    8    // External status LED
#define BUTTON_PIN        9    // User input button

// I2C expansion (additional sensors)
#define I2C_SDA_EXT       35   // Extended I2C SDA for sensors
#define I2C_SCL_EXT       36   // Extended I2C SCL for sensors

// UART expansion (GPS, LoRa, etc.)
#define UART_TX_PIN       37   // UART TX for external modules
#define UART_RX_PIN       45   // UART RX for external modules

// SPI expansion (LoRa, additional storage, etc.)
#define SPI_MISO_PIN      46   // SPI MISO for external SPI devices
#define SPI_MOSI_PIN      48   // SPI MOSI for external SPI devices
#define SPI_SCK_PIN       16   // SPI clock for external SPI devices
```

### Pin Conflicts and Limitations
```cpp
// Critical pin usage and conflicts on T-Camera Plus S3
struct PinUsage {
    // Camera interface (DO NOT USE for other purposes)
    bool camera_pins_reserved = true;
    int camera_pin_count = 11;
    int camera_pins[] = {40, 17, 18, 39, 41, 42, 12, 3, 14, 47, 13, 21, 38, 11};
    
    // Display interface (can be disabled if not using display)
    bool display_pins_reserved = true;
    int display_pin_count = 4;
    int display_pins[] = {12, 13, 14, 15};  // Note: GPIO 12,13,14 shared with camera
    
    // SD card interface (can be disabled if not using SD storage)
    bool sd_card_pins_reserved = true;
    int sd_pin_count = 1;
    int sd_pins[] = {10};  // Only CS pin, CMD/CLK/DATA use dedicated SD pins
    
    // Conflicts to be aware of
    bool gpio12_conflict = true;  // Shared between camera Y6 and display CS
    bool gpio13_conflict = true;  // Shared between camera Y2 and display DC
    bool gpio14_conflict = true;  // Shared between camera Y4 and display RST
    
    // USB pins (DO NOT USE)
    bool usb_pins_reserved = true;
    int usb_pins[] = {19, 20};  // USB D+ and D-
};
```

### Recommended Pin Assignments for Common Sensors
```cpp
// Wildlife monitoring sensor configuration
struct WildlifeSensorPins {
    // Motion detection
    gpio_num_t pir_sensor_pin = GPIO_NUM_2;        // PIR motion sensor
    gpio_num_t radar_sensor_pin = GPIO_NUM_4;      // Microwave radar sensor (optional)
    
    // Environmental monitoring
    gpio_num_t dht_sensor_pin = GPIO_NUM_5;        // DHT22 temp/humidity
    gpio_num_t bme_sda_pin = GPIO_NUM_35;          // BME280 I2C SDA
    gpio_num_t bme_scl_pin = GPIO_NUM_36;          // BME280 I2C SCL
    gpio_num_t light_sensor_pin = GPIO_NUM_6;      // Light sensor (analog)
    
    // Power monitoring
    gpio_num_t battery_voltage_pin = GPIO_NUM_1;   // Battery voltage divider
    gpio_num_t solar_voltage_pin = GPIO_NUM_7;     // Solar panel voltage
    gpio_num_t charge_status_pin = GPIO_NUM_8;     // Charging status LED
    
    // Communication modules
    gpio_num_t lora_cs_pin = GPIO_NUM_16;          // LoRa chip select
    gpio_num_t lora_rst_pin = GPIO_NUM_46;         // LoRa reset
    gpio_num_t lora_dio0_pin = GPIO_NUM_48;        // LoRa DIO0 interrupt
    
    // GPS module
    gpio_num_t gps_tx_pin = GPIO_NUM_37;           // GPS UART TX
    gpio_num_t gps_rx_pin = GPIO_NUM_45;           // GPS UART RX
    
    // User interface
    gpio_num_t status_led_pin = GPIO_NUM_9;        // Status indicator LED
    gpio_num_t buzzer_pin = GPIO_NUM_7;            // Audio feedback (optional)
    
    // Advanced features
    gpio_num_t external_trigger_pin = GPIO_NUM_2;  // External camera trigger
    gpio_num_t infrared_led_pin = GPIO_NUM_4;      // IR illumination control
};
```

### Power and Ground Connections
```cpp
// Power distribution for T-Camera Plus S3
struct PowerConnections {
    // Main power inputs
    String usb_power = "USB-C connector (5V)";
    String battery_power = "JST-PH 2.0 connector (3.7V)";
    
    // Power outputs for sensors
    float gpio_3v3_max_current_ma = 500;     // 3.3V GPIO max current
    float total_3v3_max_current_ma = 1000;   // Total 3.3V rail capacity
    
    // Ground connections
    int ground_pins_available = 4;           // Multiple ground pins available
    String ground_locations[] = {
        "Pin header GND pins",
        "USB-C connector shield", 
        "Battery connector negative",
        "Expansion connector GND"
    };
    
    // Power management features
    bool auto_power_switching = true;        // Automatic USB/battery switching
    bool battery_charging = true;            // Built-in Li-ion charging
    bool low_power_modes = true;            // Deep sleep and light sleep support
};
```

## Board-Specific Configuration

### ESP32-S3 Specific Settings

#### PSRAM Configuration
```cpp
// PSRAM optimization for ESP32-S3
struct PSRAMConfig {
    bool enable_psram = true;                    // Enable 8MB PSRAM
    String psram_type = "OPI PSRAM";             // Octal SPI PSRAM
    uint32_t psram_size_mb = 8;                  // 8MB capacity
    uint32_t psram_speed_mhz = 80;               // 80MHz access speed
    
    // Memory allocation strategy
    bool use_psram_for_camera_fb = true;         // Use PSRAM for frame buffers
    bool use_psram_for_large_arrays = true;      // Use PSRAM for large data
    uint8_t frame_buffer_count = 3;              // Multiple frame buffers
    uint32_t jpeg_buffer_size_kb = 512;          // Large JPEG buffers
    
    // Performance optimization
    bool enable_cache = true;                    // Enable PSRAM cache
    bool enable_prefetch = true;                 // Enable memory prefetch
};
```

#### Camera Initialization Parameters for ESP32-S3
```cpp
// Optimized camera settings for T-Camera Plus S3
struct CameraConfigS3 {
    // Clock and timing settings
    uint32_t xclk_freq_hz = 20000000;            // 20MHz camera clock
    ledc_timer_t ledc_timer = LEDC_TIMER_0;      // LED control timer
    ledc_channel_t ledc_channel = LEDC_CHANNEL_0; // LED control channel
    
    // Image quality settings (optimized for ESP32-S3)
    pixformat_t pixel_format = PIXFORMAT_JPEG;   // JPEG compression
    framesize_t frame_size = FRAMESIZE_UXGA;     // 1600x1200 (high quality)
    uint8_t jpeg_quality = 8;                    // High quality (ESP32-S3 can handle it)
    uint8_t frame_buffer_count = 3;              // Multiple buffers for smooth operation
    
    // ESP32-S3 specific optimizations
    bool enable_psram_fb = true;                 // Use PSRAM for frame buffers
    bool grab_mode = CAMERA_GRAB_WHEN_EMPTY;     // Efficient buffer management
    uint32_t fb_location = CAMERA_FB_IN_PSRAM;   // Frame buffers in PSRAM
    
    // Advanced image processing
    bool enable_auto_exposure = true;            // Automatic exposure
    bool enable_auto_white_balance = true;       // Automatic white balance
    bool enable_auto_gain = true;                // Automatic gain control
    int8_t brightness = 0;                       // Default brightness
    int8_t contrast = 0;                         // Default contrast
    int8_t saturation = 1;                       // Slightly enhanced saturation
    bool enable_lens_correction = true;          // Lens distortion correction
};
```

#### Display Configuration
```cpp
// TFT display configuration for ST7789
struct DisplayConfig {
    // Hardware settings
    uint16_t width = 240;                        // Display width
    uint16_t height = 240;                       // Display height
    uint8_t rotation = 1;                        // Landscape orientation
    uint32_t spi_frequency = 27000000;           // 27MHz SPI speed
    
    // Pin assignments
    gpio_num_t cs_pin = GPIO_NUM_12;             // Chip select
    gpio_num_t dc_pin = GPIO_NUM_13;             // Data/command
    gpio_num_t rst_pin = GPIO_NUM_14;            // Reset
    gpio_num_t bl_pin = GPIO_NUM_15;             // Backlight control
    
    // Power management
    uint8_t backlight_pwm_channel = 1;           // PWM channel for backlight
    uint8_t backlight_pwm_frequency = 5000;     // 5kHz PWM frequency
    uint8_t default_brightness = 128;            // 50% brightness default
    uint8_t max_brightness = 255;                // Maximum brightness
    uint8_t min_brightness = 16;                 // Minimum readable brightness
    
    // Auto-dimming settings
    bool enable_auto_dimming = true;             // Automatic brightness control
    uint32_t display_timeout_ms = 30000;         // 30 second timeout
    uint32_t backlight_fade_time_ms = 1000;      // 1 second fade
    
    // Color settings
    uint16_t background_color = 0x0000;          // Black background
    uint16_t text_color = 0xFFFF;                // White text
    uint16_t highlight_color = 0x07E0;           // Green highlights
    uint16_t warning_color = 0xFFE0;             // Yellow warnings
    uint16_t error_color = 0xF800;               // Red errors
};
```

### Power Management Settings

#### Advanced Power Control for ESP32-S3
```cpp
// Enhanced power management for longer battery life
struct PowerManagementS3 {
    // CPU frequency scaling
    uint32_t cpu_freq_mhz_active = 240;          // Full speed for camera operations
    uint32_t cpu_freq_mhz_idle = 80;             // Reduced speed for idle
    uint32_t cpu_freq_mhz_sleep = 10;            // Minimum for sleep preparation
    
    // Sleep mode configuration
    esp_sleep_wakeup_cause_t wakeup_sources = 
        ESP_SLEEP_WAKEUP_EXT0 |                  // GPIO wakeup (PIR sensor)
        ESP_SLEEP_WAKEUP_TIMER |                 // Timer wakeup
        ESP_SLEEP_WAKEUP_TOUCHPAD;               // Touch wakeup (if configured)
    
    uint64_t deep_sleep_duration_us = 300000000; // 5 minutes default
    uint64_t light_sleep_duration_us = 10000000; // 10 seconds for quick wake
    
    // Peripheral power control
    bool auto_disable_wifi_sleep = true;         // Turn off WiFi in sleep
    bool auto_disable_bluetooth_sleep = true;    // Turn off Bluetooth in sleep
    bool auto_power_down_camera_sleep = true;    // Power down camera in sleep
    bool auto_power_down_display_sleep = true;   // Power down display in sleep
    bool keep_rtc_memory = true;                 // Preserve RTC memory
    
    // Power consumption targets (mA)
    float active_capture_current = 280;          // Camera + display + WiFi
    float active_idle_current = 80;              // WiFi connected, no camera
    float light_sleep_current = 0.8;             // Light sleep with RTC
    float deep_sleep_current = 0.015;            // Deep sleep minimal
    
    // Battery monitoring
    gpio_num_t battery_adc_pin = GPIO_NUM_1;     // Battery voltage ADC
    float voltage_divider_ratio = 2.0;           // 2:1 voltage divider
    float battery_full_voltage = 4.2;            // Full charge voltage
    float battery_empty_voltage = 3.0;           // Empty battery voltage
    uint8_t low_battery_threshold_percent = 20;  // Low battery warning
    uint8_t critical_battery_threshold_percent = 5; // Critical shutdown
};
```

#### Display Power Optimization
```cpp
// Display power management
struct DisplayPowerConfig {
    // Backlight control
    bool enable_adaptive_brightness = true;      // Adjust brightness automatically
    uint8_t outdoor_brightness = 255;            // Full brightness for outdoor
    uint8_t indoor_brightness = 128;             // Medium brightness for indoor
    uint8_t night_brightness = 32;               // Low brightness for night
    
    // Auto-off timing
    uint32_t display_off_delay_ms = 60000;       // 1 minute of inactivity
    uint32_t display_dim_delay_ms = 30000;       // 30 seconds to dim
    uint32_t capture_display_time_ms = 5000;     // Show capture for 5 seconds
    
    // Sleep integration
    bool turn_off_display_sleep = true;          // Turn off display in sleep
    bool preserve_display_state = true;          // Remember last display state
    uint32_t wake_display_time_ms = 3000;        // Show display on wake for 3 sec
};
```

### Special Configuration Requirements

#### SD Card High-Speed Configuration
```cpp
// High-speed SD card setup for ESP32-S3
struct SDCardConfigS3 {
    // Interface settings
    String sd_mode = "SDMMC 4-bit";              // Use high-speed 4-bit interface
    uint32_t sd_frequency_mhz = 40;              // 40MHz for fast writes
    bool enable_ddr_mode = false;                // DDR mode (if card supports)
    
    // File system optimization
    String filesystem = "FATFS";                 // FAT32 for compatibility
    uint32_t allocation_unit_size = 32768;       // 32KB clusters for large files
    bool enable_long_filenames = true;           // Support long filenames
    
    // Performance settings
    uint32_t write_buffer_size = 8192;           // 8KB write buffer
    bool enable_write_cache = true;              // Enable write caching
    bool auto_sync_on_capture = false;           // Sync after multiple captures
    uint32_t sync_interval_captures = 5;         // Sync every 5 captures
    
    // File naming and organization
    String file_prefix = "TCAM";                 // File prefix
    String date_format = "YYYYMMDD";             // Date format in filename
    String time_format = "HHMMSS";               // Time format in filename
    bool create_daily_folders = true;            // Organize by date
    uint32_t max_files_per_folder = 1000;        // Limit files per folder
};
```

#### WiFi Configuration for ESP32-S3
```cpp
// Optimized WiFi settings for ESP32-S3
struct WiFiConfigS3 {
    // Power management
    wifi_ps_type_t power_save_mode = WIFI_PS_MIN_MODEM; // Minimal power save
    uint8_t max_tx_power = 20;                   // Maximum TX power (dBm)
    uint8_t min_tx_power = 8;                    // Minimum TX power for close range
    
    // Connection optimization
    uint8_t max_connection_attempts = 5;         // Connection retry limit
    uint32_t connection_timeout_ms = 15000;      // 15 second timeout
    bool enable_fast_connect = true;            // Use stored credentials
    bool enable_auto_reconnect = true;          // Automatic reconnection
    
    // Advanced features (ESP32-S3 specific)
    bool enable_80211n = true;                  // 802.11n support
    bool enable_wmm = true;                     // WiFi multimedia support
    wifi_bandwidth_t bandwidth = WIFI_BW_HT20;  // 20MHz bandwidth
    
    // Mesh and AP capabilities
    bool enable_mesh_networking = true;         // Support mesh networks
    uint8_t max_mesh_hops = 6;                  // Maximum mesh hops
    bool enable_ap_mode = true;                 // Can act as access point
    uint8_t max_ap_connections = 4;             // Maximum concurrent connections
    
    // Data transmission optimization
    uint32_t tcp_mss = 1436;                    // Maximum segment size
    uint32_t tcp_window_size = 8192;            // TCP window size
    bool enable_tcp_keepalive = true;           // Keep connections alive
    uint32_t keepalive_interval_sec = 60;       // Keepalive interval
};
```  

```

## Troubleshooting Section

### Common Issues and Solutions

#### Camera Initialization Failures
```cpp
// Troubleshooting camera initialization
void troubleshootCameraS3() {
    // Check PSRAM availability
    if (!psramFound()) {
        Serial.println("ERROR: PSRAM not found - check board configuration");
        Serial.println("Solution: Verify PSRAM is enabled in Arduino IDE board settings");
        return;
    }
    
    // Check available memory
    size_t psram_size = ESP.getPsramSize();
    size_t free_psram = ESP.getFreePsram();
    Serial.printf("PSRAM: %d bytes total, %d bytes free\n", psram_size, free_psram);
    
    if (free_psram < 1000000) {  // Less than 1MB free
        Serial.println("WARNING: Low PSRAM memory may cause camera issues");
    }
    
    // Test camera pins
    if (!testCameraPins()) {
        Serial.println("ERROR: Camera pin test failed");
        Serial.println("Solution: Check for pin conflicts with display or other peripherals");
    }
    
    // Test I2C communication
    if (!testI2CCommunication(SIOD_GPIO_NUM, SIOC_GPIO_NUM)) {
        Serial.println("ERROR: I2C communication with camera failed");
        Serial.println("Solution: Check SDA/SCL connections and pull-up resistors");
    }
}

bool testCameraPins() {
    // Test critical camera pins for conflicts
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << XCLK_GPIO_NUM);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    
    esp_err_t ret = gpio_config(&io_conf);
    return (ret == ESP_OK);
}
```

#### Display Issues
```
Common Display Problems:

1. "Display not initializing"
   - Check TFT_eSPI library configuration
   - Verify User_Setup.h or User_Setup_Select.h settings
   - Ensure correct pin definitions for T-Camera Plus S3

2. "Garbled display output"
   - Verify SPI frequency (should be 27MHz or lower)
   - Check display orientation setting
   - Ensure adequate power supply

3. "Backlight not working"
   - Check GPIO 15 backlight pin connection
   - Verify PWM configuration for backlight control
   - Test with manual digitalWrite(15, HIGH)
```

#### Upload and Programming Issues
```
Programming Troubleshooting:

1. "Failed to connect to ESP32-S3"
   - Hold BOOT button while plugging in USB cable
   - Press RESET button while holding BOOT, then release RESET
   - Try lower upload speed (115200 instead of 921600)

2. "Partition table error"
   - Select correct partition scheme: "16M Flash (3MB APP/9.9MB FATFS)"
   - Ensure Flash Size is set to "16MB (128Mb)"
   - Try "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)" for smaller programs

3. "Brown-out detector triggered"
   - Use high-quality USB cable
   - Try external 5V power supply
   - Reduce CPU frequency temporarily
   - Check for short circuits
```

#### SD Card Problems
```cpp
// SD card troubleshooting function
void troubleshootSDCard() {
    Serial.println("SD Card Diagnostic:");
    
    // Test SD card detection
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("SD Card mount failed");
        Serial.println("Possible causes:");
        Serial.println("- No SD card inserted");
        Serial.println("- SD card not formatted (use FAT32)");
        Serial.println("- SD card too large (use 32GB or smaller)");
        Serial.println("- SD card speed too slow (use Class 10)");
        return;
    }
    
    // Check SD card info
    sdmmc_card_t* card = SD_MMC.cardBegin();
    if (card) {
        Serial.printf("SD Card Type: %s\n", 
            card->ocr & SD_OCR_SDHC_CAP ? "SDHC" : "SDSC");
        Serial.printf("SD Card Size: %llu MB\n", 
            ((uint64_t) card->csd.capacity) * card->csd.sector_size / (1024 * 1024));
        Serial.printf("SD Card Speed: %d MHz\n", card->max_freq_khz / 1000);
    }
    
    // Test write speed
    testSDWriteSpeed();
}

void testSDWriteSpeed() {
    const size_t test_size = 10240;  // 10KB test
    uint8_t* test_data = (uint8_t*)malloc(test_size);
    if (!test_data) return;
    
    // Fill with random data
    for (size_t i = 0; i < test_size; i++) {
        test_data[i] = random(256);
    }
    
    unsigned long start_time = millis();
    File test_file = SD_MMC.open("/speed_test.bin", FILE_WRITE);
    if (test_file) {
        test_file.write(test_data, test_size);
        test_file.close();
        unsigned long write_time = millis() - start_time;
        float write_speed = (test_size / 1024.0) / (write_time / 1000.0);  // KB/s
        Serial.printf("SD Write Speed: %.2f KB/s\n", write_speed);
        
        if (write_speed < 100) {
            Serial.println("WARNING: SD card write speed is slow");
            Serial.println("Recommendation: Use Class 10 or UHS-I SD card");
        }
    }
    
    free(test_data);
    SD_MMC.remove("/speed_test.bin");
}
```

#### WiFi Connection Problems
```cpp
// WiFi diagnostics for ESP32-S3
void diagnoseWiFiIssues() {
    Serial.println("WiFi Diagnostic Report:");
    
    // Check WiFi capability
    if (!WiFi.isConnected()) {
        Serial.println("WiFi not connected - running diagnostics...");
        
        // Scan for networks
        Serial.println("Scanning for WiFi networks...");
        int n = WiFi.scanNetworks();
        if (n == 0) {
            Serial.println("ERROR: No WiFi networks found");
            Serial.println("- Check if WiFi is enabled on your router");
            Serial.println("- Verify antenna connection");
            Serial.println("- Try moving closer to router");
        } else {
            Serial.printf("Found %d networks:\n", n);
            for (int i = 0; i < n; i++) {
                Serial.printf("%d: %s (RSSI: %d)\n", 
                    i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            }
        }
        
        // Test connection with diagnostics
        WiFi.begin("YourSSID", "YourPassword");
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("\nWiFi connection failed");
            switch (WiFi.status()) {
                case WL_NO_SSID_AVAIL:
                    Serial.println("SSID not available");
                    break;
                case WL_CONNECT_FAILED:
                    Serial.println("Connection failed - check password");
                    break;
                case WL_CONNECTION_LOST:
                    Serial.println("Connection lost");
                    break;
                case WL_DISCONNECTED:
                    Serial.println("Disconnected");
                    break;
                default:
                    Serial.printf("Unknown status: %d\n", WiFi.status());
            }
        } else {
            Serial.println("\nWiFi connected successfully!");
            Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
            Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
        }
    }
}
```

#### Power and Battery Issues
```cpp
// Power system diagnostics
void diagnosePowerSystem() {
    // Read battery voltage
    float battery_voltage = analogRead(1) * 3.3 / 4095.0 * 2.0;  // Voltage divider
    Serial.printf("Battery Voltage: %.2fV\n", battery_voltage);
    
    // Calculate battery percentage
    float battery_percent = ((battery_voltage - 3.0) / (4.2 - 3.0)) * 100;
    battery_percent = constrain(battery_percent, 0, 100);
    Serial.printf("Battery Level: %.1f%%\n", battery_percent);
    
    // Power status analysis
    if (battery_voltage > 4.1) {
        Serial.println("Status: Battery fully charged");
    } else if (battery_voltage > 3.7) {
        Serial.println("Status: Battery good");
    } else if (battery_voltage > 3.4) {
        Serial.println("Status: Battery low - consider charging");
    } else if (battery_voltage > 3.0) {
        Serial.println("Status: Battery critical - charge immediately");
    } else {
        Serial.println("Status: Battery empty or not connected");
    }
    
    // Check if USB powered
    bool usb_powered = (battery_voltage > 4.5);  // USB typically provides ~5V
    Serial.printf("Power Source: %s\n", usb_powered ? "USB" : "Battery");
    
    // Memory and performance check
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
}
```

## Example Code Snippets

### Complete Camera and Display Integration

```cpp
#include "esp_camera.h"
#include "SD_MMC.h"
#include <TFT_eSPI.h>
#include <WiFi.h>

// Pin definitions for LilyGO T-Camera Plus S3
#define CAMERA_MODEL_LILYGO_T_CAMERA_PLUS_S3
#include "camera_pins.h"

TFT_eSPI tft = TFT_eSPI();

// WiFi credentials
const char* ssid = "YourWiFiNetwork";
const char* password = "YourWiFiPassword";

void setup() {
    Serial.begin(115200);
    
    // Initialize display
    initializeDisplay();
    displayStatus("Initializing...");
    
    // Initialize SD card
    if (initializeSDCard()) {
        displayStatus("SD Card OK", TFT_GREEN);
    } else {
        displayStatus("SD Card Failed", TFT_RED);
    }
    
    // Initialize camera
    if (initializeCamera()) {
        displayStatus("Camera OK", TFT_GREEN);
    } else {
        displayStatus("Camera Failed", TFT_RED);
        return;
    }
    
    // Initialize WiFi
    if (initializeWiFi()) {
        displayStatus("WiFi Connected", TFT_GREEN);
    } else {
        displayStatus("WiFi Failed", TFT_YELLOW);
    }
    
    displayStatus("Ready for Capture");
}

bool initializeDisplay() {
    tft.init();
    tft.setRotation(1);  // Landscape
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    
    // Enable backlight
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);
    
    return true;
}

void displayStatus(const char* message, uint16_t color = TFT_WHITE) {
    static int line = 0;
    tft.setTextColor(color, TFT_BLACK);
    tft.setCursor(0, line * 12);
    tft.println(message);
    line = (line + 1) % 20;  // Wrap around after 20 lines
    Serial.println(message);
}

bool initializeSDCard() {
    if (!SD_MMC.begin("/sdcard", true)) {
        return false;
    }
    
    // Check SD card info
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    return true;
}

bool initializeCamera() {
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
    config.jpeg_quality = 10;
    config.fb_count = 2;
    
    // Use PSRAM for frame buffers
    if (psramFound()) {
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    }
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return false;
    }
    
    // Optimize sensor settings
    sensor_t *s = esp_camera_sensor_get();
    if (s != NULL) {
        s->set_brightness(s, 0);     // -2 to 2
        s->set_contrast(s, 0);       // -2 to 2
        s->set_saturation(s, 1);     // -2 to 2 (slightly enhanced)
        s->set_special_effect(s, 0); // 0 to 6 (0=No Effect)
        s->set_whitebal(s, 1);       // 0 = disable, 1 = enable
        s->set_awb_gain(s, 1);       // 0 = disable, 1 = enable
        s->set_exposure_ctrl(s, 1);  // 0 = disable, 1 = enable
        s->set_aec2(s, 0);           // 0 = disable, 1 = enable
        s->set_gain_ctrl(s, 1);      // 0 = disable, 1 = enable
        s->set_bpc(s, 0);            // 0 = disable, 1 = enable
        s->set_wpc(s, 1);            // 0 = disable, 1 = enable
        s->set_raw_gma(s, 1);        // 0 = disable, 1 = enable
        s->set_lenc(s, 1);           // 0 = disable, 1 = enable
        s->set_hmirror(s, 0);        // 0 = disable, 1 = enable
        s->set_vflip(s, 0);          // 0 = disable, 1 = enable
        s->set_dcw(s, 1);            // 0 = disable, 1 = enable
    }
    
    return true;
}

bool initializeWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
        displayStatus("Connecting WiFi...");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("WiFi connected: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    return false;
}

void loop() {
    // Capture image
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        displayStatus("Capture Failed", TFT_RED);
        return;
    }
    
    // Save to SD card
    String filename = "/IMG_" + String(millis()) + ".jpg";
    File file = SD_MMC.open(filename, FILE_WRITE);
    if (file) {
        file.write(fb->buf, fb->len);
        file.close();
        
        displayStatus(("Saved: " + filename).c_str(), TFT_GREEN);
        Serial.printf("Image saved: %s (%d bytes)\n", filename.c_str(), fb->len);
        
        // Display image info on screen
        tft.fillRect(0, 180, 240, 60, TFT_BLACK);
        tft.setCursor(0, 180);
        tft.printf("Size: %d KB\n", fb->len / 1024);
        tft.printf("Resolution: %dx%d\n", fb->width, fb->height);
        tft.printf("Free: %d KB\n", ESP.getFreeHeap() / 1024);
    } else {
        displayStatus("Save Failed", TFT_RED);
    }
    
    esp_camera_fb_return(fb);
    
    // Wait before next capture
    delay(10000);
}
```

### PIR Sensor Integration with Display Feedback

```cpp
#include "esp_camera.h"
#include <TFT_eSPI.h>

const int PIR_PIN = 2;
const int BACKLIGHT_PIN = 15;
TFT_eSPI tft = TFT_eSPI();

volatile bool motionDetected = false;
unsigned long lastCaptureTime = 0;
const unsigned long CAPTURE_INTERVAL = 5000;  // 5 seconds between captures

void IRAM_ATTR motionISR() {
    motionDetected = true;
}

void setup() {
    Serial.begin(115200);
    
    // Initialize display
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    
    // Initialize PIR sensor
    pinMode(PIR_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionISR, RISING);
    
    // Initialize camera (use previous camera init code)
    initializeCamera();
    
    // Setup sleep wakeup
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1);
    
    displayWelcomeScreen();
}

void displayWelcomeScreen() {
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 50);
    tft.println("T-Camera Plus");
    tft.setCursor(30, 80);
    tft.println("Wildlife Cam");
    
    tft.setTextSize(1);
    tft.setCursor(10, 120);
    tft.println("Motion Detection: ACTIVE");
    tft.setCursor(10, 140);
    tft.println("Status: Waiting...");
    
    // Dim display after 10 seconds
    delay(10000);
    analogWrite(BACKLIGHT_PIN, 32);  // Dim backlight
}

void handleMotionDetection() {
    if (motionDetected && (millis() - lastCaptureTime > CAPTURE_INTERVAL)) {
        motionDetected = false;
        lastCaptureTime = millis();
        
        // Wake up display
        analogWrite(BACKLIGHT_PIN, 255);
        
        // Update display
        tft.fillRect(0, 140, 240, 20, TFT_BLACK);
        tft.setTextColor(TFT_GREEN);
        tft.setCursor(10, 140);
        tft.println("Status: Motion Detected!");
        
        // Capture image
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) {
            // Save image (use previous save code)
            String filename = "/MOTION_" + String(millis()) + ".jpg";
            saveImageToSD(fb, filename);
            
            // Update display with capture info
            tft.fillRect(0, 160, 240, 40, TFT_BLACK);
            tft.setTextColor(TFT_CYAN);
            tft.setCursor(10, 160);
            tft.printf("Captured: %d KB", fb->len / 1024);
            tft.setCursor(10, 180);
            tft.printf("Time: %lu", millis());
            
            esp_camera_fb_return(fb);
        }
        
        // Dim display after 5 seconds
        delay(5000);
        analogWrite(BACKLIGHT_PIN, 32);
        
        // Update status
        tft.fillRect(0, 140, 240, 20, TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(10, 140);
        tft.println("Status: Waiting...");
    }
}

void loop() {
    handleMotionDetection();
    
    // Enter light sleep when idle
    if (millis() - lastCaptureTime > 30000) {  // 30 seconds idle
        Serial.println("Entering light sleep...");
        esp_light_sleep_start();
    }
    
    delay(100);
}
```

## Conclusion

Congratulations! You have successfully deployed the LilyGO T-Camera Plus S3 with comprehensive configuration and advanced features. This guide provides all the necessary information for:

- **Complete hardware setup** with proper pin configuration
- **Optimized software configuration** for ESP32-S3 capabilities  
- **Advanced power management** for extended battery operation
- **Display integration** with status feedback and user interface
- **SD card storage** with high-speed file operations
- **WiFi connectivity** with mesh networking capabilities
- **Troubleshooting support** for common deployment issues
- **Example implementations** for real-world wildlife monitoring

The LilyGO T-Camera Plus S3 offers superior performance compared to basic ESP32-CAM boards, with its ESP32-S3 processor, 8MB PSRAM, built-in display, and integrated battery management making it ideal for advanced wildlife monitoring deployments requiring local user interface and extended autonomous operation.

For additional support and advanced configurations, refer to the project's firmware source code and board compatibility documentation.