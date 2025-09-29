/**
 * @file ai_bridge.cpp
 * @brief AI integration bridge functions for camera handler
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 * 
 * This file provides bridge functions to connect the camera handler
 * with the AI inference system and storage manager.
 */

#include "../camera/camera_handler.h"
#include "../../firmware/src/ai/ai_common.h"
#include "../../firmware/src/ai/tinyml/inference_engine.h"
#include "../data/storage_manager.h"
#include <esp_camera.h>

// Global inference engine instance
static InferenceEngine* g_inference_engine = nullptr;

/**
 * Initialize the AI inference engine
 */
bool initializeInferenceEngine() {
    if (g_inference_engine != nullptr) {
        return true; // Already initialized
    }
    
    Serial.println("AI Bridge: Initializing inference engine...");
    
    g_inference_engine = new InferenceEngine();
    if (!g_inference_engine) {
        Serial.println("AI Bridge: Failed to create inference engine");
        return false;
    }
    
    if (!g_inference_engine->init()) {
        Serial.println("AI Bridge: Failed to initialize inference engine");
        delete g_inference_engine;
        g_inference_engine = nullptr;
        return false;
    }
    
    Serial.println("AI Bridge: Inference engine initialized successfully");
    return true;
}

/**
 * Run wildlife inference on captured frame
 */
AIResult runWildlifeInference(const CameraFrame& frame, ModelType model) {
    AIResult result;
    result.confidence = 0.0f;
    result.species = SpeciesType::UNKNOWN;
    result.detected = false;
    result.error_message = "";
    
    if (!g_inference_engine) {
        Serial.println("AI Bridge: Inference engine not initialized");
        result.error_message = "AI engine not initialized";
        return result;
    }
    
    Serial.printf("AI Bridge: Running inference on %dx%d frame (%d bytes)\n", 
                  frame.width, frame.height, frame.length);
    
    try {
        // Run inference using the inference engine
        result = g_inference_engine->runInference(frame, model);
        
        Serial.printf("AI Bridge: Inference complete - Confidence: %.2f, Species: %d\n", 
                      result.confidence, (int)result.species);
        
    } catch (...) {
        Serial.println("AI Bridge: Exception during inference");
        result.error_message = "Inference exception";
        result.detected = false;
    }
    
    return result;
}

/**
 * Initialize storage manager
 */
bool initializeStorageManager() {
    Serial.println("AI Bridge: Initializing storage manager...");
    
    // Use the existing StorageManager static initialization
    if (StorageManager::init()) {
        Serial.println("AI Bridge: Storage manager initialized successfully");
        return true;
    } else {
        Serial.println("AI Bridge: Failed to initialize storage manager");
        return false;
    }
}

/**
 * Save image to storage using storage manager
 */
bool saveImageToStorage(camera_fb_t* fb, const String& filename) {
    if (!fb) {
        Serial.println("AI Bridge: Cannot save null frame buffer");
        return false;
    }
    
    Serial.printf("AI Bridge: Saving image %s (%d bytes)\n", filename.c_str(), fb->len);
    
    // Use the existing StorageManager to save the image
    return StorageManager::saveImage(fb, filename);
}

/**
 * Cleanup AI bridge resources
 */
void cleanupAIBridge() {
    if (g_inference_engine) {
        delete g_inference_engine;
        g_inference_engine = nullptr;
        Serial.println("AI Bridge: Inference engine cleaned up");
    }
}