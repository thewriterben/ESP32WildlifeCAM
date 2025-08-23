#ifndef SPECIES_DETECTOR_H
#define SPECIES_DETECTOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "../../config.h"

/**
 * @brief Advanced Species Detection and Classification System
 * 
 * Provides production-ready wildlife species detection with multi-modal AI,
 * individual recognition, and conservation-focused monitoring capabilities.
 */
class SpeciesDetector {
public:
    /**
     * @brief Species detection confidence levels
     */
    enum class ConfidenceLevel {
        VERY_LOW = 0,   // < 0.3
        LOW = 1,        // 0.3 - 0.5
        MEDIUM = 2,     // 0.5 - 0.7
        HIGH = 3,       // 0.7 - 0.9
        VERY_HIGH = 4   // > 0.9
    };

    /**
     * @brief Conservation status levels
     */
    enum class ConservationStatus {
        LEAST_CONCERN = 0,
        NEAR_THREATENED = 1,
        VULNERABLE = 2,
        ENDANGERED = 3,
        CRITICALLY_ENDANGERED = 4,
        EXTINCT_IN_WILD = 5,
        UNKNOWN = 6
    };

    /**
     * @brief Detection method types
     */
    enum class DetectionMethod {
        VISUAL_AI = 0,
        AUDIO_AI = 1,
        MOTION_PATTERN = 2,
        THERMAL = 3,
        MULTIMODAL = 4
    };

    /**
     * @brief Species detection result
     */
    struct DetectionResult {
        String species_name;
        String common_name;
        String scientific_name;
        float confidence_score;
        ConfidenceLevel confidence_level;
        ConservationStatus conservation_status;
        DetectionMethod detection_method;
        
        // Bounding box for visual detections
        struct BoundingBox {
            uint16_t x, y, width, height;
        } bounding_box;
        
        // Individual identification
        String individual_id;
        float individual_confidence;
        
        // Additional metadata
        uint32_t timestamp;
        String location_id;
        float environmental_context_score;
        std::vector<String> behavioral_tags;
        
        // Quality metrics
        float image_quality_score;
        float detection_clarity;
        bool verified_detection;
    };

    /**
     * @brief Species profile information
     */
    struct SpeciesProfile {
        String species_name;
        String scientific_name;
        ConservationStatus status;
        
        // Physical characteristics
        float avg_size_cm;
        float weight_range_kg_min;
        float weight_range_kg_max;
        std::vector<String> color_patterns;
        std::vector<String> distinguishing_features;
        
        // Behavioral characteristics
        std::vector<int> active_hours; // 24-hour format
        std::vector<String> preferred_habitats;
        bool is_social_species;
        bool is_territorial;
        bool is_migratory;
        
        // Detection characteristics
        float base_detection_difficulty;
        float motion_signature_uniqueness;
        std::vector<String> common_confusion_species;
        
        // Conservation information
        float population_estimate;
        String threat_level_description;
        bool requires_special_monitoring;
        std::vector<String> conservation_actions;
    };

    /**
     * @brief Detection statistics
     */
    struct DetectionStatistics {
        uint32_t total_detections;
        uint32_t unique_species_count;
        uint32_t high_confidence_detections;
        uint32_t conservation_alerts_triggered;
        uint32_t individual_identifications;
        
        std::map<String, uint32_t> species_counts;
        std::map<ConservationStatus, uint32_t> status_distribution;
        std::map<ConfidenceLevel, uint32_t> confidence_distribution;
        
        float average_confidence;
        float detection_accuracy_estimate;
        uint32_t last_detection_timestamp;
    };

    /**
     * @brief Initialize species detector
     * @param enable_ai Enable AI-based detection
     * @param model_path Path to AI model files
     * @return true if initialization successful
     */
    static bool initialize(bool enable_ai = true, const String& model_path = "/models/");

    /**
     * @brief Detect species in image
     * @param image_data Pointer to image data
     * @param image_size Size of image data in bytes
     * @param image_format Image format (JPEG, etc.)
     * @return Vector of detection results
     */
    static std::vector<DetectionResult> detectSpeciesInImage(const uint8_t* image_data, size_t image_size, const String& image_format = "JPEG");

    /**
     * @brief Detect species from audio data
     * @param audio_data Pointer to audio data
     * @param audio_size Size of audio data in bytes
     * @param sample_rate Audio sample rate
     * @return Vector of detection results
     */
    static std::vector<DetectionResult> detectSpeciesInAudio(const uint8_t* audio_data, size_t audio_size, uint32_t sample_rate);

    /**
     * @brief Perform multimodal species detection
     * @param image_data Image data
     * @param image_size Image size
     * @param audio_data Audio data (optional)
     * @param audio_size Audio size
     * @param motion_data Motion pattern data (optional)
     * @return Enhanced detection results combining multiple modalities
     */
    static std::vector<DetectionResult> detectSpeciesMultimodal(
        const uint8_t* image_data, size_t image_size,
        const uint8_t* audio_data = nullptr, size_t audio_size = 0,
        const String& motion_data = ""
    );

    /**
     * @brief Identify individual animal from detection
     * @param detection Detection result to enhance with individual ID
     * @param reference_database Path to individual reference database
     * @return Updated detection result with individual identification
     */
    static DetectionResult identifyIndividual(const DetectionResult& detection, const String& reference_database = "/individuals/");

    /**
     * @brief Get species profile information
     * @param species_name Name of species to lookup
     * @return Species profile data
     */
    static SpeciesProfile getSpeciesProfile(const String& species_name);

    /**
     * @brief Update species profile database
     * @param profile Species profile to add or update
     * @return true if update successful
     */
    static bool updateSpeciesProfile(const SpeciesProfile& profile);

    /**
     * @brief Get detection statistics
     * @param days_back Number of days of statistics to include
     * @return Detection statistics summary
     */
    static DetectionStatistics getDetectionStatistics(uint16_t days_back = 30);

    /**
     * @brief Set detection confidence threshold
     * @param threshold Minimum confidence level for valid detections (0.0 - 1.0)
     * @param conservation_threshold Lower threshold for conservation-status species
     */
    static void setConfidenceThreshold(float threshold, float conservation_threshold = 0.0f);

    /**
     * @brief Enable/disable specific detection methods
     * @param method Detection method to configure
     * @param enabled Enable/disable the method
     */
    static void setDetectionMethodEnabled(DetectionMethod method, bool enabled);

    /**
     * @brief Add custom species to detection database
     * @param profile Species profile for custom species
     * @param model_data AI model data for species (optional)
     * @return true if species added successfully
     */
    static bool addCustomSpecies(const SpeciesProfile& profile, const String& model_data = "");

    /**
     * @brief Process continuous detection (call in main loop)
     */
    static void process();

    /**
     * @brief Get list of detectable species
     * @return Vector of species names that can be detected
     */
    static std::vector<String> getDetectableSpecies();

    /**
     * @brief Validate detection result manually
     * @param detection_id Unique ID of detection to validate
     * @param is_valid True if detection is confirmed valid
     * @param corrected_species Corrected species name if misidentified
     * @return true if validation recorded successfully
     */
    static bool validateDetection(const String& detection_id, bool is_valid, const String& corrected_species = "");

    /**
     * @brief Export detection data for research
     * @param start_timestamp Start time for data export
     * @param end_timestamp End time for data export
     * @param include_images Include image data in export
     * @return JSON object with detection data
     */
    static DynamicJsonDocument exportDetectionData(uint32_t start_timestamp, uint32_t end_timestamp, bool include_images = false);

    /**
     * @brief Train detection model with new data
     * @param training_data Vector of validated detection results
     * @return true if training successful
     */
    static bool trainModel(const std::vector<DetectionResult>& training_data);

    /**
     * @brief Get model performance metrics
     * @return JSON object with model performance data
     */
    static DynamicJsonDocument getModelPerformance();

    /**
     * @brief Set location context for detections
     * @param location_id Unique location identifier
     * @param habitat_type Primary habitat type
     * @param expected_species List of species expected in this location
     */
    static void setLocationContext(const String& location_id, const String& habitat_type, const std::vector<String>& expected_species);

private:
    static bool initialized_;
    static bool ai_enabled_;
    static String model_path_;
    static float confidence_threshold_;
    static float conservation_threshold_;
    static std::map<DetectionMethod, bool> enabled_methods_;
    static std::map<String, SpeciesProfile> species_database_;
    static std::vector<DetectionResult> detection_history_;
    static DetectionStatistics current_statistics_;
    static String current_location_id_;
    static std::vector<String> location_expected_species_;
    
    // AI model interfaces
    static std::vector<DetectionResult> runVisualAI(const uint8_t* image_data, size_t image_size);
    static std::vector<DetectionResult> runAudioAI(const uint8_t* audio_data, size_t audio_size, uint32_t sample_rate);
    static DetectionResult enhanceWithMotionPattern(const DetectionResult& detection, const String& motion_data);
    
    // Individual identification
    static String calculateAnimalFingerprint(const DetectionResult& detection);
    static float compareIndividuals(const String& fingerprint1, const String& fingerprint2);
    
    // Confidence and validation
    static ConfidenceLevel scoreToConfidenceLevel(float score);
    static float calculateEnvironmentalContext(const DetectionResult& detection);
    static bool isPlausibleDetection(const DetectionResult& detection);
    
    // Database management
    static bool loadSpeciesDatabase();
    static bool saveSpeciesDatabase();
    static void updateStatistics(const DetectionResult& detection);
    static void pruneDetectionHistory();
    
    // Utility methods
    static String generateDetectionId();
    static String conservationStatusToString(ConservationStatus status);
    static String detectionMethodToString(DetectionMethod method);
    static bool isConservationSpecies(const String& species_name);
};

#endif // SPECIES_DETECTOR_H