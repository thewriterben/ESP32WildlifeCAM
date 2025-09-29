/**
 * @file power_manager.h
 * @brief Power management system header (firmware include)
 * @author thewriterben
 * @date 2025-08-31
 */

#ifndef FIRMWARE_POWER_MANAGER_H
#define FIRMWARE_POWER_MANAGER_H

// Forward declaration for firmware compatibility
class PowerManager;

// Include the main power manager implementation
#include "../../../src/power/power_manager.h"

// External instance declaration
extern PowerManager* g_powerManager;

#endif // FIRMWARE_POWER_MANAGER_H