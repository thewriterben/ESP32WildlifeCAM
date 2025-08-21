/**
 * @file ai_wildlife_system.h
 * @brief Main AI Wildlife Monitoring System Integration
 * 
 * Integrates all AI/ML components for comprehensive wildlife monitoring
 * with the existing ESP32 Wildlife Camera system.
 */

#ifndef AI_WILDLIFE_SYSTEM_H
#define AI_WILDLIFE_SYSTEM_H

#include "ai_common.h"
#include "tinyml/inference_engine.h"
#include "vision/species_classifier.h"
#include "vision/behavior_analyzer.h"
#include <memory>

/**
 * Complete AI Analysis Result
 */
struct WildlifeAnalysisResult {
    SpeciesResult species;
    BehaviorResult behavior;
    bool motionDetected;
    float overallConfidence;
    bool threatDetected;
    bool humanPresenceDetected;
    unsigned long processingTime;
    unsigned long timestamp;
    
    WildlifeAnalysisResult() : 
        motionDetected(false), overallConfidence(0.0f),
        threatDetected(false), humanPresenceDetected(false),
        processingTime(0), timestamp(0) {}
};

/**
 * AI Processing Configuration
 */
struct AIProcessingConfig {
    bool enableSpeciesClassification;
    bool enableBehaviorAnalysis;
    bool enableMotionDetection;
    bool enableThreatDetection;
    bool enableHumanDetection;
    bool enableRealTimeProcessing;
    bool enableBatchProcessing;
    float confidenceThreshold;
    uint32_t processingInterval;     // ms between AI analyses
    bool powerOptimized;
    
    AIProcessingConfig() : 
        enableSpeciesClassification(true),
        enableBehaviorAnalysis(true),
        enableMotionDetection(true),
        enableThreatDetection(false),
        enableHumanDetection(true),
        enableRealTimeProcessing(true),
        enableBatchProcessing(false),
        confidenceThreshold(0.6f),
        processingInterval(2000),
        powerOptimized(true) {}
};

/**
 * Intelligent Trigger System
 */
class IntelligentTrigger {
public:
    IntelligentTrigger();
    
    bool init();
    bool shouldCapture(const WildlifeAnalysisResult& analysis);
    void learnFromCapture(const WildlifeAnalysisResult& analysis, bool wasUseful);
    float predictAnimalProbability();
    void optimizeForSpecies(SpeciesType target);
    void setTimeBasedTriggers(bool enable = true);
    void setBehaviorBasedTriggers(bool enable = true);
    
    // Power optimization
    void enablePowerOptimization(bool enable = true);
    unsigned long getPredictedSleepTime();
    bool shouldWakeUp();

private:
    struct TriggerPattern {
        SpeciesType species;
        BehaviorType behavior;
        uint8_t hourOfDay;
        float probability;
        uint32_t successCount;
        uint32_t totalCount;
    };
    
    std::vector<TriggerPattern> learnedPatterns_;
    bool timeBasedTriggersEnabled_;
    bool behaviorBasedTriggersEnabled_;
    bool powerOptimizationEnabled_;
    
    void updatePatterns(const WildlifeAnalysisResult& analysis, bool success);
    float calculateTriggerProbability(const WildlifeAnalysisResult& analysis);
};

/**
 * Main AI Wildlife Monitoring System
 * 
 * Coordinates all AI components and integrates with the existing
 * camera system for intelligent wildlife monitoring.
 */
class AIWildlifeSystem {
public:
    AIWildlifeSystem();
    ~AIWildlifeSystem();

    // System initialization and configuration
    bool init();
    bool loadModels(const char* modelsDirectory);
    void configure(const AIProcessingConfig& config);
    void cleanup();
    
    // Main processing functions
    WildlifeAnalysisResult analyzeFrame(const CameraFrame& frame);
    WildlifeAnalysisResult analyzeFrameSequence(const std::vector<CameraFrame>& frames);
    bool processMotionEvent(const CameraFrame& frame);
    
    // Integration with existing system
    bool shouldTriggerCapture();
    void onImageCaptured(const CameraFrame& frame, const String& filename);
    void onMotionDetected();
    void updatePowerState(float batteryVoltage, bool isCharging);
    
    // Intelligent scheduling
    void enableIntelligentScheduling(bool enable = true);
    unsigned long getNextWakeTime();
    bool shouldEnterDeepSleep();
    void optimizeForBatteryLife();
    
    // Learning and adaptation
    void enableContinuousLearning(bool enable = true);
    void updateFromUserFeedback(const String& imageFilename, SpeciesType correctSpecies);
    void shareDataForFederatedLearning();
    
    // Performance monitoring
    AIMetrics getSystemMetrics() const;
    String getSystemStatus() const;
    void printSystemStatistics();
    
    // Configuration and settings
    void setConfidenceThreshold(float threshold);
    void enablePowerOptimization(bool enable = true);
    void setProcessingPriority(ModelType priority);
    
    // Model management
    bool updateModel(ModelType type, const char* newModelPath);
    std::vector<ModelType> getLoadedModels() const;
    bool validateAllModels();
    
    // Research and data export
    void enableResearchMode(bool enable = true);
    void exportAnalysisData(const char* filename);
    void exportLearningData(const char* filename);

private:
    // Core AI components
    std::unique_ptr<InferenceEngine> inferenceEngine_;
    std::unique_ptr<WildlifeClassifier> speciesClassifier_;
    std::unique_ptr<BehaviorAnalyzer> behaviorAnalyzer_;
    std::unique_ptr<IntelligentTrigger> intelligentTrigger_;
    
    // Configuration
    AIProcessingConfig config_;
    bool systemInitialized_;
    bool intelligentSchedulingEnabled_;
    bool continuousLearningEnabled_;
    bool researchModeEnabled_;
    
    // Performance tracking
    mutable AIMetrics systemMetrics_;
    std::vector<WildlifeAnalysisResult> recentAnalyses_;
    
    // Power management
    float batteryVoltage_;
    bool isCharging_;
    unsigned long lastProcessingTime_;
    
    // Processing optimization
    unsigned long lastAnalysisTime_;
    ModelType processingPriority_;
    
    // Learning data
    struct LearningRecord {
        String imageFilename;
        WildlifeAnalysisResult analysis;
        SpeciesType correctSpecies;
        BehaviorType correctBehavior;
        bool userVerified;
        unsigned long timestamp;
    };
    std::vector<LearningRecord> learningData_;
    
    // Internal processing functions
    bool initializeAIComponents();
    WildlifeAnalysisResult runFullAnalysis(const CameraFrame& frame);
    WildlifeAnalysisResult runOptimizedAnalysis(const CameraFrame& frame);
    
    // Power-optimized processing
    WildlifeAnalysisResult runLowPowerAnalysis(const CameraFrame& frame);
    bool shouldRunFullAnalysis();
    void adjustProcessingBasedOnPower();
    
    // Learning and adaptation
    void processLearningData();
    void updateModelsFromLearning();
    void saveLearningData();
    void loadLearningData();
    
    // Utility functions
    void updateSystemMetrics(const WildlifeAnalysisResult& result);
    void logAnalysisResult(const WildlifeAnalysisResult& result);
    bool isHighPriorityDetection(const WildlifeAnalysisResult& result);
    
    // Constants
    static const size_t MAX_RECENT_ANALYSES = 100;
    static const size_t MAX_LEARNING_RECORDS = 1000;
    static const unsigned long MIN_PROCESSING_INTERVAL = 1000; // 1 second
    static const float LOW_BATTERY_THRESHOLD = 3.4f; // Volts
};

// Global AI system instance (singleton pattern)
extern AIWildlifeSystem* g_aiSystem;

// Integration functions for existing code
bool initializeAISystem();
void cleanupAISystem();
bool isAISystemReady();
WildlifeAnalysisResult analyzeCurrentFrame(camera_fb_t* fb);
bool shouldAITriggerCapture();
void notifyAIOfImageCapture(const String& filename);

#endif // AI_WILDLIFE_SYSTEM_H