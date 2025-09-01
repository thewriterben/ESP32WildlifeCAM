/**
 * Mesh Discovery Implementation
 * Phase 2 Enhancement - Node discovery and network joining
 * 
 * Features:
 * - Automatic neighbor discovery
 * - Network joining protocols
 * - Dynamic topology detection
 * - Node capability advertisement
 */

#include "../../include/network/mesh_protocol.h"
#include <vector>
#include <map>
#include <set>

#ifndef MESH_DISCOVERY_H
#define MESH_DISCOVERY_H

class MeshDiscoveryManager {
private:
    struct DiscoveryBeacon {
        uint32_t nodeId;
        String deviceName;
        uint8_t capabilities;
        float batteryLevel;
        uint32_t uptime;
        String firmwareVersion;
        uint32_t lastSeen;
        int8_t signalStrength;
        float signalToNoise;
        uint8_t hopCount;
        
        DiscoveryBeacon() : nodeId(0), capabilities(0), batteryLevel(0.0),
                           uptime(0), lastSeen(0), signalStrength(-100),
                           signalToNoise(0.0), hopCount(0) {}
    };
    
    struct NetworkTopology {
        std::map<uint32_t, std::set<uint32_t>> adjacencyList;
        std::map<uint32_t, DiscoveryBeacon> nodeInfo;
        uint32_t lastUpdate;
        uint8_t networkDiameter;
        float averageConnectivity;
        
        NetworkTopology() : lastUpdate(0), networkDiameter(0), averageConnectivity(0.0) {}
    };
    
    uint32_t localNodeId_;
    String networkName_;
    NetworkTopology topology_;
    
    uint32_t lastDiscoveryBroadcast_;
    uint32_t lastTopologyUpdate_;
    uint32_t discoveryInterval_;
    uint32_t beaconTimeout_;
    
    bool discoveryEnabled_;
    bool broadcastBeacons_;
    bool respondToDiscovery_;
    
    std::vector<uint32_t> discoveryQueue_;
    std::map<uint32_t, uint32_t> discoveryAttempts_;

public:
    MeshDiscoveryManager();
    ~MeshDiscoveryManager();
    
    // Initialization and configuration
    bool init(uint32_t nodeId, const String& networkName);
    void cleanup();
    void setDiscoveryInterval(uint32_t intervalMs) { discoveryInterval_ = intervalMs; }
    void setBeaconTimeout(uint32_t timeoutMs) { beaconTimeout_ = timeoutMs; }
    void enableDiscovery(bool enabled) { discoveryEnabled_ = enabled; }
    void enableBeaconBroadcast(bool enabled) { broadcastBeacons_ = enabled; }
    void enableDiscoveryResponse(bool enabled) { respondToDiscovery_ = enabled; }
    
    // Discovery operations
    void performActiveDiscovery();
    void broadcastDiscoveryBeacon();
    void processDiscoveryBeacon(const DiscoveryBeacon& beacon);
    void respondToDiscoveryRequest(uint32_t requesterNodeId);
    
    // Network joining
    bool joinNetwork(const String& networkName);
    bool leaveNetwork();
    bool isNetworkJoined() const { return !networkName_.isEmpty(); }
    String getCurrentNetwork() const { return networkName_; }
    
    // Node management
    void addDiscoveredNode(const MeshNodeInfo& nodeInfo, int8_t rssi, float snr);
    void removeStaleNodes();
    void updateNodeInfo(uint32_t nodeId, const MeshNodeInfo& nodeInfo);
    std::vector<MeshNodeInfo> getDiscoveredNodes() const;
    MeshNodeInfo getNodeInfo(uint32_t nodeId) const;
    
    // Topology management
    void updateNetworkTopology();
    void analyzeNetworkStructure();
    std::vector<uint32_t> getDirectNeighbors() const;
    std::vector<uint32_t> getNodesWithinHops(uint8_t maxHops) const;
    uint8_t getHopCountToNode(uint32_t nodeId) const;
    
    // Network quality assessment
    float assessNetworkCoverage();
    float calculateConnectivityRatio();
    uint8_t calculateNetworkDiameter();
    std::vector<uint32_t> findCriticalNodes();
    std::vector<std::pair<uint32_t, uint32_t>> findCriticalLinks();
    
    // Discovery optimization
    void optimizeDiscoverySchedule();
    void adaptDiscoveryPower();
    void prioritizeDiscoveryTargets();
    void balanceDiscoveryLoad();
    
    // Event handling
    void handleNodeJoined(uint32_t nodeId);
    void handleNodeLeft(uint32_t nodeId);
    void handleTopologyChange();
    void handleNetworkPartition();
    
    // Statistics and monitoring
    void updateDiscoveryStatistics();
    String getDiscoveryReport() const;
    String getTopologyReport() const;
    uint32_t getDiscoveredNodeCount() const;
    uint32_t getActiveNeighborCount() const;
    
    // Message processing
    void processDiscoveryMessage(uint32_t sourceNode, const String& payload, int8_t rssi, float snr);
    void processBeaconMessage(uint32_t sourceNode, const String& payload, int8_t rssi, float snr);
    void processTopologyUpdate(uint32_t sourceNode, const String& payload);
    
    // Periodic tasks
    void performPeriodicTasks();
    void scheduleDiscovery(uint32_t targetNodeId);
    void cancelDiscovery(uint32_t targetNodeId);

private:
    // Discovery algorithms
    void performFloodingDiscovery();
    void performRandomWalkDiscovery();
    void performDirectedDiscovery();
    void performGossipBasedDiscovery();
    
    // Network analysis
    void calculateNetworkMetrics();
    void detectNetworkClusters();
    void identifyBridgeNodes();
    void assessNetworkResilience();
    
    // Topology construction
    void buildTopologyMap();
    void updateAdjacencyList(uint32_t nodeA, uint32_t nodeB);
    void propagateTopologyUpdate();
    bool validateTopologyData(const NetworkTopology& topology);
    
    // Discovery optimization
    uint32_t calculateOptimalDiscoveryInterval();
    float calculateDiscoveryProbability(uint32_t targetNode);
    void adjustDiscoveryParameters();
    
    // Utility functions
    DiscoveryBeacon createDiscoveryBeacon() const;
    bool isBeaconValid(const DiscoveryBeacon& beacon) const;
    bool isBeaconStale(const DiscoveryBeacon& beacon) const;
    String serializeBeacon(const DiscoveryBeacon& beacon) const;
    DiscoveryBeacon parseBeacon(const String& data) const;
    
    // Network partition detection
    std::vector<std::vector<uint32_t>> detectConnectedComponents();
    void handlePartitionDetection(const std::vector<std::vector<uint32_t>>& components);
    void attemptPartitionRepair();
    
    // Quality of service
    void prioritizeImportantNodes();
    void deprioritizeRedundantNodes();
    void balanceDiscoveryResources();
};

// Discovery event callbacks
typedef void (*NodeDiscoveredCallback)(uint32_t nodeId, const MeshNodeInfo& nodeInfo);
typedef void (*NodeLostCallback)(uint32_t nodeId);
typedef void (*TopologyChangedCallback)(uint32_t affectedNodes);
typedef void (*NetworkPartitionCallback)(const std::vector<std::vector<uint32_t>>& partitions);

// Global discovery manager instance
extern MeshDiscoveryManager meshDiscovery;

// Callback registration functions
void setNodeDiscoveredCallback(NodeDiscoveredCallback callback);
void setNodeLostCallback(NodeLostCallback callback);
void setTopologyChangedCallback(TopologyChangedCallback callback);
void setNetworkPartitionCallback(NetworkPartitionCallback callback);

#endif // MESH_DISCOVERY_H