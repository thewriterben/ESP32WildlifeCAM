/**
 * Animal Behavior Pattern Recognition Implementation
 * Phase 2 Enhancement - Advanced behavior analysis using edge AI
 */

#include "behavior_analysis.h"
#include <algorithm>
#include <cmath>

// Global behavior analysis engine
BehaviorAnalysisEngine behaviorAnalyzer;

// Callback function pointers
static BehaviorDetectedCallback g_behaviorDetectedCallback = nullptr;
static AnomalyDetectedCallback g_anomalyDetectedCallback = nullptr;
static GroupFormationCallback g_groupFormationCallback = nullptr;
static BehaviorTransitionCallback g_behaviorTransitionCallback = nullptr;

BehaviorAnalysisEngine::BehaviorAnalysisEngine() 
    : maxHistorySize_(50), anomalyDetectionWindow_ms_(300000), anomalyThreshold_(0.3),
      enableGroupAnalysis_(true), enablePredictiveModeling_(true),
      totalAnalyses_(0), averageAnalysisTime_ms_(0.0), anomaliesDetected_(0),
      behaviorTransitions_(0), initialized_(false) {
}

BehaviorAnalysisEngine::~BehaviorAnalysisEngine() {
    cleanup();
}

bool BehaviorAnalysisEngine::init() {
    if (initialized_) {
        return true;
    }
    
    // Initialize behavior models with default parameters
    for (int i = 0; i < 11; i++) {  // 11 behavior categories
        BehaviorCategory category = static_cast<BehaviorCategory>(i);
        BehaviorModel& model = behaviorModels_[category];
        model.category = category;
        model.threshold = 0.5;
        model.accuracy = 0.75;  // Default accuracy
        
        // Initialize feature weights (simplified)
        model.featureWeights = {1.0, 1.0, 1.0, 1.0, 1.0};  // 5 features
    }
    
    // Configure specific behavior models
    behaviorModels_[BEHAVIOR_FEEDING].featureWeights = {0.8, 0.2, 0.1, 0.9, 0.3};
    behaviorModels_[BEHAVIOR_RESTING].featureWeights = {0.1, 0.9, 0.8, 0.2, 0.1};
    behaviorModels_[BEHAVIOR_MOVING].featureWeights = {0.9, 0.1, 0.2, 0.3, 0.4};
    behaviorModels_[BEHAVIOR_ALERT].featureWeights = {0.3, 0.8, 0.9, 0.1, 0.2};
    behaviorModels_[BEHAVIOR_SOCIAL].featureWeights = {0.5, 0.3, 0.4, 0.1, 0.9};
    
    initialized_ = true;
    
    Serial.println("Behavior Analysis Engine initialized");
    Serial.println("Configuration:");
    Serial.println("  Max history size: " + String(maxHistorySize_));
    Serial.println("  Anomaly threshold: " + String(anomalyThreshold_));
    Serial.println("  Group analysis: " + String(enableGroupAnalysis_ ? "Enabled" : "Disabled"));
    
    return true;
}

void BehaviorAnalysisEngine::cleanup() {
    if (!initialized_) return;
    
    behaviorModels_.clear();
    animalProfiles_.clear();
    activeGroups_.clear();
    
    initialized_ = false;
}

BehaviorAnalysisResult BehaviorAnalysisEngine::analyzeBehavior(const EnhancedSpeciesResult& detection,
                                                              const std::vector<EnhancedSpeciesResult>& recentDetections) {
    if (!initialized_) {
        return BehaviorAnalysisResult();
    }
    
    uint32_t analysisStart = millis();
    BehaviorAnalysisResult result;
    
    // Extract behavioral features
    std::vector<float> features = extractBehaviorFeatures(detection, recentDetections);
    
    // Classify behavior
    result.primaryBehavior = classifyBehavior(features);
    result.confidence = calculateBehaviorConfidence(result.primaryBehavior, features);
    result.behaviorDescription = behaviorCategoryToString(result.primaryBehavior);
    
    // Calculate behavioral metrics
    result.movementIntensity = calculateMovementIntensity(detection, recentDetections);
    result.alertnessLevel = calculateAlertness(detection);
    result.directionality = calculateDirectionality(recentDetections);
    
    // Set temporal context
    result.lastUpdate = millis();
    result.behaviorStartTime = result.lastUpdate;
    result.observationDuration_ms = 0;
    
    // Set spatial context
    result.averagePosition_x = detection.boundingBox.x + detection.boundingBox.width / 2.0;
    result.averagePosition_y = detection.boundingBox.y + detection.boundingBox.height / 2.0;
    
    // Calculate movement radius from recent detections
    if (recentDetections.size() > 1) {
        float minX = result.averagePosition_x, maxX = result.averagePosition_x;
        float minY = result.averagePosition_y, maxY = result.averagePosition_y;
        
        for (const auto& det : recentDetections) {
            float centerX = det.boundingBox.x + det.boundingBox.width / 2.0;
            float centerY = det.boundingBox.y + det.boundingBox.height / 2.0;
            
            minX = std::min(minX, centerX);
            maxX = std::max(maxX, centerX);
            minY = std::min(minY, centerY);
            maxY = std::max(maxY, centerY);
        }
        
        result.movementRadius = sqrt((maxX - minX) * (maxX - minX) + (maxY - minY) * (maxY - minY));
    }
    
    // Update animal profile
    if (detection.trackingId != 0) {
        updateAnimalProfile(detection.trackingId, result);
        
        // Check for anomalous behavior
        if (animalProfiles_.find(detection.trackingId) != animalProfiles_.end()) {
            const AnimalBehaviorProfile& profile = animalProfiles_[detection.trackingId];
            if (detectAnomalousBehavior(result, profile)) {
                if (g_anomalyDetectedCallback) {
                    g_anomalyDetectedCallback(detection.trackingId, result);
                }
                anomaliesDetected_++;
            }
        }
    }
    
    // Update performance metrics
    uint32_t analysisTime = millis() - analysisStart;
    averageAnalysisTime_ms_ = (averageAnalysisTime_ms_ * totalAnalyses_ + analysisTime) / (totalAnalyses_ + 1);
    totalAnalyses_++;
    
    if (g_behaviorDetectedCallback) {
        g_behaviorDetectedCallback(detection.trackingId, result);
    }
    
    return result;
}

BehaviorCategory BehaviorAnalysisEngine::classifyBehavior(const std::vector<float>& features) {
    if (features.size() < 5) {
        return BEHAVIOR_UNKNOWN;
    }
    
    BehaviorCategory bestCategory = BEHAVIOR_UNKNOWN;
    float bestScore = 0.0;
    
    for (const auto& modelPair : behaviorModels_) {
        const BehaviorModel& model = modelPair.second;
        if (model.featureWeights.size() != features.size()) continue;
        
        // Calculate weighted sum
        float score = 0.0;
        for (size_t i = 0; i < features.size(); i++) {
            score += features[i] * model.featureWeights[i];
        }
        
        // Apply sigmoid activation
        score = sigmoidActivation(score);
        
        if (score > model.threshold && score > bestScore) {
            bestScore = score;
            bestCategory = model.category;
        }
    }
    
    return bestCategory;
}

float BehaviorAnalysisEngine::calculateBehaviorConfidence(BehaviorCategory behavior, 
                                                         const std::vector<float>& features) {
    auto modelIt = behaviorModels_.find(behavior);
    if (modelIt == behaviorModels_.end() || features.size() != modelIt->second.featureWeights.size()) {
        return 0.0;
    }
    
    const BehaviorModel& model = modelIt->second;
    
    // Calculate weighted sum
    float score = 0.0;
    for (size_t i = 0; i < features.size(); i++) {
        score += features[i] * model.featureWeights[i];
    }
    
    // Apply sigmoid and scale by model accuracy
    float confidence = sigmoidActivation(score) * model.accuracy;
    
    return std::min(confidence, 1.0f);
}

std::vector<float> BehaviorAnalysisEngine::extractBehaviorFeatures(const EnhancedSpeciesResult& detection,
                                                                   const std::vector<EnhancedSpeciesResult>& context) {
    std::vector<float> features;
    features.reserve(5);
    
    // Feature 1: Movement intensity
    features.push_back(calculateMovementIntensity(detection, context));
    
    // Feature 2: Alertness level
    features.push_back(calculateAlertness(detection));
    
    // Feature 3: Size relative to frame
    float sizeRatio = (detection.boundingBox.width * detection.boundingBox.height) / (320.0 * 240.0);
    features.push_back(std::min(sizeRatio, 1.0f));
    
    // Feature 4: Position in frame (center vs edge)
    float centerX = 160.0, centerY = 120.0;  // Assume 320x240 frame
    float detCenterX = detection.boundingBox.x + detection.boundingBox.width / 2.0;
    float detCenterY = detection.boundingBox.y + detection.boundingBox.height / 2.0;
    float distanceFromCenter = sqrt((detCenterX - centerX) * (detCenterX - centerX) + 
                                   (detCenterY - centerY) * (detCenterY - centerY));
    float normalizedDistance = distanceFromCenter / sqrt(centerX * centerX + centerY * centerY);
    features.push_back(1.0 - std::min(normalizedDistance, 1.0f));  // Higher value for center
    
    // Feature 5: Directionality
    features.push_back(calculateDirectionality(context));
    
    return features;
}

float BehaviorAnalysisEngine::calculateMovementIntensity(const EnhancedSpeciesResult& current,
                                                        const std::vector<EnhancedSpeciesResult>& recent) {
    if (recent.size() < 2) {
        return 0.0;  // Can't calculate movement with insufficient data
    }
    
    float totalMovement = 0.0;
    int validMovements = 0;
    
    for (size_t i = 1; i < recent.size(); i++) {
        float dx = (recent[i].boundingBox.x + recent[i].boundingBox.width / 2.0) - 
                   (recent[i-1].boundingBox.x + recent[i-1].boundingBox.width / 2.0);
        float dy = (recent[i].boundingBox.y + recent[i].boundingBox.height / 2.0) - 
                   (recent[i-1].boundingBox.y + recent[i-1].boundingBox.height / 2.0);
        
        float movement = sqrt(dx * dx + dy * dy);
        totalMovement += movement;
        validMovements++;
    }
    
    if (validMovements == 0) return 0.0;
    
    float averageMovement = totalMovement / validMovements;
    
    // Normalize to 0-1 range (assuming max movement of 50 pixels per frame)
    return std::min(averageMovement / 50.0f, 1.0f);
}

float BehaviorAnalysisEngine::calculateAlertness(const EnhancedSpeciesResult& detection) {
    // Simple alertness calculation based on confidence and size
    // More confident detections and larger animals (in frame) are considered more alert
    float alertness = detection.confidence;
    
    // Larger objects in frame suggest alertness (closer to camera, more visible)
    float sizeBonus = detection.boundingBox.area_ratio * 0.3;
    alertness += sizeBonus;
    
    return std::min(alertness, 1.0f);
}

float BehaviorAnalysisEngine::calculateDirectionality(const std::vector<EnhancedSpeciesResult>& trajectory) {
    if (trajectory.size() < 3) {
        return 0.0;  // Need at least 3 points to calculate directionality
    }
    
    // Calculate the angle variance of movement vectors
    std::vector<float> angles;
    
    for (size_t i = 2; i < trajectory.size(); i++) {
        float dx = (trajectory[i].boundingBox.x + trajectory[i].boundingBox.width / 2.0) - 
                   (trajectory[i-1].boundingBox.x + trajectory[i-1].boundingBox.width / 2.0);
        float dy = (trajectory[i].boundingBox.y + trajectory[i].boundingBox.height / 2.0) - 
                   (trajectory[i-1].boundingBox.y + trajectory[i-1].boundingBox.height / 2.0);
        
        if (dx != 0.0 || dy != 0.0) {
            float angle = atan2(dy, dx);
            angles.push_back(angle);
        }
    }
    
    if (angles.empty()) return 0.0;
    
    // Calculate variance of angles
    float meanAngle = 0.0;
    for (float angle : angles) {
        meanAngle += angle;
    }
    meanAngle /= angles.size();
    
    float variance = 0.0;
    for (float angle : angles) {
        float diff = angle - meanAngle;
        variance += diff * diff;
    }
    variance /= angles.size();
    
    // Convert variance to directionality (lower variance = higher directionality)
    // Normalize to 0-1 range
    float directionality = 1.0 / (1.0 + variance);
    
    return directionality;
}

bool BehaviorAnalysisEngine::detectAnomalousBehavior(const BehaviorAnalysisResult& currentBehavior,
                                                    const AnimalBehaviorProfile& profile) {
    if (profile.behaviorHistory.empty()) {
        return false;  // No baseline to compare against
    }
    
    // Calculate anomaly score
    float anomalyScore = calculateAnomalyScore(currentBehavior, profile);
    
    if (anomalyScore > anomalyThreshold_) {
        return true;
    }
    
    // Check for rapid behavior changes
    if (profile.behaviorHistory.size() > 1) {
        const BehaviorAnalysisResult& lastBehavior = profile.behaviorHistory.back();
        if (currentBehavior.primaryBehavior != lastBehavior.primaryBehavior &&
            (currentBehavior.lastUpdate - lastBehavior.lastUpdate) < 5000) {  // 5 seconds
            return true;  // Rapid behavior change
        }
    }
    
    return false;
}

float BehaviorAnalysisEngine::calculateAnomalyScore(const BehaviorAnalysisResult& behavior,
                                                   const AnimalBehaviorProfile& profile) {
    if (profile.behaviorHistory.empty()) {
        return 0.0;
    }
    
    // Calculate how different this behavior is from the animal's normal patterns
    float score = 0.0;
    
    // Check behavior frequency
    auto freqIt = profile.behaviorFrequency.find(behavior.primaryBehavior);
    if (freqIt != profile.behaviorFrequency.end()) {
        float normalFreq = freqIt->second;
        if (normalFreq < 0.1) {  // Very rare behavior
            score += 0.3;
        }
    } else {
        score += 0.5;  // Never seen this behavior before
    }
    
    // Check movement intensity deviation
    float avgMovement = 0.0;
    for (const auto& histBehavior : profile.behaviorHistory) {
        avgMovement += histBehavior.movementIntensity;
    }
    avgMovement /= profile.behaviorHistory.size();
    
    float movementDiff = abs(behavior.movementIntensity - avgMovement);
    score += movementDiff * 0.3;
    
    // Check alertness deviation
    float avgAlertness = 0.0;
    for (const auto& histBehavior : profile.behaviorHistory) {
        avgAlertness += histBehavior.alertnessLevel;
    }
    avgAlertness /= profile.behaviorHistory.size();
    
    float alertnessDiff = abs(behavior.alertnessLevel - avgAlertness);
    score += alertnessDiff * 0.2;
    
    return std::min(score, 1.0f);
}

void BehaviorAnalysisEngine::updateAnimalProfile(uint32_t animalId, const BehaviorAnalysisResult& newBehavior) {
    AnimalBehaviorProfile& profile = animalProfiles_[animalId];
    
    // Initialize profile if new
    if (profile.animalId == 0) {
        profile.animalId = animalId;
        profile.socialityScore = 0.0;
        profile.normalBehaviorBaseline = 0.0;
    }
    
    // Check for behavior transition
    if (!profile.behaviorHistory.empty()) {
        const BehaviorAnalysisResult& lastBehavior = profile.behaviorHistory.back();
        if (lastBehavior.primaryBehavior != newBehavior.primaryBehavior) {
            behaviorTransitions_++;
            if (g_behaviorTransitionCallback) {
                g_behaviorTransitionCallback(animalId, lastBehavior.primaryBehavior, newBehavior.primaryBehavior);
            }
        }
    }
    
    // Add to history
    profile.behaviorHistory.push_back(newBehavior);
    
    // Limit history size
    if (profile.behaviorHistory.size() > maxHistorySize_) {
        profile.behaviorHistory.pop_front();
    }
    
    // Update behavior frequency
    updateBehaviorFrequency(profile, newBehavior.primaryBehavior);
    
    // Update temporal patterns
    uint32_t timeOfDay = getCurrentTimeOfDay();
    updateTimeOfDayPatterns(profile, newBehavior.primaryBehavior, timeOfDay);
    
    uint8_t season = getCurrentSeason();
    updateSeasonalPatterns(profile, newBehavior.primaryBehavior, season);
    
    // Update baseline if needed
    if (profile.behaviorHistory.size() >= 10) {  // Require at least 10 observations
        updateNormalBehaviorBaseline(profile);
    }
}

void BehaviorAnalysisEngine::updateBehaviorFrequency(AnimalBehaviorProfile& profile, BehaviorCategory behavior) {
    profile.behaviorFrequency[behavior]++;
    
    // Normalize frequencies
    float totalCount = 0.0;
    for (const auto& freqPair : profile.behaviorFrequency) {
        totalCount += freqPair.second;
    }
    
    if (totalCount > 0) {
        for (auto& freqPair : profile.behaviorFrequency) {
            freqPair.second /= totalCount;
        }
    }
}

void BehaviorAnalysisEngine::updateTimeOfDayPatterns(AnimalBehaviorProfile& profile, 
                                                    BehaviorCategory behavior, uint32_t timeOfDay) {
    // Group time into 4-hour blocks for pattern analysis
    uint32_t timeBlock = (timeOfDay / 240) % 6;  // 6 blocks of 4 hours each
    profile.timeOfDayPatterns[timeBlock] = behavior;
}

void BehaviorAnalysisEngine::updateSeasonalPatterns(AnimalBehaviorProfile& profile, 
                                                   BehaviorCategory behavior, uint8_t season) {
    profile.seasonalPatterns[season] = behavior;
}

void BehaviorAnalysisEngine::updateNormalBehaviorBaseline(AnimalBehaviorProfile& profile) {
    if (profile.behaviorHistory.empty()) return;
    
    // Calculate average behavior metrics
    float avgMovement = 0.0, avgAlertness = 0.0, avgSocial = 0.0;
    
    for (const auto& behavior : profile.behaviorHistory) {
        avgMovement += behavior.movementIntensity;
        avgAlertness += behavior.alertnessLevel;
        avgSocial += behavior.socialActivity;
    }
    
    size_t historySize = profile.behaviorHistory.size();
    avgMovement /= historySize;
    avgAlertness /= historySize;
    avgSocial /= historySize;
    
    // Combine into baseline score
    profile.normalBehaviorBaseline = (avgMovement + avgAlertness + avgSocial) / 3.0;
}

float BehaviorAnalysisEngine::sigmoidActivation(float x) {
    return 1.0 / (1.0 + exp(-x));
}

uint32_t BehaviorAnalysisEngine::getCurrentTimeOfDay() {
    // Simplified: return minutes since midnight
    return (millis() / 60000) % 1440;
}

uint8_t BehaviorAnalysisEngine::getCurrentSeason() {
    // Simplified: return season based on day of year approximation
    uint32_t dayOfYear = (millis() / 86400000) % 365;
    return (dayOfYear / 91) % 4;  // Approximate 3-month seasons
}

String BehaviorAnalysisEngine::behaviorCategoryToString(BehaviorCategory category) {
    switch (category) {
        case BEHAVIOR_FEEDING: return "feeding";
        case BEHAVIOR_RESTING: return "resting";
        case BEHAVIOR_MOVING: return "moving";
        case BEHAVIOR_ALERT: return "alert";
        case BEHAVIOR_GROOMING: return "grooming";
        case BEHAVIOR_SOCIAL: return "social";
        case BEHAVIOR_MATING: return "mating";
        case BEHAVIOR_TERRITORIAL: return "territorial";
        case BEHAVIOR_HUNTING: return "hunting";
        case BEHAVIOR_FLEEING: return "fleeing";
        default: return "unknown";
    }
}

String BehaviorAnalysisEngine::getBehaviorReport(uint32_t animalId) const {
    auto profileIt = animalProfiles_.find(animalId);
    if (profileIt == animalProfiles_.end()) {
        return "No profile found for animal " + String(animalId);
    }
    
    const AnimalBehaviorProfile& profile = profileIt->second;
    
    String report = "Behavior Report for Animal " + String(animalId) + ":\n";
    report += "Species: " + profile.speciesName + "\n";
    report += "Observations: " + String(profile.behaviorHistory.size()) + "\n";
    report += "Sociality Score: " + String(profile.socialityScore, 2) + "\n";
    report += "Anomalies Detected: " + String(profile.anomalousTimestamps.size()) + "\n";
    
    report += "Behavior Frequencies:\n";
    for (const auto& freqPair : profile.behaviorFrequency) {
        report += "  " + behaviorCategoryToString(freqPair.first) + ": " + 
                 String(freqPair.second * 100.0, 1) + "%\n";
    }
    
    return report;
}

String BehaviorAnalysisEngine::getOverallBehaviorStatistics() const {
    String report = "Overall Behavior Analysis Statistics:\n";
    report += "Total Analyses: " + String(totalAnalyses_) + "\n";
    report += "Average Analysis Time: " + String(averageAnalysisTime_ms_, 2) + " ms\n";
    report += "Anomalies Detected: " + String(anomaliesDetected_) + "\n";
    report += "Behavior Transitions: " + String(behaviorTransitions_) + "\n";
    report += "Tracked Animals: " + String(animalProfiles_.size()) + "\n";
    report += "Active Groups: " + String(activeGroups_.size()) + "\n";
    
    return report;
}

// Callback registration functions
void setBehaviorDetectedCallback(BehaviorDetectedCallback callback) {
    g_behaviorDetectedCallback = callback;
}

void setAnomalyDetectedCallback(AnomalyDetectedCallback callback) {
    g_anomalyDetectedCallback = callback;
}

void setGroupFormationCallback(GroupFormationCallback callback) {
    g_groupFormationCallback = callback;
}

void setBehaviorTransitionCallback(BehaviorTransitionCallback callback) {
    g_behaviorTransitionCallback = callback;
}