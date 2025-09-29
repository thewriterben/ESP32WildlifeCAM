/**
 * @file drone_fleet_manager.h
 * @brief Autonomous Drone Fleet Management System for ESP32WildlifeCAM v4.0
 * @author thewriterben
 * @date 2025-01-01
 * @version 4.0.0
 * 
 * Implements autonomous drone fleet coordination for wildlife camera deployment,
 * maintenance, and emergency response operations.
 */

#ifndef DRONE_FLEET_MANAGER_H
#define DRONE_FLEET_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <time.h>
#include "../../satellite_comm.h"
#include "../../gps_handler.h"

// Forward declarations
struct DroneStatus;
struct MissionPlan;
struct DeploymentSite;
struct ChargingStation;

enum DroneState {
    DRONE_IDLE = 0,
    DRONE_DEPLOYING,
    DRONE_RETURNING,
    DRONE_CHARGING,
    DRONE_MAINTENANCE,
    DRONE_EMERGENCY,
    DRONE_OFFLINE
};

enum MissionType {
    MISSION_DEPLOY_CAMERA = 0,
    MISSION_BATTERY_REPLACEMENT,
    MISSION_MAINTENANCE_CHECK,
    MISSION_EMERGENCY_RETRIEVAL,
    MISSION_SURVEY_AREA,
    MISSION_RELOCATE_CAMERA
};

enum DeploymentPriority {
    PRIORITY_LOW = 1,
    PRIORITY_NORMAL = 2,
    PRIORITY_HIGH = 3,
    PRIORITY_CRITICAL = 4,
    PRIORITY_EMERGENCY = 5
};

struct GPSCoordinate {
    double latitude;
    double longitude;
    double altitude;
    float accuracy;
    time_t timestamp;
};

struct DroneCapabilities {
    float maxRange_km;          // Maximum flight range in km
    float maxPayload_kg;        // Maximum payload capacity
    float batteryCapacity_mAh;  // Battery capacity
    float cruiseSpeed_ms;       // Cruise speed in m/s
    bool hasGPS;               // GPS capability
    bool hasCamera;            // Onboard camera
    bool hasLidar;             // LiDAR for terrain mapping
    bool weatherResistant;     // Weather resistance rating
};

struct DroneStatus {
    uint32_t droneId;
    DroneState state;
    GPSCoordinate currentPosition;
    float batteryLevel;        // 0.0 - 1.0
    float signalStrength;      // Communication signal strength
    uint32_t missionId;        // Current mission ID (0 if none)
    time_t lastUpdate;
    DroneCapabilities capabilities;
    String statusMessage;
    bool emergencyMode;
};

struct DeploymentSite {
    uint32_t siteId;
    GPSCoordinate coordinates;
    float terrainDifficulty;   // 0.0 - 1.0 (easy to difficult)
    float wildlifeActivity;    // Predicted wildlife activity level
    float accessibilityScore;  // How accessible for maintenance
    time_t optimalDeployTime;  // Best time for deployment
    bool requiresSpecialEquipment;
    String siteDescription;
    std::vector<String> migrationRoutes; // Associated migration routes
};

struct MissionPlan {
    uint32_t missionId;
    MissionType type;
    DeploymentPriority priority;
    uint32_t assignedDroneId;
    DeploymentSite targetSite;
    time_t scheduledStart;
    time_t estimatedDuration;
    std::vector<GPSCoordinate> flightPath;
    float estimatedBatteryUsage;
    String missionNotes;
    bool requiresHumanApproval;
};

struct ChargingStation {
    uint32_t stationId;
    GPSCoordinate location;
    uint8_t maxDrones;         // Maximum drones that can charge simultaneously
    uint8_t currentDrones;     // Currently charging drones
    bool solarPowered;
    float powerLevel;          // Available power level
    bool operational;
    time_t lastMaintenance;
};

class DroneFleetManager {
public:
    DroneFleetManager();
    ~DroneFleetManager();
    
    // Initialization and configuration
    bool initialize();
    bool registerDrone(uint32_t droneId, const DroneCapabilities& capabilities);
    bool registerChargingStation(const ChargingStation& station);
    void setMissionParameters(float maxMissionRange_km, uint8_t maxConcurrentMissions);
    
    // Fleet status and monitoring
    std::vector<DroneStatus> getFleetStatus() const;
    DroneStatus getDroneStatus(uint32_t droneId) const;
    uint8_t getAvailableDroneCount() const;
    uint8_t getActiveMissionCount() const;
    bool isFleetOperational() const;
    
    // Mission planning and execution
    uint32_t planDeploymentMission(const DeploymentSite& site, DeploymentPriority priority);
    uint32_t planMaintenanceMission(uint32_t cameraId, MissionType type);
    bool executeMission(uint32_t missionId);
    bool abortMission(uint32_t missionId, const String& reason);
    std::vector<MissionPlan> getActiveMissions() const;
    
    // Site selection and analysis
    std::vector<DeploymentSite> analyzeOptimalSites(
        const GPSCoordinate& centerPoint, 
        float radius_km,
        uint8_t maxSites
    );
    float calculateSiteScore(const DeploymentSite& site) const;
    bool validateDeploymentSite(const DeploymentSite& site) const;
    
    // Drone coordination and communication
    bool sendDroneCommand(uint32_t droneId, const String& command);
    bool updateDroneStatus(const DroneStatus& status);
    bool establishDroneMesh(); // Drone-to-drone communication network
    void coordinateFlightPaths(); // Prevent collisions
    bool performGPSGuidedPrecisionLanding(uint32_t droneId, const GPSCoordinate& target);
    
    // GPS-guided precision landing and equipment installation
    bool enablePrecisionLandingMode(uint32_t droneId);
    float calculateLandingAccuracy(uint32_t droneId, const GPSCoordinate& target);
    
    // Charging and maintenance management
    bool scheduleCharging(uint32_t droneId, uint32_t stationId);
    bool scheduleMaintenanceCheck(uint32_t droneId);
    ChargingStation findNearestChargingStation(const GPSCoordinate& position) const;
    float estimateReturnBattery(uint32_t droneId, const GPSCoordinate& destination) const;
    
    // Emergency response
    bool handleEmergencyRecall();
    bool dispatchEmergencyMission(const GPSCoordinate& location, const String& reason);
    void enterEmergencyMode();
    void exitEmergencyMode();
    bool isEmergencyMode() const;
    
    // Weather and environmental monitoring
    bool checkWeatherConditions(const GPSCoordinate& location) const;
    bool isFlightSafe(const MissionPlan& mission) const;
    void updateEnvironmentalData();
    
    // Integration with existing systems
    bool integrateWithSatelliteComm(SatelliteComm* satComm);
    bool integrateWithGPS(GPSHandler* gpsHandler);
    void syncWithGlobalNetwork();
    
    // Performance metrics and optimization
    struct FleetMetrics {
        uint32_t totalMissions;
        uint32_t successfulMissions;
        float averageMissionTime_hours;
        float totalFlightHours;
        float fuelEfficiency;
        uint32_t maintenanceEvents;
    };
    
    FleetMetrics getFleetMetrics() const;
    void optimizeFleetOperations();
    bool exportMissionLogs(const String& filename) const;

private:
    std::vector<DroneStatus> fleet_;
    std::vector<MissionPlan> activeMissions_;
    std::vector<ChargingStation> chargingStations_;
    std::map<uint32_t, time_t> lastCommunication_;
    
    // Configuration
    float maxMissionRange_km_;
    uint8_t maxConcurrentMissions_;
    bool emergencyMode_;
    bool fleetInitialized_;
    
    // External system integration
    SatelliteComm* satelliteComm_;
    GPSHandler* gpsHandler_;
    
    // Metrics tracking
    FleetMetrics metrics_;
    time_t lastOptimization_;
    
    // Private helper methods
    bool isDroneAvailable(uint32_t droneId) const;
    uint32_t selectOptimalDrone(const MissionPlan& mission) const;
    std::vector<GPSCoordinate> planFlightPath(
        const GPSCoordinate& start,
        const GPSCoordinate& destination
    ) const;
    bool checkFlightPathSafety(const std::vector<GPSCoordinate>& path) const;
    void updateMissionStatus();
    void handleDroneCommunicationTimeout(uint32_t droneId);
    float calculateMissionComplexity(const MissionPlan& mission) const;
    bool validateMissionPlan(const MissionPlan& mission) const;
    
    // Terrain and environmental analysis
    float analyzeTerrainDifficulty(const GPSCoordinate& location) const;
    float predictWildlifeActivity(const GPSCoordinate& location) const;
    bool checkAirspaceRestrictions(const GPSCoordinate& location) const;
    
    // Battery and power management
    float calculateBatteryUsage(const MissionPlan& mission) const;
    bool requiresChargingBeforeMission(uint32_t droneId, const MissionPlan& mission) const;
    void optimizeChargingSchedule();
    
    // Network coordination
    void broadcastFleetStatus();
    void receiveFleetUpdates();
    bool coordinateWithOtherFleets();
    
    // Safety and compliance
    bool performPreFlightCheck(uint32_t droneId) const;
    void logSafetyEvent(const String& event);
    bool checkRegulatoryCompliance(const MissionPlan& mission) const;
};

#endif // DRONE_FLEET_MANAGER_H