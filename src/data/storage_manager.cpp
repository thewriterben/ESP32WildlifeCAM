#include "storage_manager.h"
#include "../utils/logger.h"
#include <time.h>

// Static member definitions
bool StorageManager::initialized = false;
uint8_t StorageManager::warningThreshold = 85;

bool StorageManager::initializeSDCard() {
    LOG_INFO("Initializing SD Card...");
    
    // Initialize SD card in MMC mode (1-bit mode for ESP32-CAM)
    if (!SD_MMC.begin("/sdcard", true)) { // true = 1-bit mode
        LOG_ERROR("Failed to initialize SD card");
        return false;
    }
    
    // Check if card is properly mounted
    uint64_t cardSize = SD_MMC.totalBytes();
    if (cardSize == 0) {
        LOG_ERROR("SD card size is 0 - card may not be properly inserted");
        return false;
    }
    
    LOG_INFO("SD Card initialized successfully");
    LOG_INFO("SD Card size: " + String((unsigned long)(cardSize / (1024 * 1024))) + "MB");
    
    return true;
}

bool StorageManager::saveImage(camera_fb_t* fb, const String& filename) {
    if (!initialized || !fb) {
        LOG_ERROR("Storage manager not initialized or invalid frame buffer");
        return false;
    }
    
    if (!hasAdequateSpace(fb->len + 1024)) { // Add 1KB buffer
        LOG_WARNING("Insufficient space for image, attempting cleanup");
        performCleanup(false);
        
        if (!hasAdequateSpace(fb->len + 1024)) {
            LOG_ERROR("Still insufficient space after cleanup");
            return false;
        }
    }
    
    // Create full path
    String fullPath = String(IMAGE_FOLDER) + "/" + filename;
    
    // Ensure directory exists
    String dirPath = fullPath.substring(0, fullPath.lastIndexOf('/'));
    if (!SD_MMC.exists(dirPath.c_str())) {
        // Create directory recursively
        String tempPath = "";
        int start = 0;
        int end = dirPath.indexOf('/', start + 1);
        
        while (end != -1) {
            tempPath = dirPath.substring(0, end);
            if (!SD_MMC.exists(tempPath.c_str())) {
                if (!SD_MMC.mkdir(tempPath.c_str())) {
                    LOG_ERROR("Failed to create directory: " + tempPath);
                }
            }
            start = end;
            end = dirPath.indexOf('/', start + 1);
        }
        
        // Create final directory
        if (!SD_MMC.exists(dirPath.c_str())) {
            if (!SD_MMC.mkdir(dirPath.c_str())) {
                LOG_ERROR("Failed to create final directory: " + dirPath);
                return false;
            }
        }
    }
    
    // Save image file
    File file = SD_MMC.open(fullPath.c_str(), FILE_WRITE);
    if (!file) {
        LOG_ERROR("Failed to create image file: " + fullPath);
        return false;
    }
    
    size_t bytesWritten = file.write(fb->buf, fb->len);
    file.close();
    
    if (bytesWritten != fb->len) {
        LOG_ERROR("Failed to write complete image data");
        SD_MMC.remove(fullPath.c_str()); // Clean up partial file
        return false;
    }
    
    LOG_INFO("Image saved successfully: " + filename + " (" + String(bytesWritten) + " bytes)");
    return true;
}

bool StorageManager::saveMetadata(const ImageMetadata& metadata) {
    if (!initialized) {
        return false;
    }
    
    // Create metadata filename
    String metadataFilename = metadata.timestamp + "_metadata.json";
    String fullPath = String(DATA_FOLDER) + "/" + metadataFilename;
    
    // Ensure directory exists
    if (!SD_MMC.exists(DATA_FOLDER)) {
        if (!SD_MMC.mkdir(DATA_FOLDER)) {
            LOG_ERROR("Failed to create data directory");
            return false;
        }
    }
    
    // Create JSON metadata
    String jsonData = "{\n";
    jsonData += "  \"timestamp\": \"" + metadata.timestamp + "\",\n";
    jsonData += "  \"batteryLevel\": " + String(metadata.batteryLevel, 2) + ",\n";
    jsonData += "  \"temperature\": " + String(metadata.temperature, 1) + ",\n";
    jsonData += "  \"humidity\": " + String(metadata.humidity, 1) + ",\n";
    jsonData += "  \"aiDetection\": \"" + metadata.aiDetection + "\",\n";
    jsonData += "  \"confidence\": " + String(metadata.confidence, 3) + ",\n";
    jsonData += "  \"location\": \"" + metadata.location + "\"\n";
    jsonData += "}\n";
    
    // Save metadata file
    File file = SD_MMC.open(fullPath.c_str(), FILE_WRITE);
    if (!file) {
        LOG_ERROR("Failed to create metadata file: " + fullPath);
        return false;
    }
    
    size_t bytesWritten = file.print(jsonData);
    file.close();
    
    if (bytesWritten == 0) {
        LOG_ERROR("Failed to write metadata");
        SD_MMC.remove(fullPath.c_str()); // Clean up empty file
        return false;
    }
    
    LOG_DEBUG("Metadata saved: " + metadataFilename);
    return true;
}

bool StorageManager::isSDCardHealthy() {
    if (!initialized) {
        return false;
    }
    
    // Test basic operations
    uint64_t totalBytes = SD_MMC.totalBytes();
    uint64_t usedBytes = SD_MMC.usedBytes();
    
    // Check if we can get valid size info
    if (totalBytes == 0) {
        LOG_WARNING("SD card reports 0 total bytes - may be unhealthy");
        return false;
    }
    
    // Test write operation with a small test file
    String testFile = "/test_health.tmp";
    File file = SD_MMC.open(testFile.c_str(), FILE_WRITE);
    if (!file) {
        LOG_WARNING("Cannot create test file - SD card may be write-protected or full");
        return false;
    }
    
    String testData = "health_check_" + String(millis());
    file.print(testData);
    file.close();
    
    // Test read operation
    file = SD_MMC.open(testFile.c_str(), FILE_READ);
    if (!file) {
        LOG_WARNING("Cannot read test file - SD card may be unhealthy");
        return false;
    }
    
    String readData = file.readString();
    file.close();
    
    // Cleanup test file
    SD_MMC.remove(testFile.c_str());
    
    // Verify data integrity
    if (readData != testData) {
        LOG_WARNING("Data corruption detected in health check");
        return false;
    }
    
    LOG_DEBUG("SD card health check passed");
    return true;
}

void StorageManager::cleanupOldFiles() {
    if (!initialized) {
        return;
    }
    
    // Perform automatic cleanup based on storage usage
    StorageStats stats = getStatistics();
    
    if (stats.usagePercentage > 85.0f) {
        LOG_INFO("Storage usage high (" + String(stats.usagePercentage, 1) + "%), starting cleanup...");
        performCleanup(true); // Aggressive cleanup
    } else if (stats.usagePercentage > 70.0f) {
        LOG_INFO("Storage usage moderate (" + String(stats.usagePercentage, 1) + "%), starting cleanup...");
        performCleanup(false); // Normal cleanup
    }
}

bool StorageManager::initialize() {
    if (initialized) {
        return true;
    }

    LOG_INFO("Initializing storage manager...");

    // Initialize SD card
    if (!initializeSDCard()) {
        LOG_ERROR("SD card initialization failed");
        return false;
    }

    // Create directory structure
    if (!createDirectoryStructure()) {
        LOG_WARNING("Some directories could not be created");
    }

    initialized = true;
    LOG_INFO("Storage manager initialized successfully");

    // Log initial statistics
    StorageStats stats = getStatistics();
    LOG_INFO("Storage capacity: " + String((unsigned long)(stats.totalSpace / (1024 * 1024))) + "MB");
    LOG_INFO("Storage used: " + String(stats.usagePercentage, 1) + "%");

    return true;
}

StorageManager::StorageStats StorageManager::getStatistics() {
    StorageStats stats = {};

    if (!initialized) {
        return stats;
    }

    // Get SD card capacity
    stats.totalSpace = SD_MMC.totalBytes();
    stats.usedSpace = SD_MMC.usedBytes();
    stats.freeSpace = stats.totalSpace - stats.usedSpace;
    stats.usagePercentage = (float)stats.usedSpace / stats.totalSpace * 100.0f;

    // Count files
    countFiles("/", stats);

    return stats;
}

bool StorageManager::hasAdequateSpace(size_t requiredSpace) {
    if (!initialized) {
        return false;
    }

    StorageStats stats = getStatistics();
    return stats.freeSpace >= requiredSpace;
}

StorageManager::CleanupResult StorageManager::performCleanup(bool aggressiveCleanup) {
    CleanupResult result = {};
    uint32_t startTime = millis();

    if (!initialized) {
        return result;
    }

    LOG_INFO("Starting storage cleanup...");

    uint64_t initialFreeSpace = SD_MMC.totalBytes() - SD_MMC.usedBytes();

    // Delete old files (30 days for normal, 7 days for aggressive)
    uint8_t maxAge = aggressiveCleanup ? 7 : 30;
    result.filesDeleted += deleteOldFiles(maxAge);

    // Optimize storage by removing low-quality data
    if (aggressiveCleanup) {
        result.filesDeleted += optimizeStorage();
    }

    // Calculate space freed
    uint64_t finalFreeSpace = SD_MMC.totalBytes() - SD_MMC.usedBytes();
    result.spaceFree = finalFreeSpace - initialFreeSpace;
    result.processingTime = millis() - startTime;
    result.success = result.filesDeleted > 0 || result.spaceFree > 0;

    LOG_INFO("Storage cleanup completed: " + String(result.filesDeleted) + 
             " files deleted, " + String((unsigned long)(result.spaceFree / 1024)) + "KB freed");

    return result;
}

uint32_t StorageManager::deleteOldFiles(uint8_t maxAgeDays, const String& filePattern) {
    if (!initialized) {
        return 0;
    }

    uint32_t filesDeleted = 0;

    // Delete old files from images directory
    filesDeleted += deleteFilesRecursive(IMAGE_FOLDER, maxAgeDays, filePattern);

    // Delete old log files
    filesDeleted += deleteFilesRecursive(LOG_FOLDER, maxAgeDays, "*.log");

    return filesDeleted;
}

uint32_t StorageManager::optimizeStorage() {
    if (!initialized) {
        return 0;
    }

    uint32_t filesRemoved = 0;

    // This is a placeholder for storage optimization
    // Real implementation might:
    // - Remove duplicate images
    // - Delete images with very low AI confidence
    // - Compress old images
    // - Remove images without wildlife detection

    LOG_DEBUG("Storage optimization completed: " + String(filesRemoved) + " files optimized");

    return filesRemoved;
}

bool StorageManager::createDirectoryStructure() {
    bool success = true;

    // Create main directories
    if (!SD_MMC.exists(IMAGE_FOLDER)) {
        success &= SD_MMC.mkdir(IMAGE_FOLDER);
    }

    if (!SD_MMC.exists(LOG_FOLDER)) {
        success &= SD_MMC.mkdir(LOG_FOLDER);
    }

    if (!SD_MMC.exists(DATA_FOLDER)) {
        success &= SD_MMC.mkdir(DATA_FOLDER);
    }

    // Create species subdirectories
    String speciesDirs[] = {
        "/deer", "/bear", "/fox", "/wolf", "/raccoon", 
        "/coyote", "/elk", "/moose", "/turkey", "/eagle", "/unknown"
    };

    for (const String& speciesDir : speciesDirs) {
        String fullPath = String(IMAGE_FOLDER) + speciesDir;
        if (!SD_MMC.exists(fullPath.c_str())) {
            success &= SD_MMC.mkdir(fullPath.c_str());
        }
    }

    return success;
}

uint16_t StorageManager::getFileAge(const String& filename) {
    if (!fileExists(filename)) {
        return 0;
    }

    // Get current time
    time_t now;
    time(&now);

    // Get file modification time (simplified)
    // In a real implementation, you would read the actual file timestamp
    File file = SD_MMC.open(filename.c_str());
    if (!file) {
        return 0;
    }

    // Extract date from filename if it follows our naming convention
    // Format: prefix_YYYYMMDD_HHMMSS_counter.ext
    String basename = filename;
    int lastSlash = basename.lastIndexOf('/');
    if (lastSlash >= 0) {
        basename = basename.substring(lastSlash + 1);
    }

    // Extract date part (YYYYMMDD)
    int dateStart = basename.indexOf('_');
    if (dateStart >= 0) {
        String datePart = basename.substring(dateStart + 1, dateStart + 9);
        if (datePart.length() == 8) {
            // Parse date
            int year = datePart.substring(0, 4).toInt();
            int month = datePart.substring(4, 6).toInt();
            int day = datePart.substring(6, 8).toInt();

            // Calculate age in days (simplified)
            struct tm fileTime = {};
            fileTime.tm_year = year - 1900;
            fileTime.tm_mon = month - 1;
            fileTime.tm_mday = day;

            time_t fileTimestamp = mktime(&fileTime);
            uint32_t ageDays = (now - fileTimestamp) / (24 * 3600);

            file.close();
            return ageDays;
        }
    }

    file.close();
    return 0;
}

bool StorageManager::fileExists(const String& filename) {
    return SD_MMC.exists(filename.c_str());
}

size_t StorageManager::getFileSize(const String& filename) {
    File file = SD_MMC.open(filename.c_str());
    if (!file) {
        return 0;
    }

    size_t size = file.size();
    file.close();
    return size;
}

uint16_t StorageManager::listFiles(const String& directory, String* fileList, uint16_t maxFiles) {
    if (!initialized || fileList == nullptr) {
        return 0;
    }

    File dir = SD_MMC.open(directory.c_str());
    if (!dir || !dir.isDirectory()) {
        return 0;
    }

    uint16_t fileCount = 0;
    File file = dir.openNextFile();

    while (file && fileCount < maxFiles) {
        if (!file.isDirectory()) {
            fileList[fileCount] = String(file.name());
            fileCount++;
        }
        file.close();
        file = dir.openNextFile();
    }

    dir.close();
    return fileCount;
}

bool StorageManager::isWarningThresholdExceeded() {
    StorageStats stats = getStatistics();
    return stats.usagePercentage >= warningThreshold;
}

void StorageManager::countFiles(const String& directory, StorageStats& stats) {
    File dir = SD_MMC.open(directory.c_str());
    if (!dir || !dir.isDirectory()) {
        return;
    }

    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            stats.totalFiles++;

            String filename = String(file.name());
            if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
                stats.imageFiles++;
            } else if (filename.endsWith(".json")) {
                stats.metadataFiles++;
            } else if (filename.endsWith(".log")) {
                stats.logFiles++;
            }
        } else {
            // Recursively count files in subdirectories
            String subDir = directory;
            if (!subDir.endsWith("/")) {
                subDir += "/";
            }
            subDir += file.name();
            countFiles(subDir, stats);
        }

        file.close();
        file = dir.openNextFile();
    }

    dir.close();
}

uint64_t StorageManager::calculateDirectorySize(const String& directory) {
    uint64_t totalSize = 0;

    File dir = SD_MMC.open(directory.c_str());
    if (!dir || !dir.isDirectory()) {
        return 0;
    }

    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            totalSize += file.size();
        } else {
            // Recursively calculate size of subdirectories
            String subDir = directory;
            if (!subDir.endsWith("/")) {
                subDir += "/";
            }
            subDir += file.name();
            totalSize += calculateDirectorySize(subDir);
        }

        file.close();
        file = dir.openNextFile();
    }

    dir.close();
    return totalSize;
}

uint32_t StorageManager::deleteFilesRecursive(const String& directory, uint8_t maxAge, const String& pattern) {
    uint32_t filesDeleted = 0;

    File dir = SD_MMC.open(directory.c_str());
    if (!dir || !dir.isDirectory()) {
        return 0;
    }

    File file = dir.openNextFile();
    while (file) {
        String filename = String(file.name());
        String fullPath = directory;
        if (!fullPath.endsWith("/")) {
            fullPath += "/";
        }
        fullPath += filename;

        if (!file.isDirectory()) {
            // Check file age and pattern
            uint16_t age = getFileAge(fullPath);
            bool matchesPattern = pattern.isEmpty() || filename.indexOf(pattern.substring(1)) >= 0; // Simple pattern matching

            if (age >= maxAge && matchesPattern) {
                file.close();
                if (SD_MMC.remove(fullPath.c_str())) {
                    filesDeleted++;
                    LOG_DEBUG("Deleted old file: " + fullPath);
                }
            } else {
                file.close();
            }
        } else {
            file.close();
            // Recursively delete files in subdirectories
            filesDeleted += deleteFilesRecursive(fullPath, maxAge, pattern);
        }

        file = dir.openNextFile();
    }

    dir.close();
    return filesDeleted;
}