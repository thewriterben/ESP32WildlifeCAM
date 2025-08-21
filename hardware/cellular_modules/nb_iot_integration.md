# NB-IoT Integration Guide

## Overview
Narrowband IoT (NB-IoT) provides ultra-low power cellular connectivity optimized for IoT applications with infrequent data transmission requirements.

## Technical Specifications

### NB-IoT Characteristics
- **Data Rate**: Up to 250kbps downlink, 20-250kbps uplink
- **Power Consumption**: 
  - Active: 50-100mA typical
  - PSM (Power Saving Mode): <10µA
  - eDRX (Extended Discontinuous Reception): 1-5mA
- **Coverage**: +20dB better penetration than LTE
- **Latency**: 1.6-10 seconds typical
- **Maximum Coupling Loss**: 164dB

### Use Cases for Wildlife Camera
- **Periodic Status Updates**: Hourly/daily health reports
- **Thumbnail Transmission**: Low-resolution preview images
- **Emergency Alerts**: Critical system notifications
- **Environmental Data**: Temperature, humidity, battery status

## Module Selection

### Recommended NB-IoT Modules

1. **Quectel BC95-G**
   - Global NB-IoT bands
   - Ultra-low power consumption
   - Compact form factor (19.9×23.6×2.2mm)

2. **SIMCom SIM7020**
   - Multi-band support
   - Extended temperature range
   - AT command interface

3. **u-blox SARA-N4**
   - High-performance NB-IoT
   - Integrated antenna detector
   - Advanced power management

## Hardware Integration

### ESP32 to NB-IoT Module Connections
```
ESP32      NB-IoT Module   Function
------     -------------   --------
GPIO16     UART_RXD        Serial communication
GPIO17     UART_TXD        Serial communication  
GPIO18     RESET_N         Module reset
GPIO19     PWRKEY          Power key
GPIO21     VDD_EXT         Power control
3.3V       VCC             Power supply
GND        GND             Ground
```

### Power Management Circuit
```
Battery ─── Buck Converter ─── 3.3V ─── NB-IoT Module
        │                   │
        └─── ESP32 ─────────┘
                │
                └─── Power Switch (GPIO21)
```

### Antenna Interface
- **Impedance**: 50Ω
- **Return Loss**: <-10dB across NB-IoT bands
- **Efficiency**: >50% minimum

## Power Optimization

### PSM (Power Saving Mode)
```cpp
// Enable PSM with 12-hour active timer, 24-hour periodic TAU
AT+CPSMS=1,"","","01001000","00100100"

// Check PSM status
AT+CPSMS?
```

### eDRX (Extended Discontinuous Reception)
```cpp
// Enable eDRX with 81.92s cycle
AT+CEDRXS=1,5,"1001"

// Check eDRX settings
AT+CEDRXS?
```

### Sleep Mode Management
```cpp
void enterDeepSleep() {
    // Configure NB-IoT for PSM
    sendATCommand("AT+CPSMS=1");
    
    // Put ESP32 in deep sleep
    esp_sleep_enable_timer_wakeup(24 * 3600 * 1000000ULL); // 24 hours
    esp_deep_sleep_start();
}
```

## Data Transmission Optimization

### Message Size Limits
- **UDP**: 1024 bytes maximum
- **CoAP**: 512 bytes recommended
- **MQTT**: 1024 bytes with overhead considerations

### Compression Strategies
```cpp
// Compress sensor data for NB-IoT transmission
struct CompressedSensorData {
    uint16_t timestamp;     // Relative timestamp (minutes)
    uint8_t battery;        // Battery level (0-100)
    int8_t temperature;     // Temperature (-128 to 127°C)
    uint8_t humidity;       // Humidity (0-100%)
    uint8_t status;         // System status flags
} __attribute__((packed));
```

### Store-and-Forward Implementation
```cpp
class NBIoTDataBuffer {
private:
    std::queue<CompressedSensorData> dataQueue;
    const size_t MAX_QUEUE_SIZE = 24; // 24 hours of data
    
public:
    void addReading(const SensorReading& reading) {
        if (dataQueue.size() >= MAX_QUEUE_SIZE) {
            dataQueue.pop(); // Remove oldest
        }
        
        CompressedSensorData compressed = compress(reading);
        dataQueue.push(compressed);
    }
    
    bool transmitQueuedData() {
        while (!dataQueue.empty() && hasConnectivity()) {
            auto data = dataQueue.front();
            if (transmitData(data)) {
                dataQueue.pop();
            } else {
                return false; // Transmission failed
            }
        }
        return true;
    }
};
```

## Network Configuration

### APN Settings
```cpp
// Configure NB-IoT APN
AT+CGDCONT=1,"NONIP","iot.provider.nbiot"

// For IP applications
AT+CGDCONT=1,"IP","nbiot.provider.com"
```

### Common NB-IoT APNs by Carrier
- **Vodafone**: "nb.iot.vodafone.com"
- **Deutsche Telekom**: "iot.telekom.com"
- **China Mobile**: "cmnbiot"
- **AT&T**: "m2m.com.attz"

### Network Registration
```cpp
bool waitForNetworkRegistration(uint32_t timeoutMs = 120000) {
    uint32_t startTime = millis();
    
    while (millis() - startTime < timeoutMs) {
        sendATCommand("AT+CEREG?");
        String response = getATResponse();
        
        // Check for registration status
        if (response.indexOf("+CEREG: 0,1") != -1 || 
            response.indexOf("+CEREG: 0,5") != -1) {
            return true; // Registered
        }
        
        delay(5000); // Check every 5 seconds
    }
    
    return false; // Timeout
}
```

## Communication Protocols

### UDP Socket Communication
```cpp
bool sendDataViaUDP(const uint8_t* data, size_t length) {
    // Create UDP socket
    sendATCommand("AT+NSOCR=DGRAM,17,8080,1");
    
    // Send data to server
    String hexData = bytesToHex(data, length);
    sendATCommand("AT+NSOST=0,\"server.com\",5000," + 
                  String(length) + ",\"" + hexData + "\"");
    
    String response = getATResponse();
    return response.indexOf("OK") != -1;
}
```

### CoAP Integration
```cpp
// Lightweight CoAP implementation for NB-IoT
class CoAPClient {
public:
    bool sendReading(const SensorReading& reading) {
        // Create CoAP POST message
        CoAPMessage msg;
        msg.setType(COAP_TYPE_NON);
        msg.setMethod(COAP_METHOD_POST);
        msg.setUri("/sensors/wildlife-cam");
        msg.setPayload(serialize(reading));
        
        return transmitMessage(msg);
    }
};
```

## Antenna Design for NB-IoT

### Frequency Bands
- **Band 1**: 2100 MHz (FDD)
- **Band 3**: 1800 MHz (FDD)
- **Band 8**: 900 MHz (FDD)
- **Band 20**: 800 MHz (FDD)
- **Band 28**: 700 MHz (FDD)

### Antenna Requirements
- **VSWR**: <2:1 across all bands
- **Peak Gain**: >-2dBi minimum
- **Efficiency**: >40% across bands

### PCB Antenna Design
```
Ground Plane Requirements:
- Minimum 40mm x 60mm for 700MHz
- No ground plane under antenna element
- Solid ground connection for counterpoise
```

## Testing and Validation

### Signal Quality Assessment
```cpp
void checkSignalQuality() {
    // RSRP (Reference Signal Received Power)
    sendATCommand("AT+NUESTATS=\"RADIO\"");
    
    // Expected range: -140 to -44 dBm
    // Good signal: > -100 dBm
    // Marginal: -110 to -100 dBm
    // Poor: < -110 dBm
}
```

### Coverage Testing
```cpp
void performCoverageTest() {
    for (int attempt = 0; attempt < 10; attempt++) {
        unsigned long startTime = millis();
        
        bool success = transmitTestMessage();
        unsigned long duration = millis() - startTime;
        
        Serial.printf("Attempt %d: %s (Duration: %dms)\n", 
                     attempt + 1, 
                     success ? "SUCCESS" : "FAILED", 
                     duration);
        
        delay(60000); // Wait 1 minute between tests
    }
}
```

### Power Consumption Measurement
```cpp
// Monitor current consumption in different modes
void measurePowerConsumption() {
    // Active transmission
    Serial.println("Measuring active power...");
    float activeCurrent = measureCurrent();
    
    // Enter PSM mode
    sendATCommand("AT+CPSMS=1");
    delay(30000); // Wait for PSM activation
    
    // PSM power consumption
    Serial.println("Measuring PSM power...");
    float psmCurrent = measureCurrent();
    
    Serial.printf("Active: %.1fmA, PSM: %.1fuA\n", 
                  activeCurrent, psmCurrent * 1000);
}
```

## Deployment Considerations

### Site Survey Requirements
1. **Signal Strength Mapping**
   - Test RSRP levels at deployment location
   - Verify coverage throughout camera detection range
   - Document seasonal variations

2. **Interference Assessment**
   - Check for local RF interference sources
   - Test during different times of day
   - Validate antenna placement options

### Installation Guidelines
- **Antenna Positioning**: Vertical polarization, clear line-of-sight
- **Cable Routing**: Minimize cable length, avoid sharp bends
- **Weatherproofing**: IP67 rating minimum for outdoor installations
- **Grounding**: Proper RF grounding for lightning protection

## Cost Analysis

### Module Costs
- **Quectel BC95-G**: $8-12 in volume
- **SIMCom SIM7020**: $6-10 in volume
- **Development cost**: Additional $15-25 for integration

### Data Plan Economics
- **Typical NB-IoT rates**: $0.05-0.20 per MB
- **Daily data budget**: 1-10 KB for status updates
- **Monthly cost**: $1-5 for basic monitoring

### Battery Life Impact
- **Daily consumption**: 5-50mAh depending on transmission frequency
- **PSM efficiency**: 95%+ power reduction when inactive
- **Expected battery life**: 6+ months with proper optimization