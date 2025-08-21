# Swarm Satellite Integration Guide

## Overview
Swarm Technologies provides cost-effective satellite IoT connectivity through a constellation of small satellites. This guide covers integration of Swarm M138 modules for affordable global connectivity in wildlife monitoring applications.

## Swarm Network Characteristics

### Constellation Specifications
- **Satellites**: 150+ active SpaceBEE satellites
- **Altitude**: 450-550 km (Low Earth Orbit)
- **Orbital Planes**: Multiple inclinations for global coverage
- **Pass Frequency**: 1-2 passes per hour at most locations
- **Message Delivery**: Store-and-forward with 24-48 hour delivery guarantee
- **Coverage**: Global (pending regulatory approval in some regions)

### Service Features
- **Message Size**: Up to 192 bytes per message
- **Delivery Confirmation**: Message receipt acknowledgment
- **Global Coverage**: Worldwide connectivity
- **Cost Effective**: Significantly lower than traditional satellite services
- **Developer Friendly**: Simple AT command interface

## Hardware Specifications

### Swarm M138 Module
```
Parameter               Value                Notes
---------               -----                -----
Operating Voltage       3.0V - 5.5V         Wide input range
Typical Current         40mA @ 3.3V         During operation
Peak Current           150mA @ 3.3V         During transmission
Sleep Current          20µA                 Deep sleep mode
Operating Temperature  -40°C to +85°C       Extended range
Dimensions             34 x 24 x 4 mm       Compact design
Weight                 5g                   Ultra-lightweight
Communication          UART                 Simple serial interface
```

### Pin Configuration
```
Pin     Name        Description
---     ----        -----------
1       VCC         Power supply (3.0-5.5V)
2       GND         Ground
3       TXD         UART transmit (module to host)
4       RXD         UART receive (host to module)
5       GPIO1       General purpose I/O
6       GPIO2       General purpose I/O
7       RST         Reset (active low)
8       BOOT        Boot mode selection
```

## Integration with ESP32

### Hardware Connections
```
ESP32-S3    Swarm M138      Function
--------    ----------      --------
GPIO25      RXD             Serial TX (ESP32 to Swarm)
GPIO26      TXD             Serial RX (ESP32 from Swarm)
GPIO27      RST             Module reset control
GPIO33      GPIO1           Status/interrupt pin
3.3V        VCC             Power supply
GND         GND             Ground reference
```

### Circuit Design
```
                    ESP32-S3
                 ┌──────────────┐
                 │          D25 ├───┐
                 │          D26 ├───┤    Swarm M138
                 │          D27 ├───┤  ┌──────────────┐
                 │          D33 ├───┤  │          RXD ├───┐
                 │              │   └──┤          TXD ├───┤
                 │         3.3V ├──────┤          RST ├───┤
                 │          GND ├──────┤        GPIO1 ├───┤
                 └──────────────┘      │          VCC ├─── 3.3V
                                       │          GND ├─── GND
                                       │          ANT ├─── Antenna
                                       └──────────────┘
```

### Power Supply Design
```
Battery+ ── LDO 3.3V ──┬── ESP32
                       └── Swarm M138
                       
Filter Capacitors:
VCC ──┬── 100µF (bulk)
      └── 100nF (bypass) ── GND
```

## Antenna Requirements

### Frequency Specifications
- **Operating Frequency**: 150 MHz (VHF band)
- **Bandwidth**: ±1 MHz around center frequency
- **Impedance**: 50Ω
- **Polarization**: Linear (vertical preferred)

### Antenna Options

1. **Quarter-Wave Monopole (Recommended)**
   ```
   Length: λ/4 = 75cm / 4 = 50cm
   Type: Whip antenna with ground plane
   Gain: 0-2 dBi
   Cost: $5-15
   ```

2. **Helical Antenna**
   ```
   Type: Normal mode helix
   Gain: 3-5 dBi
   Size: 20cm length x 5cm diameter
   Cost: $20-40
   ```

3. **Yagi Antenna**
   ```
   Type: 3-5 element Yagi
   Gain: 6-10 dBi (directional)
   Size: 1-2 meter boom
   Cost: $30-80
   Use: Fixed installations only
   ```

### Antenna Design Guidelines
```
Ground Plane Requirements:
- Minimum diameter: 60cm for quarter-wave monopole
- Radial wires: 4-8 radials, each λ/4 length
- Height above ground: >λ/4 (50cm) for best performance

PCB Antenna (Compact Option):
- Meandered monopole on PCB edge
- Ground plane: 40x40mm minimum
- Efficiency: 20-40% (significant loss)
```

## Software Implementation

### Initialization and Setup
```cpp
#include <SoftwareSerial.h>

class SwarmModem {
private:
    SoftwareSerial swarmSerial;
    bool isConnected;
    
public:
    SwarmModem(int rxPin, int txPin) : swarmSerial(rxPin, txPin), isConnected(false) {}
    
    bool initialize() {
        swarmSerial.begin(115200);
        
        // Reset module
        digitalWrite(RST_PIN, LOW);
        delay(100);
        digitalWrite(RST_PIN, HIGH);
        delay(2000);
        
        // Test communication
        if (sendCommand("$CS*", "$CS")) {
            Serial.println("Swarm module detected");
            return configureModule();
        }
        
        return false;
    }
    
    bool configureModule() {
        // Get device information
        sendCommand("$DI*", "$DI");
        
        // Check GPS status
        sendCommand("$GS*", "$GS");
        
        // Get RSSI
        sendCommand("$RT RSSI*", "$RT RSSI");
        
        isConnected = true;
        return true;
    }
};
```

### Message Transmission
```cpp
bool sendMessage(const String& message) {
    if (message.length() > 192) {
        Serial.println("Message too long for Swarm (max 192 bytes)");
        return false;
    }
    
    // Check if GPS is available (required for transmission)
    if (!isGPSValid()) {
        Serial.println("GPS fix required for Swarm transmission");
        return false;
    }
    
    // Transmit data message
    String command = "$TD \"" + message + "\"*";
    return sendCommand(command, "$TD OK");
}

bool sendBinaryMessage(const uint8_t* data, size_t length) {
    if (length > 192) return false;
    
    // Convert binary to hex string
    String hexMessage = "";
    for (size_t i = 0; i < length; i++) {
        char hex[3];
        sprintf(hex, "%02X", data[i]);
        hexMessage += hex;
    }
    
    String command = "$TD \"" + hexMessage + "\"*";
    return sendCommand(command, "$TD OK");
}
```

### GPS Integration
```cpp
struct SwarmGPS {
    bool valid;
    double latitude;
    double longitude;
    float altitude;
    int satellites;
};

SwarmGPS getGPSStatus() {
    SwarmGPS gps = {false, 0, 0, 0, 0};
    
    if (sendCommand("$GS*", "$GS")) {
        String response = getLastResponse();
        
        // Parse GPS response: $GS <valid>,<lat>,<lon>,<alt>,<course>,<speed>,<sats>*
        if (response.indexOf("$GS 1,") != -1) {
            // Parse coordinates
            int start = response.indexOf("$GS 1,") + 6;
            int end = response.indexOf('*');
            String data = response.substring(start, end);
            
            // Split comma-separated values
            int commaPos[6];
            int commaCount = 0;
            for (int i = 0; i < data.length() && commaCount < 6; i++) {
                if (data.charAt(i) == ',') {
                    commaPos[commaCount++] = i;
                }
            }
            
            if (commaCount >= 5) {
                gps.valid = true;
                gps.latitude = data.substring(0, commaPos[0]).toFloat();
                gps.longitude = data.substring(commaPos[0] + 1, commaPos[1]).toFloat();
                gps.altitude = data.substring(commaPos[1] + 1, commaPos[2]).toFloat();
                gps.satellites = data.substring(commaPos[4] + 1, commaPos[5]).toInt();
            }
        }
    }
    
    return gps;
}

bool isGPSValid() {
    SwarmGPS gps = getGPSStatus();
    return gps.valid && gps.satellites >= 4;
}
```

### Signal Quality Assessment
```cpp
int getRSSI() {
    if (sendCommand("$RT RSSI*", "$RT RSSI")) {
        String response = getLastResponse();
        int rssiStart = response.indexOf("RSSI") + 5;
        int rssiEnd = response.indexOf('*', rssiStart);
        
        if (rssiStart > 4 && rssiEnd > rssiStart) {
            String rssiStr = response.substring(rssiStart, rssiEnd);
            return rssiStr.toInt();
        }
    }
    return -999; // Invalid reading
}

bool isSignalAvailable() {
    int rssi = getRSSI();
    return rssi > -120; // Minimum usable signal level
}
```

### Message Queue Management
```cpp
class SwarmMessageQueue {
private:
    std::queue<String> messageQueue;
    unsigned long lastTransmission;
    const unsigned long MIN_INTERVAL = 300000; // 5 minutes between messages
    
public:
    void addMessage(const String& message) {
        if (messageQueue.size() < MAX_QUEUE_SIZE) {
            messageQueue.push(message);
        } else {
            // Queue full, remove oldest message
            messageQueue.pop();
            messageQueue.push(message);
        }
    }
    
    bool processQueue() {
        if (messageQueue.empty()) return true;
        
        // Check minimum interval
        if (millis() - lastTransmission < MIN_INTERVAL) {
            return false;
        }
        
        // Check GPS and signal availability
        if (!isGPSValid() || !isSignalAvailable()) {
            return false;
        }
        
        // Transmit next message
        String message = messageQueue.front();
        if (sendMessage(message)) {
            messageQueue.pop();
            lastTransmission = millis();
            Serial.println("Message transmitted successfully");
            return true;
        }
        
        return false;
    }
    
    size_t getQueueSize() {
        return messageQueue.size();
    }
};
```

## Data Compression for Swarm

### Efficient Data Encoding
```cpp
class SwarmDataEncoder {
public:
    // Compress sensor data for 192-byte limit
    String encodeSensorData(const SensorReading& reading) {
        // Use compact binary encoding
        uint8_t encoded[32];
        size_t pos = 0;
        
        // Timestamp (4 bytes - Unix time)
        uint32_t timestamp = reading.timestamp;
        encoded[pos++] = (timestamp >> 24) & 0xFF;
        encoded[pos++] = (timestamp >> 16) & 0xFF;
        encoded[pos++] = (timestamp >> 8) & 0xFF;
        encoded[pos++] = timestamp & 0xFF;
        
        // GPS coordinates (8 bytes - lat/lon as fixed point)
        int32_t lat = reading.latitude * 1000000;
        int32_t lon = reading.longitude * 1000000;
        
        encoded[pos++] = (lat >> 24) & 0xFF;
        encoded[pos++] = (lat >> 16) & 0xFF;
        encoded[pos++] = (lat >> 8) & 0xFF;
        encoded[pos++] = lat & 0xFF;
        
        encoded[pos++] = (lon >> 24) & 0xFF;
        encoded[pos++] = (lon >> 16) & 0xFF;
        encoded[pos++] = (lon >> 8) & 0xFF;
        encoded[pos++] = lon & 0xFF;
        
        // Sensor data (4 bytes)
        encoded[pos++] = reading.batteryLevel;
        encoded[pos++] = (int8_t)reading.temperature;
        encoded[pos++] = reading.humidity;
        encoded[pos++] = reading.motionDetected ? 1 : 0;
        
        // Convert to base64 for transmission
        return encodeBase64(encoded, pos);
    }
    
    // Compress image metadata
    String encodeImageMetadata(const ImageMetadata& metadata) {
        String json = "{";
        json += "\"t\":" + String(metadata.timestamp) + ",";
        json += "\"w\":" + String(metadata.width) + ",";
        json += "\"h\":" + String(metadata.height) + ",";
        json += "\"s\":" + String(metadata.size) + ",";
        json += "\"m\":" + String(metadata.motionPixels);
        json += "}";
        
        return json;
    }
};
```

### Image Thumbnail for Swarm
```cpp
String createImageThumbnail(const uint8_t* imageData, size_t imageSize) {
    // Create ultra-compressed thumbnail for Swarm transmission
    const int THUMB_WIDTH = 16;
    const int THUMB_HEIGHT = 12;
    const int THUMB_SIZE = THUMB_WIDTH * THUMB_HEIGHT;
    
    uint8_t thumbnail[THUMB_SIZE];
    
    // Extreme downsampling
    int skipX = 640 / THUMB_WIDTH;  // Assuming VGA source
    int skipY = 480 / THUMB_HEIGHT;
    
    for (int y = 0; y < THUMB_HEIGHT; y++) {
        for (int x = 0; x < THUMB_WIDTH; x++) {
            int srcX = x * skipX;
            int srcY = y * skipY;
            int srcIndex = srcY * 640 + srcX;
            
            if (srcIndex < imageSize) {
                thumbnail[y * THUMB_WIDTH + x] = imageData[srcIndex];
            }
        }
    }
    
    // Further compress using RLE
    String compressed = compressRLE(thumbnail, THUMB_SIZE);
    return compressed;
}
```

## Testing and Validation

### Communication Testing
```cpp
void testSwarmCommunication() {
    Serial.println("Starting Swarm communication test...");
    
    // Test 1: Module detection
    if (!swarm.initialize()) {
        Serial.println("FAIL: Module not responding");
        return;
    }
    Serial.println("PASS: Module detected");
    
    // Test 2: GPS acquisition
    Serial.println("Waiting for GPS fix...");
    unsigned long gpsStart = millis();
    while (!isGPSValid() && millis() - gpsStart < 300000) { // 5 minute timeout
        delay(10000);
        Serial.print(".");
    }
    
    if (isGPSValid()) {
        SwarmGPS gps = getGPSStatus();
        Serial.printf("PASS: GPS fix acquired (Sats: %d)\n", gps.satellites);
    } else {
        Serial.println("FAIL: GPS timeout");
        return;
    }
    
    // Test 3: Signal quality
    int rssi = getRSSI();
    Serial.printf("Signal RSSI: %d dBm\n", rssi);
    
    // Test 4: Message transmission
    String testMessage = "Swarm test from wildlife camera";
    if (sendMessage(testMessage)) {
        Serial.println("PASS: Test message transmitted");
    } else {
        Serial.println("FAIL: Message transmission failed");
    }
}
```

### Performance Monitoring
```cpp
class SwarmPerformanceMonitor {
private:
    int messagesAttempted;
    int messagesSuccessful;
    unsigned long totalTransmissionTime;
    
public:
    void recordTransmission(bool success, unsigned long duration) {
        messagesAttempted++;
        if (success) {
            messagesSuccessful++;
            totalTransmissionTime += duration;
        }
    }
    
    void printStatistics() {
        float successRate = (float)messagesSuccessful / messagesAttempted * 100;
        float avgTime = (float)totalTransmissionTime / messagesSuccessful / 1000;
        
        Serial.printf("Transmission Statistics:\n");
        Serial.printf("  Attempted: %d\n", messagesAttempted);
        Serial.printf("  Successful: %d\n", messagesSuccessful);
        Serial.printf("  Success Rate: %.1f%%\n", successRate);
        Serial.printf("  Avg Time: %.1f seconds\n", avgTime);
    }
};
```

## Power Management

### Low Power Operations
```cpp
void enterLowPowerMode() {
    // Put Swarm module in sleep mode
    sendCommand("$SL*", "$SL OK");
    
    // Configure ESP32 for deep sleep
    esp_sleep_enable_timer_wakeup(3600 * 1000000ULL); // Wake every hour
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0); // Wake on Swarm interrupt
    
    Serial.println("Entering deep sleep...");
    esp_deep_sleep_start();
}

void wakeFromSleep() {
    // Wake Swarm module
    digitalWrite(RST_PIN, LOW);
    delay(100);
    digitalWrite(RST_PIN, HIGH);
    delay(2000);
    
    // Re-initialize communication
    swarm.initialize();
}
```

### Battery Optimization
```cpp
class SwarmPowerManager {
private:
    float batteryVoltage;
    bool lowPowerMode;
    
public:
    void updateBatteryStatus() {
        batteryVoltage = analogRead(BATTERY_PIN) * 3.3 / 4096 * 2; // Voltage divider
        lowPowerMode = batteryVoltage < 3.4; // Low battery threshold
    }
    
    bool shouldTransmit() {
        updateBatteryStatus();
        
        if (lowPowerMode) {
            // In low power mode, only send critical messages
            return false; // Implement priority logic
        }
        
        return true; // Normal operation
    }
    
    void optimizeForBattery() {
        if (lowPowerMode) {
            // Reduce transmission frequency
            // Queue non-critical messages
            // Use maximum compression
        }
    }
};
```

## Cost Analysis

### Hardware Costs
```
Component                Cost (USD)
---------                ----------
Swarm M138 Module        $119
Development Kit          $299  
Antenna (1/4 wave)       $10-25
GPS Antenna (if needed)  $15-30
Integration Components   $5-15
Total Hardware          $168-368
```

### Service Costs
```
Swarm Data Plans:
- $5/month for 750 messages (192 bytes each)
- $60/year prepaid (20% discount)
- Additional messages: $0.007 each
- No activation fees or contracts
```

### Operational Economics
```
Daily message allowance: 25 messages (750/30 days)
Cost per message: ~$0.007
Monthly data budget: ~144KB (750 x 192 bytes)
Annual cost: $60 + hardware depreciation
```

## Best Practices

### Message Optimization
1. **Use binary encoding** instead of text when possible
2. **Compress repeated data** with simple algorithms
3. **Batch multiple sensor readings** into single messages
4. **Prioritize critical data** during low battery conditions

### Reliability Improvements
1. **Implement message acknowledgment** tracking
2. **Use store-and-forward** for non-urgent data
3. **Monitor GPS availability** before transmission attempts
4. **Queue messages** during poor signal conditions

### Power Efficiency
1. **Minimize GPS warm-up time** by maintaining almanac data
2. **Use deep sleep** between transmission windows
3. **Monitor battery voltage** and adjust behavior accordingly
4. **Implement intelligent scheduling** based on satellite passes

This comprehensive guide enables cost-effective global satellite connectivity for wildlife cameras using the Swarm constellation, providing reliable communication in remote locations at a fraction of traditional satellite communication costs.