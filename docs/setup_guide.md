# ESP32 Wildlife Camera Setup Guide

## Complete Installation and Configuration Instructions

### Prerequisites

#### Required Tools
- Soldering iron and solder
- Multimeter for testing
- 3D printer (or access to 3D printing service)
- Basic hand tools (screwdrivers, pliers)
- Computer with USB port
- PlatformIO IDE or Arduino IDE

#### Required Components
See the [Bill of Materials](../hardware/bom.md) for complete component list.

### Step 1: Hardware Assembly

#### 1.1 PCB Assembly

**Component Placement Order:**
```
1. Surface mount components (if using custom PCB)
   - TP4056 charging module
   - Voltage divider resistors
   - Capacitors and diodes

2. Through-hole components
   - ESP32-S3-DevKitC-1 board
   - Pin headers and connectors
   - Terminal blocks

3. External modules
   - Camera module connection
   - LoRa module installation
   - Sensor connections
```

**Critical Connections:**
```
Power System:
Solar Panel → 1N5819 Diode → TP4056 VIN
TP4056 BAT+ → 18650 Battery +
TP4056 OUT+ → ESP32 VIN (or 3.3V rail)

Voltage Monitoring:
Battery + → 10kΩ → GPIO35 → 10kΩ → GND
Solar + → 10kΩ → GPIO34 → 10kΩ → GND

Camera (OV2640):
[See pinout in config.h for complete connections]

LoRa Module (SX1276):
SCK → GPIO18, MISO → GPIO19, MOSI → GPIO23
CS → GPIO5, RST → GPIO14, DIO0 → GPIO26
```

#### 1.2 Power System Verification

**Pre-Power Checks:**
```bash
# Use multimeter to verify:
1. No short circuits between power rails
2. Correct polarity on battery connections
3. Solar panel voltage measurement (in sunlight)
4. TP4056 module orientation
```

**Initial Power-Up:**
```bash
1. Connect battery (should see TP4056 power LED)
2. Measure 3.3V on ESP32 power rail
3. Verify solar charging (charge LED when in sunlight)
4. Check voltage divider outputs with multimeter
```

### Step 2: Firmware Installation

#### 2.1 Development Environment Setup

**Install PlatformIO:**
```bash
# Using pip
pip install platformio

# Or using PlatformIO IDE extension in VS Code
# 1. Install VS Code
# 2. Install PlatformIO IDE extension
# 3. Restart VS Code
```

**Alternative: Arduino IDE Setup:**
```bash
# Install ESP32 board support:
# 1. Open Arduino IDE
# 2. File → Preferences → Additional Board Manager URLs
# 3. Add: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
# 4. Tools → Board → Boards Manager → Search "ESP32" → Install
```

#### 2.2 Firmware Compilation and Upload

**Using PlatformIO:**
```bash
# Clone the repository
git clone https://github.com/thewriterben/ESP32WildlifeCAM.git
cd ESP32WildlifeCAM/firmware

# Build the firmware
pio run

# Upload to device (ESP32 connected via USB)
pio run --target upload

# Monitor serial output
pio device monitor
```

**Configuration Before Upload:**
```cpp
// Edit firmware/src/config.h
#define NODE_ID 1                    // Unique camera ID
#define LORA_FREQUENCY 915E6         // Adjust for your region
#define CAMERA_FRAME_SIZE FRAMESIZE_UXGA  // Adjust for memory
#define TRIGGER_ACTIVE_HOURS_START 6  // Active hours
#define TRIGGER_ACTIVE_HOURS_END 20
```

#### 2.3 Initial Testing

**Serial Monitor Output:**
```
========================================
ESP32 Wildlife Camera Starting...
Firmware Version: 1.0.0
Build Date: [DATE] [TIME]
========================================

Initializing system components...
Camera initialized successfully
LoRa mesh network initialized - Node ID: 1
Solar power management initialized
Motion filter system initialized
System initialization complete!
```

### Step 3: Enclosure Assembly

#### 3.1 3D Printing Preparation

**Print Settings (PETG recommended):**
```
Layer Height: 0.2mm
Infill: 25%
Print Speed: 50mm/s
Nozzle Temperature: 230°C
Bed Temperature: 70°C
Support: Minimal (designed to minimize supports)
```

**Post-Processing:**
```bash
1. Remove support material carefully
2. Test fit all components before assembly
3. Light sanding of mating surfaces if needed
4. Clean with isopropyl alcohol
```

#### 3.2 Weatherproofing

**Gasket Installation:**
```
1. Use 2mm O-ring (Buna-N or EPDM)
2. Clean gasket groove thoroughly
3. Apply thin layer of silicone grease
4. Install gasket without twisting
5. Test compression before final assembly
```

**Cable Sealing:**
```
1. Install PG7 cable glands in designated holes
2. Route cables through strain relief
3. Apply thread locker to gland threads
4. Test seal with gentle water spray
```

### Step 4: Field Installation

#### 4.1 Site Selection

**Optimal Placement Criteria:**
```
Camera Positioning:
├── 2-3 meters above ground level
├── Clear view of wildlife corridors
├── Avoid direct sunlight on lens
├── Natural camouflage opportunities
└── Accessible for maintenance

Solar Panel Orientation:
├── South-facing (Northern Hemisphere)
├── 15° tilt angle optimal
├── No shading from 9AM-3PM
├── Seasonal adjustment capability
└── Protected from storm damage

Network Considerations:
├── Line of sight to other cameras (LoRa mesh)
├── Maximum 2km distance between nodes
├── Elevated position for better range
├── Clear of metal structures (interference)
└── GPS coordinates recorded
```

#### 4.2 Physical Mounting

**Tree Installation:**
```bash
1. Select healthy tree 150-300mm diameter
2. Position mount to avoid branch growth
3. Use tree-friendly mounting strap
4. Install at slight downward angle for rain runoff
5. Secure with stainless steel through-bolt
6. Apply tree wound sealant if needed
```

**Post Installation:**
```bash
1. Use galvanized steel or aluminum post
2. Dig post hole 600mm deep minimum
3. Use concrete footing for stability
4. Allow 24-48 hours for concrete to cure
5. Attach camera mount with anti-theft bolts
```

#### 4.3 Initial Configuration

**Power System Check:**
```bash
# Monitor serial output during setup
Battery: 3.85V (92%)
Solar: 4.2V (Charging)
Power State: NORMAL
Charging: YES
```

**Camera Calibration:**
```bash
# Test image capture
Motion detected!
Valid motion after filtering
Image captured: 1600x1200, 245760 bytes
Image saved: /images/20231201_143022_0001.jpg
```

**Network Setup:**
```bash
# LoRa mesh initialization
LoRa mesh network initialized - Node ID: 1
Heartbeat from node 2
Image notification from node 3: img_001.jpg (156KB)
```

### Step 5: System Configuration

#### 5.1 Motion Detection Tuning

**PIR Sensor Adjustment:**
```
Sensitivity Potentiometer:
├── Start at 50% position
├── Test with known wildlife activity
├── Adjust for optimal detection range
├── Account for vegetation movement
└── Document final setting

Time Delay Setting:
├── Set to minimum (2 seconds)
├── Software handles debouncing
├── Reduces false triggers
└── Preserves battery life
```

**Weather Filtering Calibration:**
```cpp
// Adjust in config.h based on local conditions
#define WIND_THRESHOLD 15        // km/h - local wind patterns
#define RAIN_THRESHOLD 0.5       // mm/h - regional rainfall
#define TEMP_COMP_ENABLED true   // Enable for stability
```

#### 5.2 Camera Settings Optimization

**Image Quality vs. Battery Life:**
```cpp
// High quality (shorter battery life)
#define CAMERA_FRAME_SIZE FRAMESIZE_UXGA  // 1600x1200
#define CAMERA_JPEG_QUALITY 10            // High quality

// Balanced settings (recommended)
#define CAMERA_FRAME_SIZE FRAMESIZE_SVGA  // 800x600
#define CAMERA_JPEG_QUALITY 15            // Good quality

// Extended battery life
#define CAMERA_FRAME_SIZE FRAMESIZE_VGA   // 640x480
#define CAMERA_JPEG_QUALITY 20            // Adequate quality
```

**Lighting Adaptation:**
```cpp
// Configure for local conditions
sensor->set_ae_level(sensor, 0);     // Exposure compensation
sensor->set_agc_gain(sensor, 5);     // Auto gain control
sensor->set_wb_mode(sensor, 0);      // White balance (0=auto)
```

#### 5.3 Power Management Optimization

**Sleep Schedule Configuration:**
```cpp
// Active monitoring hours
#define TRIGGER_ACTIVE_HOURS_START 6   // 6:00 AM
#define TRIGGER_ACTIVE_HOURS_END 20    // 8:00 PM

// Deep sleep duration (seconds)
#define DEEP_SLEEP_DURATION 300        // 5 minutes

// Daily trigger limits
#define MAX_DAILY_TRIGGERS 50          // Prevent storage overflow
```

**Battery Conservation Settings:**
```cpp
// Low power thresholds
#define BATTERY_LOW_THRESHOLD 3.0      // Warning level
#define BATTERY_CRITICAL_THRESHOLD 2.8 // Emergency shutdown

// Power saving features
#define ENABLE_DEEP_SLEEP true
#define REDUCE_CPU_FREQUENCY true
#define DISABLE_WIFI_SLEEP false       // Keep disabled for power saving
```

### Step 6: Network Configuration

#### 6.1 LoRa Mesh Setup

**Multi-Node Network:**
```cpp
// Assign unique node IDs
Node 1: #define NODE_ID 1  // Base station/gateway
Node 2: #define NODE_ID 2  // Remote camera 1
Node 3: #define NODE_ID 3  // Remote camera 2
...

// Network parameters (must match across all nodes)
#define LORA_FREQUENCY 915E6      // Regional frequency
#define LORA_SYNC_WORD 0x12       // Network identifier
#define LORA_TX_POWER 20          // Maximum legal power
```

**Range Optimization:**
```cpp
// For maximum range
#define LORA_SPREADING_FACTOR 12   // Slower but longer range
#define LORA_SIGNAL_BANDWIDTH 62.5E3  // Narrower bandwidth

// For faster data transfer
#define LORA_SPREADING_FACTOR 7    // Faster but shorter range
#define LORA_SIGNAL_BANDWIDTH 125E3   // Standard bandwidth
```

#### 6.2 Data Management

**Storage Configuration:**
```cpp
// SD card organization
/images/YYYYMMDD_HHMMSS_####.jpg  // Timestamped images
/logs/YYYYMMDD.log                // Daily system logs
/config/settings.json             // Configuration backup

// Automatic cleanup
#define MAX_FILES_PER_DAY 100     // Prevent SD card overflow
#define AUTO_DELETE_DAYS 30       // Remove old files automatically
```

### Step 7: Testing and Validation

#### 7.1 System Integration Test

**Functional Verification:**
```bash
# Complete system test checklist
☐ Solar charging operational
☐ Battery monitoring accurate
☐ Motion detection responsive
☐ Camera image quality acceptable
☐ LoRa network communication
☐ SD card storage working
☐ Weather filtering effective
☐ Deep sleep recovery
☐ Emergency low power mode
☐ Enclosure weatherproof seal
```

#### 7.2 Field Trial

**24-Hour Test Protocol:**
```
Day 1: Monitor continuously via serial connection
├── Log all motion detections
├── Verify solar charging curve
├── Test weather filtering
├── Check network communications
└── Validate power consumption

Day 2-7: Remote monitoring
├── Daily status checks via LoRa
├── Image review for quality
├── Battery performance tracking
├── False positive analysis
└── Network reliability assessment
```

### Step 8: Maintenance Schedule

#### 8.1 Regular Maintenance

**Monthly Tasks:**
```bash
☐ Check battery voltage and charging
☐ Clean solar panel surface
☐ Verify camera lens cleanliness
☐ Test motion detection sensitivity
☐ Review stored images
☐ Check weatherproof seals
☐ Verify network connectivity
```

**Quarterly Tasks:**
```bash
☐ Update firmware if available
☐ Deep clean enclosure interior
☐ Replace desiccant packets
☐ Calibrate voltage measurements
☐ Performance optimization review
☐ Network range testing
☐ Documentation updates
```

**Annual Tasks:**
```bash
☐ Battery capacity test/replacement
☐ Complete system inspection
☐ Firmware backup and update
☐ Enclosure gasket replacement
☐ Solar panel efficiency test
☐ Long-term data analysis
☐ Hardware upgrade planning
```

#### 8.2 Troubleshooting Quick Reference

**Common Issues:**
```
No Images Captured:
├── Check motion sensor operation
├── Verify camera initialization
├── Test SD card functionality
├── Review trigger settings
└── Check battery voltage

Poor Image Quality:
├── Clean camera lens
├── Adjust focus if manual
├── Check lighting conditions
├── Review camera settings
└── Verify stable mounting

Network Communication Failure:
├── Check LoRa antenna connection
├── Verify frequency settings
├── Test with reduced range
├── Check for interference
└── Review network topology

Power Issues:
├── Test solar panel output
├── Check charging circuit
├── Measure battery capacity
├── Review power consumption
└── Verify connections
```

### Support Resources

- **GitHub Repository**: https://github.com/thewriterben/ESP32WildlifeCAM
- **Documentation**: [Project Wiki](https://github.com/thewriterben/ESP32WildlifeCAM/wiki)
- **Community**: [Discussions](https://github.com/thewriterben/ESP32WildlifeCAM/discussions)
- **Issues**: [Bug Reports](https://github.com/thewriterben/ESP32WildlifeCAM/issues)

---

**⚠️ Safety Notes:**
- Always disconnect power when working on circuits
- Use appropriate PPE when working outdoors
- Follow local regulations for radio transmissions
- Respect wildlife and habitat protection laws
- Consider environmental impact of installations