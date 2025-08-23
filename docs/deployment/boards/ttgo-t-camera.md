# TTGO T-Camera Deployment Guide

## Prerequisites

### Hardware Requirements
- TTGO T-Camera development board
- USB-C cable for programming and power
- MicroSD card (recommended: 32GB Class 10)
- Optional: External PIR sensor, environmental sensors
- Optional: External antenna for extended WiFi range

### Software Requirements
- Arduino IDE 2.0+ or PlatformIO
- ESP32 board package
- TFT_eSPI library for display
- Required libraries: esp32-camera, TFT_eSPI, WiFi

### Skill Level
- **Beginner**: Arduino experience with basic display knowledge
- **Intermediate**: Understanding of TFT displays and GUI programming
- **Advanced**: System integration with display-based user interfaces

## Overview

The TTGO T-Camera is a feature-rich ESP32 camera board with an integrated 0.96" OLED display, combining camera functionality with real-time visual feedback. With its built-in display, comprehensive GPIO access, and balanced feature set, it's ideal for wildlife monitoring applications requiring local status display, field configuration, and interactive operation.

## Board Specifications

### Hardware Features
- **Microcontroller**: ESP32 (240MHz dual-core)
- **Camera**: OV2640 2MP camera with adjustable lens
- **Display**: 0.96" OLED display (128x64 pixels)
- **Memory**: 520KB SRAM + 4MB PSRAM + 16MB Flash
- **Connectivity**: WiFi 802.11 b/g/n, Bluetooth 4.2
- **GPIO**: 18+ available GPIO pins
- **Power**: USB-C 5V input or 3.7V Li-ion battery connector
- **Dimensions**: 54mm x 54mm x 12mm
- **Extras**: MicroSD slot, battery management, multiple buttons

### Camera Specifications
- **Sensor**: OV2640 CMOS sensor
- **Resolution**: Up to 1600x1200 (UXGA)
- **Formats**: JPEG, RGB565, YUV422
- **Features**: Auto exposure, auto white balance, manual focus
- **Frame Rate**: Up to 60fps at lower resolutions
- **Field of View**: ~66° diagonal

## Pin Configuration

### Standard GPIO Map
```cpp
// TTGO T-Camera Pin Definitions
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26  // SDA
#define SIOC_GPIO_NUM     27  // SCL

// Camera data pins
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Built-in peripherals
#define LED_GPIO_NUM      4   // Built-in LED/Flash
#define SD_CS_PIN         13  // SD card chip select
#define OLED_SDA          14  // OLED display SDA
#define OLED_SCL          15  // OLED display SCL
#define BUTTON_1          38  // User button 1
#define BUTTON_2          37  // User button 2
#define BAT_ADC           39  // Battery voltage monitoring
```

### Available GPIO for External Sensors
```cpp
// Available pins for sensors and peripherals
#define AVAILABLE_GPIOS {2, 12, 16, 17, 33}

// Pin usage recommendations
// GPIO 2: Safe for digital sensors
// GPIO 12: Can conflict with SD card, use carefully
// GPIO 16, 17: UART pins, good for sensor communication
// GPIO 33: ADC capable for analog sensors

// Secondary I2C for sensors (if OLED on different bus)
#define SENSOR_I2C_SDA    16
#define SENSOR_I2C_SCL    17
```

### Pin Limitations and Conflicts
```cpp
// Pin considerations for TTGO T-Camera
struct PinConflicts {
    // OLED display pins
    bool oled_enabled = true;             // Built-in OLED display
    int oled_sda = 14;                    // OLED I2C SDA
    int oled_scl = 15;                    // OLED I2C SCL
    
    // SD card interface
    bool sd_card_enabled = true;          // Built-in SD card slot
    int sd_cs = 13;                       // SD chip select
    int sd_mosi = 15;                     // Conflicts with OLED SCL
    int sd_miso = 2;                      // Can be used for sensors when SD disabled
    int sd_clk = 14;                      // Conflicts with OLED SDA
    
    // User interface
    bool buttons_available = true;        // Built-in buttons
    int button1 = 38;                     // User button 1
    int button2 = 37;                     // User button 2
    
    // Battery monitoring
    bool battery_monitoring = true;       // Built-in battery management
    int battery_adc = 39;                 // Battery voltage ADC
    
    // SD vs sensor trade-off
    bool sd_vs_sensors = true;            // Choose SD or more sensors
};
```

## Step-by-Step Deployment Instructions

### 1. Hardware Setup

#### Basic Assembly Steps
1. **Prepare Components**
   - TTGO T-Camera board
   - USB-C cable for programming
   - MicroSD card for storage
   - External sensors (PIR, environmental)

2. **Programming Connection**
   ```
   TTGO T-Camera      Computer
   -------------      --------
   USB-C Port     ->  USB-A/C Port
   
   Note: USB-C direct programming
   Boot button: Hold during power-on for download mode
   Display: Shows programming status
   ```

3. **Initial Programming**
   - Connect via USB-C cable
   - Hold BOOT button and press RESET for programming mode
   - Upload firmware using Arduino IDE or PlatformIO
   - Monitor via USB-C serial and OLED display

#### External Sensor Integration
```cpp
// PIR Motion Sensor Connection
// VCC  -> 3.3V
// GND  -> GND  
// OUT  -> GPIO 2

// Environmental Sensor (BME280) - Secondary I2C
// VCC  -> 3.3V
// GND  -> GND
// SDA  -> GPIO 16 (secondary I2C)
// SCL  -> GPIO 17 (secondary I2C)

// Analog sensors
// Analog sensor -> GPIO 33 (ADC1_CH5)

// Note: Careful planning needed due to OLED/SD conflicts
```

#### Power System Integration
```cpp
// Power System for TTGO T-Camera (Display Power Needs)
struct PowerSystemComponents {
    // Power system accounting for display
    float solar_panel_watts = 8.0;            // 8W for display operation
    float solar_panel_voltage = 6.0;          // 6V output
    
    // Battery (larger for display power)
    uint32_t battery_capacity_mah = 2500;     // 2.5Ah Li-ion
    float battery_voltage = 3.7;              // 3.7V nominal
    
    // Charge controller
    String charge_controller = "TP4056";      // Li-ion charger
    bool enable_protection = true;            // Battery protection
    
    // Display power management
    bool display_auto_off = true;             // Auto-off display to save power
    uint32_t display_timeout_ms = 30000;      // 30 second timeout
    uint8_t display_brightness = 128;         // Medium brightness default
    
    // Voltage regulation
    String voltage_regulator = "Built-in";    // ESP32 built-in LDO
    uint32_t max_current_ma = 800;            // Current with display active
};
```

### 2. Software Configuration

#### Arduino IDE Setup
1. Install ESP32 board package
2. Select board: "ESP32 Dev Module"
3. Configure settings:
   - CPU Frequency: 240MHz
   - Flash Size: 16MB
   - Partition Scheme: Default 4MB with spiffs
   - PSRAM: Enabled
4. Install libraries: esp32-camera, TFT_eSPI, Adafruit_SSD1306

#### PlatformIO Setup
```ini
[env:ttgo-t-camera]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600

lib_deps = 
    espressif/esp32-camera@^2.0.4
    bodmer/TFT_eSPI@^2.5.0
    adafruit/Adafruit SSD1306@^2.5.7
    bblanchon/ArduinoJson@^6.21.3
    WiFi

build_flags = 
    -D BOARD_TTGO_T_CAMERA
    -D CAMERA_MODEL_TTGO_T_CAMERA
    -D OLED_DISPLAY_ENABLED
    -D CONFIG_CAMERA_TASK_STACK_SIZE=4096
```

### 3. Firmware Upload and Testing

#### Initial Firmware Upload
1. Connect TTGO T-Camera via USB-C
2. Hold BOOT button and press RESET for programming mode
3. Upload firmware via Arduino IDE or PlatformIO
4. Monitor via serial and watch OLED display for status
5. Test camera capture and display functionality

#### Configuration and Testing
1. Configure WiFi credentials and display settings
2. Test camera capture with live preview on OLED
3. Verify button functionality for user interaction
4. Test SD card operation and file storage
5. Validate battery monitoring and power management

## Deployment Configurations

### Interactive Monitoring Setup
```cpp
// Interactive wildlife monitoring with display feedback
struct InteractiveMonitoringConfig {
    // Optimized for user interaction
    framesize_t frame_size = FRAMESIZE_VGA;     // 640x480
    uint8_t jpeg_quality = 12;                  // Good quality
    bool enable_psram = true;                   // Use available PSRAM
    
    // Display features
    bool display_enabled = true;                // Use OLED display
    bool show_live_preview = true;              // Live camera preview
    bool show_statistics = true;                // Capture statistics
    bool show_battery_status = true;            // Battery level display
    bool menu_system = true;                    // Button-based menu
    
    // Interactive controls
    bool button_capture = true;                 // Manual capture via buttons
    bool button_settings = true;                // Settings via buttons
    bool display_timeout_configurable = true;  // User-configurable timeout
    
    // Data presentation
    bool show_last_capture_time = true;         // Last capture timestamp
    bool show_images_captured = true;           // Image count
    bool show_storage_status = true;            // SD card status
    
    // Estimated cost: $110-140
    // User interaction: High (display + buttons)
    // Use case: Field research with immediate feedback
};
```

### Field Research Setup
```cpp
// Field research with display-based data validation
struct FieldResearchConfig {
    // Research-grade imaging with validation
    framesize_t frame_size = FRAMESIZE_UXGA;    // 1600x1200
    uint8_t jpeg_quality = 10;                  // High quality
    bool enable_sequence_capture = true;        // Multiple images per trigger
    
    // Research display features
    bool display_image_quality_metrics = true; // Show quality metrics
    bool display_environmental_data = true;    // Show sensor readings
    bool display_gps_coordinates = true;       // GPS data (if available)
    bool display_research_annotations = true;  // Field notes display
    
    // Field validation
    bool immediate_image_review = true;         // Review images on display
    bool quality_assessment = true;             // Automatic quality check
    bool retake_on_poor_quality = true;         // Auto-retake poor images
    
    // Research coordination
    bool multi_camera_sync = true;              // Sync with other cameras
    bool research_protocol_display = true;     // Show protocol steps
    bool data_collection_progress = true;      // Progress tracking
    
    // Estimated cost: $130-160
    // Data quality: Research-grade with validation
    // Field efficiency: High due to immediate feedback
};
```

### Educational Deployment Setup
```cpp
// Educational wildlife monitoring with learning interface
struct EducationalConfig {
    // Educational-focused operation
    framesize_t frame_size = FRAMESIZE_SVGA;    // 800x600
    uint8_t jpeg_quality = 15;                  // Moderate quality
    bool simplified_interface = true;           // Simple button controls
    
    // Educational display features
    bool species_identification_hints = true;   // Show species info
    bool educational_facts = true;              // Display wildlife facts
    bool capture_technique_tips = true;         // Photography tips
    bool conservation_messages = true;          // Conservation education
    
    // Learning features
    bool guided_setup_mode = true;              // Step-by-step setup
    bool interactive_tutorials = true;          // Button-driven tutorials
    bool progress_tracking = true;              // Track learning progress
    bool achievement_system = true;             // Educational achievements
    
    // Safety and supervision
    bool supervisor_lock = true;                // Prevent setting changes
    bool safe_deployment_warnings = true;      // Safety reminders
    bool battery_safety_education = true;      // Battery safety info
    
    // Estimated cost: $100-130
    // Educational value: High interactive learning
    // Supervision: Built-in safety features
};
```

## Hardware Assembly Guide

### Weatherproof Enclosure Requirements
- **IP Rating**: IP65 for outdoor deployment
- **Temperature Range**: -15°C to +60°C operational
- **Humidity**: Up to 95% RH non-condensing
- **Material**: Clear polycarbonate window for display visibility
- **Display Protection**: Anti-glare coating for outdoor readability
- **Button Access**: Waterproof button extensions

### 3D Printable Enclosure
```stl
// 3D printable files available in 3d_models/ttgo_t_camera/
- ttgo_t_camera_main_enclosure.stl       // Main housing with display window
- ttgo_display_protection_window.stl     // Clear protective window for OLED
- ttgo_button_extensions.stl             // Waterproof button extensions
- ttgo_camera_lens_hood.stl              // Camera lens protection
- ttgo_battery_compartment.stl           // Battery housing extension
- ttgo_mounting_bracket.stl              // Versatile mounting system
- sd_card_access_door.stl                // Easy SD card access
```

#### Print Settings
- **Layer Height**: 0.2mm (0.1mm for display window)
- **Infill**: 25%
- **Material**: PETG (clear PETG for display window)
- **Supports**: Required for button extensions
- **Print Time**: 6-8 hours total

#### Assembly Instructions
1. Print enclosure with clear window for display visibility
2. Install TTGO T-Camera with display aligned to window
3. Install waterproof button extensions
4. Route cables through cable glands
5. Install anti-glare coating on display window
6. Test all functions before final sealing

## Enclosure and Mounting

### Installation Locations
- **Research Stations**: Desk or table mounting for easy interaction
- **Field Monitoring**: Tree mounting with accessible height
- **Educational Sites**: Secure mounting with supervised access
- **Interactive Displays**: Visitor-accessible locations

### Environmental Considerations
- **Display Readability**: Position for optimal viewing angle
- **Button Access**: Ensure easy access for field operation
- **Sun Glare**: Use anti-glare protection for outdoor display
- **Battery Access**: Plan for regular battery monitoring

## Firmware Optimization

### Memory Optimization with Display
```cpp
// Memory optimization for TTGO T-Camera with display
#define CONFIG_CAMERA_FB_COUNT 2          // Frame buffers
#define CONFIG_JPEG_QUALITY 12            // Balance quality vs memory
#define CONFIG_FRAME_SIZE FRAMESIZE_VGA   // Optimal for most uses

// Display memory allocation
#define OLED_BUFFER_SIZE 1024             // OLED display buffer
#define MENU_SYSTEM_MEMORY 8192           // Menu system memory
#define CONFIG_DISPLAY_TASK_STACK 4096    // Display task stack

// UI responsiveness
#define BUTTON_DEBOUNCE_MS 50             // Button debounce
#define DISPLAY_UPDATE_RATE_MS 100        // Display update rate
```

### Power Consumption Optimization
```cpp
// Power management for TTGO T-Camera with display
struct PowerOptimization {
    // Display power management
    bool display_auto_off = true;         // Auto-off display
    uint32_t display_timeout_ms = 30000;  // 30 second timeout
    uint8_t display_brightness = 64;      // Reduced brightness
    bool display_sleep_mode = true;       // Display sleep mode
    
    // Button-triggered wake
    bool button_wake_enabled = true;      // Wake on button press
    bool display_wake_on_capture = true;  // Wake display on capture
    
    // Power scaling
    uint32_t cpu_freq_mhz = 160;         // Reduce CPU when display off
    bool dynamic_frequency = true;        // Scale based on display state
    
    // Peripheral management
    bool oled_power_gate = true;          // Power off OLED completely
    bool button_interrupt_wake = true;    // Button interrupt wake
    
    // Battery monitoring
    bool battery_level_display = true;    // Show battery on display
    uint32_t battery_check_interval = 60000; // Check every minute
};
```

## Performance Characteristics

### Display Performance
- **Display Type**: 0.96" OLED (128x64 pixels)
- **Refresh Rate**: 10-30 FPS (configurable)
- **Power Consumption**: 20-40mA (display active)
- **Visibility**: Good in shade, poor in direct sunlight
- **Interactive Response**: <100ms button response

### Camera Performance
- **Capture Speed**: 1-3 seconds with display update
- **Preview Performance**: Real-time preview on OLED
- **Image Quality**: Good balance of quality and speed
- **Storage**: Immediate display of capture status
- **Battery Life**: 8-16 hours with display use

### Environmental Limits
- **Operating Temperature**: -10°C to +50°C (display limited)
- **Humidity**: Up to 90% RH (with proper enclosure)
- **Display Visibility**: Poor in direct sunlight
- **Button Operation**: Works with gloves (with extensions)

## Troubleshooting Guide

### Common Issues and Solutions

#### Display Problems
**Symptoms**: Display not working, garbled display, dim display
**Solutions**:
1. Check I2C connections to OLED (SDA=14, SCL=15)
2. Verify OLED library configuration
3. Test with simple OLED example code
4. Check power supply voltage stability
5. Verify display initialization sequence

#### Button Response Issues
**Symptoms**: Buttons not responding, multiple triggers, stuck buttons
**Solutions**:
1. Check button debounce settings in code
2. Verify button pin assignments (38, 37)
3. Test button continuity with multimeter
4. Check for mechanical button damage
5. Implement proper interrupt handling

#### SD Card vs Display Conflicts
**Symptoms**: SD card not working when display active
**Solutions**:
1. Check pin conflict between SD and OLED (pins 14, 15)
2. Use software SPI for SD card on different pins
3. Disable SD card to free pins for display
4. Use external SD card module on different pins
5. Implement pin sharing with proper initialization

#### Power Management with Display
**Symptoms**: High power consumption, short battery life
**Solutions**:
1. Implement display auto-off timer
2. Reduce display brightness settings
3. Use display sleep mode between updates
4. Monitor power consumption in different modes
5. Optimize display update frequency

#### Camera + Display Performance
**Symptoms**: Slow camera response, display lag during capture
**Solutions**:
1. Optimize camera capture and display update sequence
2. Use separate tasks for camera and display
3. Reduce display update rate during capture
4. Implement frame buffer management
5. Balance camera quality vs display performance

### Emergency Recovery
```cpp
// Emergency diagnostics for TTGO T-Camera
void enterEmergencyDiagnostics() {
    // Initialize basic display
    initOLEDBasic();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    // Show emergency status on display
    display.println("TTGO T-CAM EMERGENCY");
    display.println("==================");
    
    // System status
    float batteryVoltage = getBatteryVoltage();
    display.printf("Battery: %.2fV\n", batteryVoltage);
    display.printf("Heap: %d KB\n", ESP.getFreeHeap() / 1024);
    
    // Test buttons
    display.println("Press BTN1 for test");
    display.display();
    
    // Simple button test
    while (true) {
        if (digitalRead(BUTTON_1) == LOW) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Button 1: OK");
            display.display();
            delay(1000);
        }
        if (digitalRead(BUTTON_2) == LOW) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Button 2: OK");
            display.display();
            delay(1000);
        }
        delay(100);
    }
}
```

## Board-Specific Considerations

### TTGO T-Camera Unique Features
- **Integrated OLED Display**: Real-time status and preview capability
- **User Buttons**: Physical interface for field operation
- **Balanced Feature Set**: Good compromise between features and complexity
- **Battery Monitoring**: Built-in battery voltage monitoring
- **SD Card Slot**: Local storage with easy access
- **Educational Value**: Excellent for learning and demonstration

### Best Practices
- **Display Management**: Implement auto-off to conserve battery
- **Button Interface**: Design intuitive button-based menus
- **Pin Planning**: Carefully manage limited GPIO with display conflicts
- **Power Budgeting**: Account for display power consumption
- **User Experience**: Leverage display for better user interaction

### Integration with Other Systems
- **Field Research**: Excellent for researcher interaction and validation
- **Educational Programs**: Ideal for hands-on learning experiences
- **Demonstration Systems**: Perfect for showing wildlife monitoring concepts
- **Prototyping**: Good platform for testing user interface concepts

## Success Stories and Case Studies

### Research Deployments
- **Field Research Training**: Graduate student field methods courses
- **Wildlife Behavior Studies**: Real-time behavior observation and validation
- **Camera Trap Development**: Prototyping advanced camera trap interfaces

### Conservation Projects
- **Community Engagement**: Local communities interact with monitoring systems
- **Ranger Training**: Field personnel training with visual feedback
- **Conservation Education**: Public demonstration of monitoring technology

### Educational Applications
- **STEM Education**: High school and undergraduate technology courses
- **Citizen Science**: Community-based monitoring with visual feedback
- **Research Methods**: Teaching field research techniques and validation

## Next Steps and Upgrades

### Firmware Updates
- **Enhanced UI**: More sophisticated menu systems and graphics
- **Display Optimization**: Better power management and visibility
- **Educational Features**: Built-in tutorials and species identification

### Hardware Upgrades
- **Larger Display**: Upgrade to color LCD for better visibility
- **Touch Interface**: Touch screen for advanced interaction
- **External Buttons**: Additional buttons for complex interfaces

### Advanced Features
- **Mobile App Integration**: Smartphone app for advanced configuration
- **Cloud Dashboard**: Real-time status display via web interface
- **Multi-Camera Coordination**: Synchronized operation with display feedback

## Cost Analysis

### Budget Breakdown
```cpp
struct CostAnalysis {
    // Core components (USD)
    float board_cost = 25.00;                  // TTGO T-Camera
    float programming_cable = 5.00;            // USB-C cable
    float sd_card_32gb = 8.00;                 // 32GB microSD card
    
    // Power system (display power needs)
    float solar_panel_8w = 22.00;              // 8W solar panel
    float battery_2500mah = 12.00;             // 2.5Ah Li-ion battery
    float charge_controller = 5.00;            // TP4056 charge controller
    
    // Sensors and peripherals
    float pir_sensor = 3.00;                   // PIR motion sensor
    float environmental_sensor = 10.00;        // BME280 sensor
    float external_antenna = 5.00;             // WiFi antenna
    
    // Enclosure and mounting (display access)
    float enclosure_with_window = 18.00;       // Display-accessible enclosure
    float button_extensions = 8.00;            // Waterproof button extensions
    float mounting_hardware = 10.00;           // Versatile mounting system
    float assembly_materials = 7.00;           // Assembly components
    
    // Total costs
    float basic_setup = 70.00;                 // Camera + display + basic power
    float standard_setup = 100.00;             // Add sensors and improved enclosure
    float advanced_setup = 130.00;             // Full interactive system
    
    // Operating costs (moderate maintenance)
    float maintenance_cost = 25.00;            // Display cleaning, button maintenance
    float educational_materials = 10.00;       // Educational content updates
};
```

### Return on Investment
```cpp
struct ROIAnalysis {
    // Deployment costs
    float initial_investment = 100.00;         // Standard interactive setup
    float installation_time_hours = 2.0;      // Moderate setup time
    float annual_maintenance_cost = 35.00;    // Moderate maintenance
    
    // Comparable alternatives
    float interactive_trail_cam = 400.00;     // Commercial interactive camera
    float research_display_system = 800.00;   // Research camera with display
    float educational_system = 600.00;        // Educational wildlife monitoring kit
    
    // Value proposition
    float cost_savings_vs_interactive = 300.00; // Savings vs interactive trail cam
    float cost_savings_vs_research = 700.00;    // Savings vs research system
    float cost_savings_vs_educational = 500.00; // Savings vs educational system
    float payback_period_months = 1;            // Fast payback for interactive needs
    
    // Data value
    uint32_t images_per_year = 5000;           // ~14 images per day
    float cost_per_image = 0.020;              // $0.020 per image
    bool interactive_capability = true;         // Unique interactive advantage
    bool educational_value = true;             // High educational value
};
```

---

*This guide is part of the ESP32 Wildlife Camera deployment documentation. For more information, visit the [main deployment guide](../README.md).*