/**
 * Environmental Correlation Analysis Implementation
 * Phase 2 Enhancement - Weather/time pattern analysis for wildlife behavior
 */

#include "environmental_correlation.h"
#include <algorithm>
#include <cmath>

// Global environmental correlation engine
EnvironmentalCorrelationEngine environmentalAnalyzer;

// Callback function pointers
static EnvironmentalEventCallback g_environmentalEventCallback = nullptr;
static WeatherChangeCallback g_weatherChangeCallback = nullptr;
static ActivityPredictionCallback g_activityPredictionCallback = nullptr;

EnvironmentalCorrelationEngine::EnvironmentalCorrelationEngine() 
    : maxHistorySize_(1000), correlationWindow_ms_(3600000), correlationThreshold_(0.5),
      enablePredictiveModeling_(true), enableWeatherForecasting_(false),
      lastEnvironmentalUpdate_(0), initialized_(false) {
}

EnvironmentalCorrelationEngine::~EnvironmentalCorrelationEngine() {
    cleanup();
}

bool EnvironmentalCorrelationEngine::init() {
    if (initialized_) {
        return true;
    }
    
    // Initialize with default species correlations
    EnvironmentalCorrelation deerCorrelation;
    deerCorrelation.temperatureOptimal_C = 18.0;
    deerCorrelation.temperatureRange_C = 15.0;
    deerCorrelation.humidityOptimal_percent = 60.0;
    deerCorrelation.humidityRange_percent = 30.0;
    deerCorrelation.isStormSensitive = true;
    
    // Set time activity patterns for deer (more active at dawn and dusk)
    deerCorrelation.timeActivityLevels[TIME_DAWN] = 0.9;
    deerCorrelation.timeActivityLevels[TIME_MORNING] = 0.6;
    deerCorrelation.timeActivityLevels[TIME_MIDDAY] = 0.3;
    deerCorrelation.timeActivityLevels[TIME_AFTERNOON] = 0.4;
    deerCorrelation.timeActivityLevels[TIME_DUSK] = 0.9;
    deerCorrelation.timeActivityLevels[TIME_NIGHT] = 0.2;
    
    speciesCorrelations_["deer"] = deerCorrelation;
    
    // Initialize bird correlations
    EnvironmentalCorrelation birdCorrelation;
    birdCorrelation.temperatureOptimal_C = 22.0;
    birdCorrelation.temperatureRange_C = 20.0;
    birdCorrelation.windSpeedTolerance_mps = 3.0;
    birdCorrelation.isStormSensitive = true;
    
    // Birds are more active during day
    birdCorrelation.timeActivityLevels[TIME_DAWN] = 0.8;
    birdCorrelation.timeActivityLevels[TIME_MORNING] = 0.9;
    birdCorrelation.timeActivityLevels[TIME_MIDDAY] = 0.7;
    birdCorrelation.timeActivityLevels[TIME_AFTERNOON] = 0.8;
    birdCorrelation.timeActivityLevels[TIME_DUSK] = 0.6;
    birdCorrelation.timeActivityLevels[TIME_NIGHT] = 0.1;
    
    speciesCorrelations_["bird"] = birdCorrelation;
    
    initialized_ = true;
    
    Serial.println("Environmental Correlation Engine initialized");
    Serial.println("Configuration:");
    Serial.println("  Max history size: " + String(maxHistorySize_));
    Serial.println("  Correlation window: " + String(correlationWindow_ms_ / 1000) + " seconds");
    Serial.println("  Correlation threshold: " + String(correlationThreshold_));
    
    return true;
}

void EnvironmentalCorrelationEngine::cleanup() {
    if (!initialized_) return;
    
    speciesCorrelations_.clear();
    environmentalHistory_.clear();
    recentEvents_.clear();
    forecasts_.clear();
    predictionModels_.clear();
    
    initialized_ = false;
}

void EnvironmentalCorrelationEngine::updateEnvironmentalConditions(float temperature, float humidity, 
                                                                  float pressure, float windSpeed, 
                                                                  uint8_t lightLevel) {
    if (!initialized_) return;
    
    EnvironmentalReading oldConditions = currentConditions_;
    
    // Update current conditions
    currentConditions_.timestamp = millis();
    currentConditions_.temperature_C = temperature;
    currentConditions_.humidity_percent = humidity;
    currentConditions_.pressure_hPa = pressure;
    currentConditions_.windSpeed_mps = windSpeed;
    currentConditions_.lightLevel = lightLevel;
    currentConditions_.condition = classifyWeatherCondition(currentConditions_);
    
    // Add to history
    environmentalHistory_.push_back(currentConditions_);
    
    // Limit history size
    if (environmentalHistory_.size() > maxHistorySize_) {
        environmentalHistory_.erase(environmentalHistory_.begin());
    }
    
    // Check for significant weather changes
    if (lastEnvironmentalUpdate_ > 0 && isWeatherChangeSignificant()) {
        if (g_weatherChangeCallback) {
            g_weatherChangeCallback(oldConditions, currentConditions_);
        }
    }
    
    // Detect environmental events
    if (environmentalHistory_.size() > 1) {
        std::vector<EnvironmentalEvent> events = detectEnvironmentalEvents();
        for (const auto& event : events) {
            processEnvironmentalEvent(event);
        }
    }
    
    lastEnvironmentalUpdate_ = millis();
}

void EnvironmentalCorrelationEngine::updateEnvironmentalConditions(const AdvancedEnvironmentalData& envData) {
    updateEnvironmentalConditions(
        envData.temperature_C,
        envData.humidity_percent,
        envData.pressure_hPa,
        envData.windSpeed_mps,
        static_cast<uint8_t>(envData.lightLevel_lux / 1000)  // Simplified conversion
    );
}

void EnvironmentalCorrelationEngine::analyzeSpeciesEnvironmentalCorrelation(const String& species, 
                                                                            const BehaviorAnalysisResult& behavior) {
    if (!initialized_ || environmentalHistory_.empty()) return;
    
    // Get or create correlation data for this species
    EnvironmentalCorrelation& correlation = speciesCorrelations_[species];
    
    // Update correlation based on current behavior and environmental conditions
    float activityLevel = 0.5;  // Default activity level
    
    switch (behavior.primaryBehavior) {
        case BEHAVIOR_FEEDING:
        case BEHAVIOR_MOVING:
        case BEHAVIOR_HUNTING:
            activityLevel = 0.8;
            break;
        case BEHAVIOR_ALERT:
        case BEHAVIOR_SOCIAL:
            activityLevel = 0.7;
            break;
        case BEHAVIOR_RESTING:
            activityLevel = 0.2;
            break;
        case BEHAVIOR_FLEEING:
            activityLevel = 0.9;
            break;
        default:
            activityLevel = 0.5;
            break;
    }
    
    updateCorrelationMetrics(correlation, currentConditions_, activityLevel);
    
    // Update time-based activity patterns
    TimePeriod currentPeriod = getCurrentTimePeriod();
    correlation.timeActivityLevels[currentPeriod] = 
        (correlation.timeActivityLevels[currentPeriod] + activityLevel) / 2.0;
    
    // Update seasonal patterns
    Season currentSeason = getCurrentSeason();
    correlation.seasonalActivityLevels[currentSeason] = 
        (correlation.seasonalActivityLevels[currentSeason] + activityLevel) / 2.0;
    
    correlation.observationCount++;
    correlation.lastUpdated = millis();
}

float EnvironmentalCorrelationEngine::predictSpeciesActivity(const String& species, 
                                                            const EnvironmentalReading& conditions) {
    auto correlationIt = speciesCorrelations_.find(species);
    if (correlationIt == speciesCorrelations_.end()) {
        return 0.5;  // Default activity level for unknown species
    }
    
    const EnvironmentalCorrelation& correlation = correlationIt->second;
    float activityScore = 0.0;
    int factorCount = 0;
    
    // Temperature factor
    float tempDiff = abs(conditions.temperature_C - correlation.temperatureOptimal_C);
    if (tempDiff <= correlation.temperatureRange_C) {
        float tempScore = 1.0 - (tempDiff / correlation.temperatureRange_C);
        activityScore += tempScore;
        factorCount++;
    }
    
    // Humidity factor
    float humidityDiff = abs(conditions.humidity_percent - correlation.humidityOptimal_percent);
    if (humidityDiff <= correlation.humidityRange_percent) {
        float humidityScore = 1.0 - (humidityDiff / correlation.humidityRange_percent);
        activityScore += humidityScore;
        factorCount++;
    }
    
    // Wind factor
    if (conditions.windSpeed_mps <= correlation.windSpeedTolerance_mps) {
        float windScore = 1.0 - (conditions.windSpeed_mps / correlation.windSpeedTolerance_mps);
        activityScore += windScore;
        factorCount++;
    }
    
    // Time of day factor
    TimePeriod currentPeriod = calculateTimePeriod(conditions.timestamp);
    auto timeIt = correlation.timeActivityLevels.find(currentPeriod);
    if (timeIt != correlation.timeActivityLevels.end()) {
        activityScore += timeIt->second;
        factorCount++;
    }
    
    // Weather condition factor
    auto weatherIt = correlation.weatherPreferences.find(conditions.condition);
    if (weatherIt != correlation.weatherPreferences.end()) {
        activityScore += weatherIt->second;
        factorCount++;
    }
    
    // Calculate average activity score
    float predictedActivity = factorCount > 0 ? activityScore / factorCount : 0.5;
    
    // Storm sensitivity check
    if (correlation.isStormSensitive && conditions.condition == ENV_STORMY) {
        predictedActivity *= 0.3;  // Significant reduction during storms
    }
    
    return std::max(0.0f, std::min(1.0f, predictedActivity));
}

std::vector<String> EnvironmentalCorrelationEngine::predictActiveSpecies(const EnvironmentalReading& conditions, 
                                                                         float threshold) {
    std::vector<String> activeSpecies;
    
    for (const auto& correlationPair : speciesCorrelations_) {
        const String& species = correlationPair.first;
        float predictedActivity = predictSpeciesActivity(species, conditions);
        
        if (predictedActivity >= threshold) {
            activeSpecies.push_back(species);
            
            if (g_activityPredictionCallback) {
                g_activityPredictionCallback(species, predictedActivity);
            }
        }
    }
    
    return activeSpecies;
}

std::vector<EnvironmentalEvent> EnvironmentalCorrelationEngine::detectEnvironmentalEvents() {
    std::vector<EnvironmentalEvent> events;
    
    if (environmentalHistory_.size() < 2) {
        return events;
    }
    
    const EnvironmentalReading& current = environmentalHistory_.back();
    const EnvironmentalReading& previous = environmentalHistory_[environmentalHistory_.size() - 2];
    
    // Temperature change event
    if (isTemperatureChangeEvent(current, previous)) {
        EnvironmentalEvent event;
        event.timestamp = current.timestamp;
        event.eventType = "temperature_change";
        event.magnitude = abs(current.temperature_C - previous.temperature_C);
        event.duration_ms = 300000;  // Assume 5 minute duration
        
        // Add species likely to be affected
        for (const auto& correlationPair : speciesCorrelations_) {
            const String& species = correlationPair.first;
            const EnvironmentalCorrelation& correlation = correlationPair.second;
            
            if (event.magnitude > correlation.temperatureChangeTrigger_C) {
                event.affectedSpecies.push_back(species);
            }
        }
        
        if (!event.affectedSpecies.empty()) {
            events.push_back(event);
        }
    }
    
    // Pressure change event
    if (isPressureChangeEvent(current, previous)) {
        EnvironmentalEvent event;
        event.timestamp = current.timestamp;
        event.eventType = "pressure_change";
        event.magnitude = abs(current.pressure_hPa - previous.pressure_hPa);
        event.duration_ms = 600000;  // Assume 10 minute duration
        
        // Add species likely to be affected (many animals are sensitive to pressure changes)
        for (const auto& correlationPair : speciesCorrelations_) {
            const String& species = correlationPair.first;
            const EnvironmentalCorrelation& correlation = correlationPair.second;
            
            if (event.magnitude > correlation.pressureChangeTrigger_hPa) {
                event.affectedSpecies.push_back(species);
                event.triggeredBehaviors.push_back(BEHAVIOR_ALERT);
            }
        }
        
        if (!event.affectedSpecies.empty()) {
            events.push_back(event);
        }
    }
    
    // Weather transition event
    if (isWeatherTransitionEvent(current, previous)) {
        EnvironmentalEvent event;
        event.timestamp = current.timestamp;
        event.eventType = "weather_transition";
        event.magnitude = 1.0;  // Qualitative change
        event.duration_ms = 1800000;  // Assume 30 minute duration
        
        // All species may be affected by weather transitions
        for (const auto& correlationPair : speciesCorrelations_) {
            event.affectedSpecies.push_back(correlationPair.first);
        }
        
        events.push_back(event);
    }
    
    return events;
}

void EnvironmentalCorrelationEngine::processEnvironmentalEvent(const EnvironmentalEvent& event) {
    recentEvents_.push_back(event);
    
    // Limit recent events
    if (recentEvents_.size() > 50) {
        recentEvents_.erase(recentEvents_.begin());
    }
    
    if (g_environmentalEventCallback) {
        g_environmentalEventCallback(event);
    }
    
    Serial.println("Environmental event detected: " + event.eventType + 
                  " (magnitude: " + String(event.magnitude) + 
                  ", affected species: " + String(event.affectedSpecies.size()) + ")");
}

bool EnvironmentalCorrelationEngine::isWeatherChangeSignificant(float threshold) {
    if (environmentalHistory_.size() < 2) {
        return false;
    }
    
    const EnvironmentalReading& current = environmentalHistory_.back();
    const EnvironmentalReading& previous = environmentalHistory_[environmentalHistory_.size() - 2];
    
    // Check for significant changes in any parameter
    float tempChange = abs(current.temperature_C - previous.temperature_C) / 30.0;  // Normalize to 30°C range
    float humidityChange = abs(current.humidity_percent - previous.humidity_percent) / 100.0;
    float pressureChange = abs(current.pressure_hPa - previous.pressure_hPa) / 100.0;  // Normalize to 100 hPa range
    float windChange = abs(current.windSpeed_mps - previous.windSpeed_mps) / 20.0;  // Normalize to 20 m/s range
    
    float totalChange = tempChange + humidityChange + pressureChange + windChange;
    
    return totalChange > threshold;
}

TimePeriod EnvironmentalCorrelationEngine::getCurrentTimePeriod() {
    return calculateTimePeriod(millis());
}

Season EnvironmentalCorrelationEngine::getCurrentSeason() {
    return calculateSeason(millis());
}

EnvironmentalCondition EnvironmentalCorrelationEngine::classifyWeatherCondition(const EnvironmentalReading& reading) {
    return determineWeatherCondition(reading.temperature_C, reading.humidity_percent,
                                   reading.pressure_hPa, reading.windSpeed_mps);
}

void EnvironmentalCorrelationEngine::updateCorrelationMetrics(EnvironmentalCorrelation& correlation,
                                                             const EnvironmentalReading& conditions, 
                                                             float activityLevel) {
    // Update optimal temperature (moving average)
    if (activityLevel > 0.7) {  // High activity
        correlation.temperatureOptimal_C = 
            (correlation.temperatureOptimal_C * 0.9) + (conditions.temperature_C * 0.1);
        correlation.humidityOptimal_percent = 
            (correlation.humidityOptimal_percent * 0.9) + (conditions.humidity_percent * 0.1);
    }
    
    // Update weather preferences
    correlation.weatherPreferences[conditions.condition] = 
        (correlation.weatherPreferences[conditions.condition] + activityLevel) / 2.0;
    
    // Update correlation strength (simplified)
    correlation.correlationStrength = std::min(1.0f, correlation.correlationStrength + 0.01f);
}

TimePeriod EnvironmentalCorrelationEngine::calculateTimePeriod(uint32_t timestamp) {
    // Simplified time calculation - in practice would use RTC
    uint32_t minutesSinceMidnight = (timestamp / 60000) % 1440;
    
    if (minutesSinceMidnight >= 300 && minutesSinceMidnight < 420) {  // 5-7 AM
        return TIME_DAWN;
    } else if (minutesSinceMidnight >= 420 && minutesSinceMidnight < 660) {  // 7-11 AM
        return TIME_MORNING;
    } else if (minutesSinceMidnight >= 660 && minutesSinceMidnight < 840) {  // 11 AM - 2 PM
        return TIME_MIDDAY;
    } else if (minutesSinceMidnight >= 840 && minutesSinceMidnight < 1080) {  // 2-6 PM
        return TIME_AFTERNOON;
    } else if (minutesSinceMidnight >= 1080 && minutesSinceMidnight < 1200) {  // 6-8 PM
        return TIME_DUSK;
    } else {  // 8 PM - 5 AM
        return TIME_NIGHT;
    }
}

Season EnvironmentalCorrelationEngine::calculateSeason(uint32_t timestamp) {
    // Simplified season calculation
    uint32_t dayOfYear = (timestamp / 86400000) % 365;
    return static_cast<Season>((dayOfYear / 91) % 4);
}

EnvironmentalCondition EnvironmentalCorrelationEngine::determineWeatherCondition(float temperature, 
                                                                                float humidity,
                                                                                float pressure, 
                                                                                float windSpeed) {
    // Simplified weather classification
    if (windSpeed > 10.0) {
        return ENV_WINDY;
    } else if (pressure < 1000.0) {
        return ENV_STORMY;
    } else if (humidity > 90.0) {
        if (temperature < 5.0) {
            return ENV_SNOWY;
        } else {
            return ENV_RAINY;
        }
    } else if (humidity > 70.0) {
        return ENV_CLOUDY;
    } else if (humidity < 30.0) {
        return ENV_SUNNY;
    } else {
        return ENV_CLOUDY;  // Default
    }
}

bool EnvironmentalCorrelationEngine::isTemperatureChangeEvent(const EnvironmentalReading& current, 
                                                             const EnvironmentalReading& previous) {
    return abs(current.temperature_C - previous.temperature_C) > 3.0;  // 3°C threshold
}

bool EnvironmentalCorrelationEngine::isPressureChangeEvent(const EnvironmentalReading& current, 
                                                          const EnvironmentalReading& previous) {
    return abs(current.pressure_hPa - previous.pressure_hPa) > 5.0;  // 5 hPa threshold
}

bool EnvironmentalCorrelationEngine::isWeatherTransitionEvent(const EnvironmentalReading& current, 
                                                             const EnvironmentalReading& previous) {
    return current.condition != previous.condition;
}

String EnvironmentalCorrelationEngine::getCorrelationReport(const String& species) const {
    auto correlationIt = speciesCorrelations_.find(species);
    if (correlationIt == speciesCorrelations_.end()) {
        return "No correlation data found for species: " + species;
    }
    
    const EnvironmentalCorrelation& correlation = correlationIt->second;
    
    String report = "Environmental Correlation Report for " + species + ":\n";
    report += "Optimal Temperature: " + String(correlation.temperatureOptimal_C, 1) + "°C\n";
    report += "Optimal Humidity: " + String(correlation.humidityOptimal_percent, 1) + "%\n";
    report += "Wind Tolerance: " + String(correlation.windSpeedTolerance_mps, 1) + " m/s\n";
    report += "Storm Sensitive: " + String(correlation.isStormSensitive ? "Yes" : "No") + "\n";
    report += "Correlation Strength: " + String(correlation.correlationStrength, 2) + "\n";
    report += "Observations: " + String(correlation.observationCount) + "\n";
    
    report += "Time Activity Levels:\n";
    for (const auto& timePair : correlation.timeActivityLevels) {
        String periodName;
        switch (timePair.first) {
            case TIME_DAWN: periodName = "Dawn"; break;
            case TIME_MORNING: periodName = "Morning"; break;
            case TIME_MIDDAY: periodName = "Midday"; break;
            case TIME_AFTERNOON: periodName = "Afternoon"; break;
            case TIME_DUSK: periodName = "Dusk"; break;
            case TIME_NIGHT: periodName = "Night"; break;
            default: periodName = "Unknown"; break;
        }
        report += "  " + periodName + ": " + String(timePair.second * 100.0, 1) + "%\n";
    }
    
    return report;
}

String EnvironmentalCorrelationEngine::getEnvironmentalSummary() const {
    String summary = "Environmental Conditions Summary:\n";
    summary += "Current Temperature: " + String(currentConditions_.temperature_C, 1) + "°C\n";
    summary += "Current Humidity: " + String(currentConditions_.humidity_percent, 1) + "%\n";
    summary += "Current Pressure: " + String(currentConditions_.pressure_hPa, 1) + " hPa\n";
    summary += "Current Wind Speed: " + String(currentConditions_.windSpeed_mps, 1) + " m/s\n";
    summary += "Current Light Level: " + String(currentConditions_.lightLevel) + "\n";
    
    summary += "Recent Events: " + String(recentEvents_.size()) + "\n";
    summary += "Environmental History: " + String(environmentalHistory_.size()) + " readings\n";
    summary += "Tracked Species: " + String(speciesCorrelations_.size()) + "\n";
    
    return summary;
}

// Callback registration functions
void setEnvironmentalEventCallback(EnvironmentalEventCallback callback) {
    g_environmentalEventCallback = callback;
}

void setWeatherChangeCallback(WeatherChangeCallback callback) {
    g_weatherChangeCallback = callback;
}

void setActivityPredictionCallback(ActivityPredictionCallback callback) {
    g_activityPredictionCallback = callback;
}