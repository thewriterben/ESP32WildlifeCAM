#ifndef CONSERVATION_ALERTS_H
#define CONSERVATION_ALERTS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "../../config.h"

/**
 * @brief Conservation Alert System for Wildlife Protection
 * 
 * Provides real-time conservation alerts for endangered species detection,
 * anti-poaching activities, and critical conservation events.
 */
class ConservationAlerts {
public:
    /**
     * @brief Alert priority levels
     */
    enum class AlertPriority {
        LOW = 0,
        MEDIUM = 1,
        HIGH = 2,
        CRITICAL = 3,
        EMERGENCY = 4
    };

    /**
     * @brief Alert types
     */
    enum class AlertType {
        ENDANGERED_SPECIES = 0,
        POACHING_ACTIVITY = 1,
        HABITAT_DISTURBANCE = 2,
        POPULATION_THRESHOLD = 3,
        BEHAVIORAL_ANOMALY = 4,
        EQUIPMENT_TAMPERING = 5,
        UNAUTHORIZED_ACCESS = 6,
        CONSERVATION_MILESTONE = 7
    };

    /**
     * @brief Conservation alert structure
     */
    struct ConservationAlert {
        String alert_id;
        AlertType type;
        AlertPriority priority;
        String species_name;
        String location_id;
        float latitude;
        float longitude;
        String description;
        uint32_t timestamp;
        String detection_confidence;
        String image_path;
        String audio_path;
        bool verified;
        bool responded;
        String response_action;
        uint32_t response_timestamp;
    };

    /**
     * @brief Initialize conservation alert system
     * @param enable_real_time Enable real-time alert processing
     * @return true if initialization successful
     */
    static bool initialize(bool enable_real_time = true);

    /**
     * @brief Trigger conservation alert
     * @param type Alert type
     * @param priority Alert priority
     * @param species Species involved (if applicable)
     * @param location Location identifier
     * @param description Alert description
     * @param confidence Detection confidence (0.0-1.0)
     * @return Alert ID for tracking
     */
    static String triggerAlert(AlertType type, AlertPriority priority, 
                              const String& species, const String& location,
                              const String& description, float confidence = 1.0f);

    /**
     * @brief Process conservation alerts (call in main loop)
     */
    static void process();

    /**
     * @brief Get active alerts
     * @param priority_filter Filter by priority level
     * @return Vector of active conservation alerts
     */
    static std::vector<ConservationAlert> getActiveAlerts(AlertPriority priority_filter = AlertPriority::LOW);

    /**
     * @brief Acknowledge alert
     * @param alert_id Alert ID to acknowledge
     * @param response_action Action taken in response
     * @return true if acknowledgment successful
     */
    static bool acknowledgeAlert(const String& alert_id, const String& response_action);

    /**
     * @brief Get alert statistics
     * @param days_back Number of days to include in statistics
     * @return JSON object with alert statistics
     */
    static DynamicJsonDocument getAlertStatistics(uint16_t days_back = 30);

private:
    static std::vector<ConservationAlert> active_alerts_;
    static std::vector<ConservationAlert> alert_history_;
    static bool real_time_enabled_;
    static bool initialized_;
    
    // Alert processing methods
    static void processEndangeredSpeciesAlert(const ConservationAlert& alert);
    static void processPoachingAlert(const ConservationAlert& alert);
    static void sendAlert(const ConservationAlert& alert);
    static String generateAlertId();
    static String alertTypeToString(AlertType type);
    static String priorityToString(AlertPriority priority);
};

#endif // CONSERVATION_ALERTS_H