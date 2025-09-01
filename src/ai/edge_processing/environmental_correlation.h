/**
 * Environmental Correlation Analysis
 * Phase 2 Enhancement - Weather/time pattern analysis for wildlife behavior
 * 
 * Features:
 * - Weather pattern correlation with wildlife activity
 * - Seasonal behavior prediction
 * - Time-of-day activity patterns
 * - Environmental trigger detection
 * - Climate adaptation analysis
 */

#include "../edge_processing/wildlife_detection.h"
#include "../edge_processing/behavior_analysis.h"
#include "../../firmware/src/sensors/advanced_environmental_sensors.h"
#include <vector>
#include <map>

#ifndef ENVIRONMENTAL_CORRELATION_H
#define ENVIRONMENTAL_CORRELATION_H

// Environmental condition categories
enum EnvironmentalCondition {
    ENV_SUNNY = 0,
    ENV_CLOUDY = 1,
    ENV_RAINY = 2,
    ENV_SNOWY = 3,
    ENV_WINDY = 4,
    ENV_FOGGY = 5,
    ENV_STORMY = 6,
    ENV_UNKNOWN = 7
};

// Time period categories
enum TimePeriod {
    TIME_DAWN = 0,      // 5-7 AM
    TIME_MORNING = 1,   // 7-11 AM
    TIME_MIDDAY = 2,    // 11 AM - 2 PM
    TIME_AFTERNOON = 3, // 2-6 PM
    TIME_DUSK = 4,      // 6-8 PM
    TIME_NIGHT = 5,     // 8 PM - 5 AM
    TIME_UNKNOWN = 6
};

// Seasonal activity patterns
enum Season {
    SEASON_SPRING = 0,
    SEASON_SUMMER = 1,
    SEASON_FALL = 2,
    SEASON_WINTER = 3
};

// Environmental correlation result
struct EnvironmentalCorrelation {
    String speciesName;
    BehaviorCategory behavior;
    
    // Weather correlations
    float temperatureOptimal_C;
    float temperatureRange_C;       // Acceptable range around optimal
    float humidityOptimal_percent;
    float humidityRange_percent;
    float pressureOptimal_hPa;
    float windSpeedTolerance_mps;
    
    // Time correlations
    std::map<TimePeriod, float> timeActivityLevels;  // 0.0 (inactive) to 1.0 (highly active)
    std::map<Season, float> seasonalActivityLevels;
    
    // Weather condition preferences
    std::map<EnvironmentalCondition, float> weatherPreferences;
    
    // Environmental triggers
    float temperatureChangeTrigger_C; // Degree change that triggers activity
    float pressureChangeTrigger_hPa;  // Pressure change that triggers activity
    bool isStormSensitive;
    bool isMoonPhaseSensitive;
    
    // Statistical metrics
    float correlationStrength;       // Overall correlation strength (0.0-1.0)
    uint32_t observationCount;
    uint32_t lastUpdated;
    
    EnvironmentalCorrelation() : temperatureOptimal_C(20.0), temperatureRange_C(10.0),
                                humidityOptimal_percent(50.0), humidityRange_percent(30.0),
                                pressureOptimal_hPa(1013.25), windSpeedTolerance_mps(5.0),
                                temperatureChangeTrigger_C(5.0), pressureChangeTrigger_hPa(10.0),
                                isStormSensitive(false), isMoonPhaseSensitive(false),
                                correlationStrength(0.0), observationCount(0), lastUpdated(0) {}
};

// Environmental event that may trigger wildlife activity
struct EnvironmentalEvent {
    uint32_t timestamp;
    String eventType;               // "temperature_drop", "pressure_change", "storm_approaching"
    float magnitude;                // Strength of the environmental change
    uint32_t duration_ms;           // How long the event lasts
    std::vector<String> affectedSpecies; // Species likely to be affected
    std::vector<BehaviorCategory> triggeredBehaviors;
    
    EnvironmentalEvent() : timestamp(0), magnitude(0.0), duration_ms(0) {}
};

// Weather forecast integration
struct WeatherForecast {
    uint32_t forecastTime;
    float temperature_C;
    float humidity_percent;
    float pressure_hPa;
    float windSpeed_mps;
    EnvironmentalCondition condition;
    float precipitationProbability; // 0.0-1.0
    
    WeatherForecast() : forecastTime(0), temperature_C(0.0), humidity_percent(0.0),
                       pressure_hPa(0.0), windSpeed_mps(0.0), condition(ENV_UNKNOWN),
                       precipitationProbability(0.0) {}
};

class EnvironmentalCorrelationEngine {
private:
    // Species-specific environmental correlations
    std::map<String, EnvironmentalCorrelation> speciesCorrelations_;
    
    // Environmental data history
    struct EnvironmentalReading {
        uint32_t timestamp;
        float temperature_C;
        float humidity_percent;
        float pressure_hPa;
        float windSpeed_mps;
        uint8_t lightLevel;
        EnvironmentalCondition condition;
        
        EnvironmentalReading() : timestamp(0), temperature_C(0.0), humidity_percent(0.0),
                                pressure_hPa(0.0), windSpeed_mps(0.0), lightLevel(0),
                                condition(ENV_UNKNOWN) {}
    };
    
    std::vector<EnvironmentalReading> environmentalHistory_;
    std::vector<EnvironmentalEvent> recentEvents_;
    std::vector<WeatherForecast> forecasts_;
    
    // Configuration
    uint32_t maxHistorySize_;
    uint32_t correlationWindow_ms_;
    float correlationThreshold_;
    bool enablePredictiveModeling_;
    bool enableWeatherForecasting_;
    
    // Current environmental state
    EnvironmentalReading currentConditions_;
    uint32_t lastEnvironmentalUpdate_;
    
    // Prediction models
    struct PredictionModel {
        String speciesName;
        BehaviorCategory behavior;
        std::vector<float> environmentalWeights;  // Weights for temp, humidity, pressure, wind, light
        float bias;
        float accuracy;
        
        PredictionModel() : behavior(BEHAVIOR_UNKNOWN), bias(0.0), accuracy(0.0) {}
    };
    
    std::map<String, std::vector<PredictionModel>> predictionModels_;
    
    bool initialized_;

public:
    EnvironmentalCorrelationEngine();
    ~EnvironmentalCorrelationEngine();
    
    // Initialization and configuration
    bool init();
    void cleanup();
    void setMaxHistorySize(uint32_t size) { maxHistorySize_ = size; }
    void setCorrelationWindow(uint32_t window_ms) { correlationWindow_ms_ = window_ms; }
    void setCorrelationThreshold(float threshold) { correlationThreshold_ = threshold; }
    void enablePredictiveModeling(bool enable) { enablePredictiveModeling_ = enable; }
    void enableWeatherForecasting(bool enable) { enableWeatherForecasting_ = enable; }
    
    // Environmental data management
    void updateEnvironmentalConditions(float temperature, float humidity, float pressure,
                                     float windSpeed, uint8_t lightLevel);
    void updateEnvironmentalConditions(const AdvancedEnvironmentalData& envData);
    EnvironmentalReading getCurrentConditions() const { return currentConditions_; }
    std::vector<EnvironmentalReading> getEnvironmentalHistory(uint32_t timeWindow_ms = 3600000) const;
    
    // Correlation analysis
    void analyzeSpeciesEnvironmentalCorrelation(const String& species, const BehaviorAnalysisResult& behavior);
    EnvironmentalCorrelation getSpeciesCorrelation(const String& species) const;
    void updateSpeciesCorrelation(const String& species, const EnhancedSpeciesResult& detection);
    
    // Environmental event detection
    std::vector<EnvironmentalEvent> detectEnvironmentalEvents();
    void processEnvironmentalEvent(const EnvironmentalEvent& event);
    bool isWeatherChangeSignificant(float threshold = 0.1);
    
    // Activity prediction
    float predictSpeciesActivity(const String& species, const EnvironmentalReading& conditions);
    std::vector<String> predictActiveSpecies(const EnvironmentalReading& conditions, float threshold = 0.7);
    BehaviorCategory predictLikelyBehavior(const String& species, const EnvironmentalReading& conditions);
    
    // Time-based analysis
    TimePeriod getCurrentTimePeriod();
    Season getCurrentSeason();
    float getSpeciesActivityForTime(const String& species, TimePeriod period);
    void updateTimeActivityPatterns(const String& species, const BehaviorAnalysisResult& behavior);
    
    // Weather condition analysis
    EnvironmentalCondition classifyWeatherCondition(const EnvironmentalReading& reading);
    float getSpeciesWeatherPreference(const String& species, EnvironmentalCondition condition);
    void updateWeatherPreferences(const String& species, const EnvironmentalReading& conditions,
                                 float activityLevel);
    
    // Seasonal adaptation
    void analyzeSeasonalAdaptation(const String& species);
    float calculateSeasonalActivityChange(const String& species, Season from, Season to);
    void predictSeasonalBehaviorChanges(const String& species, Season upcomingSeason);
    
    // Weather forecasting integration
    void addWeatherForecast(const WeatherForecast& forecast);
    std::vector<String> predictActiveSpeciesForForecast(const WeatherForecast& forecast);
    void generateActivityForecast(uint32_t hoursAhead = 24);
    
    // Environmental triggers
    void detectEnvironmentalTriggers();
    bool isEnvironmentalTrigger(const EnvironmentalEvent& event, const String& species);
    void addEnvironmentalTrigger(const String& species, const String& triggerType, float threshold);
    
    // Machine learning and adaptation
    void trainPredictionModel(const String& species, BehaviorCategory behavior);
    void adaptModelsToRegion(const String& region);  // "arctic", "temperate", "tropical"
    void updateModelAccuracy(const String& species, BehaviorCategory predicted, 
                           BehaviorCategory actual);
    
    // Climate change analysis
    void analyzeClimateTrends(uint32_t timespan_days = 365);
    float calculateSpeciesClimateResilience(const String& species);
    void detectClimateAdaptationSigns(const String& species);
    
    // Statistics and reporting
    String getCorrelationReport(const String& species) const;
    String getEnvironmentalSummary() const;
    std::map<String, float> getSpeciesActivityPredictions() const;
    float getOverallPredictionAccuracy() const;
    
    // Data export for research
    bool exportCorrelationData(const String& filename);
    bool exportEnvironmentalTimeSeries(const String& filename);

private:
    // Correlation calculation algorithms
    float calculatePearsonCorrelation(const std::vector<float>& x, const std::vector<float>& y);
    float calculateSpearmanCorrelation(const std::vector<float>& x, const std::vector<float>& y);
    void updateCorrelationMetrics(EnvironmentalCorrelation& correlation,
                                 const EnvironmentalReading& conditions, float activityLevel);
    
    // Environmental condition classification
    EnvironmentalCondition determineWeatherCondition(float temperature, float humidity,
                                                   float pressure, float windSpeed);
    TimePeriod calculateTimePeriod(uint32_t timestamp);
    Season calculateSeason(uint32_t timestamp);
    
    // Prediction algorithms
    float sigmoidActivation(float x);
    float calculateActivityProbability(const std::vector<float>& features, const PredictionModel& model);
    std::vector<float> extractEnvironmentalFeatures(const EnvironmentalReading& reading);
    
    // Data analysis helpers
    float calculateMovingAverage(const std::vector<float>& data, uint32_t windowSize);
    float calculateStandardDeviation(const std::vector<float>& data);
    void detectTrendChanges(const std::vector<float>& data, std::vector<uint32_t>& changePoints);
    
    // Event detection algorithms
    bool isTemperatureChangeEvent(const EnvironmentalReading& current, const EnvironmentalReading& previous);
    bool isPressureChangeEvent(const EnvironmentalReading& current, const EnvironmentalReading& previous);
    bool isWeatherTransitionEvent(const EnvironmentalReading& current, const EnvironmentalReading& previous);
    
    // Model training helpers
    void collectTrainingData(const String& species, BehaviorCategory behavior,
                           std::vector<std::vector<float>>& features, std::vector<float>& targets);
    void trainLinearRegression(std::vector<std::vector<float>>& features, std::vector<float>& targets,
                              PredictionModel& model);
    
    // Utility functions
    void cleanupOldData();
    float normalizeTemperature(float temperature);
    float normalizeHumidity(float humidity);
    float normalizePressure(float pressure);
    bool isDataComplete(const EnvironmentalReading& reading);
};

// Environmental correlation callbacks
typedef void (*EnvironmentalEventCallback)(const EnvironmentalEvent& event);
typedef void (*WeatherChangeCallback)(const EnvironmentalReading& oldConditions, 
                                     const EnvironmentalReading& newConditions);
typedef void (*ActivityPredictionCallback)(const String& species, float predictedActivity);

// Global environmental correlation engine
extern EnvironmentalCorrelationEngine environmentalAnalyzer;

// Callback registration functions
void setEnvironmentalEventCallback(EnvironmentalEventCallback callback);
void setWeatherChangeCallback(WeatherChangeCallback callback);
void setActivityPredictionCallback(ActivityPredictionCallback callback);

#endif // ENVIRONMENTAL_CORRELATION_H