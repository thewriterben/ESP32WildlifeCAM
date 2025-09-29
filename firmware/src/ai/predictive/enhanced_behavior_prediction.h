/**
 * @file enhanced_behavior_prediction.h
 * @brief Enhanced Wildlife Behavior Prediction System v4.0
 * @author thewriterben
 * @date 2025-01-01
 * @version 4.0.0
 * 
 * Advanced AI-powered animal behavior forecasting with seasonal adaptation,
 * breeding behavior detection, territorial mapping, and climate impact modeling.
 */

#ifndef ENHANCED_BEHAVIOR_PREDICTION_H
#define ENHANCED_BEHAVIOR_PREDICTION_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <time.h>
#include "predictive_analytics_engine.h"
#include "../../global_network/global_coordination_manager.h"

// Enhanced behavior types for Version 4.0
enum EnhancedBehaviorType {
    BEHAVIOR_FEEDING_PATTERN = 20,
    BEHAVIOR_BREEDING_CYCLE = 21,
    BEHAVIOR_TERRITORIAL_MARKING = 22,
    BEHAVIOR_MIGRATION_PREPARATION = 23,
    BEHAVIOR_CLIMATE_ADAPTATION = 24,
    BEHAVIOR_INTER_SPECIES_INTERACTION = 25,
    BEHAVIOR_RESOURCE_COMPETITION = 26,
    BEHAVIOR_PREDATOR_AVOIDANCE = 27,
    BEHAVIOR_SOCIAL_COORDINATION = 28,
    BEHAVIOR_HABITAT_SELECTION = 29
};

enum SeasonalPhase {
    SEASON_SPRING = 0,
    SEASON_SUMMER = 1,
    SEASON_AUTUMN = 2,
    SEASON_WINTER = 3,
    SEASON_TRANSITION = 4
};

enum ClimateImpactLevel {
    CLIMATE_NO_IMPACT = 0,
    CLIMATE_MINOR_ADAPTATION = 1,
    CLIMATE_MODERATE_CHANGE = 2,
    CLIMATE_MAJOR_DISRUPTION = 3,
    CLIMATE_CRITICAL_THREAT = 4
};

struct SeasonalBehaviorPattern {
    String species;
    SeasonalPhase season;
    EnhancedBehaviorType primaryBehavior;
    float activityLevel;     // 0.0 - 1.0
    uint32_t duration_hours;
    std::vector<uint8_t> timeOfDayPattern; // 24-hour activity pattern
    float confidence;
    time_t lastUpdated;
};

struct BreedingBehaviorData {
    String species;
    time_t breedingSeasonStart;
    time_t breedingSeasonEnd;
    GPSCoordinate nestingSite;
    float territoryRadius_m;
    uint32_t expectedOffspring;
    float breedingSuccessRate;
    std::vector<String> breedingBehaviors;
    bool isBreedingActive;
    time_t nextBreedingCycle;
};

struct TerritorialMapping {
    String species;
    uint32_t territoryId;
    GPSCoordinate centerPoint;
    std::vector<GPSCoordinate> boundaryPoints;
    float area_m2;
    String dominantIndividual;
    std::vector<String> challenges;
    float territoryStability;
    time_t establishedDate;
    std::vector<String> resourcesProtected;
};

struct FeedingPatternAnalysis {
    String species;
    std::vector<uint8_t> feedingTimes; // Hours of day when feeding occurs
    std::vector<GPSCoordinate> feedingSites;
    std::map<String, float> foodSourcePreferences;
    float forageEfficiency;
    uint32_t caloriesPerDay;
    bool seasonalVariation;
    std::vector<String> competingSpecies;
};

struct ClimateImpactModel {
    String species;
    ClimateImpactLevel impactLevel;
    float temperatureThreshold_C;
    float precipitationThreshold_mm;
    std::vector<String> affectedBehaviors;
    std::vector<String> adaptationStrategies;
    float adaptationEffectiveness;
    time_t assessmentDate;
    bool requiresIntervention;
};

struct InterSpeciesInteraction {
    String primarySpecies;
    String interactingSpecies;
    String interactionType;  // "predation", "competition", "mutualism", etc.
    float interactionStrength; // 0.0 - 1.0
    GPSCoordinate interactionZone;
    float radius_m;
    uint32_t frequency_per_day;
    bool beneficial;
    float impactOnPrimarySpecies;
};

struct PopulationDynamicsModel {
    String species;
    uint32_t currentPopulation;
    float growthRate;        // Per year
    uint32_t carryingCapacity;
    float survivalRate;
    float reproductionRate;
    std::vector<String> limitingFactors;
    float geneticDiversity;
    bool viablePopulation;
    time_t projectionDate;
};

struct ConservationRecommendation {
    uint32_t recommendationId;
    String species;
    String interventionType;
    String description;
    AlertPriority urgency;
    float expectedEffectiveness;
    uint32_t estimatedCost;
    uint32_t timeframe_days;
    std::vector<String> stakeholders;
    bool requiresApproval;
    String evidenceBasis;
};

class EnhancedBehaviorPredictor {
public:
    EnhancedBehaviorPredictor();
    ~EnhancedBehaviorPredictor();
    
    // Initialization and configuration
    bool initialize();
    bool integrateWithPredictiveEngine(PredictiveAnalyticsEngine* engine);
    bool integrateWithGlobalNetwork(GlobalCoordinationManager* network);
    void setConfidenceThreshold(float threshold);
    
    // AI-powered behavior forecasting
    PredictionResult forecastBehavior(const String& species, uint32_t hoursAhead);
    std::vector<PredictionResult> forecastMultiSpecies(const std::vector<String>& species, uint32_t hoursAhead);
    float calculateForecastAccuracy(const String& species) const;
    void updateForecastModels(const BehaviorResult& actualResult);
    
    // Migration pattern prediction with seasonal adaptation
    MigrationData predictMigrationPattern(const String& species, const GPSCoordinate& currentLocation);
    MigrationData predictSeasonalMigration(const String& species, SeasonalPhase season);
    std::vector<GPSCoordinate> predictMigrationRoute(const String& species, const GPSCoordinate& start, const GPSCoordinate& destination);
    bool adaptMigrationPrediction(const String& species, const EnvironmentalData& conditions);
    
    // Breeding behavior detection and prediction
    BreedingBehaviorData predictBreedingBehavior(const String& species);
    bool detectBreedingActivity(const BehaviorResult& behavior);
    time_t predictNextBreedingCycle(const String& species);
    float assessBreedingHabitat(const GPSCoordinate& location, const String& species);
    std::vector<GPSCoordinate> identifyOptimalNestingSites(const String& species, float radius_km);
    
    // Territorial behavior mapping and conflict prediction
    TerritorialMapping mapTerritorialBehavior(const String& species, const GPSCoordinate& centerPoint);
    std::vector<TerritorialMapping> identifyTerritorialConflicts(float radius_km);
    float predictTerritorialConflict(const TerritorialMapping& territory1, const TerritorialMapping& territory2);
    bool resolveTerritoryClaim(uint32_t territoryId, const String& evidence);
    
    // Feeding pattern analysis and resource optimization
    FeedingPatternAnalysis analyzeFeedingPatterns(const String& species);
    std::vector<GPSCoordinate> optimizeFeedingStations(const String& species, uint8_t stationCount);
    float predictResourceAvailability(const GPSCoordinate& location, const String& resourceType);
    bool detectResourceCompetition(const String& species1, const String& species2);
    
    // Climate change impact modeling
    ClimateImpactModel assessClimateImpact(const String& species);
    std::vector<String> predictClimateAdaptations(const String& species, float temperatureChange_C);
    bool modelBehaviorChanges(const String& species, const ClimateImpactModel& impact);
    float assessSpeciesVulnerability(const String& species);
    
    // Population dynamics prediction with genetic algorithms
    PopulationDynamicsModel predictPopulationDynamics(const String& species, uint32_t yearsAhead);
    float calculateExtinctionRisk(const String& species);
    bool optimizeConservationStrategy(const String& species, const std::vector<String>& strategies);
    uint32_t predictOptimalPopulationSize(const String& species);
    
    // Behavioral anomaly detection for conservation alerts
    bool detectBehaviorAnomaly(const BehaviorResult& behavior, float sensitivityThreshold);
    ConservationAlert generateConservationAlert(const String& species, const String& anomaly);
    std::vector<ConservationAlert> scanForAnomalies(const std::vector<String>& species);
    bool validateAnomaly(const ConservationAlert& alert);
    
    // Inter-species interaction prediction and analysis
    std::vector<InterSpeciesInteraction> predictInterSpeciesInteractions(const String& species, float radius_km);
    bool analyzeEcosystemBalance(const std::vector<String>& species);
    float assessSpeciesImpact(const String& species, const String& ecosystem);
    std::vector<String> identifyKeystone Species(const std::vector<String>& species);
    
    // Conservation intervention recommendation system
    std::vector<ConservationRecommendation> generateRecommendations(const String& species);
    bool evaluateInterventionEffectiveness(uint32_t recommendationId, float actualOutcome);
    ConservationRecommendation optimizeIntervention(const ConservationRecommendation& original);
    bool prioritizeRecommendations(std::vector<ConservationRecommendation>& recommendations);
    
    // Seasonal adaptation algorithms
    void updateSeasonalModels(SeasonalPhase currentSeason);
    SeasonalBehaviorPattern getSeasonalPattern(const String& species, SeasonalPhase season);
    bool adaptToSeasonalChange(const String& species, SeasonalPhase fromSeason, SeasonalPhase toSeason);
    std::vector<String> predictSeasonalBehaviorChanges(const String& species);
    
    // Performance metrics and optimization
    struct PredictionMetrics {
        uint32_t totalPredictions;
        uint32_t accuratePredictions;
        float averageAccuracy;
        uint32_t anomaliesDetected;
        uint32_t recommendationsGenerated;
        uint32_t successfulInterventions;
        time_t lastUpdate;
    };
    
    PredictionMetrics getPredictionMetrics() const;
    bool exportPredictionData(const String& filename) const;
    void optimizePredictionAlgorithms();
    
    // Model training and updates
    bool trainBehaviorModel(const String& species, const std::vector<BehaviorResult>& trainingData);
    bool updateModelWeights(const String& species, float accuracy);
    void synchronizeWithGlobalModels();
    bool validateModelAccuracy(const String& species, float minimumAccuracy);

private:
    bool initialized_;
    float confidenceThreshold_;
    
    // External system integration
    PredictiveAnalyticsEngine* predictiveEngine_;
    GlobalCoordinationManager* globalNetwork_;
    
    // Behavior pattern storage
    std::map<String, std::vector<SeasonalBehaviorPattern>> seasonalPatterns_;
    std::map<String, BreedingBehaviorData> breedingData_;
    std::map<String, std::vector<TerritorialMapping>> territorialMaps_;
    std::map<String, FeedingPatternAnalysis> feedingPatterns_;
    std::map<String, ClimateImpactModel> climateImpacts_;
    std::map<String, PopulationDynamicsModel> populationModels_;
    std::vector<InterSpeciesInteraction> interSpeciesInteractions_;
    
    // Prediction metrics
    PredictionMetrics metrics_;
    std::map<String, float> speciesAccuracy_;
    
    // Model parameters
    std::map<String, std::vector<float>> modelWeights_;
    time_t lastModelUpdate_;
    
    // Private helper methods
    SeasonalPhase getCurrentSeason() const;
    float calculateSeasonalWeight(SeasonalPhase season) const;
    bool isBreedingSeason(const String& species) const;
    float calculateTerritorialOverlap(const TerritorialMapping& territory1, const TerritorialMapping& territory2) const;
    
    // AI model implementations
    float runNeuralNetworkPrediction(const String& species, const std::vector<float>& inputs) const;
    std::vector<float> runGeneticAlgorithm(const String& species, const std::vector<float>& parameters) const;
    float calculateBayesianConfidence(const std::vector<float>& evidence) const;
    
    // Environmental correlation
    float correlateWithWeather(const String& species, const EnvironmentalData& weather) const;
    float correlateWithHabitat(const String& species, const GPSCoordinate& location) const;
    
    // Data validation and cleaning
    bool validateBehaviorData(const BehaviorResult& behavior) const;
    void cleanOutlierData(std::vector<BehaviorResult>& data) const;
    bool detectDataCorruption(const std::vector<BehaviorResult>& data) const;
    
    // Conservation analysis
    float assessConservationUrgency(const String& species) const;
    std::vector<String> identifyThreats(const String& species) const;
    float calculateInterventionPriority(const ConservationRecommendation& recommendation) const;
};

#endif // ENHANCED_BEHAVIOR_PREDICTION_H