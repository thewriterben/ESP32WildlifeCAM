#ifndef MESH_PROTOCOL_H
#define MESH_PROTOCOL_H

#include <Arduino.h>
#include <vector>
#include <map>

/**
 * Mesh Protocol Definitions and Data Structures
 * Phase 2 Enhancement - Advanced mesh networking protocols
 */

// Protocol version and constants
#define MESH_PROTOCOL_VERSION "2.0"
#define MAX_MESH_NODES 50
#define MAX_HOP_COUNT 8
#define MESH_HEARTBEAT_INTERVAL 30000  // 30 seconds
#define MESH_ROUTE_TIMEOUT 300000      // 5 minutes
#define MESH_DISCOVERY_INTERVAL 60000  // 1 minute
#define MESH_SYNC_INTERVAL 120000      // 2 minutes

// Message types for enhanced mesh protocol
enum MeshMessageType {
    MESH_MSG_HEARTBEAT = 0x01,
    MESH_MSG_DISCOVERY = 0x02,
    MESH_MSG_ROUTE_REQUEST = 0x03,
    MESH_MSG_ROUTE_REPLY = 0x04,
    MESH_MSG_DATA = 0x05,
    MESH_MSG_IMAGE_CHUNK = 0x06,
    MESH_MSG_TIME_SYNC = 0x07,
    MESH_MSG_STATUS = 0x08,
    MESH_MSG_ACK = 0x09,
    MESH_MSG_ERROR = 0x0A
};

// Node capabilities flags
enum NodeCapabilities {
    NODE_CAP_BASIC = 0x01,          // Basic messaging
    NODE_CAP_IMAGE = 0x02,          // Image transmission
    NODE_CAP_SENSORS = 0x04,        // Environmental sensors
    NODE_CAP_AI = 0x08,             // AI processing
    NODE_CAP_GATEWAY = 0x10,        // Gateway to external networks
    NODE_CAP_HIGH_POWER = 0x20      // High power transmission
};

// Network topology types
enum TopologyType {
    TOPOLOGY_STAR = 0,
    TOPOLOGY_MESH = 1,
    TOPOLOGY_TREE = 2,
    TOPOLOGY_HYBRID = 3
};

// Mesh node information structure
struct MeshNodeInfo {
    uint32_t nodeId;
    String deviceName;
    uint8_t capabilities;
    float batteryLevel;
    int8_t signalStrength;  // RSSI
    float signalToNoise;    // SNR
    uint32_t lastSeen;
    uint32_t uptime;
    String firmwareVersion;
    
    MeshNodeInfo() : nodeId(0), capabilities(NODE_CAP_BASIC), 
                     batteryLevel(0.0), signalStrength(-100), 
                     signalToNoise(0.0), lastSeen(0), uptime(0) {}
};

// Route information for mesh routing
struct MeshRoute {
    uint32_t destination;
    uint32_t nextHop;
    uint8_t hopCount;
    float reliability;      // 0.0 - 1.0
    uint32_t lastUsed;
    uint32_t metric;        // Combined cost metric
    
    MeshRoute() : destination(0), nextHop(0), hopCount(255), 
                  reliability(0.0), lastUsed(0), metric(UINT32_MAX) {}
};

// Mesh message header structure
struct MeshMessageHeader {
    uint8_t version;        // Protocol version
    uint8_t messageType;    // MeshMessageType
    uint32_t sourceNode;    // Source node ID
    uint32_t destNode;      // Destination node ID (0 = broadcast)
    uint16_t messageId;     // Unique message identifier
    uint8_t hopCount;       // Current hop count
    uint8_t maxHops;        // Maximum allowed hops
    uint32_t timestamp;     // Message timestamp
    uint16_t payloadSize;   // Payload size in bytes
    uint16_t checksum;      // Header checksum
    
    MeshMessageHeader() : version(2), messageType(MESH_MSG_DATA), 
                          sourceNode(0), destNode(0), messageId(0),
                          hopCount(0), maxHops(MAX_HOP_COUNT), 
                          timestamp(0), payloadSize(0), checksum(0) {}
};

// Network statistics structure
struct MeshNetworkStats {
    uint32_t messagesReceived;
    uint32_t messagesSent;
    uint32_t messagesForwarded;
    uint32_t messagesDropped;
    uint32_t routeDiscoveries;
    uint32_t networkJoins;
    float averageLatency;
    float packetLossRate;
    float networkEfficiency;
    uint32_t totalNodes;
    uint32_t activeNodes;
    
    MeshNetworkStats() : messagesReceived(0), messagesSent(0), 
                         messagesForwarded(0), messagesDropped(0),
                         routeDiscoveries(0), networkJoins(0),
                         averageLatency(0.0), packetLossRate(0.0),
                         networkEfficiency(0.0), totalNodes(0), activeNodes(0) {}
};

// Time synchronization structure
struct MeshTimeSync {
    uint32_t referenceTime;     // UTC timestamp from time server
    uint32_t localTime;         // Local system time
    int32_t timeOffset;         // Offset from reference time
    uint8_t syncAccuracy;       // Synchronization accuracy (0-100)
    uint32_t lastSyncTime;      // When last sync occurred
    uint32_t syncSource;        // Node ID of time source
    
    MeshTimeSync() : referenceTime(0), localTime(0), timeOffset(0),
                     syncAccuracy(0), lastSyncTime(0), syncSource(0) {}
};

// Advanced mesh configuration
struct MeshConfiguration {
    uint32_t nodeId;
    String networkName;
    uint8_t encryptionKey[16];
    TopologyType preferredTopology;
    bool adaptiveRouting;
    bool timeSync;
    uint32_t maxMessageSize;
    uint8_t qualityOfService;   // 0=best effort, 1=guaranteed delivery
    
    MeshConfiguration() : nodeId(0), preferredTopology(TOPOLOGY_MESH),
                          adaptiveRouting(true), timeSync(true),
                          maxMessageSize(1024), qualityOfService(0) {
        networkName = "WildlifeMesh";
        memset(encryptionKey, 0, 16);
    }
};

#endif // MESH_PROTOCOL_H