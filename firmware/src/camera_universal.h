#ifndef CAMERA_UNIVERSAL_H
#define CAMERA_UNIVERSAL_H

#include "board_abstraction.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Frame size enumeration (common ESP32 camera sizes)
typedef enum {
    FRAMESIZE_QVGA,    // 320x240
    FRAMESIZE_VGA,     // 640x480
    FRAMESIZE_SVGA,    // 800x600
    FRAMESIZE_XGA,     // 1024x768
    FRAMESIZE_SXGA,    // 1280x1024
    FRAMESIZE_UXGA     // 1600x1200
} framesize_t;

// Image format enumeration
typedef enum {
    PIXFORMAT_JPEG,
    PIXFORMAT_RGB565,
    PIXFORMAT_YUV422,
    PIXFORMAT_GRAYSCALE
} pixformat_t;

// Camera features enumeration
typedef enum {
    CAMERA_FEATURE_FLASH,
    CAMERA_FEATURE_AUTOFOCUS,
    CAMERA_FEATURE_NIGHT_MODE,
    CAMERA_FEATURE_MOTION_DETECTION,
    CAMERA_FEATURE_FACE_DETECTION,
    CAMERA_FEATURE_COLOR_EFFECTS,
    CAMERA_FEATURE_WHITE_BALANCE,
    CAMERA_FEATURE_EXPOSURE_CTRL
} camera_feature_t;

// Camera frame buffer structure
typedef struct {
    uint8_t* buf;       // Buffer containing the image data
    size_t len;         // Length of the buffer
    uint32_t width;     // Image width
    uint32_t height;    // Image height
    pixformat_t format; // Image format
    uint64_t timestamp; // Capture timestamp
} camera_fb_t;

// Camera configuration structure
typedef struct {
    framesize_t framesize;
    pixformat_t pixel_format;
    uint8_t jpeg_quality;    // 0-63, lower means higher quality
    bool flip_horizontally;
    bool flip_vertically;
    int8_t brightness;       // -2 to 2
    int8_t contrast;         // -2 to 2
    int8_t saturation;       // -2 to 2
    bool auto_exposure;
    uint16_t exposure_value; // When auto_exposure is false
    bool auto_white_balance;
    uint8_t white_balance_mode;
} camera_config_t;

// Camera interface structure (function pointers for polymorphism)
typedef struct {
    bool (*init)(const camera_config_t* config);
    camera_fb_t* (*capture)(void);
    void (*return_fb)(camera_fb_t* fb);
    bool (*set_framesize)(framesize_t size);
    bool (*set_quality)(uint8_t quality);
    bool (*set_brightness)(int8_t brightness);
    bool (*set_contrast)(int8_t contrast);
    bool (*set_saturation)(int8_t saturation);
    bool (*supports_feature)(camera_feature_t feature);
    void (*deinit)(void);
} camera_interface_t;

// Universal camera functions
bool camera_universal_init(const camera_config_t* config);
camera_fb_t* camera_universal_capture(void);
void camera_universal_return_fb(camera_fb_t* fb);
bool camera_universal_set_framesize(framesize_t size);
bool camera_universal_set_quality(uint8_t quality);
bool camera_universal_set_brightness(int8_t brightness);
bool camera_universal_set_contrast(int8_t contrast);
bool camera_universal_set_saturation(int8_t saturation);
bool camera_universal_supports_feature(camera_feature_t feature);
void camera_universal_deinit(void);

// Utility functions
const char* framesize_to_string(framesize_t size);
const char* pixformat_to_string(pixformat_t format);
uint32_t framesize_to_width(framesize_t size);
uint32_t framesize_to_height(framesize_t size);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_UNIVERSAL_H