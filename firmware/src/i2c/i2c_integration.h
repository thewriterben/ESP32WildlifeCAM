/**
 * @file i2c_integration.h
 * @brief Integration header for ESP-IDF I2C system with existing codebase
 */

#ifndef I2C_INTEGRATION_H
#define I2C_INTEGRATION_H

#include "i2c_manager.h"
#include "devices/bme280_i2c.h"
#include "../hal/camera_board.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the complete I2C system for the detected board
 * @param board_type Detected board type
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_system_init(BoardType board_type);

/**
 * @brief Deinitialize the I2C system
 * @return ESP_OK on success
 */
esp_err_t i2c_system_deinit(void);

/**
 * @brief Initialize BME280 environmental sensor if available
 * @param bme280_handle Output handle for BME280 device
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if not available
 */
esp_err_t i2c_init_bme280(uint32_t* bme280_handle);

/**
 * @brief Check if I2C system is available and ready
 * @return true if ready, false otherwise
 */
bool i2c_system_is_available(void);

/**
 * @brief Get environmental sensor reading (BME280)
 * @param reading Pointer to store sensor reading
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_get_environmental_reading(bme280_reading_t* reading);

/**
 * @brief Scan for available I2C devices and print results
 * @return Number of devices found
 */
int i2c_scan_and_report_devices(void);

#ifdef __cplusplus
}
#endif

#endif // I2C_INTEGRATION_H