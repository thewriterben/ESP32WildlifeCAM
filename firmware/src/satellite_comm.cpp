/**
 * @file satellite_comm.cpp
 * @brief Satellite communication manager for ESP32 Wildlife Camera
 * 
 * Handles LEO satellite communication (Iridium, Swarm) with store-and-forward
 * messaging and satellite pass prediction.
 */

#include "satellite_comm.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <time.h>

// Pin definitions for satellite modules
#define SAT_RX_PIN 25
#define SAT_TX_PIN 26
#define SAT_SLEEP_PIN 27
#define SAT_RING_PIN 33

SatelliteComm::SatelliteComm() :
    satSerial(SAT_RX_PIN, SAT_TX_PIN),
    isConnected(false),
    moduleType(MODULE_NONE),
    messagesSentToday(0),
    maxDailyMessages(50),
    lastPassTime(0) {
}

bool SatelliteComm::initialize(SatelliteModule module) {
    moduleType = module;
    
    // Initialize serial communication with configurable baud rate
    satSerial.begin(SATELLITE_BAUD_RATE);
    
    // Configure control pins
    pinMode(SAT_SLEEP_PIN, OUTPUT);
    pinMode(SAT_RING_PIN, INPUT);
    
    // Wake up the module
    wakeUpModule();
    
    // Test communication based on module type
    switch (moduleType) {
        case MODULE_IRIDIUM:
            return initializeIridium();
        case MODULE_SWARM:
            return initializeSwarm();
        case MODULE_ROCKBLOCK:
            return initializeRockBlock();
        default:
            Serial.println("Unknown satellite module type");
            return false;
    }
}

bool SatelliteComm::initializeIridium() {
    Serial.println("Initializing Iridium 9603N module");
    
    // Test AT communication with configurable retry count
    for (int i = 0; i < SATELLITE_RETRY_COUNT; i++) {
        satSerial.println("AT");
        delay(SERIAL_INIT_DELAY);
        if (satSerial.available()) {
            String response = satSerial.readString();
            if (response.indexOf("OK") != -1) {
                Serial.println("Iridium module responding");
                configureIridium();
                return true;
            }
        }
    }
    return false;
}

bool SatelliteComm::initializeSwarm() {
    Serial.println("Initializing Swarm M138 module");
    
    // Swarm uses different command structure
    satSerial.println("$CS*");
    delay(SATELLITE_RESPONSE_DELAY);
    
    if (satSerial.available()) {
        String response = satSerial.readString();
        if (response.indexOf("$CS") != -1) {
            Serial.println("Swarm module responding");
            configureSwarm();
            return true;
        }
    }
    return false;
}

bool SatelliteComm::initializeRockBlock() {
    Serial.println("Initializing RockBLOCK module");
    
    // RockBLOCK uses Iridium protocol
    return initializeIridium();
}

void SatelliteComm::configureIridium() {
    // Enable network registration notifications
    satSerial.println("AT+CIER=1,1,1,1");
    delay(500);
    
    // Check signal quality
    checkSignalQuality();
}

void SatelliteComm::configureSwarm() {
    // Configure Swarm-specific settings
    satSerial.println("$RT RSSI*");
    delay(500);
    
    checkSignalQuality();
}

void SatelliteComm::wakeUpModule() {
    digitalWrite(SAT_SLEEP_PIN, HIGH);
    delay(SATELLITE_WAKEUP_DELAY);
    digitalWrite(SAT_SLEEP_PIN, LOW);
    delay(SATELLITE_RESPONSE_DELAY); // Wait for module to wake up
}

bool SatelliteComm::checkSatelliteAvailability() {
    if (moduleType == MODULE_IRIDIUM || moduleType == MODULE_ROCKBLOCK) {
        satSerial.println("AT+CIER?");
        delay(2000);
        
        if (satSerial.available()) {
            String response = satSerial.readString();
            // Check for satellite network registration
            return response.indexOf("+CIEV:0,1") != -1;
        }
    } else if (moduleType == MODULE_SWARM) {
        // Swarm has different availability checking
        satSerial.println("$GS*");
        delay(2000);
        
        if (satSerial.available()) {
            String response = satSerial.readString();
            return response.indexOf("$GS 1") != -1;
        }
    }
    
    return false;
}

int SatelliteComm::checkSignalQuality() {
    if (moduleType == MODULE_IRIDIUM || moduleType == MODULE_ROCKBLOCK) {
        satSerial.println("AT+CSQ");
        delay(2000);
        
        if (satSerial.available()) {
            String response = satSerial.readString();
            int csqIndex = response.indexOf("+CSQ:");
            if (csqIndex != -1) {
                int signalStrength = response.substring(csqIndex + 5).toInt();
                Serial.printf("Satellite signal strength: %d\n", signalStrength);
                return signalStrength;
            }
        }
    } else if (moduleType == MODULE_SWARM) {
        satSerial.println("$RT RSSI*");
        delay(2000);
        
        if (satSerial.available()) {
            String response = satSerial.readString();
            // Parse Swarm RSSI response
            int rssiIndex = response.indexOf("RSSI");
            if (rssiIndex != -1) {
                // Extract RSSI value
                return response.substring(rssiIndex + 5).toInt();
            }
        }
    }
    
    return -1;
}

bool SatelliteComm::sendMessage(const String& message) {
    if (messagesSentToday >= maxDailyMessages) {
        Serial.println("Daily message limit exceeded");
        return false;
    }
    
    if (!checkSatelliteAvailability()) {
        // Store message for later transmission
        return storeMessage(message);
    }
    
    bool success = false;
    
    switch (moduleType) {
        case MODULE_IRIDIUM:
        case MODULE_ROCKBLOCK:
            success = sendIridiumMessage(message);
            break;
        case MODULE_SWARM:
            success = sendSwarmMessage(message);
            break;
        default:
            return false;
    }
    
    if (success) {
        messagesSentToday++;
        Serial.printf("Message sent successfully. Daily count: %d/%d\n", 
                     messagesSentToday, maxDailyMessages);
    }
    
    return success;
}

bool SatelliteComm::sendIridiumMessage(const String& message) {
    // Prepare message for SBD (Short Burst Data)
    String hexMessage = stringToHex(message);
    
    // Clear mobile originated buffer
    satSerial.println("AT+SBDD0");
    delay(1000);
    
    // Write message to buffer
    satSerial.println("AT+SBDWB=" + String(hexMessage.length() / 2));
    delay(1000);
    
    // Send hex data
    for (int i = 0; i < hexMessage.length(); i += 2) {
        String hexByte = hexMessage.substring(i, i + 2);
        satSerial.write(strtol(hexByte.c_str(), NULL, 16));
    }
    
    delay(1000);
    
    // Initiate SBD session
    satSerial.println("AT+SBDI");
    delay(30000); // SBD transmission can take up to 30 seconds
    
    if (satSerial.available()) {
        String response = satSerial.readString();
        Serial.println("SBD Response: " + response);
        
        // Check for successful transmission (SBDI response format: +SBDI:0,x,y,z)
        return response.indexOf("+SBDI:0") != -1;
    }
    
    return false;
}

bool SatelliteComm::sendSwarmMessage(const String& message) {
    // Swarm message format
    String swarmCommand = "$TD \"" + message + "\"*";
    
    satSerial.println(swarmCommand);
    delay(10000); // Wait for transmission
    
    if (satSerial.available()) {
        String response = satSerial.readString();
        Serial.println("Swarm Response: " + response);
        
        // Check for successful transmission
        return response.indexOf("$TD OK") != -1;
    }
    
    return false;
}

bool SatelliteComm::storeMessage(const String& message) {
    // Store message in EEPROM or flash for later transmission
    storedMessages.push_back(message);
    Serial.println("Message stored for later transmission");
    return true;
}

bool SatelliteComm::sendStoredMessages() {
    if (storedMessages.empty()) {
        return true;
    }
    
    if (!checkSatelliteAvailability()) {
        Serial.println("No satellite connectivity for stored messages");
        return false;
    }
    
    int sentCount = 0;
    auto it = storedMessages.begin();
    
    while (it != storedMessages.end() && 
           messagesSentToday < maxDailyMessages) {
        
        if (sendMessage(*it)) {
            it = storedMessages.erase(it);
            sentCount++;
        } else {
            ++it;
        }
        
        delay(5000); // Wait between transmissions
    }
    
    Serial.printf("Sent %d stored messages\n", sentCount);
    return sentCount > 0;
}

String SatelliteComm::stringToHex(const String& input) {
    String hex = "";
    for (int i = 0; i < input.length(); i++) {
        char hexByte[3];
        sprintf(hexByte, "%02X", input.charAt(i));
        hex += hexByte;
    }
    return hex;
}

bool SatelliteComm::predictNextPass() {
    // Simplified satellite pass prediction
    // In a real implementation, this would use orbital mechanics
    // and current satellite constellation data
    
    time_t now = time(nullptr);
    
    if (moduleType == MODULE_IRIDIUM || moduleType == MODULE_ROCKBLOCK) {
        // Iridium satellites pass every 100-110 minutes
        lastPassTime = now + (100 * 60); // Next pass in ~100 minutes
    } else if (moduleType == MODULE_SWARM) {
        // Swarm satellites have different orbital patterns
        lastPassTime = now + (90 * 60); // Next pass in ~90 minutes
    }
    
    Serial.printf("Next satellite pass predicted at: %ld\n", lastPassTime);
    return true;
}

time_t SatelliteComm::getNextPassTime() {
    return lastPassTime;
}

void SatelliteComm::enterSleepMode() {
    if (moduleType == MODULE_IRIDIUM || moduleType == MODULE_ROCKBLOCK) {
        satSerial.println("AT+CPWROFF");
        delay(1000);
    }
    
    digitalWrite(SAT_SLEEP_PIN, HIGH);
}

void SatelliteComm::exitSleepMode() {
    wakeUpModule();
    
    // Re-establish communication
    delay(5000);
    
    switch (moduleType) {
        case MODULE_IRIDIUM:
        case MODULE_ROCKBLOCK:
            initializeIridium();
            break;
        case MODULE_SWARM:
            initializeSwarm();
            break;
    }
}

size_t SatelliteComm::getMessageCount() {
    return messagesSentToday;
}

void SatelliteComm::resetMessageCount() {
    messagesSentToday = 0;
}

void SatelliteComm::setMessageLimit(size_t limit) {
    maxDailyMessages = limit;
}