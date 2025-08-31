// Test compilation of core files
#include "config.h"
#include "hardware/board_detector.h"
#include "core/system_manager.h" 
#include "utils/logger.h"

int main() {
    // Test basic instantiation
    BoardDetector::BoardType board = BoardDetector::detectBoard();
    SystemManager* system = new SystemManager(board);
    
    Logger::info("Test compilation successful");
    
    delete system;
    return 0;
}