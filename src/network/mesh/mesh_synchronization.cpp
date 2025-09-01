/**
 * @file mesh_synchronization.cpp
 * @brief Time Synchronization Across Mesh Nodes for Phase 2
 * 
 * This implementation provides network-wide time synchronization using
 * a hierarchical approach with coordinator-based timing and precision
 * drift compensation.
 * 
 * Phase 2 Enhancement - March 31, 2026 Target
 */

#include "../../../include/network/mesh_protocol.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <time.h>

// ===========================
// TIME SYNCHRONIZATION ENGINE
// ===========================

class MeshSynchronizationEngine {
private:
    // Time sync state
    TimeSync timeSync_;
    uint32_t nodeId_;
    bool isCoordinator_;
    
    // Sync parameters
    uint32_t syncInterval_;
    uint32_t syncTimeout_;
    uint16_t maxSyncError_;
    uint8_t syncAttempts_;
    uint8_t maxSyncAttempts_;
    
    // Timing measurements
    std::vector<int32_t> offsetHistory_;
    std::vector<uint32_t> latencyHistory_;
    uint32_t lastSyncRequest_;
    uint32_t roundTripTime_;
    
    // Drift compensation
    float clockDriftRate_;
    uint32_t lastDriftCalibration_;
    int32_t cumulativeDrift_;
    
    // External time sources
    bool ntpEnabled_;
    String ntpServer_;
    uint32_t lastNtpSync_;
    
public:
    MeshSynchronizationEngine(uint32_t nodeId) : 
        nodeId_(nodeId), isCoordinator_(false), syncInterval_(MESH_SYNC_INTERVAL),
        syncTimeout_(5000), maxSyncError_(100), syncAttempts_(0), maxSyncAttempts_(3),
        lastSyncRequest_(0), roundTripTime_(0), clockDriftRate_(0.0),
        lastDriftCalibration_(0), cumulativeDrift_(0), ntpEnabled_(false),
        ntpServer_("pool.ntp.org"), lastNtpSync_(0) {
        
        // Initialize time sync structure
        memset(&timeSync_, 0, sizeof(timeSync_));
        timeSync_.isSynced = false;
        
        // Reserve space for history
        offsetHistory_.reserve(10);
        latencyHistory_.reserve(10);
    }
    
    // ===========================
    // TIME SYNCHRONIZATION
    // ===========================
    
    /**
     * Initialize time synchronization
     */
    bool initialize() {
        // If we're a coordinator, try to sync with NTP first
        if (isCoordinator_ && WiFi.status() == WL_CONNECTED) {
            if (syncWithNTP()) {
                timeSync_.isSynced = true;
                timeSync_.coordinatorTime = getCurrentTime();
                timeSync_.localOffset = 0;
                timeSync_.lastSync = millis();
                timeSync_.syncAccuracy = 50; // 50ms accuracy from NTP
                
                Serial.println("Coordinator synced with NTP");
                return true;
            }
        }
        
        // For non-coordinators or if NTP fails, mark as needing sync
        timeSync_.isSynced = false;
        
        return true;
    }
    
    /**
     * Start time synchronization process
     */
    bool startSynchronization(uint32_t coordinatorId) {
        uint32_t currentTime = millis();
        
        // Rate limiting
        if (currentTime - lastSyncRequest_ < syncInterval_) {
            return true;
        }
        
        // If we are the coordinator, broadcast time sync
        if (isCoordinator_) {
            return broadcastTimeSync();
        }
        
        // Request time sync from coordinator
        return requestTimeSync(coordinatorId);
    }
    
    /**
     * Broadcast time synchronization (coordinator only)
     */
    bool broadcastTimeSync() {
        if (!isCoordinator_) return false;
        
        uint32_t currentTime = getCurrentTime();
        uint32_t localTime = millis();
        
        // Create time sync packet
        DynamicJsonDocument syncPacket(512);
        syncPacket["type"] = "time_sync_broadcast";
        syncPacket["coordinator"] = nodeId_;
        syncPacket["coordinator_time"] = currentTime;
        syncPacket["local_time"] = localTime;
        syncPacket["timestamp"] = localTime;
        syncPacket["sync_accuracy"] = timeSync_.syncAccuracy;
        syncPacket["drift_rate"] = clockDriftRate_;
        
        // Add timing information for precision
        syncPacket["sync_sequence"] = ++syncAttempts_;
        syncPacket["ntp_last_sync"] = lastNtpSync_;
        
        String message;
        serializeJson(syncPacket, message);
        
        Serial.printf("Broadcasting time sync: time=%u, accuracy=%dms\n", 
                     currentTime, timeSync_.syncAccuracy);
        
        return broadcastMessage(message);
    }
    
    /**
     * Request time synchronization from coordinator
     */
    bool requestTimeSync(uint32_t coordinatorId) {
        DynamicJsonDocument syncRequest(512);
        syncRequest["type"] = "time_sync_request";
        syncRequest["requester"] = nodeId_;
        syncRequest["coordinator"] = coordinatorId;
        syncRequest["request_time"] = millis();
        syncRequest["local_accuracy"] = timeSync_.syncAccuracy;
        syncRequest["last_sync"] = timeSync_.lastSync;
        
        String message;
        serializeJson(syncRequest, message);
        
        lastSyncRequest_ = millis();
        syncAttempts_++;
        
        Serial.printf("Requesting time sync from coordinator %08X (attempt %d)\n", 
                     coordinatorId, syncAttempts_);
        
        return sendToNode(coordinatorId, message);
    }
    
    /**
     * Process time sync broadcast
     */
    bool processTimeSyncBroadcast(const JsonObject& syncPacket) {
        uint32_t coordinatorTime = syncPacket["coordinator_time"];
        uint32_t coordinatorLocalTime = syncPacket["local_time"];
        uint32_t receiveTime = millis();
        uint16_t accuracy = syncPacket["sync_accuracy"];
        float driftRate = syncPacket["drift_rate"];
        
        // Calculate transmission delay (estimate)
        uint32_t transmissionDelay = receiveTime - coordinatorLocalTime;
        
        // Adjust coordinator time for transmission delay
        uint32_t adjustedCoordinatorTime = coordinatorTime + transmissionDelay;
        
        // Calculate time offset
        int32_t offset = adjustedCoordinatorTime - receiveTime;
        
        // Apply time synchronization
        return applyTimeSync(coordinatorTime, offset, accuracy, transmissionDelay);
    }
    
    /**
     * Process time sync request (coordinator only)
     */
    bool processTimeSyncRequest(const JsonObject& syncRequest) {
        if (!isCoordinator_) return false;
        
        uint32_t requester = syncRequest["requester"];
        uint32_t requestTime = syncRequest["request_time"];
        uint32_t currentTime = getCurrentTime();
        uint32_t localTime = millis();
        
        // Calculate round-trip time estimate
        uint32_t rtt = localTime - requestTime;
        
        // Create time sync response
        DynamicJsonDocument syncResponse(512);
        syncResponse["type"] = "time_sync_response";
        syncResponse["coordinator"] = nodeId_;
        syncResponse["requester"] = requester;
        syncResponse["coordinator_time"] = currentTime;
        syncResponse["local_time"] = localTime;
        syncResponse["request_time"] = requestTime;
        syncResponse["response_time"] = localTime;
        syncResponse["estimated_rtt"] = rtt;
        syncResponse["sync_accuracy"] = timeSync_.syncAccuracy;
        syncResponse["drift_rate"] = clockDriftRate_;
        
        String message;
        serializeJson(syncResponse, message);
        
        Serial.printf("Sending time sync response to %08X (RTT: %ums)\n", requester, rtt);
        
        return sendToNode(requester, message);
    }
    
    /**
     * Process time sync response
     */
    bool processTimeSyncResponse(const JsonObject& syncResponse) {
        uint32_t coordinatorTime = syncResponse["coordinator_time"];
        uint32_t responseTime = syncResponse["response_time"];
        uint32_t requestTime = syncResponse["request_time"];
        uint32_t receiveTime = millis();
        uint16_t accuracy = syncResponse["sync_accuracy"];
        
        // Calculate round-trip time
        roundTripTime_ = receiveTime - requestTime;
        
        // Estimate one-way delay (half of RTT)
        uint32_t oneWayDelay = roundTripTime_ / 2;
        
        // Adjust coordinator time for propagation delay
        uint32_t adjustedCoordinatorTime = coordinatorTime + oneWayDelay;
        
        // Calculate time offset
        int32_t offset = adjustedCoordinatorTime - receiveTime;
        
        // Apply time synchronization
        return applyTimeSync(coordinatorTime, offset, accuracy, oneWayDelay);
    }
    
    // ===========================
    // TIME MANAGEMENT
    // ===========================
    
    /**
     * Apply time synchronization
     */
    bool applyTimeSync(uint32_t coordinatorTime, int32_t offset, 
                      uint16_t accuracy, uint32_t propagationDelay) {
        
        // Store offset in history for drift calculation
        offsetHistory_.push_back(offset);
        latencyHistory_.push_back(propagationDelay);
        
        // Keep history manageable
        if (offsetHistory_.size() > 10) {
            offsetHistory_.erase(offsetHistory_.begin());
            latencyHistory_.erase(latencyHistory_.begin());
        }
        
        // Calculate average offset for stability
        int32_t avgOffset = calculateAverageOffset();
        
        // Only apply sync if offset is significant enough
        if (abs(avgOffset) > 10) { // 10ms threshold
            timeSync_.coordinatorTime = coordinatorTime;
            timeSync_.localOffset = avgOffset;
            timeSync_.lastSync = millis();
            timeSync_.syncAccuracy = accuracy + propagationDelay;
            timeSync_.isSynced = true;
            
            // Update drift compensation
            updateDriftCompensation();
            
            syncAttempts_ = 0; // Reset attempt counter on success
            
            Serial.printf("Time sync applied: offset=%dms, accuracy=%dms\n", 
                         avgOffset, timeSync_.syncAccuracy);
            
            return true;
        }
        
        // Even if we don't apply offset, update sync status
        timeSync_.lastSync = millis();
        timeSync_.isSynced = true;
        
        return true;
    }
    
    /**
     * Get synchronized time
     */
    uint32_t getSynchronizedTime() const {
        if (!timeSync_.isSynced) {
            return millis(); // Fall back to local time
        }
        
        uint32_t timeSinceSync = millis() - timeSync_.lastSync;
        uint32_t adjustedTime = timeSync_.coordinatorTime + timeSinceSync;
        
        // Apply drift compensation
        int32_t driftCompensation = (int32_t)(timeSinceSync * clockDriftRate_);
        adjustedTime += driftCompensation;
        
        // Apply local offset
        adjustedTime += timeSync_.localOffset;
        
        return adjustedTime;
    }
    
    /**
     * Check if time is synchronized
     */
    bool isSynchronized() const {
        if (!timeSync_.isSynced) return false;
        
        // Check if sync is still valid
        uint32_t timeSinceSync = millis() - timeSync_.lastSync;
        return timeSinceSync < syncInterval_ * 2; // Allow some margin
    }
    
    /**
     * Get time sync status
     */
    TimeSync getTimeSyncStatus() const {
        return timeSync_;
    }
    
    /**
     * Set coordinator status
     */
    void setCoordinator(bool isCoordinator) {
        isCoordinator_ = isCoordinator;
        
        if (isCoordinator_) {
            // Try to sync with NTP if possible
            if (WiFi.status() == WL_CONNECTED) {
                syncWithNTP();
            }
        }
    }
    
    // ===========================
    // DRIFT COMPENSATION
    // ===========================
    
    /**
     * Update clock drift compensation
     */
    void updateDriftCompensation() {
        if (offsetHistory_.size() < 3) return; // Need at least 3 samples
        
        uint32_t currentTime = millis();
        
        // Calculate drift rate from offset history
        if (currentTime - lastDriftCalibration_ > 300000) { // 5 minutes
            float totalDrift = 0.0;
            uint32_t totalTime = 0;
            
            for (size_t i = 1; i < offsetHistory_.size(); i++) {
                int32_t offsetDiff = offsetHistory_[i] - offsetHistory_[i-1];
                uint32_t timeDiff = 60000; // Assume 1 minute between samples
                
                totalDrift += offsetDiff;
                totalTime += timeDiff;
            }
            
            if (totalTime > 0) {
                clockDriftRate_ = totalDrift / totalTime;
                lastDriftCalibration_ = currentTime;
                
                Serial.printf("Updated clock drift rate: %.6f ms/ms\n", clockDriftRate_);
            }
        }
    }
    
    /**
     * Calculate average offset from history
     */
    int32_t calculateAverageOffset() const {
        if (offsetHistory_.empty()) return 0;
        
        int64_t sum = 0;
        for (int32_t offset : offsetHistory_) {
            sum += offset;
        }
        
        return (int32_t)(sum / offsetHistory_.size());
    }
    
    // ===========================
    // NTP SYNCHRONIZATION
    // ===========================
    
    /**
     * Synchronize with NTP server (coordinator only)
     */
    bool syncWithNTP() {
        if (!ntpEnabled_ || WiFi.status() != WL_CONNECTED) {
            return false;
        }
        
        // Configure NTP
        configTime(0, 0, ntpServer_.c_str());
        
        // Wait for NTP sync (with timeout)
        uint32_t startTime = millis();
        struct tm timeinfo;
        
        while (!getLocalTime(&timeinfo)) {
            if (millis() - startTime > 10000) { // 10 second timeout
                Serial.println("NTP sync timeout");
                return false;
            }
            delay(100);
        }
        
        // Get UTC time
        time_t now;
        time(&now);
        
        // Update our time base
        timeSync_.coordinatorTime = (uint32_t)now;
        timeSync_.localOffset = 0;
        timeSync_.lastSync = millis();
        timeSync_.syncAccuracy = 50; // NTP typically 50ms accuracy
        timeSync_.isSynced = true;
        
        lastNtpSync_ = millis();
        
        Serial.printf("NTP sync successful: UTC time = %u\n", (uint32_t)now);
        
        return true;
    }
    
    /**
     * Enable/disable NTP synchronization
     */
    void enableNTP(bool enable, const char* server = nullptr) {
        ntpEnabled_ = enable;
        if (server) {
            ntpServer_ = server;
        }
    }
    
    // ===========================
    // TIMING UTILITIES
    // ===========================
    
    /**
     * Get current time (local or synchronized)
     */
    uint32_t getCurrentTime() const {
        if (isCoordinator_ && timeSync_.isSynced) {
            // For coordinator, use adjusted time
            uint32_t timeSinceSync = millis() - timeSync_.lastSync;
            return timeSync_.coordinatorTime + timeSinceSync;
        } else {
            return getSynchronizedTime();
        }
    }
    
    /**
     * Get timing statistics
     */
    void getTimingStatistics(uint32_t& lastSyncTime, uint16_t& accuracy, 
                           float& driftRate, uint32_t& roundTripTime) const {
        lastSyncTime = timeSync_.lastSync;
        accuracy = timeSync_.syncAccuracy;
        driftRate = clockDriftRate_;
        roundTripTime = roundTripTime_;
    }
    
    /**
     * Convert synchronized time to human readable format
     */
    String formatSynchronizedTime() const {
        uint32_t syncTime = getSynchronizedTime();
        
        // Convert to human readable format
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%u.%03u", 
                syncTime / 1000, syncTime % 1000);
        
        return String(buffer);
    }
    
private:
    // ===========================
    // HELPER FUNCTIONS
    // ===========================
    
    bool broadcastMessage(const String& message) {
        // Interface with actual LoRa transmission
        return true;
    }
    
    bool sendToNode(uint32_t nodeId, const String& message) {
        // Interface with routing to send to specific node
        return true;
    }
};

// ===========================
// PUBLIC API
// ===========================

static MeshSynchronizationEngine* g_syncEngine = nullptr;

namespace MeshSynchronization {
    bool initialize(uint32_t nodeId) {
        if (g_syncEngine) {
            delete g_syncEngine;
        }
        
        g_syncEngine = new MeshSynchronizationEngine(nodeId);
        Serial.printf("Mesh synchronization engine initialized for node %08X\n", nodeId);
        return g_syncEngine->initialize();
    }
    
    void cleanup() {
        if (g_syncEngine) {
            delete g_syncEngine;
            g_syncEngine = nullptr;
        }
    }
    
    bool startSync(uint32_t coordinatorId) {
        return g_syncEngine ? g_syncEngine->startSynchronization(coordinatorId) : false;
    }
    
    bool isSynchronized() {
        return g_syncEngine ? g_syncEngine->isSynchronized() : false;
    }
    
    uint32_t getSynchronizedTime() {
        return g_syncEngine ? g_syncEngine->getSynchronizedTime() : millis();
    }
    
    TimeSync getTimeSyncStatus() {
        return g_syncEngine ? g_syncEngine->getTimeSyncStatus() : TimeSync{};
    }
    
    void setCoordinator(bool isCoordinator) {
        if (g_syncEngine) {
            g_syncEngine->setCoordinator(isCoordinator);
        }
    }
    
    void enableNTP(bool enable, const char* server) {
        if (g_syncEngine) {
            g_syncEngine->enableNTP(enable, server);
        }
    }
    
    String formatTime() {
        return g_syncEngine ? g_syncEngine->formatSynchronizedTime() : String(millis());
    }
    
    void getStatistics(uint32_t& lastSyncTime, uint16_t& accuracy, 
                      float& driftRate, uint32_t& roundTripTime) {
        if (g_syncEngine) {
            g_syncEngine->getTimingStatistics(lastSyncTime, accuracy, driftRate, roundTripTime);
        } else {
            lastSyncTime = 0;
            accuracy = 0;
            driftRate = 0.0;
            roundTripTime = 0;
        }
    }
}