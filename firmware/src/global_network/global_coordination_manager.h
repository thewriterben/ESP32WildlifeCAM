/**
 * @file global_coordination_manager.h
 * @brief Global Wildlife Monitoring Network Coordination System v4.0
 * @author thewriterben
 * @date 2025-01-01
 * @version 4.0.0
 * 
 * Implements worldwide camera network coordination, global data sharing,
 * and international conservation collaboration framework.
 */

#ifndef GLOBAL_COORDINATION_MANAGER_H
#define GLOBAL_COORDINATION_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <time.h>
#include "../satellite_comm.h"
#include "../autonomous/drone_fleet/drone_fleet_manager.h"

enum NetworkRegion {
    REGION_NORTH_AMERICA = 1,
    REGION_SOUTH_AMERICA = 2,
    REGION_EUROPE = 3,
    REGION_AFRICA = 4,
    REGION_ASIA = 5,
    REGION_OCEANIA = 6,
    REGION_ANTARCTICA = 7,
    REGION_ARCTIC = 8
};

enum AlertPriority {
    ALERT_INFO = 1,
    ALERT_WARNING = 2,
    ALERT_CRITICAL = 3,
    ALERT_EMERGENCY = 4
};

enum DataSharingLevel {
    SHARING_PRIVATE = 0,     // No external sharing
    SHARING_REGIONAL = 1,    // Share within region
    SHARING_GLOBAL = 2,      // Share globally
    SHARING_RESEARCH = 3     // Share with research partners only
};

struct GlobalNetworkNode {
    uint32_t nodeId;
    String nodeName;
    NetworkRegion region;
    GPSCoordinate location;
    uint32_t cameraCount;
    uint32_t droneCount;
    time_t lastSeen;
    float dataUploadRate_mbps;
    bool operational;
    String contactOrganization;
    DataSharingLevel sharingLevel;
    std::vector<String> supportedSpecies;
    std::vector<String> researchTopics;
};

struct ConservationAlert {
    uint32_t alertId;
    AlertPriority priority;
    NetworkRegion sourceRegion;
    uint32_t sourceNodeId;
    String speciesAffected;
    String alertType;
    String description;
    GPSCoordinate location;
    time_t timestamp;
    time_t expirationTime;
    bool requiresAction;
    std::vector<String> affectedNodes;
    std::vector<String> recommendedActions;
};

struct MigrationData {
    uint32_t migrationId;
    String species;
    NetworkRegion sourceRegion;
    NetworkRegion destinationRegion;
    std::vector<GPSCoordinate> migrationPath;
    time_t startTime;
    time_t estimatedArrival;
    uint32_t populationCount;
    float confidenceLevel;
    std::vector<uint32_t> trackingNodes;
    bool crossesBorders;
    String migrationStatus;
};

class GlobalCoordinationManager {
public:
    GlobalCoordinationManager();
    ~GlobalCoordinationManager();
    
    // Initialization and configuration
    bool initialize(uint32_t nodeId, NetworkRegion region);
    bool registerNode(const GlobalNetworkNode& node);
    void setDataSharingLevel(DataSharingLevel level);
    
    // Network coordination
    std::vector<GlobalNetworkNode> getActiveNodes() const;
    bool syncWithGlobalNetwork();
    bool establishInternationalConnection();
    void broadcastNodeStatus();
    
    // Global data sharing
    bool shareWildlifeData(const String& speciesData, DataSharingLevel shareLevel);
    bool shareConservationAlert(const ConservationAlert& alert);
    bool shareMigrationData(const MigrationData& migration);
    std::vector<String> receiveSharedData(NetworkRegion sourceRegion);
    
    // Migration tracking across boundaries
    bool trackSpeciesMigration(const String& species, const GPSCoordinate& location);
    std::vector<MigrationData> getActiveMigrations() const;
    MigrationData predictMigrationPath(const String& species, const GPSCoordinate& startLocation);
    
    // Global conservation threat assessment
    bool reportConservationThreat(const ConservationAlert& threat);
    std::vector<ConservationAlert> getActiveAlerts(NetworkRegion region = static_cast<NetworkRegion>(0)) const;
    float assessGlobalThreatLevel(const String& species) const;
    
    // Integration with existing systems
    bool integrateWithSatelliteComm(SatelliteComm* satComm);
    bool integrateWithDroneFleet(DroneFleetManager* droneManager);
    void syncWithBlockchainVerification();
    void updateFederatedLearningModels();

private:
    uint32_t localNodeId_;
    NetworkRegion localRegion_;
    GlobalNetworkNode localNode_;
    
    std::vector<GlobalNetworkNode> networkNodes_;
    std::vector<ConservationAlert> activeAlerts_;
    std::vector<MigrationData> activeMigrations_;
    
    DataSharingLevel dataSharingLevel_;
    bool networkInitialized_;
    bool emergencyMode_;
    time_t lastNetworkSync_;
    
    // External system integration
    SatelliteComm* satelliteComm_;
    DroneFleetManager* droneManager_;
    
    // Private helper methods
    bool authenticateNode(uint32_t nodeId) const;
    void updateNetworkTopology();
    uint32_t generateAlertId();
    uint32_t generateMigrationId();
};

#endif // GLOBAL_COORDINATION_MANAGER_H
