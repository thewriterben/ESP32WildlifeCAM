/**
 * @file inference_engine.h
 * @brief TinyML Inference Engine for ESP32 Wildlife Camera
 * 
 * Provides a unified interface for running TensorFlow Lite models
 * on ESP32 hardware with optimizations for wildlife detection.
 */

#ifndef INFERENCE_ENGINE_H
#define INFERENCE_ENGINE_H

#include "../ai_common.h"
#include <vector>
#include <memory>

// Forward declarations for TensorFlow Lite
namespace tflite {
    class MicroInterpreter;
    class Model;
    class MicroOpResolver;
}

/**
 * Model Types supported by the inference engine
 */
enum class ModelType {
    SPECIES_CLASSIFIER,
    BEHAVIOR_ANALYZER,
    MOTION_DETECTOR,
    OBJECT_TRACKER,
    THREAT_DETECTOR
};

/**
 * TinyML Inference Engine Class
 * 
 * Handles loading, optimizing, and running TensorFlow Lite models
 * optimized for ESP32 hardware constraints.
 */
class InferenceEngine {
public:
    InferenceEngine();
    ~InferenceEngine();

    // Initialization and model management
    bool init();
    bool loadModel(const uint8_t* modelData, size_t modelSize, ModelType type);
    bool loadModelFromFile(const char* filename, ModelType type);
    void unloadModel(ModelType type);
    bool isModelLoaded(ModelType type) const;
    
    // Inference operations
    AIResult runInference(const CameraFrame& frame, ModelType type);
    std::vector<AIResult> runMultipleInference(const CameraFrame& frame);
    
    // Performance and optimization
    void enableQuantization(bool enable = true);
    void enablePowerOptimization(bool enable = true);
    void setMemoryLimit(size_t maxMemoryBytes);
    AIMetrics getPerformanceMetrics() const;
    void resetMetrics();
    
    // Model information
    ModelInfo getModelInfo(ModelType type) const;
    std::vector<ModelType> getLoadedModels() const;
    size_t getTotalMemoryUsage() const;
    
    // Configuration
    void setConfidenceThreshold(float threshold);
    float getConfidenceThreshold() const;
    void setMaxInferenceTime(unsigned long maxTimeMs);
    
    // Debug and diagnostics
    bool testModel(ModelType type);
    void printModelDetails(ModelType type);
    bool validateModelIntegrity(ModelType type);

private:
    struct ModelContainer {
        std::unique_ptr<tflite::Model> model;
        std::unique_ptr<tflite::MicroInterpreter> interpreter;
        std::unique_ptr<tflite::MicroOpResolver> resolver;
        uint8_t* tensorArena;
        size_t arenaSize;
        ModelInfo info;
        bool isLoaded;
        
        ModelContainer() : 
            tensorArena(nullptr), arenaSize(0), isLoaded(false) {}
        ~ModelContainer();
    };

    // Model management
    std::vector<std::unique_ptr<ModelContainer>> models_;
    ModelContainer* getModelContainer(ModelType type);
    bool setupInterpreter(ModelContainer* container, const uint8_t* modelData, size_t modelSize);
    
    // Memory management
    size_t maxMemoryLimit_;
    size_t currentMemoryUsage_;
    bool allocateTensorArena(ModelContainer* container, size_t requiredSize);
    void deallocateTensorArena(ModelContainer* container);
    
    // Preprocessing and postprocessing
    bool preprocessImage(const CameraFrame& frame, float* inputTensor, ModelType type);
    AIResult postprocessOutput(const float* outputTensor, ModelType type, const CameraFrame& frame);
    
    // Optimization features
    bool quantizationEnabled_;
    bool powerOptimizationEnabled_;
    float confidenceThreshold_;
    unsigned long maxInferenceTime_;
    
    // Performance tracking
    mutable AIMetrics metrics_;
    void updateMetrics(float inferenceTime, bool success);
    
    // Model-specific helpers
    AIResult processSpeciesClassification(const float* output, const CameraFrame& frame);
    AIResult processBehaviorAnalysis(const float* output, const CameraFrame& frame);
    AIResult processMotionDetection(const float* output, const CameraFrame& frame);
    
    // Utility functions
    ModelType getModelTypeFromIndex(size_t index) const;
    size_t getModelIndex(ModelType type) const;
    bool validateModelFormat(const uint8_t* modelData, size_t modelSize);
    
    // Constants
    static const size_t MAX_MODELS = 5;
    static const size_t DEFAULT_ARENA_SIZE = 64 * 1024;  // 64KB
    static const size_t MAX_MEMORY_LIMIT = 512 * 1024;   // 512KB
    static const float DEFAULT_CONFIDENCE_THRESHOLD;
    static const unsigned long DEFAULT_MAX_INFERENCE_TIME;
};

// Utility functions for model management
const char* modelTypeToString(ModelType type);
ModelType stringToModelType(const char* typeStr);
size_t calculateRequiredArenaSize(const uint8_t* modelData, size_t modelSize);

#endif // INFERENCE_ENGINE_H