/**
 * @file cellular_test.ino
 * @brief Cellular communication test for ESP32 Wildlife Camera
 * 
 * This example demonstrates basic cellular connectivity testing including:
 * - Module initialization and network connection
 * - Signal strength monitoring
 * - Data transmission testing
 * - SMS backup functionality
 * - Power management optimization
 */

#include "cellular_manager.h"
#include "data_compression.h"
#include "gps_handler.h"

// Configuration
#define ENABLE_DEBUG_OUTPUT true
#define TEST_DURATION_MINUTES 30
#define TEST_INTERVAL_SECONDS 60

// Global objects
CellularManager cellular;
DataCompression compressor;
GPSHandler gps;

// Test data structures
struct TestResults {
    int totalAttempts;
    int successfulTransmissions;
    int failedTransmissions;
    float averageSignalStrength;
    unsigned long totalTransmissionTime;
    size_t totalDataSent;
};

TestResults testResults = {0, 0, 0, 0, 0, 0};

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== ESP32 Wildlife Camera - Cellular Test ===");
    Serial.println("Initializing cellular communication system...");
    
    // Initialize cellular module
    if (!cellular.initialize()) {
        Serial.println("ERROR: Failed to initialize cellular module");
        Serial.println("Check hardware connections and power supply");
        while (1) {
            delay(1000);
        }
    }
    
    // Initialize GPS for location services
    Serial.println("Initializing GPS...");
    if (gps.initialize()) {
        Serial.println("GPS initialized successfully");
        
        // Wait for GPS fix
        Serial.println("Waiting for GPS fix...");
        if (gps.waitForFix(60000)) { // 1 minute timeout
            GPSLocation location = gps.getCurrentLocation();
            Serial.printf("GPS fix acquired: %.6f, %.6f\n", 
                         location.latitude, location.longitude);
        } else {
            Serial.println("WARNING: No GPS fix acquired");
        }
    } else {
        Serial.println("WARNING: GPS initialization failed");
    }
    
    // Connect to cellular network
    Serial.println("Connecting to cellular network...");
    if (cellular.connectToNetwork()) {
        Serial.println("Successfully connected to cellular network");
        
        // Display connection information
        displayNetworkInfo();
    } else {
        Serial.println("ERROR: Failed to connect to cellular network");
        Serial.println("Check SIM card, APN settings, and signal coverage");
        while (1) {
            delay(1000);
        }
    }
    
    Serial.println("\nStarting cellular communication tests...");
    Serial.printf("Test duration: %d minutes\n", TEST_DURATION_MINUTES);
    Serial.printf("Test interval: %d seconds\n", TEST_INTERVAL_SECONDS);
    Serial.println("==============================================\n");
}

void loop() {
    static unsigned long lastTestTime = 0;
    static unsigned long testStartTime = millis();
    
    // Check if test duration is complete
    if (millis() - testStartTime > TEST_DURATION_MINUTES * 60 * 1000) {
        displayFinalResults();
        
        Serial.println("\nTest completed. Entering low power mode...");
        cellular.enterSleepMode();
        
        // Infinite loop to stop further testing
        while (1) {
            delay(60000);
        }
    }
    
    // Perform test at specified intervals
    if (millis() - lastTestTime >= TEST_INTERVAL_SECONDS * 1000) {
        performCellularTest();
        lastTestTime = millis();
    }
    
    // Monitor network status
    monitorNetworkStatus();
    
    delay(1000);
}

void displayNetworkInfo() {
    Serial.println("--- Network Information ---");
    
    // Signal strength
    int signalStrength = cellular.getSignalStrength();
    Serial.printf("Signal Strength: %d dBm ", signalStrength);
    
    if (signalStrength > -70) {
        Serial.println("(Excellent)");
    } else if (signalStrength > -85) {
        Serial.println("(Good)");
    } else if (signalStrength > -100) {
        Serial.println("(Fair)");
    } else {
        Serial.println("(Poor)");
    }
    
    // Network type
    NetworkType networkType = cellular.getNetworkType();
    Serial.print("Network Type: ");
    switch (networkType) {
        case NETWORK_LTE:
            Serial.println("LTE");
            break;
        case NETWORK_3G:
            Serial.println("3G");
            break;
        case NETWORK_2G:
            Serial.println("2G");
            break;
        default:
            Serial.println("Unknown");
            break;
    }
    
    // Data usage
    size_t dataUsage = cellular.getDataUsage();
    Serial.printf("Data Usage: %d MB\n", dataUsage);
    
    Serial.println("-----------------------------\n");
}

void performCellularTest() {
    testResults.totalAttempts++;
    
    Serial.printf("\n--- Test #%d ---\n", testResults.totalAttempts);
    
    // Update signal strength average
    int currentSignal = cellular.getSignalStrength();
    testResults.averageSignalStrength = 
        ((testResults.averageSignalStrength * (testResults.totalAttempts - 1)) + currentSignal) 
        / testResults.totalAttempts;
    
    Serial.printf("Signal: %d dBm\n", currentSignal);
    
    // Create test data
    String testData = createTestMessage();
    size_t dataSize = testData.length();
    
    Serial.printf("Sending %d bytes: %s\n", dataSize, testData.c_str());
    
    // Measure transmission time
    unsigned long startTime = millis();
    bool success = cellular.sendData((uint8_t*)testData.c_str(), dataSize);
    unsigned long transmissionTime = millis() - startTime;
    
    if (success) {
        testResults.successfulTransmissions++;
        testResults.totalTransmissionTime += transmissionTime;
        testResults.totalDataSent += dataSize;
        
        Serial.printf("SUCCESS: Transmitted in %lu ms\n", transmissionTime);
    } else {
        testResults.failedTransmissions++;
        Serial.println("FAILED: Transmission error");
        
        // Test SMS backup functionality
        testSMSBackup();
    }
    
    // Display current statistics
    displayCurrentStats();
}

String createTestMessage() {
    String message = "{";
    
    // Add timestamp
    message += "\"timestamp\":" + String(millis()) + ",";
    
    // Add GPS location if available
    if (gps.isLocationValid()) {
        GPSLocation location = gps.getCurrentLocation();
        message += "\"lat\":" + String(location.latitude, 6) + ",";
        message += "\"lon\":" + String(location.longitude, 6) + ",";
        message += "\"alt\":" + String(location.altitude, 1) + ",";
    }
    
    // Add system status
    message += "\"battery\":85,";  // Simulated battery level
    message += "\"temperature\":22.5,";  // Simulated temperature
    message += "\"humidity\":65,";  // Simulated humidity
    message += "\"motion\":false,";  // Simulated motion detection
    
    // Add test identifier
    message += "\"test_id\":" + String(testResults.totalAttempts);
    
    message += "}";
    
    return message;
}

void testSMSBackup() {
    Serial.println("Testing SMS backup functionality...");
    
    String smsMessage = "Wildlife Camera Alert: Cellular data transmission failed. ";
    smsMessage += "Test #" + String(testResults.totalAttempts);
    
    if (gps.isLocationValid()) {
        GPSLocation location = gps.getCurrentLocation();
        smsMessage += " Location: " + String(location.latitude, 4) + "," + String(location.longitude, 4);
    }
    
    // Use test phone number (replace with actual emergency contact)
    String phoneNumber = "+1234567890";  // Replace with real number for testing
    
    if (cellular.sendSMS(phoneNumber, smsMessage)) {
        Serial.println("SMS backup sent successfully");
    } else {
        Serial.println("SMS backup failed");
    }
}

void monitorNetworkStatus() {
    static unsigned long lastStatusCheck = 0;
    
    // Check network status every 10 seconds
    if (millis() - lastStatusCheck >= 10000) {
        if (!cellular.isNetworkConnected()) {
            Serial.println("WARNING: Network connection lost, attempting reconnection...");
            
            if (cellular.connectToNetwork()) {
                Serial.println("Network reconnected successfully");
            } else {
                Serial.println("Network reconnection failed");
            }
        }
        
        lastStatusCheck = millis();
    }
}

void displayCurrentStats() {
    float successRate = (float)testResults.successfulTransmissions / testResults.totalAttempts * 100;
    float avgTime = testResults.successfulTransmissions > 0 ? 
                   (float)testResults.totalTransmissionTime / testResults.successfulTransmissions : 0;
    
    Serial.printf("Stats: %d/%d success (%.1f%%), Avg signal: %.1f dBm, Avg time: %.1f ms\n",
                  testResults.successfulTransmissions, testResults.totalAttempts,
                  successRate, testResults.averageSignalStrength, avgTime);
}

void displayFinalResults() {
    Serial.println("\n=========== FINAL TEST RESULTS ===========");
    Serial.printf("Total Attempts: %d\n", testResults.totalAttempts);
    Serial.printf("Successful: %d\n", testResults.successfulTransmissions);
    Serial.printf("Failed: %d\n", testResults.failedTransmissions);
    
    float successRate = (float)testResults.successfulTransmissions / testResults.totalAttempts * 100;
    Serial.printf("Success Rate: %.1f%%\n", successRate);
    
    Serial.printf("Average Signal Strength: %.1f dBm\n", testResults.averageSignalStrength);
    
    if (testResults.successfulTransmissions > 0) {
        float avgTime = (float)testResults.totalTransmissionTime / testResults.successfulTransmissions;
        Serial.printf("Average Transmission Time: %.1f ms\n", avgTime);
        
        float throughput = (float)testResults.totalDataSent * 8 / (testResults.totalTransmissionTime / 1000.0);
        Serial.printf("Average Throughput: %.1f bps\n", throughput);
    }
    
    Serial.printf("Total Data Transmitted: %d bytes\n", testResults.totalDataSent);
    Serial.printf("Data Usage: %d MB\n", cellular.getDataUsage());
    
    // Performance assessment
    Serial.println("\n--- Performance Assessment ---");
    if (successRate >= 95) {
        Serial.println("EXCELLENT: Network performance is outstanding");
    } else if (successRate >= 85) {
        Serial.println("GOOD: Network performance is acceptable");
    } else if (successRate >= 70) {
        Serial.println("FAIR: Network performance needs attention");
    } else {
        Serial.println("POOR: Network performance is inadequate");
    }
    
    if (testResults.averageSignalStrength > -70) {
        Serial.println("Signal strength is excellent");
    } else if (testResults.averageSignalStrength > -85) {
        Serial.println("Signal strength is good");
    } else if (testResults.averageSignalStrength > -100) {
        Serial.println("Signal strength is marginal - consider antenna improvements");
    } else {
        Serial.println("Signal strength is poor - relocation or external antenna recommended");
    }
    
    Serial.println("==========================================");
}

// Utility function for testing image compression
void testImageCompression() {
    Serial.println("\n--- Testing Image Compression ---");
    
    // Simulate image data
    const size_t imageSize = 32000; // 32KB test image
    uint8_t* testImage = new uint8_t[imageSize];
    
    // Fill with test pattern
    for (size_t i = 0; i < imageSize; i++) {
        testImage[i] = (i % 256);
    }
    
    // Compress image
    uint8_t* compressedImage = new uint8_t[imageSize];
    size_t compressedSize = compressor.compressImage(testImage, imageSize, 
                                                    compressedImage, imageSize,
                                                    COMPRESSION_MEDIUM);
    
    if (compressedSize > 0) {
        float compressionRatio = (float)compressedSize / imageSize;
        Serial.printf("Compression test: %d -> %d bytes (%.1f%%)\n", 
                     imageSize, compressedSize, compressionRatio * 100);
        
        // Test cellular transmission of compressed image
        if (compressedSize < 10000) { // Only if reasonably sized
            bool success = cellular.sendData(compressedImage, compressedSize);
            Serial.printf("Compressed image transmission: %s\n", 
                         success ? "SUCCESS" : "FAILED");
        }
    } else {
        Serial.println("Image compression failed");
    }
    
    delete[] testImage;
    delete[] compressedImage;
}

// Power consumption test
void testPowerManagement() {
    Serial.println("\n--- Testing Power Management ---");
    
    // Test sleep mode entry and exit
    Serial.println("Entering cellular sleep mode...");
    cellular.enterSleepMode();
    delay(5000); // Sleep for 5 seconds
    
    Serial.println("Exiting cellular sleep mode...");
    cellular.exitSleepMode();
    
    // Wait for network reconnection
    delay(10000);
    
    if (cellular.isNetworkConnected()) {
        Serial.println("Power management test: SUCCESS");
    } else {
        Serial.println("Power management test: FAILED - network not restored");
    }
}