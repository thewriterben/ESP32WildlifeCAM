/**
 * Advanced Weather Station Implementation
 * Phase 2 Enhancement - Comprehensive weather monitoring
 * 
 * Features:
 * - Multi-sensor weather data collection
 * - Wind speed and direction measurement
 * - Atmospheric pressure monitoring with trend analysis
 * - Temperature and humidity with precision calibration
 * - Weather pattern prediction
 * - Storm detection and early warning
 */

#include "../../firmware/src/sensors/advanced_environmental_sensors.h"
#include <vector>
#include <map>

#ifndef WEATHER_STATION_H
#define WEATHER_STATION_H

// Weather measurement structure
struct WeatherMeasurement {
    uint32_t timestamp;
    
    // Temperature measurements
    float temperature_C;
    float temperatureMin24h_C;
    float temperatureMax24h_C;
    float temperatureTrend_C_per_hour;
    
    // Humidity measurements  
    float humidity_percent;
    float humidityMin24h_percent;
    float humidityMax24h_percent;
    
    // Pressure measurements
    float pressure_hPa;
    float pressureTrend_hPa_per_hour;
    float pressureMin24h_hPa;
    float pressureMax24h_hPa;
    
    // Wind measurements
    float windSpeed_mps;
    float windDirection_degrees;
    float windGust_mps;
    float windGustDirection_degrees;
    
    // Precipitation
    float precipitationRate_mm_per_hour;
    float precipitationAccumulated_mm;
    bool precipitationDetected;
    
    // Derived weather conditions
    float dewPoint_C;
    float heatIndex_C;
    float windChill_C;
    float visibility_km;
    
    WeatherMeasurement() : timestamp(0), temperature_C(0.0), temperatureMin24h_C(0.0),
                          temperatureMax24h_C(0.0), temperatureTrend_C_per_hour(0.0),
                          humidity_percent(0.0), humidityMin24h_percent(0.0),
                          humidityMax24h_percent(0.0), pressure_hPa(0.0),
                          pressureTrend_hPa_per_hour(0.0), pressureMin24h_hPa(0.0),
                          pressureMax24h_hPa(0.0), windSpeed_mps(0.0),
                          windDirection_degrees(0.0), windGust_mps(0.0),
                          windGustDirection_degrees(0.0), precipitationRate_mm_per_hour(0.0),
                          precipitationAccumulated_mm(0.0), precipitationDetected(false),
                          dewPoint_C(0.0), heatIndex_C(0.0), windChill_C(0.0),
                          visibility_km(0.0) {}
};

// Weather alert types
enum WeatherAlertType {
    ALERT_NONE = 0,
    ALERT_STORM_APPROACHING = 1,
    ALERT_EXTREME_TEMPERATURE = 2,
    ALERT_HIGH_WIND = 3,
    ALERT_PRESSURE_DROP = 4,
    ALERT_RAPID_WEATHER_CHANGE = 5,
    ALERT_FROST_WARNING = 6,
    ALERT_HEAT_WARNING = 7
};

struct WeatherAlert {
    WeatherAlertType alertType;
    uint32_t issuedTime;
    uint32_t expirationTime;
    String description;
    uint8_t severity;              // 1 (low) to 5 (extreme)
    float magnitude;               // Numerical value related to alert
    bool isActive;
    
    WeatherAlert() : alertType(ALERT_NONE), issuedTime(0), expirationTime(0),
                    severity(0), magnitude(0.0), isActive(false) {}
};

class AdvancedWeatherStation {
private:
    // Sensor calibration data
    struct SensorCalibration {
        float temperatureOffset_C;
        float humidityOffset_percent;
        float pressureOffset_hPa;
        float windSpeedMultiplier;
        uint32_t lastCalibration;
        
        SensorCalibration() : temperatureOffset_C(0.0), humidityOffset_percent(0.0),
                             pressureOffset_hPa(0.0), windSpeedMultiplier(1.0),
                             lastCalibration(0) {}
    };
    
    // Weather data history
    std::vector<WeatherMeasurement> weatherHistory_;
    std::vector<WeatherAlert> activeAlerts_;
    
    // Current state
    WeatherMeasurement currentWeather_;
    SensorCalibration calibration_;
    uint32_t lastMeasurement_;
    uint32_t measurementInterval_ms_;
    
    // Configuration
    bool enableWindMeasurement_;
    bool enablePrecipitationDetection_;
    bool enableWeatherAlerts_;
    uint32_t historyRetentionHours_;
    
    // Trend analysis
    struct TrendAnalysis {
        float temperatureTrend;
        float pressureTrend;
        float humidityTrend;
        uint32_t lastTrendUpdate;
        
        TrendAnalysis() : temperatureTrend(0.0), pressureTrend(0.0),
                         humidityTrend(0.0), lastTrendUpdate(0) {}
    } trendAnalysis_;
    
    bool initialized_;

public:
    AdvancedWeatherStation();
    ~AdvancedWeatherStation();
    
    // Initialization and configuration
    bool init();
    void cleanup();
    void setMeasurementInterval(uint32_t interval_ms) { measurementInterval_ms_ = interval_ms; }
    void setHistoryRetention(uint32_t hours) { historyRetentionHours_ = hours; }
    void enableWindMeasurement(bool enable) { enableWindMeasurement_ = enable; }
    void enablePrecipitationDetection(bool enable) { enablePrecipitationDetection_ = enable; }
    void enableWeatherAlerts(bool enable) { enableWeatherAlerts_ = enable; }
    
    // Measurement operations
    WeatherMeasurement takeMeasurement();
    void updateWeatherData();
    WeatherMeasurement getCurrentWeather() const { return currentWeather_; }
    std::vector<WeatherMeasurement> getWeatherHistory(uint32_t hours = 24) const;
    
    // Calibration
    void calibrateSensors();
    void setSensorOffsets(float tempOffset, float humidityOffset, float pressureOffset);
    void setWindSpeedMultiplier(float multiplier) { calibration_.windSpeedMultiplier = multiplier; }
    SensorCalibration getCalibrationData() const { return calibration_; }
    
    // Weather analysis
    void analyzeTrends();
    float getTemperatureTrend() const { return trendAnalysis_.temperatureTrend; }
    float getPressureTrend() const { return trendAnalysis_.pressureTrend; }
    float getHumidityTrend() const { return trendAnalysis_.humidityTrend; }
    
    // Weather calculations
    float calculateDewPoint(float temperature, float humidity);
    float calculateHeatIndex(float temperature, float humidity);
    float calculateWindChill(float temperature, float windSpeed);
    
    // Weather alerts
    void checkForWeatherAlerts();
    std::vector<WeatherAlert> getActiveAlerts() const { return activeAlerts_; }
    void addCustomAlert(WeatherAlertType type, const String& description, uint8_t severity);
    void clearExpiredAlerts();
    
    // Data export and reporting
    String generateWeatherReport() const;
    String generateAlertSummary() const;
    
    // Integration with environmental systems
    AdvancedEnvironmentalData getEnvironmentalData() const;
    void updateFromEnvironmentalSensors(const AdvancedEnvironmentalData& envData);
    
    // Maintenance and diagnostics
    bool performSelfDiagnostics();
    String getSensorStatus() const;
    void resetWeatherHistory();

private:
    // Internal sensor reading
    bool readTemperatureHumidity(float& temperature, float& humidity);
    bool readPressure(float& pressure);
    bool readWindData(float& speed, float& direction, float& gust);
    bool detectPrecipitation(float& rate, bool& detected);
    
    // Data processing
    void applyCalibration(WeatherMeasurement& measurement);
    void calculateDerivedValues(WeatherMeasurement& measurement);
    void updateMinMaxValues(const WeatherMeasurement& measurement);
    
    // Trend analysis algorithms
    float calculateLinearTrend(const std::vector<float>& values, const std::vector<uint32_t>& timestamps);
    void detectRapidChanges(const WeatherMeasurement& current, const WeatherMeasurement& previous);
    
    // Alert generation
    void checkTemperatureAlerts(const WeatherMeasurement& weather);
    void checkPressureAlerts(const WeatherMeasurement& weather);
    void checkWindAlerts(const WeatherMeasurement& weather);
    
    // Utility functions
    void addToHistory(const WeatherMeasurement& measurement);
    void cleanupOldData();
    bool isValidMeasurement(const WeatherMeasurement& measurement);
};

// Weather station callbacks
typedef void (*WeatherUpdateCallback)(const WeatherMeasurement& weather);
typedef void (*WeatherAlertCallback)(const WeatherAlert& alert);

// Global weather station instance
extern AdvancedWeatherStation weatherStation;

// Callback registration
void setWeatherUpdateCallback(WeatherUpdateCallback callback);
void setWeatherAlertCallback(WeatherAlertCallback callback);

#endif // WEATHER_STATION_H