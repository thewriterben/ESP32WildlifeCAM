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

All configuration parameters are centralized in `firmware/src/config.h`. The following table lists the main categories and options:

### Camera Configuration
| Parameter | Description | Default Value | Range/Options |
|-----------|-------------|---------------|---------------|
| `CAMERA_FRAME_SIZE` | Image resolution | `FRAMESIZE_UXGA` | 160x120 to 1600x1200 |
| `CAMERA_JPEG_QUALITY` | JPEG compression quality | `12` | 10-63 (lower = higher quality) |
| `CAMERA_FB_COUNT` | Frame buffer count | `2` | 1-3 |
| `CAMERA_BRIGHTNESS_DEFAULT` | Image brightness | `0` | -2 to 2 |
| `CAMERA_CONTRAST_DEFAULT` | Image contrast | `0` | -2 to 2 |
| `CAMERA_SATURATION_DEFAULT` | Color saturation | `0` | -2 to 2 |
| `AUTO_EXPOSURE_ENABLED` | Automatic exposure control | `true` | true/false |
| `AUTO_WHITE_BALANCE_ENABLED` | Automatic white balance | `true` | true/false |

### Motion Detection Configuration
| Parameter | Description | Default Value | Range/Options |
|-----------|-------------|---------------|---------------|
| `PIR_PIN` | PIR sensor GPIO pin | `13` | Valid GPIO pin |
| `PIR_DEBOUNCE_TIME` | Motion debounce time | `2000` | milliseconds |
| `MOTION_SENSITIVITY` | Motion detection sensitivity | `50` | 0-100 (higher = more sensitive) |
| `MOTION_CONSECUTIVE_THRESHOLD` | Required consecutive detections | `3` | 1-10 |
| `WEATHER_FILTERING_ENABLED` | Enable weather-based filtering | `true` | true/false |
| `WIND_THRESHOLD` | Maximum wind speed for detection | `15.0` | km/h |
| `RAIN_THRESHOLD` | Maximum rainfall for detection | `0.5` | mm/h |
| `TEMP_STABILITY_THRESHOLD` | Temperature stability threshold | `2.0` | °C |

### Power Management Configuration
| Parameter | Description | Default Value | Range/Options |
|-----------|-------------|---------------|---------------|
| `BATTERY_VOLTAGE_PIN` | Battery voltage ADC pin | `35` | Valid ADC pin |
| `SOLAR_VOLTAGE_PIN` | Solar panel voltage ADC pin | `34` | Valid ADC pin |
| `BATTERY_FULL_VOLTAGE` | Maximum battery voltage | `4.2` | Volts |
| `BATTERY_LOW_THRESHOLD` | Low battery warning | `3.0` | Volts |
| `BATTERY_CRITICAL_THRESHOLD` | Critical battery level | `2.8` | Volts |
| `SOLAR_CHARGING_VOLTAGE_MIN` | Minimum solar voltage for charging | `4.0` | Volts |
| `DEEP_SLEEP_DURATION` | Sleep duration between checks | `300` | seconds |
| `LOW_POWER_CPU_FREQ` | Reduced CPU frequency | `80` | MHz |

### LoRa Mesh Network Configuration
| Parameter | Description | Default Value | Range/Options |
|-----------|-------------|---------------|---------------|
| `LORA_ENABLED` | Enable LoRa functionality | `true` | true/false |
| `LORA_FREQUENCY` | Operating frequency | `915E6` | 433E6, 868E6, 915E6 Hz |
| `LORA_TX_POWER` | Transmission power | `20` | 5-20 dBm |
| `LORA_SPREADING_FACTOR` | LoRa spreading factor | `7` | 6-12 |
| `LORA_SIGNAL_BANDWIDTH` | Signal bandwidth | `125E3` | Hz |
| `NODE_ID` | Unique node identifier | `1` | 1-255 |
| `MAX_MESH_NODES` | Maximum nodes in mesh | `10` | 1-50 |
| `MESH_RETRY_COUNT` | Message retry attempts | `3` | 1-10 |

### WiFi Configuration
| Parameter | Description | Default Value | Range/Options |
|-----------|-------------|---------------|---------------|
| `WIFI_ENABLED` | Enable WiFi functionality | `false` | true/false |
| `WIFI_SSID` | Network SSID | `""` | String |
| `WIFI_PASSWORD` | Network password | `""` | String |
| `WIFI_TIMEOUT` | Connection timeout | `10000` | milliseconds |
| `WIFI_RETRY_COUNT` | Connection retry attempts | `3` | 1-10 |

### Trigger and Timing Configuration
| Parameter | Description | Default Value | Range/Options |
|-----------|-------------|---------------|---------------|
| `TRIGGER_ACTIVE_HOURS_START` | Active period start hour | `6` | 0-23 |
| `TRIGGER_ACTIVE_HOURS_END` | Active period end hour | `20` | 0-23 |
| `MAX_DAILY_TRIGGERS` | Maximum daily image captures | `50` | 1-1000 |
| `TRIGGER_COOLDOWN_PERIOD` | Minimum time between triggers | `5000` | milliseconds |
| `STATUS_REPORT_INTERVAL` | System status reporting interval | `300000` | milliseconds (5 min) |

### Environmental Sensors Configuration
| Parameter | Description | Default Value | Range/Options |
|-----------|-------------|---------------|---------------|
| `BME280_ENABLED` | Enable BME280 weather sensor | `false` | true/false |
| `BME280_ADDRESS` | I2C address | `0x76` | 0x76 or 0x77 |
| `BME280_SDA` | I2C data pin | `21` | Valid GPIO pin |
| `BME280_SCL` | I2C clock pin | `22` | Valid GPIO pin |

## 🏗️ Architecture Overview

The ESP32WildlifeCAM firmware follows a modular, object-oriented architecture with clear separation of concerns:

### System Architecture Diagram
```
┌─────────────────────────────────────────────────────────┐
│                    SystemManager                        │
│  (Central Coordinator & Main Loop Management)          │
└─────────────────┬───────────────────────────────────────┘
                  │
     ┌────────────┼────────────┐
     │            │            │
┌────▼────┐  ┌───▼────┐  ┌────▼─────┐  ┌──────────┐
│Camera   │  │Motion  │  │Power     │  │WiFi      │
│Handler  │  │Filter  │  │Manager   │  │Manager   │
└─────────┘  └────────┘  └──────────┘  └──────────┘
     │            │            │            │
┌────▼────┐  ┌───▼────┐  ┌────▼─────┐  ┌──▼─────────┐
│HAL      │  │Weather │  │Solar     │  │Network     │
│Boards   │  │Sensors │  │Charging  │  │Discovery   │
└─────────┘  └────────┘  └──────────┘  └────────────┘
```

### Core Subsystems

#### 1. SystemManager
- **Purpose**: Central coordinator for all wildlife camera subsystems
- **Key Features**:
  - Unified initialization sequence with proper error handling
  - Coordinated update cycle for all subsystems
  - Resource management and cleanup
  - Deep sleep and power state management
- **Configuration Dependencies**: All subsystem configurations

#### 2. CameraHandler
- **Purpose**: Manages ESP32 camera initialization, configuration, and image capture
- **Key Features**:
  - Hardware Abstraction Layer (HAL) for different board types
  - Automatic board detection and optimization
  - Wildlife-optimized sensor settings
  - Image capture with metadata generation
  - SD card storage with timestamp filenames
- **Configuration Dependencies**: Camera, image processing, and storage settings

#### 3. MotionFilter  
- **Purpose**: Intelligent motion detection with environmental filtering
- **Key Features**:
  - PIR sensor interrupt handling
  - Weather-based motion filtering (wind, rain, temperature)
  - Consecutive motion validation
  - Configurable sensitivity and thresholds
  - Statistics tracking and reporting
- **Configuration Dependencies**: Motion detection, weather filtering, and environmental sensor settings

#### 4. PowerManager (formerly SolarManager)
- **Purpose**: Solar power management, battery monitoring, and power optimization
- **Key Features**:
  - Real-time battery and solar voltage monitoring
  - Charging control and optimization
  - Power state management (Normal, Good, Low, Critical)
  - CPU frequency scaling for power saving
  - Battery life estimation and statistics
- **Configuration Dependencies**: Power management, ADC, and battery threshold settings

#### 5. WiFiManager
- **Purpose**: WiFi connectivity and network operations management
- **Key Features**:
  - Automatic connection and reconnection handling
  - Network scanning and discovery
  - Access Point mode for configuration
  - Power save mode support
  - Connection status monitoring and reporting
- **Configuration Dependencies**: WiFi and network settings

### Subsystem Interactions

1. **SystemManager** orchestrates all subsystems and handles the main application logic
2. **MotionFilter** triggers **CameraHandler** when valid motion is detected
3. **PowerManager** influences system behavior based on power state:
   - Low power: Reduces camera quality, extends sleep duration
   - Critical power: Enters emergency deep sleep mode
4. **WiFiManager** provides network connectivity for:
   - Time synchronization (NTP)
   - Remote configuration
   - Status reporting
5. **CameraHandler** works with **PowerManager** to optimize image capture based on available power

### Configuration System

All subsystems use centralized configuration from `config.h`:
- **Compile-time configuration**: All parameters defined as preprocessor macros
- **Categorized organization**: Settings grouped by functionality
- **Comprehensive documentation**: Each parameter includes description and valid ranges
- **Validation macros**: Built-in range checking and value clamping
- **Future-ready**: Foundation for runtime configuration capability

### Error Handling and Resource Management

- **RAII Pattern**: All subsystem classes follow Resource Acquisition Is Initialization
- **Graceful Degradation**: System continues operation even if non-critical subsystems fail
- **Resource Cleanup**: Proper cleanup on system shutdown or deep sleep
- **Error Propagation**: Initialization failures are properly reported and handled
- **Backward Compatibility**: Legacy namespace wrappers maintain compatibility with existing code

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

## 🚀 Recent Improvements (v1.0.0)

### Firmware Architecture Refactoring
- **Object-Oriented Design**: Converted from procedural code to proper C++ classes
- **Centralized Configuration**: All settings moved to `config.h` with comprehensive documentation
- **SystemManager**: New coordinator class eliminates global variables and provides clean initialization
- **Modular Subsystems**: CameraHandler, MotionFilter, PowerManager, and WiFiManager classes
- **Resource Management**: RAII patterns with proper constructors, destructors, and cleanup
- **Error Handling**: Graceful degradation and proper error propagation throughout the system

### New Features
- **WiFi Management**: Complete network connectivity with auto-reconnection and AP mode
- **Enhanced Power Management**: Intelligent power state management with CPU frequency scaling
- **Advanced Motion Filtering**: Weather-based filtering with environmental sensor integration
- **Configuration Tables**: Comprehensive documentation of all configuration parameters
- **Architecture Documentation**: Detailed system overview with subsystem interactions

### Future-Ready Foundation
- **Runtime Configuration**: Framework prepared for dynamic configuration changes
- **Logging System**: Foundation laid for comprehensive system logging
- **Message Buffering**: Infrastructure ready for improved communication reliability
- **Extensibility**: Clean interfaces for adding new sensors and communication methods

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
