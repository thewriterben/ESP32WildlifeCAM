#include "camera_universal.h"
#include "board_abstraction.h"
#include <string.h>
#include <esp_timer.h>

// Static variables for camera state
static camera_interface_t* current_camera_interface = NULL;
static camera_config_t current_camera_config;
static bool camera_initialized = false;

// Frame size mappings
typedef struct {
    framesize_t size;
    uint32_t width;
    uint32_t height;
    const char* name;
} framesize_info_t;

static const framesize_info_t framesize_table[] = {
    {FRAMESIZE_QVGA, 320, 240, "QVGA"},
    {FRAMESIZE_VGA, 640, 480, "VGA"},
    {FRAMESIZE_SVGA, 800, 600, "SVGA"},
    {FRAMESIZE_XGA, 1024, 768, "XGA"},
    {FRAMESIZE_SXGA, 1280, 1024, "SXGA"},
    {FRAMESIZE_UXGA, 1600, 1200, "UXGA"}
};

static const size_t framesize_table_size = sizeof(framesize_table) / sizeof(framesize_table[0]);

// Pixel format names
static const char* pixformat_names[] = {
    "JPEG",
    "RGB565", 
    "YUV422",
    "GRAYSCALE"
};

// Forward declarations for sensor-specific implementations
extern camera_interface_t ov2640_interface;
extern camera_interface_t ov3660_interface;
extern camera_interface_t ov5640_interface;
extern camera_interface_t gc032a_interface;
extern camera_interface_t ov7670_interface;
extern camera_interface_t hm01b0_interface;

// Get camera interface based on sensor type
static camera_interface_t* get_camera_interface(camera_sensor_t sensor_type) {
    switch (sensor_type) {
        case SENSOR_OV2640:
            return &ov2640_interface;
        case SENSOR_OV3660:
            return &ov3660_interface;
        case SENSOR_OV5640:
            return &ov5640_interface;
        case SENSOR_GC032A:
            return &gc032a_interface;
        case SENSOR_OV7670:
            return &ov7670_interface;
        case SENSOR_HM01B0:
            return &hm01b0_interface;
        default:
            return NULL;
    }
}

bool camera_universal_init(const camera_config_t* config) {
    if (camera_initialized) {
        camera_universal_deinit();
    }
    
    // Get board configuration to determine sensor type
    const board_config_t* board_config = board_get_config();
    if (!board_config) {
        return false;
    }
    
    // Get the appropriate camera interface for this sensor
    current_camera_interface = get_camera_interface(board_config->sensor_type);
    if (!current_camera_interface || !current_camera_interface->init) {
        return false;
    }
    
    // Store configuration
    if (config) {
        memcpy(&current_camera_config, config, sizeof(camera_config_t));
    } else {
        // Use default configuration
        current_camera_config.framesize = FRAMESIZE_VGA;
        current_camera_config.pixel_format = PIXFORMAT_JPEG;
        current_camera_config.jpeg_quality = 12;
        current_camera_config.flip_horizontally = false;
        current_camera_config.flip_vertically = false;
        current_camera_config.brightness = 0;
        current_camera_config.contrast = 0;
        current_camera_config.saturation = 0;
        current_camera_config.auto_exposure = true;
        current_camera_config.exposure_value = 300;
        current_camera_config.auto_white_balance = true;
        current_camera_config.white_balance_mode = 0;
    }
    
    // Initialize the camera with the sensor-specific interface
    if (current_camera_interface->init(&current_camera_config)) {
        camera_initialized = true;
        return true;
    }
    
    return false;
}

camera_fb_t* camera_universal_capture(void) {
    if (!camera_initialized || !current_camera_interface || !current_camera_interface->capture) {
        return NULL;
    }
    
    return current_camera_interface->capture();
}

void camera_universal_return_fb(camera_fb_t* fb) {
    if (!camera_initialized || !current_camera_interface || !current_camera_interface->return_fb || !fb) {
        return;
    }
    
    current_camera_interface->return_fb(fb);
}

bool camera_universal_set_framesize(framesize_t size) {
    if (!camera_initialized || !current_camera_interface || !current_camera_interface->set_framesize) {
        return false;
    }
    
    // Check if the board supports this resolution
    const board_config_t* board_config = board_get_config();
    uint32_t width = framesize_to_width(size);
    uint32_t height = framesize_to_height(size);
    
    if (width > board_config->max_resolution_width || height > board_config->max_resolution_height) {
        return false; // Resolution not supported by this board
    }
    
    if (current_camera_interface->set_framesize(size)) {
        current_camera_config.framesize = size;
        return true;
    }
    
    return false;
}

bool camera_universal_set_quality(uint8_t quality) {
    if (!camera_initialized || !current_camera_interface || !current_camera_interface->set_quality) {
        return false;
    }
    
    // Quality should be 0-63 for JPEG
    if (quality > 63) {
        quality = 63;
    }
    
    if (current_camera_interface->set_quality(quality)) {
        current_camera_config.jpeg_quality = quality;
        return true;
    }
    
    return false;
}

bool camera_universal_set_brightness(int8_t brightness) {
    if (!camera_initialized || !current_camera_interface || !current_camera_interface->set_brightness) {
        return false;
    }
    
    // Clamp brightness to valid range
    if (brightness < -2) brightness = -2;
    if (brightness > 2) brightness = 2;
    
    if (current_camera_interface->set_brightness(brightness)) {
        current_camera_config.brightness = brightness;
        return true;
    }
    
    return false;
}

bool camera_universal_set_contrast(int8_t contrast) {
    if (!camera_initialized || !current_camera_interface || !current_camera_interface->set_contrast) {
        return false;
    }
    
    // Clamp contrast to valid range
    if (contrast < -2) contrast = -2;
    if (contrast > 2) contrast = 2;
    
    if (current_camera_interface->set_contrast(contrast)) {
        current_camera_config.contrast = contrast;
        return true;
    }
    
    return false;
}

bool camera_universal_set_saturation(int8_t saturation) {
    if (!camera_initialized || !current_camera_interface || !current_camera_interface->set_saturation) {
        return false;
    }
    
    // Clamp saturation to valid range
    if (saturation < -2) saturation = -2;
    if (saturation > 2) saturation = 2;
    
    if (current_camera_interface->set_saturation(saturation)) {
        current_camera_config.saturation = saturation;
        return true;
    }
    
    return false;
}

bool camera_universal_supports_feature(camera_feature_t feature) {
    if (!camera_initialized || !current_camera_interface || !current_camera_interface->supports_feature) {
        return false;
    }
    
    return current_camera_interface->supports_feature(feature);
}

void camera_universal_deinit(void) {
    if (camera_initialized && current_camera_interface && current_camera_interface->deinit) {
        current_camera_interface->deinit();
    }
    
    current_camera_interface = NULL;
    camera_initialized = false;
}

// Utility functions
const char* framesize_to_string(framesize_t size) {
    for (size_t i = 0; i < framesize_table_size; i++) {
        if (framesize_table[i].size == size) {
            return framesize_table[i].name;
        }
    }
    return "UNKNOWN";
}

const char* pixformat_to_string(pixformat_t format) {
    if (format < sizeof(pixformat_names) / sizeof(pixformat_names[0])) {
        return pixformat_names[format];
    }
    return "UNKNOWN";
}

uint32_t framesize_to_width(framesize_t size) {
    for (size_t i = 0; i < framesize_table_size; i++) {
        if (framesize_table[i].size == size) {
            return framesize_table[i].width;
        }
    }
    return 0;
}

uint32_t framesize_to_height(framesize_t size) {
    for (size_t i = 0; i < framesize_table_size; i++) {
        if (framesize_table[i].size == size) {
            return framesize_table[i].height;
        }
    }
    return 0;
}