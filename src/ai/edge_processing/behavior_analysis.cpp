/**
 * @file behavior_analysis.cpp
 * @brief Animal Behavior Pattern Recognition for Phase 2
 * 
 * This implementation provides advanced behavior analysis capabilities
 * including activity pattern recognition, social behavior detection,
 * and behavioral state classification using temporal sequences.
 * 
 * Phase 2 Enhancement - March 31, 2026 Target
 */

#include "../../../firmware/src/ai/tensorflow_lite_implementation.h"
#include "../../../include/network/mesh_protocol.h"
#include "wildlife_detection.cpp" // For WildlifeDetectionResult
#include <ArduinoJson.h>
#include <vector>
#include <map>

// ===========================
// BEHAVIOR CLASSIFICATION
// ===========================

enum BehaviorType {
    BEHAVIOR_UNKNOWN = 0,
    BEHAVIOR_FEEDING = 1,
    BEHAVIOR_DRINKING = 2,
    BEHAVIOR_RESTING = 3,
    BEHAVIOR_SLEEPING = 4,
    BEHAVIOR_GROOMING = 5,
    BEHAVIOR_WALKING = 6,
    BEHAVIOR_RUNNING = 7,
    BEHAVIOR_HUNTING = 8,
    BEHAVIOR_PLAYING = 9,
    BEHAVIOR_MATING = 10,
    BEHAVIOR_TERRITORIAL = 11,
    BEHAVIOR_ALERT = 12,
    BEHAVIOR_FLEEING = 13,
    BEHAVIOR_AGGRESSIVE = 14,
    BEHAVIOR_SOCIAL_INTERACTION = 15,
    BEHAVIOR_PARENTAL_CARE = 16,
    BEHAVIOR_FORAGING = 17,
    BEHAVIOR_MIGRATION = 18,
    BEHAVIOR_NESTING = 19,
    BEHAVIOR_TERRITORIAL_MARKING = 20
};

enum ActivityLevel {
    ACTIVITY_INACTIVE = 0,
    ACTIVITY_LOW = 1,
    ACTIVITY_MODERATE = 2,
    ACTIVITY_HIGH = 3,
    ACTIVITY_VERY_HIGH = 4
};

enum SocialContext {
    SOCIAL_SOLITARY = 0,
    SOCIAL_PAIR = 1,
    SOCIAL_SMALL_GROUP = 2,
    SOCIAL_LARGE_GROUP = 3,
    SOCIAL_FAMILY_UNIT = 4,
    SOCIAL_MIXED_SPECIES = 5
};

struct BehaviorSequence {
    uint32_t timestamp;
    BehaviorType behavior;
    float confidence;
    ActivityLevel activity;
    SocialContext social;
    uint16_t duration;        // Duration in seconds
    uint8_t objectCount;      // Number of animals
    float movementVector[2];  // X, Y movement direction
    float movementSpeed;      // Relative movement speed
};

struct BehaviorAnalysisResult {
    BehaviorType primaryBehavior;
    float primaryConfidence;
    BehaviorType secondaryBehavior;
    float secondaryConfidence;
    ActivityLevel activityLevel;
    SocialContext socialContext;
    uint32_t sequenceDuration;
    uint8_t animalCount;
    float behaviorStability;   // How consistent behavior is over time
    uint32_t timestamp;
    bool isValid;
};

struct BehaviorMetrics {
    uint32_t totalAnalyses;
    uint32_t validBehaviors;
    uint32_t sequencesAnalyzed;
    float averageConfidence;
    uint32_t longestSequence;
    std::map<BehaviorType, uint32_t> behaviorCounts;
    uint32_t lastAnalysisTime;
};

// ===========================
// BEHAVIOR ANALYSIS ENGINE
// ===========================

class BehaviorAnalysisEngine {
private:
    // Detection sequence storage
    std::vector<WildlifeDetectionResult> detectionHistory_;
    std::vector<BehaviorSequence> behaviorHistory_;
    
    // Analysis parameters
    uint32_t maxHistorySize_;
    uint32_t minSequenceLength_;
    uint32_t maxSequenceGap_;
    float movementThreshold_;
    float stabilityThreshold_;
    
    // Performance tracking
    BehaviorMetrics metrics_;
    uint32_t lastOptimization_;
    
    // Behavior models and analysis
    bool useMotionAnalysis_;
    bool useTemporalAnalysis_;
    bool useSocialAnalysis_;
    
    // Environmental context
    float environmentalBias_;
    uint32_t timeOfDay_;
    float weatherFactor_;
    
public:
    BehaviorAnalysisEngine() : 
        maxHistorySize_(50), minSequenceLength_(3), maxSequenceGap_(30000),
        movementThreshold_(0.1), stabilityThreshold_(0.7), lastOptimization_(0),
        useMotionAnalysis_(true), useTemporalAnalysis_(true), useSocialAnalysis_(true),
        environmentalBias_(1.0), timeOfDay_(12), weatherFactor_(1.0) {
        
        // Initialize metrics
        memset(&metrics_, 0, sizeof(metrics_));
        
        // Reserve space for histories
        detectionHistory_.reserve(maxHistorySize_);
        behaviorHistory_.reserve(maxHistorySize_);
    }
    
    // ===========================
    // INITIALIZATION
    // ===========================
    
    /**
     * Initialize behavior analysis system
     */
    bool initialize() {
        Serial.println("Initializing behavior analysis engine");
        
        // Load behavior analysis models if available
        // For now, use rule-based analysis
        
        Serial.println("Behavior analysis engine initialized");
        return true;
    }
    
    /**
     * Cleanup behavior analysis
     */
    void cleanup() {
        detectionHistory_.clear();
        behaviorHistory_.clear();
    }
    
    // ===========================
    // MAIN ANALYSIS PIPELINE
    // ===========================
    
    /**
     * Analyze behavior from detection result
     */
    BehaviorAnalysisResult analyzeBehavior(const WildlifeDetectionResult& detection) {
        BehaviorAnalysisResult result = {};
        result.timestamp = millis();
        result.isValid = false;
        
        if (!detection.isValid) {
            return result;
        }
        
        // Add detection to history
        addDetectionToHistory(detection);
        
        // Check if we have enough data for analysis
        if (detectionHistory_.size() < minSequenceLength_) {
            return result; // Not enough data yet
        }
        
        // Extract behavior sequence from recent detections
        BehaviorSequence sequence = extractBehaviorSequence();
        
        // Analyze the sequence for behavior patterns
        result = analyzeBehaviorSequence(sequence);
        
        // Apply temporal analysis
        if (useTemporalAnalysis_) {
            result = applyTemporalAnalysis(result);
        }
        
        // Apply social context analysis
        if (useSocialAnalysis_) {
            result = applySocialAnalysis(result);
        }
        
        // Update behavior history
        if (result.isValid) {
            addBehaviorToHistory(sequence);
        }
        
        // Update metrics
        updateMetrics(result);
        
        return result;
    }
    
    /**
     * Analyze behavior from multiple detections
     */
    BehaviorAnalysisResult analyzeBehaviorBatch(const std::vector<WildlifeDetectionResult>& detections) {
        BehaviorAnalysisResult combinedResult = {};
        combinedResult.timestamp = millis();
        combinedResult.isValid = false;
        
        if (detections.empty()) return combinedResult;
        
        // Process each detection
        for (const auto& detection : detections) {
            addDetectionToHistory(detection);
        }
        
        // Analyze the complete sequence
        if (detectionHistory_.size() >= minSequenceLength_) {
            BehaviorSequence sequence = extractBehaviorSequence();
            combinedResult = analyzeBehaviorSequence(sequence);
            
            if (combinedResult.isValid) {
                addBehaviorToHistory(sequence);
            }
        }
        
        updateMetrics(combinedResult);
        return combinedResult;
    }
    
    // ===========================
    // SEQUENCE EXTRACTION
    // ===========================
    
    /**
     * Extract behavior sequence from detection history
     */
    BehaviorSequence extractBehaviorSequence() {
        BehaviorSequence sequence = {};
        sequence.timestamp = millis();
        
        if (detectionHistory_.size() < minSequenceLength_) {
            return sequence;
        }
        
        // Calculate movement patterns
        if (useMotionAnalysis_) {
            calculateMovementPatterns(sequence);
        }
        
        // Determine activity level
        sequence.activity = calculateActivityLevel();
        
        // Count objects for social context
        sequence.objectCount = calculateObjectCount();
        sequence.social = determineSocialContext(sequence.objectCount);
        
        // Calculate sequence duration
        uint32_t startTime = detectionHistory_[detectionHistory_.size() - minSequenceLength_].timestamp;
        uint32_t endTime = detectionHistory_.back().timestamp;
        sequence.duration = (endTime - startTime) / 1000; // Convert to seconds
        
        return sequence;
    }
    
    /**
     * Calculate movement patterns from detection history
     */
    void calculateMovementPatterns(BehaviorSequence& sequence) {
        if (detectionHistory_.size() < 2) {
            sequence.movementVector[0] = 0.0;
            sequence.movementVector[1] = 0.0;
            sequence.movementSpeed = 0.0;
            return;
        }
        
        float totalDeltaX = 0.0;
        float totalDeltaY = 0.0;
        float totalDistance = 0.0;
        uint32_t totalTime = 0;
        
        for (size_t i = 1; i < detectionHistory_.size(); i++) {
            const auto& prev = detectionHistory_[i-1];
            const auto& curr = detectionHistory_[i];
            
            // Calculate center points of bounding boxes
            float prevX = prev.boundingBox[0] + prev.boundingBox[2] / 2.0;
            float prevY = prev.boundingBox[1] + prev.boundingBox[3] / 2.0;
            float currX = curr.boundingBox[0] + curr.boundingBox[2] / 2.0;
            float currY = curr.boundingBox[1] + curr.boundingBox[3] / 2.0;
            
            float deltaX = currX - prevX;
            float deltaY = currY - prevY;
            float distance = sqrt(deltaX * deltaX + deltaY * deltaY);
            uint32_t timeDiff = curr.timestamp - prev.timestamp;
            
            totalDeltaX += deltaX;
            totalDeltaY += deltaY;
            totalDistance += distance;
            totalTime += timeDiff;
        }
        
        // Normalize movement vector
        float vectorLength = sqrt(totalDeltaX * totalDeltaX + totalDeltaY * totalDeltaY);
        if (vectorLength > 0) {
            sequence.movementVector[0] = totalDeltaX / vectorLength;
            sequence.movementVector[1] = totalDeltaY / vectorLength;
        } else {
            sequence.movementVector[0] = 0.0;
            sequence.movementVector[1] = 0.0;
        }
        
        // Calculate movement speed (pixels per second)
        sequence.movementSpeed = totalTime > 0 ? (totalDistance * 1000.0) / totalTime : 0.0;
    }
    
    /**
     * Calculate activity level from recent detections
     */
    ActivityLevel calculateActivityLevel() {
        if (detectionHistory_.size() < 2) {
            return ACTIVITY_INACTIVE;
        }
        
        float totalMovement = 0.0;
        int movementCount = 0;
        
        for (size_t i = 1; i < detectionHistory_.size(); i++) {
            const auto& prev = detectionHistory_[i-1];
            const auto& curr = detectionHistory_[i];
            
            // Calculate movement between consecutive detections
            float prevX = prev.boundingBox[0] + prev.boundingBox[2] / 2.0;
            float prevY = prev.boundingBox[1] + prev.boundingBox[3] / 2.0;
            float currX = curr.boundingBox[0] + curr.boundingBox[2] / 2.0;
            float currY = curr.boundingBox[1] + curr.boundingBox[3] / 2.0;
            
            float movement = sqrt((currX - prevX) * (currX - prevX) + 
                                (currY - prevY) * (currY - prevY));
            
            totalMovement += movement;
            movementCount++;
        }
        
        float averageMovement = movementCount > 0 ? totalMovement / movementCount : 0.0;
        
        // Classify activity level based on movement
        if (averageMovement < 5.0) {
            return ACTIVITY_INACTIVE;
        } else if (averageMovement < 15.0) {
            return ACTIVITY_LOW;
        } else if (averageMovement < 30.0) {
            return ACTIVITY_MODERATE;
        } else if (averageMovement < 50.0) {
            return ACTIVITY_HIGH;
        } else {
            return ACTIVITY_VERY_HIGH;
        }
    }
    
    /**
     * Calculate average object count
     */
    uint8_t calculateObjectCount() {
        if (detectionHistory_.empty()) return 0;
        
        uint32_t totalObjects = 0;
        for (const auto& detection : detectionHistory_) {
            totalObjects += detection.objectCount;
        }
        
        return totalObjects / detectionHistory_.size();
    }
    
    // ===========================
    // BEHAVIOR CLASSIFICATION
    // ===========================
    
    /**
     * Analyze behavior sequence to determine behavior type
     */
    BehaviorAnalysisResult analyzeBehaviorSequence(const BehaviorSequence& sequence) {
        BehaviorAnalysisResult result = {};
        result.timestamp = sequence.timestamp;
        result.activityLevel = sequence.activity;
        result.socialContext = sequence.social;
        result.animalCount = sequence.objectCount;
        result.sequenceDuration = sequence.duration;
        
        // Rule-based behavior classification
        BehaviorType behavior = classifyBehaviorFromMovement(sequence);
        float confidence = calculateBehaviorConfidence(behavior, sequence);
        
        // Apply environmental context
        confidence *= environmentalBias_ * weatherFactor_;
        
        // Validate behavior classification
        if (confidence >= 0.6) {
            result.primaryBehavior = behavior;
            result.primaryConfidence = confidence;
            result.isValid = true;
            
            // Find secondary behavior
            BehaviorType secondaryBehavior = findSecondaryBehavior(sequence, behavior);
            result.secondaryBehavior = secondaryBehavior;
            result.secondaryConfidence = calculateBehaviorConfidence(secondaryBehavior, sequence) * 0.8;
        }
        
        // Calculate behavior stability
        result.behaviorStability = calculateBehaviorStability();
        
        return result;
    }
    
    /**
     * Classify behavior based on movement patterns
     */
    BehaviorType classifyBehaviorFromMovement(const BehaviorSequence& sequence) {
        // Movement-based classification rules
        if (sequence.movementSpeed < 2.0) {
            if (sequence.activity == ACTIVITY_INACTIVE) {
                return BEHAVIOR_RESTING;
            } else {
                return BEHAVIOR_FEEDING; // Low movement but some activity
            }
        } else if (sequence.movementSpeed < 10.0) {
            if (sequence.social == SOCIAL_PAIR || sequence.social == SOCIAL_SMALL_GROUP) {
                return BEHAVIOR_SOCIAL_INTERACTION;
            } else {
                return BEHAVIOR_FORAGING;
            }
        } else if (sequence.movementSpeed < 25.0) {
            return BEHAVIOR_WALKING;
        } else if (sequence.movementSpeed < 50.0) {
            if (sequence.duration > 30) {
                return BEHAVIOR_MIGRATION; // Sustained fast movement
            } else {
                return BEHAVIOR_ALERT; // Quick movement
            }
        } else {
            return BEHAVIOR_RUNNING; // Very fast movement
        }
    }
    
    /**
     * Calculate confidence for behavior classification
     */
    float calculateBehaviorConfidence(BehaviorType behavior, const BehaviorSequence& sequence) {
        float confidence = 0.5; // Base confidence
        
        // Movement consistency bonus
        if (sequence.movementSpeed >= 0) {
            confidence += 0.2;
        }
        
        // Duration bonus
        if (sequence.duration > 10) {
            confidence += 0.1;
        }
        
        // Activity level consistency
        confidence += calculateActivityConsistency(behavior, sequence.activity) * 0.2;
        
        // Social context consistency
        confidence += calculateSocialConsistency(behavior, sequence.social) * 0.1;
        
        return std::max(0.0f, std::min(1.0f, confidence));
    }
    
    /**
     * Find secondary behavior possibility
     */
    BehaviorType findSecondaryBehavior(const BehaviorSequence& sequence, BehaviorType primary) {
        // Simple secondary behavior logic
        switch (primary) {
            case BEHAVIOR_FEEDING:
                return BEHAVIOR_ALERT; // Animals are often alert while feeding
                
            case BEHAVIOR_WALKING:
                return BEHAVIOR_FORAGING; // Walking often involves foraging
                
            case BEHAVIOR_RESTING:
                return BEHAVIOR_GROOMING; // Often groom while resting
                
            case BEHAVIOR_RUNNING:
                return BEHAVIOR_FLEEING; // Running could be fleeing
                
            default:
                return BEHAVIOR_UNKNOWN;
        }
    }
    
    // ===========================
    // TEMPORAL ANALYSIS
    // ===========================
    
    /**
     * Apply temporal analysis to behavior result
     */
    BehaviorAnalysisResult applyTemporalAnalysis(const BehaviorAnalysisResult& result) {
        if (behaviorHistory_.empty()) {
            return result;
        }
        
        BehaviorAnalysisResult enhanced = result;
        
        // Check for behavior transitions
        BehaviorType previousBehavior = behaviorHistory_.back().behavior;
        if (isValidBehaviorTransition(previousBehavior, result.primaryBehavior)) {
            enhanced.primaryConfidence += 0.1; // Boost confidence for valid transitions
        } else {
            enhanced.primaryConfidence -= 0.05; // Reduce confidence for unlikely transitions
        }
        
        // Calculate temporal stability
        enhanced.behaviorStability = calculateTemporalStability(result.primaryBehavior);
        
        return enhanced;
    }
    
    /**
     * Check if behavior transition is valid/likely
     */
    bool isValidBehaviorTransition(BehaviorType from, BehaviorType to) {
        // Define valid behavior transitions
        static const std::map<BehaviorType, std::vector<BehaviorType>> validTransitions = {
            {BEHAVIOR_RESTING, {BEHAVIOR_GROOMING, BEHAVIOR_ALERT, BEHAVIOR_WALKING}},
            {BEHAVIOR_WALKING, {BEHAVIOR_FORAGING, BEHAVIOR_RUNNING, BEHAVIOR_RESTING}},
            {BEHAVIOR_FEEDING, {BEHAVIOR_ALERT, BEHAVIOR_WALKING, BEHAVIOR_DRINKING}},
            {BEHAVIOR_ALERT, {BEHAVIOR_RUNNING, BEHAVIOR_FLEEING, BEHAVIOR_RESTING}},
            {BEHAVIOR_RUNNING, {BEHAVIOR_WALKING, BEHAVIOR_RESTING, BEHAVIOR_ALERT}}
        };
        
        auto it = validTransitions.find(from);
        if (it != validTransitions.end()) {
            return std::find(it->second.begin(), it->second.end(), to) != it->second.end();
        }
        
        return true; // Allow transition if no rules defined
    }
    
    /**
     * Calculate temporal stability of behavior
     */
    float calculateTemporalStability(BehaviorType behavior) {
        if (behaviorHistory_.size() < 3) {
            return 0.5; // Neutral stability with insufficient data
        }
        
        int consistentCount = 0;
        int recentBehaviors = std::min(5, (int)behaviorHistory_.size());
        
        for (int i = behaviorHistory_.size() - recentBehaviors; i < behaviorHistory_.size(); i++) {
            if (behaviorHistory_[i].behavior == behavior) {
                consistentCount++;
            }
        }
        
        return (float)consistentCount / recentBehaviors;
    }
    
    // ===========================
    // SOCIAL ANALYSIS
    // ===========================
    
    /**
     * Apply social context analysis
     */
    BehaviorAnalysisResult applySocialAnalysis(const BehaviorAnalysisResult& result) {
        BehaviorAnalysisResult enhanced = result;
        
        // Adjust behavior probabilities based on social context
        if (result.socialContext == SOCIAL_PAIR || result.socialContext == SOCIAL_SMALL_GROUP) {
            // Social behaviors more likely in groups
            if (result.primaryBehavior == BEHAVIOR_SOCIAL_INTERACTION ||
                result.primaryBehavior == BEHAVIOR_PLAYING ||
                result.primaryBehavior == BEHAVIOR_MATING) {
                enhanced.primaryConfidence += 0.15;
            }
        } else if (result.socialContext == SOCIAL_SOLITARY) {
            // Individual behaviors more likely when alone
            if (result.primaryBehavior == BEHAVIOR_GROOMING ||
                result.primaryBehavior == BEHAVIOR_RESTING ||
                result.primaryBehavior == BEHAVIOR_HUNTING) {
                enhanced.primaryConfidence += 0.1;
            }
        }
        
        return enhanced;
    }
    
    /**
     * Determine social context from object count and positions
     */
    SocialContext determineSocialContext(uint8_t objectCount) {
        if (objectCount == 0) {
            return SOCIAL_SOLITARY;
        } else if (objectCount == 1) {
            return SOCIAL_SOLITARY;
        } else if (objectCount == 2) {
            return SOCIAL_PAIR;
        } else if (objectCount <= 5) {
            return SOCIAL_SMALL_GROUP;
        } else {
            return SOCIAL_LARGE_GROUP;
        }
    }
    
    // ===========================
    // HELPER FUNCTIONS
    // ===========================
    
    void addDetectionToHistory(const WildlifeDetectionResult& detection) {
        detectionHistory_.push_back(detection);
        
        // Maintain history size
        if (detectionHistory_.size() > maxHistorySize_) {
            detectionHistory_.erase(detectionHistory_.begin());
        }
    }
    
    void addBehaviorToHistory(const BehaviorSequence& behavior) {
        behaviorHistory_.push_back(behavior);
        
        // Maintain history size
        if (behaviorHistory_.size() > maxHistorySize_) {
            behaviorHistory_.erase(behaviorHistory_.begin());
        }
    }
    
    float calculateActivityConsistency(BehaviorType behavior, ActivityLevel activity) {
        // Define expected activity levels for behaviors
        static const std::map<BehaviorType, ActivityLevel> expectedActivity = {
            {BEHAVIOR_RESTING, ACTIVITY_INACTIVE},
            {BEHAVIOR_FEEDING, ACTIVITY_LOW},
            {BEHAVIOR_WALKING, ACTIVITY_MODERATE},
            {BEHAVIOR_RUNNING, ACTIVITY_HIGH},
            {BEHAVIOR_ALERT, ACTIVITY_MODERATE}
        };
        
        auto it = expectedActivity.find(behavior);
        if (it != expectedActivity.end()) {
            int diff = abs(it->second - activity);
            return std::max(0.0f, 1.0f - diff * 0.25f);
        }
        
        return 0.5; // Neutral if no expectation defined
    }
    
    float calculateSocialConsistency(BehaviorType behavior, SocialContext social) {
        // Social behaviors more likely in groups
        if ((behavior == BEHAVIOR_SOCIAL_INTERACTION || behavior == BEHAVIOR_PLAYING) &&
            (social == SOCIAL_PAIR || social == SOCIAL_SMALL_GROUP)) {
            return 1.0;
        }
        
        // Individual behaviors more likely when alone
        if ((behavior == BEHAVIOR_GROOMING || behavior == BEHAVIOR_RESTING) &&
            social == SOCIAL_SOLITARY) {
            return 1.0;
        }
        
        return 0.5; // Neutral consistency
    }
    
    float calculateBehaviorStability() {
        if (behaviorHistory_.size() < 3) return 0.5;
        
        std::map<BehaviorType, int> behaviorCounts;
        for (const auto& behavior : behaviorHistory_) {
            behaviorCounts[behavior.behavior]++;
        }
        
        // Find most common behavior
        int maxCount = 0;
        for (const auto& pair : behaviorCounts) {
            maxCount = std::max(maxCount, pair.second);
        }
        
        return (float)maxCount / behaviorHistory_.size();
    }
    
    void updateMetrics(const BehaviorAnalysisResult& result) {
        metrics_.totalAnalyses++;
        metrics_.lastAnalysisTime = result.timestamp;
        
        if (result.isValid) {
            metrics_.validBehaviors++;
            metrics_.behaviorCounts[result.primaryBehavior]++;
            
            // Update average confidence
            metrics_.averageConfidence = 
                (metrics_.averageConfidence * (metrics_.validBehaviors - 1) + result.primaryConfidence) / 
                metrics_.validBehaviors;
        }
        
        if (result.sequenceDuration > metrics_.longestSequence) {
            metrics_.longestSequence = result.sequenceDuration;
        }
    }
    
public:
    // ===========================
    // PUBLIC INTERFACE
    // ===========================
    
    void updateEnvironmentalContext(uint32_t timeOfDay, float weatherFactor) {
        timeOfDay_ = timeOfDay;
        weatherFactor_ = weatherFactor;
        
        // Calculate environmental bias based on time of day
        if (timeOfDay >= 6 && timeOfDay <= 18) {
            environmentalBias_ = 1.2; // Day time - more activity
        } else if (timeOfDay >= 19 && timeOfDay <= 22) {
            environmentalBias_ = 1.0; // Dusk - normal activity
        } else {
            environmentalBias_ = 0.8; // Night - less activity
        }
    }
    
    void setAnalysisParameters(uint32_t maxHistory, uint32_t minSequence, 
                             uint32_t maxGap, float movementThresh) {
        maxHistorySize_ = maxHistory;
        minSequenceLength_ = minSequence;
        maxSequenceGap_ = maxGap;
        movementThreshold_ = movementThresh;
    }
    
    BehaviorMetrics getMetrics() const {
        return metrics_;
    }
    
    void resetMetrics() {
        memset(&metrics_, 0, sizeof(metrics_));
        metrics_.behaviorCounts.clear();
    }
    
    std::vector<BehaviorSequence> getRecentBehaviors(uint32_t count) const {
        uint32_t startIndex = behaviorHistory_.size() > count ? 
                             behaviorHistory_.size() - count : 0;
        return std::vector<BehaviorSequence>(
            behaviorHistory_.begin() + startIndex, behaviorHistory_.end());
    }
};

// ===========================
// PUBLIC API
// ===========================

static BehaviorAnalysisEngine* g_behaviorEngine = nullptr;

namespace BehaviorAnalysis {
    bool initialize() {
        if (g_behaviorEngine) {
            delete g_behaviorEngine;
        }
        
        g_behaviorEngine = new BehaviorAnalysisEngine();
        return g_behaviorEngine->initialize();
    }
    
    void cleanup() {
        if (g_behaviorEngine) {
            g_behaviorEngine->cleanup();
            delete g_behaviorEngine;
            g_behaviorEngine = nullptr;
        }
    }
    
    BehaviorAnalysisResult analyzeBehavior(const WildlifeDetectionResult& detection) {
        if (g_behaviorEngine) {
            return g_behaviorEngine->analyzeBehavior(detection);
        }
        return BehaviorAnalysisResult{};
    }
    
    BehaviorAnalysisResult analyzeBehaviorBatch(const std::vector<WildlifeDetectionResult>& detections) {
        if (g_behaviorEngine) {
            return g_behaviorEngine->analyzeBehaviorBatch(detections);
        }
        return BehaviorAnalysisResult{};
    }
    
    void updateEnvironmentalContext(uint32_t timeOfDay, float weatherFactor) {
        if (g_behaviorEngine) {
            g_behaviorEngine->updateEnvironmentalContext(timeOfDay, weatherFactor);
        }
    }
    
    BehaviorMetrics getMetrics() {
        return g_behaviorEngine ? g_behaviorEngine->getMetrics() : BehaviorMetrics{};
    }
    
    void setParameters(uint32_t maxHistory, uint32_t minSequence, 
                      uint32_t maxGap, float movementThresh) {
        if (g_behaviorEngine) {
            g_behaviorEngine->setAnalysisParameters(maxHistory, minSequence, maxGap, movementThresh);
        }
    }
    
    std::vector<BehaviorSequence> getRecentBehaviors(uint32_t count) {
        return g_behaviorEngine ? g_behaviorEngine->getRecentBehaviors(count) : std::vector<BehaviorSequence>();
    }
}