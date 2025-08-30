#ifndef CONFIG_H
#define CONFIG_H

// ===========================
// ESP32 WILDLIFE CAMERA CONFIGURATION
// Complete AI-powered wildlife monitoring system
// ===========================

// ===========================
// HARDWARE CONFIGURATION
// ===========================

// Camera Board Selection
#define CAMERA_MODEL_AI_THINKER  // AI-Thinker ESP32-CAM

// Camera Settings
#define CAMERA_FRAME_SIZE FRAMESIZE_UXGA  // 1600x1200 for high detail
#define CAMERA_JPEG_QUALITY 12            // JPEG quality: 10-63 (lower = higher quality)
#define CAMERA_FB_COUNT 2                 // Frame buffer count
#define CAMERA_PIXEL_FORMAT PIXFORMAT_JPEG

// ===========================
// MOTION DETECTION CONFIGURATION
// ===========================

// PIR Sensor Settings
#define PIR_SENSITIVITY_HIGH true         // High sensitivity for wildlife
#define PIR_DEBOUNCE_TIME 2000            // 2 second debounce
#define PIR_TRIGGER_MODE RISING           // Trigger on rising edge

// Frame Difference Settings
#define MOTION_THRESHOLD 10               // Motion detection threshold
#define MOTION_DETECTION_BLOCKS 20        // Number of blocks to analyze
#define MOTION_MIN_AREA 100               // Minimum motion area

// Hybrid Motion Detection
#define HYBRID_MOTION_ENABLED true        // Enable PIR + frame difference
#define MOTION_CONFIRMATION_TIME 1000     // Time to confirm motion (ms)

// ===========================
// ADVANCED MOTION DETECTION CONFIGURATION
// ===========================

// Multi-Zone PIR Detection
#define MULTI_ZONE_PIR_ENABLED true       // Enable multi-zone PIR system
#define MAX_PIR_ZONES 4                   // Maximum number of PIR zones
#define DEFAULT_PIR_ZONES 3               // Default number of zones to configure
#define PIR_ZONE_SENSITIVITY 0.5f         // Default zone sensitivity
#define PIR_ZONE_PRIORITY_STEP 64         // Priority step between zones

// Advanced Frame Analysis
#define ADVANCED_FRAME_ANALYSIS true      // Enable advanced frame analysis
#define BACKGROUND_SUBTRACTION_ENABLED true // Enable background subtraction
#define MOTION_VECTOR_ANALYSIS true       // Enable motion vector analysis
#define OBJECT_SIZE_FILTERING true        // Enable object size filtering
#define MIN_OBJECT_SIZE_PX 25             // Minimum object size in pixels
#define MAX_OBJECT_SIZE_PX 5000           // Maximum object size in pixels

// Background Model Settings
#define BACKGROUND_LEARNING_RATE 0.1f     // Background adaptation rate
#define BACKGROUND_UPDATE_THRESHOLD 0.3f  // Threshold for background updates
#define BACKGROUND_UPDATE_INTERVAL 5000   // Minimum time between updates (ms)
#define MAX_FRAMES_WITHOUT_BG_UPDATE 100  // Force update after this many frames

// Machine Learning False Positive Reduction
#define ML_FALSE_POSITIVE_FILTERING true  // Enable ML filtering
#define ML_LEARNING_RATE 0.05f            // ML adaptation rate
#define ML_CONFIDENCE_THRESHOLD 0.6f      // ML confidence threshold
#define ML_TRAINING_PERIOD 7200000        // Training period (2 hours in ms)
#define ML_PATTERN_MEMORY_SIZE 1000       // Number of patterns to remember

// Analytics Features
#define MOTION_ANALYTICS_ENABLED true     // Enable motion analytics
#define MOTION_HEATMAP_ENABLED false      // Disable heatmap by default (memory intensive)
#define DIRECTION_TRACKING_ENABLED true   // Enable direction tracking
#define SPEED_ESTIMATION_ENABLED true     // Enable speed estimation
#define DWELL_TIME_ANALYSIS_ENABLED true  // Enable dwell time calculation
#define HEATMAP_UPDATE_INTERVAL 10000     // Heatmap update interval (ms)
#define TRACKING_HISTORY_SIZE 100         // Number of tracking points to keep

// Enhanced Detection Modes
#define DEFAULT_ENHANCED_MODE 3           // 0=Legacy, 1=MultiZone, 2=Advanced, 3=Full, 4=Adaptive
#define ADAPTIVE_MODE_ENABLED true        // Enable automatic mode selection
#define PERFORMANCE_MONITORING true       // Monitor detection performance

// ===========================
// AI/ML CONFIGURATION
// ===========================

// TensorFlow Lite Settings
#define TFLITE_ARENA_SIZE (512 * 1024)    // 512KB tensor arena
#define MODEL_INPUT_SIZE 224               // Model input dimensions
#define INFERENCE_TIMEOUT_MS 5000          // Maximum inference time

// Species Classification
#define SPECIES_CONFIDENCE_THRESHOLD 0.7f  // Minimum confidence for species
#define MAX_SPECIES_COUNT 50               // Maximum supported species

// Behavior Analysis
#define BEHAVIOR_CONFIDENCE_THRESHOLD 0.6f // Minimum confidence for behavior
#define BEHAVIOR_HISTORY_SIZE 10           // Number of frames for temporal analysis

// Model Management
#define ADAPTIVE_MODEL_SELECTION true     // Enable adaptive model selection
#define MODEL_CALIBRATION_ENABLED true    // Enable runtime calibration
#define MODEL_CACHE_SIZE 3                 // Number of models to cache

// ===========================
// POWER MANAGEMENT CONFIGURATION
// ===========================

// Battery Monitoring
#define BATTERY_LOW_THRESHOLD 3.3f        // Low battery voltage
#define BATTERY_CRITICAL_THRESHOLD 3.0f   // Critical battery voltage
#define BATTERY_FULL_VOLTAGE 4.2f         // Full battery voltage

// Solar Charging
#define SOLAR_VOLTAGE_THRESHOLD 5.0f      // Minimum solar voltage for charging
#define CHARGING_CURRENT_LIMIT 500        // Maximum charging current (mA)

// Power Modes
#define POWER_SAVE_MODE_ENABLED true      // Enable power saving
#define DEEP_SLEEP_DURATION 60000         // Deep sleep duration (ms)
#define ADAPTIVE_DUTY_CYCLE true          // Enable adaptive duty cycling

// CPU Frequency Settings
#define MAX_CPU_FREQ_MHZ 240              // Maximum CPU frequency
#define MIN_CPU_FREQ_MHZ 80               // Minimum CPU frequency for power saving
#define BALANCED_CPU_FREQ_MHZ 160         // Balanced performance frequency

// ===========================
// DATA COLLECTION CONFIGURATION
// ===========================

// Storage Settings
#define SD_CARD_ENABLED true              // Enable SD card storage
#define IMAGE_FOLDER "/images"            // Image storage folder
#define LOG_FOLDER "/logs"                // Log storage folder
#define DATA_FOLDER "/data"               // Data storage folder

// Image Management
#define MAX_DAILY_TRIGGERS 100            // Maximum images per day
#define IMAGE_COMPRESSION_ENABLED true    // Enable image compression
#define METADATA_ENABLED true             // Save metadata with images

// Data Organization
#define SPECIES_FOLDERS_ENABLED true      // Organize by species
#define TIME_BASED_FOLDERS true           // Organize by time
#define AUTO_DELETE_OLD_DATA true         // Delete old data when storage full

// ===========================
// SYSTEM CONFIGURATION
// ===========================

// Active Hours (24-hour format)
#define ACTIVE_START_HOUR 6               // Start monitoring at 6 AM
#define ACTIVE_END_HOUR 22                // Stop monitoring at 10 PM
#define NIGHT_MODE_ENABLED true           // Enable night mode operation

// System Timing
#define STATUS_CHECK_INTERVAL 30000       // Status check every 30 seconds
#define SENSOR_UPDATE_INTERVAL 5000       // Sensor update every 5 seconds
#define AI_ANALYSIS_INTERVAL 2000         // AI analysis every 2 seconds

// Error Handling
#define MAX_RETRY_ATTEMPTS 3              // Maximum retry attempts
#define ERROR_LOG_ENABLED true            // Enable error logging
#define WATCHDOG_ENABLED true             // Enable watchdog timer

// Debug Configuration
#define DEBUG_ENABLED true                // Enable debug output
#define DEBUG_LEVEL 2                     // Debug level (0-3)
#define SERIAL_BAUDRATE 115200            // Serial communication speed

// ===========================
// NETWORK CONFIGURATION (Optional)
// ===========================

// WiFi Settings (for future expansion)
#define WIFI_ENABLED false                // WiFi disabled for power saving
#define WIFI_SSID ""                      // WiFi network name
#define WIFI_PASSWORD ""                  // WiFi password

// LoRa Settings (for future expansion)
#define LORA_ENABLED false                // LoRa disabled by default
#define LORA_FREQUENCY 915E6              // LoRa frequency (915 MHz)

// ===========================
// ADVANCED FEATURES
// ===========================

// Weather Filtering
#define WEATHER_FILTERING_ENABLED true    // Enable weather-based filtering
#define WIND_THRESHOLD 15                 // Wind speed threshold (km/h)
#define RAIN_THRESHOLD 0.5                // Rain threshold (mm/h)

// Intelligent Scheduling
#define INTELLIGENT_SCHEDULING true       // Enable AI-based scheduling
#define LEARNING_MODE_ENABLED true        // Enable continuous learning
#define ADAPTATION_ENABLED true           // Enable environmental adaptation

#endif // CONFIG_H