/**
 * Mesh Discovery Implementation
 * Phase 2 Enhancement - Node discovery and network joining
 */

#include "mesh_discovery.h"
#include <ArduinoJson.h>

// Global discovery manager instance
MeshDiscoveryManager meshDiscovery;

// Callback function pointers
static NodeDiscoveredCallback g_nodeDiscoveredCallback = nullptr;
static NodeLostCallback g_nodeLostCallback = nullptr;
static TopologyChangedCallback g_topologyChangedCallback = nullptr;
static NetworkPartitionCallback g_networkPartitionCallback = nullptr;

MeshDiscoveryManager::MeshDiscoveryManager() 
    : localNodeId_(0), lastDiscoveryBroadcast_(0), lastTopologyUpdate_(0),
      discoveryInterval_(60000), beaconTimeout_(300000), discoveryEnabled_(true),
      broadcastBeacons_(true), respondToDiscovery_(true) {
}

MeshDiscoveryManager::~MeshDiscoveryManager() {
    cleanup();
}

bool MeshDiscoveryManager::init(uint32_t nodeId, const String& networkName) {
    localNodeId_ = nodeId;
    networkName_ = networkName;
    discoveryEnabled_ = true;
    broadcastBeacons_ = true;
    respondToDiscovery_ = true;
    
    lastDiscoveryBroadcast_ = millis();
    lastTopologyUpdate_ = millis();
    
    Serial.println("Mesh Discovery initialized for node " + String(nodeId) + " on network " + networkName);
    
    // Add self to topology
    DiscoveryBeacon selfBeacon = createDiscoveryBeacon();
    topology_.nodeInfo[nodeId] = selfBeacon;
    
    return true;
}

void MeshDiscoveryManager::cleanup() {
    topology_.nodeInfo.clear();
    topology_.adjacencyList.clear();
    discoveryQueue_.clear();
    discoveryAttempts_.clear();
    networkName_ = "";
}

void MeshDiscoveryManager::performActiveDiscovery() {
    if (!discoveryEnabled_) return;
    
    uint32_t now = millis();
    if (now - lastDiscoveryBroadcast_ < discoveryInterval_) {
        return;
    }
    
    broadcastDiscoveryBeacon();
    lastDiscoveryBroadcast_ = now;
    
    Serial.println("Performing active discovery - " + String(topology_.nodeInfo.size()) + " known nodes");
}

void MeshDiscoveryManager::broadcastDiscoveryBeacon() {
    if (!broadcastBeacons_) return;
    
    DiscoveryBeacon beacon = createDiscoveryBeacon();
    String beaconData = serializeBeacon(beacon);
    
    // This would trigger a broadcast message through the mesh network layer
    Serial.println("Broadcasting discovery beacon");
}

void MeshDiscoveryManager::processDiscoveryBeacon(const DiscoveryBeacon& beacon) {
    if (beacon.nodeId == localNodeId_) {
        return;  // Ignore our own beacon
    }
    
    bool isNewNode = topology_.nodeInfo.find(beacon.nodeId) == topology_.nodeInfo.end();
    
    // Update node information
    topology_.nodeInfo[beacon.nodeId] = beacon;
    
    if (isNewNode) {
        Serial.println("Discovered new node: " + String(beacon.nodeId) + " (" + beacon.deviceName + ")");
        
        if (g_nodeDiscoveredCallback) {
            MeshNodeInfo nodeInfo;
            nodeInfo.nodeId = beacon.nodeId;
            nodeInfo.deviceName = beacon.deviceName;
            nodeInfo.capabilities = beacon.capabilities;
            nodeInfo.batteryLevel = beacon.batteryLevel;
            nodeInfo.signalStrength = beacon.signalStrength;
            nodeInfo.signalToNoise = beacon.signalToNoise;
            nodeInfo.lastSeen = beacon.lastSeen;
            nodeInfo.uptime = beacon.uptime;
            nodeInfo.firmwareVersion = beacon.firmwareVersion;
            
            g_nodeDiscoveredCallback(beacon.nodeId, nodeInfo);
        }
    }
    
    // Update topology if this is a direct neighbor
    if (beacon.hopCount == 0 || beacon.hopCount == 1) {
        updateAdjacencyList(localNodeId_, beacon.nodeId);
    }
}

void MeshDiscoveryManager::addDiscoveredNode(const MeshNodeInfo& nodeInfo, int8_t rssi, float snr) {
    DiscoveryBeacon beacon;
    beacon.nodeId = nodeInfo.nodeId;
    beacon.deviceName = nodeInfo.deviceName;
    beacon.capabilities = nodeInfo.capabilities;
    beacon.batteryLevel = nodeInfo.batteryLevel;
    beacon.uptime = nodeInfo.uptime;
    beacon.firmwareVersion = nodeInfo.firmwareVersion;
    beacon.lastSeen = millis();
    beacon.signalStrength = rssi;
    beacon.signalToNoise = snr;
    beacon.hopCount = 1;  // Direct neighbor
    
    processDiscoveryBeacon(beacon);
}

void MeshDiscoveryManager::removeStaleNodes() {
    uint32_t now = millis();
    
    for (auto it = topology_.nodeInfo.begin(); it != topology_.nodeInfo.end();) {
        if (it->first != localNodeId_ && isBeaconStale(it->second)) {
            Serial.println("Removing stale node: " + String(it->first));
            
            uint32_t nodeId = it->first;
            it = topology_.nodeInfo.erase(it);
            
            // Remove from adjacency list
            topology_.adjacencyList.erase(nodeId);
            for (auto& adjPair : topology_.adjacencyList) {
                adjPair.second.erase(nodeId);
            }
            
            if (g_nodeLostCallback) {
                g_nodeLostCallback(nodeId);
            }
        } else {
            ++it;
        }
    }
}

std::vector<MeshNodeInfo> MeshDiscoveryManager::getDiscoveredNodes() const {
    std::vector<MeshNodeInfo> nodes;
    
    for (const auto& pair : topology_.nodeInfo) {
        if (pair.first != localNodeId_) {
            const DiscoveryBeacon& beacon = pair.second;
            
            MeshNodeInfo nodeInfo;
            nodeInfo.nodeId = beacon.nodeId;
            nodeInfo.deviceName = beacon.deviceName;
            nodeInfo.capabilities = beacon.capabilities;
            nodeInfo.batteryLevel = beacon.batteryLevel;
            nodeInfo.signalStrength = beacon.signalStrength;
            nodeInfo.signalToNoise = beacon.signalToNoise;
            nodeInfo.lastSeen = beacon.lastSeen;
            nodeInfo.uptime = beacon.uptime;
            nodeInfo.firmwareVersion = beacon.firmwareVersion;
            
            nodes.push_back(nodeInfo);
        }
    }
    
    return nodes;
}

std::vector<uint32_t> MeshDiscoveryManager::getDirectNeighbors() const {
    std::vector<uint32_t> neighbors;
    
    auto it = topology_.adjacencyList.find(localNodeId_);
    if (it != topology_.adjacencyList.end()) {
        for (uint32_t neighbor : it->second) {
            neighbors.push_back(neighbor);
        }
    }
    
    return neighbors;
}

void MeshDiscoveryManager::updateNetworkTopology() {
    uint32_t now = millis();
    if (now - lastTopologyUpdate_ < 30000) {  // Update every 30 seconds
        return;
    }
    
    removeStaleNodes();
    calculateNetworkMetrics();
    
    lastTopologyUpdate_ = now;
    
    if (g_topologyChangedCallback) {
        g_topologyChangedCallback(topology_.nodeInfo.size());
    }
}

void MeshDiscoveryManager::calculateNetworkMetrics() {
    if (topology_.nodeInfo.empty()) {
        topology_.networkDiameter = 0;
        topology_.averageConnectivity = 0.0;
        return;
    }
    
    // Calculate network diameter (maximum shortest path between any two nodes)
    topology_.networkDiameter = calculateNetworkDiameter();
    
    // Calculate average connectivity
    float totalConnections = 0.0;
    for (const auto& adjPair : topology_.adjacencyList) {
        totalConnections += adjPair.second.size();
    }
    
    topology_.averageConnectivity = totalConnections / topology_.nodeInfo.size();
}

uint8_t MeshDiscoveryManager::calculateNetworkDiameter() {
    // Simplified diameter calculation
    uint8_t maxHops = 0;
    
    for (const auto& nodePair : topology_.nodeInfo) {
        if (nodePair.first != localNodeId_) {
            uint8_t hops = getHopCountToNode(nodePair.first);
            if (hops > maxHops && hops < 255) {
                maxHops = hops;
            }
        }
    }
    
    return maxHops;
}

uint8_t MeshDiscoveryManager::getHopCountToNode(uint32_t nodeId) const {
    // Simple BFS to find hop count
    if (nodeId == localNodeId_) return 0;
    
    std::map<uint32_t, uint8_t> distances;
    std::queue<uint32_t> queue;
    
    distances[localNodeId_] = 0;
    queue.push(localNodeId_);
    
    while (!queue.empty()) {
        uint32_t current = queue.front();
        queue.pop();
        
        auto adjIt = topology_.adjacencyList.find(current);
        if (adjIt != topology_.adjacencyList.end()) {
            for (uint32_t neighbor : adjIt->second) {
                if (distances.find(neighbor) == distances.end()) {
                    distances[neighbor] = distances[current] + 1;
                    queue.push(neighbor);
                    
                    if (neighbor == nodeId) {
                        return distances[neighbor];
                    }
                }
            }
        }
    }
    
    return 255;  // Not reachable
}

void MeshDiscoveryManager::updateAdjacencyList(uint32_t nodeA, uint32_t nodeB) {
    topology_.adjacencyList[nodeA].insert(nodeB);
    topology_.adjacencyList[nodeB].insert(nodeA);
    topology_.lastUpdate = millis();
}

DiscoveryBeacon MeshDiscoveryManager::createDiscoveryBeacon() const {
    DiscoveryBeacon beacon;
    beacon.nodeId = localNodeId_;
    beacon.deviceName = "WildlifeCam_" + String(localNodeId_);
    beacon.capabilities = NODE_CAP_BASIC | NODE_CAP_IMAGE | NODE_CAP_SENSORS;
    beacon.batteryLevel = 85.0;  // Should come from power management
    beacon.uptime = millis();
    beacon.firmwareVersion = "2.0.0";
    beacon.lastSeen = millis();
    beacon.signalStrength = 0;  // Not applicable for own beacon
    beacon.signalToNoise = 0.0;
    beacon.hopCount = 0;  // Direct transmission
    
    return beacon;
}

String MeshDiscoveryManager::serializeBeacon(const DiscoveryBeacon& beacon) const {
    DynamicJsonDocument doc(512);
    
    doc["nodeId"] = beacon.nodeId;
    doc["deviceName"] = beacon.deviceName;
    doc["capabilities"] = beacon.capabilities;
    doc["batteryLevel"] = beacon.batteryLevel;
    doc["uptime"] = beacon.uptime;
    doc["firmwareVersion"] = beacon.firmwareVersion;
    doc["hopCount"] = beacon.hopCount;
    
    String result;
    serializeJson(doc, result);
    return result;
}

DiscoveryBeacon MeshDiscoveryManager::parseBeacon(const String& data) const {
    DiscoveryBeacon beacon;
    
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, data);
    
    if (!error) {
        beacon.nodeId = doc["nodeId"] | 0;
        beacon.deviceName = doc["deviceName"] | "";
        beacon.capabilities = doc["capabilities"] | NODE_CAP_BASIC;
        beacon.batteryLevel = doc["batteryLevel"] | 0.0;
        beacon.uptime = doc["uptime"] | 0;
        beacon.firmwareVersion = doc["firmwareVersion"] | "";
        beacon.hopCount = doc["hopCount"] | 0;
        beacon.lastSeen = millis();
    }
    
    return beacon;
}

bool MeshDiscoveryManager::isBeaconStale(const DiscoveryBeacon& beacon) const {
    uint32_t now = millis();
    return (now - beacon.lastSeen) > beaconTimeout_;
}

void MeshDiscoveryManager::performPeriodicTasks() {
    performActiveDiscovery();
    updateNetworkTopology();
}

String MeshDiscoveryManager::getDiscoveryReport() const {
    String report = "Discovery Report:\n";
    report += "Network: " + networkName_ + "\n";
    report += "Local Node: " + String(localNodeId_) + "\n";
    report += "Discovered Nodes: " + String(topology_.nodeInfo.size() - 1) + "\n";
    report += "Direct Neighbors: " + String(getDirectNeighbors().size()) + "\n";
    report += "Network Diameter: " + String(topology_.networkDiameter) + "\n";
    report += "Average Connectivity: " + String(topology_.averageConnectivity, 2) + "\n";
    
    return report;
}

uint32_t MeshDiscoveryManager::getDiscoveredNodeCount() const {
    return topology_.nodeInfo.size() > 0 ? topology_.nodeInfo.size() - 1 : 0;  // Exclude self
}

uint32_t MeshDiscoveryManager::getActiveNeighborCount() const {
    return getDirectNeighbors().size();
}

// Callback registration functions
void setNodeDiscoveredCallback(NodeDiscoveredCallback callback) {
    g_nodeDiscoveredCallback = callback;
}

void setNodeLostCallback(NodeLostCallback callback) {
    g_nodeLostCallback = callback;
}

void setTopologyChangedCallback(TopologyChangedCallback callback) {
    g_topologyChangedCallback = callback;
}

void setNetworkPartitionCallback(NetworkPartitionCallback callback) {
    g_networkPartitionCallback = callback;
}