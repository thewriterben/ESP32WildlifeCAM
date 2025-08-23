#ifndef ENVIRONMENTAL_ADAPTER_H
#define ENVIRONMENTAL_ADAPTER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "../../config.h"

/**
 * @brief Environmental Adaptation System for Field Conditions
 * 
 * Automatically adapts camera and system parameters based on environmental
 * conditions to optimize performance in real-world deployment scenarios.
 */
class EnvironmentalAdapter {
public:
    /**
     * @brief Environmental condition types
     */
    enum class ConditionType {
        TEMPERATURE = 0,
        HUMIDITY = 1,
        WIND_SPEED = 2,
        PRECIPITATION = 3,
        LIGHT_LEVEL = 4,
        BAROMETRIC_PRESSURE = 5,
        SOLAR_IRRADIANCE = 6
    };

    /**
     * @brief Weather condition severity levels
     */
    enum class WeatherSeverity {
        NORMAL = 0,
        MILD = 1,
        MODERATE = 2,
        SEVERE = 3,
        EXTREME = 4
    };

    /**
     * @brief Environmental sensor data
     */
    struct EnvironmentalData {
        float temperature_celsius;
        float humidity_percent;
        float wind_speed_kmh;
        float precipitation_mmh;
        float light_level_lux;
        float pressure_hpa;
        float solar_irradiance_wm2;
        uint32_t timestamp;
        bool data_valid;
    };

    /**
     * @brief Adaptation strategy configuration
     */
    struct AdaptationStrategy {
        String name;
        String description;
        ConditionType trigger_condition;
        float threshold_value;
        WeatherSeverity severity_level;
        std::vector<String> parameter_adjustments;
        bool requires_immediate_action;
        float effectiveness_score;
    };

    /**
     * @brief Camera adaptation parameters
     */
    struct CameraAdaptation {
        int brightness_adjustment;
        int contrast_adjustment;
        int saturation_adjustment;
        int exposure_adjustment;
        int iso_adjustment;
        bool enable_night_mode;
        bool enable_weather_protection;
        float motion_sensitivity_multiplier;
    };

    /**
     * @brief Power adaptation parameters
     */
    struct PowerAdaptation {
        float sleep_duration_multiplier;
        bool enable_aggressive_power_saving;
        float cpu_frequency_multiplier;
        bool disable_non_essential_features;
        float battery_threshold_adjustment;
        bool optimize_for_solar_charging;
    };

    /**
     * @brief Initialize environmental adapter
     * @param enable_sensors Enable environmental sensor monitoring
     * @return true if initialization successful
     */
    static bool initialize(bool enable_sensors = true);

    /**
     * @brief Update environmental data from sensors
     * @param force_update Force sensor reading even if within interval
     * @return true if data updated successfully
     */
    static bool updateEnvironmentalData(bool force_update = false);

    /**
     * @brief Get current environmental conditions
     * @return Current environmental sensor data
     */
    static EnvironmentalData getCurrentConditions();

    /**
     * @brief Analyze environmental conditions and determine adaptations needed
     * @param conditions Environmental data to analyze
     * @return Vector of recommended adaptation strategies
     */
    static std::vector<AdaptationStrategy> analyzeConditions(const EnvironmentalData& conditions);

    /**
     * @brief Apply camera adaptations based on environmental conditions
     * @param adaptation Camera adaptation parameters
     * @return true if adaptations applied successfully
     */
    static bool applyCameraAdaptation(const CameraAdaptation& adaptation);

    /**
     * @brief Apply power management adaptations
     * @param adaptation Power adaptation parameters
     * @return true if adaptations applied successfully
     */
    static bool applyPowerAdaptation(const PowerAdaptation& adaptation);

    /**
     * @brief Get recommended camera adaptations for current conditions
     * @return Camera adaptation parameters
     */
    static CameraAdaptation getRecommendedCameraAdaptation();

    /**
     * @brief Get recommended power adaptations for current conditions
     * @return Power adaptation parameters
     */
    static PowerAdaptation getRecommendedPowerAdaptation();

    /**
     * @brief Enable/disable automatic adaptation
     * @param enabled Enable automatic environmental adaptation
     * @param adaptation_interval_seconds Interval between adaptation checks
     */
    static void setAutoAdaptation(bool enabled, uint32_t adaptation_interval_seconds = 300);

    /**
     * @brief Set adaptation sensitivity level
     * @param sensitivity Sensitivity level (0.0 = conservative, 1.0 = aggressive)
     */
    static void setAdaptationSensitivity(float sensitivity);

    /**
     * @brief Process environmental adaptation (call in main loop)
     */
    static void process();

    /**
     * @brief Get adaptation statistics
     * @return JSON object with adaptation statistics and history
     */
    static DynamicJsonDocument getAdaptationStatistics();

    /**
     * @brief Register custom adaptation strategy
     * @param strategy Custom adaptation strategy
     * @return true if registered successfully
     */
    static bool registerAdaptationStrategy(const AdaptationStrategy& strategy);

    /**
     * @brief Get weather severity level for current conditions
     * @return Current weather severity assessment
     */
    static WeatherSeverity getCurrentWeatherSeverity();

    /**
     * @brief Predict environmental conditions based on historical data
     * @param hours_ahead Hours to predict ahead
     * @return Predicted environmental conditions
     */
    static EnvironmentalData predictConditions(uint8_t hours_ahead);

    /**
     * @brief Enable/disable specific environmental sensors
     * @param condition_type Type of environmental condition
     * @param enabled Enable/disable sensor
     */
    static void setSensorEnabled(ConditionType condition_type, bool enabled);

    /**
     * @brief Calibrate environmental sensors
     * @param condition_type Type of sensor to calibrate
     * @param reference_value Known reference value for calibration
     * @return true if calibration successful
     */
    static bool calibrateSensor(ConditionType condition_type, float reference_value);

    /**
     * @brief Export environmental data history
     * @param hours_back Hours of history to export
     * @return JSON array with historical environmental data
     */
    static DynamicJsonDocument exportEnvironmentalHistory(uint16_t hours_back = 24);

private:
    static EnvironmentalData current_conditions_;
    static CameraAdaptation current_camera_adaptation_;
    static PowerAdaptation current_power_adaptation_;
    static bool auto_adaptation_enabled_;
    static uint32_t adaptation_interval_;
    static uint32_t last_adaptation_time_;
    static uint32_t last_sensor_update_;
    static float adaptation_sensitivity_;
    static bool sensors_enabled_;
    static std::vector<AdaptationStrategy> custom_strategies_;
    static std::vector<EnvironmentalData> history_buffer_;
    static bool initialized_;

    // Sensor interface methods
    static float readTemperature();
    static float readHumidity();
    static float readWindSpeed();
    static float readPrecipitation();
    static float readLightLevel();
    static float readBarometricPressure();
    static float readSolarIrradiance();

    // Analysis methods
    static WeatherSeverity assessWeatherSeverity(const EnvironmentalData& conditions);
    static CameraAdaptation calculateCameraAdaptation(const EnvironmentalData& conditions);
    static PowerAdaptation calculatePowerAdaptation(const EnvironmentalData& conditions);
    
    // Helper methods
    static String conditionTypeToString(ConditionType type);
    static String severityToString(WeatherSeverity severity);
    static void addToHistory(const EnvironmentalData& data);
    static float calculateTrend(ConditionType condition, uint8_t hours_back);
    static bool isExtremeCondition(const EnvironmentalData& conditions);
    static void logAdaptation(const String& adaptation_type, const String& details);
};

#endif // ENVIRONMENTAL_ADAPTER_H