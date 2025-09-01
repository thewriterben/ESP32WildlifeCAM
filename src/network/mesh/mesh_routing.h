/**
 * Mesh Routing Implementation
 * Phase 2 Enhancement - Dynamic routing and topology management
 * 
 * Features:
 * - Dynamic route discovery and maintenance
 * - Multi-path routing with load balancing
 * - Network partition detection and repair
 * - Adaptive routing based on network conditions
 */

#include "../../include/network/mesh_protocol.h"
#include <vector>
#include <map>
#include <algorithm>
#include <queue>

#ifndef MESH_ROUTING_H
#define MESH_ROUTING_H

class MeshRoutingManager {
private:
    struct RouteEntry {
        uint32_t destination;
        uint32_t nextHop;
        uint8_t hopCount;
        float reliability;
        float cost;
        uint32_t lastUsed;
        uint32_t lastUpdated;
        std::vector<uint32_t> alternatePaths;
        
        RouteEntry() : destination(0), nextHop(0), hopCount(255), 
                       reliability(0.0), cost(999.0), lastUsed(0), lastUpdated(0) {}
    };
    
    struct NetworkLink {
        uint32_t nodeA;
        uint32_t nodeB;
        float quality;
        float latency;
        float bandwidth;
        uint32_t lastMeasured;
        
        NetworkLink() : nodeA(0), nodeB(0), quality(0.0), 
                        latency(0.0), bandwidth(0.0), lastMeasured(0) {}
    };
    
    std::map<uint32_t, RouteEntry> routingTable_;
    std::map<std::pair<uint32_t, uint32_t>, NetworkLink> linkTable_;
    std::map<uint32_t, MeshNodeInfo> nodeTable_;
    
    uint32_t localNodeId_;
    uint32_t routeSequenceNumber_;
    bool adaptiveRoutingEnabled_;
    float loadBalanceThreshold_;
    uint32_t lastTopologyUpdate_;
    
public:
    MeshRoutingManager();
    ~MeshRoutingManager();
    
    // Initialization and configuration
    bool init(uint32_t nodeId);
    void cleanup();
    void setAdaptiveRouting(bool enabled) { adaptiveRoutingEnabled_ = enabled; }
    void setLoadBalanceThreshold(float threshold) { loadBalanceThreshold_ = threshold; }
    
    // Route management
    bool addRoute(uint32_t destination, uint32_t nextHop, uint8_t hopCount, float reliability = 1.0);
    bool removeRoute(uint32_t destination);
    bool updateRoute(uint32_t destination, uint32_t nextHop, uint8_t hopCount, float reliability);
    void cleanupStaleRoutes(uint32_t timeoutMs = 300000);
    
    // Route discovery
    MeshRoute findBestRoute(uint32_t destination);
    std::vector<MeshRoute> findAlternativeRoutes(uint32_t destination, uint8_t maxRoutes = 3);
    bool initiateRouteDiscovery(uint32_t destination);
    void processRouteDiscovery(uint32_t requester, uint32_t destination, uint8_t hopCount);
    void processRouteReply(uint32_t destination, uint32_t nextHop, uint8_t hopCount, float reliability);
    
    // Dynamic routing algorithms
    void performDijkstraRouting();
    void performBellmanFordRouting();
    void performAdaptiveRouting();
    std::vector<uint32_t> calculateShortestPath(uint32_t source, uint32_t destination);
    
    // Load balancing
    void balanceNetworkLoad();
    uint32_t selectBestNextHop(uint32_t destination, const std::vector<uint32_t>& excludeNodes = {});
    float calculateNodeLoad(uint32_t nodeId);
    void redistributeTraffic();
    
    // Network topology management
    void updateNetworkTopology();
    void detectNetworkPartitions();
    void repairNetworkPartitions();
    void optimizeTopology();
    
    // Link quality management
    void updateLinkQuality(uint32_t nodeA, uint32_t nodeB, float quality, float latency = 0.0);
    float getLinkQuality(uint32_t nodeA, uint32_t nodeB);
    void measureLinkPerformance();
    void adaptToLinkChanges();
    
    // Network analysis
    float calculateNetworkEfficiency();
    uint8_t calculateNetworkDiameter();
    float calculateClusteringCoefficient();
    std::vector<std::vector<uint32_t>> detectCommunities();
    
    // Route maintenance
    void maintainRoutes();
    void validateRoutes();
    void repairBrokenRoutes();
    void optimizeRouteTable();
    
    // Statistics and monitoring
    void updateRoutingStatistics();
    String getRoutingTableReport();
    String getNetworkTopologyReport();
    uint32_t getRoutingTableSize() const { return routingTable_.size(); }
    
    // Node management
    void addNode(const MeshNodeInfo& nodeInfo);
    void removeNode(uint32_t nodeId);
    void updateNodeInfo(uint32_t nodeId, const MeshNodeInfo& nodeInfo);
    std::vector<uint32_t> getActiveNodes() const;
    
    // Message forwarding decisions
    bool shouldForwardMessage(uint32_t sourceNode, uint32_t destNode, uint8_t currentHopCount);
    uint32_t selectForwardingNode(uint32_t destNode, uint32_t excludeNode = 0);
    void recordForwardingDecision(uint32_t destNode, uint32_t forwardingNode, bool success);

private:
    // Internal algorithms
    void dijkstraAlgorithm(uint32_t source, std::map<uint32_t, float>& distances, 
                          std::map<uint32_t, uint32_t>& predecessors);
    void bellmanFordAlgorithm(uint32_t source);
    void floydWarshallAlgorithm();
    
    // Route cost calculation
    float calculateRouteCost(const RouteEntry& route);
    float calculateLinkCost(uint32_t nodeA, uint32_t nodeB);
    float calculateNodeCost(uint32_t nodeId);
    
    // Network graph operations
    std::vector<uint32_t> getNeighbors(uint32_t nodeId);
    bool isConnected(uint32_t nodeA, uint32_t nodeB);
    void buildSpanningTree();
    
    // Route validation
    bool validateRoute(const RouteEntry& route);
    bool isRouteStale(const RouteEntry& route);
    void markRouteAsUsed(uint32_t destination);
    
    // Load balancing algorithms
    void ecmpLoadBalancing(uint32_t destination);  // Equal Cost Multi-Path
    void wcmpLoadBalancing(uint32_t destination);  // Weighted Cost Multi-Path
    void adaptiveLoadBalancing(uint32_t destination);
    
    // Network partition handling
    std::vector<std::vector<uint32_t>> findConnectedComponents();
    void attemptPartitionRepair(const std::vector<std::vector<uint32_t>>& partitions);
    
    // Utility functions
    uint32_t generateSequenceNumber() { return ++routeSequenceNumber_; }
    bool isLocalNode(uint32_t nodeId) const { return nodeId == localNodeId_; }
    void broadcastRouteUpdate(uint32_t destination, const RouteEntry& route);
    void sendRouteError(uint32_t destination, uint32_t brokenLink);
};

// Global routing manager instance
extern MeshRoutingManager meshRouter;

#endif // MESH_ROUTING_H