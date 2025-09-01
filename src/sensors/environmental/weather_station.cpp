/**
 * Advanced Weather Station Implementation
 * Phase 2 Enhancement - Comprehensive weather monitoring
 */

#include "weather_station.h"
#include <cmath>
#include <algorithm>

// Global weather station instance
AdvancedWeatherStation weatherStation;

// Callback function pointers
static WeatherUpdateCallback g_weatherUpdateCallback = nullptr;
static WeatherAlertCallback g_weatherAlertCallback = nullptr;

AdvancedWeatherStation::AdvancedWeatherStation() 
    : lastMeasurement_(0), measurementInterval_ms_(60000), enableWindMeasurement_(true),
      enablePrecipitationDetection_(true), enableWeatherAlerts_(true),
      historyRetentionHours_(48), initialized_(false) {
}

AdvancedWeatherStation::~AdvancedWeatherStation() {
    cleanup();
}

bool AdvancedWeatherStation::init() {
    if (initialized_) {
        return true;
    }
    
    // Initialize sensor calibration with default values
    calibration_.temperatureOffset_C = 0.0;
    calibration_.humidityOffset_percent = 0.0;
    calibration_.pressureOffset_hPa = 0.0;
    calibration_.windSpeedMultiplier = 1.0;
    calibration_.lastCalibration = millis();
    
    // Initialize trend analysis
    trendAnalysis_.temperatureTrend = 0.0;
    trendAnalysis_.pressureTrend = 0.0;
    trendAnalysis_.humidityTrend = 0.0;
    trendAnalysis_.lastTrendUpdate = millis();
    
    // Take initial measurement
    currentWeather_ = takeMeasurement();
    lastMeasurement_ = millis();
    
    initialized_ = true;
    
    Serial.println("Advanced Weather Station initialized");
    Serial.println("Configuration:");
    Serial.println("  Measurement interval: " + String(measurementInterval_ms_ / 1000) + " seconds");
    Serial.println("  History retention: " + String(historyRetentionHours_) + " hours");
    Serial.println("  Wind measurement: " + String(enableWindMeasurement_ ? "Enabled" : "Disabled"));
    Serial.println("  Weather alerts: " + String(enableWeatherAlerts_ ? "Enabled" : "Disabled"));
    
    return true;
}

void AdvancedWeatherStation::cleanup() {
    if (!initialized_) return;
    
    weatherHistory_.clear();
    activeAlerts_.clear();
    
    initialized_ = false;
}

WeatherMeasurement AdvancedWeatherStation::takeMeasurement() {
    WeatherMeasurement measurement;
    measurement.timestamp = millis();
    
    // Read basic environmental data
    if (readTemperatureHumidity(measurement.temperature_C, measurement.humidity_percent)) {
        Serial.println("Temperature: " + String(measurement.temperature_C, 1) + "°C, " +
                      "Humidity: " + String(measurement.humidity_percent, 1) + "%");
    }
    
    if (readPressure(measurement.pressure_hPa)) {
        Serial.println("Pressure: " + String(measurement.pressure_hPa, 1) + " hPa");
    }
    
    // Read wind data if enabled
    if (enableWindMeasurement_) {
        readWindData(measurement.windSpeed_mps, measurement.windDirection_degrees, measurement.windGust_mps);
    }
    
    // Detect precipitation if enabled
    if (enablePrecipitationDetection_) {
        detectPrecipitation(measurement.precipitationRate_mm_per_hour, measurement.precipitationDetected);
    }
    
    // Apply calibration
    applyCalibration(measurement);
    
    // Calculate derived values
    calculateDerivedValues(measurement);
    
    // Update min/max values
    updateMinMaxValues(measurement);
    
    // Validate measurement
    if (!isValidMeasurement(measurement)) {
        Serial.println("Warning: Invalid weather measurement detected");
    }
    
    return measurement;
}

void AdvancedWeatherStation::updateWeatherData() {
    if (!initialized_) return;
    
    uint32_t now = millis();
    if (now - lastMeasurement_ < measurementInterval_ms_) {
        return;  // Not time for next measurement
    }
    
    WeatherMeasurement oldWeather = currentWeather_;
    currentWeather_ = takeMeasurement();
    
    // Add to history
    addToHistory(currentWeather_);
    
    // Analyze trends
    analyzeTrends();
    
    // Check for weather alerts
    if (enableWeatherAlerts_) {
        checkForWeatherAlerts();
    }
    
    // Detect rapid changes
    detectRapidChanges(currentWeather_, oldWeather);
    
    lastMeasurement_ = now;
    
    // Trigger callback
    if (g_weatherUpdateCallback) {
        g_weatherUpdateCallback(currentWeather_);
    }
    
    // Cleanup old data periodically
    if (weatherHistory_.size() % 100 == 0) {  // Every 100 measurements
        cleanupOldData();
    }
}

void AdvancedWeatherStation::calibrateSensors() {
    Serial.println("Performing sensor calibration...");
    
    // Take multiple measurements for calibration
    std::vector<float> tempReadings, humidityReadings, pressureReadings;
    
    for (int i = 0; i < 10; i++) {
        float temp, humidity, pressure;
        if (readTemperatureHumidity(temp, humidity)) {
            tempReadings.push_back(temp);
            humidityReadings.push_back(humidity);
        }
        if (readPressure(pressure)) {
            pressureReadings.push_back(pressure);
        }
        delay(1000);  // 1 second between readings
    }
    
    // Calculate calibration offsets (simplified)
    if (!tempReadings.empty()) {
        float avgTemp = 0.0;
        for (float temp : tempReadings) avgTemp += temp;
        avgTemp /= tempReadings.size();
        
        // Assume known reference temperature (20°C for indoor calibration)
        calibration_.temperatureOffset_C = 20.0 - avgTemp;
    }
    
    calibration_.lastCalibration = millis();
    
    Serial.println("Sensor calibration completed");
    Serial.println("Temperature offset: " + String(calibration_.temperatureOffset_C, 2) + "°C");
    Serial.println("Humidity offset: " + String(calibration_.humidityOffset_percent, 2) + "%");
    Serial.println("Pressure offset: " + String(calibration_.pressureOffset_hPa, 2) + " hPa");
}

void AdvancedWeatherStation::setSensorOffsets(float tempOffset, float humidityOffset, float pressureOffset) {
    calibration_.temperatureOffset_C = tempOffset;
    calibration_.humidityOffset_percent = humidityOffset;
    calibration_.pressureOffset_hPa = pressureOffset;
    calibration_.lastCalibration = millis();
    
    Serial.println("Manual sensor offsets applied");
}

void AdvancedWeatherStation::analyzeTrends() {
    if (weatherHistory_.size() < 2) {
        return;  // Need at least 2 data points
    }
    
    uint32_t now = millis();
    if (now - trendAnalysis_.lastTrendUpdate < 300000) {  // Update every 5 minutes
        return;
    }
    
    // Get data from the last hour for trend analysis
    std::vector<float> temperatures, pressures, humidities;
    std::vector<uint32_t> timestamps;
    
    uint32_t oneHourAgo = now - 3600000;  // 1 hour ago
    
    for (const auto& measurement : weatherHistory_) {
        if (measurement.timestamp > oneHourAgo) {
            temperatures.push_back(measurement.temperature_C);
            pressures.push_back(measurement.pressure_hPa);
            humidities.push_back(measurement.humidity_percent);
            timestamps.push_back(measurement.timestamp);
        }
    }
    
    if (temperatures.size() >= 3) {  // Need at least 3 points for trend
        trendAnalysis_.temperatureTrend = calculateLinearTrend(temperatures, timestamps);
        trendAnalysis_.pressureTrend = calculateLinearTrend(pressures, timestamps);
        trendAnalysis_.humidityTrend = calculateLinearTrend(humidities, timestamps);
        
        currentWeather_.temperatureTrend_C_per_hour = trendAnalysis_.temperatureTrend;
        currentWeather_.pressureTrend_hPa_per_hour = trendAnalysis_.pressureTrend;
        
        Serial.println("Weather trends updated:");
        Serial.println("  Temperature: " + String(trendAnalysis_.temperatureTrend, 2) + "°C/hour");
        Serial.println("  Pressure: " + String(trendAnalysis_.pressureTrend, 2) + " hPa/hour");
    }
    
    trendAnalysis_.lastTrendUpdate = now;
}

float AdvancedWeatherStation::calculateDewPoint(float temperature, float humidity) {
    // Magnus formula for dew point calculation
    float a = 17.27;
    float b = 237.7;
    
    float alpha = ((a * temperature) / (b + temperature)) + log(humidity / 100.0);
    float dewPoint = (b * alpha) / (a - alpha);
    
    return dewPoint;
}

float AdvancedWeatherStation::calculateHeatIndex(float temperature, float humidity) {
    // Heat index calculation (simplified Rothfusz equation)
    if (temperature < 27.0) {  // Below 80°F, heat index = temperature
        return temperature;
    }
    
    float T = temperature;
    float RH = humidity;
    
    float HI = -8.78469475556 + 1.61139411 * T + 2.33854883889 * RH - 0.14611605 * T * RH
               - 0.012308094 * T * T - 0.0164248277778 * RH * RH + 0.002211732 * T * T * RH
               + 0.00072546 * T * RH * RH - 0.000003582 * T * T * RH * RH;
    
    return HI;
}

float AdvancedWeatherStation::calculateWindChill(float temperature, float windSpeed) {
    // Wind chill calculation (applicable for temperatures below 10°C and wind speeds above 4.8 km/h)
    if (temperature > 10.0 || windSpeed < 1.34) {  // 4.8 km/h = 1.34 m/s
        return temperature;
    }
    
    float windSpeedKmh = windSpeed * 3.6;  // Convert m/s to km/h
    
    float windChill = 13.12 + 0.6215 * temperature - 11.37 * pow(windSpeedKmh, 0.16) + 
                      0.3965 * temperature * pow(windSpeedKmh, 0.16);
    
    return windChill;
}

void AdvancedWeatherStation::checkForWeatherAlerts() {
    // Clear expired alerts first
    clearExpiredAlerts();
    
    checkTemperatureAlerts(currentWeather_);
    checkPressureAlerts(currentWeather_);
    
    if (enableWindMeasurement_) {
        checkWindAlerts(currentWeather_);
    }
}

void AdvancedWeatherStation::checkTemperatureAlerts(const WeatherMeasurement& weather) {
    // Extreme temperature alerts
    if (weather.temperature_C > 40.0) {
        addCustomAlert(ALERT_HEAT_WARNING, "Extreme heat warning: " + String(weather.temperature_C, 1) + "°C", 4);
    } else if (weather.temperature_C < -20.0) {
        addCustomAlert(ALERT_EXTREME_TEMPERATURE, "Extreme cold warning: " + String(weather.temperature_C, 1) + "°C", 4);
    } else if (weather.temperature_C < 2.0 && weather.humidity_percent > 80.0) {
        addCustomAlert(ALERT_FROST_WARNING, "Frost warning: " + String(weather.temperature_C, 1) + "°C", 2);
    }
    
    // Rapid temperature change alerts
    if (abs(weather.temperatureTrend_C_per_hour) > 5.0) {
        String direction = weather.temperatureTrend_C_per_hour > 0 ? "rising" : "falling";
        addCustomAlert(ALERT_RAPID_WEATHER_CHANGE, 
                      "Rapid temperature change: " + direction + " " + 
                      String(abs(weather.temperatureTrend_C_per_hour), 1) + "°C/hour", 3);
    }
}

void AdvancedWeatherStation::checkPressureAlerts(const WeatherMeasurement& weather) {
    // Rapid pressure drop (storm approaching)
    if (weather.pressureTrend_hPa_per_hour < -3.0) {
        addCustomAlert(ALERT_STORM_APPROACHING, 
                      "Storm approaching: pressure dropping " + 
                      String(abs(weather.pressureTrend_hPa_per_hour), 1) + " hPa/hour", 3);
    }
    
    // Very low pressure
    if (weather.pressure_hPa < 980.0) {
        addCustomAlert(ALERT_PRESSURE_DROP, 
                      "Very low pressure: " + String(weather.pressure_hPa, 1) + " hPa", 2);
    }
}

void AdvancedWeatherStation::checkWindAlerts(const WeatherMeasurement& weather) {
    // High wind alerts
    if (weather.windSpeed_mps > 15.0) {  // ~54 km/h
        addCustomAlert(ALERT_HIGH_WIND, 
                      "High wind warning: " + String(weather.windSpeed_mps, 1) + " m/s", 3);
    }
    
    if (weather.windGust_mps > 25.0) {  // ~90 km/h
        addCustomAlert(ALERT_HIGH_WIND, 
                      "Extreme wind gust: " + String(weather.windGust_mps, 1) + " m/s", 4);
    }
}

void AdvancedWeatherStation::addCustomAlert(WeatherAlertType type, const String& description, uint8_t severity) {
    // Check if similar alert already exists
    for (const auto& alert : activeAlerts_) {
        if (alert.alertType == type && alert.isActive) {
            return;  // Don't duplicate alerts
        }
    }
    
    WeatherAlert newAlert;
    newAlert.alertType = type;
    newAlert.issuedTime = millis();
    newAlert.expirationTime = newAlert.issuedTime + 3600000;  // 1 hour expiration
    newAlert.description = description;
    newAlert.severity = severity;
    newAlert.isActive = true;
    
    activeAlerts_.push_back(newAlert);
    
    Serial.println("Weather Alert: " + description + " (Severity: " + String(severity) + ")");
    
    if (g_weatherAlertCallback) {
        g_weatherAlertCallback(newAlert);
    }
}

void AdvancedWeatherStation::clearExpiredAlerts() {
    uint32_t now = millis();
    
    for (auto it = activeAlerts_.begin(); it != activeAlerts_.end();) {
        if (now > it->expirationTime) {
            it = activeAlerts_.erase(it);
        } else {
            ++it;
        }
    }
}

bool AdvancedWeatherStation::readTemperatureHumidity(float& temperature, float& humidity) {
    // Simulated sensor reading - in practice, this would read from BME280 or similar
    temperature = 20.0 + (random(-50, 50) / 10.0);  // 15-25°C range
    humidity = 50.0 + (random(-200, 200) / 10.0);   // 30-70% range
    
    // Constrain to realistic ranges
    temperature = constrain(temperature, -40.0, 60.0);
    humidity = constrain(humidity, 0.0, 100.0);
    
    return true;  // Assume successful reading
}

bool AdvancedWeatherStation::readPressure(float& pressure) {
    // Simulated pressure reading
    pressure = 1013.25 + (random(-100, 100) / 10.0);  // ±10 hPa variation
    pressure = constrain(pressure, 950.0, 1050.0);
    
    return true;
}

bool AdvancedWeatherStation::readWindData(float& speed, float& direction, float& gust) {
    // Simulated wind data
    speed = random(0, 100) / 10.0;  // 0-10 m/s
    direction = random(0, 360);     // 0-359 degrees
    gust = speed + random(0, 50) / 10.0;  // Gust = speed + 0-5 m/s
    
    return true;
}

bool AdvancedWeatherStation::detectPrecipitation(float& rate, bool& detected) {
    // Simulated precipitation detection
    rate = 0.0;
    detected = false;
    
    // Random chance of precipitation
    if (random(0, 100) < 10) {  // 10% chance
        detected = true;
        rate = random(1, 100) / 10.0;  // 0.1-10 mm/hour
    }
    
    return true;
}

void AdvancedWeatherStation::applyCalibration(WeatherMeasurement& measurement) {
    measurement.temperature_C += calibration_.temperatureOffset_C;
    measurement.humidity_percent += calibration_.humidityOffset_percent;
    measurement.pressure_hPa += calibration_.pressureOffset_hPa;
    measurement.windSpeed_mps *= calibration_.windSpeedMultiplier;
    measurement.windGust_mps *= calibration_.windSpeedMultiplier;
}

void AdvancedWeatherStation::calculateDerivedValues(WeatherMeasurement& measurement) {
    measurement.dewPoint_C = calculateDewPoint(measurement.temperature_C, measurement.humidity_percent);
    measurement.heatIndex_C = calculateHeatIndex(measurement.temperature_C, measurement.humidity_percent);
    measurement.windChill_C = calculateWindChill(measurement.temperature_C, measurement.windSpeed_mps);
    
    // Simple visibility calculation based on humidity
    measurement.visibility_km = 20.0 - (measurement.humidity_percent / 5.0);  // Simplified
    measurement.visibility_km = max(0.1f, measurement.visibility_km);
}

void AdvancedWeatherStation::updateMinMaxValues(const WeatherMeasurement& measurement) {
    // Update 24-hour min/max values
    if (weatherHistory_.empty()) {
        currentWeather_.temperatureMin24h_C = measurement.temperature_C;
        currentWeather_.temperatureMax24h_C = measurement.temperature_C;
        currentWeather_.humidityMin24h_percent = measurement.humidity_percent;
        currentWeather_.humidityMax24h_percent = measurement.humidity_percent;
        currentWeather_.pressureMin24h_hPa = measurement.pressure_hPa;
        currentWeather_.pressureMax24h_hPa = measurement.pressure_hPa;
    } else {
        // Find min/max from last 24 hours
        uint32_t twentyFourHoursAgo = millis() - 86400000;
        
        float minTemp = measurement.temperature_C, maxTemp = measurement.temperature_C;
        float minHumidity = measurement.humidity_percent, maxHumidity = measurement.humidity_percent;
        float minPressure = measurement.pressure_hPa, maxPressure = measurement.pressure_hPa;
        
        for (const auto& hist : weatherHistory_) {
            if (hist.timestamp > twentyFourHoursAgo) {
                minTemp = min(minTemp, hist.temperature_C);
                maxTemp = max(maxTemp, hist.temperature_C);
                minHumidity = min(minHumidity, hist.humidity_percent);
                maxHumidity = max(maxHumidity, hist.humidity_percent);
                minPressure = min(minPressure, hist.pressure_hPa);
                maxPressure = max(maxPressure, hist.pressure_hPa);
            }
        }
        
        currentWeather_.temperatureMin24h_C = minTemp;
        currentWeather_.temperatureMax24h_C = maxTemp;
        currentWeather_.humidityMin24h_percent = minHumidity;
        currentWeather_.humidityMax24h_percent = maxHumidity;
        currentWeather_.pressureMin24h_hPa = minPressure;
        currentWeather_.pressureMax24h_hPa = maxPressure;
    }
}

float AdvancedWeatherStation::calculateLinearTrend(const std::vector<float>& values, 
                                                  const std::vector<uint32_t>& timestamps) {
    if (values.size() < 2 || values.size() != timestamps.size()) {
        return 0.0;
    }
    
    size_t n = values.size();
    float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    
    // Convert timestamps to hours for slope calculation
    uint32_t baseTime = timestamps[0];
    
    for (size_t i = 0; i < n; i++) {
        float x = (timestamps[i] - baseTime) / 3600000.0;  // Hours since first measurement
        float y = values[i];
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    
    // Linear regression slope (trend per hour)
    float slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
    
    return slope;
}

void AdvancedWeatherStation::detectRapidChanges(const WeatherMeasurement& current, 
                                               const WeatherMeasurement& previous) {
    if (previous.timestamp == 0) return;  // No previous measurement
    
    uint32_t timeDiff = current.timestamp - previous.timestamp;
    if (timeDiff == 0) return;
    
    float tempChange = abs(current.temperature_C - previous.temperature_C);
    float pressureChange = abs(current.pressure_hPa - previous.pressure_hPa);
    float humidityChange = abs(current.humidity_percent - previous.humidity_percent);
    
    // Check for rapid changes
    if (tempChange > 2.0) {  // More than 2°C change
        Serial.println("Rapid temperature change detected: " + String(tempChange, 1) + "°C");
    }
    
    if (pressureChange > 5.0) {  // More than 5 hPa change
        Serial.println("Rapid pressure change detected: " + String(pressureChange, 1) + " hPa");
    }
    
    if (humidityChange > 15.0) {  // More than 15% humidity change
        Serial.println("Rapid humidity change detected: " + String(humidityChange, 1) + "%");
    }
}

void AdvancedWeatherStation::addToHistory(const WeatherMeasurement& measurement) {
    weatherHistory_.push_back(measurement);
    
    // Limit history size based on retention period
    cleanupOldData();
}

void AdvancedWeatherStation::cleanupOldData() {
    uint32_t cutoffTime = millis() - (historyRetentionHours_ * 3600000);
    
    weatherHistory_.erase(
        std::remove_if(weatherHistory_.begin(), weatherHistory_.end(),
                      [cutoffTime](const WeatherMeasurement& m) {
                          return m.timestamp < cutoffTime;
                      }),
        weatherHistory_.end()
    );
}

bool AdvancedWeatherStation::isValidMeasurement(const WeatherMeasurement& measurement) {
    // Basic validation checks
    if (measurement.temperature_C < -50.0 || measurement.temperature_C > 70.0) return false;
    if (measurement.humidity_percent < 0.0 || measurement.humidity_percent > 100.0) return false;
    if (measurement.pressure_hPa < 800.0 || measurement.pressure_hPa > 1100.0) return false;
    if (measurement.windSpeed_mps < 0.0 || measurement.windSpeed_mps > 100.0) return false;
    
    return true;
}

std::vector<WeatherMeasurement> AdvancedWeatherStation::getWeatherHistory(uint32_t hours) const {
    std::vector<WeatherMeasurement> recentHistory;
    uint32_t cutoffTime = millis() - (hours * 3600000);
    
    for (const auto& measurement : weatherHistory_) {
        if (measurement.timestamp > cutoffTime) {
            recentHistory.push_back(measurement);
        }
    }
    
    return recentHistory;
}

String AdvancedWeatherStation::generateWeatherReport() const {
    String report = "Advanced Weather Station Report:\n";
    report += "Current Conditions:\n";
    report += "  Temperature: " + String(currentWeather_.temperature_C, 1) + "°C\n";
    report += "  Humidity: " + String(currentWeather_.humidity_percent, 1) + "%\n";
    report += "  Pressure: " + String(currentWeather_.pressure_hPa, 1) + " hPa\n";
    report += "  Wind Speed: " + String(currentWeather_.windSpeed_mps, 1) + " m/s\n";
    report += "  Wind Direction: " + String(currentWeather_.windDirection_degrees, 0) + "°\n";
    
    report += "Derived Values:\n";
    report += "  Dew Point: " + String(currentWeather_.dewPoint_C, 1) + "°C\n";
    report += "  Heat Index: " + String(currentWeather_.heatIndex_C, 1) + "°C\n";
    report += "  Wind Chill: " + String(currentWeather_.windChill_C, 1) + "°C\n";
    
    report += "24-Hour Range:\n";
    report += "  Temperature: " + String(currentWeather_.temperatureMin24h_C, 1) + 
             "°C to " + String(currentWeather_.temperatureMax24h_C, 1) + "°C\n";
    report += "  Pressure: " + String(currentWeather_.pressureMin24h_hPa, 1) + 
             " to " + String(currentWeather_.pressureMax24h_hPa, 1) + " hPa\n";
    
    report += "Trends:\n";
    report += "  Temperature: " + String(trendAnalysis_.temperatureTrend, 2) + "°C/hour\n";
    report += "  Pressure: " + String(trendAnalysis_.pressureTrend, 2) + " hPa/hour\n";
    
    report += "Statistics:\n";
    report += "  History Records: " + String(weatherHistory_.size()) + "\n";
    report += "  Active Alerts: " + String(activeAlerts_.size()) + "\n";
    
    return report;
}

String AdvancedWeatherStation::generateAlertSummary() const {
    String summary = "Weather Alert Summary:\n";
    summary += "Active Alerts: " + String(activeAlerts_.size()) + "\n";
    
    for (const auto& alert : activeAlerts_) {
        if (alert.isActive) {
            summary += "  [Severity " + String(alert.severity) + "] " + alert.description + "\n";
        }
    }
    
    if (activeAlerts_.empty()) {
        summary += "  No active weather alerts\n";
    }
    
    return summary;
}

AdvancedEnvironmentalData AdvancedWeatherStation::getEnvironmentalData() const {
    AdvancedEnvironmentalData envData;
    
    envData.temperature_C = currentWeather_.temperature_C;
    envData.humidity_percent = currentWeather_.humidity_percent;
    envData.pressure_hPa = currentWeather_.pressure_hPa;
    envData.windSpeed_mps = currentWeather_.windSpeed_mps;
    envData.windDirection_degrees = currentWeather_.windDirection_degrees;
    envData.lightLevel_lux = 1000.0;  // Placeholder - would come from light sensor
    envData.timestamp = currentWeather_.timestamp;
    
    return envData;
}

bool AdvancedWeatherStation::performSelfDiagnostics() {
    Serial.println("Performing weather station self-diagnostics...");
    
    bool allGood = true;
    
    // Test temperature/humidity sensor
    float temp, humidity;
    if (!readTemperatureHumidity(temp, humidity)) {
        Serial.println("Temperature/humidity sensor failed");
        allGood = false;
    }
    
    // Test pressure sensor
    float pressure;
    if (!readPressure(pressure)) {
        Serial.println("Pressure sensor failed");
        allGood = false;
    }
    
    // Check calibration age
    if (millis() - calibration_.lastCalibration > 2592000000) {  // 30 days
        Serial.println("Warning: Sensor calibration is old (>30 days)");
    }
    
    Serial.println("Self-diagnostics " + String(allGood ? "passed" : "failed"));
    return allGood;
}

String AdvancedWeatherStation::getSensorStatus() const {
    String status = "Weather Station Sensor Status:\n";
    status += "Initialized: " + String(initialized_ ? "Yes" : "No") + "\n";
    status += "Last Measurement: " + String((millis() - lastMeasurement_) / 1000) + " seconds ago\n";
    status += "Measurement Interval: " + String(measurementInterval_ms_ / 1000) + " seconds\n";
    status += "History Size: " + String(weatherHistory_.size()) + " records\n";
    status += "Last Calibration: " + String((millis() - calibration_.lastCalibration) / 86400000) + " days ago\n";
    
    return status;
}

void AdvancedWeatherStation::resetWeatherHistory() {
    weatherHistory_.clear();
    activeAlerts_.clear();
    
    Serial.println("Weather history reset");
}

// Callback registration functions
void setWeatherUpdateCallback(WeatherUpdateCallback callback) {
    g_weatherUpdateCallback = callback;
}

void setWeatherAlertCallback(WeatherAlertCallback callback) {
    g_weatherAlertCallback = callback;
}