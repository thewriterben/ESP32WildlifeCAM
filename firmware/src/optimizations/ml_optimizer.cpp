#include "ml_optimizer.h"
#include "../config.h"

// QuantizedNeuralNetwork implementation
QuantizedNeuralNetwork::QuantizedNeuralNetwork() 
    : weights_q8(nullptr), scale_factor(1.0f), zero_point(0), 
      input_size(0), output_size(0), layer_count(0), initialized(false),
      lastInferenceTime(0), originalModelSize(0) {
}

QuantizedNeuralNetwork::~QuantizedNeuralNetwork() {
    cleanup();
}

bool QuantizedNeuralNetwork::init(size_t input_size, size_t output_size, size_t layers) {
    if (initialized) return true;
    
    this->input_size = input_size;
    this->output_size = output_size;
    this->layer_count = layers;
    
    allocateQuantizedMemory();
    
    initialized = true;
    DEBUG_PRINTLN("Quantized Neural Network initialized");
    return true;
}

void QuantizedNeuralNetwork::cleanup() {
    deallocateQuantizedMemory();
    initialized = false;
}

std::vector<float> QuantizedNeuralNetwork::inferenceOptimized(const std::vector<float>& input) {
    std::vector<float> output(output_size, 0.0f);
    
    if (!initialized || input.size() != input_size) {
        return output;
    }
    
    unsigned long startTime = micros();
    
    // Simplified quantized inference for demonstration
    for (size_t i = 0; i < output_size && i < input.size(); i++) {
        output[i] = input[i] * scale_factor;
    }
    
    lastInferenceTime = (micros() - startTime) / 1000.0f; // Convert to ms
    return output;
}

int32_t QuantizedNeuralNetwork::dotProductQ8(const int8_t* a, const int8_t* b, size_t len) {
    int32_t result = 0;
    for (size_t i = 0; i < len; i++) {
        result += (int32_t)a[i] * (int32_t)b[i];
    }
    return result;
}

void QuantizedNeuralNetwork::allocateQuantizedMemory() {
    size_t totalWeights = input_size * output_size * layer_count;
    weights_q8 = (int8_t*)malloc(totalWeights);
    originalModelSize = totalWeights * sizeof(float); // Original would be float
}

void QuantizedNeuralNetwork::deallocateQuantizedMemory() {
    if (weights_q8) {
        free(weights_q8);
        weights_q8 = nullptr;
    }
}

size_t QuantizedNeuralNetwork::getModelSize() const {
    return input_size * output_size * layer_count * sizeof(int8_t);
}

// FastImageProcessor implementation
FastImageProcessor::FastImageProcessor() 
    : processingBuffer(nullptr), bufferSize(0), initialized(false),
      lastProcessingTime(0), totalProcessedFrames(0) {
}

FastImageProcessor::~FastImageProcessor() {
    cleanup();
}

bool FastImageProcessor::init(size_t maxImageSize) {
    if (initialized) return true;
    
    bufferSize = maxImageSize;
    processingBuffer = (uint8_t*)malloc(bufferSize);
    
    if (!processingBuffer) {
        DEBUG_PRINTLN("ERROR: Failed to allocate image processing buffer");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("Fast Image Processor initialized");
    return true;
}

void FastImageProcessor::cleanup() {
    if (processingBuffer) {
        free(processingBuffer);
        processingBuffer = nullptr;
    }
    initialized = false;
}

bool FastImageProcessor::detectEdgesHardware(const uint8_t* image, uint8_t* edges, 
                                           uint16_t width, uint16_t height) {
    if (!initialized || !image || !edges) return false;
    
    unsigned long startTime = micros();
    
    // Simplified Sobel edge detection
    sobel3x3(image, edges, width, height);
    
    lastProcessingTime = (micros() - startTime) / 1000.0f;
    totalProcessedFrames++;
    
    return true;
}

void FastImageProcessor::rgb565ToGrayscale(const uint16_t* rgb, uint8_t* gray, size_t pixels) {
    for (size_t i = 0; i < pixels; i++) {
        uint16_t pixel = rgb[i];
        uint8_t r = (pixel >> 11) & 0x1F;
        uint8_t g = (pixel >> 5) & 0x3F;
        uint8_t b = pixel & 0x1F;
        
        // Convert to 8-bit and apply luminance formula
        r = (r * 255) / 31;
        g = (g * 255) / 63;
        b = (b * 255) / 31;
        
        gray[i] = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
    }
}

void FastImageProcessor::sobel3x3(const uint8_t* input, uint8_t* output, 
                                 uint16_t width, uint16_t height) {
    // Simplified Sobel operator
    for (uint16_t y = 1; y < height - 1; y++) {
        for (uint16_t x = 1; x < width - 1; x++) {
            int gx = 0, gy = 0;
            
            // Apply Sobel kernels
            gx += input[(y-1)*width + (x-1)] * (-1);
            gx += input[(y-1)*width + (x+1)] * (1);
            gx += input[y*width + (x-1)] * (-2);
            gx += input[y*width + (x+1)] * (2);
            gx += input[(y+1)*width + (x-1)] * (-1);
            gx += input[(y+1)*width + (x+1)] * (1);
            
            gy += input[(y-1)*width + (x-1)] * (-1);
            gy += input[(y-1)*width + x] * (-2);
            gy += input[(y-1)*width + (x+1)] * (-1);
            gy += input[(y+1)*width + (x-1)] * (1);
            gy += input[(y+1)*width + x] * (2);
            gy += input[(y+1)*width + (x+1)] * (1);
            
            int magnitude = sqrt(gx*gx + gy*gy);
            output[y*width + x] = (uint8_t)min(255, magnitude);
        }
    }
}

float FastImageProcessor::getAverageProcessingTime() const {
    return totalProcessedFrames > 0 ? lastProcessingTime : 0.0f;
}

// AdaptiveAIPipeline implementation
AdaptiveAIPipeline::AdaptiveAIPipeline() 
    : batteryThreshold(30.0f), performanceThreshold(0.8f), 
      useLightweightModel(false), initialized(false) {
}

AdaptiveAIPipeline::~AdaptiveAIPipeline() {
    cleanup();
}

bool AdaptiveAIPipeline::init() {
    if (initialized) return true;
    
    if (!primaryModel.init(784, 10, 3)) { // Example: 28x28 input, 10 classes, 3 layers
        DEBUG_PRINTLN("ERROR: Failed to initialize primary model");
        return false;
    }
    
    if (!lightweightModel.init(784, 10, 2)) { // Lighter model with 2 layers
        DEBUG_PRINTLN("ERROR: Failed to initialize lightweight model");
        return false;
    }
    
    if (!imageProcessor.init(1024 * 1024)) { // 1MB processing buffer
        DEBUG_PRINTLN("ERROR: Failed to initialize image processor");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("Adaptive AI Pipeline initialized");
    return true;
}

void AdaptiveAIPipeline::cleanup() {
    primaryModel.cleanup();
    lightweightModel.cleanup();
    imageProcessor.cleanup();
    initialized = false;
}

bool AdaptiveAIPipeline::runInference(const uint8_t* image, uint16_t width, uint16_t height,
                                     float& confidence, String& species, String& behavior) {
    if (!initialized || !image) return false;
    
    // Preprocess image
    std::vector<float> features = preprocessImage(image, width, height);
    
    // Select model based on conditions
    std::vector<float> output;
    if (useLightweightModel) {
        output = lightweightModel.inferenceOptimized(features);
    } else {
        output = primaryModel.inferenceOptimized(features);
    }
    
    // Postprocess results
    return postprocessResults(output, confidence, species, behavior);
}

void AdaptiveAIPipeline::selectModelBasedOnPower(float batteryLevel) {
    useLightweightModel = (batteryLevel < batteryThreshold);
    
    if (useLightweightModel) {
        DEBUG_PRINTLN("Switched to lightweight model for power saving");
    } else {
        DEBUG_PRINTLN("Using primary model for full accuracy");
    }
}

std::vector<float> AdaptiveAIPipeline::preprocessImage(const uint8_t* image, uint16_t width, uint16_t height) {
    // Simplified preprocessing - convert to normalized float vector
    std::vector<float> features;
    size_t totalPixels = width * height;
    features.reserve(totalPixels);
    
    for (size_t i = 0; i < totalPixels; i++) {
        features.push_back(image[i] / 255.0f); // Normalize to 0-1
    }
    
    return features;
}

bool AdaptiveAIPipeline::postprocessResults(const std::vector<float>& output, float& confidence, 
                                          String& species, String& behavior) {
    if (output.empty()) return false;
    
    // Find maximum confidence
    confidence = 0.0f;
    size_t maxIndex = 0;
    
    for (size_t i = 0; i < output.size(); i++) {
        if (output[i] > confidence) {
            confidence = output[i];
            maxIndex = i;
        }
    }
    
    // Map index to species (simplified)
    const char* speciesNames[] = {"deer", "rabbit", "fox", "bird", "bear", "squirrel"};
    if (maxIndex < sizeof(speciesNames)/sizeof(speciesNames[0])) {
        species = speciesNames[maxIndex];
    } else {
        species = "unknown";
    }
    
    behavior = "moving"; // Simplified behavior detection
    
    return confidence > 0.5f; // Minimum confidence threshold
}

AdaptiveAIPipeline::PipelineMetrics AdaptiveAIPipeline::getMetrics() const {
    PipelineMetrics metrics;
    
    metrics.averageInferenceTime = (primaryPerformance.inferenceTime + lightweightPerformance.inferenceTime) / 2.0f;
    metrics.averageAccuracy = (primaryPerformance.accuracy + lightweightPerformance.accuracy) / 2.0f;
    metrics.totalInferences = primaryPerformance.usageCount + lightweightPerformance.usageCount;
    
    return metrics;
}

// WildlifeDetectionOptimizer static methods
std::vector<WildlifeDetectionOptimizer::SpeciesProfile> WildlifeDetectionOptimizer::getCommonSpeciesProfiles() {
    std::vector<SpeciesProfile> profiles;
    
    SpeciesProfile deer;
    deer.speciesName = "deer";
    deer.sizeRange[0] = 100; deer.sizeRange[1] = 400;
    deer.colorProfile[0] = 139; deer.colorProfile[1] = 118; deer.colorProfile[2] = 102;
    deer.movementPattern = 0.3f; // Slow movement
    deer.detectionDifficulty = 0.4f;
    profiles.push_back(deer);
    
    SpeciesProfile rabbit;
    rabbit.speciesName = "rabbit";
    rabbit.sizeRange[0] = 20; rabbit.sizeRange[1] = 80;
    rabbit.colorProfile[0] = 160; rabbit.colorProfile[1] = 140; rabbit.colorProfile[2] = 120;
    rabbit.movementPattern = 0.8f; // Fast movement
    rabbit.detectionDifficulty = 0.6f;
    profiles.push_back(rabbit);
    
    return profiles;
}

float WildlifeDetectionOptimizer::calculateOptimalThreshold(const String& species, float lightLevel) {
    float baseThreshold = 0.6f;
    
    // Adjust for species difficulty
    if (species == "rabbit" || species == "squirrel") {
        baseThreshold += 0.1f; // Harder to detect small, fast animals
    }
    
    // Adjust for light conditions
    if (lightLevel < 50.0f) { // Low light
        baseThreshold -= 0.1f; // Lower threshold for difficult lighting
    }
    
    return constrain(baseThreshold, 0.3f, 0.9f);
}