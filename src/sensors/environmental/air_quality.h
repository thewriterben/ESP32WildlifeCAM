/**
 * Air Quality Monitoring Implementation
 * Phase 2 Enhancement - PM2.5, CO2, VOC monitoring
 * 
 * Features:
 * - Particulate matter (PM2.5, PM10) measurement
 * - Carbon dioxide (CO2) monitoring
 * - Volatile organic compound (VOC) detection
 * - Air quality index (AQI) calculation
 * - Pollution source identification
 * - Health impact assessment
 */

#include "../../firmware/src/sensors/advanced_environmental_sensors.h"
#include <vector>
#include <map>

#ifndef AIR_QUALITY_H
#define AIR_QUALITY_H

// Air quality measurement structure
struct AirQualityMeasurement {
    uint32_t timestamp;
    
    // Particulate matter
    float pm1_0_ug_m3;          // PM1.0 concentration
    float pm2_5_ug_m3;          // PM2.5 concentration  
    float pm10_ug_m3;           // PM10 concentration
    
    // Gases
    float co2_ppm;              // Carbon dioxide
    float co_ppm;               // Carbon monoxide
    float no2_ppb;              // Nitrogen dioxide
    float o3_ppb;               // Ozone
    float so2_ppb;              // Sulfur dioxide
    
    // Volatile organic compounds
    float voc_index;            // VOC index (0-500)
    float tvoc_ppb;             // Total VOCs
    float formaldehyde_ppb;     // Formaldehyde
    
    // Environmental context
    float temperature_C;
    float humidity_percent;
    float pressure_hPa;
    
    // Calculated indices
    float aqi;                  // Air Quality Index (0-500)
    String aqiCategory;         // "Good", "Moderate", "Unhealthy", etc.
    uint8_t healthRisk;         // 0 (low) to 5 (hazardous)
    
    AirQualityMeasurement() : timestamp(0), pm1_0_ug_m3(0.0), pm2_5_ug_m3(0.0),
                             pm10_ug_m3(0.0), co2_ppm(400.0), co_ppm(0.0),
                             no2_ppb(0.0), o3_ppb(0.0), so2_ppb(0.0),
                             voc_index(0.0), tvoc_ppb(0.0), formaldehyde_ppb(0.0),
                             temperature_C(0.0), humidity_percent(0.0), pressure_hPa(0.0),
                             aqi(0.0), healthRisk(0) {}
};

// Air quality alert levels
enum AirQualityAlertLevel {
    AQ_ALERT_NONE = 0,
    AQ_ALERT_MODERATE = 1,      // AQI 51-100
    AQ_ALERT_UNHEALTHY_SENSITIVE = 2,  // AQI 101-150
    AQ_ALERT_UNHEALTHY = 3,     // AQI 151-200
    AQ_ALERT_VERY_UNHEALTHY = 4, // AQI 201-300
    AQ_ALERT_HAZARDOUS = 5      // AQI 301+
};

struct AirQualityAlert {
    AirQualityAlertLevel alertLevel;
    uint32_t issuedTime;
    uint32_t expirationTime;
    String pollutant;           // Primary pollutant causing alert
    float concentration;        // Concentration that triggered alert
    String healthAdvice;        // Health recommendations
    bool isActive;
    
    AirQualityAlert() : alertLevel(AQ_ALERT_NONE), issuedTime(0), expirationTime(0),
                       concentration(0.0), isActive(false) {}
};

// Pollution source identification
struct PollutionSource {
    String sourceType;          // "traffic", "industrial", "wildfire", "domestic"
    float confidence;           // 0.0-1.0 confidence in identification
    String description;         // Detailed description
    uint32_t detectedTime;
    uint32_t duration_ms;
    
    PollutionSource() : confidence(0.0), detectedTime(0), duration_ms(0) {}
};

class AirQualityMonitor {
private:
    // Air quality data history
    std::vector<AirQualityMeasurement> qualityHistory_;
    std::vector<AirQualityAlert> activeAlerts_;
    std::vector<PollutionSource> detectedSources_;
    
    // Current measurements
    AirQualityMeasurement currentReading_;
    uint32_t lastMeasurement_;
    uint32_t measurementInterval_ms_;
    
    // Configuration
    bool enablePMSensors_;
    bool enableGasSensors_;
    bool enableVOCDetection_;
    bool enableSourceIdentification_;
    uint32_t historyRetentionHours_;
    
    // Sensor calibration
    struct SensorCalibration {
        float pm25_offset;
        float co2_offset;
        float voc_offset;
        uint32_t lastCalibration;
        
        SensorCalibration() : pm25_offset(0.0), co2_offset(0.0), 
                             voc_offset(0.0), lastCalibration(0) {}
    } calibration_;
    
    // AQI calculation parameters
    struct AQIBreakpoint {
        float concentrationLow;
        float concentrationHigh;
        int aqiLow;
        int aqiHigh;
    };
    
    std::vector<AQIBreakpoint> pm25Breakpoints_;
    std::vector<AQIBreakpoint> pm10Breakpoints_;
    std::vector<AQIBreakpoint> co2Breakpoints_;
    
    bool initialized_;

public:
    AirQualityMonitor();
    ~AirQualityMonitor();
    
    // Initialization and configuration
    bool init();
    void cleanup();
    void setMeasurementInterval(uint32_t interval_ms) { measurementInterval_ms_ = interval_ms; }
    void setHistoryRetention(uint32_t hours) { historyRetentionHours_ = hours; }
    void enablePMSensors(bool enable) { enablePMSensors_ = enable; }
    void enableGasSensors(bool enable) { enableGasSensors_ = enable; }
    void enableVOCDetection(bool enable) { enableVOCDetection_ = enable; }
    void enableSourceIdentification(bool enable) { enableSourceIdentification_ = enable; }
    
    // Measurement operations
    AirQualityMeasurement takeMeasurement();
    void updateAirQuality();
    AirQualityMeasurement getCurrentReading() const { return currentReading_; }
    std::vector<AirQualityMeasurement> getQualityHistory(uint32_t hours = 24) const;
    
    // Air Quality Index calculations
    float calculateAQI(const AirQualityMeasurement& measurement);
    float calculatePM25_AQI(float pm25_concentration);
    float calculatePM10_AQI(float pm10_concentration);
    float calculateCO2_AQI(float co2_concentration);
    String getAQICategory(float aqi);
    uint8_t getHealthRisk(float aqi);
    
    // Pollution analysis
    void analyzePollutionSources();
    std::vector<PollutionSource> getDetectedSources() const { return detectedSources_; }
    String identifyPrimaryPollutant(const AirQualityMeasurement& measurement);
    bool detectWildfireSmoke(const AirQualityMeasurement& measurement);
    bool detectVehicleEmissions(const AirQualityMeasurement& measurement);
    
    // Health impact assessment
    String generateHealthAdvice(const AirQualityMeasurement& measurement);
    float calculateExposureRisk(const AirQualityMeasurement& measurement, uint32_t exposureDuration_hours);
    bool isOutdoorActivitySafe(const AirQualityMeasurement& measurement);
    
    // Alert management
    void checkForAirQualityAlerts();
    std::vector<AirQualityAlert> getActiveAlerts() const { return activeAlerts_; }
    void addAirQualityAlert(AirQualityAlertLevel level, const String& pollutant, 
                           float concentration, const String& advice);
    void clearExpiredAlerts();
    
    // Calibration
    void calibrateSensors();
    void setCalibrationOffsets(float pm25_offset, float co2_offset, float voc_offset);
    SensorCalibration getCalibrationData() const { return calibration_; }
    
    // Data analysis
    void analyzeTrends();
    float getAverageAQI(uint32_t hours = 24);
    float getPeakPM25(uint32_t hours = 24);
    uint32_t getUnhealthyHours(uint32_t hours = 24);
    
    // Reporting and export
    String generateAirQualityReport() const;
    String generateHealthReport() const;
    String generatePollutionSourceReport() const;
    bool exportAirQualityData(const String& format = "csv");
    
    // Integration with other systems
    void updateFromEnvironmentalSensors(const AdvancedEnvironmentalData& envData);
    AdvancedEnvironmentalData getEnvironmentalData() const;
    
    // Maintenance and diagnostics
    bool performSelfDiagnostics();
    String getSensorStatus() const;
    void resetAirQualityHistory();

private:
    // Sensor reading functions
    bool readParticulateMatter(float& pm1_0, float& pm2_5, float& pm10);
    bool readGasConcentrations(float& co2, float& co, float& no2, float& o3, float& so2);
    bool readVOCs(float& voc_index, float& tvoc, float& formaldehyde);
    
    // Data processing
    void applyCalibration(AirQualityMeasurement& measurement);
    void calculateDerivedValues(AirQualityMeasurement& measurement);
    void smoothMeasurements(AirQualityMeasurement& measurement);
    
    // AQI calculation helpers
    float interpolateAQI(float concentration, const std::vector<AQIBreakpoint>& breakpoints);
    void initializeAQIBreakpoints();
    
    // Source identification algorithms
    PollutionSource analyzeTrafficSource(const AirQualityMeasurement& measurement);
    PollutionSource analyzeIndustrialSource(const AirQualityMeasurement& measurement);
    PollutionSource analyzeWildfireSource(const AirQualityMeasurement& measurement);
    PollutionSource analyzeDomesticSource(const AirQualityMeasurement& measurement);
    
    // Alert generation
    void checkPMAlerts(const AirQualityMeasurement& measurement);
    void checkGasAlerts(const AirQualityMeasurement& measurement);
    void checkVOCAlerts(const AirQualityMeasurement& measurement);
    
    // Utility functions
    void addToHistory(const AirQualityMeasurement& measurement);
    void cleanupOldData();
    bool isValidMeasurement(const AirQualityMeasurement& measurement);
    float calculateMovingAverage(const std::vector<float>& values, uint32_t windowSize);
};

// Air quality monitoring callbacks
typedef void (*AirQualityUpdateCallback)(const AirQualityMeasurement& measurement);
typedef void (*AirQualityAlertCallback)(const AirQualityAlert& alert);
typedef void (*PollutionSourceCallback)(const PollutionSource& source);

// Global air quality monitor instance
extern AirQualityMonitor airQualityMonitor;

// Callback registration
void setAirQualityUpdateCallback(AirQualityUpdateCallback callback);
void setAirQualityAlertCallback(AirQualityAlertCallback callback);
void setPollutionSourceCallback(PollutionSourceCallback callback);

#endif // AIR_QUALITY_H