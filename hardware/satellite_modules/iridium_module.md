# Iridium Satellite Module Integration

## Overview
The Iridium satellite constellation provides global coverage for IoT applications through Short Burst Data (SBD) messaging. This document covers integration of Iridium 9603N modules for reliable satellite communication in remote wildlife monitoring locations.

## Iridium System Characteristics

### Network Specifications
- **Constellation**: 66 active satellites in 6 polar orbital planes
- **Altitude**: 780 km above Earth
- **Coverage**: Complete global coverage including poles
- **Orbit Period**: ~100 minutes per satellite
- **Pass Duration**: 8-20 minutes typical visibility window
- **Latency**: 1-3 seconds for message delivery

### SBD (Short Burst Data) Service
- **Maximum Message Size**: 340 bytes mobile-originated (MO)
- **Maximum Message Size**: 270 bytes mobile-terminated (MT)
- **Delivery Confirmation**: Message receipt acknowledgment
- **Store and Forward**: Messages queued when no satellite visible
- **Global Coverage**: No dead zones or coverage gaps

## Hardware Integration

### Iridium 9603N Module Specifications
```
Parameter               Value               Notes
---------               -----               -----
Operating Voltage       5V ±0.5V           Requires regulated 5V supply
Peak Current           1.3A @ 5V           During transmission bursts
Average Current        150mA typical       During active communication
Sleep Current          50µA                In low-power mode
Operating Temp         -40°C to +85°C      Extended temperature range
Dimensions             24 x 43 x 5.1 mm    Compact form factor
Weight                 13.5g               Lightweight design
```

### ESP32 to Iridium 9603N Connections
```
ESP32-S3    Iridium 9603N    Function
--------    -------------    --------
GPIO16      TXD              Serial RX (ESP32 receives)
GPIO17      RXD              Serial TX (ESP32 transmits)
GPIO18      ON_OFF           Module power control
GPIO19      DTR              Data Terminal Ready
GPIO21      DSR              Data Set Ready  
GPIO22      CTS              Clear To Send
GPIO23      RTS              Request To Send
5V          PWR              5V power supply
GND         GND              Ground reference
```

### Power Supply Design
```
Battery+ ──┬── Buck Converter ──┬── 5V ──┬── Iridium Module
           │    (5V @ 1.5A)     │        │
           │                    │        └── 1000µF + 100µF capacitors
           └── 3.3V Regulator ──┴── ESP32
```

**Critical Requirements:**
- **5V regulation**: Must maintain 5V ±2% during 1.3A transmission bursts
- **Capacitance**: Minimum 1000µF bulk + 100µF ceramic bypass
- **Current capability**: 1.5A minimum for reliable operation

## Circuit Design

### Minimal Integration Schematic
```
                    ESP32-S3
                 ┌──────────────┐
                 │          D16 ├───┐
                 │          D17 ├───┤
                 │          D18 ├───┤    Iridium 9603N
                 │          D19 ├───┤  ┌────────────────┐
                 │          D21 ├───┤  │            TXD ├───┐
                 │          D22 ├───┤  │            RXD ├───┤
                 │          D23 ├───┤  │         ON_OFF ├───┤
                 │              │   └──┤            DTR ├───┤
                 │         3.3V ├──────┤            DSR ├───┤
                 │          GND ├──────┤            CTS ├───┤
                 └──────────────┘      │            RTS ├───┤
                                       │            PWR ├─── 5V
                                       │            GND ├─── GND
                                       │            ANT ├─── Antenna
                                       └────────────────┘
```

### Power Management Circuit
```
5V_IN ──┬── [1000µF] ──┬── [100µF] ── VCC (Iridium)
        │              │
        └── [Ferrite] ──┴── [10µF] ─── Power Sense
        
ON_OFF ── [10kΩ] ── VCC
       └─ [MOSFET] ── Power Switch
```

### Antenna Interface
```
Iridium ANT ── [0Ω or small inductor] ── SMA Connector ── Antenna

EMI Filtering (if required):
ANT ──┬── [L1: 10nH] ── SMA
      │
      └── [C1: 1pF] ── GND
```

## Antenna Requirements

### Iridium Frequency Band
- **Uplink (Mobile to Satellite)**: 1616.0 - 1626.5 MHz
- **Downlink (Satellite to Mobile)**: 1616.0 - 1626.5 MHz
- **Bandwidth**: 10.5 MHz
- **Channel Spacing**: 41.67 kHz

### Antenna Specifications
```
Parameter           Requirement      Typical Value
---------           -----------      -------------
Frequency Range     1610-1630 MHz    1616-1626.5 MHz
Impedance           50Ω ±10%         50Ω
VSWR               <2:1             <1.5:1
Gain               >0 dBi           +2 to +5 dBi
Polarization       LHCP             Left-hand circular
Beam Width         >120° @ 3dB      Omnidirectional
Efficiency         >50%             >70%
```

### Recommended Antenna Types

1. **Helical Antenna (Recommended)**
   ```
   Type: Quadrifilar helix
   Gain: +3 to +5 dBi
   Polarization: LHCP (ideal for Iridium)
   Size: 40mm diameter x 60mm height
   Cost: $50-100
   ```

2. **Patch Antenna**
   ```
   Type: Circular patch with sequential feed
   Gain: +4 to +6 dBi
   Polarization: LHCP
   Size: 80mm x 80mm x 10mm
   Cost: $30-60
   ```

3. **Whip Antenna (Budget Option)**
   ```
   Type: Monopole with ground plane
   Gain: 0 to +2 dBi
   Polarization: Linear (3dB penalty)
   Size: 70mm length
   Cost: $10-20
   ```

## Software Implementation

### AT Command Interface
```cpp
class IridiumModem {
private:
    HardwareSerial modemSerial;
    
public:
    bool initialize() {
        // Power on sequence
        digitalWrite(ON_OFF_PIN, HIGH);
        delay(500);
        digitalWrite(ON_OFF_PIN, LOW);
        delay(1000);
        
        // Test communication
        return sendCommand("AT", "OK", 5000);
    }
    
    bool sendCommand(String cmd, String expectedResponse, int timeout) {
        modemSerial.println(cmd);
        
        unsigned long startTime = millis();
        String response = "";
        
        while (millis() - startTime < timeout) {
            if (modemSerial.available()) {
                response += modemSerial.readString();
                if (response.indexOf(expectedResponse) != -1) {
                    return true;
                }
            }
            delay(100);
        }
        return false;
    }
};
```

### SBD Message Transmission
```cpp
bool sendSBDMessage(const uint8_t* data, size_t length) {
    if (length > 340) {
        Serial.println("Message too large for SBD");
        return false;
    }
    
    // Clear mobile originated buffer
    if (!sendCommand("AT+SBDD0", "OK", 5000)) {
        return false;
    }
    
    // Write binary data to buffer
    modemSerial.print("AT+SBDWB=");
    modemSerial.println(length);
    
    delay(1000);
    
    // Send binary data
    for (size_t i = 0; i < length; i++) {
        modemSerial.write(data[i]);
    }
    
    // Calculate and send checksum
    uint16_t checksum = 0;
    for (size_t i = 0; i < length; i++) {
        checksum += data[i];
    }
    
    modemSerial.write(checksum >> 8);
    modemSerial.write(checksum & 0xFF);
    
    // Wait for READY response
    if (!sendCommand("", "READY", 10000)) {
        return false;
    }
    
    // Initiate SBD session
    return sendCommand("AT+SBDI", "+SBDI:", 300000); // 5 minute timeout
}
```

### Signal Quality Assessment
```cpp
int getSignalQuality() {
    if (sendCommand("AT+CSQ", "+CSQ:", 10000)) {
        String response = getLastResponse();
        int csqIndex = response.indexOf("+CSQ:");
        
        if (csqIndex != -1) {
            int signalStrength = response.substring(csqIndex + 5).toInt();
            
            // Iridium signal strength scale:
            // 0: No signal
            // 1: Very weak
            // 2: Weak  
            // 3: Good
            // 4: Strong
            // 5: Very strong
            
            return signalStrength;
        }
    }
    return -1;
}
```

### Satellite Visibility Prediction
```cpp
class SatellitePredictor {
private:
    struct SatellitePass {
        unsigned long startTime;
        unsigned long endTime;
        int maxElevation;
    };
    
public:
    bool predictNextPass(double latitude, double longitude) {
        // Simplified prediction - in reality would use TLE data
        // Iridium satellites pass approximately every 100 minutes
        
        unsigned long currentTime = getCurrentTime();
        unsigned long nextPass = currentTime + (random(90, 110) * 60); // 90-110 minutes
        
        Serial.printf("Next satellite pass predicted: %s\n", 
                     timeToString(nextPass).c_str());
        
        return true;
    }
    
    bool isOptimalTime() {
        int signalQuality = getSignalQuality();
        return signalQuality >= 2; // Good or better signal
    }
};
```

## Communication Protocols

### SBD Message Structure
```cpp
struct IridiumMessage {
    uint16_t messageId;      // Unique message identifier
    uint8_t messageType;     // Message type (data, status, emergency)
    uint32_t timestamp;      // Unix timestamp
    float latitude;          // GPS coordinates
    float longitude;
    uint16_t batteryVoltage; // Battery status
    uint8_t payload[320];    // Application data
    uint16_t crc;           // Error detection
} __attribute__((packed));
```

### Data Compression for SBD
```cpp
class IridiumCompressor {
public:
    size_t compressImage(const uint8_t* image, size_t imageSize, 
                        uint8_t* compressed, size_t maxSize) {
        // Ultra-aggressive compression for SBD constraints
        
        if (imageSize < 1000) {
            // Small image - send thumbnail only
            return createThumbnail(image, imageSize, compressed, maxSize, 32, 24);
        } else {
            // Large image - send metadata and sample pixels
            return createImageSummary(image, imageSize, compressed, maxSize);
        }
    }
    
private:
    size_t createImageSummary(const uint8_t* image, size_t imageSize,
                             uint8_t* summary, size_t maxSize) {
        size_t summarySize = 0;
        
        // Header
        summary[summarySize++] = 0xAA; // Image summary marker
        summary[summarySize++] = (imageSize >> 8) & 0xFF;
        summary[summarySize++] = imageSize & 0xFF;
        
        // Sample key pixels (every Nth pixel)
        size_t sampleInterval = imageSize / (maxSize - 10);
        for (size_t i = 0; i < imageSize && summarySize < maxSize - 1; i += sampleInterval) {
            summary[summarySize++] = image[i];
        }
        
        return summarySize;
    }
};
```

## Testing and Validation

### Signal Quality Testing
```cpp
void performSignalTest() {
    Serial.println("Starting Iridium signal quality test...");
    
    for (int i = 0; i < 10; i++) {
        int signalQuality = getSignalQuality();
        
        Serial.printf("Test %d: Signal quality = %d", i + 1, signalQuality);
        
        switch (signalQuality) {
            case 0: Serial.println(" (No signal)"); break;
            case 1: Serial.println(" (Very weak)"); break;
            case 2: Serial.println(" (Weak)"); break;
            case 3: Serial.println(" (Good)"); break;
            case 4: Serial.println(" (Strong)"); break;
            case 5: Serial.println(" (Very strong)"); break;
            default: Serial.println(" (Error)"); break;
        }
        
        delay(30000); // Wait 30 seconds between tests
    }
}
```

### Message Transmission Testing
```cpp
void testMessageTransmission() {
    uint8_t testMessage[] = "Wildlife Camera Test Message";
    size_t messageSize = strlen((char*)testMessage);
    
    unsigned long startTime = millis();
    bool success = sendSBDMessage(testMessage, messageSize);
    unsigned long duration = millis() - startTime;
    
    Serial.printf("Transmission %s (Duration: %d seconds)\n",
                  success ? "successful" : "failed",
                  duration / 1000);
}
```

### Power Consumption Analysis
```cpp
void analyzePowerConsumption() {
    // Measure idle current
    Serial.println("Measuring idle power...");
    float idleCurrent = measureCurrent();
    
    // Measure transmission current
    Serial.println("Measuring transmission power...");
    uint8_t testData[] = "Test";
    unsigned long startTime = millis();
    sendSBDMessage(testData, sizeof(testData));
    unsigned long transmissionTime = millis() - startTime;
    float txCurrent = measurePeakCurrent();
    
    // Calculate energy consumption
    float energyPerMessage = (txCurrent * transmissionTime) / 3600000.0; // Wh
    
    Serial.printf("Idle: %.1fmA, TX: %.1fmA, Energy/msg: %.2fmWh\n",
                  idleCurrent, txCurrent, energyPerMessage * 1000);
}
```

## Operational Considerations

### Message Scheduling
```cpp
class MessageScheduler {
private:
    std::queue<IridiumMessage> messageQueue;
    unsigned long lastTransmission;
    const unsigned long MIN_INTERVAL = 60000; // 1 minute minimum
    
public:
    void scheduleMessage(const IridiumMessage& msg) {
        messageQueue.push(msg);
    }
    
    bool processQueue() {
        if (messageQueue.empty()) return true;
        
        if (millis() - lastTransmission < MIN_INTERVAL) {
            return false; // Too soon for next transmission
        }
        
        if (getSignalQuality() < 2) {
            return false; // Poor signal, wait for better conditions
        }
        
        IridiumMessage msg = messageQueue.front();
        if (transmitMessage(msg)) {
            messageQueue.pop();
            lastTransmission = millis();
            return true;
        }
        
        return false;
    }
};
```

### Emergency Communications
```cpp
bool sendEmergencyAlert(const String& alertMessage) {
    // Emergency messages get priority transmission
    IridiumMessage emergency;
    emergency.messageType = 0xFF; // Emergency marker
    emergency.timestamp = getCurrentTime();
    
    GPS.getCurrentLocation(&emergency.latitude, &emergency.longitude);
    
    strncpy((char*)emergency.payload, alertMessage.c_str(), 
            min(alertMessage.length(), sizeof(emergency.payload)));
    
    // Attempt transmission with retries
    for (int attempt = 0; attempt < 3; attempt++) {
        if (sendSBDMessage((uint8_t*)&emergency, sizeof(emergency))) {
            Serial.println("Emergency alert transmitted successfully");
            return true;
        }
        
        delay(60000); // Wait 1 minute between retries
    }
    
    Serial.println("Failed to transmit emergency alert");
    return false;
}
```

## Cost Analysis

### Hardware Costs
```
Component                   Cost (USD)
---------                   ----------
Iridium 9603N Module        $150-200
Development Board           $250-300
Antenna (Helical)           $60-100
Power Supply Components     $10-20
Integration Labor           $200-500
Total Hardware             $670-1120
```

### Service Costs
```
Iridium SBD Service Plans:
- Pay-per-use: $0.04-0.08 per message
- Monthly plans: $12-50/month for 25-1000 messages
- Annual contracts: Significant discounts available
```

### Operational Costs
```
Daily message budget: 10 messages
Monthly cost: $12-24 depending on plan
Annual cost: $144-288 plus hardware
```

This comprehensive guide enables reliable Iridium satellite communication integration for wildlife cameras operating in remote locations where terrestrial connectivity is unavailable.