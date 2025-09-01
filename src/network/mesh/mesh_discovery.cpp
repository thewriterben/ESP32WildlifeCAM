/**
 * @file mesh_discovery.cpp
 * @brief Node Discovery and Network Joining for Phase 2
 * 
 * This implementation handles automatic node discovery, network joining,
 * and neighbor management for the enhanced mesh network.
 * 
 * Phase 2 Enhancement - March 31, 2026 Target
 */

#include "../../../include/network/mesh_protocol.h"
#include <ArduinoJson.h>
#include <set>
#include <algorithm>

// ===========================
// MESH DISCOVERY ENGINE
// ===========================

class MeshDiscoveryEngine {
private:
    std::map<uint32_t, NodeInfoV2> discoveredNodes_;
    std::set<uint32_t> activeNeighbors_;
    std::map<uint32_t, uint32_t> lastHeartbeat_;
    
    uint32_t nodeId_;
    uint32_t discoverySequence_;
    uint32_t lastDiscoveryBroadcast_;
    uint32_t lastNeighborCheck_;
    
    // Discovery parameters
    uint32_t discoveryInterval_;
    uint32_t heartbeatInterval_;
    uint32_t neighborTimeout_;
    uint8_t maxDiscoveryAttempts_;
    
    // Network join state
    bool isJoined_;
    uint32_t coordinatorId_;
    uint32_t joinAttempts_;
    uint32_t lastJoinAttempt_;
    
public:
    MeshDiscoveryEngine(uint32_t nodeId) : 
        nodeId_(nodeId), discoverySequence_(0), lastDiscoveryBroadcast_(0),
        lastNeighborCheck_(0), discoveryInterval_(30000), heartbeatInterval_(60000),
        neighborTimeout_(180000), maxDiscoveryAttempts_(5), isJoined_(false),
        coordinatorId_(0), joinAttempts_(0), lastJoinAttempt_(0) {
    }
    
    // ===========================
    // NODE DISCOVERY
    // ===========================
    
    /**
     * Start network discovery process
     */
    bool startDiscovery() {
        uint32_t currentTime = millis();
        
        // Rate limiting: don't discover too frequently
        if (currentTime - lastDiscoveryBroadcast_ < discoveryInterval_) {
            return true;
        }
        
        // Create discovery beacon
        DynamicJsonDocument beacon(1024);
        beacon["type"] = "discovery_beacon";
        beacon["node_id"] = nodeId_;
        beacon["sequence"] = ++discoverySequence_;
        beacon["timestamp"] = currentTime;
        
        // Node capabilities and status
        beacon["capabilities"] = getNodeCapabilities();
        beacon["battery_level"] = getBatteryLevel();
        beacon["memory_free"] = ESP.getFreeHeap() / 1024;
        beacon["uptime"] = currentTime / 1000;
        beacon["temperature"] = getNodeTemperature();
        
        // Network status
        beacon["is_joined"] = isJoined_;
        beacon["coordinator"] = coordinatorId_;
        beacon["neighbor_count"] = activeNeighbors_.size();
        
        // Add neighbor list for topology discovery
        JsonArray neighbors = beacon.createNestedArray("neighbors");
        for (uint32_t neighbor : activeNeighbors_) {
            neighbors.add(neighbor);
        }
        
        // Signal quality and network metrics
        beacon["signal_quality"] = getSignalQuality();
        beacon["network_load"] = getNetworkLoad();
        beacon["packet_loss"] = getPacketLossRate();
        
        String message;
        serializeJson(beacon, message);
        
        Serial.printf("Broadcasting discovery beacon (seq: %d, neighbors: %d)\n", 
                     discoverySequence_, activeNeighbors_.size());
        
        lastDiscoveryBroadcast_ = currentTime;
        return broadcastMessage(message);
    }
    
    /**
     * Process received discovery beacon
     */
    bool processDiscoveryBeacon(const JsonObject& beacon) {
        uint32_t sourceNode = beacon["node_id"];
        uint32_t timestamp = beacon["timestamp"];
        
        // Don't process our own beacons
        if (sourceNode == nodeId_) return false;
        
        // Update node information
        NodeInfoV2 nodeInfo;
        nodeInfo.nodeId = sourceNode;
        nodeInfo.capabilities = beacon["capabilities"];
        nodeInfo.batteryLevel = beacon["battery_level"];
        nodeInfo.signalQuality = beacon["signal_quality"];
        nodeInfo.uptime = beacon["uptime"];
        nodeInfo.temperature = beacon["temperature"];
        nodeInfo.memoryFree = beacon["memory_free"];
        nodeInfo.lastSeen = millis();
        nodeInfo.isCoordinator = (sourceNode == beacon["coordinator"].as<uint32_t>());
        
        // Add or update discovered node
        discoveredNodes_[sourceNode] = nodeInfo;
        
        // Add to active neighbors (direct communication)
        activeNeighbors_.insert(sourceNode);
        lastHeartbeat_[sourceNode] = millis();
        
        // Process neighbor topology information
        if (beacon.containsKey("neighbors")) {
            std::vector<uint32_t> sourceNeighbors;
            JsonArray neighbors = beacon["neighbors"];
            for (JsonVariant neighbor : neighbors) {
                sourceNeighbors.push_back(neighbor.as<uint32_t>());
            }
            updateTopology(sourceNode, sourceNeighbors);
        }
        
        Serial.printf("Discovered node %08X (battery: %d%%, neighbors: %d)\n", 
                     sourceNode, nodeInfo.batteryLevel, 
                     beacon["neighbor_count"].as<int>());
        
        // If we're not joined and this node has a coordinator, try to join
        if (!isJoined_ && beacon["is_joined"].as<bool>()) {
            uint32_t coordinator = beacon["coordinator"];
            if (coordinator != 0) {
                return initiateNetworkJoin(coordinator);
            }
        }
        
        return true;
    }
    
    /**
     * Send heartbeat to maintain neighbor relationships
     */
    bool sendHeartbeat() {
        uint32_t currentTime = millis();
        
        if (currentTime - lastNeighborCheck_ < heartbeatInterval_) {
            return true;
        }
        
        // Create heartbeat message
        DynamicJsonDocument heartbeat(512);
        heartbeat["type"] = "heartbeat";
        heartbeat["node_id"] = nodeId_;
        heartbeat["timestamp"] = currentTime;
        heartbeat["sequence"] = ++discoverySequence_;
        
        // Minimal status information
        heartbeat["battery"] = getBatteryLevel();
        heartbeat["signal_quality"] = getSignalQuality();
        heartbeat["neighbor_count"] = activeNeighbors_.size();
        heartbeat["is_coordinator"] = (nodeId_ == coordinatorId_);
        
        String message;
        serializeJson(heartbeat, message);
        
        lastNeighborCheck_ = currentTime;
        return broadcastMessage(message);
    }
    
    /**
     * Process received heartbeat
     */
    bool processHeartbeat(const JsonObject& heartbeat) {
        uint32_t sourceNode = heartbeat["node_id"];
        
        if (sourceNode == nodeId_) return false;
        
        // Update last seen time
        lastHeartbeat_[sourceNode] = millis();
        
        // Update node info if it exists
        auto nodeIt = discoveredNodes_.find(sourceNode);
        if (nodeIt != discoveredNodes_.end()) {
            nodeIt->second.batteryLevel = heartbeat["battery"];
            nodeIt->second.signalQuality = heartbeat["signal_quality"];
            nodeIt->second.lastSeen = millis();
        }
        
        // Ensure it's in active neighbors
        activeNeighbors_.insert(sourceNode);
        
        return true;
    }
    
    // ===========================
    // NETWORK JOINING
    // ===========================
    
    /**
     * Initiate network join process
     */
    bool initiateNetworkJoin(uint32_t coordinatorId) {
        uint32_t currentTime = millis();
        
        // Rate limiting for join attempts
        if (currentTime - lastJoinAttempt_ < 30000) { // 30 seconds between attempts
            return false;
        }
        
        if (joinAttempts_ >= maxDiscoveryAttempts_) {
            Serial.println("Max join attempts reached, becoming coordinator");
            return becomeCoordinator();
        }
        
        // Create join request
        DynamicJsonDocument joinReq(512);
        joinReq["type"] = "join_request";
        joinReq["node_id"] = nodeId_;
        joinReq["coordinator"] = coordinatorId;
        joinReq["timestamp"] = currentTime;
        joinReq["attempt"] = ++joinAttempts_;
        
        // Node information for coordinator
        joinReq["capabilities"] = getNodeCapabilities();
        joinReq["battery_level"] = getBatteryLevel();
        joinReq["signal_quality"] = getSignalQuality();
        joinReq["memory_free"] = ESP.getFreeHeap() / 1024;
        
        String message;
        serializeJson(joinReq, message);
        
        lastJoinAttempt_ = currentTime;
        
        Serial.printf("Requesting to join network via coordinator %08X (attempt %d)\n", 
                     coordinatorId, joinAttempts_);
        
        return sendToNode(coordinatorId, message);
    }
    
    /**
     * Process join request (coordinator only)
     */
    bool processJoinRequest(const JsonObject& joinReq) {
        if (!isCoordinator()) return false;
        
        uint32_t requestingNode = joinReq["node_id"];
        uint32_t timestamp = joinReq["timestamp"];
        
        // Validate join request
        if (requestingNode == nodeId_) return false;
        
        // Check if we have capacity for new nodes
        if (discoveredNodes_.size() >= MESH_MAX_NODES - 1) {
            return sendJoinReply(requestingNode, false, "Network full");
        }
        
        // Add node to network
        NodeInfoV2 newNode;
        newNode.nodeId = requestingNode;
        newNode.capabilities = joinReq["capabilities"];
        newNode.batteryLevel = joinReq["battery_level"];
        newNode.signalQuality = joinReq["signal_quality"];
        newNode.memoryFree = joinReq["memory_free"];
        newNode.lastSeen = millis();
        newNode.isCoordinator = false;
        
        discoveredNodes_[requestingNode] = newNode;
        activeNeighbors_.insert(requestingNode);
        
        Serial.printf("Accepting join request from node %08X\n", requestingNode);
        
        return sendJoinReply(requestingNode, true, "Welcome to network");
    }
    
    /**
     * Send join reply
     */
    bool sendJoinReply(uint32_t requestingNode, bool accepted, const char* message) {
        DynamicJsonDocument reply(512);
        reply["type"] = "join_reply";
        reply["coordinator"] = nodeId_;
        reply["target"] = requestingNode;
        reply["accepted"] = accepted;
        reply["message"] = message;
        reply["timestamp"] = millis();
        
        if (accepted) {
            // Provide network information
            reply["network_id"] = nodeId_; // Use coordinator ID as network ID
            reply["node_count"] = discoveredNodes_.size();
            reply["heartbeat_interval"] = heartbeatInterval_;
            reply["discovery_interval"] = discoveryInterval_;
            
            // Add current neighbor list
            JsonArray neighbors = reply.createNestedArray("current_neighbors");
            for (uint32_t neighbor : activeNeighbors_) {
                if (neighbor != requestingNode) {
                    neighbors.add(neighbor);
                }
            }
        }
        
        String replyMessage;
        serializeJson(reply, replyMessage);
        
        return sendToNode(requestingNode, replyMessage);
    }
    
    /**
     * Process join reply
     */
    bool processJoinReply(const JsonObject& reply) {
        bool accepted = reply["accepted"];
        uint32_t coordinator = reply["coordinator"];
        
        if (accepted) {
            isJoined_ = true;
            coordinatorId_ = coordinator;
            joinAttempts_ = 0;
            
            // Update network parameters from coordinator
            if (reply.containsKey("heartbeat_interval")) {
                heartbeatInterval_ = reply["heartbeat_interval"];
            }
            if (reply.containsKey("discovery_interval")) {
                discoveryInterval_ = reply["discovery_interval"];
            }
            
            // Add coordinator's neighbors to our discovery
            if (reply.containsKey("current_neighbors")) {
                JsonArray neighbors = reply["current_neighbors"];
                for (JsonVariant neighbor : neighbors) {
                    uint32_t neighborId = neighbor.as<uint32_t>();
                    // These are potential neighbors to discover
                    // We'll discover them through normal beacon process
                }
            }
            
            Serial.printf("Successfully joined network with coordinator %08X\n", coordinator);
            
            // Send our first heartbeat as network member
            return sendHeartbeat();
        } else {
            Serial.printf("Join request rejected: %s\n", reply["message"].as<const char*>());
            
            // Try to find another coordinator or become one
            return findAlternativeCoordinator();
        }
    }
    
    // ===========================
    // COORDINATOR ELECTION
    // ===========================
    
    /**
     * Become network coordinator
     */
    bool becomeCoordinator() {
        coordinatorId_ = nodeId_;
        isJoined_ = true;
        joinAttempts_ = 0;
        
        // Announce coordinator role
        DynamicJsonDocument announcement(512);
        announcement["type"] = "coordinator_announcement";
        announcement["coordinator"] = nodeId_;
        announcement["network_id"] = nodeId_;
        announcement["timestamp"] = millis();
        announcement["capabilities"] = getNodeCapabilities();
        announcement["battery_level"] = getBatteryLevel();
        
        String message;
        serializeJson(announcement, message);
        
        Serial.printf("Becoming network coordinator %08X\n", nodeId_);
        
        return broadcastMessage(message);
    }
    
    /**
     * Process coordinator announcement
     */
    bool processCoordinatorAnnouncement(const JsonObject& announcement) {
        uint32_t newCoordinator = announcement["coordinator"];
        
        if (newCoordinator == nodeId_) return false;
        
        // If we don't have a coordinator or this is a better one
        if (coordinatorId_ == 0 || shouldAcceptNewCoordinator(newCoordinator)) {
            coordinatorId_ = newCoordinator;
            
            if (!isJoined_) {
                // Try to join this coordinator's network
                return initiateNetworkJoin(newCoordinator);
            }
        }
        
        return true;
    }
    
    // ===========================
    // NEIGHBOR MANAGEMENT
    // ===========================
    
    /**
     * Update neighbor timeout checking
     */
    void updateNeighborStatus() {
        uint32_t currentTime = millis();
        
        // Remove neighbors that haven't been heard from
        auto it = activeNeighbors_.begin();
        while (it != activeNeighbors_.end()) {
            auto heartbeatIt = lastHeartbeat_.find(*it);
            if (heartbeatIt != lastHeartbeat_.end() && 
                currentTime - heartbeatIt->second > neighborTimeout_) {
                
                Serial.printf("Neighbor %08X timed out\n", *it);
                
                // Remove from discovered nodes as well
                discoveredNodes_.erase(*it);
                lastHeartbeat_.erase(heartbeatIt);
                it = activeNeighbors_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    /**
     * Get list of active neighbors
     */
    std::vector<uint32_t> getActiveNeighbors() const {
        return std::vector<uint32_t>(activeNeighbors_.begin(), activeNeighbors_.end());
    }
    
    /**
     * Get discovered node information
     */
    std::vector<NodeInfoV2> getDiscoveredNodes() const {
        std::vector<NodeInfoV2> nodes;
        for (const auto& pair : discoveredNodes_) {
            nodes.push_back(pair.second);
        }
        return nodes;
    }
    
    /**
     * Get node information by ID
     */
    NodeInfoV2* getNodeInfo(uint32_t nodeId) {
        auto it = discoveredNodes_.find(nodeId);
        return (it != discoveredNodes_.end()) ? &it->second : nullptr;
    }
    
    /**
     * Check if node is coordinator
     */
    bool isCoordinator() const {
        return coordinatorId_ == nodeId_;
    }
    
    /**
     * Check if joined to network
     */
    bool isNetworkJoined() const {
        return isJoined_;
    }
    
    /**
     * Get coordinator ID
     */
    uint32_t getCoordinatorId() const {
        return coordinatorId_;
    }
    
    /**
     * Get discovery statistics
     */
    void getDiscoveryStatistics(uint32_t& totalNodes, uint32_t& activeNeighbors, 
                               uint32_t& joinAttempts, bool& isJoined) const {
        totalNodes = discoveredNodes_.size();
        activeNeighbors = activeNeighbors_.size();
        joinAttempts = joinAttempts_;
        isJoined = isJoined_;
    }
    
private:
    // ===========================
    // HELPER FUNCTIONS
    // ===========================
    
    uint16_t getNodeCapabilities() const {
        return CAP_BASIC_CAMERA | CAP_AI_PROCESSING | CAP_ENVIRONMENTAL_SENSORS;
    }
    
    uint8_t getBatteryLevel() const {
        // TODO: Implement actual battery level reading
        return 85;
    }
    
    uint8_t getSignalQuality() const {
        // TODO: Implement actual signal quality measurement
        return 75;
    }
    
    float getNodeTemperature() const {
        // TODO: Implement actual temperature reading
        return 25.0;
    }
    
    uint8_t getNetworkLoad() const {
        // TODO: Implement actual network load calculation
        return 30;
    }
    
    float getPacketLossRate() const {
        // TODO: Implement actual packet loss calculation
        return 0.05; // 5% packet loss
    }
    
    bool shouldAcceptNewCoordinator(uint32_t newCoordinator) const {
        // Simple coordinator selection: prefer node with higher ID (arbitrary)
        // In practice, this could consider battery level, capabilities, etc.
        return newCoordinator > coordinatorId_;
    }
    
    bool findAlternativeCoordinator() {
        // Look for another coordinator in discovered nodes
        for (const auto& node : discoveredNodes_) {
            if (node.second.isCoordinator && node.first != coordinatorId_) {
                return initiateNetworkJoin(node.first);
            }
        }
        
        // No alternative found, become coordinator ourselves
        return becomeCoordinator();
    }
    
    void updateTopology(uint32_t nodeId, const std::vector<uint32_t>& neighbors) {
        // This would interface with the routing engine
        // For now, just log the topology update
        Serial.printf("Topology update: Node %08X has %d neighbors\n", 
                     nodeId, neighbors.size());
    }
    
    bool broadcastMessage(const String& message) {
        // Interface with actual LoRa transmission
        return true;
    }
    
    bool sendToNode(uint32_t nodeId, const String& message) {
        // Interface with routing to send to specific node
        return true;
    }
};

// ===========================
// PUBLIC API
// ===========================

static MeshDiscoveryEngine* g_discoveryEngine = nullptr;

namespace MeshDiscovery {
    bool initialize(uint32_t nodeId) {
        if (g_discoveryEngine) {
            delete g_discoveryEngine;
        }
        
        g_discoveryEngine = new MeshDiscoveryEngine(nodeId);
        Serial.printf("Mesh discovery engine initialized for node %08X\n", nodeId);
        return true;
    }
    
    void cleanup() {
        if (g_discoveryEngine) {
            delete g_discoveryEngine;
            g_discoveryEngine = nullptr;
        }
    }
    
    bool startDiscovery() {
        return g_discoveryEngine ? g_discoveryEngine->startDiscovery() : false;
    }
    
    bool sendHeartbeat() {
        return g_discoveryEngine ? g_discoveryEngine->sendHeartbeat() : false;
    }
    
    void updateNeighborStatus() {
        if (g_discoveryEngine) {
            g_discoveryEngine->updateNeighborStatus();
        }
    }
    
    std::vector<uint32_t> getActiveNeighbors() {
        return g_discoveryEngine ? g_discoveryEngine->getActiveNeighbors() : std::vector<uint32_t>();
    }
    
    std::vector<NodeInfoV2> getDiscoveredNodes() {
        return g_discoveryEngine ? g_discoveryEngine->getDiscoveredNodes() : std::vector<NodeInfoV2>();
    }
    
    NodeInfoV2* getNodeInfo(uint32_t nodeId) {
        return g_discoveryEngine ? g_discoveryEngine->getNodeInfo(nodeId) : nullptr;
    }
    
    bool isCoordinator() {
        return g_discoveryEngine ? g_discoveryEngine->isCoordinator() : false;
    }
    
    bool isNetworkJoined() {
        return g_discoveryEngine ? g_discoveryEngine->isNetworkJoined() : false;
    }
    
    uint32_t getCoordinatorId() {
        return g_discoveryEngine ? g_discoveryEngine->getCoordinatorId() : 0;
    }
    
    void getStatistics(uint32_t& totalNodes, uint32_t& activeNeighbors, 
                      uint32_t& joinAttempts, bool& isJoined) {
        if (g_discoveryEngine) {
            g_discoveryEngine->getDiscoveryStatistics(totalNodes, activeNeighbors, 
                                                    joinAttempts, isJoined);
        } else {
            totalNodes = 0;
            activeNeighbors = 0;
            joinAttempts = 0;
            isJoined = false;
        }
    }
}