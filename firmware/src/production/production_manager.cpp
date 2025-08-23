#include "production_manager.h"

// Static member initialization
ProductionManager::ProductionProfile ProductionManager::current_profile_;
ProductionManager::DeploymentPhase ProductionManager::current_phase_ = DeploymentPhase::INITIALIZATION;
ProductionManager::HealthStatus ProductionManager::current_health_ = HealthStatus::GOOD;
ProductionManager::SystemStatistics ProductionManager::statistics_;
uint32_t ProductionManager::initialization_time_ = 0;
uint32_t ProductionManager::last_health_check_ = 0;
uint32_t ProductionManager::last_maintenance_ = 0;
bool ProductionManager::initialized_ = false;
std::vector<String> ProductionManager::system_alerts_;
void (*ProductionManager::alert_callback_)(const String&, const String&) = nullptr;

// Global instance
ProductionManager* g_productionManager = nullptr;

bool ProductionManager::initialize(const ProductionProfile& profile) {
    if (initialized_) {
        return true;
    }

    Serial.println("Initializing Production Manager for Phase 4 deployment...");

    current_profile_ = profile;
    initialization_time_ = millis();
    current_phase_ = DeploymentPhase::INITIALIZATION;

    // Initialize statistics
    statistics_.current_phase = current_phase_;
    statistics_.overall_health = HealthStatus::GOOD;
    statistics_.uptime_seconds = 0;
    statistics_.total_detections = 0;
    statistics_.successful_transmissions = 0;
    statistics_.failed_transmissions = 0;
    statistics_.battery_level = 100.0f;
    statistics_.system_efficiency = 1.0f;
    statistics_.last_maintenance_timestamp = millis() / 1000;

    // Initialize subsystems based on profile
    if (!initializeSubsystems()) {
        Serial.println("ERROR: Failed to initialize production subsystems");
        return false;
    }

    initialized_ = true;
    Serial.printf("Production Manager initialized with profile: %s\n", profile.profile_name.c_str());

    // Advance to device provisioning phase
    if (profile.target_phase >= DeploymentPhase::DEVICE_PROVISIONING) {
        advanceToNextPhase();
    }

    return true;
}

bool ProductionManager::initializeSubsystems() {
    bool all_success = true;

    // Initialize OTA Manager if enabled
    if (current_profile_.enable_ota_updates) {
        OTAManager::OTAConfig ota_config;
        ota_config.auto_update_enabled = false; // Conservative for production
        ota_config.require_manual_approval = true;
        
        if (OTAManager::initialize(ota_config)) {
            Serial.println("OTA Manager initialized successfully");
            statistics_.subsystem_health.ota_manager_healthy = true;
        } else {
            Serial.println("WARNING: OTA Manager initialization failed");
            statistics_.subsystem_health.ota_manager_healthy = false;
            all_success = false;
        }
    }

    // Initialize Config Manager
    if (ConfigManager::initialize(current_profile_.enable_cloud_sync)) {
        Serial.println("Config Manager initialized successfully");
        statistics_.subsystem_health.config_manager_healthy = true;
    } else {
        Serial.println("ERROR: Config Manager initialization failed");
        statistics_.subsystem_health.config_manager_healthy = false;
        all_success = false;
    }

    // Initialize Environmental Adapter if enabled
    if (current_profile_.enable_environmental_adaptation) {
        if (EnvironmentalAdapter::initialize(true)) {
            Serial.println("Environmental Adapter initialized successfully");
            statistics_.subsystem_health.environmental_adapter_healthy = true;
            
            // Set adaptation parameters
            EnvironmentalAdapter::setAutoAdaptation(true, current_profile_.adaptation_interval_seconds);
            EnvironmentalAdapter::setAdaptationSensitivity(0.7f); // Moderate sensitivity
        } else {
            Serial.println("WARNING: Environmental Adapter initialization failed");
            statistics_.subsystem_health.environmental_adapter_healthy = false;
        }
    }

    // Initialize Species Detector if enabled
    if (current_profile_.enable_advanced_species_detection) {
        if (SpeciesDetector::initialize(true, "/models/")) {
            Serial.println("Species Detector initialized successfully");
            statistics_.subsystem_health.species_detector_healthy = true;
            
            // Set detection parameters
            SpeciesDetector::setConfidenceThreshold(
                current_profile_.detection_confidence_threshold,
                current_profile_.prioritize_endangered_species ? 0.3f : 0.5f
            );
        } else {
            Serial.println("WARNING: Species Detector initialization failed");
            statistics_.subsystem_health.species_detector_healthy = false;
        }
    }

    // Initialize Cloud Integrator if enabled
    if (current_profile_.enable_cloud_sync) {
        CloudIntegrator::CloudConfig cloud_config;
        cloud_config.platform = CloudIntegrator::CloudPlatform::CUSTOM_API;
        cloud_config.sync_mode = CloudIntegrator::SyncMode::SMART;
        cloud_config.batch_interval_seconds = current_profile_.sync_interval_seconds;
        cloud_config.compress_data = true;
        cloud_config.encrypt_data = current_profile_.enable_data_encryption;
        
        if (CloudIntegrator::initialize(cloud_config)) {
            Serial.println("Cloud Integrator initialized successfully");
            statistics_.subsystem_health.cloud_integrator_healthy = true;
        } else {
            Serial.println("WARNING: Cloud Integrator initialization failed");
            statistics_.subsystem_health.cloud_integrator_healthy = false;
        }
    }

    // Initialize Data Protector if enabled
    if (current_profile_.enable_data_encryption || current_profile_.enable_audit_logging) {
        if (DataProtector::initialize(true)) {
            Serial.println("Data Protector initialized successfully");
            statistics_.subsystem_health.data_protector_healthy = true;
            
            // Configure protection settings
            DataProtector::setProtectionSettings(
                current_profile_.enable_data_encryption,
                true, // Always enable access control
                current_profile_.enable_audit_logging
            );
        } else {
            Serial.println("WARNING: Data Protector initialization failed");
            statistics_.subsystem_health.data_protector_healthy = false;
        }
    }

    return all_success;
}

void ProductionManager::process() {
    if (!initialized_) {
        return;
    }

    // Update uptime
    statistics_.uptime_seconds = (millis() - initialization_time_) / 1000;

    // Process all enabled subsystems
    processSubsystems();

    // Periodic health checks
    uint32_t current_time = millis();
    if (current_time - last_health_check_ > 60000) { // Every minute
        current_health_ = performHealthCheck();
        statistics_.overall_health = current_health_;
        last_health_check_ = current_time;
    }

    // Check for system alerts
    checkForAlerts();

    // Periodic maintenance
    if (current_time - last_maintenance_ > 3600000) { // Every hour
        performMaintenance("routine");
        last_maintenance_ = current_time;
    }

    // Update statistics
    updateStatistics();
}

void ProductionManager::processSubsystems() {
    // Process each enabled subsystem
    if (current_profile_.enable_ota_updates && statistics_.subsystem_health.ota_manager_healthy) {
        OTAManager::process();
    }

    if (statistics_.subsystem_health.config_manager_healthy) {
        ConfigManager::process();
    }

    if (current_profile_.enable_environmental_adaptation && statistics_.subsystem_health.environmental_adapter_healthy) {
        EnvironmentalAdapter::process();
    }

    if (current_profile_.enable_advanced_species_detection && statistics_.subsystem_health.species_detector_healthy) {
        SpeciesDetector::process();
    }

    if (current_profile_.enable_cloud_sync && statistics_.subsystem_health.cloud_integrator_healthy) {
        CloudIntegrator::process();
    }

    if ((current_profile_.enable_data_encryption || current_profile_.enable_audit_logging) && 
        statistics_.subsystem_health.data_protector_healthy) {
        DataProtector::process();
    }
}

ProductionManager::HealthStatus ProductionManager::performHealthCheck() {
    int healthy_subsystems = 0;
    int total_enabled_subsystems = 0;

    // Count healthy subsystems
    if (current_profile_.enable_ota_updates) {
        total_enabled_subsystems++;
        if (statistics_.subsystem_health.ota_manager_healthy) healthy_subsystems++;
    }

    if (true) { // Config manager always enabled
        total_enabled_subsystems++;
        if (statistics_.subsystem_health.config_manager_healthy) healthy_subsystems++;
    }

    if (current_profile_.enable_environmental_adaptation) {
        total_enabled_subsystems++;
        if (statistics_.subsystem_health.environmental_adapter_healthy) healthy_subsystems++;
    }

    if (current_profile_.enable_advanced_species_detection) {
        total_enabled_subsystems++;
        if (statistics_.subsystem_health.species_detector_healthy) healthy_subsystems++;
    }

    if (current_profile_.enable_cloud_sync) {
        total_enabled_subsystems++;
        if (statistics_.subsystem_health.cloud_integrator_healthy) healthy_subsystems++;
    }

    if (current_profile_.enable_data_encryption || current_profile_.enable_audit_logging) {
        total_enabled_subsystems++;
        if (statistics_.subsystem_health.data_protector_healthy) healthy_subsystems++;
    }

    // Calculate health based on subsystem status
    float health_ratio = (float)healthy_subsystems / (float)total_enabled_subsystems;

    if (health_ratio >= 0.95f) {
        return HealthStatus::EXCELLENT;
    } else if (health_ratio >= 0.8f) {
        return HealthStatus::GOOD;
    } else if (health_ratio >= 0.6f) {
        return HealthStatus::WARNING;
    } else if (health_ratio >= 0.3f) {
        return HealthStatus::CRITICAL;
    } else {
        return HealthStatus::FAILURE;
    }
}

bool ProductionManager::advanceToNextPhase() {
    DeploymentPhase next_phase;

    switch (current_phase_) {
        case DeploymentPhase::INITIALIZATION:
            next_phase = DeploymentPhase::DEVICE_PROVISIONING;
            break;
        case DeploymentPhase::DEVICE_PROVISIONING:
            next_phase = DeploymentPhase::FIELD_TESTING;
            break;
        case DeploymentPhase::FIELD_TESTING:
            next_phase = DeploymentPhase::PRODUCTION_DEPLOYMENT;
            break;
        case DeploymentPhase::PRODUCTION_DEPLOYMENT:
            next_phase = DeploymentPhase::MONITORING_ACTIVE;
            break;
        case DeploymentPhase::MONITORING_ACTIVE:
            // Already at target phase
            return true;
        default:
            return false;
    }

    // Don't advance beyond target phase
    if (next_phase > current_profile_.target_phase) {
        return true;
    }

    current_phase_ = next_phase;
    statistics_.current_phase = current_phase_;

    Serial.printf("Advanced to deployment phase: %s\n", phaseToString(current_phase_).c_str());
    
    return true;
}

ProductionManager::SystemStatistics ProductionManager::getSystemStatistics() {
    return statistics_;
}

void ProductionManager::updateStatistics() {
    // Update system efficiency based on subsystem health
    statistics_.system_efficiency = calculateSystemEfficiency();
    
    // Update detection count if species detector is active
    if (current_profile_.enable_advanced_species_detection && statistics_.subsystem_health.species_detector_healthy) {
        auto detector_stats = SpeciesDetector::getDetectionStatistics(1);
        statistics_.total_detections = detector_stats.total_detections;
    }
    
    // Update transmission statistics if cloud sync is active
    if (current_profile_.enable_cloud_sync && statistics_.subsystem_health.cloud_integrator_healthy) {
        auto cloud_stats = CloudIntegrator::getSyncStatistics();
        statistics_.successful_transmissions = cloud_stats.successful_syncs;
        statistics_.failed_transmissions = cloud_stats.failed_syncs;
    }
}

void ProductionManager::checkForAlerts() {
    // Check for system health degradation
    if (current_health_ == HealthStatus::CRITICAL) {
        addAlert("SYSTEM_CRITICAL", "System health is critical - multiple subsystems failing");
    } else if (current_health_ == HealthStatus::FAILURE) {
        addAlert("SYSTEM_FAILURE", "System failure detected - immediate intervention required");
    }

    // Check battery level (placeholder - would integrate with actual power management)
    if (statistics_.battery_level < 10.0f) {
        addAlert("BATTERY_CRITICAL", "Battery level critically low");
    }

    // Remove expired alerts
    removeExpiredAlerts();
}

DynamicJsonDocument ProductionManager::generateDeploymentReport(bool include_detailed_logs) {
    DynamicJsonDocument report(4096);

    // Basic system information
    report["timestamp"] = millis() / 1000;
    report["uptime_seconds"] = statistics_.uptime_seconds;
    report["current_phase"] = phaseToString(current_phase_);
    report["overall_health"] = healthToString(current_health_);

    // Profile information
    report["profile"]["name"] = current_profile_.profile_name;
    report["profile"]["scenario"] = current_profile_.deployment_scenario;
    report["profile"]["target_phase"] = phaseToString(current_profile_.target_phase);

    // System statistics
    report["statistics"]["total_detections"] = statistics_.total_detections;
    report["statistics"]["successful_transmissions"] = statistics_.successful_transmissions;
    report["statistics"]["failed_transmissions"] = statistics_.failed_transmissions;
    report["statistics"]["system_efficiency"] = statistics_.system_efficiency;

    // Subsystem health
    JsonObject subsystems = report.createNestedObject("subsystem_health");
    subsystems["ota_manager"] = statistics_.subsystem_health.ota_manager_healthy;
    subsystems["config_manager"] = statistics_.subsystem_health.config_manager_healthy;
    subsystems["environmental_adapter"] = statistics_.subsystem_health.environmental_adapter_healthy;
    subsystems["species_detector"] = statistics_.subsystem_health.species_detector_healthy;
    subsystems["cloud_integrator"] = statistics_.subsystem_health.cloud_integrator_healthy;
    subsystems["data_protector"] = statistics_.subsystem_health.data_protector_healthy;

    // Current alerts
    JsonArray alerts = report.createNestedArray("alerts");
    for (const String& alert : system_alerts_) {
        alerts.add(alert);
    }

    return report;
}

// Helper methods

String ProductionManager::phaseToString(DeploymentPhase phase) {
    switch (phase) {
        case DeploymentPhase::INITIALIZATION: return "INITIALIZATION";
        case DeploymentPhase::DEVICE_PROVISIONING: return "DEVICE_PROVISIONING";
        case DeploymentPhase::FIELD_TESTING: return "FIELD_TESTING";
        case DeploymentPhase::PRODUCTION_DEPLOYMENT: return "PRODUCTION_DEPLOYMENT";
        case DeploymentPhase::MONITORING_ACTIVE: return "MONITORING_ACTIVE";
        case DeploymentPhase::MAINTENANCE_MODE: return "MAINTENANCE_MODE";
        case DeploymentPhase::DECOMMISSIONING: return "DECOMMISSIONING";
        default: return "UNKNOWN";
    }
}

String ProductionManager::healthToString(HealthStatus health) {
    switch (health) {
        case HealthStatus::EXCELLENT: return "EXCELLENT";
        case HealthStatus::GOOD: return "GOOD";
        case HealthStatus::WARNING: return "WARNING";
        case HealthStatus::CRITICAL: return "CRITICAL";
        case HealthStatus::FAILURE: return "FAILURE";
        default: return "UNKNOWN";
    }
}

void ProductionManager::addAlert(const String& alert_type, const String& message) {
    String alert = alert_type + ": " + message + " (Time: " + String(millis() / 1000) + ")";
    system_alerts_.push_back(alert);
    
    // Limit alert history
    if (system_alerts_.size() > 50) {
        system_alerts_.erase(system_alerts_.begin());
    }
    
    // Call alert callback if registered
    if (alert_callback_) {
        alert_callback_(alert_type, message);
    }
    
    Serial.printf("PRODUCTION ALERT: %s\n", alert.c_str());
}

void ProductionManager::removeExpiredAlerts() {
    // Simple implementation - in production, would track alert timestamps
    // and remove alerts older than a certain threshold
}

float ProductionManager::calculateSystemEfficiency() {
    // Calculate efficiency based on successful operations vs failures
    uint32_t total_operations = statistics_.successful_transmissions + statistics_.failed_transmissions;
    if (total_operations == 0) {
        return 1.0f; // No operations yet, assume perfect efficiency
    }
    
    return (float)statistics_.successful_transmissions / (float)total_operations;
}

bool ProductionManager::performMaintenance(const String& maintenance_type) {
    Serial.printf("Performing %s maintenance...\n", maintenance_type.c_str());
    
    // Placeholder for maintenance tasks
    // In production, this would include:
    // - Log rotation
    // - Memory cleanup
    // - File system cleanup
    // - Sensor calibration checks
    // - Communication health checks
    
    statistics_.last_maintenance_timestamp = millis() / 1000;
    return true;
}