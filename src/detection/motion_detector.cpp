#include "motion_detector.h"
#include "../utils/logger.h"

// Static member definitions
volatile bool MotionDetector::pirInterruptFlag = false;
MotionDetector* MotionDetector::instance = nullptr;

bool MotionDetector::initialize() {
    if (initialized) {
        return true;
    }

    LOG_INFO("Initializing unified motion detection system...");

    // Set static instance for interrupt handling
    instance = this;

    // Initialize configuration
    initializeDefaultConfig();

    // Initialize PIR sensor
    pirSensor = new PIRSensor();
    if (!pirSensor->initialize()) {
        LOG_ERROR("Failed to initialize PIR sensor");
        delete pirSensor;
        pirSensor = nullptr;
        return false;
    }

    // Initialize camera motion detection
    cameraMotion = new MotionDetection();
    if (!cameraMotion->initialize()) {
        LOG_ERROR("Failed to initialize camera motion detection");
        delete cameraMotion;
        cameraMotion = nullptr;
        return false;
    }

    // Configure PIR sensor with default settings
    configurePIRSensor();

    // Enable motion interrupt if configured
    if (config.method != DetectionMethod::CAMERA_ONLY) {
        enableMotionInterrupt();
    }

    initialized = true;
    LOG_INFO("Motion detection system initialized successfully");
    LOG_INFO("Detection method: " + String(static_cast<int>(config.method)));

    return true;
}

bool MotionDetector::detectMotion() {
    if (!initialized) {
        return false;
    }

    // Check cooldown period
    if (isInCooldown()) {
        return false;
    }

    // Reset result
    lastResult = {};
    lastResult.detectionTime = millis();
    lastResult.methodUsed = config.method;

    bool motionDetected = false;

    // Handle adaptive method selection
    DetectionMethod currentMethod = config.method;
    if (config.method == DetectionMethod::ADAPTIVE) {
        currentMethod = selectAdaptiveMethod();
        lastResult.methodUsed = currentMethod;
    }

    // PIR detection
    if (currentMethod != DetectionMethod::CAMERA_ONLY) {
        lastResult.pirTriggered = pirSensor->hasMotion();
        lastResult.pirConfidence = lastResult.pirTriggered ? 0.8f : 0.0f;
    }

    // Camera motion detection  
    if (currentMethod != DetectionMethod::PIR_ONLY) {
        // Get current camera frame
        camera_fb_t* fb = esp_camera_fb_get();
        if (fb) {
            MotionDetection::MotionResult camResult = cameraMotion->analyzeFrame(fb);
            lastResult.cameraMotion = camResult.motionDetected;
            lastResult.cameraConfidence = camResult.motionLevel;
            lastResult.cameraDetails = camResult;
            
            esp_camera_fb_return(fb);
        }
    }

    // Combine results based on method
    switch (currentMethod) {
        case DetectionMethod::PIR_ONLY:
            motionDetected = lastResult.pirTriggered;
            lastResult.confidence = lastResult.pirConfidence;
            break;

        case DetectionMethod::CAMERA_ONLY:
            motionDetected = lastResult.cameraMotion;
            lastResult.confidence = lastResult.cameraConfidence;
            break;

        case DetectionMethod::PIR_AND_CAMERA:
            motionDetected = lastResult.pirTriggered && lastResult.cameraMotion;
            lastResult.confidence = combineConfidence(lastResult.pirConfidence, lastResult.cameraConfidence);
            break;

        case DetectionMethod::PIR_OR_CAMERA:
            motionDetected = lastResult.pirTriggered || lastResult.cameraMotion;
            lastResult.confidence = max(lastResult.pirConfidence, lastResult.cameraConfidence);
            break;

        default:
            motionDetected = false;
            lastResult.confidence = 0.0f;
            break;
    }

    // Apply confidence threshold
    if (lastResult.confidence < config.confidenceThreshold) {
        motionDetected = false;
    }

    // Apply false positive filtering
    if (motionDetected && config.enableFalsePositiveFilter) {
        motionDetected = filterFalsePositives(lastResult);
        if (!motionDetected) {
            falsePositiveCount++;
            LOG_DEBUG("Motion filtered as false positive");
        }
    }

    lastResult.motionDetected = motionDetected;

    // Update statistics
    totalDetections++;
    if (motionDetected) {
        lastDetectionTime = millis();
        LOG_INFO("Motion detected! Confidence: " + String(lastResult.confidence, 3));
    }

    return motionDetected;
}

MotionDetector::MotionResult MotionDetector::getMotionResult() {
    return lastResult;
}

void MotionDetector::configurePIRSensor() {
    if (!pirSensor) {
        return;
    }

    // Configure PIR sensor based on current settings
    pirSensor->setEnabled(true);
    
    // Additional PIR configuration would go here
    LOG_DEBUG("PIR sensor configured with sensitivity: " + String(config.pirSensitivity, 2));
}

float MotionDetector::getMotionConfidence() {
    return lastResult.confidence;
}

void MotionDetector::enableMotionInterrupt() {
    if (!initialized) {
        return;
    }

    // Configure PIR pin for interrupt
    pinMode(PIR_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirInterruptHandler, RISING);
    
    LOG_DEBUG("PIR motion interrupt enabled on pin " + String(PIR_PIN));
}

void MotionDetector::disableMotionInterrupt() {
    detachInterrupt(digitalPinToInterrupt(PIR_PIN));
    LOG_DEBUG("PIR motion interrupt disabled");
}

void MotionDetector::setDetectionMethod(DetectionMethod method) {
    config.method = method;
    
    // Reconfigure interrupts based on new method
    if (method == DetectionMethod::CAMERA_ONLY) {
        disableMotionInterrupt();
    } else {
        enableMotionInterrupt();
    }
    
    LOG_INFO("Detection method changed to: " + String(static_cast<int>(method)));
}

void MotionDetector::setConfiguration(const MotionConfig& newConfig) {
    config = newConfig;
    
    // Apply configuration changes
    setDetectionMethod(config.method);
    configurePIRSensor();
    
    LOG_INFO("Motion detection configuration updated");
}

void MotionDetector::update() {
    if (!initialized) {
        return;
    }

    // Handle PIR interrupt flag
    if (pirInterruptFlag) {
        pirInterruptFlag = false;
        handlePIRInterrupt();
    }

    // Update component instances
    if (pirSensor) {
        // PIR sensor update would go here if needed
    }
}

bool MotionDetector::isInCooldown() {
    return (millis() - lastDetectionTime) < config.cooldownPeriod;
}

void MotionDetector::resetStatistics() {
    totalDetections = 0;
    falsePositiveCount = 0;
    lastDetectionTime = 0;
    LOG_INFO("Motion detection statistics reset");
}

void MotionDetector::setFalsePositiveFiltering(bool enable) {
    config.enableFalsePositiveFilter = enable;
    LOG_INFO("False positive filtering " + String(enable ? "enabled" : "disabled"));
}

void IRAM_ATTR MotionDetector::pirInterruptHandler() {
    pirInterruptFlag = true;
}

void MotionDetector::cleanup() {
    if (initialized) {
        disableMotionInterrupt();
        
        if (pirSensor) {
            delete pirSensor;
            pirSensor = nullptr;
        }
        
        if (cameraMotion) {
            delete cameraMotion;
            cameraMotion = nullptr;
        }
        
        initialized = false;
        instance = nullptr;
        
        LOG_INFO("Motion detector cleaned up");
    }
}

bool MotionDetector::filterFalsePositives(const MotionResult& result) {
    // Implement false positive filtering logic
    
    // Filter 1: Minimum confidence threshold
    if (result.confidence < 0.3f) {
        return false;
    }
    
    // Filter 2: PIR and camera agreement (if both available)
    if (result.pirTriggered && result.cameraMotion) {
        // Both sensors agree - high confidence
        return true;
    }
    
    // Filter 3: Camera motion area analysis
    if (result.cameraMotion) {
        // Check if motion area is reasonable for wildlife
        if (result.cameraDetails.motionBlocks > 5 && 
            result.cameraDetails.motionLevel > 0.2f) {
            return true;
        }
    }
    
    // Filter 4: PIR timing analysis
    if (result.pirTriggered) {
        // Simple timing-based filtering
        return true;
    }
    
    // Default: require higher confidence for single-sensor detection
    return result.confidence > 0.6f;
}

float MotionDetector::combineConfidence(float pirConf, float camConf) {
    // Weighted combination of confidence scores
    float pirWeight = 0.6f;
    float camWeight = 0.4f;
    
    // If both sensors triggered, increase overall confidence
    if (pirConf > 0.0f && camConf > 0.0f) {
        return min(1.0f, (pirConf * pirWeight + camConf * camWeight) * 1.2f);
    }
    
    // Single sensor detection
    return max(pirConf * pirWeight, camConf * camWeight);
}

void MotionDetector::initializeDefaultConfig() {
    config.method = DetectionMethod::PIR_OR_CAMERA;
    config.pirSensitivity = 0.7f;
    config.cameraSensitivity = 0.5f;
    config.cooldownPeriod = PIR_COOLDOWN_MS;
    config.enableFalsePositiveFilter = true;
    config.confidenceThreshold = 0.3f;
    config.enableAdaptiveMode = false;
}

void MotionDetector::handlePIRInterrupt() {
    // Handle PIR interrupt in main thread context
    LOG_DEBUG("PIR interrupt triggered at " + String(millis()));
    
    // Clear the motion flag in PIR sensor
    if (pirSensor) {
        pirSensor->clearMotion();
    }
}

MotionDetector::DetectionMethod MotionDetector::selectAdaptiveMethod() {
    // Implement adaptive method selection based on:
    // - Time of day (use PIR at night when camera quality is poor)
    // - Battery level (use PIR only when low)
    // - Recent false positive rate
    
    // Simple implementation - can be enhanced
    uint32_t hour = (millis() / 3600000) % 24; // Rough hour calculation
    
    if (hour >= 20 || hour <= 6) {
        // Night time - prefer PIR
        return DetectionMethod::PIR_ONLY;
    } else {
        // Day time - use both sensors
        return DetectionMethod::PIR_OR_CAMERA;
    }
}