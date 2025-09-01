/**
 * @file mesh_routing.cpp
 * @brief Dynamic Routing and Topology Management for Phase 2
 * 
 * This implementation provides advanced routing capabilities including
 * dynamic route discovery, topology optimization, and load balancing
 * for the enhanced mesh network.
 * 
 * Phase 2 Enhancement - March 31, 2026 Target
 */

#include "../../../include/network/mesh_protocol.h"
#include <ArduinoJson.h>
#include <algorithm>
#include <queue>

// ===========================
// MESH ROUTING ENGINE
// ===========================

class MeshRoutingEngine {
private:
    std::map<uint32_t, RouteInfoV2> routingTable_;
    std::map<uint32_t, std::vector<uint32_t>> topologyGraph_;
    std::map<uint32_t, float> nodeLoadMap_;
    
    uint32_t nodeId_;
    uint32_t lastTopologyUpdate_;
    uint32_t routeDiscoverySequence_;
    
    // Routing optimization parameters
    float loadBalanceThreshold_;
    uint32_t maxRouteAge_;
    uint8_t maxHopLimit_;
    
public:
    MeshRoutingEngine(uint32_t nodeId) : 
        nodeId_(nodeId), lastTopologyUpdate_(0), routeDiscoverySequence_(0),
        loadBalanceThreshold_(0.8), maxRouteAge_(300000), maxHopLimit_(MESH_MAX_HOPS) {
    }
    
    // ===========================
    // DYNAMIC ROUTE DISCOVERY
    // ===========================
    
    /**
     * Initiate route discovery using enhanced AODV protocol
     */
    bool initiateRouteDiscovery(uint32_t destination) {
        // Check if we already have a valid route
        auto existingRoute = routingTable_.find(destination);
        if (existingRoute != routingTable_.end() && 
            millis() - existingRoute->second.lastUsed < maxRouteAge_) {
            return true; // Valid route exists
        }
        
        // Create Route Request (RREQ) packet
        DynamicJsonDocument rreq(512);
        rreq["type"] = "route_request";
        rreq["originator"] = nodeId_;
        rreq["destination"] = destination;
        rreq["sequence"] = ++routeDiscoverySequence_;
        rreq["hop_count"] = 0;
        rreq["timestamp"] = millis();
        
        // Add quality metrics for route selection
        rreq["originator_load"] = getNodeLoad(nodeId_);
        rreq["min_reliability"] = 0.7; // Minimum acceptable route reliability
        rreq["max_latency"] = 2000;    // Maximum acceptable latency (ms)
        
        String message;
        serializeJson(rreq, message);
        
        Serial.printf("Initiating route discovery to %08X (seq: %d)\n", 
                     destination, routeDiscoverySequence_);
        
        return broadcastMessage(message);
    }
    
    /**
     * Process received Route Request
     */
    bool processRouteRequest(const JsonObject& rreq) {
        uint32_t originator = rreq["originator"];
        uint32_t destination = rreq["destination"];
        uint32_t sequence = rreq["sequence"];
        uint8_t hopCount = rreq["hop_count"];
        
        // Don't process our own requests
        if (originator == nodeId_) return false;
        
        // Check hop limit
        if (hopCount >= maxHopLimit_) return false;
        
        // Update reverse route to originator
        RouteInfoV2 reverseRoute;
        reverseRoute.destination = originator;
        reverseRoute.nextHop = originator; // Direct reverse hop
        reverseRoute.hopCount = hopCount + 1;
        reverseRoute.lastUsed = millis();
        reverseRoute.reliability = 0.9; // High reliability for fresh discovery
        reverseRoute.signalStrength = -60; // Estimated signal strength
        reverseRoute.bandwidth = 1000;
        reverseRoute.priority = 128;
        
        updateRoute(reverseRoute);
        
        // If we are the destination, send Route Reply
        if (destination == nodeId_) {
            return sendRouteReply(originator, sequence, hopCount);
        }
        
        // Check if we have a route to destination
        auto destRoute = routingTable_.find(destination);
        if (destRoute != routingTable_.end() && 
            destRoute->second.reliability >= rreq["min_reliability"].as<float>()) {
            // Send Route Reply with known route
            return sendRouteReply(originator, sequence, 
                                hopCount + destRoute->second.hopCount);
        }
        
        // Forward the request if conditions are met
        if (shouldForwardRequest(rreq)) {
            return forwardRouteRequest(rreq);
        }
        
        return true;
    }
    
    /**
     * Send Route Reply (RREP)
     */
    bool sendRouteReply(uint32_t originator, uint32_t sequence, uint8_t hopCount) {
        DynamicJsonDocument rrep(512);
        rrep["type"] = "route_reply";
        rrep["originator"] = originator;
        rrep["destination"] = nodeId_;
        rrep["sequence"] = sequence;
        rrep["hop_count"] = hopCount;
        rrep["timestamp"] = millis();
        
        // Add route quality metrics
        rrep["reliability"] = calculatePathReliability(originator);
        rrep["bandwidth"] = estimatePathBandwidth(originator);
        rrep["latency"] = estimatePathLatency(originator);
        rrep["node_load"] = getNodeLoad(nodeId_);
        
        String message;
        serializeJson(rrep, message);
        
        Serial.printf("Sending route reply to %08X (hops: %d)\n", originator, hopCount);
        
        return sendToNextHop(originator, message);
    }
    
    /**
     * Process received Route Reply
     */
    bool processRouteReply(const JsonObject& rrep) {
        uint32_t destination = rrep["destination"];
        uint8_t hopCount = rrep["hop_count"];
        float reliability = rrep["reliability"];
        uint32_t bandwidth = rrep["bandwidth"];
        uint32_t latency = rrep["latency"];
        
        // Create or update route
        RouteInfoV2 newRoute;
        newRoute.destination = destination;
        newRoute.nextHop = destination; // TODO: Set to actual next hop
        newRoute.hopCount = hopCount;
        newRoute.lastUsed = millis();
        newRoute.reliability = reliability;
        newRoute.signalStrength = -60; // Estimated
        newRoute.bandwidth = bandwidth;
        newRoute.priority = 128;
        
        return updateRoute(newRoute);
    }
    
    // ===========================
    // TOPOLOGY MANAGEMENT
    // ===========================
    
    /**
     * Update network topology based on neighbor discovery
     */
    bool updateTopology(uint32_t nodeId, const std::vector<uint32_t>& neighbors) {
        topologyGraph_[nodeId] = neighbors;
        lastTopologyUpdate_ = millis();
        
        // Recalculate optimal routes if topology changed significantly
        if (hasSignificantTopologyChange(nodeId, neighbors)) {
            return optimizeRoutingTable();
        }
        
        return true;
    }
    
    /**
     * Calculate shortest paths using Dijkstra's algorithm
     */
    bool calculateShortestPaths(uint32_t source) {
        std::map<uint32_t, float> distances;
        std::map<uint32_t, uint32_t> previous;
        std::priority_queue<std::pair<float, uint32_t>, 
                           std::vector<std::pair<float, uint32_t>>,
                           std::greater<std::pair<float, uint32_t>>> pq;
        
        // Initialize distances
        for (const auto& node : topologyGraph_) {
            distances[node.first] = std::numeric_limits<float>::infinity();
        }
        distances[source] = 0.0;
        pq.push({0.0, source});
        
        while (!pq.empty()) {
            float dist = pq.top().first;
            uint32_t u = pq.top().second;
            pq.pop();
            
            if (dist > distances[u]) continue;
            
            // Check all neighbors of u
            auto neighbors = topologyGraph_.find(u);
            if (neighbors != topologyGraph_.end()) {
                for (uint32_t v : neighbors->second) {
                    float weight = calculateLinkWeight(u, v);
                    float alt = distances[u] + weight;
                    
                    if (alt < distances[v]) {
                        distances[v] = alt;
                        previous[v] = u;
                        pq.push({alt, v});
                    }
                }
            }
        }
        
        // Update routing table with calculated paths
        for (const auto& dest : distances) {
            if (dest.first != source && dest.second < std::numeric_limits<float>::infinity()) {
                uint32_t nextHop = dest.first;
                uint32_t current = dest.first;
                
                // Trace back to find next hop
                while (previous.find(current) != previous.end() && previous[current] != source) {
                    current = previous[current];
                }
                if (previous.find(current) != previous.end()) {
                    nextHop = current;
                }
                
                RouteInfoV2 route;
                route.destination = dest.first;
                route.nextHop = nextHop;
                route.hopCount = calculateHopCount(source, dest.first);
                route.lastUsed = millis();
                route.reliability = 1.0 / (1.0 + dest.second); // Convert distance to reliability
                route.signalStrength = -60; // Estimated
                route.bandwidth = 1000;
                route.priority = 128;
                
                updateRoute(route);
            }
        }
        
        return true;
    }
    
    /**
     * Optimize routing table by removing stale routes and recalculating paths
     */
    bool optimizeRoutingTable() {
        uint32_t currentTime = millis();
        
        // Remove stale routes
        auto it = routingTable_.begin();
        while (it != routingTable_.end()) {
            if (currentTime - it->second.lastUsed > maxRouteAge_) {
                Serial.printf("Removing stale route to %08X\n", it->first);
                it = routingTable_.erase(it);
            } else {
                ++it;
            }
        }
        
        // Recalculate shortest paths
        return calculateShortestPaths(nodeId_);
    }
    
    // ===========================
    // LOAD BALANCING
    // ===========================
    
    /**
     * Balance network load by distributing traffic across multiple paths
     */
    bool balanceNetworkLoad() {
        // Identify overloaded nodes
        std::vector<uint32_t> overloadedNodes;
        for (const auto& load : nodeLoadMap_) {
            if (load.second > loadBalanceThreshold_) {
                overloadedNodes.push_back(load.first);
            }
        }
        
        if (overloadedNodes.empty()) return true; // No load balancing needed
        
        // Find alternative routes that avoid overloaded nodes
        for (uint32_t overloadedNode : overloadedNodes) {
            redistributeTraffic(overloadedNode);
        }
        
        return true;
    }
    
    /**
     * Update node load information
     */
    void updateNodeLoad(uint32_t nodeId, float load) {
        nodeLoadMap_[nodeId] = load;
    }
    
    /**
     * Get current load for a node
     */
    float getNodeLoad(uint32_t nodeId) const {
        auto it = nodeLoadMap_.find(nodeId);
        return (it != nodeLoadMap_.end()) ? it->second : 0.0;
    }
    
    // ===========================
    // ROUTE MANAGEMENT
    // ===========================
    
    /**
     * Update or add a route in the routing table
     */
    bool updateRoute(const RouteInfoV2& route) {
        auto existing = routingTable_.find(route.destination);
        
        if (existing != routingTable_.end()) {
            // Compare route quality
            float existingScore = calculateRouteScore(existing->second);
            float newScore = calculateRouteScore(route);
            
            if (newScore > existingScore) {
                routingTable_[route.destination] = route;
                Serial.printf("Updated route to %08X (score: %.2f)\n", 
                             route.destination, newScore);
                return true;
            }
        } else {
            // Add new route
            routingTable_[route.destination] = route;
            Serial.printf("Added new route to %08X\n", route.destination);
            return true;
        }
        
        return false;
    }
    
    /**
     * Find best route to destination
     */
    RouteInfoV2* findBestRoute(uint32_t destination) {
        auto it = routingTable_.find(destination);
        if (it != routingTable_.end()) {
            // Update last used time
            it->second.lastUsed = millis();
            return &it->second;
        }
        return nullptr;
    }
    
    /**
     * Get all available routes
     */
    const std::map<uint32_t, RouteInfoV2>& getRoutingTable() const {
        return routingTable_;
    }
    
    /**
     * Get routing statistics
     */
    void getRoutingStatistics(uint32_t& totalRoutes, float& avgHopCount, 
                             float& avgReliability) const {
        totalRoutes = routingTable_.size();
        
        if (totalRoutes == 0) {
            avgHopCount = 0.0;
            avgReliability = 0.0;
            return;
        }
        
        float totalHops = 0.0;
        float totalReliability = 0.0;
        
        for (const auto& route : routingTable_) {
            totalHops += route.second.hopCount;
            totalReliability += route.second.reliability;
        }
        
        avgHopCount = totalHops / totalRoutes;
        avgReliability = totalReliability / totalRoutes;
    }
    
private:
    // ===========================
    // HELPER FUNCTIONS
    // ===========================
    
    bool shouldForwardRequest(const JsonObject& rreq) {
        // Don't forward if hop count too high
        if (rreq["hop_count"].as<uint8_t>() >= maxHopLimit_ - 1) return false;
        
        // Don't forward if node is overloaded
        if (getNodeLoad(nodeId_) > loadBalanceThreshold_) return false;
        
        // Forward with some probability to reduce broadcast storm
        return random(100) < 80; // 80% forwarding probability
    }
    
    bool forwardRouteRequest(const JsonObject& rreq) {
        DynamicJsonDocument forwardedRreq(512);
        forwardedRreq = rreq;
        forwardedRreq["hop_count"] = rreq["hop_count"].as<uint8_t>() + 1;
        
        String message;
        serializeJson(forwardedRreq, message);
        
        return broadcastMessage(message);
    }
    
    float calculateLinkWeight(uint32_t from, uint32_t to) {
        // Weight based on load, reliability, and hop count
        float loadFactor = 1.0 + getNodeLoad(to);
        float reliabilityFactor = 2.0; // Default reliability factor
        
        // Look for existing route info
        auto route = routingTable_.find(to);
        if (route != routingTable_.end()) {
            reliabilityFactor = 2.0 - route->second.reliability;
        }
        
        return loadFactor * reliabilityFactor;
    }
    
    uint8_t calculateHopCount(uint32_t source, uint32_t destination) {
        // Simple BFS to calculate hop count
        std::queue<std::pair<uint32_t, uint8_t>> queue;
        std::set<uint32_t> visited;
        
        queue.push({source, 0});
        visited.insert(source);
        
        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();
            
            if (current.first == destination) {
                return current.second;
            }
            
            auto neighbors = topologyGraph_.find(current.first);
            if (neighbors != topologyGraph_.end()) {
                for (uint32_t neighbor : neighbors->second) {
                    if (visited.find(neighbor) == visited.end()) {
                        visited.insert(neighbor);
                        queue.push({neighbor, current.second + 1});
                    }
                }
            }
        }
        
        return MESH_MAX_HOPS; // No path found
    }
    
    float calculateRouteScore(const RouteInfoV2& route) {
        // Combine multiple factors into a single score
        float hopScore = 1.0 / (1.0 + route.hopCount);
        float reliabilityScore = route.reliability;
        float freshnessScore = 1.0 / (1.0 + (millis() - route.lastUsed) / 60000.0); // Age in minutes
        
        return hopScore * 0.3 + reliabilityScore * 0.5 + freshnessScore * 0.2;
    }
    
    float calculatePathReliability(uint32_t destination) {
        auto route = routingTable_.find(destination);
        return (route != routingTable_.end()) ? route->second.reliability : 0.5;
    }
    
    uint32_t estimatePathBandwidth(uint32_t destination) {
        auto route = routingTable_.find(destination);
        return (route != routingTable_.end()) ? route->second.bandwidth : 1000;
    }
    
    uint32_t estimatePathLatency(uint32_t destination) {
        auto route = routingTable_.find(destination);
        return (route != routingTable_.end()) ? route->second.hopCount * 50 : 1000; // 50ms per hop
    }
    
    bool hasSignificantTopologyChange(uint32_t nodeId, const std::vector<uint32_t>& newNeighbors) {
        auto existing = topologyGraph_.find(nodeId);
        if (existing == topologyGraph_.end()) return true; // New node
        
        // Check if neighbor set has changed significantly
        const auto& oldNeighbors = existing->second;
        
        // Count common neighbors
        std::set<uint32_t> common;
        std::set_intersection(oldNeighbors.begin(), oldNeighbors.end(),
                            newNeighbors.begin(), newNeighbors.end(),
                            std::inserter(common, common.begin()));
        
        float changeRatio = 1.0 - (float)common.size() / max(oldNeighbors.size(), newNeighbors.size());
        return changeRatio > 0.3; // Consider 30% change as significant
    }
    
    void redistributeTraffic(uint32_t overloadedNode) {
        // Find routes that go through the overloaded node
        std::vector<uint32_t> affectedDestinations;
        
        for (const auto& route : routingTable_) {
            if (route.second.nextHop == overloadedNode) {
                affectedDestinations.push_back(route.first);
            }
        }
        
        // Find alternative routes for affected destinations
        for (uint32_t dest : affectedDestinations) {
            // Remove current route and trigger route discovery
            routingTable_.erase(dest);
            // Route discovery will be triggered when the route is needed
        }
    }
    
    bool broadcastMessage(const String& message) {
        // This would interface with the actual LoRa transmission system
        // For now, return true to indicate successful queuing
        return true;
    }
    
    bool sendToNextHop(uint32_t destination, const String& message) {
        auto route = findBestRoute(destination);
        if (route) {
            // Send message to next hop
            return true; // Placeholder
        }
        return false;
    }
};

// ===========================
// PUBLIC API
// ===========================

static MeshRoutingEngine* g_routingEngine = nullptr;

namespace MeshRouting {
    bool initialize(uint32_t nodeId) {
        if (g_routingEngine) {
            delete g_routingEngine;
        }
        
        g_routingEngine = new MeshRoutingEngine(nodeId);
        Serial.printf("Mesh routing engine initialized for node %08X\n", nodeId);
        return true;
    }
    
    void cleanup() {
        if (g_routingEngine) {
            delete g_routingEngine;
            g_routingEngine = nullptr;
        }
    }
    
    bool discoverRoute(uint32_t destination) {
        return g_routingEngine ? g_routingEngine->initiateRouteDiscovery(destination) : false;
    }
    
    bool updateTopology(uint32_t nodeId, const std::vector<uint32_t>& neighbors) {
        return g_routingEngine ? g_routingEngine->updateTopology(nodeId, neighbors) : false;
    }
    
    bool optimizeRoutes() {
        return g_routingEngine ? g_routingEngine->optimizeRoutingTable() : false;
    }
    
    bool balanceLoad() {
        return g_routingEngine ? g_routingEngine->balanceNetworkLoad() : false;
    }
    
    RouteInfoV2* findRoute(uint32_t destination) {
        return g_routingEngine ? g_routingEngine->findBestRoute(destination) : nullptr;
    }
    
    void updateNodeLoad(uint32_t nodeId, float load) {
        if (g_routingEngine) {
            g_routingEngine->updateNodeLoad(nodeId, load);
        }
    }
    
    void getStatistics(uint32_t& totalRoutes, float& avgHopCount, float& avgReliability) {
        if (g_routingEngine) {
            g_routingEngine->getRoutingStatistics(totalRoutes, avgHopCount, avgReliability);
        } else {
            totalRoutes = 0;
            avgHopCount = 0.0;
            avgReliability = 0.0;
        }
    }
}