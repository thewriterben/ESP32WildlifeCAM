# ESP32-CAM Integration Guide

## ESP32-CAM (AI-Thinker) Wildlife Camera Setup

The ESP32-CAM by AI-Thinker is the most popular and affordable ESP32 camera board, making it ideal for wildlife monitoring applications.

### Hardware Overview

- **MCU**: ESP32-WROOM-32 
- **Camera**: OV2640 (2MP)
- **Flash Memory**: 4MB
- **Connectivity**: WiFi 802.11 b/g/n
- **GPIO**: 9 available pins
- **Power**: 5V via USB or 3.3V direct
- **Form Factor**: 40.5mm x 27mm

### Pin Configuration

```cpp
// Camera pins
#define CAMERA_SDA_PIN    21
#define CAMERA_SCL_PIN    22
#define CAMERA_D0_PIN     5
#define CAMERA_D1_PIN     18
#define CAMERA_D2_PIN     19
#define CAMERA_D3_PIN     21
#define CAMERA_D4_PIN     36
#define CAMERA_D5_PIN     39
#define CAMERA_D6_PIN     34
#define CAMERA_D7_PIN     35
#define CAMERA_XCLK_PIN   0
#define CAMERA_PCLK_PIN   22
#define CAMERA_VSYNC_PIN  25
#define CAMERA_HREF_PIN   23
#define CAMERA_PWDN_PIN   32
#define LED_FLASH_PIN     4
```

### Wildlife Monitoring Setup

#### 1. Basic Wildlife Camera
```cpp
#include "firmware/src/camera_universal.h"
#include "firmware/src/board_abstraction.h"

void setup() {
    // Initialize board detection
    board_type_t board = board_detect();
    board_configure(board);
    
    // Configure camera for wildlife monitoring
    camera_config_t config = {
        .framesize = FRAMESIZE_UXGA,     // 1600x1200 for detail
        .pixel_format = PIXFORMAT_JPEG,   // Compressed for storage
        .jpeg_quality = 10,               // Good quality
        .flip_horizontally = false,
        .flip_vertically = false,
        .brightness = 0,
        .contrast = 0,
        .saturation = 0,
        .auto_exposure = true,
        .auto_white_balance = true
    };
    
    camera_universal_init(&config);
}

void loop() {
    // Capture wildlife image
    camera_fb_t* fb = camera_universal_capture();
    if (fb) {
        // Process and transmit image
        process_wildlife_image(fb);
        camera_universal_return_fb(fb);
    }
    
    // Sleep to conserve power
    delay(60000); // 1 minute between captures
}
```

#### 2. Solar Powered Deployment
```cpp
#include "firmware/boards/esp32_cam/power_mgmt.h"

void setup_solar_wildlife_cam() {
    // Initialize low power mode for solar operation
    esp32_cam_power_init(POWER_PROFILE_LOW);
    esp32_cam_optimize_for_solar();
    
    // Configure for intermittent operation
    camera_config_t config = {
        .framesize = FRAMESIZE_VGA,      // Lower resolution to save power
        .jpeg_quality = 15,              // Higher compression
        .auto_exposure = true
    };
    
    camera_universal_init(&config);
}

void solar_capture_loop() {
    // Check if sufficient power available
    if (esp32_cam_is_external_power_available()) {
        // Capture image
        camera_fb_t* fb = camera_universal_capture();
        if (fb) {
            transmit_to_base_station(fb);
            camera_universal_return_fb(fb);
        }
    }
    
    // Enter deep sleep to conserve power
    esp32_cam_enter_deep_sleep(1800000000); // 30 minutes
}
```

### Hardware Integration

#### Solar Panel Connection
```
Solar Panel (6V, 1W) -> Charge Controller -> Battery (3.7V Li-ion) -> ESP32-CAM
```

#### Cellular Module (SIM800L)
```cpp
// Connect SIM800L to ESP32-CAM
#define SIM800_TX_PIN  1  // Connect to GPIO1 (TX0)
#define SIM800_RX_PIN  3  // Connect to GPIO3 (RX0)
#define SIM800_PWR_PIN 12 // Power control
```

#### LoRa Module (SX1278)
```cpp
// Connect LoRa module
#define LORA_SCK_PIN   14
#define LORA_MISO_PIN  12
#define LORA_MOSI_PIN  13
#define LORA_CS_PIN    15
#define LORA_RST_PIN   16
#define LORA_DIO0_PIN  2
```

### Wildlife Monitoring Features

#### Motion Detection
```cpp
bool detect_motion_software(camera_fb_t* current_frame, camera_fb_t* previous_frame) {
    // Software-based motion detection
    uint32_t diff_pixels = 0;
    uint8_t* curr_data = current_frame->buf;
    uint8_t* prev_data = previous_frame->buf;
    
    for (size_t i = 0; i < current_frame->len; i += 10) { // Sample every 10th pixel
        int diff = abs(curr_data[i] - prev_data[i]);
        if (diff > MOTION_THRESHOLD) {
            diff_pixels++;
        }
    }
    
    return (diff_pixels > MIN_MOTION_PIXELS);
}
```

#### Time-lapse Photography
```cpp
void setup_timelapse() {
    // Configure for time-lapse capture
    camera_config_t config = {
        .framesize = FRAMESIZE_SXGA,     // 1280x1024
        .jpeg_quality = 8,               // High quality for time-lapse
        .auto_exposure = true
    };
    
    camera_universal_init(&config);
    
    // Set capture interval (e.g., every 5 minutes)
    esp_timer_create_args_t timer_args = {
        .callback = timelapse_capture_callback,
        .name = "timelapse_timer"
    };
    
    esp_timer_handle_t timer;
    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, 300000000); // 5 minutes in microseconds
}
```

### Deployment Considerations

#### Weatherproof Enclosure
- IP65 rated enclosure minimum
- Transparent window for camera lens
- Ventilation to prevent condensation
- Cable glands for power and antenna connections

#### Power Requirements
- **Normal Operation**: ~300mW
- **Deep Sleep**: ~10µW
- **Solar Panel**: 6V, 1W minimum
- **Battery**: 3.7V, 2000mAh Li-ion recommended

#### Mounting Options
- Tree mount with adjustable angle
- Post mount for open areas
- Camouflaged housing for wildlife acceptance
- Anti-theft security measures

#### Communication Range
- **WiFi**: 50-100m in open area
- **Cellular**: Depends on carrier coverage
- **LoRa**: 1-5km line of sight

### Troubleshooting

#### Common Issues
1. **Camera fails to initialize**
   - Check power supply (needs stable 3.3V)
   - Verify GPIO connections
   - Ensure sufficient current (min 500mA)

2. **Poor image quality**
   - Clean camera lens
   - Adjust focus if adjustable lens
   - Check for IR filter if needed

3. **High power consumption**
   - Enable power management features
   - Use deep sleep between captures
   - Reduce CPU frequency

4. **WiFi connectivity issues**
   - Check antenna connection
   - Verify signal strength
   - Use external antenna for better range

### Performance Optimization

#### Memory Management
```cpp
// Optimize JPEG buffer size
#define JPEG_BUFFER_SIZE 32768

// Use double buffering for smooth operation
camera_fb_t* frame_buffers[2];
int current_buffer = 0;
```

#### Image Processing
```cpp
// Resize image for transmission
bool resize_for_transmission(camera_fb_t* fb, uint8_t scale_factor) {
    // Implementation depends on requirements
    // Consider using ESP32 image processing capabilities
}
```

This ESP32-CAM setup provides a robust foundation for wildlife monitoring with excellent cost-effectiveness and expandability options.