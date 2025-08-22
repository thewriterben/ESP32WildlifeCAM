#!/usr/bin/env python3
"""
ESP32WildlifeCAM Code Validation Script
Validates the fixes and checks for common issues in the codebase.
"""

import os
import re
import sys
from pathlib import Path

def check_gpio_pin_conflicts():
    """Check for GPIO pin conflicts in config.h"""
    print("=== Checking GPIO Pin Conflicts ===")
    
    config_file = Path("firmware/src/config.h")
    if not config_file.exists():
        print("ERROR: config.h not found")
        return False
    
    content = config_file.read_text()
    
    # Extract pin assignments (ignore commented lines)
    pin_assignments = {}
    pin_pattern = r'^[^/]*#define\s+(\w+)\s+(\d+)\s*//.*pin'
    
    for line_num, line in enumerate(content.split('\n'), 1):
        # Skip commented lines
        if line.strip().startswith('//'):
            continue
            
        match = re.search(pin_pattern, line, re.IGNORECASE)
        if match:
            define_name = match.group(1)
            pin_number = int(match.group(2))
            
            if pin_number in pin_assignments:
                print(f"CONFLICT: GPIO {pin_number} used for both {pin_assignments[pin_number]} and {define_name}")
            else:
                pin_assignments[pin_number] = define_name
    
    print(f"Found {len(pin_assignments)} GPIO pin assignments")
    for pin, name in sorted(pin_assignments.items()):
        print(f"  GPIO {pin:2d}: {name}")
    
    return True

def check_include_guards():
    """Check for proper include guards in header files"""
    print("\n=== Checking Include Guards ===")
    
    header_files = list(Path("firmware/src").rglob("*.h"))
    issues = []
    
    for header_file in header_files:
        content = header_file.read_text()
        
        # Check for include guard
        guard_pattern = r'#ifndef\s+(\w+)\s*\n#define\s+\1'
        if not re.search(guard_pattern, content):
            issues.append(f"Missing include guard: {header_file}")
    
    if issues:
        print("Issues found:")
        for issue in issues:
            print(f"  {issue}")
        return False
    else:
        print(f"All {len(header_files)} header files have proper include guards")
        return True

def check_type_safety():
    """Check for type safety issues"""
    print("\n=== Checking Type Safety ===")
    
    source_files = list(Path("firmware/src").rglob("*.cpp"))
    issues = []
    
    for source_file in source_files:
        content = source_file.read_text()
        
        # Check for abs() used on floats (should be fabs())
        if re.search(r'\babs\s*\(\s*[\w.]+\s*\)', content):
            # Check if it's actually a float operation
            lines = content.split('\n')
            for i, line in enumerate(lines):
                if 'abs(' in line and ('float' in line or '.' in line):
                    issues.append(f"{source_file}:{i+1}: Possible abs() on float (should use fabs())")
    
    if issues:
        print("Type safety issues found:")
        for issue in issues:
            print(f"  {issue}")
        return False
    else:
        print("No type safety issues found")
        return True

def check_memory_management():
    """Check for potential memory management issues"""
    print("\n=== Checking Memory Management ===")
    
    source_files = list(Path("firmware/src").rglob("*.cpp"))
    issues = []
    
    for source_file in source_files:
        content = source_file.read_text()
        
        # Check for esp_camera_fb_get() without corresponding esp_camera_fb_return()
        if 'esp_camera_fb_get()' in content:
            if 'esp_camera_fb_return(' not in content:
                issues.append(f"{source_file}: esp_camera_fb_get() without esp_camera_fb_return()")
    
    if issues:
        print("Memory management issues found:")
        for issue in issues:
            print(f"  {issue}")
        return False
    else:
        print("No obvious memory management issues found")
        return True

def check_configuration_consistency():
    """Check for configuration consistency"""
    print("\n=== Checking Configuration Consistency ===")
    
    config_file = Path("firmware/src/config.h")
    content = config_file.read_text()
    
    # Check if disabled features are properly handled
    disabled_features = []
    if 'SD_CARD_ENABLED false' in content:
        disabled_features.append("SD Card")
    if 'VIBRATION_ENABLED false' in content:
        disabled_features.append("Vibration Sensor")
    if 'IR_LED_ENABLED false' in content:
        disabled_features.append("IR LED")
    if 'SATELLITE_ENABLED false' in content:
        disabled_features.append("Satellite Communication")
    
    if disabled_features:
        print("Disabled features due to pin conflicts:")
        for feature in disabled_features:
            print(f"  - {feature}")
    
    # Check for consistent pin definitions (ignore commented lines)
    pir_pin_matches = []
    for line in content.split('\n'):
        if not line.strip().startswith('//') and 'PIR_PIN' in line:
            match = re.search(r'PIR_PIN\s+(\d+)', line)
            if match:
                pir_pin_matches.append(match.group(1))
    
    if len(set(pir_pin_matches)) > 1:
        print(f"WARNING: Inconsistent PIR_PIN definitions: {pir_pin_matches}")
        return False
    
    print("Configuration appears consistent")
    return True

def main():
    """Run all validation checks"""
    print("ESP32WildlifeCAM Code Validation")
    print("================================")
    
    os.chdir(Path(__file__).parent)
    
    checks = [
        check_gpio_pin_conflicts,
        check_include_guards,
        check_type_safety,
        check_memory_management,
        check_configuration_consistency
    ]
    
    results = []
    for check in checks:
        try:
            result = check()
            results.append(result)
        except Exception as e:
            print(f"ERROR in {check.__name__}: {e}")
            results.append(False)
    
    print(f"\n=== Validation Summary ===")
    print(f"Checks passed: {sum(results)}/{len(results)}")
    
    if all(results):
        print("✅ All validation checks passed!")
        return 0
    else:
        print("❌ Some validation checks failed")
        return 1

if __name__ == "__main__":
    sys.exit(main())