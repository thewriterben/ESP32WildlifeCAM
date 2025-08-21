# ESP32WildlifeCAM
ESP32 CAM Solar Powered Wireless Wildlife Monitoring System

## Universal ESP32 Camera Board Support

This project provides comprehensive support for all major ESP32 camera boards, creating a universal platform for wildlife monitoring that works with any ESP32-based camera hardware.

### Supported ESP32 Camera Boards

- **ESP32-CAM** (AI-Thinker) - Most popular, affordable option
- **ESP32-S3-CAM** - Latest with improved performance and dual-core
- **ESP-EYE** (Espressif official) - Advanced computer vision capabilities  
- **M5Stack Timer Camera** - Compact with built-in battery
- **TTGO T-Camera** - ESP32-WROVER with OV2640
- **XIAO ESP32S3 Sense** - Ultra-compact with camera

### Supported Camera Sensors

- **OV2640** (2MP) - Most common, good power efficiency
- **OV3660** (3MP) - Higher resolution option
- **OV5640** (5MP) - Professional quality imaging
- **GC032A** (VGA) - Ultra-low power option
- **OV7670** (VGA) - Classic CMOS sensor
- **HM01B0** (QVGA) - Ultra-low power monochrome

## Features

- **Automatic Board Detection** - Detects and configures ESP32 camera boards automatically
- **Universal Camera API** - Single interface works with all supported sensors
- **Power Management** - Optimized for solar-powered remote deployments
- **Multi-Board Build System** - PlatformIO environments for each board type
- **Wildlife Optimized** - Configurations tuned for wildlife monitoring

## Quick Start

1. **Clone the repository**
   ```bash
   git clone https://github.com/thewriterben/ESP32WildlifeCAM.git
   cd ESP32WildlifeCAM
   ```

2. **Build for your board**
   ```bash
   # For ESP32-CAM (AI-Thinker)
   pio run -e esp32cam_wildlife
   
   # For ESP32-S3-CAM
   pio run -e esp32s3cam_wildlife
   
   # For ESP-EYE
   pio run -e espeye_wildlife
   ```

3. **Upload to your board**
   ```bash
   pio run -e esp32cam_wildlife --target upload
   ```

## Project Structure

```
firmware/
├── boards/          # Board-specific configurations
│   ├── esp32_cam/
│   ├── esp32_s3_cam/
│   ├── esp_eye/
│   └── ...
├── sensors/         # Camera sensor drivers
│   ├── ov2640/
│   ├── ov3660/
│   └── ...
└── src/            # Universal hardware abstraction layer

examples/
├── board_detection/     # Automatic board detection example
├── multi_board_support/ # Multi-board compatibility examples
└── board_specific_examples/

hardware/
├── board_specific/      # Integration guides for each board
└── camera_sensors/      # Sensor comparison and selection guide
```

## Example Usage

```cpp
#include "firmware/src/board_abstraction.h"
#include "firmware/src/camera_universal.h"

void setup() {
    // Automatic board detection and configuration
    board_type_t board = board_auto_detect();
    board_configure(board);
    
    // Initialize camera with wildlife-optimized settings
    camera_config_t config = {
        .framesize = FRAMESIZE_UXGA,
        .pixel_format = PIXFORMAT_JPEG,
        .jpeg_quality = 10,
        .auto_exposure = true
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
    
    delay(60000); // 1 minute between captures
}
```

## Documentation

- [ESP32-CAM Integration Guide](hardware/board_specific/esp32_cam_integration.md)
- [Camera Sensor Comparison](hardware/camera_sensors/sensor_comparison.md)
- [Board Detection Example](examples/board_detection/)

## Deployment Scenarios

### Trail Camera Setup
- Long-term wildlife monitoring
- Solar-powered remote operation
- Cellular or LoRa data transmission
- Motion-triggered capture

### Research Installation  
- High-resolution species documentation
- Continuous monitoring capabilities
- Multi-sensor configurations
- Professional data logging

### Educational Projects
- Wildlife observation learning
- Camera technology demonstration
- Solar power education
- IoT connectivity examples

## Contributing

Contributions welcome! Please see our contributing guidelines for:
- Adding support for new ESP32 camera boards
- Implementing additional camera sensors
- Improving power management features
- Adding new wildlife monitoring capabilities

## License

This project is open source under the MIT License.
