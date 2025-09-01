/**
 * @file environmental_correlation.cpp
 * @brief Weather/Time Pattern Analysis for Wildlife Behavior Correlation
 * 
 * This implementation correlates wildlife detection and behavior patterns
 * with environmental conditions including weather, time, and seasonal
 * factors to enhance prediction accuracy and ecological insights.
 * 
 * Phase 2 Enhancement - March 31, 2026 Target
 */

#include "../../../firmware/src/ai/tensorflow_lite_implementation.h"
#include "../../../include/network/mesh_protocol.h"
#include "wildlife_detection.cpp"
#include "behavior_analysis.cpp"
#include <ArduinoJson.h>
#include <map>
#include <cmath>

// ===========================
// ENVIRONMENTAL DATA STRUCTURES
// ===========================

struct WeatherConditions {
    float temperature;        // Celsius
    float humidity;          // Percentage 0-100
    float pressure;          // hPa
    float windSpeed;         // m/s
    uint16_t windDirection;  // Degrees 0-360
    uint8_t cloudCover;      // Percentage 0-100
    float precipitation;     // mm/hour
    uint8_t uvIndex;         // 0-11
    float visibility;        // km
    uint32_t timestamp;
};

struct TimeContext {
    uint8_t hour;           // 0-23
    uint8_t dayOfWeek;      // 0=Sunday, 6=Saturday
    uint8_t dayOfMonth;     // 1-31
    uint8_t month;          // 1-12
    uint16_t dayOfYear;     // 1-366
    bool isDaylight;        // True if daylight hours
    uint8_t season;         // 0=Spring, 1=Summer, 2=Fall, 3=Winter
    uint32_t timestamp;
};

struct EnvironmentalPattern {
    WildlifeSpecies species;
    BehaviorType behavior;
    float temperatureRange[2];   // Min, Max preferred temperature
    float humidityRange[2];      // Min, Max preferred humidity
    float pressureRange[2];      // Min, Max preferred pressure
    uint8_t preferredHours[24];  // Activity level by hour (0-100)
    uint8_t seasonalActivity[4]; // Activity by season (0-100)
    float weatherCorrelation[8]; // Correlation with weather types
    uint32_t sampleCount;        // Number of observations
    float confidence;            // Pattern confidence 0-1
};

struct CorrelationResult {
    float environmentalScore;     // Overall environmental match 0-1
    float temperatureScore;       // Temperature correlation 0-1
    float weatherScore;          // Weather condition correlation 0-1
    float temporalScore;         // Time/season correlation 0-1
    float activityPrediction;    // Predicted activity level 0-1
    float detectionProbability;  // Likelihood of detection 0-1
    std::vector<String> insights; // Ecological insights
    uint32_t timestamp;
    bool isValid;
};

// ===========================
// ENVIRONMENTAL CORRELATION ENGINE
// ===========================

class EnvironmentalCorrelationEngine {
private:
    // Pattern storage
    std::map<WildlifeSpecies, EnvironmentalPattern> speciesPatterns_;
    std::map<BehaviorType, EnvironmentalPattern> behaviorPatterns_;
    
    // Environmental data history
    std::vector<WeatherConditions> weatherHistory_;
    std::vector<TimeContext> timeHistory_;
    
    // Correlation data
    std::vector<std::pair<WildlifeDetectionResult, WeatherConditions>> detectionWeatherPairs_;
    std::vector<std::pair<BehaviorAnalysisResult, WeatherConditions>> behaviorWeatherPairs_;
    
    // Analysis parameters
    uint32_t maxHistorySize_;
    uint32_t minSampleSize_;
    float correlationThreshold_;
    float predictionWindow_;
    
    // Current conditions
    WeatherConditions currentWeather_;
    TimeContext currentTime_;
    bool hasCurrentData_;
    
public:
    EnvironmentalCorrelationEngine() : 
        maxHistorySize_(1000), minSampleSize_(10), correlationThreshold_(0.6),
        predictionWindow_(3600000), hasCurrentData_(false) {
        
        // Initialize default patterns
        initializeDefaultPatterns();
        
        // Reserve space for histories
        weatherHistory_.reserve(maxHistorySize_);
        timeHistory_.reserve(maxHistorySize_);
        detectionWeatherPairs_.reserve(maxHistorySize_);
        behaviorWeatherPairs_.reserve(maxHistorySize_);
    }
    
    // ===========================
    // INITIALIZATION
    // ===========================
    
    /**
     * Initialize environmental correlation system
     */
    bool initialize() {
        Serial.println("Initializing environmental correlation engine");
        
        // Load existing patterns from storage if available
        // For now, use default patterns
        
        // Initialize current time context
        updateTimeContext();
        
        Serial.println("Environmental correlation engine initialized");
        return true;
    }
    
    /**
     * Initialize default environmental patterns for common species
     */
    void initializeDefaultPatterns() {
        // White-tailed deer pattern
        EnvironmentalPattern deerPattern = {};
        deerPattern.species = SPECIES_DEER_WHITE_TAILED;
        deerPattern.temperatureRange[0] = -10.0;  // Min temp
        deerPattern.temperatureRange[1] = 30.0;   // Max temp
        deerPattern.humidityRange[0] = 30.0;
        deerPattern.humidityRange[1] = 90.0;
        
        // Dawn and dusk activity (crepuscular)
        for (int i = 0; i < 24; i++) {
            if (i >= 5 && i <= 8) {
                deerPattern.preferredHours[i] = 90; // Dawn
            } else if (i >= 17 && i <= 20) {
                deerPattern.preferredHours[i] = 90; // Dusk
            } else if (i >= 9 && i <= 16) {
                deerPattern.preferredHours[i] = 30; // Day
            } else {
                deerPattern.preferredHours[i] = 10; // Night
            }
        }
        
        // Seasonal activity
        deerPattern.seasonalActivity[0] = 80; // Spring
        deerPattern.seasonalActivity[1] = 70; // Summer
        deerPattern.seasonalActivity[2] = 90; // Fall
        deerPattern.seasonalActivity[3] = 60; // Winter
        
        deerPattern.confidence = 0.8;
        speciesPatterns_[SPECIES_DEER_WHITE_TAILED] = deerPattern;
        
        // Add more species patterns...
        initializeBearPattern();
        initializeBirdPatterns();
        initializeBehaviorPatterns();
    }
    
    // ===========================
    // DATA COLLECTION
    // ===========================
    
    /**
     * Update current weather conditions
     */
    void updateWeatherConditions(const WeatherConditions& weather) {
        currentWeather_ = weather;
        hasCurrentData_ = true;
        
        // Add to history
        weatherHistory_.push_back(weather);
        if (weatherHistory_.size() > maxHistorySize_) {
            weatherHistory_.erase(weatherHistory_.begin());
        }
        
        Serial.printf("Weather updated: T=%.1f°C, H=%.0f%%, P=%.1fhPa\n",
                     weather.temperature, weather.humidity, weather.pressure);
    }
    
    /**
     * Update time context
     */
    void updateTimeContext() {
        // Get current time - would typically use RTC or NTP
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        
        currentTime_.hour = timeinfo->tm_hour;
        currentTime_.dayOfWeek = timeinfo->tm_wday;
        currentTime_.dayOfMonth = timeinfo->tm_mday;
        currentTime_.month = timeinfo->tm_mon + 1;
        currentTime_.dayOfYear = timeinfo->tm_yday + 1;
        currentTime_.timestamp = millis();
        
        // Determine daylight (simplified)
        currentTime_.isDaylight = (currentTime_.hour >= 6 && currentTime_.hour <= 18);
        
        // Determine season (Northern Hemisphere)
        if (currentTime_.month >= 3 && currentTime_.month <= 5) {
            currentTime_.season = 0; // Spring
        } else if (currentTime_.month >= 6 && currentTime_.month <= 8) {
            currentTime_.season = 1; // Summer
        } else if (currentTime_.month >= 9 && currentTime_.month <= 11) {
            currentTime_.season = 2; // Fall
        } else {
            currentTime_.season = 3; // Winter
        }
        
        // Add to history
        timeHistory_.push_back(currentTime_);
        if (timeHistory_.size() > maxHistorySize_) {
            timeHistory_.erase(timeHistory_.begin());
        }
    }
    
    /**
     * Record wildlife detection with environmental context
     */
    void recordDetection(const WildlifeDetectionResult& detection) {
        if (!hasCurrentData_) return;
        
        detectionWeatherPairs_.push_back({detection, currentWeather_});
        if (detectionWeatherPairs_.size() > maxHistorySize_) {
            detectionWeatherPairs_.erase(detectionWeatherPairs_.begin());
        }
        
        // Update species pattern
        updateSpeciesPattern(detection);
    }
    
    /**
     * Record behavior analysis with environmental context
     */
    void recordBehavior(const BehaviorAnalysisResult& behavior) {
        if (!hasCurrentData_) return;
        
        behaviorWeatherPairs_.push_back({behavior, currentWeather_});
        if (behaviorWeatherPairs_.size() > maxHistorySize_) {
            behaviorWeatherPairs_.erase(behaviorWeatherPairs_.begin());
        }
        
        // Update behavior pattern
        updateBehaviorPattern(behavior);
    }
    
    // ===========================
    // CORRELATION ANALYSIS
    // ===========================
    
    /**
     * Analyze environmental correlation for detection
     */
    CorrelationResult analyzeDetectionCorrelation(const WildlifeDetectionResult& detection) {
        CorrelationResult result = {};
        result.timestamp = millis();
        result.isValid = false;
        
        if (!hasCurrentData_) return result;
        
        // Get species pattern
        auto patternIt = speciesPatterns_.find(detection.primarySpecies);
        if (patternIt == speciesPatterns_.end()) {
            // Use generic pattern or create one
            result.environmentalScore = 0.5;
            result.isValid = true;
            return result;
        }
        
        const EnvironmentalPattern& pattern = patternIt->second;
        
        // Calculate environmental scores
        result.temperatureScore = calculateTemperatureScore(pattern);
        result.weatherScore = calculateWeatherScore(pattern);
        result.temporalScore = calculateTemporalScore(pattern);
        
        // Overall environmental score
        result.environmentalScore = (result.temperatureScore * 0.4 + 
                                   result.weatherScore * 0.3 + 
                                   result.temporalScore * 0.3);
        
        // Predict activity and detection probability
        result.activityPrediction = predictActivityLevel(pattern);
        result.detectionProbability = calculateDetectionProbability(pattern, result.environmentalScore);
        
        // Generate ecological insights
        result.insights = generateInsights(detection.primarySpecies, pattern, result);
        
        result.isValid = true;
        return result;
    }
    
    /**
     * Analyze environmental correlation for behavior
     */
    CorrelationResult analyzeBehaviorCorrelation(const BehaviorAnalysisResult& behavior) {
        CorrelationResult result = {};
        result.timestamp = millis();
        result.isValid = false;
        
        if (!hasCurrentData_) return result;
        
        // Get behavior pattern
        auto patternIt = behaviorPatterns_.find(behavior.primaryBehavior);
        if (patternIt == behaviorPatterns_.end()) {
            result.environmentalScore = 0.5;
            result.isValid = true;
            return result;
        }
        
        const EnvironmentalPattern& pattern = patternIt->second;
        
        // Calculate correlation scores
        result.temperatureScore = calculateTemperatureScore(pattern);
        result.weatherScore = calculateWeatherScore(pattern);
        result.temporalScore = calculateTemporalScore(pattern);
        
        result.environmentalScore = (result.temperatureScore * 0.3 + 
                                   result.weatherScore * 0.4 + 
                                   result.temporalScore * 0.3);
        
        result.activityPrediction = predictBehaviorProbability(pattern);
        
        result.insights = generateBehaviorInsights(behavior.primaryBehavior, pattern, result);
        
        result.isValid = true;
        return result;
    }
    
    /**
     * Predict optimal conditions for species detection
     */
    CorrelationResult predictOptimalConditions(WildlifeSpecies species) {
        CorrelationResult result = {};
        result.timestamp = millis();
        result.isValid = false;
        
        auto patternIt = speciesPatterns_.find(species);
        if (patternIt == speciesPatterns_.end()) {
            return result;
        }
        
        const EnvironmentalPattern& pattern = patternIt->second;
        
        // Find optimal time of day
        uint8_t bestHour = 0;
        uint8_t maxActivity = 0;
        for (int i = 0; i < 24; i++) {
            if (pattern.preferredHours[i] > maxActivity) {
                maxActivity = pattern.preferredHours[i];
                bestHour = i;
            }
        }
        
        // Calculate prediction for current conditions
        result.temperatureScore = calculateTemperatureScore(pattern);
        result.weatherScore = calculateWeatherScore(pattern);
        result.temporalScore = calculateTemporalScore(pattern);
        
        result.environmentalScore = (result.temperatureScore * 0.4 + 
                                   result.weatherScore * 0.3 + 
                                   result.temporalScore * 0.3);
        
        result.activityPrediction = predictActivityLevel(pattern);
        result.detectionProbability = calculateDetectionProbability(pattern, result.environmentalScore);
        
        // Generate prediction insights
        result.insights.push_back("Optimal hour: " + String(bestHour) + ":00");
        result.insights.push_back("Preferred temperature: " + 
                                String(pattern.temperatureRange[0]) + "-" + 
                                String(pattern.temperatureRange[1]) + "°C");
        
        result.isValid = true;
        return result;
    }
    
    // ===========================
    // PATTERN LEARNING
    // ===========================
    
    /**
     * Update species pattern based on new detection
     */
    void updateSpeciesPattern(const WildlifeDetectionResult& detection) {
        auto& pattern = speciesPatterns_[detection.primarySpecies];
        
        // Update temperature preferences
        float temp = currentWeather_.temperature;
        pattern.temperatureRange[0] = std::min(pattern.temperatureRange[0], temp);
        pattern.temperatureRange[1] = std::max(pattern.temperatureRange[1], temp);
        
        // Update humidity preferences
        float humidity = currentWeather_.humidity;
        pattern.humidityRange[0] = std::min(pattern.humidityRange[0], humidity);
        pattern.humidityRange[1] = std::max(pattern.humidityRange[1], humidity);
        
        // Update hourly activity
        uint8_t hour = currentTime_.hour;
        pattern.preferredHours[hour] = std::min(100, pattern.preferredHours[hour] + 5);
        
        // Update seasonal activity
        pattern.seasonalActivity[currentTime_.season] = 
            std::min(100, pattern.seasonalActivity[currentTime_.season] + 3);
        
        pattern.sampleCount++;
        
        // Update confidence based on sample size
        pattern.confidence = std::min(1.0f, (float)pattern.sampleCount / 50.0f);
    }
    
    /**
     * Update behavior pattern based on new observation
     */
    void updateBehaviorPattern(const BehaviorAnalysisResult& behavior) {
        auto& pattern = behaviorPatterns_[behavior.primaryBehavior];
        
        // Update environmental preferences for this behavior
        float temp = currentWeather_.temperature;
        pattern.temperatureRange[0] = std::min(pattern.temperatureRange[0], temp);
        pattern.temperatureRange[1] = std::max(pattern.temperatureRange[1], temp);
        
        // Update temporal patterns
        uint8_t hour = currentTime_.hour;
        pattern.preferredHours[hour] = std::min(100, pattern.preferredHours[hour] + 3);
        
        pattern.sampleCount++;
        pattern.confidence = std::min(1.0f, (float)pattern.sampleCount / 30.0f);
    }
    
    // ===========================
    // SCORING FUNCTIONS
    // ===========================
    
    /**
     * Calculate temperature correlation score
     */
    float calculateTemperatureScore(const EnvironmentalPattern& pattern) {
        float temp = currentWeather_.temperature;
        float minTemp = pattern.temperatureRange[0];
        float maxTemp = pattern.temperatureRange[1];
        
        if (temp >= minTemp && temp <= maxTemp) {
            // Within preferred range
            float center = (minTemp + maxTemp) / 2.0;
            float distance = abs(temp - center);
            float range = (maxTemp - minTemp) / 2.0;
            
            if (range > 0) {
                return 1.0 - (distance / range) * 0.5; // Score 0.5 to 1.0
            } else {
                return 1.0; // Perfect match if range is zero
            }
        } else {
            // Outside preferred range
            float distance = std::min(abs(temp - minTemp), abs(temp - maxTemp));
            return std::max(0.0f, 0.5f - distance * 0.02f); // Decrease by 2% per degree
        }
    }
    
    /**
     * Calculate weather condition correlation score
     */
    float calculateWeatherScore(const EnvironmentalPattern& pattern) {
        float score = 0.5; // Base score
        
        // Humidity factor
        float humidity = currentWeather_.humidity;
        if (humidity >= pattern.humidityRange[0] && humidity <= pattern.humidityRange[1]) {
            score += 0.2;
        }
        
        // Precipitation factor (most wildlife avoids heavy rain)
        if (currentWeather_.precipitation > 5.0) {
            score -= 0.3; // Heavy rain reduces activity
        } else if (currentWeather_.precipitation > 1.0) {
            score -= 0.1; // Light rain slightly reduces activity
        }
        
        // Wind factor (high wind reduces activity)
        if (currentWeather_.windSpeed > 15.0) {
            score -= 0.2; // Strong wind
        } else if (currentWeather_.windSpeed > 8.0) {
            score -= 0.1; // Moderate wind
        }
        
        // Cloud cover (affects visibility)
        if (currentWeather_.cloudCover > 80) {
            score -= 0.1; // Very cloudy
        }
        
        return std::max(0.0f, std::min(1.0f, score));
    }
    
    /**
     * Calculate temporal correlation score
     */
    float calculateTemporalScore(const EnvironmentalPattern& pattern) {
        float score = 0.0;
        
        // Hourly activity score
        uint8_t hour = currentTime_.hour;
        float hourlyScore = pattern.preferredHours[hour] / 100.0f;
        score += hourlyScore * 0.6;
        
        // Seasonal activity score
        uint8_t season = currentTime_.season;
        float seasonalScore = pattern.seasonalActivity[season] / 100.0f;
        score += seasonalScore * 0.4;
        
        return std::max(0.0f, std::min(1.0f, score));
    }
    
    /**
     * Predict activity level based on current conditions
     */
    float predictActivityLevel(const EnvironmentalPattern& pattern) {
        float baseActivity = pattern.preferredHours[currentTime_.hour] / 100.0f;
        float seasonalModifier = pattern.seasonalActivity[currentTime_.season] / 100.0f;
        float weatherModifier = calculateWeatherScore(pattern);
        
        return baseActivity * seasonalModifier * weatherModifier;
    }
    
    /**
     * Calculate detection probability
     */
    float calculateDetectionProbability(const EnvironmentalPattern& pattern, float environmentalScore) {
        float baseProbability = pattern.confidence;
        float activityLevel = predictActivityLevel(pattern);
        
        return baseProbability * activityLevel * environmentalScore;
    }
    
    /**
     * Predict behavior probability
     */
    float predictBehaviorProbability(const EnvironmentalPattern& pattern) {
        float temporalScore = calculateTemporalScore(pattern);
        float weatherScore = calculateWeatherScore(pattern);
        
        return temporalScore * weatherScore * pattern.confidence;
    }
    
    // ===========================
    // INSIGHT GENERATION
    // ===========================
    
    /**
     * Generate ecological insights for species detection
     */
    std::vector<String> generateInsights(WildlifeSpecies species, 
                                       const EnvironmentalPattern& pattern,
                                       const CorrelationResult& result) {
        std::vector<String> insights;
        
        // Temperature insights
        if (result.temperatureScore > 0.8) {
            insights.push_back("Optimal temperature conditions for " + getSpeciesName(species));
        } else if (result.temperatureScore < 0.3) {
            insights.push_back("Temperature outside preferred range for " + getSpeciesName(species));
        }
        
        // Weather insights
        if (currentWeather_.precipitation > 1.0) {
            insights.push_back("Precipitation may reduce activity");
        }
        
        if (currentWeather_.windSpeed > 10.0) {
            insights.push_back("High wind conditions may affect behavior");
        }
        
        // Temporal insights
        if (result.temporalScore > 0.8) {
            insights.push_back("Peak activity time for " + getSpeciesName(species));
        } else if (result.temporalScore < 0.3) {
            insights.push_back("Low activity period for " + getSpeciesName(species));
        }
        
        // Seasonal insights
        String seasonName = getSeasonName(currentTime_.season);
        if (pattern.seasonalActivity[currentTime_.season] > 80) {
            insights.push_back("High seasonal activity in " + seasonName);
        } else if (pattern.seasonalActivity[currentTime_.season] < 40) {
            insights.push_back("Low seasonal activity in " + seasonName);
        }
        
        return insights;
    }
    
    /**
     * Generate behavioral insights
     */
    std::vector<String> generateBehaviorInsights(BehaviorType behavior,
                                                const EnvironmentalPattern& pattern,
                                                const CorrelationResult& result) {
        std::vector<String> insights;
        
        String behaviorName = getBehaviorName(behavior);
        
        if (result.environmentalScore > 0.7) {
            insights.push_back("Favorable conditions for " + behaviorName);
        } else if (result.environmentalScore < 0.4) {
            insights.push_back("Challenging conditions for " + behaviorName);
        }
        
        // Behavior-specific insights
        switch (behavior) {
            case BEHAVIOR_FEEDING:
                if (currentWeather_.precipitation > 0.5) {
                    insights.push_back("Rain may encourage shelter-seeking over feeding");
                }
                break;
                
            case BEHAVIOR_DRINKING:
                if (currentWeather_.humidity < 40) {
                    insights.push_back("Low humidity increases water needs");
                }
                break;
                
            case BEHAVIOR_RESTING:
                if (currentWeather_.temperature > 25) {
                    insights.push_back("High temperature favors resting behavior");
                }
                break;
                
            default:
                break;
        }
        
        return insights;
    }
    
private:
    // ===========================
    // HELPER FUNCTIONS
    // ===========================
    
    void initializeBearPattern() {
        EnvironmentalPattern bearPattern = {};
        bearPattern.species = SPECIES_BEAR_BLACK;
        bearPattern.temperatureRange[0] = -20.0;
        bearPattern.temperatureRange[1] = 25.0;
        bearPattern.humidityRange[0] = 40.0;
        bearPattern.humidityRange[1] = 95.0;
        
        // Bears are most active dawn/dusk and during day
        for (int i = 0; i < 24; i++) {
            if (i >= 6 && i <= 19) {
                bearPattern.preferredHours[i] = 70; // Active during day
            } else {
                bearPattern.preferredHours[i] = 20; // Less active at night
            }
        }
        
        bearPattern.seasonalActivity[0] = 90; // Spring (emerging from hibernation)
        bearPattern.seasonalActivity[1] = 80; // Summer
        bearPattern.seasonalActivity[2] = 95; // Fall (pre-hibernation feeding)
        bearPattern.seasonalActivity[3] = 10; // Winter (hibernation)
        
        bearPattern.confidence = 0.75;
        speciesPatterns_[SPECIES_BEAR_BLACK] = bearPattern;
    }
    
    void initializeBirdPatterns() {
        // General bird pattern
        EnvironmentalPattern birdPattern = {};
        birdPattern.species = SPECIES_BIRD_MEDIUM;
        birdPattern.temperatureRange[0] = -10.0;
        birdPattern.temperatureRange[1] = 35.0;
        
        // Birds are most active during daylight
        for (int i = 0; i < 24; i++) {
            if (i >= 5 && i <= 9) {
                birdPattern.preferredHours[i] = 95; // Dawn chorus
            } else if (i >= 10 && i <= 17) {
                birdPattern.preferredHours[i] = 70; // Day activity
            } else if (i >= 18 && i <= 20) {
                birdPattern.preferredHours[i] = 80; // Evening activity
            } else {
                birdPattern.preferredHours[i] = 10; // Night (most birds)
            }
        }
        
        birdPattern.seasonalActivity[0] = 95; // Spring (breeding)
        birdPattern.seasonalActivity[1] = 80; // Summer
        birdPattern.seasonalActivity[2] = 85; // Fall (migration)
        birdPattern.seasonalActivity[3] = 60; // Winter
        
        birdPattern.confidence = 0.7;
        speciesPatterns_[SPECIES_BIRD_MEDIUM] = birdPattern;
    }
    
    void initializeBehaviorPatterns() {
        // Feeding behavior pattern
        EnvironmentalPattern feedingPattern = {};
        feedingPattern.behavior = BEHAVIOR_FEEDING;
        feedingPattern.temperatureRange[0] = -5.0;
        feedingPattern.temperatureRange[1] = 30.0;
        
        // Feeding often occurs dawn and dusk
        for (int i = 0; i < 24; i++) {
            if (i >= 5 && i <= 8 || i >= 17 && i <= 20) {
                feedingPattern.preferredHours[i] = 90;
            } else {
                feedingPattern.preferredHours[i] = 40;
            }
        }
        
        feedingPattern.confidence = 0.8;
        behaviorPatterns_[BEHAVIOR_FEEDING] = feedingPattern;
        
        // Add more behavior patterns...
    }
    
    String getSpeciesName(WildlifeSpecies species) {
        switch (species) {
            case SPECIES_DEER_WHITE_TAILED: return "White-tailed Deer";
            case SPECIES_BEAR_BLACK: return "Black Bear";
            case SPECIES_BIRD_MEDIUM: return "Medium Bird";
            // Add more species names...
            default: return "Unknown Species";
        }
    }
    
    String getBehaviorName(BehaviorType behavior) {
        switch (behavior) {
            case BEHAVIOR_FEEDING: return "feeding";
            case BEHAVIOR_DRINKING: return "drinking";
            case BEHAVIOR_RESTING: return "resting";
            case BEHAVIOR_WALKING: return "walking";
            case BEHAVIOR_RUNNING: return "running";
            // Add more behavior names...
            default: return "unknown behavior";
        }
    }
    
    String getSeasonName(uint8_t season) {
        switch (season) {
            case 0: return "Spring";
            case 1: return "Summer";
            case 2: return "Fall";
            case 3: return "Winter";
            default: return "Unknown";
        }
    }
    
public:
    // ===========================
    // PUBLIC INTERFACE
    // ===========================
    
    void getPatternStatistics(uint32_t& speciesPatterns, uint32_t& behaviorPatterns,
                             uint32_t& totalObservations) {
        speciesPatterns = speciesPatterns_.size();
        behaviorPatterns = behaviorPatterns_.size();
        totalObservations = detectionWeatherPairs_.size() + behaviorWeatherPairs_.size();
    }
    
    std::vector<WildlifeSpecies> getTrackedSpecies() {
        std::vector<WildlifeSpecies> species;
        for (const auto& pair : speciesPatterns_) {
            species.push_back(pair.first);
        }
        return species;
    }
    
    EnvironmentalPattern getSpeciesPattern(WildlifeSpecies species) {
        auto it = speciesPatterns_.find(species);
        return (it != speciesPatterns_.end()) ? it->second : EnvironmentalPattern{};
    }
    
    void setAnalysisParameters(uint32_t maxHistory, uint32_t minSamples, 
                              float correlationThresh) {
        maxHistorySize_ = maxHistory;
        minSampleSize_ = minSamples;
        correlationThreshold_ = correlationThresh;
    }
};

// ===========================
// PUBLIC API
// ===========================

static EnvironmentalCorrelationEngine* g_correlationEngine = nullptr;

namespace EnvironmentalCorrelation {
    bool initialize() {
        if (g_correlationEngine) {
            delete g_correlationEngine;
        }
        
        g_correlationEngine = new EnvironmentalCorrelationEngine();
        return g_correlationEngine->initialize();
    }
    
    void cleanup() {
        if (g_correlationEngine) {
            delete g_correlationEngine;
            g_correlationEngine = nullptr;
        }
    }
    
    void updateWeather(float temperature, float humidity, float pressure,
                      float windSpeed, uint16_t windDirection, uint8_t cloudCover,
                      float precipitation, uint8_t uvIndex, float visibility) {
        if (g_correlationEngine) {
            WeatherConditions weather = {};
            weather.temperature = temperature;
            weather.humidity = humidity;
            weather.pressure = pressure;
            weather.windSpeed = windSpeed;
            weather.windDirection = windDirection;
            weather.cloudCover = cloudCover;
            weather.precipitation = precipitation;
            weather.uvIndex = uvIndex;
            weather.visibility = visibility;
            weather.timestamp = millis();
            
            g_correlationEngine->updateWeatherConditions(weather);
        }
    }
    
    void updateTime() {
        if (g_correlationEngine) {
            g_correlationEngine->updateTimeContext();
        }
    }
    
    void recordDetection(const WildlifeDetectionResult& detection) {
        if (g_correlationEngine) {
            g_correlationEngine->recordDetection(detection);
        }
    }
    
    void recordBehavior(const BehaviorAnalysisResult& behavior) {
        if (g_correlationEngine) {
            g_correlationEngine->recordBehavior(behavior);
        }
    }
    
    CorrelationResult analyzeDetection(const WildlifeDetectionResult& detection) {
        if (g_correlationEngine) {
            return g_correlationEngine->analyzeDetectionCorrelation(detection);
        }
        return CorrelationResult{};
    }
    
    CorrelationResult analyzeBehavior(const BehaviorAnalysisResult& behavior) {
        if (g_correlationEngine) {
            return g_correlationEngine->analyzeBehaviorCorrelation(behavior);
        }
        return CorrelationResult{};
    }
    
    CorrelationResult predictOptimalConditions(WildlifeSpecies species) {
        if (g_correlationEngine) {
            return g_correlationEngine->predictOptimalConditions(species);
        }
        return CorrelationResult{};
    }
    
    void getStatistics(uint32_t& speciesPatterns, uint32_t& behaviorPatterns,
                      uint32_t& totalObservations) {
        if (g_correlationEngine) {
            g_correlationEngine->getPatternStatistics(speciesPatterns, behaviorPatterns, totalObservations);
        } else {
            speciesPatterns = 0;
            behaviorPatterns = 0;
            totalObservations = 0;
        }
    }
}