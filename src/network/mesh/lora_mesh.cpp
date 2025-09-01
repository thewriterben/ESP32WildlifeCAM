/**
 * @file lora_mesh.cpp
 * @brief Enhanced Multi-hop LoRa Mesh Protocol Implementation for Phase 2
 * 
 * This implementation extends the existing LoRa mesh capabilities with
 * advanced features for wildlife monitoring including dynamic routing,
 * adaptive timing, and network resilience.
 * 
 * Phase 2 Enhancement - March 31, 2026 Target
 */

#include "mesh_protocol.h"
#include "../../../firmware/src/meshtastic/mesh_interface.h"
#include "../../../firmware/src/lora_mesh.h"
#include <ArduinoJson.h>
#include <LoRa.h>

// ===========================
// ENHANCED LORA MESH CLASS
// ===========================

class EnhancedLoRaMesh : public MeshProtocolV2 {
private:
    std::map<uint32_t, RouteInfoV2> routingTable_;
    std::map<uint32_t, NodeInfoV2> nodeRegistry_;
    NetworkTopology topology_;
    NetworkHealthMetrics healthMetrics_;
    TimeSync timeSync_;
    
    uint32_t nodeId_;
    bool isInitialized_;
    bool isCoordinator_;
    uint32_t lastBeacon_;
    uint32_t lastDiscovery_;
    uint32_t lastOptimization_;
    
    // Adaptive timing parameters
    uint32_t adaptiveBeaconInterval_;
    uint32_t collisionBackoff_;
    uint8_t networkLoad_;
    
    // Performance metrics
    uint32_t routeOptimizationCount_;
    uint32_t collisionAvoidanceCount_;
    float avgLatency_;

public:
    EnhancedLoRaMesh() : 
        nodeId_(0), isInitialized_(false), isCoordinator_(false),
        lastBeacon_(0), lastDiscovery_(0), lastOptimization_(0),
        adaptiveBeaconInterval_(MESH_BEACON_INTERVAL),
        collisionBackoff_(100), networkLoad_(0),
        routeOptimizationCount_(0), collisionAvoidanceCount_(0),
        avgLatency_(0.0) {
        
        // Initialize metrics
        memset(&healthMetrics_, 0, sizeof(healthMetrics_));
        memset(&timeSync_, 0, sizeof(timeSync_));
    }
    
    // ===========================
    // CORE PROTOCOL IMPLEMENTATION
    // ===========================
    
    bool initializeNetwork() override {
        if (isInitialized_) return true;
        
        // Initialize base LoRa system
        if (!LoraMesh::init()) {
            Serial.println("Failed to initialize base LoRa mesh");
            return false;
        }
        
        // Generate node ID from MAC address
        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        nodeId_ = (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5];
        
        // Initialize topology
        topology_.nodeId = nodeId_;
        topology_.lastUpdated = millis();
        topology_.nodeCount = 1;
        topology_.networkHealth = 1.0;
        
        // Register self in node registry
        NodeInfoV2 selfNode;
        selfNode.nodeId = nodeId_;
        selfNode.capabilities = CAP_BASIC_CAMERA | CAP_AI_PROCESSING | CAP_ENVIRONMENTAL_SENSORS;
        selfNode.batteryLevel = 100; // TODO: Get actual battery level
        selfNode.signalQuality = 100;
        selfNode.uptime = millis() / 1000;
        selfNode.temperature = 25.0; // TODO: Get actual temperature
        selfNode.memoryFree = ESP.getFreeHeap() / 1024;
        selfNode.lastSeen = millis();
        selfNode.isCoordinator = false;
        
        nodeRegistry_[nodeId_] = selfNode;
        
        isInitialized_ = true;
        Serial.printf("Enhanced LoRa mesh initialized with node ID: %08X\n", nodeId_);
        
        return true;
    }
    
    bool addRoute(const RouteInfoV2& route) override {
        if (!isInitialized_) return false;
        
        // Validate route
        if (route.destination == nodeId_ || route.nextHop == 0) {
            return false;
        }
        
        // Check if this is a better route
        auto existing = routingTable_.find(route.destination);
        if (existing != routingTable_.end()) {
            // Compare routes: prefer higher reliability, lower hop count
            float existingScore = existing->second.reliability * (10 - existing->second.hopCount);
            float newScore = route.reliability * (10 - route.hopCount);
            
            if (newScore <= existingScore) {
                return false; // Existing route is better
            }
        }
        
        // Add or update route
        routingTable_[route.destination] = route;
        
        // Update topology
        if (std::find(topology_.neighbors.begin(), topology_.neighbors.end(), route.nextHop) == topology_.neighbors.end()) {
            topology_.neighbors.push_back(route.nextHop);
        }
        
        topology_.routes[route.destination] = route;
        topology_.lastUpdated = millis();
        
        Serial.printf("Added route to %08X via %08X (hops: %d, reliability: %.2f)\n", 
                     route.destination, route.nextHop, route.hopCount, route.reliability);
        
        return true;
    }
    
    bool removeRoute(uint32_t destination) override {
        auto it = routingTable_.find(destination);
        if (it != routingTable_.end()) {
            routingTable_.erase(it);
            topology_.routes.erase(destination);
            topology_.lastUpdated = millis();
            return true;
        }
        return false;
    }
    
    RouteInfoV2 findBestRoute(uint32_t destination) override {
        auto it = routingTable_.find(destination);
        if (it != routingTable_.end()) {
            // Update last used time
            it->second.lastUsed = millis();
            return it->second;
        }
        
        // Return empty route if not found
        RouteInfoV2 emptyRoute = {};
        return emptyRoute;
    }
    
    bool updateTopology(const NetworkTopology& topology) override {
        // Merge received topology with our local topology
        for (const auto& route : topology.routes) {
            RouteInfoV2 updatedRoute = route.second;
            updatedRoute.hopCount++; // Increment hop count
            updatedRoute.reliability *= 0.95; // Slight reliability decrease for multi-hop
            
            addRoute(updatedRoute);
        }
        
        topology_.lastUpdated = millis();
        return true;
    }
    
    // ===========================
    // NODE DISCOVERY AND MANAGEMENT
    // ===========================
    
    bool startNodeDiscovery() override {
        if (!isInitialized_) return false;
        
        // Create discovery packet
        DynamicJsonDocument doc(512);
        doc["type"] = "node_discovery_v2";
        doc["requester"] = nodeId_;
        doc["timestamp"] = millis();
        doc["capabilities"] = nodeRegistry_[nodeId_].capabilities;
        doc["battery"] = nodeRegistry_[nodeId_].batteryLevel;
        doc["signal_quality"] = nodeRegistry_[nodeId_].signalQuality;
        
        String message;
        serializeJson(doc, message);
        
        // Broadcast discovery request with collision avoidance
        if (!avoidCollisions()) {
            return false;
        }
        
        bool success = LoraMesh::queueMessage(message);
        if (success) {
            lastDiscovery_ = millis();
        }
        
        return success;
    }
    
    bool registerNode(const NodeInfoV2& nodeInfo) override {
        if (nodeInfo.nodeId == nodeId_) return false; // Don't register self
        
        nodeRegistry_[nodeInfo.nodeId] = nodeInfo;
        
        // If this is a new neighbor, add to topology
        if (std::find(topology_.neighbors.begin(), topology_.neighbors.end(), nodeInfo.nodeId) == topology_.neighbors.end()) {
            topology_.neighbors.push_back(nodeInfo.nodeId);
            
            // Create direct route to neighbor
            RouteInfoV2 directRoute;
            directRoute.destination = nodeInfo.nodeId;
            directRoute.nextHop = nodeInfo.nodeId;
            directRoute.hopCount = 1;
            directRoute.lastUsed = millis();
            directRoute.reliability = 1.0;
            directRoute.signalStrength = nodeInfo.signalQuality * -1; // Convert to dBm estimate
            directRoute.bandwidth = 1000; // Default bandwidth estimate
            directRoute.priority = 128;
            
            addRoute(directRoute);
        }
        
        topology_.nodeCount = nodeRegistry_.size();
        topology_.lastUpdated = millis();
        
        return true;
    }
    
    std::vector<NodeInfoV2> getActiveNodes() override {
        std::vector<NodeInfoV2> activeNodes;
        uint32_t currentTime = millis();
        
        for (auto& pair : nodeRegistry_) {
            if (currentTime - pair.second.lastSeen < MESH_ROUTE_TIMEOUT) {
                activeNodes.push_back(pair.second);
            }
        }
        
        return activeNodes;
    }
    
    bool setNodeCapabilities(uint16_t capabilities) override {
        if (!isInitialized_) return false;
        
        nodeRegistry_[nodeId_].capabilities = capabilities;
        return true;
    }
    
    // ===========================
    // WILDLIFE DATA TRANSMISSION
    // ===========================
    
    bool transmitWildlifeData(const WildlifeDetection& detection) override {
        if (!isInitialized_) return false;
        
        // Compress wildlife data
        uint8_t buffer[256];
        size_t compressedSize = compressWildlifeData(detection, buffer, sizeof(buffer));
        
        if (compressedSize == 0) {
            Serial.println("Failed to compress wildlife data");
            return false;
        }
        
        // Create transmission packet
        DynamicJsonDocument doc(512);
        doc["type"] = "wildlife_detection";
        doc["source"] = nodeId_;
        doc["timestamp"] = detection.timestamp;
        doc["species"] = detection.speciesId;
        doc["confidence"] = detection.confidence;
        doc["behavior"] = detection.behaviorCode;
        doc["image_id"] = detection.imageId;
        doc["env_score"] = detection.environmentalScore;
        
        // Add compressed bounding box
        JsonArray bbox = doc.createNestedArray("bbox");
        for (int i = 0; i < 4; i++) {
            bbox.add(detection.boundingBox[i]);
        }
        
        String message;
        serializeJson(doc, message);
        
        return LoraMesh::queueMessage(message);
    }
    
    bool transmitEnvironmentalData(const EnvironmentalData& envData) override {
        if (!isInitialized_) return false;
        
        // Create environmental data packet
        DynamicJsonDocument doc(512);
        doc["type"] = "environmental_data";
        doc["source"] = nodeId_;
        doc["timestamp"] = envData.timestamp;
        doc["temp"] = envData.temperature;
        doc["humidity"] = envData.humidity;
        doc["pressure"] = envData.pressure;
        doc["light"] = envData.lightLevel;
        doc["pm25"] = envData.pm25;
        doc["co2"] = envData.co2;
        doc["soil"] = envData.soilMoisture;
        doc["wind_speed"] = envData.windSpeed;
        doc["wind_dir"] = envData.windDirection;
        doc["aqi"] = envData.airQualityIndex;
        
        String message;
        serializeJson(doc, message);
        
        return LoraMesh::queueMessage(message);
    }
    
    bool requestDataSync() override {
        if (!isInitialized_) return false;
        
        DynamicJsonDocument doc(256);
        doc["type"] = "data_sync_request";
        doc["requester"] = nodeId_;
        doc["timestamp"] = millis();
        doc["last_sync"] = 0; // TODO: Track last sync time
        
        String message;
        serializeJson(doc, message);
        
        return LoraMesh::queueMessage(message);
    }
    
    // ===========================
    // NETWORK OPTIMIZATION
    // ===========================
    
    bool optimizeRoutes() override {
        if (!isInitialized_) return false;
        
        uint32_t currentTime = millis();
        if (currentTime - lastOptimization_ < 30000) { // Optimize at most every 30 seconds
            return true;
        }
        
        // Remove stale routes
        auto it = routingTable_.begin();
        while (it != routingTable_.end()) {
            if (currentTime - it->second.lastUsed > MESH_ROUTE_TIMEOUT) {
                Serial.printf("Removing stale route to %08X\n", it->first);
                it = routingTable_.erase(it);
            } else {
                ++it;
            }
        }
        
        // Update network health
        calculateNetworkHealth();
        
        // If network health is poor, trigger discovery
        if (topology_.networkHealth < 0.5) {
            startNodeDiscovery();
        }
        
        lastOptimization_ = currentTime;
        routeOptimizationCount_++;
        
        return true;
    }
    
    bool balanceNetworkLoad() override {
        // Simple load balancing: distribute traffic across available routes
        networkLoad_ = (healthMetrics_.totalPacketsSent + healthMetrics_.totalPacketsReceived) % 100;
        
        if (networkLoad_ > 80) {
            // High load: increase beacon interval to reduce traffic
            adaptiveBeaconInterval_ = min(MESH_BEACON_INTERVAL * 2, 300000UL);
        } else if (networkLoad_ < 20) {
            // Low load: decrease beacon interval for better connectivity
            adaptiveBeaconInterval_ = max(MESH_BEACON_INTERVAL / 2, 30000UL);
        }
        
        return true;
    }
    
    bool avoidCollisions() override {
        // Simple collision avoidance using random backoff
        if (networkLoad_ > 50) {
            uint32_t backoff = random(collisionBackoff_, collisionBackoff_ * 3);
            delay(backoff);
            collisionBackoff_ = min(collisionBackoff_ * 2, 5000UL); // Exponential backoff
            collisionAvoidanceCount_++;
        } else {
            collisionBackoff_ = max(collisionBackoff_ / 2, 100UL); // Reduce backoff
        }
        
        return true;
    }
    
    NetworkHealthMetrics getNetworkHealth() override {
        return healthMetrics_;
    }
    
    // ===========================
    // TIME SYNCHRONIZATION
    // ===========================
    
    bool synchronizeTime() override {
        if (!isInitialized_) return false;
        
        if (isCoordinator_) {
            // As coordinator, broadcast time sync
            DynamicJsonDocument doc(256);
            doc["type"] = "time_sync";
            doc["coordinator"] = nodeId_;
            doc["timestamp"] = millis();
            doc["utc_time"] = 0; // TODO: Get actual UTC time
            
            String message;
            serializeJson(doc, message);
            
            timeSync_.coordinatorTime = millis();
            timeSync_.lastSync = millis();
            timeSync_.isSynced = true;
            timeSync_.syncAccuracy = 10; // Assume 10ms accuracy for coordinator
            
            return LoraMesh::queueMessage(message);
        } else {
            // Request time sync from coordinator
            DynamicJsonDocument doc(256);
            doc["type"] = "time_sync_request";
            doc["requester"] = nodeId_;
            doc["timestamp"] = millis();
            
            String message;
            serializeJson(doc, message);
            
            return LoraMesh::queueMessage(message);
        }
    }
    
    TimeSync getTimeSync() override {
        return timeSync_;
    }
    
    bool isTimeSynced() override {
        return timeSync_.isSynced && (millis() - timeSync_.lastSync < MESH_SYNC_INTERVAL);
    }
    
private:
    void calculateNetworkHealth() {
        float packetSuccessRate = 0.0;
        if (healthMetrics_.totalPacketsSent > 0) {
            packetSuccessRate = 1.0 - (float)healthMetrics_.packetsDropped / healthMetrics_.totalPacketsSent;
        }
        
        float routeQuality = 0.0;
        if (!routingTable_.empty()) {
            float totalReliability = 0.0;
            for (const auto& route : routingTable_) {
                totalReliability += route.second.reliability;
            }
            routeQuality = totalReliability / routingTable_.size();
        }
        
        float connectivityScore = min(1.0f, (float)topology_.neighbors.size() / 5.0f);
        
        topology_.networkHealth = (packetSuccessRate * 0.4 + routeQuality * 0.4 + connectivityScore * 0.2);
    }
};

// ===========================
// UTILITY FUNCTION IMPLEMENTATIONS
// ===========================

float calculateRouteReliability(int16_t rssi, float packetLoss) {
    // Convert RSSI to reliability score (assuming -30 to -120 dBm range)
    float rssiScore = max(0.0f, (rssi + 120.0f) / 90.0f);
    float lossScore = 1.0f - packetLoss;
    
    return (rssiScore * 0.6f + lossScore * 0.4f);
}

uint8_t calculateOptimalHops(uint8_t nodeCount, float networkDensity) {
    if (nodeCount < 5) return 2;
    if (nodeCount < 20) return 3;
    if (nodeCount < 50) return 4;
    return min(6, (int)(log(nodeCount) / log(2)) + 1);
}

uint32_t generatePacketId() {
    static uint32_t packetCounter = 0;
    return (millis() << 16) | (++packetCounter & 0xFFFF);
}

bool validatePacket(const uint8_t* packet, size_t length) {
    if (length < 4) return false; // Minimum packet size
    if (length > 255) return false; // Maximum LoRa packet size
    
    // Simple checksum validation
    uint8_t checksum = 0;
    for (size_t i = 0; i < length - 1; i++) {
        checksum ^= packet[i];
    }
    
    return checksum == packet[length - 1];
}

size_t compressWildlifeData(const WildlifeDetection& data, uint8_t* buffer, size_t bufferSize) {
    if (bufferSize < 16) return 0; // Minimum required size
    
    // Simple compression: pack essential data into binary format
    size_t offset = 0;
    
    // Timestamp (4 bytes)
    memcpy(buffer + offset, &data.timestamp, 4);
    offset += 4;
    
    // Species ID (1 byte)
    buffer[offset++] = data.speciesId;
    
    // Confidence (1 byte, scaled to 0-255)
    buffer[offset++] = (uint8_t)(data.confidence * 255);
    
    // Bounding box (8 bytes)
    memcpy(buffer + offset, data.boundingBox, 8);
    offset += 8;
    
    // Behavior code (1 byte)
    buffer[offset++] = data.behaviorCode;
    
    // Image ID (2 bytes)
    memcpy(buffer + offset, &data.imageId, 2);
    offset += 2;
    
    return offset;
}

bool decompressWildlifeData(const uint8_t* buffer, size_t length, WildlifeDetection& data) {
    if (length < 16) return false;
    
    size_t offset = 0;
    
    // Timestamp
    memcpy(&data.timestamp, buffer + offset, 4);
    offset += 4;
    
    // Species ID
    data.speciesId = buffer[offset++];
    
    // Confidence
    data.confidence = buffer[offset++] / 255.0f;
    
    // Bounding box
    memcpy(data.boundingBox, buffer + offset, 8);
    offset += 8;
    
    // Behavior code
    data.behaviorCode = buffer[offset++];
    
    // Image ID
    memcpy(&data.imageId, buffer + offset, 2);
    offset += 2;
    
    return true;
}

// Global instance
static EnhancedLoRaMesh g_enhancedLoRaMesh;

// ===========================
// PUBLIC API FUNCTIONS
// ===========================

namespace MeshNetworkV2 {
    bool initialize() {
        return g_enhancedLoRaMesh.initializeNetwork();
    }
    
    bool transmitWildlifeDetection(const WildlifeDetection& detection) {
        return g_enhancedLoRaMesh.transmitWildlifeData(detection);
    }
    
    bool transmitEnvironmentalData(const EnvironmentalData& envData) {
        return g_enhancedLoRaMesh.transmitEnvironmentalData(envData);
    }
    
    bool optimizeNetwork() {
        return g_enhancedLoRaMesh.optimizeRoutes() && 
               g_enhancedLoRaMesh.balanceNetworkLoad();
    }
    
    std::vector<NodeInfoV2> getActiveNodes() {
        return g_enhancedLoRaMesh.getActiveNodes();
    }
    
    NetworkHealthMetrics getNetworkHealth() {
        return g_enhancedLoRaMesh.getNetworkHealth();
    }
    
    bool synchronizeTime() {
        return g_enhancedLoRaMesh.synchronizeTime();
    }
}