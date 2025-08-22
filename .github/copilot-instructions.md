# ESP32 Wildlife Camera Development Instructions

**CRITICAL**: Always follow these instructions first and only search for additional information if these instructions are incomplete or found to be in error.

## Working Effectively

### Bootstrap and Build System Setup
Install PlatformIO and dependencies:
```bash
# Install PlatformIO (Python 3.7+ required)
pip3 install platformio

# Verify installation
pio --version

# Navigate to firmware directory
cd firmware/

# Install platform and dependencies (FIRST BUILD ONLY)
pio platform install espressif32
pio lib install
```

### Building the Firmware
```bash
# Build for ESP32-S3 (default target)
pio run -e esp32-s3-devkitc-1

# Build with verbose output for debugging
pio run -e esp32-s3-devkitc-1 --verbose

# Clean build cache if needed
pio run -t clean
```

**TIMING**: Initial build takes 15-45 minutes depending on internet connection and host performance. **NEVER CANCEL** - set timeout to 60+ minutes minimum.

**NETWORK REQUIREMENT**: First build requires internet access to download ESP32 platform and libraries. Subsequent builds work offline.

### Code Validation and Testing
```bash
# Run comprehensive validation script (ALWAYS WORKS OFFLINE)
python3 validate_fixes.py

# Syntax checking and static analysis (requires network for first-time setup)
pio check --skip-packages

# Static analysis with specific tool (requires network + tool installation)
pio check --tool cppcheck --severity medium
```

**TIMING**: Validation script completes in under 30 seconds. Static analysis takes 2-5 minutes. **NEVER CANCEL** builds or analysis.

**OFFLINE CAPABILITY**: The validation script works completely offline. PlatformIO commands require initial network setup.

## Hardware Configuration

### Board Support
- **Primary**: AI-Thinker ESP32-CAM (fully supported)
- **Planned**: ESP32-S3-CAM, ESP-EYE, M5Stack Timer Camera
- **Auto-detection**: System automatically detects board type via GPIO fingerprinting

### Configuration Files
Edit `firmware/src/config.h` for hardware-specific settings:
```cpp
// Camera board type (auto-detected if not specified)
#define CAMERA_MODEL_AI_THINKER

// LoRa networking (conflicts with SD card on AI-Thinker)
#define LORA_ENABLED true
#define SD_CARD_ENABLED false  // Disabled due to pin conflicts

// Power management
#define SOLAR_VOLTAGE_THRESHOLD 3.2
#define DEEP_SLEEP_DURATION 300  // seconds
```

### Pin Conflict Resolution
**CRITICAL**: AI-Thinker ESP32-CAM has limited GPIO pins. Current configuration:
- LoRa enabled, SD card disabled (pin conflicts)
- Vibration sensor disabled (conflicts with LoRa CS)
- IR LED disabled (conflicts with LoRa DIO0)

Always check `validate_fixes.py` for pin conflict validation.

## Project Navigation

### Key Directory Structure
```
firmware/src/
├── main.cpp              # Main application entry point
├── config.h              # Hardware and feature configuration
├── hal/                  # Hardware Abstraction Layer
│   ├── board_detector.cpp # Automatic board detection
│   ├── camera_board.h    # Board interface definitions
│   └── esp32_cam.cpp     # AI-Thinker specific implementation
├── configs/              # Board-specific profiles
│   └── board_profiles.h  # GPIO maps and power profiles
└── [modules]/            # Feature modules (camera, LoRa, solar, etc.)

docs/
├── board_compatibility.md    # Supported hardware
└── hardware_selection_guide.md # Hardware recommendations

examples/
├── board_detection/      # Board detection example
└── multi_board_farm/     # Multi-board deployment example
```

### Critical Files to Review When Making Changes
- `firmware/src/config.h` - All configuration parameters
- `firmware/src/hal/board_detector.cpp` - Board detection logic
- `firmware/src/configs/board_profiles.h` - Hardware-specific GPIO maps
- `validate_fixes.py` - Comprehensive validation script

## Validation Scenarios

### After Making Hardware Changes
```bash
# Validate GPIO pin assignments
python3 validate_fixes.py

# Check specific board configuration
cd examples/board_detection/
pio run -e esp32-s3-devkitc-1 --target upload --upload-port [USB_PORT]
```

### After Code Changes
```bash
# Compile and validate syntax
pio run -e esp32-s3-devkitc-1

# Run static analysis
pio check --skip-packages

# Validate all fixes
python3 validate_fixes.py
```

**MANUAL VALIDATION**: After successful compilation, always test on actual hardware:
1. Connect ESP32-CAM via USB programmer (FTDI or CP2102)
2. Upload firmware: `pio run -t upload --upload-port /dev/ttyUSB0`
3. Monitor serial output: `pio device monitor -b 115200`
4. Verify board detection and camera initialization
5. Test motion detection if PIR sensor connected

**NOTE**: Hardware testing requires physical ESP32-CAM board connected via USB programmer. Development environment validation uses code analysis only.

### Pre-commit Validation
Always run before committing changes:
```bash
# Full validation suite
python3 validate_fixes.py

# Build verification
pio run -e esp32-s3-devkitc-1

# Check code formatting (if applicable)
# Note: No automatic formatter configured - maintain existing style
```

## Common Development Tasks

### Adding New Board Support
1. Add GPIO map to `firmware/src/configs/board_profiles.h`
2. Implement board class in `firmware/src/hal/[board_name].cpp`
3. Update detection logic in `firmware/src/hal/board_detector.cpp`
4. Add validation to `validate_fixes.py`
5. Test with example: `examples/board_detection/`

### Modifying Hardware Configuration
1. Edit `firmware/src/config.h`
2. Check for pin conflicts: `python3 validate_fixes.py`
3. Update board profiles if needed: `firmware/src/configs/board_profiles.h`
4. Test build: `pio run -e esp32-s3-devkitc-1`

### Power Optimization Changes
1. Modify sleep settings in `firmware/src/config.h`
2. Update power profiles in `firmware/src/configs/board_profiles.h`
3. Test with solar manager: check `firmware/src/solar_manager.cpp`
4. Validate power consumption calculations

## Build Timing and Expectations

### Expected Build Times
- **Clean build**: 15-45 minutes (includes platform download)
- **Incremental build**: 30 seconds - 2 minutes
- **Library updates**: 5-15 minutes
- **Platform updates**: 10-30 minutes

### Critical Timeout Settings
**NEVER CANCEL** any of these operations:
- `pio platform install espressif32` - Set timeout to 30+ minutes
- `pio run` (clean build) - Set timeout to 60+ minutes  
- `pio lib install` - Set timeout to 20+ minutes
- `pio check --tool cppcheck` - Set timeout to 10+ minutes

### Network Dependencies
Build requires internet connectivity for:
- ESP32 platform download (first build only)
- Library dependencies download
- Toolchain installation

**KNOWN ISSUE**: In restricted network environments, platform downloads may fail with `HTTPClientError`. This is expected behavior and requires network access to resolve.

## Troubleshooting

### Build Failures
1. Check Python version: `python3 --version` (3.7+ required)
2. Verify PlatformIO: `pio --version`
3. Clean build cache: `pio run -t clean`
4. Reset platform: `pio platform uninstall espressif32 && pio platform install espressif32`

### Pin Conflict Errors
1. Run validation: `python3 validate_fixes.py`
2. Review pin assignments in `firmware/src/config.h`
3. Check board-specific limitations in `docs/board_compatibility.md`
4. Consider disabling conflicting features

### Hardware Detection Issues
1. Test detection example: `examples/board_detection/`
2. Check GPIO availability with multimeter
3. Verify board type in `firmware/src/config.h`
4. Review detection logic in `firmware/src/hal/board_detector.cpp`

## Network Configuration (LoRa Mesh)

### LoRa Setup
```cpp
// Configure in firmware/src/config.h
#define LORA_ENABLED true
#define LORA_FREQUENCY 915E6      // US frequency (433E6 for EU)
#define NODE_ID 1                 // Unique node identifier
#define MAX_MESH_NODES 10         // Network size limit
```

### Testing Network Functionality
1. Build and upload to multiple boards
2. Monitor serial output for mesh messages
3. Test with example: `examples/multi_board_farm/`

## Code Quality Standards

### Validation Requirements
- All changes must pass `python3 validate_fixes.py`
- No GPIO pin conflicts allowed
- Include guards required in all header files
- Type safety validation must pass

### Documentation Standards
- Update pin assignments in comments when changing GPIO
- Document disabled features and reasons
- Maintain board compatibility matrix in `docs/`

## Emergency Procedures

### Complete Build Reset
```bash
# Nuclear option - reset everything
rm -rf .pio/
pio platform uninstall espressif32
pio platform install espressif32
pio run -e esp32-s3-devkitc-1
```

### Reverting Hardware Changes
```bash
# Check current conflicts
python3 validate_fixes.py

# Revert specific file
git checkout HEAD -- firmware/src/config.h

# Validate restoration
python3 validate_fixes.py
```

**Remember**: This is an embedded systems project. Build times are inherently long, hardware constraints are real, and validation is critical. Be patient with build processes and always validate changes thoroughly.

## Frequently Referenced Information

### Repository Root Contents
```
.github/                  # GitHub configurations and this file
.gitignore               # Git ignore patterns
AUDIT_REPORT.md          # Comprehensive code audit results
README.md                # Project overview and quick start
docs/                    # Technical documentation
examples/                # Usage examples and test code
firmware/                # ESP32 firmware source code
hardware/                # Circuit schematics and PCB files
validate_fixes.py        # Comprehensive validation script
```

### firmware/src/ Contents
```
camera_handler.cpp/.h    # Camera capture and processing
cellular_manager.cpp/.h  # Cellular communication
config.h                 # MAIN CONFIGURATION FILE
configs/                 # Board-specific configurations
data_compression.cpp/.h  # Image and data compression
gps_handler.cpp/.h       # GPS location services
hal/                     # Hardware Abstraction Layer
lora_mesh.cpp/.h         # LoRa mesh networking
main.cpp                 # Application entry point
motion_filter.cpp/.h     # Motion detection filtering
network_selector.cpp/.h  # Network selection logic
satellite_comm.cpp/.h    # Satellite communication
solar_manager.cpp/.h     # Solar power management
```

### Quick Reference Commands
```bash
# Check repository status
ls -la                                    # List all files
python3 validate_fixes.py               # Run validation suite
cat firmware/src/config.h | grep define # Show all configurations

# Build and development (requires network for initial setup)
cd firmware/
pio run -t clean                        # Clean build artifacts
pio run -e esp32-s3-devkitc-1          # Build firmware
pio device monitor -b 115200            # Monitor serial output

# File navigation
find . -name "*.h" | head -10           # Find header files
grep -r "GPIO" firmware/src/             # Search for GPIO usage
ls firmware/src/hal/                     # List HAL implementations
```

## Command Verification Checklist

Before relying on these instructions, verify key commands work in your environment:

```bash
# Core tools (should always work)
python3 --version                       # Should show Python 3.7+
pip3 --version                          # Should show pip version
python3 validate_fixes.py              # Should show "All validation checks passed!"

# PlatformIO tools (require installation)
pio --version                           # Should show PlatformIO version 6.x+
pio platform list                      # Should list available platforms

# Repository navigation (should always work)
find . -name "*.h" | wc -l             # Should show ~16 header files
cat firmware/src/config.h | grep -c define # Should show ~154 define statements
ls firmware/src/hal/                   # Should show 7 HAL files
```

If any verification command fails, check the troubleshooting section above.