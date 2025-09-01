/**
 * Mesh Time Synchronization Implementation
 * Phase 2 Enhancement - Time synchronization across mesh nodes
 */

#include "mesh_synchronization.h"
#include <ArduinoJson.h>

// Global synchronizer instance
MeshTimeSynchronizer meshTimeSync;

// Callback function pointers
static TimeSyncEventCallback g_timeSyncEventCallback = nullptr;
static TimeSourceCallback g_timeSourceCallback = nullptr;

MeshTimeSynchronizer::MeshTimeSynchronizer() 
    : localNodeId_(0), synchronizationEnabled_(true), isTimeSource_(false),
      localStratum_(255), lastSyncBroadcast_(0), lastSyncRequest_(0),
      syncInterval_(120000), syncTimeout_(30000), cumulativeOffset_(0),
      averageDrift_(0.0), syncCount_(0), lastClockAdjustment_(0) {
}

MeshTimeSynchronizer::~MeshTimeSynchronizer() {
    cleanup();
}

bool MeshTimeSynchronizer::init(uint32_t nodeId, bool isTimeSource) {
    localNodeId_ = nodeId;
    isTimeSource_ = isTimeSource;
    synchronizationEnabled_ = true;
    
    if (isTimeSource) {
        localStratum_ = 1;  // Primary time source
        primaryReference_.nodeId = nodeId;
        primaryReference_.stratum = 1;
        primaryReference_.accuracy = 95;
        primaryReference_.reliability = 1.0;
        primaryReference_.lastSync = millis();
    } else {
        localStratum_ = 255;  // Unsynchronized
    }
    
    Serial.println("Time Synchronizer initialized for node " + String(nodeId) + 
                  (isTimeSource ? " (Time Source)" : ""));
    
    return true;
}

void MeshTimeSynchronizer::cleanup() {
    timeReferences_.clear();
    pendingSyncMessages_.clear();
    synchronizationEnabled_ = false;
}

void MeshTimeSynchronizer::performTimeSynchronization() {
    if (!synchronizationEnabled_) return;
    
    uint32_t now = millis();
    
    // Time sources broadcast announcements
    if (isTimeSource_ && (now - lastSyncBroadcast_ > syncInterval_)) {
        broadcastTimeAnnouncement();
        lastSyncBroadcast_ = now;
    }
    
    // Non-time sources request synchronization
    if (!isTimeSource_ && (now - lastSyncRequest_ > syncInterval_)) {
        requestTimeSync();
        lastSyncRequest_ = now;
    }
    
    // Maintain time references
    maintainTimeReferences();
    
    // Apply clock drift compensation
    if (isTimeSynchronized() && (now - lastClockAdjustment_ > 60000)) {
        compensateClockDrift();
        lastClockAdjustment_ = now;
    }
}

void MeshTimeSynchronizer::broadcastTimeAnnouncement() {
    if (!isTimeSource_) return;
    
    SyncMessage announcement = createTimeAnnouncement();
    broadcastSyncMessage(announcement);
    
    Serial.println("Broadcasting time announcement (stratum " + String(localStratum_) + ")");
}

void MeshTimeSynchronizer::requestTimeSync(uint32_t targetNode) {
    if (isTimeSource_) return;  // Time sources don't request sync
    
    SyncMessage request = createSyncRequest(targetNode);
    broadcastSyncMessage(request);
    
    pendingSyncMessages_.push_back(request);
    
    Serial.println("Requesting time synchronization");
}

void MeshTimeSynchronizer::processTimeSyncMessage(const SyncMessage& message, int8_t rssi, float snr) {
    if (!validateSyncMessage(message)) {
        return;
    }
    
    switch (message.messageType) {
        case 1: // Sync request
            handleSyncRequest(message);
            break;
        case 2: // Sync response
            handleSyncResponse(message);
            break;
        case 3: // Time announcement
            handleTimeAnnouncement(message);
            break;
        default:
            Serial.println("Unknown sync message type: " + String(message.messageType));
            break;
    }
}

uint32_t MeshTimeSynchronizer::getNetworkTime() const {
    if (!isTimeSynchronized()) {
        return millis();  // Fall back to local time
    }
    
    return interpolateNetworkTime();
}

bool MeshTimeSynchronizer::isTimeSynchronized() const {
    if (isTimeSource_) {
        return true;  // Time sources are always synchronized
    }
    
    uint32_t now = millis();
    return primaryReference_.nodeId != 0 && 
           primaryReference_.accuracy > 50 &&
           (now - primaryReference_.lastSync) < (syncTimeout_ * 2);
}

float MeshTimeSynchronizer::getSynchronizationQuality() const {
    if (isTimeSource_) {
        return 1.0;  // Perfect quality for time sources
    }
    
    if (!isTimeSynchronized()) {
        return 0.0;
    }
    
    // Quality decreases with time since last sync and stratum
    uint32_t timeSinceSync = millis() - primaryReference_.lastSync;
    float timeQuality = 1.0 - (float(timeSinceSync) / float(syncTimeout_ * 2));
    float stratumQuality = 1.0 - (float(primaryReference_.stratum) / 16.0);
    float accuracyQuality = float(primaryReference_.accuracy) / 100.0;
    
    return (timeQuality + stratumQuality + accuracyQuality) / 3.0;
}

void MeshTimeSynchronizer::addTimeReference(uint32_t nodeId, const TimeReference& reference) {
    if (nodeId == localNodeId_) return;  // Don't add self as reference
    
    timeReferences_[nodeId] = reference;
    
    // Check if this is a better time source
    if (!isTimeSource_ && isReferenceBetter(reference, primaryReference_)) {
        uint32_t oldSource = primaryReference_.nodeId;
        primaryReference_ = reference;
        localStratum_ = reference.stratum + 1;
        
        Serial.println("Switched to better time source: " + String(nodeId) + 
                      " (stratum " + String(reference.stratum) + ")");
        
        if (g_timeSourceCallback && oldSource != nodeId) {
            g_timeSourceCallback(oldSource, nodeId);
        }
        
        if (g_timeSyncEventCallback) {
            g_timeSyncEventCallback(TIME_SOURCE_CHANGED, nodeId, getClockOffset());
        }
    }
}

void MeshTimeSynchronizer::handleTimeAnnouncement(const SyncMessage& announcement) {
    TimeReference reference;
    reference.nodeId = announcement.sourceNode;
    reference.referenceTime = announcement.originTimestamp;
    reference.localTime = millis();
    reference.stratum = announcement.stratum;
    reference.accuracy = static_cast<uint8_t>(announcement.accuracy);
    reference.lastSync = millis();
    reference.reliability = 0.8;  // Default reliability
    
    // Calculate offset
    reference.clockOffset = int32_t(announcement.originTimestamp) - int32_t(reference.localTime);
    
    addTimeReference(announcement.sourceNode, reference);
}

void MeshTimeSynchronizer::handleSyncRequest(const SyncMessage& request) {
    if (!isTimeSynchronized()) {
        return;  // Can't provide sync if we're not synchronized
    }
    
    // Send sync response
    sendSyncResponse(request.sourceNode, request);
}

void MeshTimeSynchronizer::handleSyncResponse(const SyncMessage& response) {
    // Find matching pending request
    for (auto it = pendingSyncMessages_.begin(); it != pendingSyncMessages_.end(); ++it) {
        if (it->sourceNode == localNodeId_ && response.sourceNode != 0) {
            // Calculate clock offset using simplified NTP algorithm
            int32_t offset = calculateClockOffset(response);
            
            // Update time reference
            TimeReference reference;
            reference.nodeId = response.sourceNode;
            reference.referenceTime = response.transmitTimestamp;
            reference.localTime = millis();
            reference.clockOffset = offset;
            reference.stratum = response.stratum;
            reference.accuracy = static_cast<uint8_t>(response.accuracy);
            reference.lastSync = millis();
            reference.reliability = 0.9;
            
            addTimeReference(response.sourceNode, reference);
            
            // Apply clock adjustment if significant
            if (abs(offset) > 100) {  // 100ms threshold
                adjustSystemClock(offset);
            }
            
            pendingSyncMessages_.erase(it);
            syncCount_++;
            
            if (g_timeSyncEventCallback) {
                g_timeSyncEventCallback(TIME_SYNC_COMPLETED, response.sourceNode, offset);
            }
            
            break;
        }
    }
}

void MeshTimeSynchronizer::sendSyncResponse(uint32_t requesterNode, const SyncMessage& request) {
    SyncMessage response = createSyncResponse(request);
    response.destinationTimestamp = millis();
    
    // Send response (implementation would depend on mesh networking layer)
    Serial.println("Sending sync response to node " + String(requesterNode));
}

int32_t MeshTimeSynchronizer::calculateClockOffset(const SyncMessage& syncData) {
    // Simplified NTP offset calculation
    // offset = ((T2 - T1) + (T3 - T4)) / 2
    // where T1=origin, T2=receive, T3=transmit, T4=destination
    
    int32_t t1 = int32_t(syncData.originTimestamp);
    int32_t t2 = int32_t(syncData.receiveTimestamp);
    int32_t t3 = int32_t(syncData.transmitTimestamp);
    int32_t t4 = int32_t(syncData.destinationTimestamp);
    
    return ((t2 - t1) + (t3 - t4)) / 2;
}

uint32_t MeshTimeSynchronizer::interpolateNetworkTime() const {
    if (primaryReference_.nodeId == 0) {
        return millis();
    }
    
    uint32_t localNow = millis();
    uint32_t timeSinceReference = localNow - primaryReference_.localTime;
    
    // Apply offset and drift compensation
    int32_t networkTime = int32_t(primaryReference_.referenceTime + timeSinceReference);
    networkTime += primaryReference_.clockOffset;
    
    // Simple drift compensation
    if (averageDrift_ != 0.0) {
        float driftCorrection = averageDrift_ * float(timeSinceReference) / 1000000.0;  // ppm to ms
        networkTime += int32_t(driftCorrection);
    }
    
    return uint32_t(networkTime);
}

void MeshTimeSynchronizer::adjustSystemClock(int32_t offsetMs) {
    cumulativeOffset_ += offsetMs;
    
    // Update drift estimate
    if (syncCount_ > 1) {
        updateDriftEstimate(offsetMs, syncInterval_);
    }
    
    Serial.println("Clock adjusted by " + String(offsetMs) + "ms");
    
    if (g_timeSyncEventCallback) {
        g_timeSyncEventCallback(CLOCK_ADJUSTED, primaryReference_.nodeId, offsetMs);
    }
}

void MeshTimeSynchronizer::updateDriftEstimate(int32_t offset, uint32_t interval) {
    // Calculate drift rate in parts per million (ppm)
    float drift = (float(offset) / float(interval)) * 1000000.0;
    
    // Exponential moving average
    if (syncCount_ == 1) {
        averageDrift_ = drift;
    } else {
        averageDrift_ = (averageDrift_ * 0.8) + (drift * 0.2);
    }
}

void MeshTimeSynchronizer::compensateClockDrift() {
    if (averageDrift_ == 0.0) return;
    
    uint32_t timeSinceLastAdjustment = millis() - lastClockAdjustment_;
    float driftCorrection = averageDrift_ * float(timeSinceLastAdjustment) / 1000000.0;
    
    if (abs(driftCorrection) > 1.0) {  // Only apply if significant
        primaryReference_.clockOffset += int32_t(driftCorrection);
        
        Serial.println("Applied drift compensation: " + String(driftCorrection) + "ms");
    }
}

SyncMessage MeshTimeSynchronizer::createTimeAnnouncement() {
    SyncMessage announcement;
    announcement.messageType = 3;  // Time announcement
    announcement.sourceNode = localNodeId_;
    announcement.originTimestamp = getNetworkTime();
    announcement.stratum = localStratum_;
    announcement.accuracy = 95.0;  // High accuracy for time source
    
    return announcement;
}

SyncMessage MeshTimeSynchronizer::createSyncRequest(uint32_t targetNode) {
    SyncMessage request;
    request.messageType = 1;  // Sync request
    request.sourceNode = localNodeId_;
    request.originTimestamp = millis();
    request.stratum = localStratum_;
    request.accuracy = getSynchronizationQuality() * 100.0;
    
    return request;
}

SyncMessage MeshTimeSynchronizer::createSyncResponse(const SyncMessage& request) {
    SyncMessage response;
    response.messageType = 2;  // Sync response
    response.sourceNode = localNodeId_;
    response.originTimestamp = request.originTimestamp;
    response.receiveTimestamp = millis();
    response.transmitTimestamp = millis();
    response.stratum = localStratum_;
    response.accuracy = getSynchronizationQuality() * 100.0;
    
    return response;
}

void MeshTimeSynchronizer::broadcastSyncMessage(const SyncMessage& message) {
    // This would be implemented by the mesh networking layer
    // For now, just log the message
    Serial.println("Broadcasting sync message type " + String(message.messageType));
}

bool MeshTimeSynchronizer::validateSyncMessage(const SyncMessage& message) {
    // Basic validation
    if (message.sourceNode == localNodeId_) {
        return false;  // Ignore own messages
    }
    
    if (message.stratum >= 16) {
        return false;  // Invalid stratum
    }
    
    if (message.accuracy > 100.0) {
        return false;  // Invalid accuracy
    }
    
    return true;
}

bool MeshTimeSynchronizer::isReferenceBetter(const TimeReference& newRef, const TimeReference& currentRef) {
    if (currentRef.nodeId == 0) {
        return true;  // Any reference is better than none
    }
    
    // Lower stratum is better
    if (newRef.stratum < currentRef.stratum) {
        return true;
    } else if (newRef.stratum > currentRef.stratum) {
        return false;
    }
    
    // Same stratum - compare accuracy and reliability
    float newQuality = newRef.accuracy * newRef.reliability;
    float currentQuality = currentRef.accuracy * currentRef.reliability;
    
    return newQuality > currentQuality;
}

void MeshTimeSynchronizer::maintainTimeReferences() {
    uint32_t now = millis();
    
    // Remove stale references
    for (auto it = timeReferences_.begin(); it != timeReferences_.end();) {
        if (now - it->second.lastSync > syncTimeout_ * 3) {
            Serial.println("Removing stale time reference: " + String(it->first));
            it = timeReferences_.erase(it);
        } else {
            ++it;
        }
    }
    
    // Check if primary reference is still valid
    if (primaryReference_.nodeId != 0 && !isTimeSynchronized()) {
        Serial.println("Primary time reference lost - searching for new source");
        primaryReference_ = TimeReference();  // Reset
        localStratum_ = 255;
        
        // Find best available reference
        TimeReference bestRef = getBestTimeReference();
        if (bestRef.nodeId != 0) {
            primaryReference_ = bestRef;
            localStratum_ = bestRef.stratum + 1;
        }
    }
}

TimeReference MeshTimeSynchronizer::getBestTimeReference() const {
    TimeReference best;
    
    for (const auto& pair : timeReferences_) {
        if (isReferenceBetter(pair.second, best)) {
            best = pair.second;
        }
    }
    
    return best;
}

void MeshTimeSynchronizer::performPeriodicTasks() {
    performTimeSynchronization();
}

String MeshTimeSynchronizer::getSyncReport() const {
    String report = "Time Synchronization Report:\n";
    report += "Node ID: " + String(localNodeId_) + "\n";
    report += "Is Time Source: " + String(isTimeSource_ ? "Yes" : "No") + "\n";
    report += "Stratum: " + String(localStratum_) + "\n";
    report += "Synchronized: " + String(isTimeSynchronized() ? "Yes" : "No") + "\n";
    report += "Quality: " + String(getSynchronizationQuality() * 100.0, 1) + "%\n";
    report += "Clock Offset: " + String(getClockOffset()) + "ms\n";
    report += "Clock Drift: " + String(averageDrift_, 2) + "ppm\n";
    report += "Sync Count: " + String(syncCount_) + "\n";
    report += "Time References: " + String(timeReferences_.size()) + "\n";
    
    if (primaryReference_.nodeId != 0) {
        report += "Primary Source: " + String(primaryReference_.nodeId) + "\n";
        report += "Source Accuracy: " + String(primaryReference_.accuracy) + "%\n";
    }
    
    return report;
}

// Callback registration functions
void setTimeSyncEventCallback(TimeSyncEventCallback callback) {
    g_timeSyncEventCallback = callback;
}

void setTimeSourceCallback(TimeSourceCallback callback) {
    g_timeSourceCallback = callback;
}