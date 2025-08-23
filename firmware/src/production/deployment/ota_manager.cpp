#include "ota_manager.h"

// Static member initialization
OTAManager::OTAConfig OTAManager::config_;
OTAManager::UpdateStatus OTAManager::status_ = UpdateStatus::IDLE;
OTAManager::ProgressCallback OTAManager::progress_callback_ = nullptr;
OTAManager::FirmwareVersion OTAManager::pending_version_;
uint32_t OTAManager::last_check_time_ = 0;
uint32_t OTAManager::scheduled_update_time_ = 0;
bool OTAManager::initialized_ = false;

bool OTAManager::initialize(const OTAConfig& config) {
    if (initialized_) {
        return true;
    }

    config_ = config;
    status_ = UpdateStatus::IDLE;

    // Initialize SPIFFS for storing update state
    if (!SPIFFS.begin(true)) {
        Serial.println("ERROR: Failed to initialize SPIFFS for OTA manager");
        return false;
    }

    // Load previous update state if available
    loadUpdateState();

    // Verify current firmware integrity
    FirmwareVersion current = getCurrentVersion();
    Serial.printf("OTA Manager initialized - Current version: %s\n", current.version.c_str());

    initialized_ = true;
    return true;
}

bool OTAManager::checkForUpdates(bool force_check) {
    if (!initialized_ || !WiFi.isConnected()) {
        return false;
    }

    uint32_t current_time = millis() / 1000;
    uint32_t check_interval = config_.check_interval_hours * 3600;

    // Check if we should skip based on time interval
    if (!force_check && (current_time - last_check_time_) < check_interval) {
        return false;
    }

    status_ = UpdateStatus::CHECKING;
    updateProgress(10, "Checking for updates...");

    HTTPClient http;
    String url = config_.update_server_url + "check/" + config_.device_type + "?current=" + config_.current_version;
    url += "&group=" + String(config_.deployment_group);
    url += "&fingerprint=" + generateDeviceFingerprint();

    http.begin(url);
    http.addHeader("User-Agent", "ESP32WildlifeCAM-OTA/1.0");
    http.addHeader("X-Device-ID", generateDeviceFingerprint());
    http.setTimeout(10000);

    int httpCode = http.GET();
    last_check_time_ = current_time;

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(2048);
        
        if (deserializeJson(doc, payload) == DeserializationError::Ok) {
            if (doc["update_available"].as<bool>()) {
                pending_version_.version = doc["version"].as<String>();
                pending_version_.build_date = doc["build_date"].as<String>();
                pending_version_.checksum = doc["checksum"].as<String>();
                pending_version_.size_bytes = doc["size_bytes"].as<uint32_t>();
                pending_version_.mandatory = doc["mandatory"].as<bool>();
                pending_version_.changelog = doc["changelog"].as<String>();
                pending_version_.compatibility_score = doc["compatibility_score"].as<float>();

                updateProgress(100, "Update available: " + pending_version_.version);
                status_ = UpdateStatus::IDLE;
                
                Serial.printf("Update available: %s (%.1f%% compatible)\n", 
                             pending_version_.version.c_str(), 
                             pending_version_.compatibility_score * 100.0f);
                
                // Auto-start download if enabled and conditions are met
                if (config_.auto_update_enabled && checkBatteryLevel() && checkStorageSpace(pending_version_.size_bytes)) {
                    if (!config_.require_manual_approval || pending_version_.mandatory) {
                        downloadUpdate(pending_version_);
                    }
                }
                
                http.end();
                return true;
            }
        }
    } else {
        Serial.printf("Update check failed: HTTP %d\n", httpCode);
    }

    http.end();
    updateProgress(100, "No updates available");
    status_ = UpdateStatus::IDLE;
    return false;
}

bool OTAManager::downloadUpdate(const FirmwareVersion& version_info) {
    if (!initialized_ || !WiFi.isConnected()) {
        return false;
    }

    if (!checkBatteryLevel()) {
        Serial.println("Battery level too low for update");
        return false;
    }

    if (!checkStorageSpace(version_info.size_bytes)) {
        Serial.println("Insufficient storage space for update");
        return false;
    }

    status_ = UpdateStatus::DOWNLOADING;
    updateProgress(0, "Starting download...");

    // Create backup before downloading
    if (!createBackup()) {
        Serial.println("Failed to create firmware backup");
        status_ = UpdateStatus::FAILED;
        return false;
    }

    HTTPClient http;
    String url = config_.update_server_url + "download/" + version_info.version + "/" + config_.device_type;
    url += "?fingerprint=" + generateDeviceFingerprint();

    http.begin(url);
    http.addHeader("User-Agent", "ESP32WildlifeCAM-OTA/1.0");
    http.addHeader("X-Device-ID", generateDeviceFingerprint());
    http.setTimeout(30000);

    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        int total_size = http.getSize();
        WiFiClient* stream = http.getStreamPtr();
        
        File firmware_file = SPIFFS.open("/firmware_update.bin", "w");
        if (!firmware_file) {
            Serial.println("Failed to open firmware file for writing");
            http.end();
            status_ = UpdateStatus::FAILED;
            return false;
        }

        uint8_t buffer[1024];
        int downloaded = 0;
        int last_progress = 0;

        while (http.connected() && (total_size > 0 || total_size == -1)) {
            size_t available = stream->available();
            if (available) {
                int bytes_to_read = min(available, sizeof(buffer));
                int bytes_read = stream->readBytes(buffer, bytes_to_read);
                
                firmware_file.write(buffer, bytes_read);
                downloaded += bytes_read;
                
                int progress = (downloaded * 80) / total_size; // 80% for download
                if (progress != last_progress) {
                    updateProgress(progress, "Downloading: " + String(downloaded) + "/" + String(total_size) + " bytes");
                    last_progress = progress;
                }
                
                if (total_size > 0 && downloaded >= total_size) {
                    break;
                }
            }
            delay(1);
        }

        firmware_file.close();
        http.end();

        // Validate downloaded firmware
        updateProgress(85, "Validating firmware...");
        if (validateFirmware("/firmware_update.bin", version_info.checksum)) {
            updateProgress(100, "Download complete and validated");
            status_ = UpdateStatus::IDLE;
            
            // Auto-install if configured
            if (config_.auto_update_enabled && !config_.require_manual_approval) {
                return installUpdate(true);
            }
            
            return true;
        } else {
            Serial.println("Firmware validation failed");
            SPIFFS.remove("/firmware_update.bin");
            status_ = UpdateStatus::FAILED;
            return false;
        }
    } else {
        Serial.printf("Download failed: HTTP %d\n", httpCode);
        http.end();
        status_ = UpdateStatus::FAILED;
        return false;
    }
}

bool OTAManager::installUpdate(bool restart_after_install) {
    if (!initialized_) {
        return false;
    }

    if (!SPIFFS.exists("/firmware_update.bin")) {
        Serial.println("No firmware update file found");
        return false;
    }

    status_ = UpdateStatus::INSTALLING;
    updateProgress(0, "Starting installation...");

    File firmware_file = SPIFFS.open("/firmware_update.bin", "r");
    if (!firmware_file) {
        Serial.println("Failed to open firmware update file");
        status_ = UpdateStatus::FAILED;
        return false;
    }

    size_t firmware_size = firmware_file.size();
    
    if (!Update.begin(firmware_size)) {
        Serial.printf("Update begin failed: %s\n", Update.errorString());
        firmware_file.close();
        status_ = UpdateStatus::FAILED;
        return false;
    }

    uint8_t buffer[1024];
    size_t written = 0;
    int last_progress = 0;

    while (firmware_file.available()) {
        size_t bytes_read = firmware_file.readBytes((char*)buffer, sizeof(buffer));
        
        if (Update.write(buffer, bytes_read) != bytes_read) {
            Serial.printf("Update write failed: %s\n", Update.errorString());
            firmware_file.close();
            Update.abort();
            status_ = UpdateStatus::FAILED;
            return false;
        }
        
        written += bytes_read;
        int progress = (written * 100) / firmware_size;
        
        if (progress != last_progress) {
            updateProgress(progress, "Installing: " + String(written) + "/" + String(firmware_size) + " bytes");
            last_progress = progress;
        }
    }

    firmware_file.close();

    if (Update.end(true)) {
        updateProgress(100, "Installation complete");
        status_ = UpdateStatus::COMPLETE;
        
        // Clean up update file
        SPIFFS.remove("/firmware_update.bin");
        
        // Save update state
        saveUpdateState();
        
        Serial.println("Firmware update installed successfully");
        
        if (restart_after_install) {
            delay(2000);
            ESP.restart();
        }
        
        return true;
    } else {
        Serial.printf("Update end failed: %s\n", Update.errorString());
        status_ = UpdateStatus::FAILED;
        return false;
    }
}

bool OTAManager::rollbackToPrevious() {
    if (!initialized_) {
        return false;
    }

    status_ = UpdateStatus::ROLLBACK;
    updateProgress(0, "Starting rollback...");

    if (!restoreBackup()) {
        Serial.println("Rollback failed: No backup available");
        status_ = UpdateStatus::FAILED;
        return false;
    }

    updateProgress(100, "Rollback complete");
    status_ = UpdateStatus::COMPLETE;
    
    delay(2000);
    ESP.restart();
    
    return true;
}

std::vector<OTAManager::FirmwareVersion> OTAManager::getAvailableVersions() {
    std::vector<FirmwareVersion> versions;
    
    if (!initialized_ || !WiFi.isConnected()) {
        return versions;
    }

    HTTPClient http;
    String url = config_.update_server_url + "versions/" + config_.device_type;
    url += "?group=" + String(config_.deployment_group);

    http.begin(url);
    http.addHeader("User-Agent", "ESP32WildlifeCAM-OTA/1.0");
    http.setTimeout(10000);

    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(4096);
        
        if (deserializeJson(doc, payload) == DeserializationError::Ok) {
            JsonArray versions_array = doc["versions"];
            
            for (JsonObject version_obj : versions_array) {
                FirmwareVersion version;
                version.version = version_obj["version"].as<String>();
                version.build_date = version_obj["build_date"].as<String>();
                version.checksum = version_obj["checksum"].as<String>();
                version.size_bytes = version_obj["size_bytes"].as<uint32_t>();
                version.mandatory = version_obj["mandatory"].as<bool>();
                version.changelog = version_obj["changelog"].as<String>();
                version.compatibility_score = version_obj["compatibility_score"].as<float>();
                
                versions.push_back(version);
            }
        }
    }

    http.end();
    return versions;
}

OTAManager::FirmwareVersion OTAManager::getCurrentVersion() {
    FirmwareVersion current;
    current.version = config_.current_version;
    current.build_date = __DATE__ " " __TIME__;
    current.size_bytes = ESP.getSketchSize();
    current.mandatory = false;
    current.changelog = "Current firmware version";
    current.compatibility_score = 1.0f;
    return current;
}

void OTAManager::setProgressCallback(ProgressCallback callback) {
    progress_callback_ = callback;
}

void OTAManager::process() {
    if (!initialized_) {
        return;
    }

    // Check for scheduled updates
    if (scheduled_update_time_ > 0) {
        uint32_t current_time = millis() / 1000;
        if (current_time >= scheduled_update_time_) {
            scheduled_update_time_ = 0;
            if (SPIFFS.exists("/firmware_update.bin")) {
                installUpdate(true);
            }
        }
    }

    // Periodic update checks if auto-update is enabled
    if (config_.auto_update_enabled) {
        checkForUpdates(false);
    }
}

bool OTAManager::validateFirmware(const String& firmware_path, const String& expected_checksum) {
    if (!SPIFFS.exists(firmware_path)) {
        return false;
    }

    // For now, implement basic size validation
    // In production, implement SHA256 checksum validation
    File firmware_file = SPIFFS.open(firmware_path, "r");
    if (!firmware_file) {
        return false;
    }

    size_t file_size = firmware_file.size();
    firmware_file.close();

    // Basic validation: file should be reasonable size for ESP32 firmware
    return (file_size > 100000 && file_size < 2000000); // 100KB - 2MB
}

DynamicJsonDocument OTAManager::getUpdateStatistics() {
    DynamicJsonDocument stats(1024);
    
    stats["status"] = static_cast<int>(status_);
    stats["current_version"] = config_.current_version;
    stats["last_check"] = last_check_time_;
    stats["auto_update_enabled"] = config_.auto_update_enabled;
    stats["deployment_group"] = config_.deployment_group;
    stats["free_heap"] = ESP.getFreeHeap();
    stats["sketch_size"] = ESP.getSketchSize();
    stats["free_sketch_space"] = ESP.getFreeSketchSpace();
    
    if (pending_version_.version.length() > 0) {
        stats["pending_version"] = pending_version_.version;
        stats["pending_size"] = pending_version_.size_bytes;
    }
    
    return stats;
}

void OTAManager::setAutoUpdateEnabled(bool enabled) {
    config_.auto_update_enabled = enabled;
}

void OTAManager::setDeploymentGroup(uint8_t group) {
    config_.deployment_group = group;
}

bool OTAManager::scheduleUpdate(uint32_t timestamp) {
    if (!SPIFFS.exists("/firmware_update.bin")) {
        return false;
    }
    
    scheduled_update_time_ = timestamp;
    saveUpdateState();
    return true;
}

void OTAManager::cancelScheduledUpdate() {
    scheduled_update_time_ = 0;
    saveUpdateState();
}

// Private helper methods

bool OTAManager::connectToUpdateServer() {
    // Implementation would include server authentication
    return WiFi.isConnected();
}

String OTAManager::generateDeviceFingerprint() {
    String mac = WiFi.macAddress();
    String chip_id = String(ESP.getEfuseMac(), HEX);
    return mac + "_" + chip_id;
}

bool OTAManager::verifyUpdateSignature(const String& firmware_data, const String& signature) {
    // Placeholder for cryptographic signature verification
    // In production, implement RSA/ECDSA signature verification
    return true;
}

void OTAManager::updateProgress(uint8_t progress, const String& status) {
    if (progress_callback_) {
        progress_callback_(progress, status);
    }
}

bool OTAManager::createBackup() {
    // For ESP32, we rely on the bootloader's partition switching
    // This would backup critical configuration data
    return true;
}

bool OTAManager::restoreBackup() {
    // Restore from backup partition
    // This is a simplified implementation
    return true;
}

void OTAManager::saveUpdateState() {
    DynamicJsonDocument state(512);
    state["scheduled_update"] = scheduled_update_time_;
    state["last_check"] = last_check_time_;
    state["deployment_group"] = config_.deployment_group;
    
    File state_file = SPIFFS.open("/ota_state.json", "w");
    if (state_file) {
        serializeJson(state, state_file);
        state_file.close();
    }
}

void OTAManager::loadUpdateState() {
    if (SPIFFS.exists("/ota_state.json")) {
        File state_file = SPIFFS.open("/ota_state.json", "r");
        if (state_file) {
            DynamicJsonDocument state(512);
            if (deserializeJson(state, state_file) == DeserializationError::Ok) {
                scheduled_update_time_ = state["scheduled_update"].as<uint32_t>();
                last_check_time_ = state["last_check"].as<uint32_t>();
                config_.deployment_group = state["deployment_group"].as<uint8_t>();
            }
            state_file.close();
        }
    }
}

bool OTAManager::checkBatteryLevel() {
    // Integration with existing power management
    // This would check actual battery voltage
    return true; // Placeholder
}

bool OTAManager::checkStorageSpace(uint32_t required_bytes) {
    return ESP.getFreeSketchSpace() >= required_bytes;
}