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

### Camera Settings
- Resolution: 160x120 to 1600x1200
- Quality: 10-63 (JPEG compression)
- Frame rate: 1-25 fps
- Special effects: None, Negative, Grayscale, etc.

### Motion Detection
- PIR sensitivity adjustment
- Time-based filtering
- Weather compensation
- Configurable trigger zones

### Power Management
- Solar charging optimization
- Battery voltage monitoring
- Deep sleep scheduling
- Power-saving modes

### LoRa Mesh Network
- Frequency: 433/868/915 MHz
- Range: Up to 10km line-of-sight
- Mesh topology with auto-routing
- Image compression and transmission

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
