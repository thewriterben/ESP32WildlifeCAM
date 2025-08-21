# Circuit Schematic Documentation

## ESP32 Wildlife Camera - Solar Power Integration

### Main Circuit Description

This directory contains the circuit schematics for the ESP32 Wildlife Camera system. The design integrates solar power management, LoRa communication, and environmental sensors in a compact, weather-resistant package.

### Circuit Blocks

#### 1. Power Management Circuit
```
Solar Panel (5V 2W) → Schottky Diode → TP4056 Charging Module → 18650 Battery → ESP32-S3
                                     ↓
                              Status LEDs & Protection Circuit
```

**Key Features:**
- Reverse current protection via Schottky diode
- Automatic battery charging with overvoltage/undervoltage protection
- Voltage monitoring for power management
- LED indicators for charging status

#### 2. ESP32-S3 Core Circuit
```
ESP32-S3-DevKitC-1 Board
├── GPIO 0-48 (various functions)
├── Camera interface (parallel)
├── SPI for LoRa module
├── I2C for environmental sensors
├── ADC for voltage monitoring
└── Deep sleep wake-up circuits
```

#### 3. Camera Interface (OV2640)
```
ESP32-S3 Camera Pins:
├── PWDN_GPIO_NUM: 32  (Power Down)
├── RESET_GPIO_NUM: -1 (Reset - not used)
├── XCLK_GPIO_NUM: 0   (Clock)
├── SIOD_GPIO_NUM: 26  (I2C Data)
├── SIOC_GPIO_NUM: 27  (I2C Clock)
├── Y9_GPIO_NUM: 35    (Data 9)
├── Y8_GPIO_NUM: 34    (Data 8)
├── Y7_GPIO_NUM: 39    (Data 7)
├── Y6_GPIO_NUM: 36    (Data 6)
├── Y5_GPIO_NUM: 21    (Data 5)
├── Y4_GPIO_NUM: 19    (Data 4)
├── Y3_GPIO_NUM: 18    (Data 3)
├── Y2_GPIO_NUM: 5     (Data 2)
├── VSYNC_GPIO_NUM: 25 (V-Sync)
├── HREF_GPIO_NUM: 23  (H-Ref)
└── PCLK_GPIO_NUM: 22  (Pixel Clock)
```

#### 4. LoRa Module Interface (SX1276)
```
ESP32-S3 SPI Pins:
├── LORA_SCK: 18   (SPI Clock)
├── LORA_MISO: 19  (SPI MISO)
├── LORA_MOSI: 23  (SPI MOSI)
├── LORA_CS: 5     (Chip Select)
├── LORA_RST: 14   (Reset)
└── LORA_DIO0: 26  (Interrupt)
```

#### 5. Sensor Interfaces
```
PIR Motion Sensor (HC-SR501):
├── VCC → 3.3V
├── GND → Ground
└── OUT → GPIO 13 (Interrupt capable)

BME280 Environmental Sensor (I2C):
├── VCC → 3.3V
├── GND → Ground
├── SDA → GPIO 21
└── SCL → GPIO 22

Voltage Monitoring:
├── Solar Voltage → GPIO 34 (ADC1_CH6) via voltage divider
└── Battery Voltage → GPIO 35 (ADC1_CH7) via voltage divider
```

### Voltage Divider Circuits

#### Battery Voltage Monitoring
```
Battery+ ─── 10kΩ ─── GPIO35 (ADC) ─── 10kΩ ─── GND
                      │
                   (Vmeas = Vbat/2)
```

#### Solar Panel Voltage Monitoring
```
Solar+ ─── 10kΩ ─── GPIO34 (ADC) ─── 10kΩ ─── GND
                     │
                  (Vmeas = Vsolar/2)
```

### Power Supply Distribution

```
18650 Battery (3.7V nominal)
├── ESP32-S3 Main Power (3.3V via onboard regulator)
├── Camera Module (3.3V)
├── LoRa Module (3.3V)
├── PIR Sensor (3.3V)
├── BME280 Sensor (3.3V)
└── Status LEDs (3.3V via current limiting resistors)
```

### Charging Circuit Detail

```
Solar Panel (+) ──── 1N5819 Diode ──── TP4056 VIN
                                        │
Solar Panel (-) ────────────────────── TP4056 GND
                                        │
                    TP4056 BAT+ ──── 18650 (+)
                    TP4056 BAT- ──── 18650 (-)
                                        │
                    TP4056 OUT+ ──── ESP32 VIN
                    TP4056 OUT- ──── ESP32 GND
```

### PCB Layout Considerations

#### Layer Stack-up (2-layer PCB)
- **Top Layer**: Component placement and signal routing
- **Bottom Layer**: Ground plane and power distribution

#### Critical Routing Guidelines
1. **Camera Signals**: Keep parallel data lines equal length, minimize crosstalk
2. **SPI Signals**: Use controlled impedance, keep traces short
3. **Power Lines**: Use wide traces for current carrying capacity
4. **Antenna**: Keep LoRa antenna traces away from noisy digital signals
5. **Crystal**: Keep ESP32 crystal traces short and isolated

#### Component Placement
```
[Solar Input] ────┐
                  │
[TP4056]          │    [ESP32-S3]     [Camera]
                  │         │            │
[Battery]         │    [LoRa Module]     │
                  │         │            │
[Voltage Dividers]│    [BME280]     [PIR Sensor]
                  │         │            │
[Status LEDs] ────┴─────────┴────────────┘
```

### Signal Integrity Notes

#### Camera Interface
- Use 75Ω controlled impedance for high-speed camera signals
- Minimize via usage on camera data lines
- Place decoupling capacitors close to camera power pins

#### LoRa Module
- Maintain 50Ω impedance on antenna trace
- Use ground plane stitching around LoRa module
- Place antenna connector at board edge

#### Power Distribution
- Use multiple vias for power connections
- Place bulk and bypass capacitors strategically
- Minimize ground loops

### Environmental Protection

#### Moisture Protection
- Conformal coating on PCB (optional)
- Sealed enclosure with desiccant
- Breathable membrane for pressure equalization

#### Temperature Management
- Operating range: -20°C to +60°C
- Component derating for extreme temperatures
- Thermal relief for ground planes

### Test Points

Add test points for critical signals:
- Battery voltage
- Solar voltage  
- 3.3V rail
- LoRa SPI signals
- Camera clock signal
- I2C bus

### Design Files

The following files should be included in this directory:

1. **main_circuit.sch** - Complete schematic in KiCad format
2. **power_management.sch** - Detailed power circuit
3. **esp32_connections.sch** - ESP32 pin assignments
4. **sensor_interfaces.sch** - Sensor connection details
5. **manufacturing_notes.pdf** - PCB fabrication specifications

### Simulation and Verification

#### SPICE Models
- TP4056 charging behavior simulation
- Battery discharge curves
- Voltage divider accuracy analysis

#### Signal Integrity Analysis
- Camera signal timing analysis
- SPI signal quality verification
- Power supply noise analysis

### Compliance and Certification

#### EMC Considerations
- LoRa module FCC/CE certification
- PCB layout for EMI reduction
- Shielding requirements

#### Safety Standards
- Battery protection circuit compliance
- Solar panel connection safety
- Outdoor installation requirements

---

**Note**: Actual schematic files would be created using professional EDA software like KiCad, Altium Designer, or Eagle CAD. The text descriptions above provide the foundation for creating these detailed schematics.