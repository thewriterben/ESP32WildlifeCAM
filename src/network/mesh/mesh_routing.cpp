/**
 * Mesh Routing Implementation
 * Phase 2 Enhancement - Dynamic routing and topology management
 */

#include "mesh_routing.h"
#include <limits>
#include <cmath>

// Global routing manager instance
MeshRoutingManager meshRouter;

MeshRoutingManager::MeshRoutingManager() 
    : localNodeId_(0), routeSequenceNumber_(0), adaptiveRoutingEnabled_(true),
      loadBalanceThreshold_(0.7), lastTopologyUpdate_(0) {
}

MeshRoutingManager::~MeshRoutingManager() {
    cleanup();
}

bool MeshRoutingManager::init(uint32_t nodeId) {
    localNodeId_ = nodeId;
    routeSequenceNumber_ = 0;
    adaptiveRoutingEnabled_ = true;
    loadBalanceThreshold_ = 0.7;
    lastTopologyUpdate_ = millis();
    
    Serial.println("Mesh Routing Manager initialized for node " + String(nodeId));
    return true;
}

void MeshRoutingManager::cleanup() {
    routingTable_.clear();
    linkTable_.clear();
    nodeTable_.clear();
}

bool MeshRoutingManager::addRoute(uint32_t destination, uint32_t nextHop, uint8_t hopCount, float reliability) {
    if (destination == localNodeId_) {
        return false;  // Don't add route to self
    }
    
    RouteEntry& route = routingTable_[destination];
    
    // Check if this is a better route
    float newCost = hopCount + (1.0 - reliability) * 10.0;
    if (route.destination == 0 || newCost < route.cost) {
        route.destination = destination;
        route.nextHop = nextHop;
        route.hopCount = hopCount;
        route.reliability = reliability;
        route.cost = newCost;
        route.lastUpdated = millis();
        
        Serial.println("Added/Updated route to " + String(destination) + 
                      " via " + String(nextHop) + " (cost: " + String(newCost) + ")");
        return true;
    }
    
    return false;
}

bool MeshRoutingManager::removeRoute(uint32_t destination) {
    auto it = routingTable_.find(destination);
    if (it != routingTable_.end()) {
        routingTable_.erase(it);
        Serial.println("Removed route to " + String(destination));
        return true;
    }
    return false;
}

bool MeshRoutingManager::updateRoute(uint32_t destination, uint32_t nextHop, uint8_t hopCount, float reliability) {
    return addRoute(destination, nextHop, hopCount, reliability);
}

void MeshRoutingManager::cleanupStaleRoutes(uint32_t timeoutMs) {
    uint32_t now = millis();
    
    for (auto it = routingTable_.begin(); it != routingTable_.end();) {
        if (now - it->second.lastUpdated > timeoutMs) {
            Serial.println("Removing stale route to " + String(it->first));
            it = routingTable_.erase(it);
        } else {
            ++it;
        }
    }
}

MeshRoute MeshRoutingManager::findBestRoute(uint32_t destination) {
    MeshRoute result;
    
    auto it = routingTable_.find(destination);
    if (it != routingTable_.end() && !isRouteStale(it->second)) {
        result.destination = it->second.destination;
        result.nextHop = it->second.nextHop;
        result.hopCount = it->second.hopCount;
        result.reliability = it->second.reliability;
        result.lastUsed = millis();
        result.metric = static_cast<uint32_t>(it->second.cost);
        
        // Mark route as used
        it->second.lastUsed = millis();
    }
    
    return result;
}

std::vector<MeshRoute> MeshRoutingManager::findAlternativeRoutes(uint32_t destination, uint8_t maxRoutes) {
    std::vector<MeshRoute> routes;
    
    // Find primary route
    MeshRoute primary = findBestRoute(destination);
    if (primary.destination != 0) {
        routes.push_back(primary);
    }
    
    // Find alternative routes through different next hops
    std::vector<std::pair<float, RouteEntry*>> alternatives;
    
    for (auto& pair : routingTable_) {
        if (pair.first == destination) continue;
        
        // Check if this node can reach the destination
        auto destRoute = routingTable_.find(destination);
        if (destRoute != routingTable_.end() && 
            pair.second.nextHop != destRoute->second.nextHop) {
            
            alternatives.push_back({pair.second.cost, &pair.second});
        }
    }
    
    // Sort by cost and add best alternatives
    std::sort(alternatives.begin(), alternatives.end());
    
    for (size_t i = 0; i < alternatives.size() && routes.size() < maxRoutes; i++) {
        MeshRoute route;
        route.destination = destination;
        route.nextHop = alternatives[i].second->nextHop;
        route.hopCount = alternatives[i].second->hopCount + 1;  // Additional hop
        route.reliability = alternatives[i].second->reliability * 0.9;  // Slight penalty
        route.lastUsed = millis();
        route.metric = static_cast<uint32_t>(alternatives[i].first);
        
        routes.push_back(route);
    }
    
    return routes;
}

bool MeshRoutingManager::initiateRouteDiscovery(uint32_t destination) {
    // Check if we already have a recent route
    auto it = routingTable_.find(destination);
    if (it != routingTable_.end() && !isRouteStale(it->second)) {
        return true;  // Route already exists and is fresh
    }
    
    Serial.println("Initiating route discovery for node " + String(destination));
    
    // Broadcast route request - this would need to be implemented
    // in conjunction with the main mesh networking layer
    
    return true;
}

void MeshRoutingManager::processRouteDiscovery(uint32_t requester, uint32_t destination, uint8_t hopCount) {
    if (destination == localNodeId_) {
        // We are the destination - send route reply back to requester
        Serial.println("Received route discovery for us from " + String(requester));
        
        // This would trigger a route reply message
        return;
    }
    
    // Check if we have a route to the destination
    auto it = routingTable_.find(destination);
    if (it != routingTable_.end() && !isRouteStale(it->second)) {
        // We have a route - can send intermediate reply
        Serial.println("Forwarding route discovery: " + String(requester) + 
                      " -> " + String(destination) + " (hop " + String(hopCount) + ")");
        
        // Forward the route discovery with incremented hop count
        // This would be implemented in the main mesh layer
    }
}

void MeshRoutingManager::processRouteReply(uint32_t destination, uint32_t nextHop, uint8_t hopCount, float reliability) {
    addRoute(destination, nextHop, hopCount, reliability);
    Serial.println("Processed route reply for " + String(destination) + 
                  " via " + String(nextHop));
}

void MeshRoutingManager::performDijkstraRouting() {
    // Implement Dijkstra's algorithm for shortest path routing
    std::map<uint32_t, float> distances;
    std::map<uint32_t, uint32_t> predecessors;
    
    dijkstraAlgorithm(localNodeId_, distances, predecessors);
    
    // Update routing table with calculated paths
    for (const auto& pair : distances) {
        if (pair.first != localNodeId_ && pair.second < std::numeric_limits<float>::infinity()) {
            uint32_t nextHop = pair.first;
            uint32_t current = pair.first;
            
            // Trace back to find next hop
            while (predecessors[current] != localNodeId_) {
                current = predecessors[current];
                nextHop = current;
            }
            
            // Calculate hop count
            uint8_t hopCount = 0;
            current = pair.first;
            while (current != localNodeId_) {
                current = predecessors[current];
                hopCount++;
            }
            
            float reliability = 1.0 / (1.0 + pair.second);  // Convert distance to reliability
            addRoute(pair.first, nextHop, hopCount, reliability);
        }
    }
}

void MeshRoutingManager::dijkstraAlgorithm(uint32_t source, std::map<uint32_t, float>& distances, 
                                         std::map<uint32_t, uint32_t>& predecessors) {
    
    // Initialize distances
    for (const auto& pair : nodeTable_) {
        distances[pair.first] = std::numeric_limits<float>::infinity();
        predecessors[pair.first] = 0;
    }
    distances[source] = 0.0;
    
    // Priority queue for unvisited nodes
    std::priority_queue<std::pair<float, uint32_t>, 
                       std::vector<std::pair<float, uint32_t>>,
                       std::greater<std::pair<float, uint32_t>>> pq;
    
    pq.push({0.0, source});
    
    while (!pq.empty()) {
        uint32_t current = pq.top().second;
        float currentDist = pq.top().first;
        pq.pop();
        
        if (currentDist > distances[current]) {
            continue;  // Already processed with better distance
        }
        
        // Check all neighbors
        std::vector<uint32_t> neighbors = getNeighbors(current);
        for (uint32_t neighbor : neighbors) {
            float linkCost = calculateLinkCost(current, neighbor);
            float newDist = distances[current] + linkCost;
            
            if (newDist < distances[neighbor]) {
                distances[neighbor] = newDist;
                predecessors[neighbor] = current;
                pq.push({newDist, neighbor});
            }
        }
    }
}

void MeshRoutingManager::performAdaptiveRouting() {
    if (!adaptiveRoutingEnabled_) {
        return;
    }
    
    uint32_t now = millis();
    if (now - lastTopologyUpdate_ < 60000) {  // Update every minute
        return;
    }
    
    // Measure current network conditions
    measureLinkPerformance();
    
    // Adapt routing based on network load
    if (calculateNetworkEfficiency() < 0.8) {
        performDijkstraRouting();  // Recalculate optimal paths
    }
    
    // Balance load if necessary
    balanceNetworkLoad();
    
    lastTopologyUpdate_ = now;
}

void MeshRoutingManager::balanceNetworkLoad() {
    for (auto& routePair : routingTable_) {
        uint32_t destination = routePair.first;
        RouteEntry& route = routePair.second;
        
        float nodeLoad = calculateNodeLoad(route.nextHop);
        if (nodeLoad > loadBalanceThreshold_) {
            // Try to find alternative path
            std::vector<MeshRoute> alternatives = findAlternativeRoutes(destination, 3);
            
            for (const auto& alt : alternatives) {
                float altNodeLoad = calculateNodeLoad(alt.nextHop);
                if (altNodeLoad < nodeLoad * 0.8) {  // Significantly less loaded
                    // Switch to alternative route
                    route.nextHop = alt.nextHop;
                    route.hopCount = alt.hopCount;
                    route.reliability = alt.reliability;
                    route.cost = alt.metric;
                    
                    Serial.println("Load balanced route to " + String(destination) + 
                                  " via " + String(alt.nextHop));
                    break;
                }
            }
        }
    }
}

uint32_t MeshRoutingManager::selectBestNextHop(uint32_t destination, const std::vector<uint32_t>& excludeNodes) {
    std::vector<MeshRoute> routes = findAlternativeRoutes(destination, 5);
    
    for (const auto& route : routes) {
        bool excluded = std::find(excludeNodes.begin(), excludeNodes.end(), route.nextHop) != excludeNodes.end();
        if (!excluded) {
            return route.nextHop;
        }
    }
    
    return 0;  // No suitable next hop found
}

float MeshRoutingManager::calculateNodeLoad(uint32_t nodeId) {
    // Simple load calculation based on number of routes through this node
    float load = 0.0;
    
    for (const auto& routePair : routingTable_) {
        if (routePair.second.nextHop == nodeId) {
            load += 1.0;
        }
    }
    
    // Normalize by total routes
    if (routingTable_.size() > 0) {
        load /= routingTable_.size();
    }
    
    return load;
}

void MeshRoutingManager::updateLinkQuality(uint32_t nodeA, uint32_t nodeB, float quality, float latency) {
    std::pair<uint32_t, uint32_t> linkKey = {std::min(nodeA, nodeB), std::max(nodeA, nodeB)};
    
    NetworkLink& link = linkTable_[linkKey];
    link.nodeA = nodeA;
    link.nodeB = nodeB;
    link.quality = quality;
    link.latency = latency;
    link.lastMeasured = millis();
}

float MeshRoutingManager::getLinkQuality(uint32_t nodeA, uint32_t nodeB) {
    std::pair<uint32_t, uint32_t> linkKey = {std::min(nodeA, nodeB), std::max(nodeA, nodeB)};
    
    auto it = linkTable_.find(linkKey);
    if (it != linkTable_.end()) {
        return it->second.quality;
    }
    
    return 0.0;  // Unknown link quality
}

void MeshRoutingManager::measureLinkPerformance() {
    // Measure link performance to immediate neighbors
    // This would typically involve sending test messages and measuring response times
    
    for (auto& linkPair : linkTable_) {
        NetworkLink& link = linkPair.second;
        
        // Age link quality measurements
        uint32_t age = millis() - link.lastMeasured;
        if (age > 300000) {  // 5 minutes
            link.quality *= 0.9;  // Decay quality if not recently measured
        }
    }
}

float MeshRoutingManager::calculateNetworkEfficiency() {
    if (routingTable_.empty()) {
        return 0.0;
    }
    
    float totalReliability = 0.0;
    uint32_t validRoutes = 0;
    
    for (const auto& routePair : routingTable_) {
        if (!isRouteStale(routePair.second)) {
            totalReliability += routePair.second.reliability;
            validRoutes++;
        }
    }
    
    return validRoutes > 0 ? totalReliability / validRoutes : 0.0;
}

std::vector<uint32_t> MeshRoutingManager::getNeighbors(uint32_t nodeId) {
    std::vector<uint32_t> neighbors;
    
    for (const auto& linkPair : linkTable_) {
        const NetworkLink& link = linkPair.second;
        
        if (link.nodeA == nodeId && link.quality > 0.1) {
            neighbors.push_back(link.nodeB);
        } else if (link.nodeB == nodeId && link.quality > 0.1) {
            neighbors.push_back(link.nodeA);
        }
    }
    
    return neighbors;
}

float MeshRoutingManager::calculateLinkCost(uint32_t nodeA, uint32_t nodeB) {
    float quality = getLinkQuality(nodeA, nodeB);
    if (quality <= 0.0) {
        return std::numeric_limits<float>::infinity();
    }
    
    // Cost is inverse of quality, with latency penalty
    float cost = 1.0 / quality;
    
    // Add latency component if available
    std::pair<uint32_t, uint32_t> linkKey = {std::min(nodeA, nodeB), std::max(nodeA, nodeB)};
    auto it = linkTable_.find(linkKey);
    if (it != linkTable_.end() && it->second.latency > 0) {
        cost += it->second.latency / 1000.0;  // Normalize latency to cost units
    }
    
    return cost;
}

bool MeshRoutingManager::isRouteStale(const RouteEntry& route) {
    uint32_t now = millis();
    return (now - route.lastUpdated) > 300000;  // 5 minutes
}

void MeshRoutingManager::addNode(const MeshNodeInfo& nodeInfo) {
    nodeTable_[nodeInfo.nodeId] = nodeInfo;
    
    // Initialize link quality to this node if it's a neighbor
    // This would typically be done when we receive a direct message from the node
}

void MeshRoutingManager::removeNode(uint32_t nodeId) {
    nodeTable_.erase(nodeId);
    
    // Remove all routes through this node
    for (auto it = routingTable_.begin(); it != routingTable_.end();) {
        if (it->second.nextHop == nodeId) {
            Serial.println("Removing route to " + String(it->first) + " (via removed node " + String(nodeId) + ")");
            it = routingTable_.erase(it);
        } else {
            ++it;
        }
    }
    
    // Remove links involving this node
    for (auto it = linkTable_.begin(); it != linkTable_.end();) {
        if (it->second.nodeA == nodeId || it->second.nodeB == nodeId) {
            it = linkTable_.erase(it);
        } else {
            ++it;
        }
    }
}

bool MeshRoutingManager::shouldForwardMessage(uint32_t sourceNode, uint32_t destNode, uint8_t currentHopCount) {
    // Don't forward if we're the destination
    if (destNode == localNodeId_) {
        return false;
    }
    
    // Don't forward if hop count exceeds maximum
    if (currentHopCount >= MAX_HOP_COUNT) {
        return false;
    }
    
    // Check if we have a route to the destination
    auto it = routingTable_.find(destNode);
    return it != routingTable_.end() && !isRouteStale(it->second);
}

uint32_t MeshRoutingManager::selectForwardingNode(uint32_t destNode, uint32_t excludeNode) {
    auto it = routingTable_.find(destNode);
    if (it != routingTable_.end() && !isRouteStale(it->second)) {
        if (it->second.nextHop != excludeNode) {
            return it->second.nextHop;
        }
        
        // Primary route is excluded, find alternative
        std::vector<uint32_t> excludeNodes = {excludeNode};
        return selectBestNextHop(destNode, excludeNodes);
    }
    
    return 0;  // No forwarding path found
}

String MeshRoutingManager::getRoutingTableReport() {
    String report = "Routing Table (" + String(routingTable_.size()) + " entries):\n";
    
    for (const auto& routePair : routingTable_) {
        const RouteEntry& route = routePair.second;
        report += "Dest: " + String(route.destination) + 
                 " -> Next: " + String(route.nextHop) + 
                 " (Hops: " + String(route.hopCount) + 
                 ", Rel: " + String(route.reliability, 2) + 
                 ", Cost: " + String(route.cost, 2) + ")\n";
    }
    
    return report;
}

std::vector<uint32_t> MeshRoutingManager::getActiveNodes() const {
    std::vector<uint32_t> nodes;
    for (const auto& pair : nodeTable_) {
        nodes.push_back(pair.first);
    }
    return nodes;
}