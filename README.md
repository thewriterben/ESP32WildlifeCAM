# 🦌 ESP32WildlifeCAM

**Solar-Powered Wireless Wildlife Monitoring System**

Created by: @thewriterben  
Date: 2025-08-31  
Version: 0.1.0

## 🎯 Overview

ESP32WildlifeCAM is an open-source wildlife monitoring system that uses ESP32 microcontrollers to create a network of autonomous camera nodes. The system features:

- 📸 Motion-triggered photography
- 🤖 On-device AI for species identification  
- 🔗 LoRa mesh networking
- ☀️ Solar power with battery backup
- 🔐 Blockchain-verified image authenticity
- 📡 Remote monitoring and OTA updates

## 📋 Planning & Development

**🔗 UNIFIED DEVELOPMENT PLAN**: All planning documents have been consolidated into [`UNIFIED_DEVELOPMENT_PLAN.md`](UNIFIED_DEVELOPMENT_PLAN.md)

- **58+ TODOs integrated** from scattered codebase comments
- **Step-by-step implementation guide** with clear priorities  
- **Offline development optimized** for firewall constraints
- **Critical path identified**: Camera driver implementation (Step 1.4)

See [`PLANNING_CONSOLIDATION_README.md`](PLANNING_CONSOLIDATION_README.md) for details on what changed.

## 🚀 Quick Start

### Prerequisites

1. **Hardware**
   - ESP32-CAM board (AI-Thinker recommended)
   - LoRa module (SX1276/SX1278)
   - PIR motion sensor
   - Solar panel (5W minimum)
   - LiPo battery (3000mAh+)
   - SD card (16GB+)

2. **Software**
   - PlatformIO IDE
   - Git
   - Python 3.8+

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/thewriterben/ESP32WildlifeCAM.git
   cd ESP32WildlifeCAM
   ```

2. **Set up development environment**
   ```bash
   # Run the setup script
   chmod +x setup_project.sh
   ./setup_project.sh
   
   # Install PlatformIO
   pip install platformio
   
   # Download dependencies for offline use
   pio pkg install --global
   ```

3. **Configure for your board**
   ```bash
   # Copy and edit the configuration
   cp src/config.h src/config_local.h
   # Edit src/config_local.h with your settings
   ```

4. **Build and upload**
   ```bash
   # Build for AI-Thinker ESP32-CAM
   pio run -e esp32cam
   
   # Upload to board
   pio run -e esp32cam -t upload
   
   # Monitor serial output
   pio device monitor
   ```

## 📁 Project Structure

```
ESP32WildlifeCAM/
├── src/                    # Source code
│   ├── main.cpp           # Main entry point
│   ├── config.h           # Configuration
│   ├── core/              # Core system components
│   ├── hardware/          # Hardware abstraction
│   ├── camera/            # Camera handling
│   ├── network/           # Networking (LoRa/WiFi)
│   ├── ai/                # AI/ML components
│   ├── storage/           # SD card management
│   ├── power/             # Power management
│   └── utils/             # Utilities
├── lib/                   # External libraries
├── test/                  # Unit tests
├── data/                  # Web UI files
├── docs/                  # Documentation
└── platformio.ini         # PlatformIO configuration
```

## 🔧 Current Development Status

### ✅ Completed
- [x] Project structure setup
- [x] Board detection system
- [x] Basic logging framework
- [x] PlatformIO configuration

### 🚧 In Progress
- [ ] GPIO conflict resolution
- [ ] Camera driver implementation
- [ ] Motion detection system
- [ ] SD card storage

### 📋 Next Steps
1. Complete Phase 1 foundation components
2. Implement basic image capture
3. Add motion detection
4. Test on hardware

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](docs/CONTRIBUTING.md) for details.

### Development Workflow
1. Fork the repository
2. Create a feature branch
3. Implement with tests
4. Submit a pull request

## 📚 Documentation

- [Technical Specification](docs/TECHNICAL_SPECIFICATION.md)
- [Hardware Guide](docs/hardware-guide/README.md)
- [API Reference](docs/api-reference/README.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)

## 🐛 Known Issues

- GPIO conflicts on AI-Thinker boards between LoRa and flash LED
- Limited available pins require careful peripheral selection
- PSRAM cache workaround needed for camera stability

## 📝 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- ESP32 Arduino Core team
- TensorFlow Lite for Microcontrollers team
- LoRa library by Sandeep Mistry
- The open-source community

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/thewriterben/ESP32WildlifeCAM/issues)
- **Discussions**: [GitHub Discussions](https://github.com/thewriterben/ESP32WildlifeCAM/discussions)
- **Email**: thewriterben@[domain]

---

*Happy Wildlife Monitoring! 🦊🦝🦌*