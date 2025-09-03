/**
 * @file storage_manager.h
 * @brief Simple storage management for captured images
 * @author thewriterben
 * @date 2025-09-03
 */

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <esp_camera.h>

/**
 * @brief Simple storage manager for wildlife camera images
 */
class StorageManager {
public:
    StorageManager();
    ~StorageManager();
    
    /**
     * @brief Initialize SD card storage
     * @return true if successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Check if storage is ready
     * @return true if ready, false otherwise
     */
    bool isReady() const { return initialized; }
    
    /**
     * @brief Save image to SD card
     * @param fb Frame buffer containing image data
     * @param filename Optional filename (auto-generated if not provided)
     * @return Full path of saved file, or empty string on failure
     */
    String saveImage(camera_fb_t* fb, const String& filename = "");
    
    /**
     * @brief Get storage statistics
     * @param totalMB Total storage capacity in MB
     * @param usedMB Used storage in MB
     * @param freeMB Free storage in MB
     * @return true if statistics retrieved successfully
     */
    bool getStorageStats(uint64_t& totalMB, uint64_t& usedMB, uint64_t& freeMB);
    
    /**
     * @brief Create required directories on SD card
     * @return true if successful
     */
    bool createDirectories();

private:
    bool initialized;
    int imageCounter;
    
    /**
     * @brief Generate timestamp-based filename
     * @return Filename string
     */
    String generateFilename();
    
    /**
     * @brief Ensure directory exists
     * @param path Directory path
     * @return true if directory exists or was created
     */
    bool ensureDirectory(const String& path);
};

#endif // STORAGE_MANAGER_H