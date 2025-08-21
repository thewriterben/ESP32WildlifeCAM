# API Reference Documentation

## ESP32 Wildlife Camera - Code Documentation

### Overview

This document provides comprehensive API documentation for the ESP32 Wildlife Camera firmware. The system is built using a modular architecture with separate namespaces for each major subsystem.

### Architecture Overview

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Main Loop     │    │  Camera Handler │    │  Motion Filter  │
│   (main.cpp)    │───▶│  (camera_*)     │───▶│  (motion_*)     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  Solar Manager  │    │   LoRa Mesh     │    │   Configuration │
│  (solar_*)      │    │   (lora_*)      │    │   (config.h)    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## Core Configuration (config.h)

### System Constants

#### Camera Configuration
```cpp
// Camera model selection
#define CAMERA_MODEL_AI_THINKER    // AI-Thinker ESP32-CAM board

// Image quality settings
#define CAMERA_FRAME_SIZE FRAMESIZE_UXGA  // 1600x1200 pixels
#define CAMERA_JPEG_QUALITY 12            // 10-63, lower = higher quality
#define CAMERA_FB_COUNT 2                 // Frame buffer count
```

#### Motion Detection
```cpp
// PIR sensor configuration
#define PIR_PIN 13                        // GPIO pin for PIR sensor
#define PIR_DEBOUNCE_TIME 2000           // Milliseconds between triggers
#define MOTION_DETECTION_ENABLED true    // Enable motion detection
#define MOTION_SENSITIVITY 50            // 0-100 sensitivity level
```

#### Power Management
```cpp
// Voltage monitoring
#define SOLAR_VOLTAGE_PIN 34             // ADC pin for solar voltage
#define BATTERY_VOLTAGE_PIN 35           // ADC pin for battery voltage
#define SOLAR_VOLTAGE_THRESHOLD 3.2      // Minimum solar charging voltage
#define BATTERY_LOW_THRESHOLD 3.0        // Low battery warning level
#define BATTERY_CRITICAL_THRESHOLD 2.8   // Critical battery shutdown
```

#### LoRa Network
```cpp
// Radio configuration
#define LORA_FREQUENCY 915E6             // Frequency in Hz
#define LORA_TX_POWER 20                // dBm (5-20)
#define LORA_SPREADING_FACTOR 7          // 6-12
#define LORA_SIGNAL_BANDWIDTH 125E3     // Signal bandwidth
#define NODE_ID 1                       // Unique node identifier
```

### Utility Macros
```cpp
// Debug output macros
#define DEBUG_PRINT(x) if(DEBUG_ENABLED) Serial.print(x)
#define DEBUG_PRINTLN(x) if(DEBUG_ENABLED) Serial.println(x)
#define DEBUG_PRINTF(format, ...) if(DEBUG_ENABLED) Serial.printf(format, ##__VA_ARGS__)

// Conversion utilities
#define VOLTAGE_TO_ADC(voltage) ((voltage) * 4095 / 3.3)
#define ADC_TO_VOLTAGE(adc) ((adc) * 3.3 / 4095)
#define PERCENTAGE(value, max_value) ((value) * 100 / (max_value))
```

## Camera Handler Module

### Namespace: CameraHandler

#### Data Structures

```cpp
enum LightingCondition {
    LIGHTING_BRIGHT = 0,    // Bright sunlight conditions
    LIGHTING_NORMAL = 1,    // Normal daylight
    LIGHTING_DIM = 2,       // Overcast or shade
    LIGHTING_DARK = 3       // Dawn/dusk conditions
};

struct CameraStatus {
    bool initialized;        // Camera initialization status
    bool sensorDetected;     // Camera sensor detection
    int imageCount;         // Total images captured
    esp_err_t lastError;    // Last error code
};
```

#### Core Functions

```cpp
bool init();
```
**Description**: Initialize camera with optimal settings for wildlife photography  
**Returns**: `true` if initialization successful, `false` otherwise  
**Usage**:
```cpp
if (CameraHandler::init()) {
    DEBUG_PRINTLN("Camera ready");
} else {
    DEBUG_PRINTLN("Camera initialization failed");
}
```

```cpp
camera_fb_t* captureImage();
```
**Description**: Capture a single image from camera  
**Returns**: Pointer to camera frame buffer, or `nullptr` on failure  
**Usage**:
```cpp
camera_fb_t* fb = CameraHandler::captureImage();
if (fb) {
    // Process image
    esp_camera_fb_return(fb);  // Always return buffer
}
```

```cpp
String saveImage(camera_fb_t* fb, const char* folder);
```
**Description**: Save image to SD card with timestamp and metadata  
**Parameters**:
- `fb`: Camera frame buffer containing image data
- `folder`: Target folder path on SD card

**Returns**: Filename of saved image, or empty string on failure  
**Usage**:
```cpp
String filename = CameraHandler::saveImage(fb, "/images");
if (filename.length() > 0) {
    DEBUG_PRINTF("Image saved: %s\n", filename.c_str());
}
```

```cpp
void adjustForLighting(LightingCondition condition);
```
**Description**: Adjust camera settings for different lighting conditions  
**Parameters**:
- `condition`: Lighting condition enumeration

**Usage**:
```cpp
// Automatically adjust for current lighting
CameraHandler::adjustForLighting(LIGHTING_DIM);
```

```cpp
CameraStatus getStatus();
```
**Description**: Get comprehensive camera status information  
**Returns**: CameraStatus structure with current state  
**Usage**:
```cpp
CameraStatus status = CameraHandler::getStatus();
DEBUG_PRINTF("Images captured: %d\n", status.imageCount);
```

## Motion Filter Module

### Namespace: MotionFilter

#### Data Structures

```cpp
struct MotionStats {
    unsigned long lastMotionTime;  // Timestamp of last motion
    int consecutiveCount;          // Consecutive motion count
    float windSpeed;              // Current wind speed (km/h)
    float rainfall;               // Current rainfall (mm/h)
    float temperature;            // Current temperature (°C)
    bool filteringEnabled;        // Weather filtering status
};

struct MotionFilterStatus {
    bool initialized;             // Module initialization status
    unsigned long lastMotionTime; // Last motion detection time
    bool currentlyDetecting;      // Currently detecting motion
    bool weatherSensorActive;     // Weather sensor availability
    bool filteringActive;         // Weather filtering active
    int consecutiveMotions;       // Consecutive motion count
};
```

#### Core Functions

```cpp
bool init();
```
**Description**: Initialize motion detection system with PIR sensor and weather filtering  
**Returns**: `true` if initialization successful  
**Usage**:
```cpp
if (MotionFilter::init()) {
    DEBUG_PRINTLN("Motion detection ready");
}
```

```cpp
bool isMotionDetected();
```
**Description**: Check if motion is currently detected (debounced)  
**Returns**: `true` if motion detected, `false` otherwise  
**Usage**:
```cpp
if (MotionFilter::isMotionDetected()) {
    // Handle motion event
    handleMotionDetection();
}
```

```cpp
bool isValidMotion();
```
**Description**: Validate motion against weather conditions and filtering rules  
**Returns**: `true` if motion is valid, `false` if filtered out  
**Usage**:
```cpp
if (MotionFilter::isMotionDetected() && MotionFilter::isValidMotion()) {
    // Valid motion - capture image
    captureWildlifeImage();
}
```

```cpp
MotionStats getMotionStats();
```
**Description**: Get detailed motion detection statistics  
**Returns**: MotionStats structure with current data  
**Usage**:
```cpp
MotionStats stats = MotionFilter::getMotionStats();
DEBUG_PRINTF("Wind speed: %.1f km/h\n", stats.windSpeed);
```

```cpp
void setMotionSensitivity(int sensitivity);
```
**Description**: Adjust motion detection sensitivity  
**Parameters**:
- `sensitivity`: Sensitivity level 0-100 (0=least sensitive, 100=most sensitive)

**Usage**:
```cpp
// Reduce sensitivity for windy conditions
MotionFilter::setMotionSensitivity(30);
```

## Solar Manager Module

### Namespace: SolarManager

#### Data Structures

```cpp
enum PowerState {
    POWER_CRITICAL = 0,  // Battery critically low (<2.8V)
    POWER_LOW = 1,       // Battery low (2.8-3.0V)
    POWER_GOOD = 2,      // Battery good (3.0-3.8V)
    POWER_NORMAL = 3     // Battery normal/full (>3.8V)
};

struct PowerStats {
    float batteryVoltage;          // Current battery voltage
    float solarVoltage;            // Current solar panel voltage
    int batteryPercentage;         // Battery charge percentage
    bool isCharging;               // Charging status
    PowerState powerState;         // Current power state
    unsigned long totalChargingTime; // Total charging time (ms)
    float estimatedRuntime;        // Estimated runtime (hours)
};

struct PowerSystemStatus {
    bool initialized;              // System initialization status
    float batteryVoltage;          // Current battery voltage
    float solarVoltage;            // Current solar voltage
    int batteryPercentage;         // Battery charge percentage
    bool isCharging;               // Charging status
    PowerState powerState;         // Power state
    float estimatedRuntime;        // Runtime estimate
    unsigned long totalChargingTime; // Charging time
    bool lowPowerMode;             // Low power mode active
};
```

#### Core Functions

```cpp
bool init();
```
**Description**: Initialize solar power management system  
**Returns**: `true` if initialization successful  
**Usage**:
```cpp
if (SolarManager::init()) {
    DEBUG_PRINTLN("Power management ready");
}
```

```cpp
void update();
```
**Description**: Update power system status (call regularly in main loop)  
**Usage**:
```cpp
void loop() {
    SolarManager::update();  // Update every loop iteration
    // ... other code
}
```

```cpp
float getBatteryVoltage();
```
**Description**: Get current battery voltage  
**Returns**: Battery voltage in volts  
**Usage**:
```cpp
float voltage = SolarManager::getBatteryVoltage();
if (voltage < BATTERY_LOW_THRESHOLD) {
    enterLowPowerMode();
}
```

```cpp
int getBatteryPercentage();
```
**Description**: Get battery charge level as percentage  
**Returns**: Battery charge percentage (0-100)  
**Usage**:
```cpp
int charge = SolarManager::getBatteryPercentage();
DEBUG_PRINTF("Battery: %d%%\n", charge);
```

```cpp
bool isCharging();
```
**Description**: Check if battery is currently charging from solar panel  
**Returns**: `true` if charging, `false` otherwise  
**Usage**:
```cpp
if (SolarManager::isCharging()) {
    // Solar power available
    setHighPerformanceMode();
}
```

```cpp
PowerState getPowerState();
```
**Description**: Get current power management state  
**Returns**: PowerState enumeration value  
**Usage**:
```cpp
PowerState state = SolarManager::getPowerState();
switch (state) {
    case POWER_CRITICAL:
        enterEmergencyMode();
        break;
    case POWER_LOW:
        reducePowerConsumption();
        break;
    // ... handle other states
}
```

```cpp
bool shouldEnterLowPower();
```
**Description**: Check if system should enter low power mode  
**Returns**: `true` if low power mode recommended  
**Usage**:
```cpp
if (SolarManager::shouldEnterLowPower()) {
    enterDeepSleep();
}
```

## LoRa Mesh Module

### Namespace: LoraMesh

#### Data Structures

```cpp
enum SignalStrength {
    SIGNAL_POOR = 0,        // RSSI < -100 dBm
    SIGNAL_FAIR = 1,        // RSSI -100 to -85 dBm
    SIGNAL_GOOD = 2,        // RSSI -85 to -70 dBm
    SIGNAL_EXCELLENT = 3    // RSSI > -70 dBm
};

struct SignalQuality {
    int rssi;               // Received Signal Strength Indicator
    float snr;              // Signal-to-Noise Ratio
    SignalStrength strength; // Categorized signal strength
};

struct MeshNetworkStatus {
    bool initialized;           // Network initialization status
    int nodeId;                // This node's ID
    int connectedNodes;        // Number of connected nodes
    unsigned long lastMessageTime; // Last message timestamp
    int rssi;                  // Last received RSSI
    float snr;                 // Last received SNR
    unsigned long packetsReceived; // Total packets received
    unsigned long packetsSent;     // Total packets sent
};
```

#### Core Functions

```cpp
bool init();
```
**Description**: Initialize LoRa mesh networking  
**Returns**: `true` if initialization successful  
**Usage**:
```cpp
if (LoraMesh::init()) {
    DEBUG_PRINTLN("LoRa mesh network ready");
}
```

```cpp
void processMessages();
```
**Description**: Process incoming messages and handle mesh routing (call in main loop)  
**Usage**:
```cpp
void loop() {
    LoraMesh::processMessages();  // Handle network traffic
    // ... other code
}
```

```cpp
bool transmitImage(camera_fb_t* fb, const String& filename);
```
**Description**: Transmit image metadata over LoRa mesh network  
**Parameters**:
- `fb`: Camera frame buffer containing image data
- `filename`: Name of the image file

**Returns**: `true` if transmission initiated successfully  
**Usage**:
```cpp
camera_fb_t* fb = CameraHandler::captureImage();
String filename = CameraHandler::saveImage(fb, "/images");
LoraMesh::transmitImage(fb, filename);
```

```cpp
bool sendStatus(const JsonObject& statusData);
```
**Description**: Send status update to mesh network  
**Parameters**:
- `statusData`: JSON object containing status information

**Returns**: `true` if message queued successfully  
**Usage**:
```cpp
DynamicJsonDocument doc(256);
doc["battery"] = SolarManager::getBatteryPercentage();
doc["temperature"] = 23.5;
LoraMesh::sendStatus(doc.as<JsonObject>());
```

```cpp
MeshNetworkStatus getNetworkStatus();
```
**Description**: Get comprehensive network status  
**Returns**: MeshNetworkStatus structure  
**Usage**:
```cpp
MeshNetworkStatus status = LoraMesh::getNetworkStatus();
DEBUG_PRINTF("Connected nodes: %d\n", status.connectedNodes);
```

```cpp
SignalQuality getSignalQuality();
```
**Description**: Get signal quality information for last received message  
**Returns**: SignalQuality structure  
**Usage**:
```cpp
SignalQuality quality = LoraMesh::getSignalQuality();
DEBUG_PRINTF("RSSI: %d dBm, SNR: %.1f\n", quality.rssi, quality.snr);
```

## Main Application Functions

### System Initialization
```cpp
void initializeSystem();
```
**Description**: Initialize all system components in correct order  
**Called from**: `setup()`

### Motion Handling
```cpp
void handleMotionDetection();
```
**Description**: Handle detected motion event - capture and store image  
**Called from**: `loop()` when motion detected

### Power Management
```cpp
void enterDeepSleep();
void handleLowPower();
```
**Description**: Power management functions for battery conservation  
**Called from**: `loop()` based on battery status

### Status Logging
```cpp
void logSystemStatus();
```
**Description**: Log comprehensive system status to serial and storage  
**Called from**: `loop()` periodically

## Error Handling

### Common Error Codes
```cpp
// Camera errors
ESP_ERR_CAMERA_NOT_DETECTED    // Camera module not found
ESP_ERR_CAMERA_FAILED_TO_INIT  // Camera initialization failed
ESP_ERR_CAMERA_CAPTURE_FAILED  // Image capture failed

// Storage errors
ESP_ERR_NOT_FOUND              // SD card not found
ESP_ERR_NO_MEM                 // Insufficient memory

// Network errors
ESP_ERR_TIMEOUT                // LoRa transmission timeout
ESP_ERR_INVALID_RESPONSE       // Invalid network response
```

### Error Recovery Strategies
```cpp
// Automatic retry with exponential backoff
int retryCount = 0;
while (retryCount < 3) {
    if (operation_success()) break;
    delay(1000 << retryCount);  // 1s, 2s, 4s delays
    retryCount++;
}

// Graceful degradation
if (!cameraInitialized) {
    // Continue operation without camera
    enableMotionLoggingOnly();
}
```

## Performance Optimization

### Memory Management
```cpp
// Always return camera frame buffers
camera_fb_t* fb = esp_camera_fb_get();
// ... process image
esp_camera_fb_return(fb);  // Critical: always return

// Use local variables to minimize heap usage
void processImage() {
    uint8_t buffer[1024];  // Stack allocation
    // ... process
}  // Automatic cleanup
```

### Power Optimization
```cpp
// Use deep sleep for power conservation
esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);  // PIR wake-up
esp_sleep_enable_timer_wakeup(300 * 1000000ULL);  // 5-minute backup
esp_deep_sleep_start();

// Reduce CPU frequency when possible
setCpuFrequencyMhz(80);  // Reduce from 240MHz
```

### Network Optimization
```cpp
// Compress data before transmission
if (IMAGE_COMPRESSION_ENABLED) {
    compressImageData(fb);
}

// Use acknowledgments for critical data
if (sendCriticalData(data)) {
    waitForAcknowledgment(5000);  // 5-second timeout
}
```

## Debugging and Diagnostics

### Debug Output Levels
```cpp
#define LOG_LEVEL_ERROR 0    // Critical errors only
#define LOG_LEVEL_WARN 1     // Warnings and errors
#define LOG_LEVEL_INFO 2     // General information
#define LOG_LEVEL_DEBUG 3    // Detailed debugging
```

### System Health Monitoring
```cpp
// Monitor key system metrics
struct SystemHealth {
    uint32_t freeHeap;           // Available memory
    uint32_t uptime;             // System uptime
    float cpuTemperature;        // ESP32 internal temperature
    int wifiSignal;              // WiFi signal strength (if used)
    bool allSystemsOk;           // Overall system status
};
```

### Performance Profiling
```cpp
// Measure function execution time
unsigned long startTime = millis();
performOperation();
unsigned long execTime = millis() - startTime;
DEBUG_PRINTF("Operation took %lu ms\n", execTime);
```

---

**Note**: This API documentation reflects the current firmware implementation. Always refer to the source code for the most up-to-date function signatures and behavior.