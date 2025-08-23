#include "config_manager.h"
#include <HTTPClient.h>
#include <WiFi.h>

// Static member initialization
DynamicJsonDocument ConfigManager::config_data_(4096);
String ConfigManager::current_config_path_ = "/config.json";
bool ConfigManager::remote_sync_enabled_ = false;
uint32_t ConfigManager::last_sync_time_ = 0;
uint32_t ConfigManager::last_backup_time_ = 0;
bool ConfigManager::auto_backup_enabled_ = true;
uint8_t ConfigManager::backup_interval_hours_ = 24;
void (*ConfigManager::change_callback_)(ConfigSection, const String&, const String&, const String&) = nullptr;
bool ConfigManager::initialized_ = false;

bool ConfigManager::initialize(bool enable_remote_sync) {
    if (initialized_) {
        return true;
    }

    // Initialize SPIFFS for configuration storage
    if (!SPIFFS.begin(true)) {
        Serial.println("ERROR: Failed to initialize SPIFFS for ConfigManager");
        return false;
    }

    remote_sync_enabled_ = enable_remote_sync;

    // Load default configuration first
    config_data_ = getDefaultConfiguration();

    // Try to load existing configuration
    if (SPIFFS.exists(current_config_path_)) {
        if (!loadConfiguration(current_config_path_)) {
            Serial.println("WARNING: Failed to load existing configuration, using defaults");
        }
    } else {
        // Save default configuration
        saveConfiguration(current_config_path_, false);
    }

    Serial.println("ConfigManager initialized successfully");
    initialized_ = true;
    return true;
}

bool ConfigManager::loadConfiguration(const String& config_path) {
    if (!SPIFFS.exists(config_path)) {
        Serial.printf("Configuration file not found: %s\n", config_path.c_str());
        return false;
    }

    File config_file = SPIFFS.open(config_path, "r");
    if (!config_file) {
        Serial.printf("Failed to open configuration file: %s\n", config_path.c_str());
        return false;
    }

    DynamicJsonDocument temp_config(4096);
    DeserializationError error = deserializeJson(temp_config, config_file);
    config_file.close();

    if (error) {
        Serial.printf("Failed to parse configuration JSON: %s\n", error.c_str());
        return false;
    }

    // Validate configuration before applying
    ValidationResult validation = validateConfiguration(temp_config);
    if (!validation.valid) {
        Serial.printf("Configuration validation failed: %s\n", validation.error_message.c_str());
        return false;
    }

    // Apply configuration
    config_data_ = temp_config;
    current_config_path_ = config_path;

    Serial.printf("Configuration loaded successfully from: %s\n", config_path.c_str());
    
    // Print warnings if any
    for (const String& warning : validation.warnings) {
        Serial.printf("CONFIG WARNING: %s\n", warning.c_str());
    }

    return true;
}

bool ConfigManager::saveConfiguration(const String& config_path, bool create_backup) {
    if (!initialized_) {
        return false;
    }

    // Create backup if requested and file exists
    if (create_backup && SPIFFS.exists(config_path)) {
        String backup_path = config_path + ".backup." + String(millis());
        if (SPIFFS.rename(config_path, backup_path)) {
            Serial.printf("Configuration backup created: %s\n", backup_path.c_str());
        }
    }

    File config_file = SPIFFS.open(config_path, "w");
    if (!config_file) {
        Serial.printf("Failed to open configuration file for writing: %s\n", config_path.c_str());
        return false;
    }

    // Add metadata to configuration
    config_data_["metadata"]["version"] = "1.0";
    config_data_["metadata"]["timestamp"] = String(millis());
    config_data_["metadata"]["device_id"] = generateDeviceFingerprint();
    config_data_["metadata"]["checksum"] = generateConfigChecksum(config_data_);

    size_t bytes_written = serializeJsonPretty(config_data_, config_file);
    config_file.close();

    if (bytes_written > 0) {
        Serial.printf("Configuration saved successfully to: %s (%d bytes)\n", config_path.c_str(), bytes_written);
        current_config_path_ = config_path;
        return true;
    } else {
        Serial.printf("Failed to write configuration to: %s\n", config_path.c_str());
        return false;
    }
}

String ConfigManager::getParameter(ConfigSection section, const String& key, const String& default_value) {
    if (!initialized_) {
        return default_value;
    }

    String section_name = sectionToString(section);
    
    if (config_data_[section_name].containsKey(key)) {
        return config_data_[section_name][key].as<String>();
    }
    
    return default_value;
}

bool ConfigManager::setParameter(ConfigSection section, const String& key, const String& value, bool immediate_save) {
    if (!initialized_) {
        return false;
    }

    String section_name = sectionToString(section);
    String old_value = getParameter(section, key, "");

    // Validate parameter value
    if (!validateParameterValue(section, key, value)) {
        Serial.printf("Invalid parameter value: %s.%s = %s\n", section_name.c_str(), key.c_str(), value.c_str());
        return false;
    }

    // Set the parameter
    config_data_[section_name][key] = value;

    // Notify change callback
    notifyConfigChange(section, key, old_value, value);

    // Save immediately if requested
    if (immediate_save) {
        saveConfiguration(current_config_path_, true);
    }

    Serial.printf("Parameter set: %s.%s = %s\n", section_name.c_str(), key.c_str(), value.c_str());
    return true;
}

int ConfigManager::getIntParameter(ConfigSection section, const String& key, int default_value) {
    String value = getParameter(section, key, String(default_value));
    return value.toInt();
}

float ConfigManager::getFloatParameter(ConfigSection section, const String& key, float default_value) {
    String value = getParameter(section, key, String(default_value, 2));
    return value.toFloat();
}

bool ConfigManager::getBoolParameter(ConfigSection section, const String& key, bool default_value) {
    String value = getParameter(section, key, default_value ? "true" : "false");
    return (value.equalsIgnoreCase("true") || value.equals("1"));
}

ConfigManager::ValidationResult ConfigManager::validateConfiguration(const DynamicJsonDocument& config_json) {
    ValidationResult result;
    result.valid = true;
    result.compatibility_score = 1.0f;

    // Basic structure validation
    if (!config_json.containsKey("camera")) {
        result.valid = false;
        result.error_message = "Missing camera configuration section";
        return result;
    }

    if (!config_json.containsKey("motion_detection")) {
        result.warnings.push_back("Missing motion detection configuration");
        result.compatibility_score -= 0.1f;
    }

    if (!config_json.containsKey("power_management")) {
        result.warnings.push_back("Missing power management configuration");
        result.compatibility_score -= 0.1f;
    }

    // Validate camera parameters
    JsonObject camera = config_json["camera"];
    if (camera.containsKey("frame_size")) {
        int frame_size = camera["frame_size"];
        if (frame_size < 0 || frame_size > 20) {
            result.valid = false;
            result.error_message = "Invalid camera frame size";
            return result;
        }
    }

    if (camera.containsKey("jpeg_quality")) {
        int quality = camera["jpeg_quality"];
        if (quality < 4 || quality > 63) {
            result.warnings.push_back("JPEG quality should be between 4 and 63");
            result.compatibility_score -= 0.05f;
        }
    }

    // Validate power management parameters
    if (config_json.containsKey("power_management")) {
        JsonObject power = config_json["power_management"];
        
        if (power.containsKey("battery_low_threshold")) {
            float threshold = power["battery_low_threshold"];
            if (threshold < 2.0f || threshold > 4.2f) {
                result.warnings.push_back("Battery threshold should be between 2.0V and 4.2V");
                result.compatibility_score -= 0.05f;
            }
        }
    }

    // Additional validation rules can be added here

    return result;
}

bool ConfigManager::applyProfile(const String& profile_name) {
    String profile_path = "/profiles/" + profile_name + ".json";
    
    if (!SPIFFS.exists(profile_path)) {
        Serial.printf("Profile not found: %s\n", profile_name.c_str());
        return false;
    }

    File profile_file = SPIFFS.open(profile_path, "r");
    if (!profile_file) {
        Serial.printf("Failed to open profile: %s\n", profile_name.c_str());
        return false;
    }

    DynamicJsonDocument profile_config(2048);
    DeserializationError error = deserializeJson(profile_config, profile_file);
    profile_file.close();

    if (error) {
        Serial.printf("Failed to parse profile JSON: %s\n", error.c_str());
        return false;
    }

    // Apply profile parameters to current configuration
    if (profile_config.containsKey("parameters")) {
        JsonObject params = profile_config["parameters"];
        
        for (JsonPair param : params) {
            String key = param.key().c_str();
            String value = param.value().as<String>();
            
            // Parse section.key format
            int dot_index = key.indexOf('.');
            if (dot_index > 0) {
                String section_str = key.substring(0, dot_index);
                String param_key = key.substring(dot_index + 1);
                
                ConfigSection section = stringToSection(section_str);
                setParameter(section, param_key, value, false);
            }
        }
    }

    saveConfiguration(current_config_path_, true);
    Serial.printf("Applied configuration profile: %s\n", profile_name.c_str());
    return true;
}

bool ConfigManager::createProfile(const ConfigProfile& profile) {
    // Ensure profiles directory exists
    if (!SPIFFS.exists("/profiles")) {
        SPIFFS.mkdir("/profiles");
    }

    String profile_path = "/profiles/" + profile.name + ".json";
    
    DynamicJsonDocument profile_doc(2048);
    profile_doc["name"] = profile.name;
    profile_doc["description"] = profile.description;
    profile_doc["primary_section"] = sectionToString(profile.primary_section);
    profile_doc["requires_restart"] = profile.requires_restart;
    profile_doc["priority"] = profile.priority;
    
    JsonObject params = profile_doc.createNestedObject("parameters");
    for (const auto& param : profile.parameters) {
        params[param.first] = param.second;
    }

    File profile_file = SPIFFS.open(profile_path, "w");
    if (!profile_file) {
        Serial.printf("Failed to create profile file: %s\n", profile_path.c_str());
        return false;
    }

    size_t bytes_written = serializeJsonPretty(profile_doc, profile_file);
    profile_file.close();

    if (bytes_written > 0) {
        Serial.printf("Profile created: %s\n", profile.name.c_str());
        return true;
    } else {
        Serial.printf("Failed to write profile: %s\n", profile.name.c_str());
        return false;
    }
}

std::vector<String> ConfigManager::getAvailableProfiles() {
    std::vector<String> profiles;
    
    File root = SPIFFS.open("/profiles");
    if (!root || !root.isDirectory()) {
        return profiles;
    }

    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String filename = file.name();
            if (filename.endsWith(".json")) {
                String profile_name = filename.substring(0, filename.lastIndexOf('.'));
                profiles.push_back(profile_name);
            }
        }
        file = root.openNextFile();
    }

    return profiles;
}

bool ConfigManager::syncWithRemote(const String& server_url) {
    if (!remote_sync_enabled_ || !WiFi.isConnected()) {
        return false;
    }

    String url = server_url.isEmpty() ? "https://config.wildlifecam.org/api/sync" : server_url;
    url += "?device_id=" + generateDeviceFingerprint();

    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("User-Agent", "ESP32WildlifeCAM-Config/1.0");

    // Send current configuration
    String current_config = exportConfiguration(true);
    int httpCode = http.POST(current_config);

    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        DynamicJsonDocument response_doc(4096);
        
        if (deserializeJson(response_doc, response) == DeserializationError::Ok) {
            if (response_doc["update_required"].as<bool>()) {
                String remote_config = response_doc["configuration"].as<String>();
                
                if (importConfiguration(remote_config, true)) {
                    last_sync_time_ = millis() / 1000;
                    Serial.println("Configuration synchronized with remote server");
                    http.end();
                    return true;
                }
            }
        }
    }

    http.end();
    return false;
}

bool ConfigManager::checkForUpdates() {
    if (!remote_sync_enabled_ || !WiFi.isConnected()) {
        return false;
    }

    String url = "https://config.wildlifecam.org/api/check";
    url += "?device_id=" + generateDeviceFingerprint();
    url += "&version=" + String(config_data_["metadata"]["checksum"].as<String>());

    HTTPClient http;
    http.begin(url);
    http.addHeader("User-Agent", "ESP32WildlifeCAM-Config/1.0");

    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        DynamicJsonDocument response_doc(1024);
        
        if (deserializeJson(response_doc, response) == DeserializationError::Ok) {
            bool updates_available = response_doc["update_available"].as<bool>();
            http.end();
            return updates_available;
        }
    }

    http.end();
    return false;
}

std::vector<ConfigManager::ConfigVersion> ConfigManager::getVersionHistory() {
    std::vector<ConfigVersion> versions;
    
    File root = SPIFFS.open("/");
    if (!root || !root.isDirectory()) {
        return versions;
    }

    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String filename = file.name();
            if (filename.startsWith("config.json.backup.")) {
                ConfigVersion version;
                version.version = filename.substring(19); // Remove "config.json.backup."
                version.timestamp = version.version; // Simplified
                versions.push_back(version);
            }
        }
        file = root.openNextFile();
    }

    return versions;
}

String ConfigManager::exportConfiguration(bool include_metadata) {
    if (!initialized_) {
        return "{}";
    }

    DynamicJsonDocument export_doc(4096);
    
    if (include_metadata) {
        export_doc = config_data_;
    } else {
        // Copy without metadata
        for (JsonPair kv : config_data_.as<JsonObject>()) {
            if (String(kv.key().c_str()) != "metadata") {
                export_doc[kv.key()] = kv.value();
            }
        }
    }

    String result;
    serializeJsonPretty(export_doc, result);
    return result;
}

void ConfigManager::process() {
    if (!initialized_) {
        return;
    }

    uint32_t current_time = millis() / 1000;

    // Auto backup
    if (auto_backup_enabled_ && 
        (current_time - last_backup_time_) > (backup_interval_hours_ * 3600)) {
        createConfigBackup(String(current_time));
        last_backup_time_ = current_time;
    }

    // Periodic remote sync (if enabled)
    if (remote_sync_enabled_ && 
        (current_time - last_sync_time_) > 3600) { // Sync every hour
        syncWithRemote();
    }
}

// Private helper methods

String ConfigManager::sectionToString(ConfigSection section) {
    switch (section) {
        case ConfigSection::CAMERA: return "camera";
        case ConfigSection::MOTION_DETECTION: return "motion_detection";
        case ConfigSection::POWER_MANAGEMENT: return "power_management";
        case ConfigSection::NETWORK: return "network";
        case ConfigSection::AI_PROCESSING: return "ai_processing";
        case ConfigSection::DEPLOYMENT: return "deployment";
        case ConfigSection::SECURITY: return "security";
        case ConfigSection::CUSTOM: return "custom";
        default: return "unknown";
    }
}

ConfigManager::ConfigSection ConfigManager::stringToSection(const String& section_str) {
    if (section_str == "camera") return ConfigSection::CAMERA;
    if (section_str == "motion_detection") return ConfigSection::MOTION_DETECTION;
    if (section_str == "power_management") return ConfigSection::POWER_MANAGEMENT;
    if (section_str == "network") return ConfigSection::NETWORK;
    if (section_str == "ai_processing") return ConfigSection::AI_PROCESSING;
    if (section_str == "deployment") return ConfigSection::DEPLOYMENT;
    if (section_str == "security") return ConfigSection::SECURITY;
    return ConfigSection::CUSTOM;
}

String ConfigManager::generateConfigChecksum(const DynamicJsonDocument& config) {
    // Simplified checksum generation
    String config_str;
    serializeJson(config, config_str);
    return String(config_str.length(), HEX);
}

bool ConfigManager::createConfigBackup(const String& suffix) {
    String backup_path = current_config_path_ + ".backup." + suffix;
    return SPIFFS.rename(current_config_path_, backup_path);
}

DynamicJsonDocument ConfigManager::getDefaultConfiguration() {
    DynamicJsonDocument defaults(4096);
    
    // Camera defaults
    defaults["camera"]["frame_size"] = CAMERA_FRAME_SIZE;
    defaults["camera"]["jpeg_quality"] = CAMERA_JPEG_QUALITY;
    defaults["camera"]["brightness"] = CAMERA_BRIGHTNESS_DEFAULT;
    defaults["camera"]["contrast"] = CAMERA_CONTRAST_DEFAULT;
    defaults["camera"]["saturation"] = CAMERA_SATURATION_DEFAULT;
    
    // Motion detection defaults
    defaults["motion_detection"]["pir_enabled"] = true;
    defaults["motion_detection"]["sensitivity"] = 50;
    defaults["motion_detection"]["debounce_time"] = 2000;
    
    // Power management defaults
    defaults["power_management"]["battery_low_threshold"] = 3.0;
    defaults["power_management"]["deep_sleep_duration"] = 300;
    defaults["power_management"]["solar_enabled"] = true;
    
    // Network defaults
    defaults["network"]["wifi_enabled"] = false;
    defaults["network"]["lora_enabled"] = true;
    defaults["network"]["mesh_enabled"] = true;
    
    return defaults;
}

String ConfigManager::generateDeviceFingerprint() {
    String mac = WiFi.macAddress();
    String chip_id = String(ESP.getEfuseMac(), HEX);
    return mac + "_" + chip_id;
}

bool ConfigManager::validateParameterValue(ConfigSection section, const String& key, const String& value) {
    // Basic validation - can be extended with specific rules
    if (section == ConfigSection::CAMERA) {
        if (key == "jpeg_quality") {
            int quality = value.toInt();
            return (quality >= 4 && quality <= 63);
        }
        if (key == "frame_size") {
            int size = value.toInt();
            return (size >= 0 && size <= 20);
        }
    }
    
    if (section == ConfigSection::POWER_MANAGEMENT) {
        if (key == "battery_low_threshold") {
            float threshold = value.toFloat();
            return (threshold >= 2.0f && threshold <= 4.2f);
        }
    }
    
    return true; // Default: allow all values
}

void ConfigManager::notifyConfigChange(ConfigSection section, const String& key, const String& old_value, const String& new_value) {
    if (change_callback_) {
        change_callback_(section, key, old_value, new_value);
    }
}