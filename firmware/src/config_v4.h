/**
 * @file config_v4.h
 * @brief Version 4.0 Configuration Settings
 * @author thewriterben
 * @date 2025-01-01
 * @version 4.0.0
 * 
 * Configuration settings for autonomous global wildlife monitoring network.
 */

#ifndef CONFIG_V4_H
#define CONFIG_V4_H

// Version 4.0 Feature Flags
#define ENABLE_DRONE_FLEET 1
#define ENABLE_GLOBAL_NETWORK 1
#define ENABLE_ENHANCED_PREDICTION 1
#define ENABLE_LEO_SATELLITES 1
#define ENABLE_CLIMATE_REFUGES 1
#define ENABLE_WILDLIFE_CORRIDORS 1

// Drone Fleet Configuration
#define MAX_DRONES_PER_FLEET 10
#define MAX_CONCURRENT_MISSIONS 5
#define DRONE_COMMUNICATION_TIMEOUT_MS 300000  // 5 minutes
#define CHARGING_STATION_RADIUS_KM 100.0f

// Global Network Configuration
#define NETWORK_SYNC_INTERVAL_MS 3600000  // 1 hour
#define MAX_NETWORK_NODES 1000
#define CONSERVATION_ALERT_TTL_HOURS 48
#define MIGRATION_PREDICTION_DAYS 30

// Enhanced Prediction Configuration
#define PREDICTION_CONFIDENCE_THRESHOLD 0.75f
#define BEHAVIORAL_ANOMALY_THRESHOLD 0.80f
#define SEASONAL_ADAPTATION_ENABLED 1
#define CLIMATE_IMPACT_MODELING_ENABLED 1

// Satellite Configuration
#define LEO_CONSTELLATION_COUNT 4
#define SATELLITE_HANDOFF_TIMEOUT_MS 30000
#define GLOBAL_DATA_COMPRESSION_ENABLED 1
#define EMERGENCY_SATELLITE_PRIORITY 1

// Conservation Configuration
#define MAX_CONSERVATION_ALERTS 100
#define THREAT_ASSESSMENT_INTERVAL_HOURS 6
#define BIODIVERSITY_UPDATE_INTERVAL_HOURS 24
#define INTERVENTION_APPROVAL_REQUIRED 1

// Performance Configuration
#define V4_MEMORY_RESERVE_KB 512
#define V4_PROCESSING_BUDGET_MS 5000
#define V4_NETWORK_BUFFER_KB 64
#define V4_PREDICTION_CACHE_SIZE 50

#endif // CONFIG_V4_H