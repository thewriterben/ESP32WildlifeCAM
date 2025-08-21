# Solar Power Test Example

## ESP32 Solar Charging System Validation

This example tests and validates the solar power management system independently from the main wildlife camera firmware. Use this to verify charging circuits, battery monitoring, and power optimization.

### Hardware Required

- ESP32-S3-DevKitC-1 or compatible board
- TP4056 Li-ion charging module
- 18650 Li-ion battery (3000mAh+ recommended)
- Solar panel (5V 2W minimum)
- 1N5819 Schottky diode
- Voltage divider resistors (2x 10kΩ)
- Multimeter for verification
- Breadboard and jumper wires

### Circuit Diagram

```
Solar Panel (+) ──── 1N5819 Diode ──── TP4056 VIN
                                        │
Solar Panel (-) ────────────────────── TP4056 GND
                                        │
                    TP4056 BAT+ ──── 18650 (+) ──── 10kΩ ──── GPIO35 (Battery Monitor)
                                        │                      │
                    TP4056 BAT- ──── 18650 (-) ──── 10kΩ ──── GND
                                        │          
                    TP4056 OUT+ ──── ESP32 VIN
                    TP4056 OUT- ──── ESP32 GND

Solar Monitor: Solar (+) ──── 10kΩ ──── GPIO34 ──── 10kΩ ──── GND

LED Indicators:
GPIO2 ──── 220Ω ──── LED (+) ──── LED (-) ──── GND (Charging Indicator)
GPIO4 ──── 220Ω ──── LED (+) ──── LED (-) ──── GND (Status Indicator)
```

### Code Files

#### platformio.ini
```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

monitor_speed = 115200
upload_speed = 921600

build_flags = 
    -DCORE_DEBUG_LEVEL=1
    -DBOARD_HAS_PSRAM

lib_deps = 
    bblanchon/ArduinoJson@^6.21.3
```

#### src/main.cpp
```cpp
#include <Arduino.h>
#include <ArduinoJson.h>

// Pin definitions
#define BATTERY_VOLTAGE_PIN 35    // ADC pin for battery voltage monitoring
#define SOLAR_VOLTAGE_PIN 34      // ADC pin for solar voltage monitoring
#define CHARGING_LED_PIN 2        // LED to indicate charging status
#define STATUS_LED_PIN 4          // LED for general status

// Power thresholds (volts)
#define BATTERY_FULL_VOLTAGE 4.1
#define BATTERY_GOOD_VOLTAGE 3.8
#define BATTERY_LOW_VOLTAGE 3.0
#define BATTERY_CRITICAL_VOLTAGE 2.8
#define SOLAR_CHARGING_THRESHOLD 4.0

// Measurement settings
#define VOLTAGE_DIVIDER_RATIO 2.0  // 10k/10k voltage divider
#define ADC_RESOLUTION 4095        // 12-bit ADC
#define REFERENCE_VOLTAGE 3.3      // ESP32 reference voltage
#define MEASUREMENT_INTERVAL 5000  // 5 seconds between measurements

// Global variables
float batteryVoltage = 0.0;
float solarVoltage = 0.0;
bool isCharging = false;
unsigned long lastMeasurement = 0;
unsigned long testStartTime = 0;
int measurementCount = 0;

// Data logging arrays (last 100 measurements)
float batteryHistory[100];
float solarHistory[100];
bool chargingHistory[100];
unsigned long timeHistory[100];
int historyIndex = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("ESP32 Solar Power Test");
    Serial.println("=====================");
    
    // Initialize pins
    pinMode(CHARGING_LED_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    pinMode(SOLAR_VOLTAGE_PIN, INPUT);
    
    // Set ADC resolution
    analogReadResolution(12);
    
    // Initialize arrays
    for (int i = 0; i < 100; i++) {
        batteryHistory[i] = 0.0;
        solarHistory[i] = 0.0;
        chargingHistory[i] = false;
        timeHistory[i] = 0;
    }
    
    testStartTime = millis();
    
    Serial.println("Solar power monitoring started");
    Serial.println("Commands:");
    Serial.println("  's' - Show current status");
    Serial.println("  'h' - Show measurement history");
    Serial.println("  'r' - Reset statistics");
    Serial.println("  'c' - Calibrate voltage readings");
    Serial.println("  't' - Run automated test sequence");
    Serial.println("  'j' - Export data as JSON");
    Serial.println();
    
    // Initial measurement
    takeMeasurement();
    displayStatus();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Take measurements at regular intervals
    if (currentTime - lastMeasurement >= MEASUREMENT_INTERVAL) {
        takeMeasurement();
        lastMeasurement = currentTime;
    }
    
    // Update LED indicators
    updateLEDs();
    
    // Handle serial commands
    if (Serial.available() > 0) {
        handleSerialCommand();
    }
    
    delay(100);
}

void takeMeasurement() {
    // Read battery voltage
    int batteryADC = analogRead(BATTERY_VOLTAGE_PIN);
    batteryVoltage = adcToVoltage(batteryADC);
    
    // Read solar voltage
    int solarADC = analogRead(SOLAR_VOLTAGE_PIN);
    solarVoltage = adcToVoltage(solarADC);
    
    // Determine charging status
    isCharging = (solarVoltage > SOLAR_CHARGING_THRESHOLD) && 
                 (batteryVoltage < BATTERY_FULL_VOLTAGE);
    
    // Store in history
    batteryHistory[historyIndex] = batteryVoltage;
    solarHistory[historyIndex] = solarVoltage;
    chargingHistory[historyIndex] = isCharging;
    timeHistory[historyIndex] = millis() - testStartTime;
    
    historyIndex = (historyIndex + 1) % 100;
    measurementCount++;
    
    // Print measurement (every 10th measurement to reduce spam)
    if (measurementCount % 10 == 0) {
        displayMeasurement();
    }
}

float adcToVoltage(int adcValue) {
    // Convert ADC reading to actual voltage
    float voltage = (adcValue * REFERENCE_VOLTAGE / ADC_RESOLUTION) * VOLTAGE_DIVIDER_RATIO;
    return voltage;
}

void displayMeasurement() {
    Serial.printf("[%8lu] Battery: %5.2fV | Solar: %5.2fV | %s | %s\n",
                  (millis() - testStartTime) / 1000,
                  batteryVoltage,
                  solarVoltage,
                  isCharging ? "CHARGING" : "--------",
                  getBatteryStatusString().c_str());
}

void displayStatus() {
    Serial.println("\n=== CURRENT STATUS ===");
    Serial.printf("Test Runtime: %lu seconds\n", (millis() - testStartTime) / 1000);
    Serial.printf("Measurements: %d\n", measurementCount);
    Serial.println();
    
    Serial.printf("Battery Voltage: %.3f V\n", batteryVoltage);
    Serial.printf("Battery Status:  %s\n", getBatteryStatusString().c_str());
    Serial.printf("Battery Level:   %d%%\n", getBatteryPercentage());
    Serial.println();
    
    Serial.printf("Solar Voltage:   %.3f V\n", solarVoltage);
    Serial.printf("Solar Status:    %s\n", getSolarStatusString().c_str());
    Serial.println();
    
    Serial.printf("Charging:        %s\n", isCharging ? "YES" : "NO");
    Serial.printf("Power State:     %s\n", getPowerStateString().c_str());
    Serial.println();
    
    // Calculate charging statistics
    calculateStatistics();
    
    Serial.println("====================\n");
}

String getBatteryStatusString() {
    if (batteryVoltage >= BATTERY_FULL_VOLTAGE) return "FULL";
    else if (batteryVoltage >= BATTERY_GOOD_VOLTAGE) return "GOOD";
    else if (batteryVoltage >= BATTERY_LOW_VOLTAGE) return "LOW";
    else if (batteryVoltage >= BATTERY_CRITICAL_VOLTAGE) return "CRITICAL";
    else return "DEAD";
}

String getSolarStatusString() {
    if (solarVoltage >= 5.0) return "EXCELLENT";
    else if (solarVoltage >= 4.5) return "GOOD";
    else if (solarVoltage >= 3.0) return "POOR";
    else return "INSUFFICIENT";
}

String getPowerStateString() {
    if (isCharging && batteryVoltage >= BATTERY_GOOD_VOLTAGE) return "OPTIMAL";
    else if (isCharging) return "RECOVERING";
    else if (batteryVoltage >= BATTERY_GOOD_VOLTAGE) return "STABLE";
    else if (batteryVoltage >= BATTERY_LOW_VOLTAGE) return "DECLINING";
    else return "EMERGENCY";
}

int getBatteryPercentage() {
    // Li-ion battery voltage curve approximation
    const float minVoltage = 2.8;
    const float maxVoltage = 4.2;
    
    if (batteryVoltage <= minVoltage) return 0;
    if (batteryVoltage >= maxVoltage) return 100;
    
    return (int)((batteryVoltage - minVoltage) / (maxVoltage - minVoltage) * 100);
}

void calculateStatistics() {
    int chargingCount = 0;
    float avgBatteryVoltage = 0;
    float avgSolarVoltage = 0;
    float minBatteryVoltage = 999;
    float maxBatteryVoltage = 0;
    float minSolarVoltage = 999;
    float maxSolarVoltage = 0;
    
    int sampleCount = min(measurementCount, 100);
    
    for (int i = 0; i < sampleCount; i++) {
        if (chargingHistory[i]) chargingCount++;
        
        avgBatteryVoltage += batteryHistory[i];
        avgSolarVoltage += solarHistory[i];
        
        if (batteryHistory[i] < minBatteryVoltage) minBatteryVoltage = batteryHistory[i];
        if (batteryHistory[i] > maxBatteryVoltage) maxBatteryVoltage = batteryHistory[i];
        if (solarHistory[i] < minSolarVoltage) minSolarVoltage = solarHistory[i];
        if (solarHistory[i] > maxSolarVoltage) maxSolarVoltage = solarHistory[i];
    }
    
    if (sampleCount > 0) {
        avgBatteryVoltage /= sampleCount;
        avgSolarVoltage /= sampleCount;
        
        float chargingPercentage = (float)chargingCount / sampleCount * 100;
        
        Serial.println("=== STATISTICS (last 100 measurements) ===");
        Serial.printf("Charging Time: %.1f%%\n", chargingPercentage);
        Serial.printf("Battery: Avg %.2fV | Min %.2fV | Max %.2fV\n", 
                      avgBatteryVoltage, minBatteryVoltage, maxBatteryVoltage);
        Serial.printf("Solar:   Avg %.2fV | Min %.2fV | Max %.2fV\n", 
                      avgSolarVoltage, minSolarVoltage, maxSolarVoltage);
    }
}

void updateLEDs() {
    // Charging LED: solid when charging, off when not
    digitalWrite(CHARGING_LED_PIN, isCharging ? HIGH : LOW);
    
    // Status LED: blink pattern indicates battery state
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    unsigned long blinkInterval;
    
    if (batteryVoltage >= BATTERY_GOOD_VOLTAGE) {
        blinkInterval = 2000;  // Slow blink - good battery
    } else if (batteryVoltage >= BATTERY_LOW_VOLTAGE) {
        blinkInterval = 1000;  // Medium blink - low battery
    } else {
        blinkInterval = 200;   // Fast blink - critical battery
    }
    
    if (millis() - lastBlink >= blinkInterval) {
        ledState = !ledState;
        digitalWrite(STATUS_LED_PIN, ledState ? HIGH : LOW);
        lastBlink = millis();
    }
}

void handleSerialCommand() {
    char command = Serial.read();
    
    switch (command) {
        case 's':
        case 'S':
            displayStatus();
            break;
            
        case 'h':
        case 'H':
            displayHistory();
            break;
            
        case 'r':
        case 'R':
            resetStatistics();
            break;
            
        case 'c':
        case 'C':
            calibrateVoltages();
            break;
            
        case 't':
        case 'T':
            runAutomatedTest();
            break;
            
        case 'j':
        case 'J':
            exportDataJSON();
            break;
            
        default:
            // Ignore unknown commands
            break;
    }
}

void displayHistory() {
    Serial.println("\n=== MEASUREMENT HISTORY ===");
    Serial.println("Time(s) | Battery(V) | Solar(V) | Charging");
    Serial.println("--------|------------|----------|----------");
    
    int sampleCount = min(measurementCount, 100);
    int startIndex = (historyIndex - sampleCount + 100) % 100;
    
    for (int i = 0; i < sampleCount; i++) {
        int index = (startIndex + i) % 100;
        Serial.printf("%7lu | %10.3f | %8.3f | %s\n",
                      timeHistory[index] / 1000,
                      batteryHistory[index],
                      solarHistory[index],
                      chargingHistory[index] ? "YES" : "NO");
    }
    Serial.println("===========================\n");
}

void resetStatistics() {
    Serial.println("Resetting statistics...");
    
    measurementCount = 0;
    historyIndex = 0;
    testStartTime = millis();
    
    for (int i = 0; i < 100; i++) {
        batteryHistory[i] = 0.0;
        solarHistory[i] = 0.0;
        chargingHistory[i] = false;
        timeHistory[i] = 0;
    }
    
    Serial.println("Statistics reset complete\n");
}

void calibrateVoltages() {
    Serial.println("\n=== VOLTAGE CALIBRATION ===");
    Serial.println("Use multimeter to measure actual voltages:");
    Serial.printf("Current battery reading: %.3f V\n", batteryVoltage);
    Serial.printf("Current solar reading:   %.3f V\n", solarVoltage);
    Serial.println();
    Serial.println("Enter actual battery voltage (e.g., 3.85): ");
    
    // In a real implementation, you would read from serial
    // and calculate calibration factors to store in EEPROM
    Serial.println("Calibration would be implemented here");
    Serial.println("Store calibration factors in non-volatile memory");
    Serial.println("===============================\n");
}

void runAutomatedTest() {
    Serial.println("\n=== AUTOMATED TEST SEQUENCE ===");
    
    // Test 1: Voltage reading stability
    Serial.println("Test 1: Voltage Reading Stability");
    float voltageSum = 0;
    float voltageSquaredSum = 0;
    int testSamples = 20;
    
    for (int i = 0; i < testSamples; i++) {
        int adc = analogRead(BATTERY_VOLTAGE_PIN);
        float voltage = adcToVoltage(adc);
        voltageSum += voltage;
        voltageSquaredSum += voltage * voltage;
        delay(100);
    }
    
    float mean = voltageSum / testSamples;
    float variance = (voltageSquaredSum / testSamples) - (mean * mean);
    float stdDev = sqrt(variance);
    
    Serial.printf("  Mean voltage: %.4f V\n", mean);
    Serial.printf("  Std deviation: %.4f V\n", stdDev);
    Serial.printf("  Result: %s\n", stdDev < 0.01 ? "PASS" : "FAIL");
    
    // Test 2: Charging detection
    Serial.println("\nTest 2: Charging Detection Logic");
    Serial.printf("  Solar voltage: %.2f V\n", solarVoltage);
    Serial.printf("  Battery voltage: %.2f V\n", batteryVoltage);
    Serial.printf("  Charging threshold: %.2f V\n", SOLAR_CHARGING_THRESHOLD);
    Serial.printf("  Should be charging: %s\n", 
                  (solarVoltage > SOLAR_CHARGING_THRESHOLD) ? "YES" : "NO");
    Serial.printf("  Actually charging: %s\n", isCharging ? "YES" : "NO");
    
    // Test 3: Power consumption measurement
    Serial.println("\nTest 3: System Power Analysis");
    Serial.printf("  ESP32 CPU frequency: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("  Free heap memory: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("  Estimated current: ~80-150 mA\n");
    
    Serial.println("===============================\n");
}

void exportDataJSON() {
    Serial.println("\n=== JSON DATA EXPORT ===");
    
    DynamicJsonDocument doc(4096);
    
    // System information
    doc["system"]["firmware"] = "Solar Test v1.0";
    doc["system"]["uptime_seconds"] = (millis() - testStartTime) / 1000;
    doc["system"]["measurement_count"] = measurementCount;
    doc["system"]["cpu_frequency_mhz"] = getCpuFrequencyMhz();
    doc["system"]["free_heap"] = ESP.getFreeHeap();
    
    // Current readings
    doc["current"]["battery_voltage"] = batteryVoltage;
    doc["current"]["solar_voltage"] = solarVoltage;
    doc["current"]["is_charging"] = isCharging;
    doc["current"]["battery_percentage"] = getBatteryPercentage();
    doc["current"]["battery_status"] = getBatteryStatusString();
    doc["current"]["solar_status"] = getSolarStatusString();
    doc["current"]["power_state"] = getPowerStateString();
    
    // Configuration
    doc["config"]["battery_low_threshold"] = BATTERY_LOW_VOLTAGE;
    doc["config"]["battery_critical_threshold"] = BATTERY_CRITICAL_VOLTAGE;
    doc["config"]["solar_charging_threshold"] = SOLAR_CHARGING_THRESHOLD;
    doc["config"]["voltage_divider_ratio"] = VOLTAGE_DIVIDER_RATIO;
    doc["config"]["measurement_interval_ms"] = MEASUREMENT_INTERVAL;
    
    // Recent history (last 10 measurements)
    JsonArray history = doc.createNestedArray("history");
    int sampleCount = min(measurementCount, 10);
    int startIndex = (historyIndex - sampleCount + 100) % 100;
    
    for (int i = 0; i < sampleCount; i++) {
        int index = (startIndex + i) % 100;
        JsonObject measurement = history.createNestedObject();
        measurement["time_ms"] = timeHistory[index];
        measurement["battery_voltage"] = batteryHistory[index];
        measurement["solar_voltage"] = solarHistory[index];
        measurement["charging"] = chargingHistory[index];
    }
    
    serializeJsonPretty(doc, Serial);
    Serial.println("\n=======================\n");
}
```

### Usage Instructions

1. **Hardware Assembly:**
   ```bash
   1. Build the solar charging circuit as shown in diagram
   2. Connect voltage dividers for monitoring
   3. Add LED indicators for visual feedback
   4. Double-check all connections with multimeter
   ```

2. **Safety Checks:**
   ```bash
   1. Verify battery polarity before connecting
   2. Check solar panel voltage in sunlight (<6V)
   3. Ensure TP4056 module is correct orientation
   4. Test circuit with multimeter before powering ESP32
   ```

3. **Software Testing:**
   ```bash
   # Upload firmware
   pio run --target upload
   
   # Open serial monitor
   pio device monitor
   
   # Expected startup output:
   ESP32 Solar Power Test
   =====================
   Solar power monitoring started
   Commands:
     's' - Show current status
     ...
   ```

4. **Testing Procedures:**

   **Basic Functionality Test:**
   ```bash
   1. Cover solar panel - should show low solar voltage
   2. Expose to sunlight - should show charging
   3. Monitor battery voltage changes over time
   4. Verify LED indicators work correctly
   ```

   **Charging Performance Test:**
   ```bash
   1. Start with partially discharged battery
   2. Place in direct sunlight
   3. Monitor charging rate over 30 minutes
   4. Document voltage increases
   ```

   **Power Consumption Analysis:**
   ```bash
   1. Measure current draw with ammeter
   2. Test different ESP32 clock speeds
   3. Verify deep sleep current (if implemented)
   4. Calculate daily power budget
   ```

### Expected Test Results

#### Typical Voltage Readings
```bash
Battery States:
- Full charge: 4.1-4.2V
- Good charge: 3.8-4.1V  
- Low charge: 3.0-3.8V
- Critical: 2.8-3.0V
- Dead: <2.8V

Solar Panel (5V 2W in direct sun):
- Excellent: >5.0V
- Good: 4.5-5.0V
- Poor: 3.0-4.5V
- Insufficient: <3.0V
```

#### Charging Performance
```bash
Typical charging rates:
- From 3.0V to 3.8V: 2-4 hours
- From 3.8V to 4.1V: 1-2 hours  
- Maintenance charging: Continuous

Current consumption:
- ESP32 active: 80-150mA
- ESP32 deep sleep: 0.01-1mA
- TP4056 quiescent: ~1mA
```

### Troubleshooting

#### No Charging Detected
```bash
1. Check solar panel voltage in direct sunlight
2. Verify diode orientation (band toward TP4056)
3. Test TP4056 module with known good battery
4. Measure voltage at TP4056 input terminals
```

#### Inaccurate Voltage Readings
```bash
1. Verify voltage divider resistor values
2. Check ADC reference voltage (should be 3.3V)
3. Calibrate with known accurate voltmeter
4. Consider ADC non-linearity at extremes
```

#### Battery Not Holding Charge
```bash
1. Test battery capacity with external charger
2. Check for parasitic current draw
3. Verify battery protection circuit function
4. Replace battery if capacity <80% of rated
```

### Data Analysis

#### Performance Metrics
```bash
# Calculate charging efficiency
charging_efficiency = energy_stored / solar_energy_available

# Determine autonomy period
autonomy_hours = battery_capacity_wh / average_power_consumption_w

# Assess solar sizing
required_panel_w = daily_energy_wh / (peak_sun_hours * 0.7)
```

#### Optimization Recommendations
```bash
Based on test results:
1. Battery capacity adequate? (3+ days autonomy)
2. Solar panel sufficient? (charges during shortest day)
3. Power consumption optimized? (<100mA average)
4. Charging circuit efficient? (>80% efficiency)
```

### Next Steps

1. **Integration Testing:**
   - Combine with camera module
   - Test under varying weather conditions
   - Validate long-term reliability

2. **Optimization:**
   - Implement MPPT for better efficiency
   - Add temperature compensation
   - Optimize charging algorithm

3. **Monitoring:**
   - Add data logging to SD card
   - Implement remote monitoring
   - Create performance dashboards

---

**This test validates the foundation of the wildlife camera's power system, ensuring reliable operation in remote locations.**