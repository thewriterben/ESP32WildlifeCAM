/**
 * Enhanced LoRa Mesh Network Implementation
 * Phase 2 Enhancement - Multi-hop LoRa mesh protocol implementation
 */

#include "lora_mesh.h"
#include <algorithm>

// Global instance
EnhancedLoraMesh meshNetwork;

// Callback function pointers
static MeshMessageCallback g_messageCallback = nullptr;
static MeshNetworkEventCallback g_networkEventCallback = nullptr;
static MeshRouteChangeCallback g_routeChangeCallback = nullptr;

// Constructor
EnhancedLoraMesh::EnhancedLoraMesh() 
    : nextMessageId_(1), networkLoad_(0.0), lastOptimization_(0),
      lastDiscovery_(0), lastHeartbeat_(0), lastTransmission_(0),
      backoffTime_(100), retryCount_(0), initialized_(false) {
}

// Destructor
EnhancedLoraMesh::~EnhancedLoraMesh() {
    cleanup();
}

// Initialize the enhanced mesh network
bool EnhancedLoraMesh::init(const MeshConfiguration& config) {
    if (initialized_) {
        return true;
    }
    
    config_ = config;
    
    // Initialize LoRa radio
    if (!LoRa.begin(915E6)) {  // 915 MHz for North America
        Serial.println("Starting LoRa failed!");
        return false;
    }
    
    // Configure LoRa parameters for mesh networking
    LoRa.setSpreadingFactor(7);  // SF7 for better data rate
    LoRa.setSignalBandwidth(125E3);  // 125 kHz bandwidth
    LoRa.setCodingRate4(5);  // 4/5 coding rate
    LoRa.setSyncWord(0x34);  // Sync word for mesh network
    LoRa.setPreambleLength(8);
    
    // Enable CRC and set TX power
    LoRa.enableCrc();
    LoRa.setTxPower(20);  // Maximum power for long range
    
    // Set receive callback
    LoRa.onReceive([](int packetSize) {
        if (packetSize > 0) {
            String message = "";
            while (LoRa.available()) {
                message += (char)LoRa.read();
            }
            meshNetwork.handleIncomingMessage(message, LoRa.packetRssi(), LoRa.packetSnr());
        }
    });
    
    // Start receiving
    LoRa.receive();
    
    // Initialize network statistics
    stats_ = MeshNetworkStats();
    stats_.totalNodes = 1;  // This node
    stats_.activeNodes = 1;
    
    // Initialize time sync
    timeSync_ = MeshTimeSync();
    timeSync_.localTime = millis();
    
    // Add this node to the node table
    MeshNodeInfo thisNode;
    thisNode.nodeId = config_.nodeId;
    thisNode.deviceName = "WildlifeCam_" + String(config_.nodeId);
    thisNode.capabilities = NODE_CAP_BASIC | NODE_CAP_IMAGE | NODE_CAP_SENSORS;
    thisNode.batteryLevel = 100.0;  // Will be updated by power management
    thisNode.signalStrength = 0;
    thisNode.lastSeen = millis();
    thisNode.firmwareVersion = "2.0.0";
    
    nodeTable_[config_.nodeId] = thisNode;
    activeNodes_.push_back(config_.nodeId);
    
    initialized_ = true;
    
    Serial.println("Enhanced LoRa Mesh initialized successfully");
    Serial.println("Node ID: " + String(config_.nodeId));
    Serial.println("Network: " + config_.networkName);
    
    // Start initial network discovery
    performNetworkDiscovery();
    
    return true;
}

// Cleanup mesh resources
void EnhancedLoraMesh::cleanup() {
    if (!initialized_) return;
    
    LoRa.end();
    nodeTable_.clear();
    routingTable_.clear();
    activeNodes_.clear();
    messageQueue_.clear();
    pendingAcks_.clear();
    
    initialized_ = false;
}

// Process incoming and outgoing messages
void EnhancedLoraMesh::processMessages() {
    if (!initialized_) return;
    
    uint32_t now = millis();
    
    // Send periodic heartbeat
    if (now - lastHeartbeat_ > MESH_HEARTBEAT_INTERVAL) {
        sendHeartbeat();
        lastHeartbeat_ = now;
    }
    
    // Perform periodic network discovery
    if (now - lastDiscovery_ > MESH_DISCOVERY_INTERVAL) {
        performNetworkDiscovery();
        lastDiscovery_ = now;
    }
    
    // Optimize network topology periodically
    if (now - lastOptimization_ > 300000) {  // 5 minutes
        optimizeNetworkTopology();
        lastOptimization_ = now;
    }
    
    // Process outgoing message queue
    if (!messageQueue_.empty() && (now - lastTransmission_ > backoffTime_)) {
        if (isChannelClear()) {
            String message = messageQueue_.front();
            messageQueue_.erase(messageQueue_.begin());
            
            LoRa.beginPacket();
            LoRa.print(message);
            LoRa.endPacket(true);  // Async transmission
            
            lastTransmission_ = now;
            backoffTime_ = calculateBackoffTime();
            retryCount_ = 0;
            
            stats_.messagesSent++;
        } else {
            // Channel busy, increase backoff
            backoffTime_ = min(backoffTime_ * 2, 2000U);
        }
    }
    
    // Clean up stale routes and nodes
    if (now % 60000 == 0) {  // Every minute
        cleanupStaleRoutes();
    }
    
    // Handle pending acknowledgments timeout
    for (auto it = pendingAcks_.begin(); it != pendingAcks_.end();) {
        if (now - it->second > 30000) {  // 30 second timeout
            it = pendingAcks_.erase(it);
        } else {
            ++it;
        }
    }
    
    // Synchronize time if enabled
    if (config_.timeSync && (now % MESH_SYNC_INTERVAL == 0)) {
        synchronizeTime();
    }
}

// Send a message to a specific node
bool EnhancedLoraMesh::sendMessage(uint32_t destNode, const String& payload, uint8_t messageType) {
    if (!initialized_) return false;
    
    // Create message header
    MeshMessageHeader header;
    header.messageType = messageType;
    header.sourceNode = config_.nodeId;
    header.destNode = destNode;
    header.messageId = nextMessageId_++;
    header.timestamp = getNetworkTime();
    header.payloadSize = payload.length();
    header.checksum = calculateChecksum(header);
    
    // Create JSON payload
    DynamicJsonDocument doc(1024);
    doc["data"] = payload;
    
    String message = createMeshMessage(header, doc.as<JsonObject>());
    
    if (message.length() > config_.maxMessageSize) {
        Serial.println("Message too large: " + String(message.length()) + " bytes");
        return false;
    }
    
    // Add to queue or send immediately
    if (isChannelClear() && messageQueue_.empty()) {
        LoRa.beginPacket();
        LoRa.print(message);
        LoRa.endPacket(true);
        
        lastTransmission_ = millis();
        stats_.messagesSent++;
        
        // Track acknowledgment if required
        if (config_.qualityOfService > 0) {
            pendingAcks_[header.messageId] = millis();
        }
        
        return true;
    } else {
        return queueMessage(destNode, payload, messageType);
    }
}

// Broadcast message to all nodes
bool EnhancedLoraMesh::broadcastMessage(const String& payload, uint8_t messageType) {
    return sendMessage(0, payload, messageType);  // 0 = broadcast
}

// Queue message for later transmission
bool EnhancedLoraMesh::queueMessage(uint32_t destNode, const String& payload, uint8_t messageType) {
    if (messageQueue_.size() >= 10) {  // Queue size limit
        messageQueue_.erase(messageQueue_.begin());  // Remove oldest
        stats_.messagesDropped++;
    }
    
    // Create message header
    MeshMessageHeader header;
    header.messageType = messageType;
    header.sourceNode = config_.nodeId;
    header.destNode = destNode;
    header.messageId = nextMessageId_++;
    header.timestamp = getNetworkTime();
    header.payloadSize = payload.length();
    header.checksum = calculateChecksum(header);
    
    // Create JSON payload
    DynamicJsonDocument doc(1024);
    doc["data"] = payload;
    
    String message = createMeshMessage(header, doc.as<JsonObject>());
    messageQueue_.push_back(message);
    
    return true;
}

// Handle incoming messages
void EnhancedLoraMesh::handleIncomingMessage(const String& rawMessage, int rssi, float snr) {
    MeshMessageHeader header;
    DynamicJsonDocument doc(1024);
    
    if (!parseMeshMessage(rawMessage, header, doc.as<JsonObject>())) {
        stats_.messagesDropped++;
        return;
    }
    
    stats_.messagesReceived++;
    
    // Update routing table based on source
    updateRoutingTable(header, rssi, snr);
    
    // Process based on message type
    switch (header.messageType) {
        case MESH_MSG_HEARTBEAT:
            processHeartbeat(header, doc.as<JsonObject>());
            break;
        case MESH_MSG_DISCOVERY:
            processDiscovery(header, doc.as<JsonObject>());
            break;
        case MESH_MSG_ROUTE_REQUEST:
            processRouteRequest(header, doc.as<JsonObject>());
            break;
        case MESH_MSG_ROUTE_REPLY:
            processRouteReply(header, doc.as<JsonObject>());
            break;
        case MESH_MSG_DATA:
            processDataMessage(header, doc.as<JsonObject>());
            break;
        case MESH_MSG_TIME_SYNC:
            processTimeSync(header, doc.as<JsonObject>());
            break;
        case MESH_MSG_ACK:
            processAcknowledgment(header, doc.as<JsonObject>());
            break;
        default:
            Serial.println("Unknown message type: " + String(header.messageType));
            break;
    }
}

// Calculate backoff time for collision avoidance
uint32_t EnhancedLoraMesh::calculateBackoffTime() {
    // Exponential backoff with jitter
    uint32_t base = 100 + (retryCount_ * 50);
    uint32_t jitter = random(0, base / 4);
    return min(base + jitter, 2000U);
}

// Check if channel is clear for transmission
bool EnhancedLoraMesh::isChannelClear() {
    // Simple channel assessment - check if we're not receiving
    return !LoRa.parsePacket();
}

// Send heartbeat message
void EnhancedLoraMesh::sendHeartbeat() {
    DynamicJsonDocument doc(512);
    doc["node_id"] = config_.nodeId;
    doc["battery"] = 85.0;  // Should be read from power management
    doc["uptime"] = millis();
    doc["capabilities"] = NODE_CAP_BASIC | NODE_CAP_IMAGE | NODE_CAP_SENSORS;
    doc["neighbors"] = activeNodes_.size() - 1;  // Exclude self
    
    broadcastMessage("", MESH_MSG_HEARTBEAT);
}

// Perform network discovery
void EnhancedLoraMesh::performNetworkDiscovery() {
    DynamicJsonDocument doc(256);
    doc["requester"] = config_.nodeId;
    doc["timestamp"] = getNetworkTime();
    
    broadcastMessage("", MESH_MSG_DISCOVERY);
    stats_.routeDiscoveries++;
}

// Get network time (synchronized if available)
uint32_t EnhancedLoraMesh::getNetworkTime() const {
    if (isTimeSynchronized()) {
        return timeSync_.referenceTime + (millis() - timeSync_.localTime) + timeSync_.timeOffset;
    }
    return millis();
}

// Check if time is synchronized
bool EnhancedLoraMesh::isTimeSynchronized() const {
    return timeSync_.syncAccuracy > 50 && 
           (millis() - timeSync_.lastSyncTime) < 600000;  // 10 minutes
}

// Create mesh message string
String EnhancedLoraMesh::createMeshMessage(const MeshMessageHeader& header, const JsonObject& payload) {
    DynamicJsonDocument doc(1024);
    
    // Add header
    doc["v"] = header.version;
    doc["t"] = header.messageType;
    doc["s"] = header.sourceNode;
    doc["d"] = header.destNode;
    doc["id"] = header.messageId;
    doc["hc"] = header.hopCount;
    doc["mh"] = header.maxHops;
    doc["ts"] = header.timestamp;
    doc["ps"] = header.payloadSize;
    doc["cs"] = header.checksum;
    
    // Add payload
    doc["payload"] = payload;
    
    String result;
    serializeJson(doc, result);
    return result;
}

// Parse mesh message from string
bool EnhancedLoraMesh::parseMeshMessage(const String& rawMessage, MeshMessageHeader& header, JsonObject& payload) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, rawMessage);
    
    if (error) {
        return false;
    }
    
    // Parse header
    header.version = doc["v"] | 2;
    header.messageType = doc["t"] | MESH_MSG_DATA;
    header.sourceNode = doc["s"] | 0;
    header.destNode = doc["d"] | 0;
    header.messageId = doc["id"] | 0;
    header.hopCount = doc["hc"] | 0;
    header.maxHops = doc["mh"] | MAX_HOP_COUNT;
    header.timestamp = doc["ts"] | 0;
    header.payloadSize = doc["ps"] | 0;
    header.checksum = doc["cs"] | 0;
    
    // Validate checksum
    uint16_t calculatedChecksum = calculateChecksum(header);
    if (calculatedChecksum != header.checksum) {
        return false;
    }
    
    // Extract payload
    payload = doc["payload"].as<JsonObject>();
    
    return true;
}

// Calculate header checksum
uint16_t EnhancedLoraMesh::calculateChecksum(const MeshMessageHeader& header) {
    uint16_t checksum = 0;
    checksum ^= header.version;
    checksum ^= header.messageType;
    checksum ^= (header.sourceNode & 0xFFFF);
    checksum ^= (header.sourceNode >> 16);
    checksum ^= (header.destNode & 0xFFFF);
    checksum ^= (header.destNode >> 16);
    checksum ^= header.messageId;
    checksum ^= header.hopCount;
    checksum ^= header.maxHops;
    checksum ^= header.payloadSize;
    return checksum;
}

// Process different message types (stub implementations)
void EnhancedLoraMesh::processHeartbeat(const MeshMessageHeader& header, const JsonObject& payload) {
    // Update node information
    MeshNodeInfo nodeInfo;
    nodeInfo.nodeId = header.sourceNode;
    nodeInfo.batteryLevel = payload["battery"] | 0.0;
    nodeInfo.uptime = payload["uptime"] | 0;
    nodeInfo.capabilities = payload["capabilities"] | NODE_CAP_BASIC;
    nodeInfo.lastSeen = millis();
    
    addNode(nodeInfo);
}

void EnhancedLoraMesh::processDiscovery(const MeshMessageHeader& header, const JsonObject& payload) {
    // Respond to discovery requests
    if (header.sourceNode != config_.nodeId) {
        DynamicJsonDocument response(256);
        response["responder"] = config_.nodeId;
        response["capabilities"] = NODE_CAP_BASIC | NODE_CAP_IMAGE | NODE_CAP_SENSORS;
        
        sendMessage(header.sourceNode, "", MESH_MSG_DISCOVERY);
    }
}

void EnhancedLoraMesh::processRouteRequest(const MeshMessageHeader& header, const JsonObject& payload) {
    // Handle route requests - implementation needed
}

void EnhancedLoraMesh::processRouteReply(const MeshMessageHeader& header, const JsonObject& payload) {
    // Handle route replies - implementation needed
}

void EnhancedLoraMesh::processDataMessage(const MeshMessageHeader& header, const JsonObject& payload) {
    if (header.destNode == config_.nodeId || header.destNode == 0) {
        // Message for this node
        if (g_messageCallback) {
            g_messageCallback(header.sourceNode, payload["data"] | "");
        }
        
        // Send acknowledgment if required
        if (config_.qualityOfService > 0) {
            sendAcknowledgment(header.sourceNode, header.messageId);
        }
    } else {
        // Forward message
        // Implementation needed for multi-hop forwarding
        stats_.messagesForwarded++;
    }
}

void EnhancedLoraMesh::processTimeSync(const MeshMessageHeader& header, const JsonObject& payload) {
    // Handle time synchronization - implementation needed
}

void EnhancedLoraMesh::processAcknowledgment(const MeshMessageHeader& header, const JsonObject& payload) {
    uint16_t messageId = payload["msg_id"] | 0;
    pendingAcks_.erase(messageId);
}

// Update routing table
void EnhancedLoraMesh::updateRoutingTable(const MeshMessageHeader& header, int rssi, float snr) {
    MeshRoute route;
    route.destination = header.sourceNode;
    route.nextHop = header.sourceNode;  // Direct route
    route.hopCount = 1;
    route.reliability = min(1.0f, (rssi + 100) / 100.0f);  // Convert RSSI to reliability
    route.lastUsed = millis();
    route.metric = (100 - rssi) + (header.hopCount * 10);  // Lower is better
    
    routingTable_[header.sourceNode] = route;
}

// Add node to network
void EnhancedLoraMesh::addNode(const MeshNodeInfo& nodeInfo) {
    if (nodeInfo.nodeId == config_.nodeId) return;  // Don't add self
    
    bool isNewNode = nodeTable_.find(nodeInfo.nodeId) == nodeTable_.end();
    
    nodeTable_[nodeInfo.nodeId] = nodeInfo;
    
    if (isNewNode) {
        activeNodes_.push_back(nodeInfo.nodeId);
        stats_.totalNodes++;
        stats_.activeNodes++;
        
        if (g_networkEventCallback) {
            g_networkEventCallback(nodeInfo.nodeId, true);
        }
    }
}

// Cleanup stale routes
void EnhancedLoraMesh::cleanupStaleRoutes() {
    uint32_t now = millis();
    
    for (auto it = routingTable_.begin(); it != routingTable_.end();) {
        if (now - it->second.lastUsed > MESH_ROUTE_TIMEOUT) {
            it = routingTable_.erase(it);
        } else {
            ++it;
        }
    }
    
    for (auto it = nodeTable_.begin(); it != nodeTable_.end();) {
        if (now - it->second.lastSeen > MESH_ROUTE_TIMEOUT) {
            // Remove from active nodes
            activeNodes_.erase(
                std::remove(activeNodes_.begin(), activeNodes_.end(), it->first),
                activeNodes_.end()
            );
            
            it = nodeTable_.erase(it);
            stats_.activeNodes--;
            
            if (g_networkEventCallback) {
                g_networkEventCallback(it->first, false);
            }
        } else {
            ++it;
        }
    }
}

// Send acknowledgment
void EnhancedLoraMesh::sendAcknowledgment(uint32_t destNode, uint16_t messageId) {
    DynamicJsonDocument doc(128);
    doc["msg_id"] = messageId;
    
    sendMessage(destNode, "", MESH_MSG_ACK);
}

// Optimize network topology (stub)
void EnhancedLoraMesh::optimizeNetworkTopology() {
    // Placeholder for topology optimization algorithms
    updateNetworkStatistics();
}

// Update network statistics
void EnhancedLoraMesh::updateNetworkStatistics() {
    stats_.activeNodes = activeNodes_.size();
    stats_.networkEfficiency = calculateNetworkEfficiency();
}

// Calculate network efficiency
float EnhancedLoraMesh::calculateNetworkEfficiency() {
    if (stats_.messagesSent == 0) return 0.0;
    
    float deliveryRate = 1.0f - (float(stats_.messagesDropped) / float(stats_.messagesSent + stats_.messagesDropped));
    float forwardingEfficiency = stats_.messagesForwarded > 0 ? 
        1.0f - (float(stats_.messagesDropped) / float(stats_.messagesForwarded)) : 1.0f;
    
    return (deliveryRate + forwardingEfficiency) / 2.0f;
}

// Synchronize time (stub)
void EnhancedLoraMesh::synchronizeTime() {
    // Placeholder for time synchronization implementation
}

// Get network status report
String EnhancedLoraMesh::getNetworkStatusReport() {
    DynamicJsonDocument doc(1024);
    
    doc["nodeId"] = config_.nodeId;
    doc["networkName"] = config_.networkName;
    doc["activeNodes"] = stats_.activeNodes;
    doc["messagesSent"] = stats_.messagesSent;
    doc["messagesReceived"] = stats_.messagesReceived;
    doc["messagesForwarded"] = stats_.messagesForwarded;
    doc["messagesDropped"] = stats_.messagesDropped;
    doc["networkEfficiency"] = stats_.networkEfficiency;
    doc["isTimeSynchronized"] = isTimeSynchronized();
    
    String result;
    serializeJson(doc, result);
    return result;
}

// Callback registration functions
void setMeshMessageCallback(MeshMessageCallback callback) {
    g_messageCallback = callback;
}

void setMeshNetworkEventCallback(MeshNetworkEventCallback callback) {
    g_networkEventCallback = callback;
}

void setMeshRouteChangeCallback(MeshRouteChangeCallback callback) {
    g_routeChangeCallback = callback;
}