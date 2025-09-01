/**
 * @file wildlife_detection.cpp
 * @brief Enhanced On-Device Species Classification for Phase 2
 * 
 * This implementation provides advanced wildlife detection capabilities
 * using TensorFlow Lite Micro with species classification, confidence
 * scoring, and environmental context integration.
 * 
 * Phase 2 Enhancement - March 31, 2026 Target
 */

#include "../../../firmware/src/ai/tensorflow_lite_implementation.h"
#include "../../../include/network/mesh_protocol.h"
#include <ArduinoJson.h>
#include <esp_camera.h>

// ===========================
// SPECIES CLASSIFICATION
// ===========================

enum WildlifeSpecies {
    SPECIES_UNKNOWN = 0,
    SPECIES_DEER_WHITE_TAILED = 1,
    SPECIES_DEER_MULE = 2,
    SPECIES_ELK = 3,
    SPECIES_BEAR_BLACK = 4,
    SPECIES_BEAR_BROWN = 5,
    SPECIES_WOLF = 6,
    SPECIES_COYOTE = 7,
    SPECIES_FOX_RED = 8,
    SPECIES_FOX_GRAY = 9,
    SPECIES_RABBIT_COTTONTAIL = 10,
    SPECIES_SQUIRREL_GRAY = 11,
    SPECIES_SQUIRREL_RED = 12,
    SPECIES_RACCOON = 13,
    SPECIES_OPOSSUM = 14,
    SPECIES_SKUNK = 15,
    SPECIES_TURKEY_WILD = 16,
    SPECIES_HAWK_RED_TAILED = 17,
    SPECIES_EAGLE_BALD = 18,
    SPECIES_OWL_GREAT_HORNED = 19,
    SPECIES_DOMESTIC_CAT = 20,
    SPECIES_DOMESTIC_DOG = 21,
    SPECIES_LIVESTOCK_CATTLE = 22,
    SPECIES_LIVESTOCK_SHEEP = 23,
    SPECIES_HUMAN = 24,
    SPECIES_VEHICLE = 25,
    SPECIES_BIRD_SMALL = 26,
    SPECIES_BIRD_MEDIUM = 27,
    SPECIES_BIRD_LARGE = 28,
    SPECIES_MAMMAL_SMALL = 29,
    SPECIES_MAMMAL_MEDIUM = 30,
    SPECIES_MAMMAL_LARGE = 31
};

struct WildlifeDetectionResult {
    WildlifeSpecies primarySpecies;
    float primaryConfidence;
    WildlifeSpecies secondarySpecies;
    float secondaryConfidence;
    uint16_t boundingBox[4];      // x, y, width, height
    uint8_t objectCount;          // Number of detected objects
    float environmentalScore;     // Environmental context score
    uint32_t timestamp;
    bool isValid;
};

struct DetectionMetrics {
    uint32_t totalDetections;
    uint32_t validDetections;
    uint32_t falsePositives;
    float averageConfidence;
    uint32_t processingTimeMs;
    uint32_t lastDetectionTime;
};

// ===========================
// WILDLIFE DETECTION ENGINE
// ===========================

class WildlifeDetectionEngine {
private:
    // AI Model components
    TensorFlowLiteImplementation* tfLite_;
    bool isInitialized_;
    
    // Detection parameters
    float confidenceThreshold_;
    float nmsThreshold_;           // Non-maximum suppression threshold
    uint16_t minObjectSize_;       // Minimum object size in pixels
    uint16_t maxObjectSize_;       // Maximum object size in pixels
    
    // Performance tracking
    DetectionMetrics metrics_;
    uint32_t lastOptimization_;
    
    // Environmental context
    float timeOfDayScore_;
    float weatherScore_;
    float seasonalScore_;
    float locationScore_;
    
    // Detection history for temporal filtering
    std::vector<WildlifeDetectionResult> recentDetections_;
    uint32_t maxHistorySize_;
    
public:
    WildlifeDetectionEngine() : 
        tfLite_(nullptr), isInitialized_(false), confidenceThreshold_(0.7),
        nmsThreshold_(0.5), minObjectSize_(32), maxObjectSize_(512),
        lastOptimization_(0), timeOfDayScore_(1.0), weatherScore_(1.0),
        seasonalScore_(1.0), locationScore_(1.0), maxHistorySize_(10) {
        
        // Initialize metrics
        memset(&metrics_, 0, sizeof(metrics_));
        
        // Reserve space for detection history
        recentDetections_.reserve(maxHistorySize_);
    }
    
    // ===========================
    // INITIALIZATION
    // ===========================
    
    /**
     * Initialize wildlife detection system
     */
    bool initialize() {
        if (isInitialized_) return true;
        
        // Initialize TensorFlow Lite system
        if (!initializeTensorFlowLite()) {
            Serial.println("Failed to initialize TensorFlow Lite");
            return false;
        }
        
        tfLite_ = g_tensorflowImplementation;
        if (!tfLite_ || !tfLite_->isInitialized()) {
            Serial.println("TensorFlow Lite implementation not available");
            return false;
        }
        
        // Load wildlife detection models
        if (!loadWildlifeModels("/models/wildlife")) {
            Serial.println("Failed to load wildlife models");
            return false;
        }
        
        // Verify species classifier model is loaded
        if (!tfLite_->isModelLoaded(MODEL_SPECIES_CLASSIFIER)) {
            Serial.println("Species classifier model not loaded");
            return false;
        }
        
        isInitialized_ = true;
        Serial.println("Wildlife detection engine initialized successfully");
        
        return true;
    }
    
    /**
     * Cleanup detection engine
     */
    void cleanup() {
        if (tfLite_) {
            tfLite_->cleanup();
        }
        isInitialized_ = false;
    }
    
    // ===========================
    // MAIN DETECTION PIPELINE
    // ===========================
    
    /**
     * Detect wildlife in camera frame
     */
    WildlifeDetectionResult detectWildlife(camera_fb_t* fb) {
        WildlifeDetectionResult result = {};
        result.isValid = false;
        result.timestamp = millis();
        
        if (!isInitialized_ || !fb || !fb->buf || fb->len == 0) {
            return result;
        }
        
        uint32_t startTime = millis();
        
        // Preprocess image for inference
        if (!preprocessImage(fb)) {
            Serial.println("Image preprocessing failed");
            return result;
        }
        
        // Run species classification inference
        InferenceResult inferenceResult = tfLite_->runInference(
            MODEL_SPECIES_CLASSIFIER, fb->buf, fb->width, fb->height, 3);
        
        if (!inferenceResult.isValid) {
            Serial.println("Species classification inference failed");
            return result;
        }
        
        // Process inference results
        result = processInferenceResult(inferenceResult, fb);
        
        // Apply environmental context scoring
        result.environmentalScore = calculateEnvironmentalScore(result);
        
        // Apply temporal filtering
        result = applyTemporalFiltering(result);
        
        // Update performance metrics
        uint32_t processingTime = millis() - startTime;
        updateMetrics(result, processingTime);
        
        // Add to detection history
        addToHistory(result);
        
        if (result.isValid) {
            Serial.printf("Wildlife detected: species=%d, confidence=%.2f, env_score=%.2f\n",
                         result.primarySpecies, result.primaryConfidence, result.environmentalScore);
        }
        
        return result;
    }
    
    /**
     * Detect wildlife with custom parameters
     */
    WildlifeDetectionResult detectWildlifeCustom(camera_fb_t* fb, 
                                                float customThreshold,
                                                bool enableTemporalFilter) {
        float originalThreshold = confidenceThreshold_;
        confidenceThreshold_ = customThreshold;
        
        WildlifeDetectionResult result = detectWildlife(fb);
        
        if (!enableTemporalFilter) {
            // Skip temporal filtering for this detection
            result = processInferenceResult(
                tfLite_->runInference(MODEL_SPECIES_CLASSIFIER, fb->buf, fb->width, fb->height, 3), 
                fb);
        }
        
        confidenceThreshold_ = originalThreshold;
        return result;
    }
    
    // ===========================
    // CONFIDENCE AND VALIDATION
    // ===========================
    
    /**
     * Calculate detection confidence with multiple factors
     */
    float calculateOverallConfidence(const InferenceResult& inference, 
                                   const WildlifeDetectionResult& detection) {
        float baseConfidence = inference.confidence;
        
        // Environmental context boost
        float environmentalBoost = detection.environmentalScore * 0.1; // Up to 10% boost
        
        // Size validation boost
        float sizeBoost = validateObjectSize(detection.boundingBox) ? 0.05 : -0.1;
        
        // Temporal consistency boost (if available)
        float temporalBoost = calculateTemporalConsistency(detection) * 0.1;
        
        // Calculate final confidence
        float finalConfidence = baseConfidence + environmentalBoost + sizeBoost + temporalBoost;
        
        // Clamp to valid range
        return std::max(0.0f, std::min(1.0f, finalConfidence));
    }
    
    /**
     * Validate detection using multiple criteria
     */
    bool validateDetection(const WildlifeDetectionResult& detection) {
        // Confidence threshold check
        if (detection.primaryConfidence < confidenceThreshold_) {
            return false;
        }
        
        // Object size validation
        if (!validateObjectSize(detection.boundingBox)) {
            return false;
        }
        
        // Environmental context check
        if (detection.environmentalScore < 0.3) { // Minimum environmental score
            return false;
        }
        
        // Species-specific validation
        if (!validateSpeciesContext(detection.primarySpecies, detection.environmentalScore)) {
            return false;
        }
        
        return true;
    }
    
    /**
     * Apply uncertainty quantification
     */
    float calculateUncertainty(const InferenceResult& inference) {
        // Calculate entropy-based uncertainty
        float entropy = 0.0;
        const float* probabilities = inference.classificationResults;
        
        for (int i = 0; i < 32; i++) { // Assuming 32 species classes
            if (probabilities[i] > 0) {
                entropy -= probabilities[i] * log(probabilities[i]);
            }
        }
        
        // Normalize entropy (max entropy for 32 classes is log(32))
        float normalizedEntropy = entropy / log(32);
        
        return normalizedEntropy;
    }
    
    // ===========================
    // ENVIRONMENTAL CONTEXT
    // ===========================
    
    /**
     * Calculate environmental context score
     */
    float calculateEnvironmentalScore(const WildlifeDetectionResult& detection) {
        float score = 1.0;
        
        // Time of day factor
        score *= getTimeOfDayScore(detection.primarySpecies);
        
        // Seasonal factor
        score *= getSeasonalScore(detection.primarySpecies);
        
        // Weather factor
        score *= getWeatherScore(detection.primarySpecies);
        
        // Location/habitat factor
        score *= getLocationScore(detection.primarySpecies);
        
        return std::max(0.0f, std::min(1.0f, score));
    }
    
    /**
     * Update environmental context parameters
     */
    void updateEnvironmentalContext(float temperature, float humidity, 
                                  uint8_t lightLevel, uint32_t timestamp) {
        // Update time of day score based on light level
        timeOfDayScore_ = calculateTimeOfDayScore(lightLevel, timestamp);
        
        // Update weather score based on temperature and humidity
        weatherScore_ = calculateWeatherScore(temperature, humidity);
        
        // Seasonal score could be updated based on date/time
        // This would require RTC or NTP time
        
        Serial.printf("Environmental context updated: time=%.2f, weather=%.2f\n",
                     timeOfDayScore_, weatherScore_);
    }
    
    // ===========================
    // PERFORMANCE OPTIMIZATION
    // ===========================
    
    /**
     * Optimize detection parameters based on performance
     */
    bool optimizePerformance() {
        uint32_t currentTime = millis();
        
        // Only optimize every 5 minutes
        if (currentTime - lastOptimization_ < 300000) {
            return true;
        }
        
        // Calculate current performance metrics
        float falsePositiveRate = metrics_.totalDetections > 0 ? 
            (float)metrics_.falsePositives / metrics_.totalDetections : 0.0;
        
        float validDetectionRate = metrics_.totalDetections > 0 ?
            (float)metrics_.validDetections / metrics_.totalDetections : 0.0;
        
        // Adjust confidence threshold based on performance
        if (falsePositiveRate > 0.2) { // Too many false positives
            confidenceThreshold_ = std::min(0.9f, confidenceThreshold_ + 0.05f);
            Serial.printf("Increased confidence threshold to %.2f (FP rate: %.2f)\n",
                         confidenceThreshold_, falsePositiveRate);
        } else if (falsePositiveRate < 0.05 && validDetectionRate > 0.8) {
            confidenceThreshold_ = std::max(0.5f, confidenceThreshold_ - 0.02f);
            Serial.printf("Decreased confidence threshold to %.2f (FP rate: %.2f)\n",
                         confidenceThreshold_, falsePositiveRate);
        }
        
        lastOptimization_ = currentTime;
        return true;
    }
    
    /**
     * Get detection metrics
     */
    DetectionMetrics getMetrics() const {
        return metrics_;
    }
    
    /**
     * Reset detection metrics
     */
    void resetMetrics() {
        memset(&metrics_, 0, sizeof(metrics_));
    }
    
    // ===========================
    // CONFIGURATION
    // ===========================
    
    /**
     * Set detection parameters
     */
    void setDetectionParameters(float confidenceThreshold, float nmsThreshold,
                              uint16_t minSize, uint16_t maxSize) {
        confidenceThreshold_ = std::max(0.1f, std::min(0.99f, confidenceThreshold));
        nmsThreshold_ = std::max(0.1f, std::min(0.9f, nmsThreshold));
        minObjectSize_ = std::max(16, (int)minSize);
        maxObjectSize_ = std::min(1024, (int)maxSize);
        
        Serial.printf("Detection parameters updated: conf=%.2f, nms=%.2f, size=%d-%d\n",
                     confidenceThreshold_, nmsThreshold_, minObjectSize_, maxObjectSize_);
    }
    
    /**
     * Get current detection parameters
     */
    void getDetectionParameters(float& confidenceThreshold, float& nmsThreshold,
                              uint16_t& minSize, uint16_t& maxSize) const {
        confidenceThreshold = confidenceThreshold_;
        nmsThreshold = nmsThreshold_;
        minSize = minObjectSize_;
        maxSize = maxObjectSize_;
    }
    
private:
    // ===========================
    // HELPER FUNCTIONS
    // ===========================
    
    bool preprocessImage(camera_fb_t* fb) {
        // Basic image validation
        if (fb->width < 64 || fb->height < 64) {
            return false;
        }
        
        // TODO: Add image preprocessing steps
        // - Resize to model input size
        // - Normalize pixel values
        // - Apply noise reduction if needed
        
        return true;
    }
    
    WildlifeDetectionResult processInferenceResult(const InferenceResult& inference, 
                                                 camera_fb_t* fb) {
        WildlifeDetectionResult result = {};
        result.timestamp = millis();
        result.isValid = false;
        
        if (!inference.isValid || inference.confidence < confidenceThreshold_) {
            return result;
        }
        
        // Extract primary and secondary species
        result.primarySpecies = (WildlifeSpecies)inference.classificationId;
        result.primaryConfidence = inference.confidence;
        
        // Find secondary species (second highest confidence)
        float secondaryConf = 0.0;
        uint8_t secondaryId = 0;
        for (int i = 0; i < 32; i++) {
            if (i != inference.classificationId && 
                inference.classificationResults[i] > secondaryConf) {
                secondaryConf = inference.classificationResults[i];
                secondaryId = i;
            }
        }
        result.secondarySpecies = (WildlifeSpecies)secondaryId;
        result.secondaryConfidence = secondaryConf;
        
        // Extract bounding box (if available from detection results)
        if (inference.detectionResults.size() > 0) {
            const auto& detection = inference.detectionResults[0];
            result.boundingBox[0] = detection.x;
            result.boundingBox[1] = detection.y;
            result.boundingBox[2] = detection.width;
            result.boundingBox[3] = detection.height;
            result.objectCount = inference.detectionResults.size();
        } else {
            // Default to full frame if no detection box available
            result.boundingBox[0] = 0;
            result.boundingBox[1] = 0;
            result.boundingBox[2] = fb->width;
            result.boundingBox[3] = fb->height;
            result.objectCount = 1;
        }
        
        // Calculate overall confidence
        result.primaryConfidence = calculateOverallConfidence(inference, result);
        
        // Validate detection
        result.isValid = validateDetection(result);
        
        return result;
    }
    
    WildlifeDetectionResult applyTemporalFiltering(const WildlifeDetectionResult& current) {
        if (recentDetections_.empty()) {
            return current;
        }
        
        // Check for consistency with recent detections
        int consistentCount = 0;
        for (const auto& recent : recentDetections_) {
            if (recent.primarySpecies == current.primarySpecies ||
                recent.secondarySpecies == current.primarySpecies) {
                consistentCount++;
            }
        }
        
        WildlifeDetectionResult filtered = current;
        
        // Boost confidence if temporally consistent
        if (consistentCount > 0) {
            float boost = 0.1 * (float)consistentCount / recentDetections_.size();
            filtered.primaryConfidence = std::min(1.0f, filtered.primaryConfidence + boost);
        }
        
        return filtered;
    }
    
    void addToHistory(const WildlifeDetectionResult& detection) {
        if (!detection.isValid) return;
        
        recentDetections_.push_back(detection);
        
        // Maintain history size
        if (recentDetections_.size() > maxHistorySize_) {
            recentDetections_.erase(recentDetections_.begin());
        }
    }
    
    void updateMetrics(const WildlifeDetectionResult& result, uint32_t processingTime) {
        metrics_.totalDetections++;
        metrics_.processingTimeMs = processingTime;
        
        if (result.isValid) {
            metrics_.validDetections++;
            metrics_.lastDetectionTime = result.timestamp;
            
            // Update average confidence
            metrics_.averageConfidence = 
                (metrics_.averageConfidence * (metrics_.validDetections - 1) + result.primaryConfidence) / 
                metrics_.validDetections;
        }
        
        // TODO: Implement false positive detection logic
        // This would require ground truth data or user feedback
    }
    
    bool validateObjectSize(const uint16_t boundingBox[4]) const {
        uint16_t width = boundingBox[2];
        uint16_t height = boundingBox[3];
        uint16_t area = width * height;
        
        return (width >= minObjectSize_ && width <= maxObjectSize_ &&
                height >= minObjectSize_ && height <= maxObjectSize_ &&
                area >= minObjectSize_ * minObjectSize_);
    }
    
    bool validateSpeciesContext(WildlifeSpecies species, float environmentalScore) const {
        // Species-specific validation rules
        switch (species) {
            case SPECIES_BEAR_BLACK:
            case SPECIES_BEAR_BROWN:
                return environmentalScore > 0.4; // Bears are cautious
                
            case SPECIES_DEER_WHITE_TAILED:
            case SPECIES_DEER_MULE:
                return environmentalScore > 0.3; // Deer are alert
                
            case SPECIES_DOMESTIC_CAT:
            case SPECIES_DOMESTIC_DOG:
                return true; // Domestic animals less constrained by environment
                
            case SPECIES_HUMAN:
                return environmentalScore > 0.2; // Humans less constrained
                
            default:
                return environmentalScore > 0.3; // Default threshold
        }
    }
    
    float calculateTemporalConsistency(const WildlifeDetectionResult& detection) const {
        if (recentDetections_.empty()) return 0.0;
        
        int matches = 0;
        for (const auto& recent : recentDetections_) {
            if (recent.primarySpecies == detection.primarySpecies) {
                matches++;
            }
        }
        
        return (float)matches / recentDetections_.size();
    }
    
    float getTimeOfDayScore(WildlifeSpecies species) const {
        // This would be enhanced with actual time and species behavior data
        return timeOfDayScore_;
    }
    
    float getSeasonalScore(WildlifeSpecies species) const {
        return seasonalScore_;
    }
    
    float getWeatherScore(WildlifeSpecies species) const {
        return weatherScore_;
    }
    
    float getLocationScore(WildlifeSpecies species) const {
        return locationScore_;
    }
    
    float calculateTimeOfDayScore(uint8_t lightLevel, uint32_t timestamp) const {
        // Simple day/night scoring based on light level
        if (lightLevel > 128) {
            return 1.0; // Daylight
        } else if (lightLevel > 64) {
            return 0.8; // Twilight
        } else {
            return 0.6; // Night
        }
    }
    
    float calculateWeatherScore(float temperature, float humidity) const {
        // Simple weather scoring
        float tempScore = 1.0;
        if (temperature < -10 || temperature > 40) {
            tempScore = 0.7; // Extreme temperatures reduce activity
        }
        
        float humidityScore = 1.0;
        if (humidity > 90) {
            humidityScore = 0.8; // High humidity reduces activity
        }
        
        return tempScore * humidityScore;
    }
};

// ===========================
// PUBLIC API
// ===========================

static WildlifeDetectionEngine* g_detectionEngine = nullptr;

namespace WildlifeDetection {
    bool initialize() {
        if (g_detectionEngine) {
            delete g_detectionEngine;
        }
        
        g_detectionEngine = new WildlifeDetectionEngine();
        return g_detectionEngine->initialize();
    }
    
    void cleanup() {
        if (g_detectionEngine) {
            g_detectionEngine->cleanup();
            delete g_detectionEngine;
            g_detectionEngine = nullptr;
        }
    }
    
    WildlifeDetectionResult detectWildlife(camera_fb_t* fb) {
        if (g_detectionEngine) {
            return g_detectionEngine->detectWildlife(fb);
        }
        return WildlifeDetectionResult{};
    }
    
    WildlifeDetectionResult detectWildlifeCustom(camera_fb_t* fb, 
                                               float threshold, 
                                               bool enableTemporalFilter) {
        if (g_detectionEngine) {
            return g_detectionEngine->detectWildlifeCustom(fb, threshold, enableTemporalFilter);
        }
        return WildlifeDetectionResult{};
    }
    
    void updateEnvironmentalContext(float temperature, float humidity, 
                                  uint8_t lightLevel, uint32_t timestamp) {
        if (g_detectionEngine) {
            g_detectionEngine->updateEnvironmentalContext(temperature, humidity, 
                                                        lightLevel, timestamp);
        }
    }
    
    bool optimizePerformance() {
        return g_detectionEngine ? g_detectionEngine->optimizePerformance() : false;
    }
    
    DetectionMetrics getMetrics() {
        return g_detectionEngine ? g_detectionEngine->getMetrics() : DetectionMetrics{};
    }
    
    void setParameters(float confidenceThreshold, float nmsThreshold,
                      uint16_t minSize, uint16_t maxSize) {
        if (g_detectionEngine) {
            g_detectionEngine->setDetectionParameters(confidenceThreshold, nmsThreshold, 
                                                    minSize, maxSize);
        }
    }
}