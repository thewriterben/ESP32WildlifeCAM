#ifndef DEVICE_PROVISIONING_H
#define DEVICE_PROVISIONING_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "../../config.h"

/**
 * @brief Automated Device Provisioning and Setup System
 * 
 * Provides automated device onboarding, network configuration, and initial
 * setup for production wildlife camera deployments.
 */
class DeviceProvisioning {
public:
    /**
     * @brief Provisioning states
     */
    enum class ProvisioningState {
        UNPROVISIONED = 0,
        SETUP_MODE = 1,
        CONFIGURING = 2,
        PROVISIONED = 3,
        ERROR = 4
    };

    /**
     * @brief Provisioning configuration
     */
    struct ProvisioningConfig {
        String device_name;
        String setup_ssid;
        String setup_password;
        String default_location;
        bool auto_provision_enabled;
        uint32_t setup_timeout_seconds;
        bool require_authentication;
    };

    /**
     * @brief Initialize device provisioning system
     * @param config Provisioning configuration
     * @return true if initialization successful
     */
    static bool initialize(const ProvisioningConfig& config = ProvisioningConfig());

    /**
     * @brief Start provisioning process
     * @return true if provisioning started successfully
     */
    static bool startProvisioning();

    /**
     * @brief Check if device is provisioned
     * @return true if device is fully provisioned
     */
    static bool isProvisioned();

    /**
     * @brief Get current provisioning state
     * @return Current provisioning state
     */
    static ProvisioningState getState() { return current_state_; }

    /**
     * @brief Process provisioning tasks (call in main loop)
     */
    static void process();

    /**
     * @brief Reset provisioning state
     * @return true if reset successful
     */
    static bool resetProvisioning();

    /**
     * @brief Get provisioning status
     * @return JSON object with provisioning status
     */
    static DynamicJsonDocument getProvisioningStatus();

private:
    static ProvisioningConfig config_;
    static ProvisioningState current_state_;
    static WebServer* setup_server_;
    static DNSServer* dns_server_;
    static bool initialized_;
    
    // Web interface handlers
    static void handleRoot();
    static void handleSetup();
    static void handleStatus();
    static void handleNotFound();
    
    // Setup methods
    static bool createSetupAP();
    static bool saveConfiguration(const DynamicJsonDocument& config);
    static void completeProvisioning();
};

#endif // DEVICE_PROVISIONING_H