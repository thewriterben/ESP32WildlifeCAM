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
    
    # Check feature states to handle conditional pin assignments
    sd_card_enabled = 'SD_CARD_ENABLED true' in content
    i2s_microphone_enabled = 'I2S_MICROPHONE_ENABLED true' in content
    analog_microphone_enabled = 'ANALOG_MICROPHONE_ENABLED true' in content
    camera_ai_thinker = 'CAMERA_MODEL_AI_THINKER' in content and not content.find('// #define CAMERA_MODEL_AI_THINKER') >= 0
    
    print(f"Configuration state:")
    print(f"  SD Card: {'ENABLED' if sd_card_enabled else 'DISABLED'}")
    print(f"  I2S Microphone: {'ENABLED' if i2s_microphone_enabled else 'DISABLED'}")
    print(f"  Analog Microphone: {'ENABLED' if analog_microphone_enabled else 'DISABLED'}")
    print(f"  AI-Thinker Camera: {'ENABLED' if camera_ai_thinker else 'DISABLED'}")
    
    # Extract pin assignments (ignore commented lines and handle conditionals)
    pin_assignments = {}
    # Specific patterns to match only actual GPIO pin assignments
    pin_patterns = [
        r'^[^/]*#define\s+(\w*_PIN)\s+(\d+)',                 # Pattern for *_PIN defines
        r'^[^/]*#define\s+(\w*GPIO_NUM)\s+(\d+)',             # Pattern for *GPIO_NUM defines  
        r'^[^/]*#define\s+(LORA_SCK|LORA_MISO|LORA_MOSI|LORA_CS|LORA_RST|LORA_DIO0)\s+(\d+)', # LoRa GPIO pins only
        r'^[^/]*#define\s+(PIR_PIN)\s+(\d+)',                 # PIR sensor pin
    ]
    conflicts_found = False
    
    # Track conditional blocks
    in_conditional = False
    conditional_depth = 0
    skip_current_block = False
    
    for line_num, line in enumerate(content.split('\n'), 1):
        stripped_line = line.strip()
        
        # Skip commented lines
        if stripped_line.startswith('//'):
            continue
        
        # Handle conditional compilation directives
        if stripped_line.startswith('#if'):
            conditional_depth += 1
            # Check if this conditional should be skipped based on current configuration
            if '#if SD_CARD_ENABLED' in stripped_line:
                skip_current_block = not sd_card_enabled  # Skip if SD card is disabled
            elif '#if !SD_CARD_ENABLED' in stripped_line:
                skip_current_block = sd_card_enabled  # Skip if SD card is enabled
            elif '#if !defined(CAMERA_MODEL_AI_THINKER)' in stripped_line:
                skip_current_block = camera_ai_thinker  # Skip if AI-Thinker is defined
            else:
                skip_current_block = False
            continue
        elif stripped_line.startswith('#else'):
            skip_current_block = not skip_current_block  # Flip the skip state for else block
            continue
        elif stripped_line.startswith('#endif'):
            conditional_depth -= 1
            if conditional_depth == 0:
                skip_current_block = False  # Reset when exiting conditional blocks
            continue
        elif stripped_line.startswith('#undef'):
            continue  # Skip undef directives
            
        # Skip processing if we're in a conditional block that should be ignored
        if skip_current_block:
            continue
            
        for pattern in pin_patterns:
            match = re.search(pattern, line, re.IGNORECASE)
            if match:
                define_name = match.group(1)
                pin_number = int(match.group(2))
                
                # Skip negative pins (they indicate "not used")
                if pin_number < 0:
                    continue
                    
                # Only consider valid ESP32 GPIO pins (0-39)
                if pin_number > 39:
                    continue
                
                # Skip disabled features
                if 'I2S_' in define_name and not i2s_microphone_enabled:
                    continue
                if 'ANALOG_MIC_PIN' in define_name and not analog_microphone_enabled:
                    continue
                
                if pin_number in pin_assignments:
                    print(f"CONFLICT: GPIO {pin_number} used for both {pin_assignments[pin_number]} and {define_name}")
                    conflicts_found = True
                else:
                    pin_assignments[pin_number] = define_name
                break  # Only match first pattern to avoid duplicates
    
    print(f"Found {len(pin_assignments)} active GPIO pin assignments")
    for pin, name in sorted(pin_assignments.items()):
        print(f"  GPIO {pin:2d}: {name}")
    
    if conflicts_found:
        print("❌ GPIO pin conflicts detected!")
        return False
    else:
        print("✅ No GPIO pin conflicts found")
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
    frame_buffer_usage = {}
    
    for source_file in source_files:
        content = source_file.read_text()
        
        # Count esp_camera_fb_get() and esp_camera_fb_return() calls
        get_calls = len(re.findall(r'esp_camera_fb_get\s*\(\s*\)', content))
        return_calls = len(re.findall(r'esp_camera_fb_return\s*\(', content))
        
        if get_calls > 0 or return_calls > 0:
            frame_buffer_usage[str(source_file)] = {
                'get_calls': get_calls,
                'return_calls': return_calls
            }
            
            # Check for obvious imbalances within the same file
            # Note: It's OK for a function to get a frame buffer and return it to caller
            # Only flag if there are significantly more gets than returns in same file
            if get_calls > return_calls + 1:  # Allow for one extra get (returned to caller)
                issues.append(f"{source_file}: {get_calls} esp_camera_fb_get() calls vs {return_calls} esp_camera_fb_return() calls - potential memory leak")
    
    if frame_buffer_usage:
        print("Frame buffer usage analysis:")
        for file, usage in frame_buffer_usage.items():
            print(f"  {file}: {usage['get_calls']} get, {usage['return_calls']} return")
    
    if issues:
        print("Memory management issues found:")
        for issue in issues:
            print(f"  {issue}")
        return False
    else:
        print("Frame buffer usage appears balanced across the codebase")
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