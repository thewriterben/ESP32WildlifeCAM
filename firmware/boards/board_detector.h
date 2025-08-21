#ifndef BOARD_DETECTOR_H
#define BOARD_DETECTOR_H

#include "../src/board_abstraction.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Automatically detect the ESP32 camera board type
 * 
 * This function attempts to identify the specific ESP32 camera board
 * by analyzing chip features, GPIO states, and hardware signatures.
 * 
 * @return board_type_t The detected board type, or BOARD_UNKNOWN if detection fails
 */
board_type_t board_auto_detect(void);

/**
 * @brief Validate that the detected board type is correct
 * 
 * Performs additional checks to confirm the board detection is accurate.
 * 
 * @param detected_board The board type to validate
 * @return true if validation passes, false otherwise
 */
bool board_validate_detection(board_type_t detected_board);

/**
 * @brief Get information about how a board type was detected
 * 
 * @param board_type The board type to get detection info for
 * @return const char* Description of the detection method used
 */
const char* board_get_detection_info(board_type_t board_type);

#ifdef __cplusplus
}
#endif

#endif // BOARD_DETECTOR_H