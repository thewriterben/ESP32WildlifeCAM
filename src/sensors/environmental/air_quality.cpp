/**
 * Air Quality Monitoring Implementation
 * Phase 2 Enhancement - PM2.5, CO2, VOC monitoring (Simplified)
 */

#include "air_quality.h"
#include <algorithm>
#include <cmath>

// Global air quality monitor instance
AirQualityMonitor airQualityMonitor;

// Callback function pointers
static AirQualityUpdateCallback g_airQualityUpdateCallback = nullptr;
static AirQualityAlertCallback g_airQualityAlertCallback = nullptr;
static PollutionSourceCallback g_pollutionSourceCallback = nullptr;

AirQualityMonitor::AirQualityMonitor() 
    : lastMeasurement_(0), measurementInterval_ms_(300000), // 5 minutes
      enablePMSensors_(true), enableGasSensors_(true), enableVOCDetection_(true),
      enableSourceIdentification_(true), historyRetentionHours_(72), // 3 days
      initialized_(false) {
}

AirQualityMonitor::~AirQualityMonitor() {
    cleanup();
}

bool AirQualityMonitor::init() {
    if (initialized_) {
        return true;
    }
    
    // Initialize AQI breakpoints
    initializeAQIBreakpoints();
    
    // Initialize sensor calibration
    calibration_.pm25_offset = 0.0;
    calibration_.co2_offset = 0.0;
    calibration_.voc_offset = 0.0;
    calibration_.lastCalibration = millis();
    
    // Take initial measurement
    currentReading_ = takeMeasurement();
    lastMeasurement_ = millis();
    
    initialized_ = true;
    
    Serial.println("Air Quality Monitor initialized");
    Serial.println("Configuration:");
    Serial.println("  Measurement interval: " + String(measurementInterval_ms_ / 1000) + " seconds");
    Serial.println("  PM sensors: " + String(enablePMSensors_ ? "Enabled" : "Disabled"));
    Serial.println("  Gas sensors: " + String(enableGasSensors_ ? "Enabled" : "Disabled"));
    Serial.println("  VOC detection: " + String(enableVOCDetection_ ? "Enabled" : "Disabled"));
    
    return true;
}

void AirQualityMonitor::cleanup() {
    if (!initialized_) return;
    
    qualityHistory_.clear();
    activeAlerts_.clear();
    detectedSources_.clear();
    
    initialized_ = false;
}

AirQualityMeasurement AirQualityMonitor::takeMeasurement() {
    AirQualityMeasurement measurement;
    measurement.timestamp = millis();
    
    // Read particulate matter sensors
    if (enablePMSensors_) {
        readParticulateMatter(measurement.pm1_0_ug_m3, measurement.pm2_5_ug_m3, measurement.pm10_ug_m3);
    }
    
    // Read gas sensors
    if (enableGasSensors_) {
        readGasConcentrations(measurement.co2_ppm, measurement.co_ppm, measurement.no2_ppb,
                             measurement.o3_ppb, measurement.so2_ppb);
    }
    
    // Read VOC sensors
    if (enableVOCDetection_) {
        readVOCs(measurement.voc_index, measurement.tvoc_ppb, measurement.formaldehyde_ppb);
    }
    
    // Apply calibration
    applyCalibration(measurement);
    
    // Calculate derived values
    calculateDerivedValues(measurement);
    
    return measurement;
}

void AirQualityMonitor::updateAirQuality() {
    if (!initialized_) return;
    
    uint32_t now = millis();
    if (now - lastMeasurement_ < measurementInterval_ms_) {
        return;  // Not time for next measurement
    }
    
    currentReading_ = takeMeasurement();
    
    // Add to history
    addToHistory(currentReading_);
    
    // Check for alerts
    checkForAirQualityAlerts();
    
    // Analyze pollution sources
    if (enableSourceIdentification_) {
        analyzePollutionSources();
    }
    
    lastMeasurement_ = now;
    
    // Trigger callback
    if (g_airQualityUpdateCallback) {
        g_airQualityUpdateCallback(currentReading_);
    }
    
    // Periodic cleanup
    if (qualityHistory_.size() % 50 == 0) {  // Every 50 measurements
        cleanupOldData();
    }
}

float AirQualityMonitor::calculateAQI(const AirQualityMeasurement& measurement) {
    float maxAQI = 0.0;
    
    // Calculate AQI for each pollutant and take the maximum
    if (enablePMSensors_) {
        float pm25AQI = calculatePM25_AQI(measurement.pm2_5_ug_m3);
        float pm10AQI = calculatePM10_AQI(measurement.pm10_ug_m3);
        maxAQI = max(maxAQI, max(pm25AQI, pm10AQI));
    }
    
    if (enableGasSensors_) {
        float co2AQI = calculateCO2_AQI(measurement.co2_ppm);
        maxAQI = max(maxAQI, co2AQI);
    }
    
    return maxAQI;
}

float AirQualityMonitor::calculatePM25_AQI(float pm25_concentration) {
    return interpolateAQI(pm25_concentration, pm25Breakpoints_);
}

float AirQualityMonitor::calculatePM10_AQI(float pm10_concentration) {
    return interpolateAQI(pm10_concentration, pm10Breakpoints_);
}

float AirQualityMonitor::calculateCO2_AQI(float co2_concentration) {
    return interpolateAQI(co2_concentration, co2Breakpoints_);
}

String AirQualityMonitor::getAQICategory(float aqi) {
    if (aqi <= 50) return "Good";
    else if (aqi <= 100) return "Moderate";
    else if (aqi <= 150) return "Unhealthy for Sensitive Groups";
    else if (aqi <= 200) return "Unhealthy";
    else if (aqi <= 300) return "Very Unhealthy";
    else return "Hazardous";
}

uint8_t AirQualityMonitor::getHealthRisk(float aqi) {
    if (aqi <= 50) return 0;      // Good
    else if (aqi <= 100) return 1; // Moderate
    else if (aqi <= 150) return 2; // Unhealthy for sensitive
    else if (aqi <= 200) return 3; // Unhealthy
    else if (aqi <= 300) return 4; // Very unhealthy
    else return 5;                 // Hazardous
}

void AirQualityMonitor::checkForAirQualityAlerts() {
    // Clear expired alerts first
    clearExpiredAlerts();
    
    if (enablePMSensors_) {
        checkPMAlerts(currentReading_);
    }
    
    if (enableGasSensors_) {
        checkGasAlerts(currentReading_);
    }
    
    if (enableVOCDetection_) {
        checkVOCAlerts(currentReading_);
    }
}

void AirQualityMonitor::checkPMAlerts(const AirQualityMeasurement& measurement) {
    // PM2.5 alerts
    if (measurement.pm2_5_ug_m3 > 35.5) {  // Unhealthy for sensitive groups
        String advice = "Consider reducing outdoor activities if you are sensitive to air pollution.";
        addAirQualityAlert(AQ_ALERT_UNHEALTHY_SENSITIVE, "PM2.5", measurement.pm2_5_ug_m3, advice);
    }
    
    if (measurement.pm2_5_ug_m3 > 55.5) {  // Unhealthy
        String advice = "Everyone should reduce prolonged outdoor exertion.";
        addAirQualityAlert(AQ_ALERT_UNHEALTHY, "PM2.5", measurement.pm2_5_ug_m3, advice);
    }
    
    // PM10 alerts
    if (measurement.pm10_ug_m3 > 155) {  // Unhealthy for sensitive groups
        String advice = "Sensitive individuals should avoid outdoor activities.";
        addAirQualityAlert(AQ_ALERT_UNHEALTHY_SENSITIVE, "PM10", measurement.pm10_ug_m3, advice);
    }
}

void AirQualityMonitor::checkGasAlerts(const AirQualityMeasurement& measurement) {
    // CO2 alerts
    if (measurement.co2_ppm > 1000) {  // Poor indoor air quality
        String advice = "Increase ventilation to improve air quality.";
        addAirQualityAlert(AQ_ALERT_MODERATE, "CO2", measurement.co2_ppm, advice);
    }
    
    if (measurement.co2_ppm > 5000) {  // Potentially harmful
        String advice = "Immediate ventilation required - CO2 levels are dangerously high.";
        addAirQualityAlert(AQ_ALERT_HAZARDOUS, "CO2", measurement.co2_ppm, advice);
    }
    
    // CO alerts
    if (measurement.co_ppm > 9) {  // 8-hour average limit
        String advice = "Carbon monoxide levels are elevated - ensure proper ventilation.";
        addAirQualityAlert(AQ_ALERT_UNHEALTHY, "CO", measurement.co_ppm, advice);
    }
}

void AirQualityMonitor::checkVOCAlerts(const AirQualityMeasurement& measurement) {
    // VOC index alerts
    if (measurement.voc_index > 150) {  // Moderate VOC levels
        String advice = "VOC levels are elevated - check for sources of chemical emissions.";
        addAirQualityAlert(AQ_ALERT_MODERATE, "VOC", measurement.voc_index, advice);
    }
    
    if (measurement.voc_index > 300) {  // High VOC levels
        String advice = "High VOC levels detected - identify and eliminate sources.";
        addAirQualityAlert(AQ_ALERT_UNHEALTHY, "VOC", measurement.voc_index, advice);
    }
}

void AirQualityMonitor::addAirQualityAlert(AirQualityAlertLevel level, const String& pollutant, 
                                          float concentration, const String& advice) {
    // Check if similar alert already exists
    for (const auto& alert : activeAlerts_) {
        if (alert.pollutant == pollutant && alert.isActive) {
            return;  // Don't duplicate alerts
        }
    }
    
    AirQualityAlert newAlert;
    newAlert.alertLevel = level;
    newAlert.issuedTime = millis();
    newAlert.expirationTime = newAlert.issuedTime + 3600000;  // 1 hour expiration
    newAlert.pollutant = pollutant;
    newAlert.concentration = concentration;
    newAlert.healthAdvice = advice;
    newAlert.isActive = true;
    
    activeAlerts_.push_back(newAlert);
    
    Serial.println("Air Quality Alert: " + pollutant + " = " + String(concentration) + 
                  " (" + advice + ")");
    
    if (g_airQualityAlertCallback) {
        g_airQualityAlertCallback(newAlert);
    }
}

bool AirQualityMonitor::readParticulateMatter(float& pm1_0, float& pm2_5, float& pm10) {
    // Simulated PM sensor readings - in practice, would read from PMS5003 or similar
    pm1_0 = 5.0 + (random(-20, 20) / 10.0);   // 3-7 µg/m³
    pm2_5 = 12.0 + (random(-50, 50) / 10.0);  // 7-17 µg/m³
    pm10 = 20.0 + (random(-80, 80) / 10.0);   // 12-28 µg/m³
    
    // Ensure realistic values
    pm1_0 = max(0.0f, pm1_0);
    pm2_5 = max(pm1_0, pm2_5);  // PM2.5 >= PM1.0
    pm10 = max(pm2_5, pm10);    // PM10 >= PM2.5
    
    return true;
}

bool AirQualityMonitor::readGasConcentrations(float& co2, float& co, float& no2, float& o3, float& so2) {
    // Simulated gas sensor readings
    co2 = 400.0 + (random(-50, 200));        // 350-600 ppm (outdoor range)
    co = random(0, 20) / 10.0;               // 0-2 ppm
    no2 = random(0, 50);                     // 0-50 ppb
    o3 = random(20, 80);                     // 20-80 ppb
    so2 = random(0, 30);                     // 0-30 ppb
    
    return true;
}

bool AirQualityMonitor::readVOCs(float& voc_index, float& tvoc, float& formaldehyde) {
    // Simulated VOC sensor readings
    voc_index = random(0, 200);              // 0-200 index
    tvoc = random(0, 1000);                  // 0-1000 ppb
    formaldehyde = random(0, 100);           // 0-100 ppb
    
    return true;
}

void AirQualityMonitor::applyCalibration(AirQualityMeasurement& measurement) {
    measurement.pm2_5_ug_m3 += calibration_.pm25_offset;
    measurement.co2_ppm += calibration_.co2_offset;
    measurement.voc_index += calibration_.voc_offset;
    
    // Ensure non-negative values
    measurement.pm1_0_ug_m3 = max(0.0f, measurement.pm1_0_ug_m3);
    measurement.pm2_5_ug_m3 = max(0.0f, measurement.pm2_5_ug_m3);
    measurement.pm10_ug_m3 = max(0.0f, measurement.pm10_ug_m3);
    measurement.co2_ppm = max(300.0f, measurement.co2_ppm);  // Minimum atmospheric CO2
}

void AirQualityMonitor::calculateDerivedValues(AirQualityMeasurement& measurement) {
    // Calculate AQI
    measurement.aqi = calculateAQI(measurement);
    
    // Set AQI category
    measurement.aqiCategory = getAQICategory(measurement.aqi);
    
    // Set health risk level
    measurement.healthRisk = getHealthRisk(measurement.aqi);
}

void AirQualityMonitor::initializeAQIBreakpoints() {
    // PM2.5 breakpoints (µg/m³)
    pm25Breakpoints_ = {
        {0.0, 12.0, 0, 50},      // Good
        {12.1, 35.4, 51, 100},   // Moderate
        {35.5, 55.4, 101, 150},  // Unhealthy for sensitive
        {55.5, 150.4, 151, 200}, // Unhealthy
        {150.5, 250.4, 201, 300}, // Very unhealthy
        {250.5, 500.0, 301, 500}  // Hazardous
    };
    
    // PM10 breakpoints (µg/m³)
    pm10Breakpoints_ = {
        {0.0, 54.0, 0, 50},      // Good
        {55.0, 154.0, 51, 100},  // Moderate
        {155.0, 254.0, 101, 150}, // Unhealthy for sensitive
        {255.0, 354.0, 151, 200}, // Unhealthy
        {355.0, 424.0, 201, 300}, // Very unhealthy
        {425.0, 604.0, 301, 500}  // Hazardous
    };
    
    // CO2 breakpoints (ppm) - simplified scale
    co2Breakpoints_ = {
        {300.0, 600.0, 0, 50},    // Good (outdoor levels)
        {601.0, 1000.0, 51, 100}, // Moderate
        {1001.0, 2000.0, 101, 150}, // Unhealthy for sensitive
        {2001.0, 5000.0, 151, 200}, // Unhealthy
        {5001.0, 10000.0, 201, 300}, // Very unhealthy
        {10001.0, 40000.0, 301, 500} // Hazardous
    };
}

float AirQualityMonitor::interpolateAQI(float concentration, const std::vector<AQIBreakpoint>& breakpoints) {
    // Find appropriate breakpoint
    for (const auto& bp : breakpoints) {
        if (concentration >= bp.concentrationLow && concentration <= bp.concentrationHigh) {
            // Linear interpolation
            float aqiRange = bp.aqiHigh - bp.aqiLow;
            float concRange = bp.concentrationHigh - bp.concentrationLow;
            float concOffset = concentration - bp.concentrationLow;
            
            return bp.aqiLow + (aqiRange * concOffset / concRange);
        }
    }
    
    // If concentration exceeds all breakpoints, return maximum AQI
    return 500.0;
}

void AirQualityMonitor::analyzePollutionSources() {
    // Simple source identification based on pollutant patterns
    if (currentReading_.co_ppm > 2.0 && currentReading_.no2_ppb > 30) {
        // Traffic pattern
        PollutionSource source;
        source.sourceType = "traffic";
        source.confidence = 0.7;
        source.description = "Vehicle emissions detected";
        source.detectedTime = millis();
        source.duration_ms = 600000;  // 10 minutes
        
        detectedSources_.push_back(source);
        
        if (g_pollutionSourceCallback) {
            g_pollutionSourceCallback(source);
        }
    }
    
    if (currentReading_.pm2_5_ug_m3 > 50 && currentReading_.voc_index > 200) {
        // Industrial or wildfire pattern
        PollutionSource source;
        source.sourceType = "industrial";
        source.confidence = 0.6;
        source.description = "Industrial emissions or wildfire smoke";
        source.detectedTime = millis();
        source.duration_ms = 1800000;  // 30 minutes
        
        detectedSources_.push_back(source);
        
        if (g_pollutionSourceCallback) {
            g_pollutionSourceCallback(source);
        }
    }
}

void AirQualityMonitor::clearExpiredAlerts() {
    uint32_t now = millis();
    
    for (auto it = activeAlerts_.begin(); it != activeAlerts_.end();) {
        if (now > it->expirationTime) {
            it = activeAlerts_.erase(it);
        } else {
            ++it;
        }
    }
}

void AirQualityMonitor::addToHistory(const AirQualityMeasurement& measurement) {
    qualityHistory_.push_back(measurement);
    
    // Cleanup old data if needed
    cleanupOldData();
}

void AirQualityMonitor::cleanupOldData() {
    uint32_t cutoffTime = millis() - (historyRetentionHours_ * 3600000);
    
    qualityHistory_.erase(
        std::remove_if(qualityHistory_.begin(), qualityHistory_.end(),
                      [cutoffTime](const AirQualityMeasurement& m) {
                          return m.timestamp < cutoffTime;
                      }),
        qualityHistory_.end()
    );
}

String AirQualityMonitor::generateAirQualityReport() const {
    String report = "Air Quality Monitor Report:\n";
    report += "Current Conditions:\n";
    report += "  AQI: " + String(currentReading_.aqi, 0) + " (" + currentReading_.aqiCategory + ")\n";
    report += "  PM2.5: " + String(currentReading_.pm2_5_ug_m3, 1) + " µg/m³\n";
    report += "  PM10: " + String(currentReading_.pm10_ug_m3, 1) + " µg/m³\n";
    report += "  CO2: " + String(currentReading_.co2_ppm, 0) + " ppm\n";
    report += "  CO: " + String(currentReading_.co_ppm, 1) + " ppm\n";
    report += "  VOC Index: " + String(currentReading_.voc_index, 0) + "\n";
    
    report += "Health Information:\n";
    report += "  Risk Level: " + String(currentReading_.healthRisk) + "/5\n";
    report += "  Active Alerts: " + String(activeAlerts_.size()) + "\n";
    
    report += "Statistics:\n";
    report += "  History Records: " + String(qualityHistory_.size()) + "\n";
    report += "  Detected Sources: " + String(detectedSources_.size()) + "\n";
    
    return report;
}

bool AirQualityMonitor::performSelfDiagnostics() {
    Serial.println("Performing air quality sensor diagnostics...");
    
    bool allGood = true;
    
    // Test PM sensors
    float pm1, pm25, pm10;
    if (!readParticulateMatter(pm1, pm25, pm10)) {
        Serial.println("Particulate matter sensors failed");
        allGood = false;
    }
    
    // Test gas sensors
    float co2, co, no2, o3, so2;
    if (!readGasConcentrations(co2, co, no2, o3, so2)) {
        Serial.println("Gas sensors failed");
        allGood = false;
    }
    
    // Test VOC sensors
    float voc, tvoc, formaldehyde;
    if (!readVOCs(voc, tvoc, formaldehyde)) {
        Serial.println("VOC sensors failed");
        allGood = false;
    }
    
    Serial.println("Air quality diagnostics " + String(allGood ? "passed" : "failed"));
    return allGood;
}

// Callback registration functions
void setAirQualityUpdateCallback(AirQualityUpdateCallback callback) {
    g_airQualityUpdateCallback = callback;
}

void setAirQualityAlertCallback(AirQualityAlertCallback callback) {
    g_airQualityAlertCallback = callback;
}

void setPollutionSourceCallback(PollutionSourceCallback callback) {
    g_pollutionSourceCallback = callback;
}