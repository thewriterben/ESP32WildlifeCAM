# ESP32WildlifeCAM
ESP32 CAM Solar Powered Wireless Wildlife Monitoring System with Multi-Network Connectivity

## Overview
The ESP32 Wildlife Camera is a comprehensive remote monitoring solution that combines solar power with advanced multi-network communication capabilities. The system automatically selects the optimal communication method from LoRa mesh, cellular (LTE Cat-M1/NB-IoT/2G/3G/4G), and satellite options based on availability, cost, and power constraints.

## Key Features

### 🌐 Multi-Network Communication
- **LoRa Mesh Networking**: Long-range, low-power local connectivity (5-15km range)
- **Cellular Communication**: 
  - LTE Cat-M1/NB-IoT for IoT-optimized connectivity
  - 2G/3G/4G fallback for universal coverage
  - SMS backup for critical alerts
- **Satellite Communication**:
  - Iridium global coverage for remote locations
  - Swarm low-cost satellite IoT
  - Store-and-forward messaging
- **Automatic Network Selection**: Intelligent switching based on signal quality, cost, and power

### 🔋 Power Management
- **Solar-powered operation** with battery backup
- **Adaptive power modes** based on battery level and network availability
- **Ultra-low power sleep modes** with scheduled wake-up
- **Power-aware network selection** to maximize battery life

### 📡 Advanced Features
- **GPS/GNSS Integration**: Multi-constellation positioning and timing
- **Data Compression**: Optimized for bandwidth-limited connections
- **Real-time Image Processing**: Motion detection and thumbnail generation
- **Emergency Communications**: Priority messaging with multi-network redundancy
- **Remote Configuration**: OTA updates via cellular connection

## System Architecture

```
┌─────────────────┐    ┌──────────────────────────────────────┐
│   Solar Panel   │    │              ESP32-S3                │
│   + Battery     ├────┤  Wildlife Camera Controller          │
└─────────────────┘    │                                      │
                       │  ┌─────────────────────────────────┐ │
┌─────────────────┐    │  │        Network Selector        │ │
│   Camera        ├────┤  │                                 │ │
│   Module        │    │  │  ┌─────┐ ┌─────────┐ ┌────────┐ │ │
└─────────────────┘    │  │  │LoRa │ │Cellular │ │Satellite│ │ │
                       │  │  │Mesh │ │LTE/NB   │ │Iridium │ │ │
┌─────────────────┐    │  │  └─────┘ └─────────┘ └────────┘ │ │
│   GPS/GNSS      ├────┤  └─────────────────────────────────┘ │
│   Receiver      │    │                                      │
└─────────────────┘    └──────────────────────────────────────┘
                                        │
                       ┌────────────────┼────────────────┐
                       │                │                │
                   ┌───▼───┐     ┌─────▼─────┐     ┌────▼────┐
                   │ LoRa  │     │ Cellular  │     │Satellite│
                   │Gateway│     │  Tower    │     │   Sat   │
                   └───────┘     └───────────┘     └─────────┘
```

## Project Structure

```
ESP32WildlifeCAM/
├── firmware/src/
│   ├── cellular_manager.cpp      # LTE Cat-M1/NB-IoT communication
│   ├── cellular_manager.h
│   ├── satellite_comm.cpp        # Iridium/Swarm satellite communication
│   ├── satellite_comm.h
│   ├── network_selector.cpp      # Automatic network selection
│   ├── network_selector.h
│   ├── data_compression.cpp      # Image/data compression utilities
│   ├── data_compression.h
│   ├── gps_handler.cpp          # GPS/GNSS location services
│   └── gps_handler.h
├── hardware/
│   ├── cellular_modules/
│   │   ├── lte_catm1_schematic.md
│   │   ├── nb_iot_integration.md
│   │   └── antenna_design.md
│   └── satellite_modules/
│       ├── iridium_module.md
│       ├── swarm_integration.md
│       └── gnss_receiver.md
├── docs/
│   ├── connectivity_guide.md     # Communication technology comparison
│   ├── network_selection.md      # Network selection algorithms
│   └── satellite_setup.md        # Satellite communication setup
├── examples/
│   ├── cellular_test/            # Cellular communication testing
│   ├── satellite_comm/           # Satellite communication examples
│   └── multi_network/            # Multi-network demonstration
└── README.md
```

## Quick Start

### Hardware Requirements

#### Core Components
- **ESP32-S3** development board with camera support
- **Camera module** (OV2640 or similar)
- **Solar panel** (5-10W) with charge controller
- **LiPo battery** (5000-10000mAh)
- **Weatherproof enclosure** (IP65/67 rated)

#### Communication Modules (Choose based on deployment needs)

**For Cellular Communication:**
- SIM7600/SIM7000 series LTE Cat-M1/NB-IoT module
- Cellular antenna (multi-band)
- SIM card with IoT data plan

**For Satellite Communication:**
- Iridium 9603N or Swarm M138 module
- Satellite antenna (appropriate frequency)
- Satellite service subscription

**For LoRa Mesh:**
- LoRa radio module (SX1276/SX1278)
- LoRa antenna (915MHz or 868MHz)

#### Additional Components
- **GPS/GNSS module** (NEO-M8N or ZED-F9P)
- **Environmental sensors** (temperature, humidity)
- **PIR motion sensor**
- **MicroSD card** for local storage

### Software Setup

1. **Install Arduino IDE** with ESP32 board support
2. **Install required libraries**:
   ```
   - ArduinoJson
   - Time
   - SoftwareSerial
   - WiFi (built-in)
   ```

3. **Clone this repository**:
   ```bash
   git clone https://github.com/thewriterben/ESP32WildlifeCAM.git
   cd ESP32WildlifeCAM
   ```

4. **Configure your deployment**:
   ```cpp
   // In your main sketch
   #define ENABLE_CELLULAR true
   #define ENABLE_SATELLITE false  // Set to true if using satellite
   #define ENABLE_LORA true
   
   // Network priority
   #define PRIMARY_NETWORK LORA_MESH
   #define SECONDARY_NETWORK CELLULAR
   #define BACKUP_NETWORK SATELLITE
   ```

### Basic Usage Example

```cpp
#include "network_selector.h"
#include "cellular_manager.h"
#include "data_compression.h"

NetworkSelector networkSelector;
CellularManager cellularManager;
DataCompression compressor;

void setup() {
    Serial.begin(115200);
    
    // Initialize communication systems
    cellularManager.initialize();
    cellularManager.connectToNetwork();
    
    // Initialize network selector
    networkSelector.initialize(&cellularManager, nullptr);
}

void loop() {
    // Capture image (your camera code here)
    uint8_t* imageData = captureImage();
    size_t imageSize = getImageSize();
    
    // Compress for transmission
    uint8_t compressedData[32000];
    size_t compressedSize = compressor.compressImage(
        imageData, imageSize, 
        compressedData, sizeof(compressedData)
    );
    
    // Send via optimal network
    bool success = networkSelector.sendData(
        compressedData, compressedSize, PRIORITY_NORMAL
    );
    
    if (success) {
        Serial.println("Image transmitted successfully");
    }
    
    delay(300000); // Wait 5 minutes
}
```

## Configuration Options

### Network Configuration
```cpp
// Enable/disable communication technologies
#define ENABLE_CELLULAR true
#define ENABLE_SATELLITE true
#define ENABLE_LORA_MESH true

// Priority settings (1 = highest priority)
#define PRIMARY_NETWORK LORA_MESH
#define SECONDARY_NETWORK CELLULAR
#define BACKUP_NETWORK SATELLITE

// Data limits
#define CELLULAR_DAILY_LIMIT_MB 100
#define SATELLITE_MONTHLY_MESSAGES 50

// Power management
#define ENABLE_DEEP_SLEEP true
#define SLEEP_DURATION_MINUTES 30
```

### Cellular Configuration
```cpp
// APN settings (configure for your carrier)
const char* APN = "iot.provider.com";
const char* USERNAME = "";
const char* PASSWORD = "";

// Preferred network modes
// AT+CNMP=38 (LTE only)
// AT+CNMP=13 (GSM/WCDMA/LTE auto)
```

### Satellite Configuration
```cpp
// Satellite service provider
#define SATELLITE_PROVIDER MODULE_SWARM  // or MODULE_IRIDIUM

// Message limits and scheduling
#define MAX_DAILY_SATELLITE_MESSAGES 10
#define SATELLITE_RETRY_ATTEMPTS 3
```

## Deployment Guide

### 1. Site Selection
- **Signal Coverage**: Test cellular signal strength at deployment location
- **Solar Exposure**: Ensure 6+ hours direct sunlight for solar panel
- **Line of Sight**: Clear sky view for GPS and satellite communication
- **Wildlife Activity**: Position for optimal animal detection
- **Accessibility**: Consider maintenance access requirements

### 2. Power System Design
```
Solar Panel (10W) → Charge Controller → Battery (10Ah)
                                    ↓
                              ESP32 + Modules (1-5W)
```

**Power Budget Example:**
- ESP32-S3: 100mA average
- Camera: 200mA during capture (30s/hour)
- Cellular: 220mA during transmission (5min/hour)
- LoRa: 20mA during transmission (1min/hour)
- Sleep mode: 10mA average
- **Total: ~150mA average (3.6Ah/day)**

### 3. Antenna Placement
- **Cellular**: Vertical polarization, 20cm+ from other antennas
- **Satellite**: Clear sky view, appropriate polarization
- **LoRa**: Elevated position for maximum range
- **GPS**: Horizontal placement with sky view

### 4. Network Setup

#### LoRa Mesh Network
1. Configure frequency band for your region (915MHz US, 868MHz EU)
2. Set up gateway node with internet connectivity
3. Configure mesh routing parameters
4. Test range and signal quality

#### Cellular Setup
1. Obtain IoT SIM card from carrier
2. Configure APN settings
3. Test signal strength and data connectivity
4. Set up data usage monitoring

#### Satellite Setup
1. Register with satellite service provider
2. Activate device on network
3. Configure message routing
4. Test message delivery

## Examples and Testing

The project includes comprehensive examples to demonstrate functionality:

### Cellular Test Example
```bash
examples/cellular_test/cellular_test.ino
```
- Tests cellular module initialization and connectivity
- Measures signal strength and network performance
- Demonstrates SMS backup functionality
- Provides detailed performance metrics

### Multi-Network Demo
```bash
examples/multi_network/multi_network_demo.ino
```
- Shows automatic network selection in action
- Demonstrates different data types (telemetry, images, status)
- Includes emergency communication scenarios
- Provides cost and performance analysis

Run these examples to validate your hardware setup and understand system capabilities.

## Cost Analysis

### Initial Hardware Cost
```
Component               Budget    Professional
ESP32-S3 + Camera      $40       $80
Cellular Module        $25       $150
Satellite Module       -         $300
LoRa Module           $15       $50
GPS Module            $15       $100
Solar + Battery       $100      $300
Enclosure             $50       $200
Antennas              $30       $150
Total                 $275      $1,330
```

### Operational Costs (Annual)
```
Service               Budget    Professional
Cellular Data Plan    $60       $180
Satellite Messages    -         $300
Maintenance          $50       $500
Total Annual         $110      $980
```

## Troubleshooting

### Common Issues

#### Cellular Connection Problems
1. **Check SIM card activation** and data plan status
2. **Verify APN settings** for your carrier
3. **Test signal strength** at deployment location
4. **Try different carriers** for better coverage

#### GPS Not Acquiring Fix
1. **Ensure clear sky view** (no obstructions above 15°)
2. **Wait for satellite almanac download** (can take 12+ minutes)
3. **Check antenna connections**
4. **Try different GPS module** if persistent issues

#### Battery Drain Issues
1. **Verify solar panel operation** and positioning
2. **Check for current leaks** in sleep mode
3. **Optimize transmission schedules**
4. **Consider larger battery** for extended cloudy periods

### Support Resources
- **Documentation**: Comprehensive guides in `/docs` folder
- **Examples**: Working code examples in `/examples` folder
- **Hardware Guides**: Detailed integration guides in `/hardware` folder
- **Community Support**: GitHub Issues and Discussions

## Contributing

We welcome contributions to improve the ESP32 Wildlife Camera project:

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Commit your changes**: `git commit -m 'Add amazing feature'`
4. **Push to the branch**: `git push origin feature/amazing-feature`
5. **Open a Pull Request**

### Areas for Contribution
- Additional communication protocols (5G, WiFi HaLow)
- Enhanced image processing algorithms
- Power optimization improvements
- New sensor integrations
- Documentation improvements

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- ESP32 community for excellent development resources
- LoRa Alliance for long-range communication standards
- Cellular IoT ecosystem providers
- Satellite communication technology pioneers
- Wildlife conservation community for inspiration

---

**Disclaimer**: This project is for educational and research purposes. Ensure compliance with local regulations regarding radio communications, wildlife monitoring, and environmental protection in your deployment area.
