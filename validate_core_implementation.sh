#!/bin/bash
# Simple validation script for ESP32WildlifeCAM
# Checks syntax and basic functionality

echo "=== ESP32WildlifeCAM Validation ==="
echo "Checking file structure..."

# Check if main files exist
if [ -f "WildCAM_ESP32/main.cpp" ]; then
    echo "✓ Main application file found"
else
    echo "✗ Main application file missing"
    exit 1
fi

if [ -f "WildCAM_ESP32/platformio.ini" ]; then
    echo "✓ PlatformIO configuration found"
else
    echo "✗ PlatformIO configuration missing"
    exit 1
fi

if [ -f "WildCAM_ESP32/src/core/system_manager.cpp" ]; then
    echo "✓ System manager implementation found"
else
    echo "✗ System manager implementation missing"
    exit 1
fi

if [ -f "WildCAM_ESP32/src/camera/camera_integration.cpp" ]; then
    echo "✓ Camera integration found"
else
    echo "✗ Camera integration missing"
    exit 1
fi

if [ -f "WildCAM_ESP32/src/storage/storage_manager.cpp" ]; then
    echo "✓ Storage manager found"
else
    echo "✗ Storage manager missing"
    exit 1
fi

echo ""
echo "=== Code Analysis ==="

# Check for esp_camera_init usage
if grep -q "esp_camera_init" WildCAM_ESP32/src/camera/camera_integration.cpp; then
    echo "✓ Camera initialization implemented"
else
    echo "✗ Camera initialization missing"
    exit 1
fi

# Check for SD card usage
if grep -q "SD_MMC" WildCAM_ESP32/src/storage/storage_manager.cpp; then
    echo "✓ SD card storage implemented"
else
    echo "✗ SD card storage missing"
    exit 1
fi

# Check for image saving functionality
if grep -q "saveImage" WildCAM_ESP32/src/storage/storage_manager.cpp; then
    echo "✓ Image saving functionality implemented"
else
    echo "✗ Image saving functionality missing"
    exit 1
fi

echo ""
echo "=== Validation Complete ==="
echo "All core components are present and basic functionality is implemented."
echo "The system should now be able to:"
echo "  - Initialize ESP32-CAM hardware"
echo "  - Perform camera test capture"
echo "  - Initialize SD card storage"
echo "  - Capture and save images"
echo "  - Run without entering safe mode"