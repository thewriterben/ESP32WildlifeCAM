/**
 * @file mesh_protocol.h
 * @brief Advanced Mesh Protocol Definitions for ESP32WildlifeCAM Phase 2
 * 
 * This file defines the enhanced mesh protocol structures and interfaces
 * for the Phase 2 advanced features development, building on existing
 * Meshtastic infrastructure while adding wildlife-specific capabilities.
 * 
 * Phase 2 Enhancement - March 31, 2026 Target
 */

#ifndef MESH_PROTOCOL_H
#define MESH_PROTOCOL_H

#include <Arduino.h>
#include <vector>
#include <map>

// ===========================
// MESH NETWORK CONFIGURATION
// ===========================

#define MESH_MAX_NODES 255
#define MESH_MAX_HOPS 10
#define MESH_ROUTE_TIMEOUT 300000  // 5 minutes
#define MESH_BEACON_INTERVAL 60000 // 1 minute
#define MESH_DISCOVERY_TIMEOUT 30000 // 30 seconds
#define MESH_SYNC_INTERVAL 300000  // 5 minutes for time sync

// ===========================
// ENHANCED PACKET TYPES
// ===========================

enum MeshPacketTypeV2 {
    // Core protocol packets
    PACKET_ROUTE_DISCOVERY = 0x10,
    PACKET_ROUTE_REPLY = 0x11,
    PACKET_ROUTE_ERROR = 0x12,
    PACKET_TOPOLOGY_UPDATE = 0x13,
    
    // Wildlife-specific packets
    PACKET_WILDLIFE_DETECTION = 0x20,
    PACKET_BEHAVIOR_ANALYSIS = 0x21,
    PACKET_ENVIRONMENTAL_DATA = 0x22,
    PACKET_SENSOR_FUSION = 0x23,
    
    // Network management packets
    PACKET_TIME_SYNC = 0x30,
    PACKET_NETWORK_HEALTH = 0x31,
    PACKET_LOAD_BALANCE = 0x32,
    PACKET_COLLISION_AVOIDANCE = 0x33
};

// ===========================
// ROUTING STRUCTURES
// ===========================

struct RouteInfoV2 {
    uint32_t destination;
    uint32_t nextHop;
    uint8_t hopCount;
    uint32_t lastUsed;
    float reliability;      // 0.0-1.0 route reliability score
    int16_t signalStrength; // RSSI in dBm
    uint32_t bandwidth;     // Available bandwidth estimate
    uint8_t priority;       // Route priority (0=low, 255=high)
};

struct NetworkTopology {
    uint32_t nodeId;
    std::vector<uint32_t> neighbors;
    std::map<uint32_t, RouteInfoV2> routes;
    uint32_t lastUpdated;
    uint8_t nodeCount;
    float networkHealth;    // 0.0-1.0 overall network health
};

// ===========================
// NODE INFORMATION
// ===========================

enum NodeCapabilities {
    CAP_BASIC_CAMERA = 0x01,
    CAP_AI_PROCESSING = 0x02,
    CAP_ENVIRONMENTAL_SENSORS = 0x04,
    CAP_ADVANCED_SENSORS = 0x08,
    CAP_SOLAR_POWER = 0x10,
    CAP_WEATHER_STATION = 0x20,
    CAP_THERMAL_IMAGING = 0x40,
    CAP_RADAR_DETECTION = 0x80
};

struct NodeInfoV2 {
    uint32_t nodeId;
    uint16_t capabilities;
    uint8_t batteryLevel;     // 0-100%
    uint8_t signalQuality;    // 0-100%
    uint32_t uptime;          // Seconds since boot
    float temperature;        // Node temperature in Celsius
    uint16_t memoryFree;      // Free memory in KB
    uint32_t lastSeen;        // Last communication timestamp
    bool isCoordinator;       // Is this node a mesh coordinator
};

// ===========================
// WILDLIFE DATA STRUCTURES
// ===========================

struct WildlifeDetection {
    uint32_t timestamp;
    uint8_t speciesId;        // Detected species identifier
    float confidence;         // Detection confidence 0.0-1.0
    uint16_t boundingBox[4];  // x, y, width, height
    uint8_t behaviorCode;     // Behavior classification
    uint16_t imageId;         // Reference to captured image
    float environmentalScore; // Environmental context score
};

struct EnvironmentalData {
    uint32_t timestamp;
    float temperature;        // Celsius
    float humidity;          // Percentage
    float pressure;          // hPa
    float lightLevel;        // Lux
    uint16_t pm25;           // PM2.5 μg/m³
    uint16_t co2;            // CO2 ppm
    float soilMoisture;      // Percentage
    float windSpeed;         // m/s
    uint16_t windDirection;  // Degrees
    uint8_t airQualityIndex; // 0-500 AQI
};

// ===========================
// NETWORK MANAGEMENT
// ===========================

struct NetworkHealthMetrics {
    uint32_t totalPacketsSent;
    uint32_t totalPacketsReceived;
    uint32_t packetsDropped;
    uint32_t routingErrors;
    float avgLatency;         // Average packet latency in ms
    float packetLossRate;     // Packet loss percentage
    uint8_t networkLoad;      // Network utilization 0-100%
    uint32_t lastUpdated;
};

struct TimeSync {
    uint32_t coordinatorTime; // UTC timestamp from coordinator
    uint32_t localOffset;     // Local time offset
    uint16_t syncAccuracy;    // Sync accuracy in milliseconds
    uint32_t lastSync;        // Last successful sync timestamp
    bool isSynced;           // Sync status
};

// ===========================
// MESH PROTOCOL INTERFACE
// ===========================

class MeshProtocolV2 {
public:
    virtual ~MeshProtocolV2() = default;
    
    // Core routing functions
    virtual bool initializeNetwork() = 0;
    virtual bool addRoute(const RouteInfoV2& route) = 0;
    virtual bool removeRoute(uint32_t destination) = 0;
    virtual RouteInfoV2 findBestRoute(uint32_t destination) = 0;
    virtual bool updateTopology(const NetworkTopology& topology) = 0;
    
    // Node discovery and management
    virtual bool startNodeDiscovery() = 0;
    virtual bool registerNode(const NodeInfoV2& nodeInfo) = 0;
    virtual std::vector<NodeInfoV2> getActiveNodes() = 0;
    virtual bool setNodeCapabilities(uint16_t capabilities) = 0;
    
    // Wildlife data transmission
    virtual bool transmitWildlifeData(const WildlifeDetection& detection) = 0;
    virtual bool transmitEnvironmentalData(const EnvironmentalData& envData) = 0;
    virtual bool requestDataSync() = 0;
    
    // Network optimization
    virtual bool optimizeRoutes() = 0;
    virtual bool balanceNetworkLoad() = 0;
    virtual bool avoidCollisions() = 0;
    virtual NetworkHealthMetrics getNetworkHealth() = 0;
    
    // Time synchronization
    virtual bool synchronizeTime() = 0;
    virtual TimeSync getTimeSync() = 0;
    virtual bool isTimeSynced() = 0;
};

// ===========================
// UTILITY FUNCTIONS
// ===========================

/**
 * Calculate route reliability based on signal strength and packet loss
 */
float calculateRouteReliability(int16_t rssi, float packetLoss);

/**
 * Determine optimal hop count for network topology
 */
uint8_t calculateOptimalHops(uint8_t nodeCount, float networkDensity);

/**
 * Generate unique packet ID for tracking
 */
uint32_t generatePacketId();

/**
 * Validate packet integrity and format
 */
bool validatePacket(const uint8_t* packet, size_t length);

/**
 * Compress wildlife data for transmission
 */
size_t compressWildlifeData(const WildlifeDetection& data, uint8_t* buffer, size_t bufferSize);

/**
 * Decompress received wildlife data
 */
bool decompressWildlifeData(const uint8_t* buffer, size_t length, WildlifeDetection& data);

#endif // MESH_PROTOCOL_H