/**
 * Motion Filter Module
 * 
 * Implements intelligent motion detection with weather-based filtering
 * to eliminate false positives from wind, rain, and other environmental factors.
 */

#include "motion_filter.h"
#include "config.h"
#include <Arduino.h>
#include <math.h>

#ifdef BME280_ENABLED
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#endif

namespace MotionFilter {

// Static variables
static bool initialized = false;
static unsigned long lastMotionTime = 0;
static bool motionDetected = false;
static int consecutiveMotions = 0;
static float currentWindSpeed = 0.0;
static float currentRainfall = 0.0;
static float currentTemperature = DEFAULT_TEMPERATURE;

#ifdef BME280_ENABLED
static Adafruit_BME280 bme;
static bool bmeInitialized = false;
#endif

// Function prototypes
static void updateWeatherData();
static bool isWeatherSuitable();
static void pirInterrupt();
static float estimateWindSpeed();
static bool isTemperatureStable();

/**
 * Initialize motion detection system
 */
bool init() {
    DEBUG_PRINTLN("Initializing motion filter system...");
    
    // Configure PIR sensor pin
    pinMode(PIR_PIN, INPUT);
    
    // Attach interrupt for motion detection
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirInterrupt, RISING);
    
    #ifdef BME280_ENABLED
    // Initialize BME280 weather sensor
    Wire.begin(BME280_SDA, BME280_SCL);
    bmeInitialized = bme.begin(BME280_ADDRESS);
    
    if (bmeInitialized) {
        DEBUG_PRINTLN("BME280 weather sensor initialized");
        
        // Configure BME280 settings
        bme.setSampling(Adafruit_BME280::MODE_NORMAL,     // Operating Mode
                       Adafruit_BME280::SAMPLING_X2,      // Temp. oversampling
                       Adafruit_BME280::SAMPLING_X16,     // Pressure oversampling
                       Adafruit_BME280::SAMPLING_X1,      // Humidity oversampling
                       Adafruit_BME280::FILTER_X16,       // Filtering
                       Adafruit_BME280::STANDBY_MS_500);  // Standby time
    } else {
        DEBUG_PRINTLN("Warning: BME280 initialization failed - using estimated values");
    }
    #endif
    
    initialized = true;
    DEBUG_PRINTLN("Motion filter system initialized");
    
    return true;
}

/**
 * Check if motion is currently detected
 */
bool isMotionDetected() {
    if (!initialized) return false;
    
    // Check if enough time has passed since last detection (debounce)
    if (millis() - lastMotionTime < PIR_DEBOUNCE_TIME) {
        return false;
    }
    
    // Update motion state
    bool currentMotion = motionDetected;
    motionDetected = false;  // Reset flag
    
    return currentMotion;
}

/**
 * Validate motion against weather conditions and other filters
 */
bool isValidMotion() {
    if (!WEATHER_FILTERING_ENABLED) {
        return true;  // Accept all motion if filtering disabled
    }
    
    // Update weather data
    updateWeatherData();
    
    // Check weather conditions
    if (!isWeatherSuitable()) {
        DEBUG_PRINTLN("Motion filtered: unsuitable weather conditions");
        return false;
    }
    
    // Check temperature stability for PIR compensation
    if (TEMP_COMP_ENABLED && !isTemperatureStable()) {
        DEBUG_PRINTLN("Motion filtered: temperature fluctuation");
        return false;
    }
    
    // Consecutive motion validation (reduces false positives)
    consecutiveMotions++;
    if (consecutiveMotions < 2) {
        DEBUG_PRINTLN("Motion filtered: awaiting confirmation");
        return false;
    }
    
    consecutiveMotions = 0;  // Reset counter
    return true;
}

/**
 * Get current motion detection statistics
 */
MotionStats getMotionStats() {
    MotionStats stats;
    stats.lastMotionTime = lastMotionTime;
    stats.consecutiveCount = consecutiveMotions;
    stats.windSpeed = currentWindSpeed;
    stats.rainfall = currentRainfall;
    stats.temperature = currentTemperature;
    stats.filteringEnabled = WEATHER_FILTERING_ENABLED;
    
    return stats;
}

/**
 * PIR interrupt handler
 */
static void IRAM_ATTR pirInterrupt() {
    motionDetected = true;
    lastMotionTime = millis();
}

/**
 * Update weather data from sensors
 */
static void updateWeatherData() {
    #ifdef BME280_ENABLED
    if (bmeInitialized) {
        currentTemperature = bme.readTemperature();
        float pressure = bme.readPressure() / 100.0F;  // Convert to hPa
        float humidity = bme.readHumidity();
        
        // Estimate rainfall from humidity and pressure changes
        static float lastPressure = pressure;
        float pressureDelta = pressure - lastPressure;
        lastPressure = pressure;
        
        // Simple rainfall estimation (would need calibration)
        if (humidity > 85 && pressureDelta < -0.5) {
            currentRainfall = (90 - humidity) * 0.1;  // Rough estimate
        } else {
            currentRainfall = 0.0;
        }
        
        DEBUG_PRINTF("Weather: T=%.1f°C, H=%.1f%%, P=%.1fhPa, Rain=%.1fmm/h\n",
                    currentTemperature, humidity, pressure, currentRainfall);
    } else
    #endif
    {
        // Fallback: use default values or simple estimations
        currentTemperature = 20.0;  // Default temperature
        currentRainfall = 0.0;      // Assume no rain
    }
    
    // Estimate wind speed from motion sensor sensitivity
    currentWindSpeed = estimateWindSpeed();
}

/**
 * Check if weather conditions are suitable for motion detection
 */
static bool isWeatherSuitable() {
    // Check wind threshold
    if (currentWindSpeed > WIND_THRESHOLD) {
        DEBUG_PRINTF("Wind too strong: %.1f km/h (threshold: %d)\n", 
                    currentWindSpeed, WIND_THRESHOLD);
        return false;
    }
    
    // Check rainfall threshold
    if (currentRainfall > RAIN_THRESHOLD) {
        DEBUG_PRINTF("Rain too heavy: %.1f mm/h (threshold: %.1f)\n", 
                    currentRainfall, RAIN_THRESHOLD);
        return false;
    }
    
    return true;
}

/**
 * Estimate wind speed from motion sensor behavior
 */
static float estimateWindSpeed() {
    static unsigned long lastWindCheck = 0;
    static int falseMotionCount = 0;
    static float lastEstimatedWind = 0.0;
    
    unsigned long now = millis();
    
    // Count false motions over the last minute
    if (now - lastWindCheck > 60000) {  // Every minute
        // Calculate wind speed based on false motion events
        float estimatedWind = falseMotionCount * 2.5;  // Rough correlation
        
        // Apply reasonable bounds (wind speed 0-50 km/h max for estimation)
        estimatedWind = constrain(estimatedWind, 0.0, 50.0);
        
        DEBUG_PRINTF("Wind estimation: %d false motions -> %.1f km/h\n", 
                    falseMotionCount, estimatedWind);
        
        falseMotionCount = 0;
        lastWindCheck = now;
        lastEstimatedWind = estimatedWind;
        currentWindSpeed = estimatedWind;  // Update global wind speed
        
        return estimatedWind;
    }
    
    // Increment false motion counter for consecutive quick triggers
    if (consecutiveMotions > 5) {
        falseMotionCount++;
        // Prevent excessive accumulation
        if (falseMotionCount > 20) {
            falseMotionCount = 20;
        }
    }
    
    return lastEstimatedWind;  // Return last calculated value with proper tracking
}

/**
 * Check if temperature is stable enough for reliable PIR operation
 */
static bool isTemperatureStable() {
    static float lastTemperature = currentTemperature;
    static unsigned long lastTempCheck = 0;
    
    unsigned long now = millis();
    
    // Check temperature change rate
    if (now - lastTempCheck > 30000) {  // Every 30 seconds
        float tempDelta = fabs(currentTemperature - lastTemperature);
        lastTemperature = currentTemperature;
        lastTempCheck = now;
        
        // PIR sensors can be affected by rapid temperature changes
        if (tempDelta > 3.0) {  // More than 3°C change in 30 seconds
            DEBUG_PRINTF("Rapid temperature change: %.1f°C\n", tempDelta);
            return false;
        }
    }
    
    return true;
}

/**
 * Set motion sensitivity (0-100)
 */
void setMotionSensitivity(int sensitivity) {
    if (sensitivity < 0) sensitivity = 0;
    if (sensitivity > 100) sensitivity = 100;
    
    // This would typically adjust analog threshold or digital filtering
    // For now, we'll adjust the consecutive motion requirement
    int requiredConsecutive = map(sensitivity, 0, 100, 5, 1);
    
    DEBUG_PRINTF("Motion sensitivity set to %d%% (consecutive: %d)\n", 
                sensitivity, requiredConsecutive);
}

/**
 * Enable or disable weather filtering
 */
void setWeatherFiltering(bool enabled) {
    // This would typically update a global flag
    DEBUG_PRINTF("Weather filtering %s\n", enabled ? "enabled" : "disabled");
}

/**
 * Get detailed motion filter status
 */
MotionFilterStatus getStatus() {
    MotionFilterStatus status;
    status.initialized = initialized;
    status.lastMotionTime = lastMotionTime;
    status.currentlyDetecting = (millis() - lastMotionTime) < PIR_DEBOUNCE_TIME;
    status.weatherSensorActive = false;
    
    #ifdef BME280_ENABLED
    status.weatherSensorActive = bmeInitialized;
    #endif
    
    status.filteringActive = WEATHER_FILTERING_ENABLED;
    status.consecutiveMotions = consecutiveMotions;
    
    return status;
}

/**
 * Reset motion detection statistics
 */
void resetStats() {
    consecutiveMotions = 0;
    lastMotionTime = 0;
    DEBUG_PRINTLN("Motion detection statistics reset");
}

/**
 * Cleanup motion filter resources
 */
void cleanup() {
    if (initialized) {
        detachInterrupt(digitalPinToInterrupt(PIR_PIN));
        initialized = false;
        DEBUG_PRINTLN("Motion filter system cleaned up");
    }
}

} // namespace MotionFilter