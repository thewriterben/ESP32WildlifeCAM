/**
 * Animal Behavior Pattern Recognition
 * Phase 2 Enhancement - Advanced behavior analysis using edge AI
 * 
 * Features:
 * - Real-time behavior classification
 * - Temporal pattern analysis
 * - Social interaction detection
 * - Anomaly detection in behavior patterns
 * - Predictive behavior modeling
 */

#include "../../include/network/mesh_protocol.h"
#include "../edge_processing/wildlife_detection.h"
#include <vector>
#include <map>
#include <deque>

#ifndef BEHAVIOR_ANALYSIS_H
#define BEHAVIOR_ANALYSIS_H

// Behavior classification categories
enum BehaviorCategory {
    BEHAVIOR_FEEDING = 0,
    BEHAVIOR_RESTING = 1,
    BEHAVIOR_MOVING = 2,
    BEHAVIOR_ALERT = 3,
    BEHAVIOR_GROOMING = 4,
    BEHAVIOR_SOCIAL = 5,
    BEHAVIOR_MATING = 6,
    BEHAVIOR_TERRITORIAL = 7,
    BEHAVIOR_HUNTING = 8,
    BEHAVIOR_FLEEING = 9,
    BEHAVIOR_UNKNOWN = 10
};

// Detailed behavior analysis result
struct BehaviorAnalysisResult {
    BehaviorCategory primaryBehavior;
    float confidence;
    String behaviorDescription;
    
    // Behavioral features
    float movementIntensity;     // 0.0 (stationary) to 1.0 (very active)
    float aggressionLevel;       // 0.0 (calm) to 1.0 (highly aggressive)
    float alertnessLevel;        // 0.0 (relaxed) to 1.0 (highly alert)
    float socialActivity;        // 0.0 (solitary) to 1.0 (highly social)
    
    // Temporal context
    uint32_t observationDuration_ms;
    uint32_t behaviorStartTime;
    uint32_t lastUpdate;
    bool behaviorChanged;
    
    // Spatial context
    float averagePosition_x, averagePosition_y;
    float movementRadius;        // Area of movement in pixels
    float directionality;        // 0.0 (random) to 1.0 (very directional)
    
    // Environmental correlation
    bool correlatedWithWeather;
    bool correlatedWithTimeOfDay;
    bool correlatedWithSeason;
    float environmentalInfluence; // How much environment affects this behavior
    
    BehaviorAnalysisResult() : primaryBehavior(BEHAVIOR_UNKNOWN), confidence(0.0),
                              movementIntensity(0.0), aggressionLevel(0.0),
                              alertnessLevel(0.0), socialActivity(0.0),
                              observationDuration_ms(0), behaviorStartTime(0),
                              lastUpdate(0), behaviorChanged(false),
                              averagePosition_x(0.0), averagePosition_y(0.0),
                              movementRadius(0.0), directionality(0.0),
                              correlatedWithWeather(false), correlatedWithTimeOfDay(false),
                              correlatedWithSeason(false), environmentalInfluence(0.0) {}
};

// Individual animal behavior tracking
struct AnimalBehaviorProfile {
    uint32_t animalId;
    String speciesName;
    std::deque<BehaviorAnalysisResult> behaviorHistory;
    
    // Behavioral patterns
    std::map<BehaviorCategory, float> behaviorFrequency;
    std::map<uint32_t, BehaviorCategory> timeOfDayPatterns;  // Minutes since midnight
    std::map<uint8_t, BehaviorCategory> seasonalPatterns;    // Season (0-3)
    
    // Anomaly detection
    float normalBehaviorBaseline;
    std::vector<uint32_t> anomalousTimestamps;
    uint32_t lastAnomalyDetection;
    
    // Social behavior
    std::vector<uint32_t> associatedAnimals;    // IDs of animals seen together
    float socialityScore;                       // 0.0 (solitary) to 1.0 (highly social)
    
    AnimalBehaviorProfile() : animalId(0), normalBehaviorBaseline(0.0),
                             lastAnomalyDetection(0), socialityScore(0.0) {}
};

// Group behavior analysis
struct GroupBehaviorResult {
    std::vector<uint32_t> groupMembers;
    BehaviorCategory groupBehavior;
    float groupCohesion;         // How well-coordinated the group is
    float groupSize;
    String interactionType;      // "cooperative", "competitive", "mating", "territorial"
    uint32_t groupFormationTime;
    uint32_t groupDissolutionTime;
    
    GroupBehaviorResult() : groupBehavior(BEHAVIOR_UNKNOWN), groupCohesion(0.0),
                           groupSize(0.0), groupFormationTime(0), groupDissolutionTime(0) {}
};

class BehaviorAnalysisEngine {
private:
    // Behavior recognition models
    struct BehaviorModel {
        BehaviorCategory category;
        std::vector<float> featureWeights;
        float threshold;
        float accuracy;
        uint32_t trainingSamples;
        
        BehaviorModel() : category(BEHAVIOR_UNKNOWN), threshold(0.5), 
                         accuracy(0.0), trainingSamples(0) {}
    };
    
    std::map<BehaviorCategory, BehaviorModel> behaviorModels_;
    std::map<uint32_t, AnimalBehaviorProfile> animalProfiles_;
    std::vector<GroupBehaviorResult> activeGroups_;
    
    // Configuration
    uint32_t maxHistorySize_;
    uint32_t anomalyDetectionWindow_ms_;
    float anomalyThreshold_;
    bool enableGroupAnalysis_;
    bool enablePredictiveModeling_;
    
    // Performance metrics
    uint32_t totalAnalyses_;
    float averageAnalysisTime_ms_;
    uint32_t anomaliesDetected_;
    uint32_t behaviorTransitions_;
    
    bool initialized_;

public:
    BehaviorAnalysisEngine();
    ~BehaviorAnalysisEngine();
    
    // Initialization and configuration
    bool init();
    void cleanup();
    void setMaxHistorySize(uint32_t size) { maxHistorySize_ = size; }
    void setAnomalyThreshold(float threshold) { anomalyThreshold_ = threshold; }
    void enableGroupAnalysis(bool enable) { enableGroupAnalysis_ = enable; }
    void enablePredictiveModeling(bool enable) { enablePredictiveModeling_ = enable; }
    
    // Core behavior analysis
    BehaviorAnalysisResult analyzeBehavior(const EnhancedSpeciesResult& detection,
                                          const std::vector<EnhancedSpeciesResult>& recentDetections);
    BehaviorAnalysisResult analyzeTemporalBehavior(uint32_t animalId,
                                                  const std::vector<EnhancedSpeciesResult>& history);
    std::vector<GroupBehaviorResult> analyzeGroupBehavior(const std::vector<EnhancedSpeciesResult>& detections);
    
    // Pattern recognition
    BehaviorCategory classifyBehavior(const std::vector<float>& features);
    float calculateBehaviorConfidence(BehaviorCategory behavior, const std::vector<float>& features);
    std::vector<float> extractBehaviorFeatures(const EnhancedSpeciesResult& detection,
                                              const std::vector<EnhancedSpeciesResult>& context);
    
    // Anomaly detection
    bool detectAnomalousBehavior(const BehaviorAnalysisResult& currentBehavior,
                                const AnimalBehaviorProfile& profile);
    float calculateAnomalyScore(const BehaviorAnalysisResult& behavior,
                               const AnimalBehaviorProfile& profile);
    void updateNormalBehaviorBaseline(AnimalBehaviorProfile& profile);
    
    // Temporal pattern analysis
    void analyzeTemporalPatterns(AnimalBehaviorProfile& profile);
    BehaviorCategory predictNextBehavior(const AnimalBehaviorProfile& profile);
    float calculateBehaviorTransitionProbability(BehaviorCategory from, BehaviorCategory to,
                                                const AnimalBehaviorProfile& profile);
    
    // Social behavior analysis
    void analyzeSocialInteractions(const std::vector<EnhancedSpeciesResult>& detections);
    float calculateSocialityScore(const AnimalBehaviorProfile& profile);
    std::vector<uint32_t> identifyAssociatedAnimals(uint32_t animalId,
                                                   const std::vector<EnhancedSpeciesResult>& detections);
    
    // Environmental correlation
    void correlateWithEnvironment(BehaviorAnalysisResult& result,
                                 float temperature, float humidity, uint8_t lightLevel);
    bool isBehaviorSeasonallyTypical(BehaviorCategory behavior, const String& species, uint8_t season);
    bool isBehaviorTimeTypical(BehaviorCategory behavior, const String& species, uint32_t timeOfDay);
    
    // Profile management
    void updateAnimalProfile(uint32_t animalId, const BehaviorAnalysisResult& newBehavior);
    AnimalBehaviorProfile getAnimalProfile(uint32_t animalId) const;
    std::vector<AnimalBehaviorProfile> getAllProfiles() const;
    void cleanupStaleProfiles(uint32_t maxAge_ms = 86400000);  // 24 hours
    
    // Model training and adaptation
    void trainBehaviorModel(BehaviorCategory category, const std::vector<std::vector<float>>& features,
                           const std::vector<bool>& labels);
    void adaptModelsToRegion(const String& region);
    void updateModelAccuracy(BehaviorCategory category, bool correct);
    
    // Statistics and reporting
    String getBehaviorReport(uint32_t animalId) const;
    String getOverallBehaviorStatistics() const;
    std::map<BehaviorCategory, uint32_t> getBehaviorFrequencyMap() const;
    float getAverageAnalysisTime() const { return averageAnalysisTime_ms_; }
    uint32_t getTotalAnomaliesDetected() const { return anomaliesDetected_; }

private:
    // Feature extraction helpers
    float calculateMovementIntensity(const EnhancedSpeciesResult& current,
                                   const std::vector<EnhancedSpeciesResult>& recent);
    float calculateAlertness(const EnhancedSpeciesResult& detection);
    float calculateDirectionality(const std::vector<EnhancedSpeciesResult>& trajectory);
    std::vector<float> extractPostureFeatures(const EnhancedSpeciesResult& detection);
    
    // Behavior classification algorithms
    float sigmoidActivation(float x);
    std::vector<float> normalizeFeatures(const std::vector<float>& features);
    BehaviorCategory applyBehaviorModel(const std::vector<float>& features, const BehaviorModel& model);
    
    // Group analysis helpers
    std::vector<std::vector<uint32_t>> identifyGroups(const std::vector<EnhancedSpeciesResult>& detections);
    float calculateGroupCohesion(const std::vector<EnhancedSpeciesResult>& groupMembers);
    String classifyGroupInteraction(const std::vector<EnhancedSpeciesResult>& groupMembers);
    
    // Temporal analysis helpers
    void updateBehaviorFrequency(AnimalBehaviorProfile& profile, BehaviorCategory behavior);
    void updateTimeOfDayPatterns(AnimalBehaviorProfile& profile, BehaviorCategory behavior, uint32_t timeOfDay);
    void updateSeasonalPatterns(AnimalBehaviorProfile& profile, BehaviorCategory behavior, uint8_t season);
    
    // Utility functions
    String behaviorCategoryToString(BehaviorCategory category);
    BehaviorCategory stringToBehaviorCategory(const String& behaviorStr);
    float calculateDistance(const EnhancedSpeciesResult& det1, const EnhancedSpeciesResult& det2);
    uint32_t getCurrentTimeOfDay();
    uint8_t getCurrentSeason();
    
    // Model persistence (for future implementation)
    bool saveModel(BehaviorCategory category, const String& filename);
    bool loadModel(BehaviorCategory category, const String& filename);
};

// Behavior analysis callbacks
typedef void (*BehaviorDetectedCallback)(uint32_t animalId, const BehaviorAnalysisResult& behavior);
typedef void (*AnomalyDetectedCallback)(uint32_t animalId, const BehaviorAnalysisResult& anomalous);
typedef void (*GroupFormationCallback)(const GroupBehaviorResult& group);
typedef void (*BehaviorTransitionCallback)(uint32_t animalId, BehaviorCategory from, BehaviorCategory to);

// Global behavior analysis engine
extern BehaviorAnalysisEngine behaviorAnalyzer;

// Callback registration functions
void setBehaviorDetectedCallback(BehaviorDetectedCallback callback);
void setAnomalyDetectedCallback(AnomalyDetectedCallback callback);
void setGroupFormationCallback(GroupFormationCallback callback);
void setBehaviorTransitionCallback(BehaviorTransitionCallback callback);

#endif // BEHAVIOR_ANALYSIS_H