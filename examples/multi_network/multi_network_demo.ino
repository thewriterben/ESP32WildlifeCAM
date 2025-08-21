/**
 * @file multi_network_demo.ino
 * @brief Multi-network communication demonstration for ESP32 Wildlife Camera
 * 
 * This example demonstrates the automatic network selection and failover
 * capabilities across LoRa mesh, cellular, and satellite communication.
 * 
 * Features demonstrated:
 * - Automatic network discovery and selection
 * - Priority-based network switching
 * - Cost optimization and power management
 * - Emergency communication fallback
 * - Data compression and optimization for each network type
 */

#include "network_selector.h"
#include "cellular_manager.h"
#include "satellite_comm.h"
#include "data_compression.h"
#include "gps_handler.h"

// Configuration
#define ENABLE_CELLULAR true
#define ENABLE_SATELLITE true
#define ENABLE_LORA_SIMULATION true  // Simulated LoRa for demo

// Network priority configuration
#define PRIMARY_NETWORK LORA_MESH
#define SECONDARY_NETWORK CELLULAR
#define BACKUP_NETWORK SATELLITE

// Data transmission limits
#define CELLULAR_DAILY_LIMIT_MB 100
#define SATELLITE_MONTHLY_MESSAGES 50

// Global objects
NetworkSelector networkSelector;
CellularManager cellularManager;
SatelliteComm satelliteComm;
DataCompression dataCompressor;
GPSHandler gpsHandler;

// Demo data structures
struct WildlifeCameraData {
    unsigned long timestamp;
    float latitude;
    float longitude;
    float altitude;
    int batteryLevel;
    float temperature;
    float humidity;
    bool motionDetected;
    int imageSize;
    uint16_t imageCRC;
};

struct NetworkStats {
    int messagesAttempted[4];  // Index: NONE, LoRa, Cellular, Satellite
    int messagesSuccessful[4];
    unsigned long totalTransmissionTime[4];
    float totalCost;
};

NetworkStats stats = {};

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== ESP32 Wildlife Camera - Multi-Network Demo ===");
    printConfiguration();
    
    // Initialize GPS first (needed for satellite communication)
    Serial.println("Initializing GPS...");
    if (gpsHandler.initialize()) {
        Serial.println("✓ GPS initialized successfully");
        
        // Wait for GPS fix
        if (gpsHandler.waitForFix(60000)) {
            GPSLocation location = gpsHandler.getCurrentLocation();
            Serial.printf("✓ GPS fix acquired: %.6f, %.6f\n", 
                         location.latitude, location.longitude);
        } else {
            Serial.println("⚠ GPS fix timeout - satellite communication may be limited");
        }
    } else {
        Serial.println("✗ GPS initialization failed");
    }
    
    // Initialize cellular module
    #if ENABLE_CELLULAR
    Serial.println("Initializing cellular communication...");
    if (cellularManager.initialize()) {
        cellularManager.setDataLimit(CELLULAR_DAILY_LIMIT_MB);
        
        if (cellularManager.connectToNetwork()) {
            Serial.println("✓ Cellular network connected");
            displayCellularInfo();
        } else {
            Serial.println("⚠ Cellular connection failed - will retry later");
        }
    } else {
        Serial.println("✗ Cellular initialization failed");
    }
    #endif
    
    // Initialize satellite communication
    #if ENABLE_SATELLITE
    Serial.println("Initializing satellite communication...");
    if (satelliteComm.initialize(MODULE_SWARM)) {
        satelliteComm.setMessageLimit(SATELLITE_MONTHLY_MESSAGES);
        Serial.println("✓ Satellite communication initialized");
    } else {
        Serial.println("✗ Satellite initialization failed");
    }
    #endif
    
    // Initialize network selector
    Serial.println("Initializing network selector...");
    if (networkSelector.initialize(&cellularManager, &satelliteComm)) {
        Serial.println("✓ Network selector initialized");
        
        // Perform initial network scan
        networkSelector.scanAvailableNetworks();
        displayAvailableNetworks();
    } else {
        Serial.println("✗ Network selector initialization failed");
    }
    
    Serial.println("\n=== Starting Multi-Network Communication Demo ===");
    Serial.println("The system will automatically select the optimal network");
    Serial.println("for each type of data transmission.\n");
}

void loop() {
    static unsigned long lastTelemetryTime = 0;
    static unsigned long lastImageTime = 0;
    static unsigned long lastStatusTime = 0;
    static unsigned long lastNetworkScanTime = 0;
    
    unsigned long currentTime = millis();
    
    // Periodic network scanning (every 5 minutes)
    if (currentTime - lastNetworkScanTime >= 300000) {
        networkSelector.periodicNetworkCheck();
        lastNetworkScanTime = currentTime;
    }
    
    // Send telemetry data every 2 minutes
    if (currentTime - lastTelemetryTime >= 120000) {
        sendTelemetryData();
        lastTelemetryTime = currentTime;
    }
    
    // Send image data every 15 minutes (simulate motion detection)
    if (currentTime - lastImageTime >= 900000) {
        simulateImageCapture();
        lastImageTime = currentTime;
    }
    
    // Send status report every 30 minutes
    if (currentTime - lastStatusTime >= 1800000) {
        sendStatusReport();
        displayNetworkStatistics();
        lastStatusTime = currentTime;
    }
    
    // Simulate emergency event (uncomment to test)
    // if (currentTime >= 600000 && currentTime <= 600100) { // At 10 minutes
    //     simulateEmergencyEvent();
    // }
    
    delay(10000); // Check every 10 seconds
}

void sendTelemetryData() {
    Serial.println("\n--- Sending Telemetry Data ---");
    
    WildlifeCameraData data = generateCameraData();
    
    // Create compact telemetry message
    String telemetryJson = createTelemetryMessage(data);
    
    Serial.printf("Telemetry size: %d bytes\n", telemetryJson.length());
    Serial.println("Data: " + telemetryJson);
    
    // Select optimal network for telemetry (low priority, small data)
    bool success = networkSelector.sendData((uint8_t*)telemetryJson.c_str(), 
                                           telemetryJson.length(), 
                                           PRIORITY_LOW);
    
    NetworkType usedNetwork = networkSelector.getCurrentNetwork();
    updateNetworkStats(usedNetwork, success, telemetryJson.length());
    
    Serial.printf("Transmission: %s via %s\n", 
                  success ? "SUCCESS" : "FAILED",
                  networkSelector.getNetworkTypeName(usedNetwork));
}

void simulateImageCapture() {
    Serial.println("\n--- Simulating Image Capture ---");
    
    // Simulate image data (64KB typical for compressed wildlife image)
    const size_t imageSize = 65536;
    uint8_t* imageData = new uint8_t[imageSize];
    
    // Fill with simulated image data
    for (size_t i = 0; i < imageSize; i++) {
        imageData[i] = random(0, 256);
    }
    
    WildlifeCameraData metadata = generateCameraData();
    metadata.imageSize = imageSize;
    metadata.imageCRC = calculateCRC16(imageData, imageSize);
    
    Serial.printf("Captured image: %d bytes, CRC: 0x%04X\n", 
                  metadata.imageSize, metadata.imageCRC);
    
    // Determine transmission strategy based on available networks
    std::vector<NetworkInfo> networks = networkSelector.getAvailableNetworks();
    
    bool hasCellular = false;
    bool hasSatellite = false;
    
    for (const auto& network : networks) {
        if (network.type == NETWORK_TYPE_CELLULAR && network.available) {
            hasCellular = true;
        }
        if (network.type == NETWORK_TYPE_SATELLITE && network.available) {
            hasSatellite = true;
        }
    }
    
    if (hasCellular) {
        // Send full image via cellular
        sendImageViaCellular(imageData, imageSize, metadata);
    } else if (hasSatellite) {
        // Send compressed metadata via satellite
        sendImageMetadataViaSatellite(metadata);
    } else {
        // Store image locally for later transmission
        Serial.println("No suitable network available - storing image locally");
    }
    
    delete[] imageData;
}

void sendImageViaCellular(uint8_t* imageData, size_t imageSize, const WildlifeCameraData& metadata) {
    Serial.println("Sending full image via cellular...");
    
    // Compress image for cellular transmission
    uint8_t* compressedImage = new uint8_t[imageSize];
    size_t compressedSize = dataCompressor.optimizeForCellular(imageData, imageSize, 
                                                              compressedImage, imageSize, 2);
    
    if (compressedSize > 0) {
        Serial.printf("Image compressed: %d -> %d bytes (%.1f%%)\n", 
                     imageSize, compressedSize, 
                     (float)compressedSize / imageSize * 100);
        
        bool success = networkSelector.sendData(compressedImage, compressedSize, PRIORITY_NORMAL);
        
        updateNetworkStats(NETWORK_TYPE_CELLULAR, success, compressedSize);
        
        Serial.printf("Image transmission: %s\n", success ? "SUCCESS" : "FAILED");
    } else {
        Serial.println("Image compression failed");
    }
    
    delete[] compressedImage;
}

void sendImageMetadataViaSatellite(const WildlifeCameraData& metadata) {
    Serial.println("Sending image metadata via satellite...");
    
    // Create metadata message optimized for satellite
    String metadataMessage = createSatelliteMessage(metadata);
    
    Serial.printf("Metadata size: %d bytes\n", metadataMessage.length());
    Serial.println("Message: " + metadataMessage);
    
    bool success = networkSelector.sendData((uint8_t*)metadataMessage.c_str(), 
                                           metadataMessage.length(), 
                                           PRIORITY_HIGH);
    
    updateNetworkStats(NETWORK_TYPE_SATELLITE, success, metadataMessage.length());
    
    Serial.printf("Metadata transmission: %s\n", success ? "SUCCESS" : "FAILED");
}

void sendStatusReport() {
    Serial.println("\n--- Sending Status Report ---");
    
    WildlifeCameraData data = generateCameraData();
    
    // Create comprehensive status message
    String statusMessage = createStatusMessage(data);
    
    Serial.printf("Status report size: %d bytes\n", statusMessage.length());
    
    // Send via most reliable available network
    bool success = networkSelector.sendData((uint8_t*)statusMessage.c_str(), 
                                           statusMessage.length(), 
                                           PRIORITY_NORMAL);
    
    NetworkType usedNetwork = networkSelector.getCurrentNetwork();
    updateNetworkStats(usedNetwork, success, statusMessage.length());
    
    Serial.printf("Status report: %s via %s\n", 
                  success ? "SUCCESS" : "FAILED",
                  networkSelector.getNetworkTypeName(usedNetwork));
}

void simulateEmergencyEvent() {
    Serial.println("\n!!! EMERGENCY EVENT DETECTED !!!");
    Serial.println("Low battery + motion detected - sending emergency alert");
    
    WildlifeCameraData data = generateCameraData();
    data.batteryLevel = 15; // Critical battery level
    data.motionDetected = true;
    
    String emergencyMessage = createEmergencyMessage(data);
    
    Serial.printf("Emergency message: %s\n", emergencyMessage.c_str());
    
    // Send with emergency priority - will try all available networks
    bool success = networkSelector.sendData((uint8_t*)emergencyMessage.c_str(), 
                                           emergencyMessage.length(), 
                                           PRIORITY_EMERGENCY);
    
    if (success) {
        Serial.println("✓ Emergency alert transmitted successfully");
    } else {
        Serial.println("✗ Emergency alert transmission failed");
    }
}

WildlifeCameraData generateCameraData() {
    WildlifeCameraData data;
    
    data.timestamp = millis();
    
    // Get GPS location if available
    if (gpsHandler.isLocationValid()) {
        GPSLocation location = gpsHandler.getCurrentLocation();
        data.latitude = location.latitude;
        data.longitude = location.longitude;
        data.altitude = location.altitude;
    } else {
        data.latitude = 0.0;
        data.longitude = 0.0;
        data.altitude = 0.0;
    }
    
    // Simulate sensor readings
    data.batteryLevel = random(70, 100);
    data.temperature = random(15, 35) + (random(0, 100) / 100.0);
    data.humidity = random(40, 80);
    data.motionDetected = random(0, 10) < 2; // 20% chance of motion
    
    return data;
}

String createTelemetryMessage(const WildlifeCameraData& data) {
    String message = "{";
    message += "\"t\":" + String(data.timestamp) + ",";
    message += "\"b\":" + String(data.batteryLevel) + ",";
    message += "\"temp\":" + String(data.temperature, 1) + ",";
    message += "\"hum\":" + String(data.humidity) + ",";
    message += "\"motion\":" + String(data.motionDetected ? 1 : 0);
    
    if (data.latitude != 0.0 && data.longitude != 0.0) {
        message += ",\"lat\":" + String(data.latitude, 6);
        message += ",\"lon\":" + String(data.longitude, 6);
    }
    
    message += "}";
    return message;
}

String createSatelliteMessage(const WildlifeCameraData& data) {
    // Ultra-compact format for satellite transmission
    String message = "IMG:";
    message += String(data.timestamp) + ",";
    message += String(data.imageSize) + ",";
    message += String(data.imageCRC, HEX) + ",";
    message += String(data.batteryLevel) + ",";
    
    if (data.latitude != 0.0 && data.longitude != 0.0) {
        message += String(data.latitude, 4) + ",";
        message += String(data.longitude, 4);
    }
    
    return message;
}

String createStatusMessage(const WildlifeCameraData& data) {
    String message = "{";
    message += "\"timestamp\":" + String(data.timestamp) + ",";
    message += "\"location\":{";
    message += "\"lat\":" + String(data.latitude, 6) + ",";
    message += "\"lon\":" + String(data.longitude, 6) + ",";
    message += "\"alt\":" + String(data.altitude, 1);
    message += "},";
    message += "\"sensors\":{";
    message += "\"battery\":" + String(data.batteryLevel) + ",";
    message += "\"temperature\":" + String(data.temperature, 1) + ",";
    message += "\"humidity\":" + String(data.humidity);
    message += "},";
    message += "\"system\":{";
    message += "\"uptime\":" + String(millis()) + ",";
    message += "\"memory\":" + String(ESP.getFreeHeap()) + ",";
    message += "\"cellular_usage\":" + String(cellularManager.getDataUsage()) + ",";
    message += "\"satellite_msgs\":" + String(satelliteComm.getMessageCount());
    message += "}";
    message += "}";
    
    return message;
}

String createEmergencyMessage(const WildlifeCameraData& data) {
    String message = "EMERGENCY:";
    message += "BAT=" + String(data.batteryLevel) + "%,";
    
    if (data.latitude != 0.0 && data.longitude != 0.0) {
        message += "LOC=" + String(data.latitude, 4) + "," + String(data.longitude, 4) + ",";
    }
    
    message += "TIME=" + String(data.timestamp);
    
    return message;
}

void updateNetworkStats(NetworkType networkType, bool success, size_t dataSize) {
    int index = (int)networkType;
    if (index >= 0 && index < 4) {
        stats.messagesAttempted[index]++;
        if (success) {
            stats.messagesSuccessful[index]++;
        }
    }
    
    // Estimate costs (simplified)
    switch (networkType) {
        case NETWORK_TYPE_CELLULAR:
            stats.totalCost += dataSize * 0.0001; // $0.10 per MB
            break;
        case NETWORK_TYPE_SATELLITE:
            stats.totalCost += 0.007; // $0.007 per message (Swarm pricing)
            break;
        case NETWORK_TYPE_LORA:
            // Free
            break;
    }
}

void displayAvailableNetworks() {
    Serial.println("\n--- Available Networks ---");
    
    std::vector<NetworkInfo> networks = networkSelector.getAvailableNetworks();
    
    for (const auto& network : networks) {
        Serial.printf("%s: ", networkSelector.getNetworkTypeName(network.type));
        
        if (network.available) {
            Serial.printf("Available (Signal: %d, Cost: $%.3f)\n", 
                         network.signalStrength, network.estimatedCost);
        } else {
            Serial.println("Unavailable");
        }
    }
    
    Serial.println("----------------------------");
}

void displayCellularInfo() {
    Serial.println("--- Cellular Information ---");
    Serial.printf("Signal Strength: %d dBm\n", cellularManager.getSignalStrength());
    
    NetworkType type = cellularManager.getNetworkType();
    Serial.print("Network Type: ");
    switch (type) {
        case NETWORK_LTE: Serial.println("LTE"); break;
        case NETWORK_3G: Serial.println("3G"); break;
        case NETWORK_2G: Serial.println("2G"); break;
        default: Serial.println("Unknown"); break;
    }
    
    Serial.printf("Data Usage: %d MB\n", cellularManager.getDataUsage());
    Serial.println("-----------------------------");
}

void displayNetworkStatistics() {
    Serial.println("\n========== Network Statistics ==========");
    
    const char* networkNames[] = {"None", "LoRa", "Cellular", "Satellite"};
    
    for (int i = 1; i < 4; i++) {
        if (stats.messagesAttempted[i] > 0) {
            float successRate = (float)stats.messagesSuccessful[i] / stats.messagesAttempted[i] * 100;
            Serial.printf("%s: %d/%d (%.1f%% success)\n", 
                         networkNames[i], 
                         stats.messagesSuccessful[i], 
                         stats.messagesAttempted[i], 
                         successRate);
        }
    }
    
    Serial.printf("Total estimated cost: $%.2f\n", stats.totalCost);
    Serial.println("=======================================");
}

void printConfiguration() {
    Serial.println("\n--- Configuration ---");
    Serial.printf("Cellular: %s\n", ENABLE_CELLULAR ? "Enabled" : "Disabled");
    Serial.printf("Satellite: %s\n", ENABLE_SATELLITE ? "Enabled" : "Disabled");
    Serial.printf("LoRa Simulation: %s\n", ENABLE_LORA_SIMULATION ? "Enabled" : "Disabled");
    Serial.printf("Cellular Data Limit: %d MB/day\n", CELLULAR_DAILY_LIMIT_MB);
    Serial.printf("Satellite Message Limit: %d messages/month\n", SATELLITE_MONTHLY_MESSAGES);
    Serial.println("--------------------");
}

uint16_t calculateCRC16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}