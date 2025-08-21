/**
 * @file satellite_comm.h
 * @brief Header file for satellite communication manager
 */

#ifndef SATELLITE_COMM_H
#define SATELLITE_COMM_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <vector>
#include <time.h>

enum SatelliteModule {
    MODULE_NONE = 0,
    MODULE_IRIDIUM,
    MODULE_SWARM,
    MODULE_ROCKBLOCK
};

class SatelliteComm {
public:
    SatelliteComm();
    
    // Initialization
    bool initialize(SatelliteModule module);
    
    // Communication
    bool sendMessage(const String& message);
    bool sendStoredMessages();
    
    // Satellite status
    bool checkSatelliteAvailability();
    int checkSignalQuality();
    bool predictNextPass();
    time_t getNextPassTime();
    
    // Power management
    void enterSleepMode();
    void exitSleepMode();
    
    // Message management
    size_t getMessageCount();
    void resetMessageCount();
    void setMessageLimit(size_t limit);

private:
    SoftwareSerial satSerial;
    bool isConnected;
    SatelliteModule moduleType;
    size_t messagesSentToday;
    size_t maxDailyMessages;
    time_t lastPassTime;
    std::vector<String> storedMessages;
    
    // Module-specific initialization
    bool initializeIridium();
    bool initializeSwarm();
    bool initializeRockBlock();
    
    // Configuration
    void configureIridium();
    void configureSwarm();
    void wakeUpModule();
    
    // Message transmission
    bool sendIridiumMessage(const String& message);
    bool sendSwarmMessage(const String& message);
    bool storeMessage(const String& message);
    
    // Utility functions
    String stringToHex(const String& input);
};

#endif // SATELLITE_COMM_H