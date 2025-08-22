#ifndef ML_OPTIMIZER_H
#define ML_OPTIMIZER_H

#include <Arduino.h>
#include <vector>
#include <memory>

/**
 * ML Optimizer - Enhanced AI/ML performance for ESP32WildlifeCAM
 * 
 * Implements quantized neural networks, optimized image processing,
 * and adaptive AI pipeline for 3x speed improvement and better accuracy.
 */

// Forward declarations
struct CameraFrame;

// Quantized neural network for 3x speed improvement
class QuantizedNeuralNetwork {
    int8_t* weights_q8;          // 8-bit quantized weights
    float scale_factor;
    int32_t zero_point;
    size_t input_size;
    size_t output_size;
    size_t layer_count;
    bool initialized;
    
public:
    QuantizedNeuralNetwork();
    ~QuantizedNeuralNetwork();
    
    bool init(size_t input_size, size_t output_size, size_t layers);
    void cleanup();
    
    // SIMD-optimized inference for ESP32
    std::vector<float> inferenceOptimized(const std::vector<float>& input);
    
    // Fixed-point arithmetic for ESP32 optimization
    int32_t dotProductQ8(const int8_t* a, const int8_t* b, size_t len);
    int32_t matrixMultiplyQ8(const int8_t* matrix, const int8_t* vector, 
                           size_t rows, size_t cols);
    
    // Model compression and optimization
    void pruneWeights(float threshold = 0.01f);
    bool quantizeFromFloat(const std::vector<std::vector<float>>& weights);
    void optimizeForESP32();
    
    // Performance monitoring
    float getInferenceTime() const { return lastInferenceTime; }
    size_t getModelSize() const;
    float getCompressionRatio() const;
    
private:
    float lastInferenceTime;
    size_t originalModelSize;
    
    void allocateQuantizedMemory();
    void deallocateQuantizedMemory();
    float dequantize(int8_t value);
    int8_t quantize(float value);
};

// Optimized image processing pipeline
class FastImageProcessor {
    uint8_t* processingBuffer;
    size_t bufferSize;
    bool initialized;
    
    // Performance metrics
    float lastProcessingTime;
    uint32_t totalProcessedFrames;
    
public:
    FastImageProcessor();
    ~FastImageProcessor();
    
    bool init(size_t maxImageSize);
    void cleanup();
    
    // Hardware-accelerated edge detection using ESP32 optimizations
    bool detectEdgesHardware(const uint8_t* image, uint8_t* edges, 
                           uint16_t width, uint16_t height);
    
    // Optimized color space conversion
    void rgb565ToGrayscale(const uint16_t* rgb, uint8_t* gray, size_t pixels);
    void yuvToRgb(const uint8_t* yuv, uint8_t* rgb, size_t pixels);
    void applyGammaCorrection(uint8_t* image, size_t pixels, float gamma = 2.2f);
    
    // Motion detection using background subtraction
    bool detectMotionOptimized(const uint8_t* current, const uint8_t* background,
                              uint16_t width, uint16_t height, float threshold);
    
    // Feature extraction optimization
    std::vector<float> extractHOGFeatures(const uint8_t* image, 
                                         uint16_t width, uint16_t height);
    std::vector<float> extractLBPFeatures(const uint8_t* image,
                                         uint16_t width, uint16_t height);
    
    // Image enhancement
    void enhanceContrast(uint8_t* image, uint16_t width, uint16_t height);
    void reduceNoise(uint8_t* image, uint16_t width, uint16_t height);
    void sharpenImage(uint8_t* image, uint16_t width, uint16_t height);
    
    // Performance monitoring
    float getProcessingTime() const { return lastProcessingTime; }
    uint32_t getProcessedFrameCount() const { return totalProcessedFrames; }
    float getAverageProcessingTime() const;

private:
    // Assembly-optimized kernels for ESP32
    void convolveASM(const uint8_t* input, uint8_t* output, 
                    const int8_t* kernel, uint16_t width, uint16_t height);
    void sobel3x3(const uint8_t* input, uint8_t* output, 
                  uint16_t width, uint16_t height);
    void gaussian3x3(const uint8_t* input, uint8_t* output,
                     uint16_t width, uint16_t height);
    
    // Optimized histogram operations
    void calculateHistogram(const uint8_t* image, uint32_t* histogram, size_t pixels);
    void equalizeHistogram(uint8_t* image, size_t pixels);
};

// Adaptive AI pipeline for dynamic model selection
class AdaptiveAIPipeline {
    QuantizedNeuralNetwork primaryModel;
    QuantizedNeuralNetwork lightweightModel;
    FastImageProcessor imageProcessor;
    
    float batteryThreshold;
    float performanceThreshold;
    bool useLightweightModel;
    bool initialized;
    
    // Model performance tracking
    struct ModelPerformance {
        float accuracy;
        float inferenceTime;
        float powerConsumption;
        uint32_t usageCount;
        
        ModelPerformance() : accuracy(0), inferenceTime(0), 
                           powerConsumption(0), usageCount(0) {}
    };
    
    ModelPerformance primaryPerformance;
    ModelPerformance lightweightPerformance;
    
public:
    AdaptiveAIPipeline();
    ~AdaptiveAIPipeline();
    
    bool init();
    void cleanup();
    
    // Dynamic model selection based on power/performance requirements
    bool runInference(const uint8_t* image, uint16_t width, uint16_t height,
                     float& confidence, String& species, String& behavior);
    
    void selectModelBasedOnPower(float batteryLevel);
    void selectModelBasedOnPerformance(float requiredAccuracy, float maxInferenceTime);
    
    // Progressive inference - stop early if confident
    bool earlyExitInference(const std::vector<float>& features, 
                           float confidenceThreshold = 0.9f);
    
    // Model management
    bool loadPrimaryModel(const uint8_t* modelData, size_t modelSize);
    bool loadLightweightModel(const uint8_t* modelData, size_t modelSize);
    void unloadModels();
    
    // Performance optimization
    void optimizeForBatteryLife();
    void optimizeForAccuracy();
    void optimizeForSpeed();
    
    // Analytics and monitoring
    struct PipelineMetrics {
        float averageInferenceTime;
        float averageAccuracy;
        float powerEfficiency;
        uint32_t totalInferences;
        uint32_t successfulInferences;
        
        PipelineMetrics() : averageInferenceTime(0), averageAccuracy(0),
                          powerEfficiency(0), totalInferences(0), successfulInferences(0) {}
    };
    
    PipelineMetrics getMetrics() const;
    void resetMetrics();
    
private:
    void updatePerformanceMetrics(bool primaryUsed, float inferenceTime, 
                                float accuracy, float powerUsed);
    bool shouldUseLightweightModel(float batteryLevel, float urgency);
    std::vector<float> preprocessImage(const uint8_t* image, uint16_t width, uint16_t height);
    bool postprocessResults(const std::vector<float>& output, float& confidence, 
                          String& species, String& behavior);
};

// Specialized wildlife detection optimizations
class WildlifeDetectionOptimizer {
public:
    // Species-specific optimization
    struct SpeciesProfile {
        String speciesName;
        float sizeRange[2];        // Min/max size in pixels
        float colorProfile[3];     // Average RGB values
        float movementPattern;     // Movement speed indicator
        float detectionDifficulty; // 0-1 scale
        
        SpeciesProfile() : speciesName("unknown"), movementPattern(0.5f), 
                          detectionDifficulty(0.5f) {
            sizeRange[0] = 10; sizeRange[1] = 500;
            colorProfile[0] = colorProfile[1] = colorProfile[2] = 128;
        }
    };
    
    static std::vector<SpeciesProfile> getCommonSpeciesProfiles();
    static SpeciesProfile getProfileForSpecies(const String& species);
    
    // Adaptive detection parameters
    static float calculateOptimalThreshold(const String& species, float lightLevel);
    static void adjustCameraSettings(const String& targetSpecies, float lightLevel);
    static bool isOptimalTimeForSpecies(const String& species, uint32_t timeOfDay);
    
    // Environmental optimization
    static float compensateForWeather(float baseThreshold, float temperature, 
                                    float humidity, float windSpeed);
    static void optimizeForLightConditions(float lightLevel, bool& useIRIllumination,
                                         float& exposureAdjustment);
    
    // Behavior-specific optimizations
    static float getPredictedActivityLevel(const String& species, uint32_t timeOfDay, 
                                         uint32_t season);
    static std::vector<String> getActiveSpeciesForTime(uint32_t timeOfDay, uint32_t season);
};

#endif // ML_OPTIMIZER_H