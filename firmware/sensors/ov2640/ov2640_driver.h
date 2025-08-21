#ifndef OV2640_DRIVER_H
#define OV2640_DRIVER_H

#include "../../src/camera_universal.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// OV2640 specific definitions
#define OV2640_CHIP_ID          0x2642
#define OV2640_I2C_ADDR         0x30

// OV2640 register definitions
#define OV2640_REG_CHIP_ID_H    0x0A
#define OV2640_REG_CHIP_ID_L    0x0B
#define OV2640_REG_COM1         0x04
#define OV2640_REG_COM7         0x12
#define OV2640_REG_COM10        0x15

// OV2640 resolution capabilities
#define OV2640_MAX_WIDTH        1600
#define OV2640_MAX_HEIGHT       1200
#define OV2640_MIN_WIDTH        160
#define OV2640_MIN_HEIGHT       120

// OV2640 camera interface implementation
extern camera_interface_t ov2640_interface;

// OV2640 specific functions
bool ov2640_init(const camera_config_t* config);
camera_fb_t* ov2640_capture(void);
void ov2640_return_fb(camera_fb_t* fb);
bool ov2640_set_framesize(framesize_t size);
bool ov2640_set_quality(uint8_t quality);
bool ov2640_set_brightness(int8_t brightness);
bool ov2640_set_contrast(int8_t contrast);
bool ov2640_set_saturation(int8_t saturation);
bool ov2640_supports_feature(camera_feature_t feature);
void ov2640_deinit(void);

// OV2640 register access
bool ov2640_write_reg(uint8_t reg, uint8_t value);
bool ov2640_read_reg(uint8_t reg, uint8_t* value);
bool ov2640_check_chip_id(void);

// OV2640 configuration helpers
bool ov2640_set_resolution(uint32_t width, uint32_t height);
bool ov2640_set_pixel_format(pixformat_t format);
bool ov2640_enable_test_pattern(bool enable);

#ifdef __cplusplus
}
#endif

#endif // OV2640_DRIVER_H