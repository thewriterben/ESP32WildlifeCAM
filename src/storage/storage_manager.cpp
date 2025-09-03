/**
 * @file storage_manager.cpp
 * @brief Simple storage management implementation
 * @author thewriterben
 * @date 2025-09-03
 */

#include "storage_manager.h"
#include "../utils/logger.h"

// Default directories for wildlife camera
#define IMAGES_DIR "/images"
#define LOGS_DIR "/logs"

StorageManager::StorageManager() : initialized(false), imageCounter(0) {
}

StorageManager::~StorageManager() {
    if (initialized) {
        SD_MMC.end();
        initialized = false;
    }
}

bool StorageManager::init() {
    if (initialized) {
        return true; // Already initialized
    }
    
    Serial.println("Initializing SD card storage...");
    
    // Initialize SD card in 1-bit mode (safer for ESP32-CAM)
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("SD Card initialization failed");
        return false;
    }
    
    // Check card type
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        SD_MMC.end();
        return false;
    }
    
    // Print card information
    const char* cardTypeStr = "UNKNOWN";
    if (cardType == CARD_MMC) {
        cardTypeStr = "MMC";
    } else if (cardType == CARD_SD) {
        cardTypeStr = "SDSC";
    } else if (cardType == CARD_SDHC) {
        cardTypeStr = "SDHC";
    }
    
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Type: %s, Size: %lluMB\n", cardTypeStr, cardSize);
    
    // Create required directories
    if (!createDirectories()) {
        Serial.println("Failed to create required directories");
        SD_MMC.end();
        return false;
    }
    
    initialized = true;
    Serial.println("SD card storage initialized successfully");
    
    return true;
}

String StorageManager::saveImage(camera_fb_t* fb, const String& filename) {
    if (!initialized || !fb) {
        Serial.println("Storage not ready or invalid frame buffer");
        return "";
    }
    
    // Generate filename if not provided
    String imageFilename = filename.isEmpty() ? generateFilename() : filename;
    String fullPath = String(IMAGES_DIR) + "/" + imageFilename;
    
    Serial.printf("Saving image: %s (%d bytes)\n", fullPath.c_str(), fb->len);
    
    // Open file for writing
    fs::FS &fs = SD_MMC;
    File file = fs.open(fullPath.c_str(), FILE_WRITE);
    if (!file) {
        Serial.printf("Failed to create file: %s\n", fullPath.c_str());
        return "";
    }
    
    // Write image data
    size_t written = file.write(fb->buf, fb->len);
    file.close();
    
    if (written != fb->len) {
        Serial.printf("Failed to write complete image data. Expected: %d, Written: %d\n", fb->len, written);
        // Try to delete incomplete file
        fs.remove(fullPath.c_str());
        return "";
    }
    
    imageCounter++;
    Serial.printf("Image saved successfully: %s (%d bytes)\n", fullPath.c_str(), written);
    
    return fullPath;
}

bool StorageManager::getStorageStats(uint64_t& totalMB, uint64_t& usedMB, uint64_t& freeMB) {
    if (!initialized) {
        return false;
    }
    
    totalMB = SD_MMC.cardSize() / (1024 * 1024);
    usedMB = SD_MMC.usedBytes() / (1024 * 1024);
    freeMB = totalMB - usedMB;
    
    return true;
}

bool StorageManager::createDirectories() {
    fs::FS &fs = SD_MMC;
    
    // Create images directory
    if (!ensureDirectory(IMAGES_DIR)) {
        Serial.printf("Failed to create directory: %s\n", IMAGES_DIR);
        return false;
    }
    
    // Create logs directory
    if (!ensureDirectory(LOGS_DIR)) {
        Serial.printf("Failed to create directory: %s\n", LOGS_DIR);
        return false;
    }
    
    Serial.println("Required directories created successfully");
    return true;
}

String StorageManager::generateFilename() {
    // Generate filename based on timestamp and counter
    // Format: IMG_YYYYMMDD_HHMMSS_NNN.jpg
    
    // Since we don't have RTC set up yet, use millis() and counter
    unsigned long timestamp = millis();
    
    char filename[32];
    snprintf(filename, sizeof(filename), "IMG_%08lX_%04d.jpg", timestamp, imageCounter);
    
    return String(filename);
}

bool StorageManager::ensureDirectory(const String& path) {
    fs::FS &fs = SD_MMC;
    
    // Check if directory already exists
    File dir = fs.open(path.c_str());
    if (dir && dir.isDirectory()) {
        dir.close();
        return true;
    }
    if (dir) {
        dir.close();
    }
    
    // Try to create directory
    return fs.mkdir(path.c_str());
}