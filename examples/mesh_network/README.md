# LoRa Mesh Network Example

## ESP32 LoRa Communication Test and Mesh Networking

This example demonstrates LoRa wireless communication between multiple ESP32 devices, establishing a mesh network for remote wildlife camera data transmission.

### Hardware Required (Per Node)

- ESP32-S3-DevKitC-1 or compatible board
- SX1276/SX1262 LoRa module (915MHz for North America)
- LoRa antenna (915MHz, 3-5 dBi gain)
- U.FL to SMA pigtail cable
- Breadboard and jumper wires
- Optional: OLED display for status

### Frequency Selection by Region

```bash
North America: 915MHz (ISM band)
Europe: 868MHz (ISM band)
Asia: 433MHz (ISM band)
Australia: 915MHz (ISM band)
```

### Circuit Diagram

```
ESP32-S3-DevKitC-1    SX1276 LoRa Module
==================    ==================
3.3V             ──── VCC
GND              ──── GND
GPIO 18          ──── SCK (SPI Clock)
GPIO 19          ──── MISO (SPI MISO)
GPIO 23          ──── MOSI (SPI MOSI)
GPIO 5           ──── NSS/CS (Chip Select)
GPIO 14          ──── RESET
GPIO 26          ──── DIO0 (Interrupt)

Antenna Connection:
SX1276 ANT ──── U.FL connector ──── SMA pigtail ──── 915MHz Antenna

Optional Status Display:
GPIO 21 ──── SDA (I2C Data) ──── OLED Display
GPIO 22 ──── SCL (I2C Clock) ──── OLED Display
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

lib_deps = 
    sandeepmistry/LoRa@^0.8.0
    bblanchon/ArduinoJson@^6.21.3
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.5

build_flags = 
    -DCORE_DEBUG_LEVEL=1
    -DBOARD_HAS_PSRAM
```

#### src/main.cpp
```cpp
#include <Arduino.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Wire.h>

// Optional display support
#ifdef USE_DISPLAY
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

// LoRa pin definitions
#define LORA_SCK 18
#define LORA_MISO 19
#define LORA_MOSI 23
#define LORA_CS 5
#define LORA_RST 14
#define LORA_DIO0 26

// Network configuration
#define LORA_FREQUENCY 915E6      // Adjust for your region
#define LORA_TX_POWER 20          // dBm (5-20)
#define LORA_SPREADING_FACTOR 7   // 6-12 (higher = longer range, slower)
#define LORA_SIGNAL_BANDWIDTH 125E3  // Hz
#define LORA_CODING_RATE 5        // 5-8
#define LORA_PREAMBLE_LENGTH 8    // 6-65535
#define LORA_SYNC_WORD 0x12       // Network identifier

// Node configuration (change for each device)
#define NODE_ID 1                 // Unique node ID (1, 2, 3, etc.)
#define MAX_NODES 10              // Maximum nodes in network
#define HEARTBEAT_INTERVAL 30000  // 30 seconds
#define MESSAGE_TIMEOUT 5000      // 5 seconds

// Message types
#define MSG_HEARTBEAT 1
#define MSG_DATA 2
#define MSG_IMAGE_NOTIFY 3
#define MSG_ROUTE_REQUEST 4
#define MSG_ROUTE_REPLY 5
#define MSG_ACK 6

// Global variables
struct Node {
    int id;
    unsigned long lastSeen;
    int rssi;
    float snr;
    int hopCount;
    bool isActive;
};

Node networkNodes[MAX_NODES];
int nodeCount = 0;
unsigned long lastHeartbeat = 0;
unsigned long packetCounter = 0;
int lastRSSI = 0;
float lastSNR = 0.0;

// Message queue
struct QueuedMessage {
    String payload;
    int targetNode;
    unsigned long timestamp;
    int retryCount;
};

QueuedMessage messageQueue[10];
int queueSize = 0;

// Statistics
unsigned long packetsReceived = 0;
unsigned long packetsSent = 0;
unsigned long packetsLost = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("ESP32 LoRa Mesh Network Test");
    Serial.println("============================");
    Serial.printf("Node ID: %d\n", NODE_ID);
    Serial.printf("Frequency: %.0f MHz\n", LORA_FREQUENCY / 1E6);
    
    // Initialize LoRa
    if (initLoRa()) {
        Serial.println("✓ LoRa initialized successfully");
    } else {
        Serial.println("✗ LoRa initialization failed");
        while (1) delay(1000);
    }
    
    #ifdef USE_DISPLAY
    // Initialize display
    if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("✓ OLED display initialized");
        updateDisplay();
    }
    #endif
    
    // Initialize network nodes array
    for (int i = 0; i < MAX_NODES; i++) {
        networkNodes[i].id = -1;
        networkNodes[i].isActive = false;
    }
    
    Serial.println("\nCommands:");
    Serial.println("  's' - Show network status");
    Serial.println("  'n' - Show neighbor nodes");
    Serial.println("  'p' - Send ping to all nodes");
    Serial.println("  't' - Send test message");
    Serial.println("  'r' - Reset network statistics");
    Serial.println("  'q' - Show message queue");
    Serial.println("  'd' - Toggle debug mode");
    Serial.println();
    
    // Send initial heartbeat
    sendHeartbeat();
    
    Serial.println("LoRa mesh network ready!");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Send periodic heartbeat
    if (currentTime - lastHeartbeat > HEARTBEAT_INTERVAL) {
        sendHeartbeat();
        lastHeartbeat = currentTime;
    }
    
    // Process incoming messages
    if (LoRa.parsePacket()) {
        handleReceivedMessage();
    }
    
    // Process message queue
    processMessageQueue();
    
    // Clean up stale nodes
    cleanupStaleNodes();
    
    // Handle serial commands
    if (Serial.available() > 0) {
        handleSerialCommand();
    }
    
    #ifdef USE_DISPLAY
    // Update display every 5 seconds
    static unsigned long lastDisplayUpdate = 0;
    if (currentTime - lastDisplayUpdate > 5000) {
        updateDisplay();
        lastDisplayUpdate = currentTime;
    }
    #endif
    
    delay(100);
}

bool initLoRa() {
    // Set LoRa pins
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
    
    // Initialize LoRa
    if (!LoRa.begin(LORA_FREQUENCY)) {
        return false;
    }
    
    // Configure LoRa parameters
    LoRa.setTxPower(LORA_TX_POWER);
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setSignalBandwidth(LORA_SIGNAL_BANDWIDTH);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    LoRa.setPreambleLength(LORA_PREAMBLE_LENGTH);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    
    Serial.println("LoRa Configuration:");
    Serial.printf("  Frequency: %.1f MHz\n", LORA_FREQUENCY / 1E6);
    Serial.printf("  TX Power: %d dBm\n", LORA_TX_POWER);
    Serial.printf("  Spreading Factor: %d\n", LORA_SPREADING_FACTOR);
    Serial.printf("  Bandwidth: %.1f kHz\n", LORA_SIGNAL_BANDWIDTH / 1000);
    Serial.printf("  Coding Rate: 4/%d\n", LORA_CODING_RATE);
    Serial.printf("  Sync Word: 0x%02X\n", LORA_SYNC_WORD);
    
    return true;
}

void sendMessage(int msgType, JsonObject& data, int targetNode = 0) {
    DynamicJsonDocument doc(512);
    
    doc["type"] = msgType;
    doc["source"] = NODE_ID;
    doc["target"] = targetNode;
    doc["timestamp"] = millis();
    doc["packet_id"] = packetCounter++;
    doc["hop_count"] = 0;
    
    // Copy data payload
    for (JsonPair kv : data) {
        doc[kv.key()] = kv.value();
    }
    
    String message;
    serializeJson(doc, message);
    
    // Send immediately or queue if busy
    if (LoRa.beginPacket()) {
        LoRa.print(message);
        if (LoRa.endPacket()) {
            packetsSent++;
            Serial.printf("Sent: %s\n", message.c_str());
        } else {
            Serial.println("✗ Failed to send packet");
            packetsLost++;
        }
    } else {
        // Queue for later transmission
        queueMessage(message, targetNode);
    }
}

void sendHeartbeat() {
    DynamicJsonDocument data(256);
    data["node_id"] = NODE_ID;
    data["uptime"] = millis();
    data["free_heap"] = ESP.getFreeHeap();
    data["rssi_last"] = lastRSSI;
    data["snr_last"] = lastSNR;
    data["packets_rx"] = packetsReceived;
    data["packets_tx"] = packetsSent;
    
    sendMessage(MSG_HEARTBEAT, data.as<JsonObject>());
}

void handleReceivedMessage() {
    String message = "";
    while (LoRa.available()) {
        message += (char)LoRa.read();
    }
    
    // Get signal quality
    lastRSSI = LoRa.packetRssi();
    lastSNR = LoRa.packetSnr();
    packetsReceived++;
    
    Serial.printf("Received (RSSI: %d, SNR: %.1f): %s\n", lastRSSI, lastSNR, message.c_str());
    
    // Parse JSON message
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.printf("JSON parse error: %s\n", error.c_str());
        return;
    }
    
    int msgType = doc["type"];
    int sourceNode = doc["source"];
    int targetNode = doc["target"];
    int hopCount = doc["hop_count"];
    
    // Update node information
    updateNodeInfo(sourceNode, lastRSSI, lastSNR, hopCount);
    
    // Handle different message types
    switch (msgType) {
        case MSG_HEARTBEAT:
            handleHeartbeat(doc.as<JsonObject>());
            break;
            
        case MSG_DATA:
            handleDataMessage(doc.as<JsonObject>());
            break;
            
        case MSG_IMAGE_NOTIFY:
            handleImageNotification(doc.as<JsonObject>());
            break;
            
        case MSG_PING:
            handlePing(doc.as<JsonObject>());
            break;
            
        default:
            Serial.printf("Unknown message type: %d\n", msgType);
            break;
    }
    
    // Forward message if not for us
    if (targetNode != 0 && targetNode != NODE_ID) {
        forwardMessage(doc.as<JsonObject>());
    }
}

void updateNodeInfo(int nodeId, int rssi, float snr, int hopCount) {
    // Find existing node or create new entry
    int nodeIndex = -1;
    for (int i = 0; i < nodeCount; i++) {
        if (networkNodes[i].id == nodeId) {
            nodeIndex = i;
            break;
        }
    }
    
    if (nodeIndex == -1 && nodeCount < MAX_NODES) {
        // Add new node
        nodeIndex = nodeCount++;
        networkNodes[nodeIndex].id = nodeId;
        Serial.printf("New node discovered: %d\n", nodeId);
    }
    
    if (nodeIndex != -1) {
        networkNodes[nodeIndex].lastSeen = millis();
        networkNodes[nodeIndex].rssi = rssi;
        networkNodes[nodeIndex].snr = snr;
        networkNodes[nodeIndex].hopCount = hopCount + 1;
        networkNodes[nodeIndex].isActive = true;
    }
}

void handleHeartbeat(JsonObject& msg) {
    int sourceNode = msg["node_id"];
    int uptime = msg["uptime"];
    int freeHeap = msg["free_heap"];
    
    Serial.printf("Heartbeat from node %d (uptime: %ds, heap: %d)\n", 
                  sourceNode, uptime / 1000, freeHeap);
}

void handleDataMessage(JsonObject& msg) {
    String dataType = msg["data_type"];
    Serial.printf("Data message: %s\n", dataType.c_str());
    
    // Process different data types
    if (dataType == "sensor") {
        float temperature = msg["temperature"];
        float humidity = msg["humidity"];
        Serial.printf("  Sensor data - Temp: %.1f°C, Humidity: %.1f%%\n", 
                      temperature, humidity);
    } else if (dataType == "battery") {
        float voltage = msg["voltage"];
        int percentage = msg["percentage"];
        Serial.printf("  Battery data - Voltage: %.2fV, Level: %d%%\n", 
                      voltage, percentage);
    }
}

void handleImageNotification(JsonObject& msg) {
    String filename = msg["filename"];
    int fileSize = msg["file_size"];
    int sourceNode = msg["source"];
    
    Serial.printf("Image notification from node %d: %s (%d bytes)\n", 
                  sourceNode, filename.c_str(), fileSize);
}

void queueMessage(const String& message, int targetNode) {
    if (queueSize < 10) {
        messageQueue[queueSize].payload = message;
        messageQueue[queueSize].targetNode = targetNode;
        messageQueue[queueSize].timestamp = millis();
        messageQueue[queueSize].retryCount = 0;
        queueSize++;
        Serial.printf("Message queued (queue size: %d)\n", queueSize);
    } else {
        Serial.println("Message queue full - dropping message");
        packetsLost++;
    }
}

void processMessageQueue() {
    if (queueSize == 0) return;
    
    // Try to send oldest message
    QueuedMessage& msg = messageQueue[0];
    
    if (LoRa.beginPacket()) {
        LoRa.print(msg.payload);
        if (LoRa.endPacket()) {
            packetsSent++;
            Serial.printf("Queued message sent: %s\n", msg.payload.c_str());
            
            // Remove from queue
            for (int i = 1; i < queueSize; i++) {
                messageQueue[i-1] = messageQueue[i];
            }
            queueSize--;
        } else {
            msg.retryCount++;
            if (msg.retryCount > 3 || (millis() - msg.timestamp) > MESSAGE_TIMEOUT) {
                Serial.println("Message expired - removing from queue");
                packetsLost++;
                
                // Remove expired message
                for (int i = 1; i < queueSize; i++) {
                    messageQueue[i-1] = messageQueue[i];
                }
                queueSize--;
            }
        }
    }
}

void cleanupStaleNodes() {
    unsigned long currentTime = millis();
    
    for (int i = 0; i < nodeCount; i++) {
        if (networkNodes[i].isActive && 
            (currentTime - networkNodes[i].lastSeen) > 300000) { // 5 minutes
            
            Serial.printf("Node %d is stale - marking inactive\n", networkNodes[i].id);
            networkNodes[i].isActive = false;
        }
    }
}

void handleSerialCommand() {
    char command = Serial.read();
    
    switch (command) {
        case 's':
        case 'S':
            showNetworkStatus();
            break;
            
        case 'n':
        case 'N':
            showNeighborNodes();
            break;
            
        case 'p':
        case 'P':
            sendPingToAllNodes();
            break;
            
        case 't':
        case 'T':
            sendTestMessage();
            break;
            
        case 'r':
        case 'R':
            resetStatistics();
            break;
            
        case 'q':
        case 'Q':
            showMessageQueue();
            break;
            
        default:
            // Ignore unknown commands
            break;
    }
}

void showNetworkStatus() {
    Serial.println("\n=== NETWORK STATUS ===");
    Serial.printf("Node ID: %d\n", NODE_ID);
    Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
    Serial.printf("Frequency: %.1f MHz\n", LORA_FREQUENCY / 1E6);
    Serial.printf("TX Power: %d dBm\n", LORA_TX_POWER);
    Serial.printf("Spreading Factor: %d\n", LORA_SPREADING_FACTOR);
    Serial.println();
    
    Serial.println("Statistics:");
    Serial.printf("  Packets Received: %lu\n", packetsReceived);
    Serial.printf("  Packets Sent: %lu\n", packetsSent);
    Serial.printf("  Packets Lost: %lu\n", packetsLost);
    Serial.printf("  Success Rate: %.1f%%\n", 
                  (packetsSent > 0) ? (float)(packetsSent - packetsLost) / packetsSent * 100 : 0);
    Serial.printf("  Last RSSI: %d dBm\n", lastRSSI);
    Serial.printf("  Last SNR: %.1f dB\n", lastSNR);
    Serial.printf("  Queue Size: %d\n", queueSize);
    Serial.println();
    
    Serial.printf("Active Nodes: %d\n", getActiveNodeCount());
    Serial.println("======================\n");
}

void showNeighborNodes() {
    Serial.println("\n=== NEIGHBOR NODES ===");
    Serial.println("ID | Last Seen | RSSI | SNR  | Hops | Status");
    Serial.println("---|-----------|------|------|------|--------");
    
    for (int i = 0; i < nodeCount; i++) {
        if (networkNodes[i].id != -1) {
            unsigned long timeSince = (millis() - networkNodes[i].lastSeen) / 1000;
            Serial.printf("%2d | %8lus | %4d | %4.1f | %4d | %s\n",
                          networkNodes[i].id,
                          timeSince,
                          networkNodes[i].rssi,
                          networkNodes[i].snr,
                          networkNodes[i].hopCount,
                          networkNodes[i].isActive ? "ACTIVE" : "STALE");
        }
    }
    Serial.println("======================\n");
}

void sendPingToAllNodes() {
    Serial.println("Sending ping to all nodes...");
    
    DynamicJsonDocument data(128);
    data["ping_id"] = random(1000, 9999);
    data["timestamp"] = millis();
    
    sendMessage(MSG_PING, data.as<JsonObject>());
}

void sendTestMessage() {
    Serial.println("Sending test data message...");
    
    DynamicJsonDocument data(256);
    data["data_type"] = "test";
    data["sequence"] = random(1, 1000);
    data["temperature"] = 23.5 + random(-50, 50) / 10.0;
    data["humidity"] = 65 + random(-20, 20);
    data["battery_voltage"] = 3.7 + random(-30, 50) / 100.0;
    
    sendMessage(MSG_DATA, data.as<JsonObject>());
}

void resetStatistics() {
    packetsReceived = 0;
    packetsSent = 0;
    packetsLost = 0;
    packetCounter = 0;
    Serial.println("Statistics reset");
}

void showMessageQueue() {
    Serial.printf("\n=== MESSAGE QUEUE (%d messages) ===\n", queueSize);
    
    for (int i = 0; i < queueSize; i++) {
        QueuedMessage& msg = messageQueue[i];
        unsigned long age = (millis() - msg.timestamp) / 1000;
        Serial.printf("%d: Target=%d, Age=%lus, Retries=%d\n", 
                      i, msg.targetNode, age, msg.retryCount);
        Serial.printf("   Payload: %s\n", msg.payload.c_str());
    }
    Serial.println("===========================\n");
}

int getActiveNodeCount() {
    int count = 0;
    for (int i = 0; i < nodeCount; i++) {
        if (networkNodes[i].isActive) count++;
    }
    return count;
}

#ifdef USE_DISPLAY
void updateDisplay() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    display.printf("LoRa Node %d\n", NODE_ID);
    display.printf("Freq: %.0fMHz\n", LORA_FREQUENCY / 1E6);
    display.printf("Nodes: %d\n", getActiveNodeCount());
    display.printf("RX: %lu TX: %lu\n", packetsReceived, packetsSent);
    display.printf("RSSI: %d dBm\n", lastRSSI);
    display.printf("SNR: %.1f dB\n", lastSNR);
    display.printf("Queue: %d\n", queueSize);
    
    display.display();
}
#endif
```

### Multi-Node Setup Instructions

#### Node Configuration
```cpp
// Node 1 (Base Station/Gateway)
#define NODE_ID 1

// Node 2 (Remote Camera 1)  
#define NODE_ID 2

// Node 3 (Remote Camera 2)
#define NODE_ID 3

// Ensure all nodes use the same:
#define LORA_FREQUENCY 915E6
#define LORA_SYNC_WORD 0x12
```

#### Testing Procedure

1. **Single Node Test:**
   ```bash
   1. Upload firmware to first ESP32 with NODE_ID 1
   2. Open serial monitor
   3. Verify LoRa initialization
   4. Send test commands (s, t, p)
   ```

2. **Two Node Test:**
   ```bash
   1. Upload to second ESP32 with NODE_ID 2
   2. Place nodes 10-50 meters apart
   3. Monitor both serial outputs
   4. Verify heartbeat exchange
   5. Test ping and data messages
   ```

3. **Multi-Node Mesh:**
   ```bash
   1. Add additional nodes incrementally
   2. Test communication between all pairs
   3. Verify message forwarding/routing
   4. Monitor network statistics
   ```

### Range Testing

#### Line-of-Sight Testing
```bash
Testing Procedure:
1. Start with nodes 100m apart
2. Gradually increase distance by 200m
3. Monitor RSSI and packet loss
4. Document maximum reliable range

Expected Results:
- 1km+ in open areas
- 500m in suburban areas  
- 200m in dense urban/forest
```

#### Obstacle Testing
```bash
Test Scenarios:
1. Through buildings (concrete/metal)
2. Through vegetation (forests)
3. Over terrain variations (hills)
4. Different weather conditions

Optimization:
- Increase spreading factor for longer range
- Reduce bandwidth for better sensitivity
- Use higher gain antennas
- Adjust transmission power
```

### Network Topology Examples

#### Star Network (Centralized)
```
    Node 2
      |
Node 3--Base--Node 4
      |
    Node 5
```

#### Mesh Network (Distributed)
```
Node 1 ---- Node 2 ---- Node 3
   |           |           |
Node 4 ---- Node 5 ---- Node 6
```

#### Linear Network (Repeater Chain)
```
Base ---- Node 2 ---- Node 3 ---- Node 4 ---- Node 5
(Gateway)              (Repeater)
```

### Performance Optimization

#### Range vs Data Rate Trade-offs
```cpp
// Maximum Range Configuration
#define LORA_SPREADING_FACTOR 12      // Slowest, longest range
#define LORA_SIGNAL_BANDWIDTH 62.5E3  // Narrowest bandwidth
#define LORA_CODING_RATE 8            // Maximum error correction

// High Data Rate Configuration  
#define LORA_SPREADING_FACTOR 7       // Fastest, shorter range
#define LORA_SIGNAL_BANDWIDTH 250E3   // Widest bandwidth
#define LORA_CODING_RATE 5            // Minimum error correction

// Balanced Configuration (recommended)
#define LORA_SPREADING_FACTOR 7-9     // Good range/speed balance
#define LORA_SIGNAL_BANDWIDTH 125E3   // Standard bandwidth
#define LORA_CODING_RATE 5            // Standard error correction
```

#### Power Optimization
```cpp
// Adaptive power control based on distance
void adjustTransmissionPower(int rssi) {
    if (rssi > -70) {
        LoRa.setTxPower(10);  // Close range - reduce power
    } else if (rssi > -90) {
        LoRa.setTxPower(15);  // Medium range
    } else {
        LoRa.setTxPower(20);  // Long range - maximum power
    }
}

// Duty cycle compliance (1% in Europe, varies by region)
unsigned long lastTransmission = 0;
const unsigned long MIN_INTERVAL = 3600; // 36 seconds for 1% duty cycle

bool canTransmit() {
    return (millis() - lastTransmission) > MIN_INTERVAL;
}
```

### Troubleshooting

#### No Communication Between Nodes
```bash
1. Verify identical frequency settings
2. Check antenna connections
3. Confirm sync word matches
4. Test with nodes very close together
5. Monitor serial output for error messages
```

#### Poor Range Performance
```bash
1. Check antenna impedance and SWR
2. Verify clear line of sight
3. Increase spreading factor
4. Use higher gain antennas
5. Check for interference sources
```

#### High Packet Loss
```bash
1. Reduce transmission rate
2. Implement acknowledgments and retries
3. Check for duty cycle violations
4. Monitor for interference
5. Adjust error correction coding
```

### Legal Considerations

#### Frequency Regulations
```bash
North America (FCC):
- 902-928 MHz ISM band
- Max power: 1W (30 dBm)
- No license required

Europe (ETSI):
- 863-870 MHz ISM band  
- Max power: 25mW (14 dBm)
- 1% duty cycle limit

Always verify local regulations before deployment!
```

### Integration with Wildlife Camera

#### Message Types for Camera Network
```cpp
#define MSG_IMAGE_CAPTURE 10     // Trigger image capture
#define MSG_IMAGE_METADATA 11    // Image information
#define MSG_BATTERY_STATUS 12    // Power status
#define MSG_MOTION_DETECTION 13  // Motion event
#define MSG_SYSTEM_STATUS 14     // General health
#define MSG_CONFIG_UPDATE 15     // Configuration changes
```

#### Data Compression
```cpp
// Compress image metadata for transmission
void compressImageData(JsonObject& imageData) {
    // Use abbreviations to reduce packet size
    imageData["t"] = imageData["timestamp"];     // timestamp
    imageData["s"] = imageData["size"];          // size
    imageData["w"] = imageData["width"];         // width  
    imageData["h"] = imageData["height"];        // height
    imageData["n"] = imageData["node_id"];       // node_id
    
    // Remove verbose fields
    imageData.remove("timestamp");
    imageData.remove("size");
    imageData.remove("width");
    imageData.remove("height");
    imageData.remove("node_id");
}
```

---

**This example establishes the foundation for reliable long-range communication between wildlife cameras deployed in remote locations without cellular or WiFi coverage.**