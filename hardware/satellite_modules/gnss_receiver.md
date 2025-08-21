# GNSS Receiver Integration Guide

## Overview
Global Navigation Satellite Systems (GNSS) provide essential location and timing services for the ESP32 Wildlife Camera. This guide covers integration of multi-constellation GNSS receivers for accurate positioning, satellite timing, and enhanced communication coordination.

## GNSS Constellation Support

### Supported Systems
```
System      Satellites   Frequency Bands         Coverage
------      ----------   ---------------         --------
GPS         31 active    L1 (1575.42 MHz)       Global
GLONASS     24 active    L1 (1600 MHz)          Global  
Galileo     24+ active   E1 (1575.42 MHz)       Global
BeiDou      35+ active   B1 (1561.098 MHz)      Global
QZSS        4+ active    L1 (1575.42 MHz)       Asia-Pacific
IRNSS       7 active     L5 (1176.45 MHz)       India region
```

### Multi-Constellation Benefits
- **Improved Accuracy**: More satellites = better positioning
- **Faster TTFF**: Time To First Fix reduced from 30s to 5-10s
- **Urban Canyon Performance**: Better signal availability in obstructed areas
- **Reliability**: Redundancy against single constellation failures

## Recommended GNSS Modules

### 1. u-blox ZED-F9P (High Precision)
```
Specifications:
- Accuracy: 10cm with RTK correction
- Constellations: GPS, GLONASS, Galileo, BeiDou, QZSS
- Update Rate: Up to 20Hz
- Power: 67mA typical
- Interface: UART, I2C, SPI
- Price: $180-250
```

### 2. u-blox NEO-M8N (Standard)
```
Specifications:
- Accuracy: 2.5m CEP
- Constellations: GPS, GLONASS, Galileo, BeiDou, QZSS
- Update Rate: Up to 10Hz
- Power: 23mA typical
- Interface: UART, I2C
- Price: $25-40
```

### 3. SIMCom SIM68M (Budget)
```
Specifications:
- Accuracy: 2.5m CEP
- Constellations: GPS, GLONASS, Galileo, BeiDou
- Update Rate: Up to 5Hz
- Power: 20mA typical
- Interface: UART
- Price: $8-15
```

## Hardware Integration

### ESP32 to GNSS Module Connections
```
ESP32-S3    GNSS Module     Function
--------    -----------     --------
GPIO32      RXD             Serial TX (ESP32 to GNSS)
GPIO33      TXD             Serial RX (ESP32 from GNSS)
GPIO25      PPS             Pulse Per Second timing
GPIO26      EN              Module enable/disable
GPIO27      RST             Reset control
3.3V        VCC             Power supply
GND         GND             Ground
```

### Circuit Design
```
                    ESP32-S3
                 ┌──────────────┐
                 │          D32 ├───┐
                 │          D33 ├───┤    GNSS Module
                 │          D25 ├───┤  ┌────────────────┐
                 │          D26 ├───┤  │            RXD ├───┐
                 │          D27 ├───┤  │            TXD ├───┤
                 │              │   └──┤            PPS ├───┤
                 │         3.3V ├──────┤             EN ├───┤
                 │          GND ├──────┤            RST ├───┤
                 └──────────────┘      │            VCC ├─── 3.3V
                                       │            GND ├─── GND
                                       │            ANT ├─── GNSS Antenna
                                       └────────────────┘
```

### Power Supply Considerations
```
Battery+ ── Buck/LDO ── 3.3V ──┬── ESP32
                               └── GNSS Module
                               
Backup Power (Optional):
VCC ──┬── Main Power
      └── [Diode] ── Supercap/Battery ── Backup Power
```

## Antenna Design and Selection

### GNSS Antenna Requirements
```
Parameter           Requirement         Typical Value
---------           -----------         -------------
Frequency Range     1559-1610 MHz       L1/E1/B1 bands
Gain               >0 dBic             +3 to +5 dBic
Polarization       RHCP                Right-hand circular
Axial Ratio        <3 dB               <2 dB preferred
Impedance          50Ω ±10%            50Ω
VSWR               <2:1                <1.5:1
```

### Antenna Types

1. **Patch Antenna (Recommended)**
   ```
   Type: Ceramic patch with RHCP
   Size: 25x25x4mm typical
   Gain: +3 to +5 dBic
   Cost: $10-25
   Pros: Good performance, compact
   Cons: Requires ground plane
   ```

2. **Helical Antenna**
   ```
   Type: Quadrifilar helix
   Size: 15mm diameter x 30mm height
   Gain: +2 to +4 dBic
   Cost: $15-35
   Pros: Omnidirectional, good multipath rejection
   Cons: Larger size
   ```

3. **Active Antenna**
   ```
   Type: Patch with integrated LNA
   Gain: +25 to +35 dB (including LNA)
   Power: 3-5mA @ 3.3V
   Cost: $20-50
   Pros: High sensitivity
   Cons: Requires power, more complex
   ```

### Antenna Placement Guidelines
```
Optimal Placement:
- Clear sky view (minimal obstructions above 15° elevation)
- Away from metal objects (>10cm separation)
- Horizontal orientation for patch antennas
- Weather protection for outdoor installations

Avoid:
- Direct contact with camera module
- Proximity to switching power supplies
- Metal enclosures without antenna cutouts
- Orientation with patch facing down
```

## Software Implementation

### GNSS Module Initialization
```cpp
#include <SoftwareSerial.h>

class GNSSReceiver {
private:
    SoftwareSerial gnssSerial;
    bool isInitialized;
    volatile bool ppsReceived;
    
public:
    GNSSReceiver(int rxPin, int txPin) : gnssSerial(rxPin, txPin), isInitialized(false), ppsReceived(false) {}
    
    bool initialize() {
        gnssSerial.begin(9600); // Standard NMEA baud rate
        
        // Enable module
        digitalWrite(EN_PIN, HIGH);
        delay(100);
        
        // Reset module
        digitalWrite(RST_PIN, LOW);
        delay(100);
        digitalWrite(RST_PIN, HIGH);
        delay(1000);
        
        // Configure for multi-constellation
        return configureConstellations();
    }
    
    bool configureConstellations() {
        // Enable GPS + GLONASS + Galileo + BeiDou (u-blox specific)
        sendUBXCommand("$PUBX,40,GLL,0,0,0,0*5C"); // Disable GLL
        sendUBXCommand("$PUBX,40,GSA,0,0,0,0*4E"); // Disable GSA
        sendUBXCommand("$PUBX,40,GSV,0,0,0,0*59"); // Disable GSV
        sendUBXCommand("$PUBX,40,VTG,0,0,0,0*5E"); // Disable VTG
        
        // Keep GGA and RMC only for efficiency
        sendUBXCommand("$PUBX,40,GGA,0,1,0,0*5B"); // Enable GGA
        sendUBXCommand("$PUBX,40,RMC,0,1,0,0*46"); // Enable RMC
        
        // Set update rate to 1Hz for power efficiency
        sendUBXCommand("$PUBX,40,GGA,0,1,0,0*5B");
        
        isInitialized = true;
        return true;
    }
    
    void sendUBXCommand(const String& command) {
        gnssSerial.println(command);
        delay(100); // Allow processing time
    }
};
```

### NMEA Sentence Parsing
```cpp
struct GNSSFix {
    bool valid;
    double latitude;
    double longitude;
    float altitude;
    float hdop;        // Horizontal dilution of precision
    int satellites;    // Satellites in use
    uint32_t timestamp; // GPS time
    float speed;       // Speed over ground
    float course;      // Course over ground
};

class NMEAParser {
private:
    GNSSFix currentFix;
    
public:
    bool parseGGA(const String& sentence) {
        // $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
        
        if (!sentence.startsWith("$G") || sentence.indexOf("GGA") == -1) {
            return false;
        }
        
        int commaPositions[15];
        int commaCount = findCommas(sentence, commaPositions, 15);
        
        if (commaCount < 14) return false;
        
        // Quality indicator
        String quality = extractField(sentence, commaPositions, 5);
        if (quality.toInt() == 0) {
            currentFix.valid = false;
            return false;
        }
        
        // Parse coordinates
        String latStr = extractField(sentence, commaPositions, 1);
        String latDir = extractField(sentence, commaPositions, 2);
        String lonStr = extractField(sentence, commaPositions, 3);
        String lonDir = extractField(sentence, commaPositions, 4);
        
        if (latStr.length() > 0 && lonStr.length() > 0) {
            currentFix.latitude = parseCoordinate(latStr, latDir);
            currentFix.longitude = parseCoordinate(lonStr, lonDir);
        }
        
        // Satellites and HDOP
        currentFix.satellites = extractField(sentence, commaPositions, 6).toInt();
        currentFix.hdop = extractField(sentence, commaPositions, 7).toFloat();
        
        // Altitude
        String altStr = extractField(sentence, commaPositions, 8);
        if (altStr.length() > 0) {
            currentFix.altitude = altStr.toFloat();
        }
        
        currentFix.valid = true;
        return true;
    }
    
    bool parseRMC(const String& sentence) {
        // $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
        
        if (!sentence.startsWith("$G") || sentence.indexOf("RMC") == -1) {
            return false;
        }
        
        int commaPositions[12];
        int commaCount = findCommas(sentence, commaPositions, 12);
        
        if (commaCount < 11) return false;
        
        // Status
        String status = extractField(sentence, commaPositions, 1);
        if (status != "A") {
            currentFix.valid = false;
            return false;
        }
        
        // Speed and course
        String speedStr = extractField(sentence, commaPositions, 6);
        String courseStr = extractField(sentence, commaPositions, 7);
        
        if (speedStr.length() > 0) {
            currentFix.speed = speedStr.toFloat() * 0.514444; // Convert knots to m/s
        }
        
        if (courseStr.length() > 0) {
            currentFix.course = courseStr.toFloat();
        }
        
        // Parse date and time
        String timeStr = extractField(sentence, commaPositions, 0);
        String dateStr = extractField(sentence, commaPositions, 8);
        
        if (timeStr.length() >= 6 && dateStr.length() >= 6) {
            currentFix.timestamp = parseDateTime(timeStr, dateStr);
        }
        
        return true;
    }
    
private:
    double parseCoordinate(const String& coord, const String& direction) {
        if (coord.length() < 4) return 0.0;
        
        // Format: DDMM.MMMM or DDDMM.MMMM
        int dotPos = coord.indexOf('.');
        if (dotPos < 3) return 0.0;
        
        String degreesStr = coord.substring(0, dotPos - 2);
        String minutesStr = coord.substring(dotPos - 2);
        
        double degrees = degreesStr.toFloat();
        double minutes = minutesStr.toFloat();
        
        double result = degrees + (minutes / 60.0);
        
        if (direction == "S" || direction == "W") {
            result = -result;
        }
        
        return result;
    }
    
    uint32_t parseDateTime(const String& timeStr, const String& dateStr) {
        // Convert GPS time to Unix timestamp
        if (timeStr.length() < 6 || dateStr.length() < 6) return 0;
        
        int hours = timeStr.substring(0, 2).toInt();
        int minutes = timeStr.substring(2, 4).toInt();
        int seconds = timeStr.substring(4, 6).toInt();
        
        int day = dateStr.substring(0, 2).toInt();
        int month = dateStr.substring(2, 4).toInt();
        int year = 2000 + dateStr.substring(4, 6).toInt();
        
        // Convert to Unix timestamp (simplified)
        struct tm timeinfo;
        timeinfo.tm_year = year - 1900;
        timeinfo.tm_mon = month - 1;
        timeinfo.tm_mday = day;
        timeinfo.tm_hour = hours;
        timeinfo.tm_min = minutes;
        timeinfo.tm_sec = seconds;
        
        return mktime(&timeinfo);
    }
    
    int findCommas(const String& sentence, int* positions, int maxCommas) {
        int count = 0;
        for (int i = 0; i < sentence.length() && count < maxCommas; i++) {
            if (sentence.charAt(i) == ',') {
                positions[count++] = i;
            }
        }
        return count;
    }
    
    String extractField(const String& sentence, int* commaPos, int fieldIndex) {
        if (fieldIndex == 0) {
            return sentence.substring(sentence.indexOf(',') + 1, commaPos[0]);
        } else if (fieldIndex < 15) {
            return sentence.substring(commaPos[fieldIndex - 1] + 1, commaPos[fieldIndex]);
        }
        return "";
    }
};
```

### PPS (Pulse Per Second) Integration
```cpp
volatile unsigned long ppsTime = 0;
volatile bool newPPS = false;

void IRAM_ATTR ppsInterrupt() {
    ppsTime = micros();
    newPPS = true;
}

void setupPPS() {
    pinMode(PPS_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PPS_PIN), ppsInterrupt, RISING);
}

void syncSystemTime() {
    if (newPPS && currentFix.valid) {
        // Synchronize system time to GPS time using PPS
        struct timeval tv;
        tv.tv_sec = currentFix.timestamp;
        tv.tv_usec = 0; // PPS provides precise second boundary
        
        settimeofday(&tv, NULL);
        
        Serial.println("System time synchronized to GPS");
        newPPS = false;
    }
}
```

## Advanced GNSS Features

### RTK (Real-Time Kinematic) Integration
```cpp
class RTKCorrection {
private:
    WiFiClient ntripClient;
    String ntripServer;
    int ntripPort;
    String mountpoint;
    String username;
    String password;
    
public:
    bool connectToNTRIP(const String& server, int port, const String& mount, 
                       const String& user, const String& pass) {
        ntripServer = server;
        ntripPort = port;
        mountpoint = mount;
        username = user;
        password = pass;
        
        if (ntripClient.connect(server.c_str(), port)) {
            // Send NTRIP request
            String request = "GET /" + mountpoint + " HTTP/1.0\r\n";
            request += "User-Agent: ESP32_WildlifeCam\r\n";
            request += "Authorization: Basic " + encodeBase64(user + ":" + pass) + "\r\n";
            request += "\r\n";
            
            ntripClient.print(request);
            
            // Wait for ICY 200 OK response
            unsigned long timeout = millis() + 10000;
            while (millis() < timeout) {
                if (ntripClient.available()) {
                    String response = ntripClient.readStringUntil('\n');
                    if (response.indexOf("ICY 200 OK") != -1) {
                        Serial.println("NTRIP connection established");
                        return true;
                    }
                }
            }
        }
        
        return false;
    }
    
    void processRTKCorrections() {
        while (ntripClient.available()) {
            // Forward RTCM correction data to GNSS module
            uint8_t correctionData = ntripClient.read();
            gnssSerial.write(correctionData);
        }
    }
};
```

### Satellite Constellation Status
```cpp
struct ConstellationStatus {
    int gpsCount;
    int glonassCount;
    int galileoCount;
    int beidouCount;
    float avgCNO;        // Average carrier-to-noise ratio
    float pdop;          // Position dilution of precision
};

ConstellationStatus getConstellationStatus() {
    ConstellationStatus status = {0, 0, 0, 0, 0, 0};
    
    // Parse GSA sentences for satellite information
    // This would require collecting GSA sentences for each constellation
    
    return status;
}

void optimizeConstellationUsage() {
    ConstellationStatus status = getConstellationStatus();
    
    // Disable weak constellations to save power
    if (status.gpsCount >= 6) {
        // GPS sufficient, disable others for power saving
        sendUBXCommand("$PUBX,40,GNS,0,0,0,0*41"); // Disable GLONASS
    }
    
    // Re-enable if GPS becomes insufficient
    if (status.gpsCount < 4) {
        sendUBXCommand("$PUBX,40,GNS,0,1,0,0*40"); // Enable GLONASS
    }
}
```

## Power Management

### Adaptive Power Modes
```cpp
class GNSSPowerManager {
private:
    enum PowerMode {
        CONTINUOUS,      // Always on
        PERIODIC,        // On/off cycling
        ON_DEMAND       // Only when needed
    };
    
    PowerMode currentMode;
    unsigned long lastFixTime;
    unsigned long cycleInterval;
    
public:
    void optimizePowerConsumption() {
        float batteryLevel = getBatteryLevel();
        
        if (batteryLevel > 70) {
            currentMode = CONTINUOUS;
            cycleInterval = 0;
        } else if (batteryLevel > 30) {
            currentMode = PERIODIC;
            cycleInterval = 300000; // 5 minutes
        } else {
            currentMode = ON_DEMAND;
            enterSleepMode();
        }
    }
    
    void processPowerCycle() {
        switch (currentMode) {
            case PERIODIC:
                if (millis() - lastFixTime > cycleInterval) {
                    wakeUpGNSS();
                    waitForFix();
                    sleepGNSS();
                    lastFixTime = millis();
                }
                break;
                
            case ON_DEMAND:
                // Only wake when specifically requested
                break;
                
            case CONTINUOUS:
            default:
                // No power management
                break;
        }
    }
    
    void enterSleepMode() {
        // Put GNSS in backup mode
        sendUBXCommand("$PUBX,00*33"); // Backup mode
        digitalWrite(EN_PIN, LOW);
    }
    
    void wakeUpGNSS() {
        digitalWrite(EN_PIN, HIGH);
        delay(100);
        
        // Send any character to wake from backup mode
        gnssSerial.write(0xFF);
        delay(1000);
    }
};
```

### Hot Start Optimization
```cpp
class GNSSHotStart {
private:
    struct AlmanacData {
        uint32_t timestamp;
        uint8_t data[1024]; // Simplified almanac storage
        bool valid;
    };
    
    AlmanacData storedAlmanac;
    
public:
    void saveAlmanacData() {
        // Save current almanac for hot starts
        // In real implementation, this would extract ephemeris data
        storedAlmanac.timestamp = millis();
        storedAlmanac.valid = true;
        
        // Store in EEPROM or flash memory
        EEPROM.put(0, storedAlmanac);
        EEPROM.commit();
    }
    
    bool loadAlmanacData() {
        EEPROM.get(0, storedAlmanac);
        
        // Check if almanac is still valid (less than 4 hours old)
        if (storedAlmanac.valid && 
            (millis() - storedAlmanac.timestamp) < 4 * 3600 * 1000) {
            
            // Inject almanac data to GNSS module
            // This would require module-specific commands
            return true;
        }
        
        return false;
    }
    
    void enableHotStart() {
        if (loadAlmanacData()) {
            Serial.println("Hot start enabled - faster TTFF expected");
        } else {
            Serial.println("Cold start required - acquiring new almanac");
        }
    }
};
```

## Testing and Validation

### GNSS Performance Testing
```cpp
void testGNSSPerformance() {
    Serial.println("Starting GNSS performance test...");
    
    // Test 1: Time to First Fix
    unsigned long ttffStart = millis();
    while (!gnss.hasValidFix() && millis() - ttffStart < 300000) {
        gnss.update();
        delay(1000);
    }
    
    if (gnss.hasValidFix()) {
        unsigned long ttff = millis() - ttffStart;
        Serial.printf("TTFF: %lu seconds\n", ttff / 1000);
    } else {
        Serial.println("TTFF timeout (>5 minutes)");
        return;
    }
    
    // Test 2: Position accuracy over time
    double positions[100][2]; // lat, lon
    int posCount = 0;
    
    for (int i = 0; i < 100 && posCount < 100; i++) {
        gnss.update();
        if (gnss.hasValidFix()) {
            GNSSFix fix = gnss.getCurrentFix();
            positions[posCount][0] = fix.latitude;
            positions[posCount][1] = fix.longitude;
            posCount++;
        }
        delay(10000); // Sample every 10 seconds
    }
    
    // Calculate position accuracy
    double meanLat = 0, meanLon = 0;
    for (int i = 0; i < posCount; i++) {
        meanLat += positions[i][0];
        meanLon += positions[i][1];
    }
    meanLat /= posCount;
    meanLon /= posCount;
    
    double maxError = 0;
    for (int i = 0; i < posCount; i++) {
        double distance = calculateDistance(meanLat, meanLon, 
                                          positions[i][0], positions[i][1]);
        if (distance > maxError) {
            maxError = distance;
        }
    }
    
    Serial.printf("Position accuracy: %.1fm (max error from mean)\n", maxError);
    
    // Test 3: Constellation utilization
    ConstellationStatus status = getConstellationStatus();
    Serial.printf("Satellites: GPS=%d, GLONASS=%d, Galileo=%d, BeiDou=%d\n",
                  status.gpsCount, status.glonassCount, 
                  status.galileoCount, status.beidouCount);
}
```

### Signal Quality Assessment
```cpp
void assessSignalQuality() {
    struct SatelliteInfo {
        int prn;           // Satellite PRN
        int elevation;     // Elevation angle
        int azimuth;       // Azimuth angle
        int snr;          // Signal-to-noise ratio
    };
    
    std::vector<SatelliteInfo> satellites;
    
    // Parse GSV sentences to get satellite information
    // This would require implementing GSV parser
    
    // Analyze signal quality
    int strongSignals = 0;
    int weakSignals = 0;
    
    for (const auto& sat : satellites) {
        if (sat.snr > 40) {
            strongSignals++;
        } else if (sat.snr < 25) {
            weakSignals++;
        }
    }
    
    Serial.printf("Signal quality: %d strong, %d weak signals\n", 
                  strongSignals, weakSignals);
    
    if (weakSignals > strongSignals) {
        Serial.println("Consider antenna positioning or active antenna");
    }
}
```

## Integration with Communication Systems

### Satellite Communication Timing
```cpp
class SatelliteTimingOptimizer {
public:
    bool isOptimalTransmissionTime() {
        GNSSFix fix = gnss.getCurrentFix();
        
        if (!fix.valid) return false;
        
        // Predict satellite passes based on location and time
        double latitude = fix.latitude;
        double longitude = fix.longitude;
        uint32_t currentTime = fix.timestamp;
        
        // Simplified prediction - real implementation would use orbital mechanics
        return predictSatelliteVisibility(latitude, longitude, currentTime);
    }
    
private:
    bool predictSatelliteVisibility(double lat, double lon, uint32_t time) {
        // This would implement actual satellite pass prediction
        // For demonstration, use simple time-based logic
        
        int hourOfDay = (time % 86400) / 3600;
        
        // Iridium satellites are more visible during certain hours
        return (hourOfDay >= 6 && hourOfDay <= 18); // Daytime hours
    }
};
```

### Location-Based Services
```cpp
class LocationServices {
public:
    bool isWithinGeofence(double centerLat, double centerLon, double radiusMeters) {
        GNSSFix fix = gnss.getCurrentFix();
        
        if (!fix.valid) return false;
        
        double distance = calculateDistance(fix.latitude, fix.longitude,
                                          centerLat, centerLon);
        
        return distance <= radiusMeters;
    }
    
    bool hasMovedSignificantly(double thresholdMeters = 10.0) {
        static bool hasLastPosition = false;
        static double lastLat = 0, lastLon = 0;
        
        GNSSFix fix = gnss.getCurrentFix();
        
        if (!fix.valid) return false;
        
        if (!hasLastPosition) {
            lastLat = fix.latitude;
            lastLon = fix.longitude;
            hasLastPosition = true;
            return false;
        }
        
        double distance = calculateDistance(fix.latitude, fix.longitude,
                                          lastLat, lastLon);
        
        if (distance > thresholdMeters) {
            lastLat = fix.latitude;
            lastLon = fix.longitude;
            return true;
        }
        
        return false;
    }
    
    String getLocationString(int precision = 6) {
        GNSSFix fix = gnss.getCurrentFix();
        
        if (!fix.valid) return "No GPS fix";
        
        return String(fix.latitude, precision) + "," + 
               String(fix.longitude, precision) + "," +
               String(fix.altitude, 1);
    }
};
```

This comprehensive GNSS integration guide provides accurate positioning and timing services essential for coordinating satellite communications and providing location context for wildlife monitoring data.