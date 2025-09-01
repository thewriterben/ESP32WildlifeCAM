/**
 * Mesh Time Synchronization Implementation
 * Phase 2 Enhancement - Time synchronization across mesh nodes
 * 
 * Features:
 * - Distributed time synchronization
 * - Clock drift compensation
 * - Multiple time sources support
 * - Precision time protocol adaptation
 */

#include "../../include/network/mesh_protocol.h"
#include <vector>
#include <map>

#ifndef MESH_SYNCHRONIZATION_H
#define MESH_SYNCHRONIZATION_H

class MeshTimeSynchronizer {
private:
    struct TimeReference {
        uint32_t nodeId;
        uint32_t referenceTime;     // UTC timestamp
        uint32_t localTime;         // Local time when reference was received
        int32_t clockOffset;        // Offset from reference clock
        float clockDrift;           // Clock drift rate (ppm)
        uint8_t stratum;            // Distance from primary time source
        uint8_t accuracy;           // Time accuracy (0-100%)
        uint32_t lastSync;          // When this reference was last updated
        float reliability;          // Reliability of this time source
        
        TimeReference() : nodeId(0), referenceTime(0), localTime(0), 
                         clockOffset(0), clockDrift(0.0), stratum(255),
                         accuracy(0), lastSync(0), reliability(0.0) {}
    };
    
    struct SyncMessage {
        uint8_t messageType;        // Sync request, response, announce
        uint32_t sourceNode;
        uint32_t originTimestamp;   // T1 - when message was sent
        uint32_t receiveTimestamp;  // T2 - when message was received
        uint32_t transmitTimestamp; // T3 - when response was sent
        uint32_t destinationTimestamp; // T4 - when response was received
        uint8_t stratum;
        float accuracy;
        
        SyncMessage() : messageType(0), sourceNode(0), originTimestamp(0),
                       receiveTimestamp(0), transmitTimestamp(0), 
                       destinationTimestamp(0), stratum(255), accuracy(0.0) {}
    };
    
    uint32_t localNodeId_;
    bool synchronizationEnabled_;
    bool isTimeSource_;
    uint8_t localStratum_;
    
    TimeReference primaryReference_;
    std::map<uint32_t, TimeReference> timeReferences_;
    std::vector<SyncMessage> pendingSyncMessages_;
    
    uint32_t lastSyncBroadcast_;
    uint32_t lastSyncRequest_;
    uint32_t syncInterval_;
    uint32_t syncTimeout_;
    
    int32_t cumulativeOffset_;
    float averageDrift_;
    uint32_t syncCount_;
    uint32_t lastClockAdjustment_;

public:
    MeshTimeSynchronizer();
    ~MeshTimeSynchronizer();
    
    // Initialization and configuration
    bool init(uint32_t nodeId, bool isTimeSource = false);
    void cleanup();
    void enableSynchronization(bool enabled) { synchronizationEnabled_ = enabled; }
    void setAsTimeSource(bool isSource, uint8_t stratum = 1);
    void setSyncInterval(uint32_t intervalMs) { syncInterval_ = intervalMs; }
    void setSyncTimeout(uint32_t timeoutMs) { syncTimeout_ = timeoutMs; }
    
    // Time synchronization operations
    void performTimeSynchronization();
    void broadcastTimeAnnouncement();
    void requestTimeSync(uint32_t targetNode = 0);  // 0 = broadcast
    void processTimeSyncMessage(const SyncMessage& message, int8_t rssi, float snr);
    
    // Time management
    uint32_t getNetworkTime() const;
    uint32_t getLocalTime() const { return millis(); }
    int32_t getClockOffset() const { return primaryReference_.clockOffset; }
    float getClockDrift() const { return averageDrift_; }
    uint8_t getTimeAccuracy() const { return primaryReference_.accuracy; }
    uint8_t getStratum() const { return localStratum_; }
    
    // Time source management
    void addTimeReference(uint32_t nodeId, const TimeReference& reference);
    void removeTimeReference(uint32_t nodeId);
    void updateTimeReference(uint32_t nodeId, uint32_t referenceTime, uint8_t stratum, float accuracy);
    TimeReference getBestTimeReference() const;
    std::vector<TimeReference> getTimeReferences() const;
    
    // Clock adjustment
    void adjustSystemClock(int32_t offsetMs);
    void compensateClockDrift();
    void calibrateClock();
    void resetClockSync();
    
    // Synchronization quality
    bool isTimeSynchronized() const;
    float getSynchronizationQuality() const;
    uint32_t getTimeSinceLastSync() const;
    void validateTimeReferences();
    
    // Network time protocol (simplified)
    void handleSyncRequest(const SyncMessage& request);
    void handleSyncResponse(const SyncMessage& response);
    void handleTimeAnnouncement(const SyncMessage& announcement);
    void sendSyncResponse(uint32_t requesterNode, const SyncMessage& request);
    
    // Clock drift analysis
    void measureClockDrift();
    void updateDriftEstimate(int32_t offset, uint32_t interval);
    float predictClockOffset(uint32_t futureTime) const;
    void adaptSyncInterval();
    
    // Statistics and monitoring
    void updateSyncStatistics();
    String getSyncReport() const;
    uint32_t getSyncCount() const { return syncCount_; }
    float getAverageOffset() const;
    float getMaxOffset() const;
    
    // Periodic tasks
    void performPeriodicTasks();
    void maintainTimeReferences();
    void optimizeSyncSchedule();

private:
    // Time calculation algorithms
    int32_t calculateClockOffset(const SyncMessage& syncData);
    float calculateRoundTripDelay(const SyncMessage& syncData);
    uint32_t interpolateNetworkTime() const;
    void applyClockCorrection(int32_t correction);
    
    // Reference selection
    TimeReference selectBestReference(const std::vector<TimeReference>& references);
    float calculateReferenceQuality(const TimeReference& reference);
    bool isReferenceBetter(const TimeReference& newRef, const TimeReference& currentRef);
    
    // Message handling
    SyncMessage createSyncRequest(uint32_t targetNode);
    SyncMessage createSyncResponse(const SyncMessage& request);
    SyncMessage createTimeAnnouncement();
    void broadcastSyncMessage(const SyncMessage& message);
    
    // Clock drift compensation
    void updateClockModel();
    int32_t predictedOffset(uint32_t targetTime) const;
    void adjustForDrift();
    
    // Validation and filtering
    bool validateSyncMessage(const SyncMessage& message);
    bool isTimeReferenceValid(const TimeReference& reference);
    void filterOutliers();
    void detectClockSteps();
    
    // Utility functions
    uint32_t getCurrentTimestamp() const;
    bool isMessageStale(const SyncMessage& message) const;
    void cleanupStaleReferences();
    float calculateSyncAccuracy(const TimeReference& reference);
};

// Time synchronization events
enum TimeSyncEvent {
    TIME_SYNC_STARTED,
    TIME_SYNC_COMPLETED,
    TIME_SYNC_FAILED,
    TIME_SOURCE_CHANGED,
    CLOCK_ADJUSTED,
    SYNC_ACCURACY_IMPROVED,
    SYNC_ACCURACY_DEGRADED
};

// Callback function types
typedef void (*TimeSyncEventCallback)(TimeSyncEvent event, uint32_t nodeId, int32_t offset);
typedef void (*TimeSourceCallback)(uint32_t oldSource, uint32_t newSource);

// Global synchronizer instance
extern MeshTimeSynchronizer meshTimeSync;

// Callback registration
void setTimeSyncEventCallback(TimeSyncEventCallback callback);
void setTimeSourceCallback(TimeSourceCallback callback);

#endif // MESH_SYNCHRONIZATION_H