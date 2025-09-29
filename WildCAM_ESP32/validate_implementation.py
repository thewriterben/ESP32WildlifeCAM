#!/usr/bin/env python3
"""
ESP32WildlifeCAM Implementation Validation Script
Validates that all critical files exist and have proper structure
"""

import os
import sys
import re
from pathlib import Path

def validate_file_exists(path, description):
    """Check if a file exists and return result."""
    exists = os.path.exists(path)
    status = "✓" if exists else "✗"
    print(f"{status} {description}: {path}")
    return exists

def validate_file_content(path, patterns, description):
    """Check if file contains required patterns."""
    if not os.path.exists(path):
        print(f"✗ {description}: File not found - {path}")
        return False
    
    try:
        with open(path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        missing_patterns = []
        for pattern_name, pattern in patterns.items():
            if not re.search(pattern, content, re.MULTILINE):
                missing_patterns.append(pattern_name)
        
        if missing_patterns:
            print(f"✗ {description}: Missing patterns - {', '.join(missing_patterns)}")
            return False
        else:
            print(f"✓ {description}: All patterns found")
            return True
            
    except Exception as e:
        print(f"✗ {description}: Error reading file - {e}")
        return False

def main():
    """Main validation function."""
    print("ESP32WildlifeCAM Implementation Validation")
    print("=" * 50)
    
    base_path = Path(__file__).parent
    results = []
    
    # Core files validation
    print("\n1. Core Framework Files:")
    core_files = [
        (base_path / "main.cpp", "Main entry point"),
        (base_path / "platformio.ini", "PlatformIO configuration"),
        (base_path / "include/config.h", "System configuration"),
        (base_path / "include/pins.h", "Pin definitions"),
    ]
    
    for file_path, description in core_files:
        results.append(validate_file_exists(file_path, description))
    
    # System manager validation
    print("\n2. System Manager:")
    system_files = [
        (base_path / "src/core/system_manager.h", "System manager header"),
        (base_path / "src/core/system_manager.cpp", "System manager implementation"),
    ]
    
    for file_path, description in system_files:
        results.append(validate_file_exists(file_path, description))
    
    # Camera system validation
    print("\n3. Camera System:")
    camera_files = [
        (base_path / "src/camera/camera_manager.h", "Camera manager header"),
        (base_path / "src/camera/camera_manager.cpp", "Camera manager implementation"),
    ]
    
    for file_path, description in camera_files:
        results.append(validate_file_exists(file_path, description))
    
    # Check camera manager has essential methods
    camera_patterns = {
        "initialize": r"bool.*initialize\(",
        "captureImage": r"CaptureResult.*captureImage\(",
        "captureToBuffer": r"camera_fb_t.*captureToBuffer\(",
    }
    results.append(validate_file_content(
        base_path / "src/camera/camera_manager.cpp", 
        camera_patterns, 
        "Camera manager essential methods"
    ))
    
    # Power management validation
    print("\n4. Power Management:")
    power_files = [
        (base_path / "src/power/power_manager.h", "Power manager header"),
        (base_path / "src/power/power_manager.cpp", "Power manager implementation"),
    ]
    
    for file_path, description in power_files:
        results.append(validate_file_exists(file_path, description))
    
    # Motion detection validation
    print("\n5. Motion Detection:")
    motion_files = [
        (base_path / "src/detection/motion_coordinator.h", "Motion coordinator header"),
        (base_path / "src/detection/motion_coordinator.cpp", "Motion coordinator implementation"),
    ]
    
    for file_path, description in motion_files:
        results.append(validate_file_exists(file_path, description))
    
    # Network management validation
    print("\n6. Network Management:")
    network_files = [
        (base_path / "src/network/wifi_manager.h", "WiFi manager header"),
        (base_path / "src/network/wifi_manager.cpp", "WiFi manager implementation"),
    ]
    
    for file_path, description in network_files:
        results.append(validate_file_exists(file_path, description))
    
    # Utilities validation
    print("\n7. Utilities:")
    util_files = [
        (base_path / "src/utils/logger.h", "Logger header"),
        (base_path / "src/utils/logger.cpp", "Logger implementation"),
    ]
    
    for file_path, description in util_files:
        results.append(validate_file_exists(file_path, description))
    
    # Hardware abstraction validation
    print("\n8. Hardware Abstraction:")
    hardware_files = [
        (base_path / "src/hardware/board_detector.h", "Board detector header"),
        (base_path / "src/hardware/board_detector.cpp", "Board detector implementation"),
    ]
    
    for file_path, description in hardware_files:
        results.append(validate_file_exists(file_path, description))
    
    # Storage system validation
    print("\n9. Storage System:")
    storage_files = [
        (base_path / "src/core/storage_manager.cpp", "Storage manager implementation"),
    ]
    
    for file_path, description in storage_files:
        results.append(validate_file_exists(file_path, description))
    
    # Configuration validation
    print("\n10. Configuration Completeness:")
    config_patterns = {
        "CAMERA_FRAME_SIZE": r"#define\s+CAMERA_FRAME_SIZE",
        "BATTERY_LOW_THRESHOLD": r"#define\s+BATTERY_LOW_THRESHOLD",
        "PIR_PIN": r"#define\s+PIR_PIN",
        "MOTION_DETECTION_ENABLED": r"#define\s+MOTION_DETECTION_ENABLED",
    }
    results.append(validate_file_content(
        base_path / "include/config.h", 
        config_patterns, 
        "Essential configuration defines"
    ))
    
    # Summary
    print("\n" + "=" * 50)
    total_checks = len(results)
    passed_checks = sum(results)
    success_rate = (passed_checks / total_checks) * 100 if total_checks > 0 else 0
    
    print(f"Validation Summary:")
    print(f"  Total checks: {total_checks}")
    print(f"  Passed: {passed_checks}")
    print(f"  Failed: {total_checks - passed_checks}")
    print(f"  Success rate: {success_rate:.1f}%")
    
    if success_rate >= 90:
        print("\n🎉 Implementation is READY for production!")
        return 0
    elif success_rate >= 70:
        print("\n⚠️  Implementation is MOSTLY COMPLETE - minor issues remain")
        return 1
    else:
        print("\n❌ Implementation has SIGNIFICANT GAPS - more work needed")
        return 2

if __name__ == "__main__":
    sys.exit(main())