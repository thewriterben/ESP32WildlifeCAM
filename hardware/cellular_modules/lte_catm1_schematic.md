# LTE Cat-M1 Module Integration

## Overview
This document describes the integration of LTE Cat-M1 cellular modules for low-power IoT connectivity in the ESP32 Wildlife Camera system.

## Recommended Modules

### SIM7000 Series
- **SIM7000G**: Global LTE Cat-M1/NB-IoT/EDGE/GPRS
- **SIM7000A**: North America LTE Cat-M1/NB-IoT/2G
- **SIM7000E**: Europe LTE Cat-M1/NB-IoT/2G

### Key Specifications
- **Power Consumption**: 
  - LTE Cat-M1: ~3mA standby, ~220mA active
  - NB-IoT: ~3mA standby, ~130mA active
  - Sleep mode: <5µA
- **Data Rates**:
  - LTE Cat-M1: Up to 1Mbps DL/UL
  - NB-IoT: Up to 250kbps DL, 200kbps UL
- **Operating Voltage**: 3.4V - 4.2V
- **Temperature Range**: -40°C to +85°C

## Hardware Connections

### ESP32-S3 to SIM7000 Pinout
```
ESP32-S3    SIM7000     Function
--------    -------     --------
GPIO16      UART_RXD    Serial RX
GPIO17      UART_TXD    Serial TX
GPIO18      PWRKEY      Power control
GPIO19      RST         Reset
GPIO21      STATUS      Status indicator
3.8V        VCC         Power supply
GND         GND         Ground
```

### Power Supply Circuit
```
Battery+ ─── LDO 3.8V ─── SIM7000 VCC
         │              │
         └── ESP32 VIN   └── 1000µF capacitor ─── GND
```

**Important**: SIM7000 requires stable 3.8V supply with large capacitor for transmission peaks.

## Schematic Design

### Minimal Integration Circuit
```
                     ESP32-S3
                   ┌─────────────┐
                   │         D16 ├─── RX ───┐
                   │         D17 ├─── TX ───┤
                   │         D18 ├─── PWR ──┤    SIM7000
                   │         D19 ├─── RST ──┤  ┌──────────┐
                   │         D21 ├─── STS ──┤  │          │
                   │             │          ├──┤ UART     │
                   │        3.3V ├──────────┤  │          │
                   │         GND ├──────────┤  │      ANT ├─── Cellular Antenna
                   └─────────────┘          └──┤          │
                                               │      SIM ├─── SIM Card Holder
                                               └──────────┘
```

### SIM Card Holder
- Use industrial-grade SIM card holder with spring contacts
- Include ESD protection diodes
- Add pull-up resistors on SIM data lines

### Power Management
- **LDO Regulator**: TPS73701 or similar for 3.8V generation
- **Bypass Capacitors**: 100nF ceramic + 1000µF electrolytic
- **Power Switch**: P-channel MOSFET for complete power isolation

## Antenna Design

### Cellular Antenna Requirements
- **Frequency Bands**:
  - LTE Cat-M1: 700/800/850/900/1800/1900/2100 MHz
  - NB-IoT: Same as Cat-M1
  - 2G/3G: 850/900/1800/1900 MHz

### Antenna Options
1. **Internal PCB Antenna**
   - Pros: Compact, integrated
   - Cons: Lower gain, requires ground plane tuning
   - Recommended for prototype testing

2. **External Whip/Rod Antenna**
   - Pros: Better performance, omnidirectional
   - Cons: Larger size, more mounting complexity
   - Recommended for production

3. **Patch Antenna**
   - Pros: Good gain, weather resistant
   - Cons: Directional, larger size
   - Suitable for fixed installations

### Antenna Placement
- Minimum 20cm from ESP32 and other radio modules
- Clear line of sight (no metal obstructions)
- Consider weatherproofing for outdoor installations

## PCB Layout Guidelines

### Critical Design Rules
1. **Keep cellular module away from switching power supplies**
2. **Use dedicated ground plane for RF sections**
3. **Minimize trace lengths for antenna connection**
4. **Add RF shielding between WiFi and cellular**

### Layer Stack-up (4-layer PCB)
```
Layer 1: Component placement and routing
Layer 2: Ground plane (continuous)
Layer 3: Power planes (3.3V, 3.8V)
Layer 4: Signal routing and ground fills
```

### Trace Impedance
- **Antenna traces**: 50Ω controlled impedance
- **UART signals**: Standard digital traces
- **Power traces**: Wide for current capacity

## Software Integration

### AT Command Initialization
```cpp
// Power on sequence
digitalWrite(PWRKEY_PIN, LOW);
delay(1000);
digitalWrite(PWRKEY_PIN, HIGH);
delay(3000);

// Test communication
Serial.println("AT");
// Expected response: OK

// Configure for Cat-M1
Serial.println("AT+CNMP=38");  // LTE only
Serial.println("AT+CMNB=1");   // Cat-M1 preferred
```

### Network Registration
```cpp
// Check registration status
Serial.println("AT+CREG?");
// Response: +CREG: 0,1 (registered home network)
//          +CREG: 0,5 (registered roaming)

// Signal quality
Serial.println("AT+CSQ");
// Response: +CSQ: <rssi>,<ber>
// RSSI: 0-31 (higher is better)
```

## Testing and Validation

### Signal Strength Testing
- Use AT+CSQ command for RSSI measurement
- Test at various locations around deployment site
- Document signal levels for optimal antenna placement

### Power Consumption Testing
- Measure current in different modes:
  - Idle: ~3mA
  - Connecting: ~220mA peaks
  - Transmitting: ~400mA peaks
  - Sleep: <5µA

### Data Transmission Testing
- Test with various data sizes
- Measure transmission times and success rates
- Validate data compression effectiveness

## Troubleshooting

### Common Issues

1. **Module not responding to AT commands**
   - Check power supply voltage (must be 3.8V ±5%)
   - Verify UART connections and baud rate
   - Ensure proper power-on sequence

2. **Network registration failures**
   - Check SIM card insertion and carrier activation
   - Verify antenna connection and placement
   - Test different locations for signal coverage

3. **High power consumption**
   - Ensure module enters sleep mode correctly
   - Check for RF interference causing retransmissions
   - Verify power management circuit efficiency

### Debug AT Commands
```
AT+CGMI     # Manufacturer information
AT+CGMM     # Model identification  
AT+CGMR     # Revision identification
AT+COPS?    # Operator selection
AT+CREG=2   # Enable network registration URC
```

## Carrier Compatibility

### Recommended IoT Plans
- **AT&T**: IoT Device Plan
- **Verizon**: ThingSpace IoT
- **T-Mobile**: IoT Device Connection
- **Global**: 1NCE, KORE, Hologram

### APN Configuration
```cpp
// Example APNs
AT+CGDCONT=1,"IP","iot.provider.com"  // Generic IoT APN
AT+CGDCONT=1,"IP","m2m"               // AT&T
AT+CGDCONT=1,"IP","vzwinternet"       // Verizon
```

## Cost Considerations

### Module Costs
- SIM7000G: ~$25-35 in low quantities
- Development board: ~$40-60
- Production integration: ~$15-25

### Data Plans
- Typical IoT plans: $2-10/month for 1-10MB
- Pay-per-use: $0.10-0.50 per MB
- Annual plans often provide better value

### Power Budget Impact
- Daily cellular usage: ~50-200mAh
- Sleep mode: ~0.1mAh/day
- Total impact: 10-20% of camera battery budget