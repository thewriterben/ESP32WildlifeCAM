/**
 * @file motion_coordinator.cpp
 * @brief Motion detection coordinator implementation
 * @author thewriterben
 * @date 2025-08-31
 */

#include "motion_coordinator.h"
#include "../utils/logger.h"
#include "../include/config.h"

bool MotionCoordinator::initialize(int pirPin, MotionCallback callback) {
    if (m_initialized) {
        return true;
    }

    m_pirPin = pirPin;
    m_callback = callback;

    if (m_pirPin < 0) {
        Logger::error("Invalid PIR pin: %d", m_pirPin);
        return false;
    }

    // Configure PIR pin
    pinMode(m_pirPin, INPUT);
    m_lastPirState = digitalRead(m_pirPin);
    m_lastMotionTime = millis();

    Logger::info("Motion coordinator initialized on pin %d", m_pirPin);
    
    m_initialized = true;
    resetStatistics();
    
    return true;
}

void MotionCoordinator::update() {
    if (!m_initialized || !m_enabled) {
        return;
    }

    uint32_t now = millis();
    
    // Check for PIR motion
    bool motion = checkPIRMotion();
    
    if (motion) {
        // Apply debouncing
        if (now - m_lastMotionTime >= m_debounceTime) {
            // Apply environmental filtering
            float confidence = applyEnvironmentalFiltering(true);
            
            if (confidence > 0.3f) { // Minimum threshold
                m_stats.totalDetections++;
                m_stats.lastDetectionTime = now;
                
                // Update running average confidence
                m_stats.averageConfidence = (m_stats.averageConfidence * (m_stats.totalDetections - 1) + confidence) / m_stats.totalDetections;
                
                // Call motion detection callback
                if (m_callback) {
                    m_callback(confidence);
                }
                
                Logger::info("Motion detected! Confidence: %.2f", confidence);
                m_lastMotionTime = now;
            } else {
                m_stats.filteredDetections++;
                Logger::debug("Motion filtered out by environmental conditions (confidence: %.2f)", confidence);
            }
        }
    }
}

void MotionCoordinator::setEnvironmentalConditions(const EnvironmentalConditions& conditions) {
    m_conditions = conditions;
    
    Logger::debug("Environmental conditions updated: T=%.1f°C, H=%.1f%%, Light=%.1f%%, Wind=%.1fkm/h", 
                 conditions.temperature, conditions.humidity, conditions.lightLevel, conditions.windSpeed);
}

void MotionCoordinator::setEnabled(bool enable) {
    m_enabled = enable;
    Logger::info("Motion detection %s", enable ? "enabled" : "disabled");
}

void MotionCoordinator::setSensitivity(float sensitivity) {
    m_sensitivity = constrain(sensitivity, 0.0f, 1.0f);
    Logger::info("Motion sensitivity set to %.2f", m_sensitivity);
}

void MotionCoordinator::resetStatistics() {
    m_stats = MotionStats{};
    Logger::info("Motion statistics reset");
}

bool MotionCoordinator::checkPIRMotion() {
    if (m_pirPin < 0) {
        return false;
    }

    bool currentState = digitalRead(m_pirPin);
    bool motionDetected = false;

    // Detect rising edge (motion start)
    if (currentState && !m_lastPirState) {
        motionDetected = true;
    }

    m_lastPirState = currentState;
    return motionDetected;
}

float MotionCoordinator::applyEnvironmentalFiltering(bool rawDetection) {
    if (!rawDetection) {
        return 0.0f;
    }

    float confidence = calculateConfidence(rawDetection);
    
    // Apply wind filtering
    if (m_conditions.windSpeed > WIND_THRESHOLD) {
        confidence *= 0.3f; // Reduce confidence in windy conditions
        Logger::debug("Wind filtering applied: %.1fkm/h", m_conditions.windSpeed);
    }
    
    // Apply temperature compensation
    if (m_conditions.temperature < 5.0f || m_conditions.temperature > 35.0f) {
        confidence *= 0.7f; // PIR can be less reliable in extreme temperatures
        Logger::debug("Temperature compensation applied: %.1f°C", m_conditions.temperature);
    }
    
    // Apply light level filtering for false positive reduction
    if (!m_conditions.isDaytime && m_conditions.lightLevel < 10.0f) {
        confidence *= 1.2f; // Boost confidence at night when animals are more active
    } else if (m_conditions.lightLevel > 90.0f) {
        confidence *= 0.8f; // Reduce in very bright conditions (possible false triggers)
    }

    // Apply sensitivity scaling
    confidence *= m_sensitivity;

    return constrain(confidence, 0.0f, 1.0f);
}

float MotionCoordinator::calculateConfidence(bool detection) {
    if (!detection) {
        return 0.0f;
    }

    // Base confidence for PIR detection
    float confidence = 0.8f;

    // Adjust based on environmental conditions
    // This is a simplified algorithm - in production, this could use ML
    
    // Time-based adjustment (animals more active at dawn/dusk)
    uint8_t hour = (millis() / 3600000) % 24; // Simple hour calculation
    if ((hour >= 5 && hour <= 8) || (hour >= 17 && hour <= 20)) {
        confidence *= 1.1f; // Boost during active hours
    }

    // Humidity adjustment (some animals prefer certain humidity levels)
    if (m_conditions.humidity >= 40.0f && m_conditions.humidity <= 70.0f) {
        confidence *= 1.05f; // Optimal humidity range
    }

    return constrain(confidence, 0.0f, 1.0f);
}