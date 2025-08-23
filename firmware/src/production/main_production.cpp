/**
 * @file main_production.cpp
 * @brief Main application entry point with Phase 4 Production Integration
 * 
 * Demonstrates integration of Phase 4 production features with existing
 * wildlife camera system, providing a complete production-ready solution.
 */

#include <Arduino.h>

// Core system includes
#include "camera_handler.h"
#include "motion_filter.h"
#include "power_manager.h"
#include "wifi_manager.h"

// Phase 4 Production includes
#include "production/production_manager.h"
#include "production/deployment/ota_manager.h"
#include "production/monitoring/conservation_alerts.h"
#include "production/security/audit_logger.h"

// System state
bool system_initialized = false;
uint32_t last_system_check = 0;
uint32_t boot_timestamp = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    boot_timestamp = millis();
    
    Serial.println("\n=== ESP32 Wildlife Camera - Production System v2.0 ===");
    Serial.println("Initializing with Phase 4 Production Features...\n");
    
    // Initialize core systems first
    if (!initializeCoreSystem()) {
        Serial.println("CRITICAL: Core system initialization failed!");
        return;
    }
    
    // Initialize Phase 4 production system
    if (!initializeProductionSystem()) {
        Serial.println("WARNING: Production system initialization failed, running in basic mode");
    }
    
    // Log system startup
    AuditLogger::logEvent(
        AuditLogger::EventType::SYSTEM_STARTUP,
        AuditLogger::Severity::INFO,
        "system",
        "startup",
        "main_system",
        "success",
        "Production wildlife camera system started"
    );
    
    system_initialized = true;
    Serial.println("=== System Initialization Complete ===\n");
}

void loop() {
    if (!system_initialized) {
        delay(1000);
        return;
    }
    
    // Process core systems
    processCoreSystem();
    
    // Process Phase 4 production systems
    processProductionSystem();
    
    // Periodic system health checks
    uint32_t current_time = millis();
    if (current_time - last_system_check > 60000) { // Every minute
        performSystemHealthCheck();
        last_system_check = current_time;
    }
    
    delay(100); // Small delay for system stability
}

/**
 * @brief Initialize core wildlife camera systems
 */
bool initializeCoreSystem() {
    Serial.println("Initializing core systems...");
    
    // Initialize camera
    if (!CameraHandler::initialize()) {
        Serial.println("ERROR: Camera initialization failed");
        return false;
    }
    Serial.println("✓ Camera system initialized");
    
    // Initialize motion detection
    if (!MotionFilter::initialize()) {
        Serial.println("ERROR: Motion filter initialization failed");
        return false;
    }
    Serial.println("✓ Motion detection initialized");
    
    // Initialize power management
    if (!PowerManager::initialize()) {
        Serial.println("ERROR: Power manager initialization failed");
        return false;
    }
    Serial.println("✓ Power management initialized");
    
    // Initialize WiFi (optional for field deployment)
    WiFiManager::initialize();
    Serial.println("✓ WiFi manager initialized");
    
    return true;
}

/**
 * @brief Initialize Phase 4 production systems
 */
bool initializeProductionSystem() {
    Serial.println("Initializing Phase 4 production systems...");
    
    // Initialize audit logger first for security tracking
    AuditLogger::AuditConfig audit_config;
    audit_config.enabled = true;
    audit_config.log_to_file = true;
    audit_config.real_time_monitoring = true;
    audit_config.retention_days = 365;
    audit_config.minimum_severity = AuditLogger::Severity::INFO;
    
    if (!AuditLogger::initialize(audit_config)) {
        Serial.println("WARNING: Audit logger initialization failed");
    } else {
        Serial.println("✓ Audit logging system initialized");
    }
    
    // Initialize conservation alerts
    if (!ConservationAlerts::initialize(true)) {
        Serial.println("WARNING: Conservation alerts initialization failed");
    } else {
        Serial.println("✓ Conservation alert system initialized");
    }
    
    // Create production profile for field deployment
    ProductionManager::ProductionProfile field_profile;
    field_profile.profile_name = "Field_Deployment_v2.0";
    field_profile.deployment_scenario = "conservation_monitoring";
    field_profile.target_phase = ProductionManager::DeploymentPhase::MONITORING_ACTIVE;
    
    // Configure for field deployment
    field_profile.enable_ota_updates = true;
    field_profile.enable_cloud_sync = false; // Start offline for field deployment
    field_profile.enable_environmental_adaptation = true;
    field_profile.enable_advanced_species_detection = true;
    field_profile.enable_data_encryption = true;
    field_profile.enable_audit_logging = true;
    
    // Set conservation-focused parameters
    field_profile.detection_confidence_threshold = 0.7f;
    field_profile.prioritize_endangered_species = true;
    field_profile.enable_conservation_alerts = true;
    field_profile.location_privacy_level = 0.9f; // High privacy for conservation
    
    // Initialize production manager
    if (!ProductionManager::initialize(field_profile)) {
        Serial.println("ERROR: Production manager initialization failed");
        return false;
    }
    Serial.println("✓ Production management system initialized");
    
    return true;
}

/**
 * @brief Process core wildlife camera systems
 */
void processCoreSystem() {
    // Process camera system
    CameraHandler::process();
    
    // Process motion detection
    if (MotionFilter::motionDetected()) {
        // Wildlife detected - capture image
        if (CameraHandler::captureImage()) {
            String image_path = CameraHandler::getLastImagePath();
            
            // Log wildlife detection event
            AuditLogger::logWildlifeEvent(
                "Unknown", // Species will be determined by AI processing
                0.8f,      // Motion detection confidence
                "field_location_001",
                image_path,
                "Unknown"
            );
            
            Serial.printf("Wildlife motion detected - Image saved: %s\n", image_path.c_str());
        }
    }
    
    // Process power management
    PowerManager::process();
    
    // Check battery levels and trigger alerts if needed
    float battery_level = PowerManager::getBatteryVoltage();
    if (battery_level < 3.0f) {
        ConservationAlerts::triggerAlert(
            ConservationAlerts::AlertType::EQUIPMENT_TAMPERING,
            ConservationAlerts::AlertPriority::HIGH,
            "",
            "field_location_001",
            "Critical battery level detected: " + String(battery_level, 2) + "V",
            1.0f
        );
    }
    
    // Process WiFi if enabled
    WiFiManager::process();
}

/**
 * @brief Process Phase 4 production systems
 */
void processProductionSystem() {
    // Process production manager (coordinates all subsystems)
    ProductionManager::process();
    
    // Process conservation alerts
    ConservationAlerts::process();
    
    // Process audit logger
    AuditLogger::process();
    
    // Check for system alerts
    auto alerts = ProductionManager::getSystemAlerts();
    for (const String& alert : alerts) {
        Serial.printf("PRODUCTION ALERT: %s\n", alert.c_str());
        
        // Log production alerts to audit system
        AuditLogger::logEvent(
            AuditLogger::EventType::ALERT_TRIGGERED,
            AuditLogger::Severity::WARNING,
            "system",
            "alert_generated",
            "production_system",
            "active",
            alert
        );
    }
}

/**
 * @brief Perform comprehensive system health check
 */
void performSystemHealthCheck() {
    Serial.println("--- System Health Check ---");
    
    // Get production system statistics
    auto stats = ProductionManager::getSystemStatistics();
    
    Serial.printf("System Uptime: %u seconds\n", stats.uptime_seconds);
    Serial.printf("System Health: %s\n", getHealthStatusString(stats.overall_health).c_str());
    Serial.printf("Deployment Phase: %s\n", getPhaseString(stats.current_phase).c_str());
    Serial.printf("System Efficiency: %.1f%%\n", stats.system_efficiency * 100.0f);
    
    // Check core system health
    bool camera_healthy = CameraHandler::isInitialized();
    bool motion_healthy = MotionFilter::isEnabled();
    bool power_healthy = PowerManager::getBatteryVoltage() > 2.8f;
    
    Serial.printf("Camera System: %s\n", camera_healthy ? "✓ Healthy" : "✗ Error");
    Serial.printf("Motion Detection: %s\n", motion_healthy ? "✓ Healthy" : "✗ Error");
    Serial.printf("Power System: %s (%.2fV)\n", 
                  power_healthy ? "✓ Healthy" : "✗ Low Battery", 
                  PowerManager::getBatteryVoltage());
    
    // Log health check results
    AuditLogger::logEvent(
        AuditLogger::EventType::MAINTENANCE_ACTION,
        AuditLogger::Severity::INFO,
        "system",
        "health_check",
        "all_systems",
        "completed",
        "Periodic system health verification"
    );
    
    Serial.println("--- Health Check Complete ---\n");
}

// Helper functions for string conversion
String getHealthStatusString(ProductionManager::HealthStatus health) {
    switch (health) {
        case ProductionManager::HealthStatus::EXCELLENT: return "Excellent";
        case ProductionManager::HealthStatus::GOOD: return "Good";
        case ProductionManager::HealthStatus::WARNING: return "Warning";
        case ProductionManager::HealthStatus::CRITICAL: return "Critical";
        case ProductionManager::HealthStatus::FAILURE: return "Failure";
        default: return "Unknown";
    }
}

String getPhaseString(ProductionManager::DeploymentPhase phase) {
    switch (phase) {
        case ProductionManager::DeploymentPhase::INITIALIZATION: return "Initialization";
        case ProductionManager::DeploymentPhase::DEVICE_PROVISIONING: return "Device Provisioning";
        case ProductionManager::DeploymentPhase::FIELD_TESTING: return "Field Testing";
        case ProductionManager::DeploymentPhase::PRODUCTION_DEPLOYMENT: return "Production Deployment";
        case ProductionManager::DeploymentPhase::MONITORING_ACTIVE: return "Monitoring Active";
        case ProductionManager::DeploymentPhase::MAINTENANCE_MODE: return "Maintenance Mode";
        default: return "Unknown";
    }
}