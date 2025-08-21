#include "../../src/camera_universal.h"

// Stub implementations for camera sensors
// These would be replaced with actual sensor drivers in production

// Forward declarations for all sensor interfaces
camera_interface_t ov3660_interface;
camera_interface_t ov5640_interface;
camera_interface_t gc032a_interface;
camera_interface_t ov7670_interface;
camera_interface_t hm01b0_interface;

// Stub implementation for OV3660
bool ov3660_init_stub(const camera_config_t* config) { return false; }
camera_fb_t* ov3660_capture_stub(void) { return NULL; }
void ov3660_return_fb_stub(camera_fb_t* fb) { }
bool ov3660_set_framesize_stub(framesize_t size) { return false; }
bool ov3660_set_quality_stub(uint8_t quality) { return false; }
bool ov3660_set_brightness_stub(int8_t brightness) { return false; }
bool ov3660_set_contrast_stub(int8_t contrast) { return false; }
bool ov3660_set_saturation_stub(int8_t saturation) { return false; }
bool ov3660_supports_feature_stub(camera_feature_t feature) { return false; }
void ov3660_deinit_stub(void) { }

camera_interface_t ov3660_interface = {
    .init = ov3660_init_stub,
    .capture = ov3660_capture_stub,
    .return_fb = ov3660_return_fb_stub,
    .set_framesize = ov3660_set_framesize_stub,
    .set_quality = ov3660_set_quality_stub,
    .set_brightness = ov3660_set_brightness_stub,
    .set_contrast = ov3660_set_contrast_stub,
    .set_saturation = ov3660_set_saturation_stub,
    .supports_feature = ov3660_supports_feature_stub,
    .deinit = ov3660_deinit_stub
};

// Stub implementation for OV5640
bool ov5640_init_stub(const camera_config_t* config) { return false; }
camera_fb_t* ov5640_capture_stub(void) { return NULL; }
void ov5640_return_fb_stub(camera_fb_t* fb) { }
bool ov5640_set_framesize_stub(framesize_t size) { return false; }
bool ov5640_set_quality_stub(uint8_t quality) { return false; }
bool ov5640_set_brightness_stub(int8_t brightness) { return false; }
bool ov5640_set_contrast_stub(int8_t contrast) { return false; }
bool ov5640_set_saturation_stub(int8_t saturation) { return false; }
bool ov5640_supports_feature_stub(camera_feature_t feature) { return false; }
void ov5640_deinit_stub(void) { }

camera_interface_t ov5640_interface = {
    .init = ov5640_init_stub,
    .capture = ov5640_capture_stub,
    .return_fb = ov5640_return_fb_stub,
    .set_framesize = ov5640_set_framesize_stub,
    .set_quality = ov5640_set_quality_stub,
    .set_brightness = ov5640_set_brightness_stub,
    .set_contrast = ov5640_set_contrast_stub,
    .set_saturation = ov5640_set_saturation_stub,
    .supports_feature = ov5640_supports_feature_stub,
    .deinit = ov5640_deinit_stub
};

// Stub implementation for GC032A
bool gc032a_init_stub(const camera_config_t* config) { return false; }
camera_fb_t* gc032a_capture_stub(void) { return NULL; }
void gc032a_return_fb_stub(camera_fb_t* fb) { }
bool gc032a_set_framesize_stub(framesize_t size) { return false; }
bool gc032a_set_quality_stub(uint8_t quality) { return false; }
bool gc032a_set_brightness_stub(int8_t brightness) { return false; }
bool gc032a_set_contrast_stub(int8_t contrast) { return false; }
bool gc032a_set_saturation_stub(int8_t saturation) { return false; }
bool gc032a_supports_feature_stub(camera_feature_t feature) { return false; }
void gc032a_deinit_stub(void) { }

camera_interface_t gc032a_interface = {
    .init = gc032a_init_stub,
    .capture = gc032a_capture_stub,
    .return_fb = gc032a_return_fb_stub,
    .set_framesize = gc032a_set_framesize_stub,
    .set_quality = gc032a_set_quality_stub,
    .set_brightness = gc032a_set_brightness_stub,
    .set_contrast = gc032a_set_contrast_stub,
    .set_saturation = gc032a_set_saturation_stub,
    .supports_feature = gc032a_supports_feature_stub,
    .deinit = gc032a_deinit_stub
};

// Stub implementation for OV7670
bool ov7670_init_stub(const camera_config_t* config) { return false; }
camera_fb_t* ov7670_capture_stub(void) { return NULL; }
void ov7670_return_fb_stub(camera_fb_t* fb) { }
bool ov7670_set_framesize_stub(framesize_t size) { return false; }
bool ov7670_set_quality_stub(uint8_t quality) { return false; }
bool ov7670_set_brightness_stub(int8_t brightness) { return false; }
bool ov7670_set_contrast_stub(int8_t contrast) { return false; }
bool ov7670_set_saturation_stub(int8_t saturation) { return false; }
bool ov7670_supports_feature_stub(camera_feature_t feature) { return false; }
void ov7670_deinit_stub(void) { }

camera_interface_t ov7670_interface = {
    .init = ov7670_init_stub,
    .capture = ov7670_capture_stub,
    .return_fb = ov7670_return_fb_stub,
    .set_framesize = ov7670_set_framesize_stub,
    .set_quality = ov7670_set_quality_stub,
    .set_brightness = ov7670_set_brightness_stub,
    .set_contrast = ov7670_set_contrast_stub,
    .set_saturation = ov7670_set_saturation_stub,
    .supports_feature = ov7670_supports_feature_stub,
    .deinit = ov7670_deinit_stub
};

// Stub implementation for HM01B0
bool hm01b0_init_stub(const camera_config_t* config) { return false; }
camera_fb_t* hm01b0_capture_stub(void) { return NULL; }
void hm01b0_return_fb_stub(camera_fb_t* fb) { }
bool hm01b0_set_framesize_stub(framesize_t size) { return false; }
bool hm01b0_set_quality_stub(uint8_t quality) { return false; }
bool hm01b0_set_brightness_stub(int8_t brightness) { return false; }
bool hm01b0_set_contrast_stub(int8_t contrast) { return false; }
bool hm01b0_set_saturation_stub(int8_t saturation) { return false; }
bool hm01b0_supports_feature_stub(camera_feature_t feature) { return false; }
void hm01b0_deinit_stub(void) { }

camera_interface_t hm01b0_interface = {
    .init = hm01b0_init_stub,
    .capture = hm01b0_capture_stub,
    .return_fb = hm01b0_return_fb_stub,
    .set_framesize = hm01b0_set_framesize_stub,
    .set_quality = hm01b0_set_quality_stub,
    .set_brightness = hm01b0_set_brightness_stub,
    .set_contrast = hm01b0_set_contrast_stub,
    .set_saturation = hm01b0_set_saturation_stub,
    .supports_feature = hm01b0_supports_feature_stub,
    .deinit = hm01b0_deinit_stub
};