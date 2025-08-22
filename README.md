# ESP32 Wildlife Camera Project

A solar-powered wildlife trail camera system built with ESP32-S3, featuring intelligent motion detection, LoRa mesh networking, and weatherproof design.

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

## 📷 Supported Camera Boards

The ESP32WildlifeCAM project supports **15 different ESP32 camera boards**, making it compatible with the majority of popular ESP32 camera modules available in the market.

### Fully Supported Boards ✅

| Board | Chip | PSRAM | Features | Power Profile |
|-------|------|-------|----------|---------------|
| **AI-Thinker ESP32-CAM** | ESP32 | Optional | Most popular, good all-around | 120mA active, 10mA sleep |
| **ESP32-S3-CAM** | ESP32-S3 | Yes | High performance, AI ready | 100mA active, 7mA sleep |
| **ESP-EYE** | ESP32 | Yes | Espressif official, OLED display | 110mA active, 8mA sleep |
| **M5Stack Timer CAM** | ESP32 | Optional | Battery optimized, external power | 90mA active, 5mA sleep |
| **TTGO T-Camera** | ESP32 | Yes | Built-in display, compact | 95mA active, 6mA sleep |
| **XIAO ESP32S3 Sense** | ESP32-S3 | Yes | Ultra-compact, microphone | 80mA active, 4mA sleep |
| **FireBeetle ESP32-CAM** | ESP32 | Optional | **Extreme low power design** | 70mA active, 3mA sleep |
| **ESP32-S3-EYE** | ESP32-S3 | Yes | Advanced AI vision board | 100mA active, 5mA sleep |
| **Freenove ESP32-WROVER-CAM** | ESP32 | Yes | Educational favorite, well documented | 110mA active, 8mA sleep |
| **M5Stack ESP32CAM** | ESP32 | Optional | Stackable modular design | 95mA active, 6mA sleep |

**📖 For detailed hardware specifications and deployment guides, see [docs/deployment/](docs/deployment/README.md)**
| **TTGO T-Journal** | ESP32 | Yes | E-paper display integration | 120mA active, 8mA sleep |
| **LilyGO T-Camera Plus** | ESP32 | Yes | Enhanced features | 90mA active, 5mA sleep |
| **Espressif ESP32-CAM-MB** | ESP32 | Optional | Official development board | 115mA active, 9mA sleep |
| **M5Stack UnitCAM** | ESP32 | Optional | **Tiny form factor** for stealth | 70mA active, 3mA sleep |
| **TTGO T-Camera V1.7** | ESP32 | Yes | Latest version with improvements | 85mA active, 4mA sleep |

### Wildlife-Specific Optimizations

Each board implementation includes optimizations specifically for wildlife monitoring:

#### 🔋 **Power Management**
- **Extended Sleep Modes**: 3µA to 10mA sleep current depending on board
- **Solar Integration**: Automatic charging management and voltage monitoring
- **Wake Sources**: PIR motion sensors, timer-based, or manual triggers
- **CPU Scaling**: Dynamic frequency adjustment (40MHz to 240MHz)

#### 📸 **Camera Settings**
- **Outdoor Optimized**: Auto-exposure and white balance for varying light
- **Motion Blur Reduction**: Fast shutter speeds for moving wildlife
- **Night Photography**: High gain settings and flash/LED control
- **Storage Efficient**: JPEG quality balanced for file size vs detail

#### 🌐 **Network & Communication**
- **LoRa Mesh**: Long-range communication for remote deployment areas
- **Satellite Ready**: Integration points for satellite communication
- **Local Storage**: SD card and flash memory management
- **Data Compression**: Image and telemetry compression algorithms

#### 🏞️ **Environmental Features**
- **Weather Filtering**: Motion detection ignoring wind/rain false positives
- **Temperature Monitoring**: Sensor health and environmental logging
- **Weatherproof Design**: IP65+ rated enclosure support
- **Solar Tracking**: Optimal panel positioning feedback

### Board Selection Guide

**For Maximum Battery Life**: FireBeetle ESP32-CAM, M5Stack UnitCAM
**For AI/ML Features**: ESP32-S3-EYE, ESP32-S3-CAM, XIAO ESP32S3 Sense  
**For Stealth Monitoring**: XIAO ESP32S3 Sense, M5Stack UnitCAM
**For Development/Testing**: AI-Thinker ESP32-CAM, ESP-EYE
**For Remote Deployment**: M5Stack Timer CAM, LilyGO T-Camera Plus
**For Educational Use**: Freenove ESP32-WROVER-CAM, ESP-EYE

### Automatic Board Detection

The system automatically detects the connected board type using:
- **GPIO Fingerprinting**: Tests board-specific pin configurations
- **PSRAM Detection**: Checks for external PSRAM availability  
- **Chip Model Identification**: ESP32 vs ESP32-S3 detection
- **I2C Device Scanning**: Identifies board-specific sensors/displays
- **Fallback Logic**: Graceful degradation to compatible board types

## 🎯 Deployment Scenarios

The ESP32WildlifeCAM supports five major deployment scenarios, each optimized for different use cases:

### 🔬 Research Deployment
**Large-scale scientific monitoring (50+ nodes)**
- High-resolution image capture for detailed analysis
- AI-powered species classification and behavior analysis
- Multi-hop mesh networking with data aggregation
- Integration with research databases and publications
- **Cost**: $150-200 per node | **Battery Life**: 60+ days

### 💰 Budget Conservation
**Cost-effective community monitoring**
- Simplified hardware using AI-Thinker ESP32-CAM
- Basic motion detection with local storage
- 3D printable enclosures and DIY solar systems
- Community-friendly setup and maintenance
- **Cost**: $60-80 per node | **Battery Life**: 30-45 days

### 🏙️ Urban Monitoring
**Stealth urban wildlife tracking**
- Advanced filtering for urban false positives
- Anti-vandalism features and concealed installation
- WiFi/cellular connectivity for real-time data
- Encrypted storage and secure transmission
- **Cost**: $120-180 per node | **Battery Life**: 25-35 days

### 🌨️ Extreme Environment
**Arctic/desert monitoring with satellite backup**
- Enhanced weatherproofing for harsh conditions
- Extended battery systems and thermal management
- Satellite communication for remote locations
- Survival mode operation for extended autonomy
- **Cost**: $200-300 per node | **Battery Life**: 45+ days

### 🤖 AI-Enhanced Research
**Cutting-edge AI processing and analysis**
- Real-time species classification and behavior analysis
- Edge computing with cloud-assisted processing
- Continuous learning and model improvement
- Advanced conservation alerts and insights
- **Cost**: $250-400 per node | **Processing**: Real-time AI

**📋 Detailed deployment guides: [docs/deployment/scenarios/](docs/deployment/scenarios/)**

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

### Environmental Sensors Configuration
| Parameter | Description | Default Value | Range/Options |
|-----------|-------------|---------------|---------------|
| `WEATHER_SENSOR_ENABLED` | Enable weather sensor | `false` | true/false |
| `BME280_I2C_ADDRESS` | BME280 sensor I2C address | `0x76` | 0x76, 0x77 |
| `TEMPERATURE_OFFSET` | Temperature calibration offset | `0.0` | °C |
| `HUMIDITY_OFFSET` | Humidity calibration offset | `0.0` | % |

### Timing and Trigger Configuration
| Parameter | Description | Default Value | Range/Options |
|-----------|-------------|---------------|---------------|
| `STARTUP_DELAY` | Initial startup delay | `5000` | milliseconds |
| `CAMERA_WARMUP_TIME` | Camera initialization time | `2000` | milliseconds |
| `CAPTURE_INTERVAL_MIN` | Minimum time between captures | `30` | seconds |
| `MAX_DAILY_CAPTURES` | Maximum captures per day | `100` | 1-1000 |
| `NIGHT_MODE_ENABLED` | Enable night mode operation | `true` | true/false |
| `QUIET_HOURS_START` | Start of quiet hours | `22` | 0-23 (hour) |
| `QUIET_HOURS_END` | End of quiet hours | `6` | 0-23 (hour) |

## 🏗️ Architecture Overview

This firmware implements a modern object-oriented architecture with clear separation of concerns and centralized configuration management.

### System Components

```
┌─────────────────────────────────────────────────────────────┐
│                     SystemManager                          │
├─────────────────────────────────────────────────────────────┤
│  • Central coordinator and state management                │
│  • Handles subsystem initialization and coordination       │
│  • Manages main update loop and error handling            │
└─┬───────────────────────────────────────────────────────┬───┘
  │                                                       │
┌─▼─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────▼─────┐
│ CameraHandler │ │MotionFilter │ │PowerManager │ │WiFiManager│
├───────────────┤ ├─────────────┤ ├─────────────┤ ├───────────┤
│• Image capture│ │• PIR sensor │ │• Battery    │ │• Network  │
│• HAL abstraction│ │• Weather   │ │  monitoring │ │  connectivity│
│• Format handling│ │  filtering  │ │• Solar      │ │• AP mode  │
│• Quality control│ │• Debouncing │ │  charging   │ │• Power    │
└───────────────┘ └─────────────┘ └─────────────┘ └───────────┘
```

### Key Architecture Features

- **Centralized Configuration**: All parameters consolidated in `config.h` with comprehensive documentation
- **Class-Based Design**: Each major subsystem implemented as a C++ class with proper encapsulation
- **RAII Pattern**: Automatic resource management with constructors, destructors, and proper cleanup
- **Error Handling**: Graceful degradation and comprehensive error reporting
- **Legacy Compatibility**: Namespace wrappers maintain backward compatibility
- **Extensible Design**: Well-defined interfaces for adding new functionality

### Benefits of the New Architecture

- **Maintainability**: Clear separation of concerns and encapsulation
- **Extensibility**: Well-defined interfaces for adding new functionality
- **Configuration Management**: Single source of truth for all system parameters
- **Code Quality**: Modern C++ patterns with proper resource management
- **Error Propagation**: Initialization failures are properly reported and handled
- **Backward Compatibility**: Legacy namespace wrappers maintain compatibility with existing code

## 🔍 Advanced Features

### Weather-Based Motion Filtering
The system includes intelligent weather-based motion filtering to reduce false positives:

- **Wind Detection**: Filters out motion during high wind conditions
- **Rain Filtering**: Reduces sensitivity during precipitation
- **Temperature Stability**: Accounts for thermal fluctuations
- **Adaptive Thresholds**: Adjusts sensitivity based on environmental conditions

### Mesh Networking Architecture
The LoRa mesh network enables distributed camera deployments:

- **Multi-hop Routing**: Messages can traverse multiple nodes
- **Automatic Discovery**: Nodes automatically discover neighbors
- **Load Balancing**: Traffic distributed across available paths
- **Fault Tolerance**: Network self-heals when nodes fail

### Power Optimization Strategies
Advanced power management for extended deployment:

- **Dynamic CPU Scaling**: Reduces processor speed during idle periods
- **Selective Component Power**: Powers down unused peripherals
- **Solar Tracking**: Optimizes charging based on available solar energy
- **Predictive Sleep**: Adjusts sleep duration based on activity patterns

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
