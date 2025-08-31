/**
 * @file system_manager.h
 * @brief Main system coordination and management
 * @author thewriterben
 * @date 2025-08-31
 */

#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>
#include "../hardware/board_detector.h"

class SystemManager {
public:
    SystemManager(BoardDetector::BoardType board);
    ~SystemManager();
    
    bool initialize();
    void update();
    
private:
    BoardDetector::BoardType m_boardType;
    BoardDetector::PinConfig m_pinConfig;
    
    bool m_initialized;
    unsigned long m_lastUpdate;
    
    bool initializeHardware();
    bool initializePeripherals();
    bool initializeNetwork();
    bool initializeTasks();
};

#endif // SYSTEM_MANAGER_H