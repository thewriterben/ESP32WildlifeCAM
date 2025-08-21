# ESP32 Wildlife Camera Project

A comprehensive solar-powered wildlife trail camera system built with ESP32-S3, featuring intelligent motion detection, LoRa mesh networking, and weatherproof design.

## 🌟 Features

- **ESP32-S3** with high-resolution camera module
- **Solar-powered** using repurposed solar path lights
- **LoRa mesh networking** for remote deployment
- **Weather motion filtering** to eliminate false positives
- **3D printable** weatherproof enclosure
- **Low power consumption** with deep sleep optimization
- **Modular architecture** for easy customization

## 📋 Hardware Requirements

### Core Components
- ESP32-S3 development board with PSRAM
- OV2640 or OV5640 camera module
- LoRa radio module (SX1276 or SX1262)
- Solar panel (5V, 2W minimum)
- 18650 Li-ion battery (3000mAh+)
- TP4056 charging module
- PIR motion sensor
- Weather sensors (optional: BME280)

### Power Management
- Solar panel: 5V/2W (repurposed path light)
- Battery: 18650 Li-ion 3.7V 3000mAh
- Charging: TP4056 with protection
- Power consumption: <100µA in sleep mode

## 🚀 Quick Start

### 1. Hardware Setup
```bash
# Clone the repository
git clone https://github.com/thewriterben/ESP32WildlifeCAM.git
cd ESP32WildlifeCAM

# Review hardware schematics
open hardware/schematics/main_circuit.pdf
```

### 2. Firmware Installation
```bash
# Install PlatformIO
pip install platformio

# Build and upload firmware
cd firmware
pio run --target upload
```

### 3. Configuration
```cpp
// Edit firmware/src/config.h
#define CAMERA_MODEL_AI_THINKER
#define ENABLE_LORA_MESH true
#define SOLAR_VOLTAGE_THRESHOLD 3.2
```

## 📁 Project Structure

```
├── README.md                    # This file
├── firmware/                    # ESP32 firmware
│   ├── src/                     # Source code
│   ├── lib/                     # Libraries
│   └── platformio.ini           # Build configuration
├── hardware/                    # Circuit designs
│   ├── schematics/              # Circuit diagrams
│   ├── pcb/                     # PCB layouts
│   └── bom.md                   # Bill of materials
├── 3d_models/                   # 3D printable parts
│   ├── enclosure/               # Main housing
│   ├── mounting/                # Mounting brackets
│   └── README.md                # Printing guidelines
├── docs/                        # Documentation
│   ├── setup_guide.md           # Setup instructions
│   ├── api_reference.md         # Code documentation
│   └── troubleshooting.md       # Common issues
└── examples/                    # Example projects
    ├── basic_camera/            # Simple camera setup
    ├── solar_test/              # Power testing
    └── mesh_network/            # LoRa networking
```

## 🔧 Configuration Options

All configuration parameters are centralized in `firmware/src/config.h` for easy customization. The following sections detail the available configuration options:

### Camera Settings
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `CAMERA_FRAME_SIZE` | `FRAMESIZE_UXGA` | Various | Camera resolution (160x120 to 1600x1200) |
| `CAMERA_JPEG_QUALITY` | `12` | 10-63 | JPEG compression quality (lower = higher quality) |
| `CAMERA_FB_COUNT` | `2` | 1-4 | Number of frame buffers |
| `CAMERA_LED_PIN` | `4` | GPIO | Built-in LED pin for AI-Thinker ESP32-CAM |

### Motion Detection
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `PIR_PIN` | `13` | GPIO | PIR motion sensor input pin |
| `PIR_DEBOUNCE_TIME` | `2000` | ms | Prevent multiple triggers within this time |
| `MOTION_DETECTION_ENABLED` | `true` | bool | Enable/disable motion detection |
| `MOTION_SENSITIVITY` | `50` | 0-100 | PIR sensitivity (higher = more sensitive) |
| `MOTION_CONSECUTIVE_THRESHOLD` | `3` | 1-10 | Consecutive motions needed for valid detection |
| `DEFAULT_TEMPERATURE` | `20.0` | °C | Default temperature for motion compensation |

### Power Management
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `SOLAR_VOLTAGE_PIN` | `34` | ADC GPIO | ADC pin for solar voltage monitoring |
| `BATTERY_VOLTAGE_PIN` | `35` | ADC GPIO | ADC pin for battery voltage monitoring |
| `SOLAR_VOLTAGE_THRESHOLD` | `3.2` | V | Minimum solar voltage for charging |
| `BATTERY_LOW_THRESHOLD` | `3.0` | V | Low battery warning threshold |
| `BATTERY_CRITICAL_THRESHOLD` | `2.8` | V | Critical battery shutdown threshold |
| `DEEP_SLEEP_DURATION` | `300` | seconds | Sleep duration between checks |
| `CHARGING_LED_PIN` | `2` | GPIO | Charging indicator LED pin |
| `NORMAL_CPU_FREQUENCY` | `240` | MHz | Normal operation CPU frequency |
| `POWER_SAVE_CPU_FREQUENCY` | `80` | MHz | Power saving mode CPU frequency |

### Timing Configuration
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `VOLTAGE_CHECK_INTERVAL` | `5000` | ms | Interval for reading battery/solar voltages |
| `POWER_LOG_INTERVAL` | `60000` | ms | Interval for logging power status |
| `SENSOR_READ_INTERVAL` | `1000` | ms | Interval for reading environmental sensors |
| `HEARTBEAT_INTERVAL` | `30000` | ms | Interval for sending network heartbeats |
| `SERIAL_INIT_DELAY` | `1000` | ms | Delay for serial port initialization |
| `NETWORK_RETRY_DELAY` | `2000` | ms | Delay between network transmission attempts |
| `COMMUNICATION_SETUP_DELAY` | `5000` | ms | Delay for communication module setup |

### LoRa Mesh Network
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `LORA_ENABLED` | `true` | bool | Enable LoRa mesh networking |
| `LORA_FREQUENCY` | `915E6` | Hz | LoRa frequency (433E6, 868E6, 915E6) |
| `LORA_TX_POWER` | `20` | dBm | Transmission power (5-20 dBm) |
| `LORA_SPREADING_FACTOR` | `7` | 6-12 | LoRa spreading factor |
| `LORA_SIGNAL_BANDWIDTH` | `125E3` | Hz | Signal bandwidth |
| `LORA_CODING_RATE` | `5` | 5-8 | Error correction coding rate |
| `LORA_PREAMBLE_LENGTH` | `8` | 6-65535 | Preamble length |
| `LORA_SYNC_WORD` | `0x12` | hex | Network identification word |
| `LORA_MESSAGE_QUEUE_SIZE` | `10` | count | Maximum queued outgoing messages |
| `LORA_TRANSMISSION_DELAY` | `1000` | ms | Simulated transmission delay |

### Satellite Communication
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `SATELLITE_BAUD_RATE` | `19200` | baud | Serial baud rate for satellite module |
| `SATELLITE_RETRY_COUNT` | `5` | count | Number of retry attempts for commands |
| `SATELLITE_MESSAGE_MAX_LENGTH` | `100` | bytes | Maximum bytes per satellite message |
| `SATELLITE_RESPONSE_DELAY` | `2000` | ms | Delay waiting for module response |
| `SATELLITE_WAKEUP_DELAY` | `100` | ms | Delay for module wake up sequence |
| `SAT_SLEEP_PIN` | `12` | GPIO | Satellite module sleep control pin |
| `SAT_RING_PIN` | `13` | GPIO | Satellite module ring indicator pin |

### Hardware Configuration
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `ADC_RESOLUTION` | `12` | bits | ADC resolution (8, 10, 11, 12 bits) |
| `VOLTAGE_DIVIDER_RATIO` | `2.0` | ratio | Voltage divider circuit ratio |
| `ADC_REFERENCE_VOLTAGE` | `3.3` | V | Reference voltage for ADC measurements |
| `VOLTAGE_CALIBRATION_SAMPLES` | `10` | count | Samples for voltage averaging |

### Weather Filtering
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `WEATHER_FILTERING_ENABLED` | `true` | bool | Enable weather-based motion filtering |
| `WIND_THRESHOLD` | `15` | km/h | Ignore motion above this wind speed |
| `RAIN_THRESHOLD` | `0.5` | mm/h | Ignore motion during rain |
| `TEMP_COMP_ENABLED` | `true` | bool | Temperature compensation for PIR |

### File System
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `SD_CS_PIN` | `12` | GPIO | SD card chip select pin |
| `SD_MOSI_PIN` | `15` | GPIO | SD card MOSI pin |
| `SD_CLK_PIN` | `14` | GPIO | SD card clock pin |
| `SD_MISO_PIN` | `2` | GPIO | SD card MISO pin |
| `IMAGE_FOLDER` | `"/images"` | path | Folder for captured images |
| `LOG_FOLDER` | `"/logs"` | path | Folder for system logs |
| `MAX_FILES_PER_DAY` | `100` | count | Maximum images per day |

### Image Processing
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `IMAGE_TIMESTAMP_ENABLED` | `true` | bool | Add timestamp to images |
| `IMAGE_COMPRESSION_ENABLED` | `true` | bool | Enable image compression for transmission |
| `THUMBNAIL_ENABLED` | `true` | bool | Generate thumbnails for quick preview |
| `THUMBNAIL_SIZE` | `160` | pixels | Thumbnail width in pixels |
| `RLE_MAX_COUNT` | `255` | count | Maximum run-length encoding count |
| `COMPRESSION_MIN_EFFICIENCY` | `3` | bytes | Minimum bytes for RLE compression |

### Trigger Settings
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `TRIGGER_ACTIVE_HOURS_START` | `6` | hour | Start of active period (24h format) |
| `TRIGGER_ACTIVE_HOURS_END` | `20` | hour | End of active period (24h format) |
| `NIGHT_MODE_ENABLED` | `false` | bool | Enable night photography |
| `MAX_DAILY_TRIGGERS` | `50` | count | Maximum triggers per day |

### Memory and Buffer Configuration
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `BUFFER_SIZE_SMALL` | `256` | bytes | Small buffer size for general use |
| `BUFFER_SIZE_MEDIUM` | `1024` | bytes | Medium buffer size for general use |
| `BUFFER_SIZE_LARGE` | `4096` | bytes | Large buffer size for general use |
| `JSON_BUFFER_SMALL` | `256` | bytes | Small JSON document buffer |
| `JSON_BUFFER_MEDIUM` | `512` | bytes | Medium JSON document buffer |
| `JSON_BUFFER_LARGE` | `1024` | bytes | Large JSON document buffer |

### Debug and Logging
| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `DEBUG_ENABLED` | `true` | bool | Enable debug output |
| `SERIAL_BAUD_RATE` | `115200` | baud | Serial monitor baud rate |
| `CURRENT_LOG_LEVEL` | `LOG_LEVEL_INFO` | level | Current logging verbosity |

### Customization Examples

```cpp
// Example: High sensitivity setup for small animals
#define MOTION_SENSITIVITY 80
#define PIR_DEBOUNCE_TIME 1000
#define TRIGGER_ACTIVE_HOURS_START 0  // 24/7 monitoring
#define TRIGGER_ACTIVE_HOURS_END 24

// Example: Power-optimized setup for remote deployment
#define VOLTAGE_CHECK_INTERVAL 10000   // Check less frequently
#define POWER_LOG_INTERVAL 300000      // Log every 5 minutes
#define DEEP_SLEEP_DURATION 600        // Sleep longer between checks
#define POWER_SAVE_CPU_FREQUENCY 40    // Lower CPU frequency

// Example: High-quality imaging setup
#define CAMERA_JPEG_QUALITY 8          // Higher quality images
#define CAMERA_FB_COUNT 3              // More frame buffers
#define THUMBNAIL_SIZE 320             // Larger thumbnails
```

## 🌦️ Weather Resistance

The system is designed for outdoor deployment with:
- IP65-rated enclosure
- Temperature range: -20°C to +60°C
- Humidity protection
- UV-resistant materials
- Condensation prevention

## 📊 Performance

- **Battery life**: 30+ days on single charge
- **Detection range**: 5-12 meters
- **Image quality**: Up to 5MP
- **Network range**: 1-10km (depending on terrain)
- **Storage**: MicroSD card up to 32GB

## 🔧 Development

### Building from Source
```bash
# Install dependencies
pio lib install

# Build firmware
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor
```

### Testing
```bash
# Run hardware tests
cd examples/solar_test
pio run --target upload

# Test mesh network
cd examples/mesh_network
pio run --target upload
```

## 📖 Documentation

- [Setup Guide](docs/setup_guide.md) - Complete installation instructions
- [API Reference](docs/api_reference.md) - Code documentation
- [Troubleshooting](docs/troubleshooting.md) - Common issues and solutions
- [3D Printing Guide](3d_models/README.md) - Enclosure fabrication

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- ESP32 community for hardware support
- LoRa Alliance for mesh networking protocols
- OpenSource hardware community
- Wildlife conservation organizations

## 📞 Support

- GitHub Issues: [Report bugs](https://github.com/thewriterben/ESP32WildlifeCAM/issues)
- Documentation: [Wiki](https://github.com/thewriterben/ESP32WildlifeCAM/wiki)
- Community: [Discussions](https://github.com/thewriterben/ESP32WildlifeCAM/discussions)

---

**Built with ❤️ for wildlife conservation**
