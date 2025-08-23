/**
 * @file production_deployment_example.cpp
 * @brief Phase 4 Production Deployment Example
 * 
 * Demonstrates complete integration of Phase 4 production features
 * including deployment automation, field optimization, enterprise
 * integration, and security for wildlife camera networks.
 */

#include <Arduino.h>
#include "production/production_manager.h"

// Global variables
bool setup_complete = false;
uint32_t last_status_report = 0;

/**
 * @brief Setup function - Initialize Phase 4 production deployment
 */
void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=== ESP32 Wildlife Camera - Phase 4 Production Deployment ===");
    Serial.println("Initializing production-ready wildlife monitoring system...\n");

    // Create production profile for research deployment scenario
    ProductionManager::ProductionProfile research_profile;
    research_profile.profile_name = "Research_Deployment_v1.0";
    research_profile.deployment_scenario = "large_scale_research";
    research_profile.target_phase = ProductionManager::DeploymentPhase::MONITORING_ACTIVE;
    
    // Enable production features
    research_profile.enable_ota_updates = true;
    research_profile.enable_cloud_sync = true;
    research_profile.enable_environmental_adaptation = true;
    research_profile.enable_advanced_species_detection = true;
    research_profile.enable_data_encryption = true;
    research_profile.enable_audit_logging = true;
    
    // Configure performance parameters
    research_profile.detection_confidence_threshold = 0.75f;
    research_profile.sync_interval_seconds = 300; // 5 minutes
    research_profile.adaptation_interval_seconds = 600; // 10 minutes
    research_profile.power_optimization_level = 7; // High optimization
    
    // Configure conservation parameters
    research_profile.prioritize_endangered_species = true;
    research_profile.enable_conservation_alerts = true;
    research_profile.enable_anti_poaching_mode = false; // Not needed for research
    research_profile.location_privacy_level = 0.8f; // High privacy
    
    // Initialize production manager
    if (ProductionManager::initialize(research_profile)) {
        Serial.println("✓ Production Manager initialized successfully");
        setup_complete = true;
        
        // Print initial system status
        printSystemStatus();
        
    } else {
        Serial.println("✗ Failed to initialize Production Manager");
        Serial.println("System will operate in basic mode only");
    }
    
    Serial.println("\n=== Phase 4 Production Deployment Active ===\n");
}

/**
 * @brief Main loop - Process production systems and demonstrate features
 */
void loop() {
    if (!setup_complete) {
        delay(1000);
        return;
    }
    
    // Process all production subsystems
    ProductionManager::process();
    
    // Periodic status reports
    uint32_t current_time = millis();
    if (current_time - last_status_report > 30000) { // Every 30 seconds for demo
        printSystemStatus();
        demonstrateProductionFeatures();
        last_status_report = current_time;
    }
    
    // Handle any system alerts
    handleSystemAlerts();
    
    delay(1000); // Main loop delay
}

/**
 * @brief Print current system status and statistics
 */
void printSystemStatus() {
    Serial.println("=== PRODUCTION SYSTEM STATUS ===");
    
    auto stats = ProductionManager::getSystemStatistics();
    auto phase = ProductionManager::getCurrentPhase();
    
    Serial.printf("Current Phase: %s\n", getPhaseString(phase).c_str());
    Serial.printf("System Health: %s\n", getHealthString(stats.overall_health).c_str());
    Serial.printf("Uptime: %u seconds (%.1f hours)\n", stats.uptime_seconds, stats.uptime_seconds / 3600.0f);
    Serial.printf("System Efficiency: %.1f%%\n", stats.system_efficiency * 100.0f);
    
    // Subsystem health
    Serial.println("\nSubsystem Health:");
    Serial.printf("  OTA Manager: %s\n", stats.subsystem_health.ota_manager_healthy ? "✓ Healthy" : "✗ Error");
    Serial.printf("  Config Manager: %s\n", stats.subsystem_health.config_manager_healthy ? "✓ Healthy" : "✗ Error");
    Serial.printf("  Environmental Adapter: %s\n", stats.subsystem_health.environmental_adapter_healthy ? "✓ Healthy" : "✗ Error");
    Serial.printf("  Species Detector: %s\n", stats.subsystem_health.species_detector_healthy ? "✓ Healthy" : "✗ Error");
    Serial.printf("  Cloud Integrator: %s\n", stats.subsystem_health.cloud_integrator_healthy ? "✓ Healthy" : "✗ Error");
    Serial.printf("  Data Protector: %s\n", stats.subsystem_health.data_protector_healthy ? "✓ Healthy" : "✗ Error");
    
    // Operational statistics
    Serial.println("\nOperational Statistics:");
    Serial.printf("  Total Detections: %u\n", stats.total_detections);
    Serial.printf("  Successful Transmissions: %u\n", stats.successful_transmissions);
    Serial.printf("  Failed Transmissions: %u\n", stats.failed_transmissions);
    Serial.printf("  Battery Level: %.1f%%\n", stats.battery_level);
    
    Serial.println("================================\n");
}

/**
 * @brief Demonstrate key production features
 */
void demonstrateProductionFeatures() {
    static uint8_t demo_cycle = 0;
    demo_cycle++;
    
    Serial.println("=== PRODUCTION FEATURES DEMONSTRATION ===");
    
    switch (demo_cycle % 6) {
        case 1:
            demonstrateOTAManager();
            break;
        case 2:
            demonstrateConfigManager();
            break;
        case 3:
            demonstrateEnvironmentalAdapter();
            break;
        case 4:
            demonstrateSpeciesDetector();
            break;
        case 5:
            demonstrateCloudIntegrator();
            break;
        case 0:
            demonstrateDataProtector();
            break;
    }
    
    Serial.println("==========================================\n");
}

/**
 * @brief Demonstrate OTA Manager functionality
 */
void demonstrateOTAManager() {
    Serial.println("📡 OTA Manager Demo:");
    
    // Check for updates (simulated)
    Serial.println("  Checking for firmware updates...");
    bool updates_available = OTAManager::checkForUpdates(false);
    
    if (updates_available) {
        Serial.println("  ✓ Firmware update available");
        auto versions = OTAManager::getAvailableVersions();
        Serial.printf("  Available versions: %d\n", versions.size());
    } else {
        Serial.println("  ✓ System up to date");
    }
    
    // Get update statistics
    auto stats = OTAManager::getUpdateStatistics();
    Serial.printf("  Current version: %s\n", stats["current_version"].as<String>().c_str());
    Serial.printf("  Auto-update: %s\n", stats["auto_update_enabled"].as<bool>() ? "Enabled" : "Disabled");
}

/**
 * @brief Demonstrate Config Manager functionality
 */
void demonstrateConfigManager() {
    Serial.println("⚙️ Config Manager Demo:");
    
    // Get current configuration parameter
    String camera_quality = ConfigManager::getParameter(
        ConfigManager::ConfigSection::CAMERA, 
        "jpeg_quality", 
        "12"
    );
    Serial.printf("  Current JPEG quality: %s\n", camera_quality.c_str());
    
    // Get available profiles
    auto profiles = ConfigManager::getAvailableProfiles();
    Serial.printf("  Available profiles: %d\n", profiles.size());
    
    // Export configuration
    String config_export = ConfigManager::exportConfiguration(false);
    Serial.printf("  Configuration size: %d bytes\n", config_export.length());
    
    Serial.println("  ✓ Configuration management active");
}

/**
 * @brief Demonstrate Environmental Adapter functionality
 */
void demonstrateEnvironmentalAdapter() {
    Serial.println("🌡️ Environmental Adapter Demo:");
    
    // Get current environmental conditions
    auto conditions = EnvironmentalAdapter::getCurrentConditions();
    
    if (conditions.data_valid) {
        Serial.printf("  Temperature: %.1f°C\n", conditions.temperature_celsius);
        Serial.printf("  Humidity: %.1f%%\n", conditions.humidity_percent);
        Serial.printf("  Light Level: %.1f lux\n", conditions.light_level_lux);
        
        // Get recommended adaptations
        auto camera_adaptation = EnvironmentalAdapter::getRecommendedCameraAdaptation();
        Serial.printf("  Brightness adjustment: %+d\n", camera_adaptation.brightness_adjustment);
        Serial.printf("  Night mode: %s\n", camera_adaptation.enable_night_mode ? "Enabled" : "Disabled");
        
    } else {
        Serial.println("  Environmental sensors not available (simulated data)");
        Serial.println("  Adaptation system would respond to:");
        Serial.println("    - Temperature changes");
        Serial.println("    - Light level variations");
        Serial.println("    - Weather conditions");
    }
    
    Serial.println("  ✓ Environmental adaptation active");
}

/**
 * @brief Demonstrate Species Detector functionality
 */
void demonstrateSpeciesDetector() {
    Serial.println("🦌 Species Detector Demo:");
    
    // Get detection statistics
    auto stats = SpeciesDetector::getDetectionStatistics(7); // Last 7 days
    
    Serial.printf("  Total detections: %u\n", stats.total_detections);
    Serial.printf("  Unique species: %u\n", stats.unique_species_count);
    Serial.printf("  High confidence: %u\n", stats.high_confidence_detections);
    Serial.printf("  Average confidence: %.1f%%\n", stats.average_confidence * 100.0f);
    
    // Get detectable species
    auto species_list = SpeciesDetector::getDetectableSpecies();
    Serial.printf("  Detectable species: %d types\n", species_list.size());
    
    // Show conservation alerts
    if (stats.conservation_alerts_triggered > 0) {
        Serial.printf("  ⚠️ Conservation alerts: %u\n", stats.conservation_alerts_triggered);
    }
    
    Serial.println("  ✓ Advanced species detection active");
}

/**
 * @brief Demonstrate Cloud Integrator functionality
 */
void demonstrateCloudIntegrator() {
    Serial.println("☁️ Cloud Integrator Demo:");
    
    // Get connection status
    auto status = CloudIntegrator::getConnectionStatus();
    Serial.printf("  Connection status: %s\n", getConnectionStatusString(status).c_str());
    
    // Get sync statistics
    auto sync_stats = CloudIntegrator::getSyncStatistics();
    Serial.printf("  Pending sync items: %u\n", sync_stats.pending_items);
    Serial.printf("  Sync success rate: %.1f%%\n", sync_stats.sync_success_rate * 100.0f);
    Serial.printf("  Total bytes synced: %llu\n", sync_stats.total_bytes_synced);
    
    // Test connectivity
    bool connectivity = CloudIntegrator::testConnectivity();
    Serial.printf("  Cloud connectivity: %s\n", connectivity ? "✓ Available" : "✗ Unavailable");
    
    Serial.println("  ✓ Enterprise cloud integration active");
}

/**
 * @brief Demonstrate Data Protector functionality
 */
void demonstrateDataProtector() {
    Serial.println("🔒 Data Protector Demo:");
    
    // Get protection statistics
    auto stats = DataProtector::getProtectionStatistics();
    Serial.printf("  Encryption: %s\n", stats["encryption_enabled"].as<bool>() ? "✓ Enabled" : "Disabled");
    Serial.printf("  Access control: %s\n", stats["access_control_enabled"].as<bool>() ? "✓ Enabled" : "Disabled");
    Serial.printf("  Audit logging: %s\n", stats["audit_enabled"].as<bool>() ? "✓ Enabled" : "Disabled");
    
    // Demonstrate data encryption
    String test_data = "Wildlife observation: Red fox detected at 14:30";
    String encrypted = DataProtector::encryptData(test_data, DataProtector::DataClassification::CONFIDENTIAL);
    
    if (encrypted.length() > 0) {
        Serial.println("  ✓ Data encryption functional");
        Serial.printf("  Original: %d bytes, Encrypted: %d bytes\n", test_data.length(), encrypted.length());
    }
    
    // Generate secure hash
    String hash = DataProtector::generateSecureHash("system_integrity_check");
    Serial.printf("  Secure hash length: %d characters\n", hash.length());
    
    Serial.println("  ✓ Comprehensive data protection active");
}

/**
 * @brief Handle system alerts
 */
void handleSystemAlerts() {
    auto alerts = ProductionManager::getSystemAlerts();
    
    if (alerts.size() > 0) {
        Serial.println("⚠️ SYSTEM ALERTS:");
        for (const String& alert : alerts) {
            Serial.printf("  %s\n", alert.c_str());
        }
        Serial.println();
    }
}

// Helper functions for string conversion

String getPhaseString(ProductionManager::DeploymentPhase phase) {
    switch (phase) {
        case ProductionManager::DeploymentPhase::INITIALIZATION: return "Initialization";
        case ProductionManager::DeploymentPhase::DEVICE_PROVISIONING: return "Device Provisioning";
        case ProductionManager::DeploymentPhase::FIELD_TESTING: return "Field Testing";
        case ProductionManager::DeploymentPhase::PRODUCTION_DEPLOYMENT: return "Production Deployment";
        case ProductionManager::DeploymentPhase::MONITORING_ACTIVE: return "Monitoring Active";
        case ProductionManager::DeploymentPhase::MAINTENANCE_MODE: return "Maintenance Mode";
        case ProductionManager::DeploymentPhase::DECOMMISSIONING: return "Decommissioning";
        default: return "Unknown";
    }
}

String getHealthString(ProductionManager::HealthStatus health) {
    switch (health) {
        case ProductionManager::HealthStatus::EXCELLENT: return "Excellent";
        case ProductionManager::HealthStatus::GOOD: return "Good";
        case ProductionManager::HealthStatus::WARNING: return "Warning";
        case ProductionManager::HealthStatus::CRITICAL: return "Critical";
        case ProductionManager::HealthStatus::FAILURE: return "Failure";
        default: return "Unknown";
    }
}

String getConnectionStatusString(CloudIntegrator::ConnectionStatus status) {
    switch (status) {
        case CloudIntegrator::ConnectionStatus::DISCONNECTED: return "Disconnected";
        case CloudIntegrator::ConnectionStatus::CONNECTING: return "Connecting";
        case CloudIntegrator::ConnectionStatus::CONNECTED: return "Connected";
        case CloudIntegrator::ConnectionStatus::SYNCING: return "Syncing";
        case CloudIntegrator::ConnectionStatus::ERROR: return "Error";
        case CloudIntegrator::ConnectionStatus::THROTTLED: return "Throttled";
        default: return "Unknown";
    }
}