# Troubleshooting Guide

## ESP32 Wildlife Camera - Common Issues and Solutions

### Quick Diagnostic Checklist

Before diving into specific issues, run through this quick checklist:

```bash
☐ Power LED on ESP32 board is illuminated
☐ Battery voltage above 3.0V (check with multimeter)
☐ Solar panel producing voltage in sunlight
☐ SD card properly inserted and formatted
☐ All cable connections secure
☐ No visible moisture in enclosure
☐ LoRa antenna properly connected
☐ Camera lens clean and unobstructed
☐ PIR sensor not blocked by vegetation
☐ Serial monitor showing system startup messages
```

## Power System Issues

### Problem: Camera Won't Turn On

**Symptoms:**
- No LED activity on ESP32 board
- No serial output when connected to computer
- System completely unresponsive

**Diagnostic Steps:**
```bash
1. Check battery voltage with multimeter
   - Should be >3.0V for operation
   - <2.8V indicates critically discharged battery

2. Verify solar charging circuit
   - Measure solar panel voltage in direct sunlight
   - Should see 4-6V depending on panel specification
   - Check TP4056 charging module LEDs

3. Test power connections
   - Verify battery polarity (red=+, black=-)
   - Check all solder joints for continuity
   - Ensure TP4056 output connected to ESP32 VIN
```

**Solutions:**
```bash
# Battery completely discharged
1. Connect external 5V charger to TP4056 micro-USB port
2. Charge for 2-4 hours before testing
3. Replace battery if won't hold charge after full charging cycle

# Solar charging not working
1. Clean solar panel surface
2. Check diode orientation (cathode toward TP4056)
3. Verify cable connections are not corroded
4. Test panel output voltage with multimeter

# Power supply issues
1. Check ESP32 3.3V regulator function
2. Measure voltage at ESP32 VIN pin
3. Look for short circuits with multimeter continuity test
4. Replace TP4056 module if damaged
```

### Problem: Battery Drains Quickly

**Symptoms:**
- System runs for <24 hours on full charge
- Battery voltage drops rapidly
- Frequent low battery warnings

**Diagnostic Steps:**
```bash
1. Monitor current consumption
   - Use ammeter in series with battery
   - Normal deep sleep: <1mA
   - Active operation: 50-200mA
   - Camera capture: 200-500mA peak

2. Check sleep mode operation
   Serial output should show:
   "Entering deep sleep..."
   followed by silence

3. Review trigger frequency
   - Check daily trigger counts
   - Look for excessive false positives
```

**Solutions:**
```bash
# Excessive power consumption
1. Verify deep sleep is working:
   esp_sleep_enable_timer_wakeup(300 * 1000000ULL);
   esp_deep_sleep_start();

2. Reduce image quality settings:
   #define CAMERA_FRAME_SIZE FRAMESIZE_VGA  // Smaller images
   #define CAMERA_JPEG_QUALITY 20           // Lower quality

3. Adjust trigger sensitivity:
   #define MAX_DAILY_TRIGGERS 20            // Reduce daily limit
   MotionFilter::setMotionSensitivity(30);  // Less sensitive

4. Optimize active hours:
   #define TRIGGER_ACTIVE_HOURS_START 7
   #define TRIGGER_ACTIVE_HOURS_END 19      // Shorter active period
```

### Problem: Solar Charging Inconsistent

**Symptoms:**
- Charging LED flickers or doesn't illuminate
- Battery voltage varies significantly
- System works in direct sun but fails in shade

**Diagnostic Steps:**
```bash
1. Measure solar panel performance
   - Full sun: Should produce rated voltage
   - Partial shade: Voltage drops significantly
   - Check for physical damage or dirt

2. Test charging circuit
   - Measure voltage before and after diode
   - Check TP4056 input voltage and current
   - Verify battery connection to TP4056

3. Monitor charging behavior
   Serial output should show:
   "Charging started (Solar: 4.2V, Battery: 3.8V)"
```

**Solutions:**
```bash
# Optimize solar panel placement
1. Ensure south-facing orientation (Northern Hemisphere)
2. Adjust tilt angle seasonally (15° optimal)
3. Clear vegetation blocking sunlight
4. Clean panel surface regularly

# Improve charging efficiency
1. Use higher capacity panel if needed (up to 5W)
2. Add larger filtering capacitor (470µF)
3. Consider MPPT charge controller for better efficiency
4. Verify diode is low voltage drop type (Schottky)

# Battery maintenance
1. Replace battery annually in high-use applications
2. Use protected 18650 cells only
3. Check for sulfation if using lead-acid backup
4. Calibrate voltage monitoring:
   SolarManager::calibrateVoltage(actual_battery_V, actual_solar_V);
```

## Camera System Issues

### Problem: No Images Captured

**Symptoms:**
- Motion detection triggers but no images saved
- SD card remains empty or has old images only
- Camera initialization errors in serial output

**Diagnostic Steps:**
```bash
1. Check camera initialization
   Serial should show: "Camera initialized successfully"
   If not: "Error: Camera initialization failed!"

2. Test camera manually
   CameraHandler::testCamera();  // In firmware
   
3. Verify SD card operation
   - Format as FAT32 before first use
   - Check for corruption with computer
   - Ensure card is Class 10 or faster

4. Check file system
   ls /images/     // Should show captured images
```

**Solutions:**
```bash
# Camera module issues
1. Check camera cable connection to ESP32
2. Verify camera power supply (3.3V)
3. Test with known good camera module
4. Check pin definitions in config.h match hardware

# SD card problems
1. Reformat SD card (FAT32, 32KB cluster size)
2. Try different SD card (max 32GB recommended)
3. Check SD card slot for proper contact
4. Verify SD card pins in config.h:
   #define SD_CS_PIN 12
   #define SD_MOSI_PIN 15
   #define SD_CLK_PIN 14
   #define SD_MISO_PIN 2

# File system issues
1. Initialize LittleFS if using internal storage:
   LittleFS.format();  // Will erase all data!
   
2. Check available space:
   size_t totalBytes = SD_MMC.totalBytes();
   size_t usedBytes = SD_MMC.usedBytes();
   
3. Verify directory creation:
   SD_MMC.mkdir("/images");
   SD_MMC.mkdir("/logs");
```

### Problem: Poor Image Quality

**Symptoms:**
- Images are blurry, too dark, or overexposed
- Color balance issues
- Focus problems

**Diagnostic Steps:**
```bash
1. Check camera settings
   sensor_t* sensor = esp_camera_sensor_get();
   // Review all sensor settings

2. Test different lighting conditions
   - Bright sunlight
   - Overcast conditions  
   - Dawn/dusk lighting

3. Verify camera mounting
   - Check for vibration
   - Ensure lens is clean
   - Verify focus adjustment
```

**Solutions:**
```bash
# Optimize camera settings for wildlife
1. Adjust exposure for outdoor conditions:
   sensor->set_ae_level(sensor, 0);        // Auto exposure
   sensor->set_agc_gain(sensor, 5);        // Moderate gain
   sensor->set_aec_value(sensor, 300);     // Exposure value

2. Fine-tune for lighting conditions:
   CameraHandler::adjustForLighting(LIGHTING_NORMAL);
   
3. Improve focus and clarity:
   sensor->set_lenc(sensor, 1);            // Lens correction
   sensor->set_raw_gma(sensor, 1);         // Gamma correction
   sensor->set_bpc(sensor, 0);             // Black pixel cancel
   sensor->set_wpc(sensor, 1);             // White pixel cancel

4. Color balance optimization:
   sensor->set_whitebal(sensor, 1);        // Auto white balance
   sensor->set_awb_gain(sensor, 1);        // AWB gain
   sensor->set_wb_mode(sensor, 0);         // Auto WB mode

# Physical adjustments
1. Clean camera lens with microfiber cloth
2. Adjust camera angle to avoid direct sunlight
3. Use camera hood to reduce glare
4. Ensure stable mounting without vibration
```

### Problem: Motion Detection Not Working

**Symptoms:**
- No motion triggers despite wildlife activity
- Excessive false positives from wind/weather
- PIR sensor not responding

**Diagnostic Steps:**
```bash
1. Test PIR sensor directly
   - Wave hand in front of sensor
   - Check for trigger LED on sensor board
   - Verify power supply to sensor (3.3V)

2. Monitor serial output for motion events
   Should see: "Motion detected!"
   Followed by: "Valid motion after filtering"

3. Check motion filter settings
   MotionStats stats = MotionFilter::getMotionStats();
   // Review wind speed, temperature, etc.
```

**Solutions:**
```bash
# PIR sensor adjustment
1. Adjust sensitivity potentiometer on sensor
   - Start at 50% position
   - Increase for longer range detection
   - Decrease to reduce false positives

2. Optimize time delay setting
   - Set to minimum on sensor board
   - Software handles debouncing
   - Prevents multiple rapid triggers

3. Positioning and coverage
   - Mount 2-3 meters above ground
   - Angle slightly downward
   - Clear vegetation from detection zone
   - Test detection range with walking

# Software motion filtering
1. Adjust motion sensitivity:
   MotionFilter::setMotionSensitivity(75);  // Increase sensitivity
   
2. Fine-tune weather filtering:
   #define WIND_THRESHOLD 20               // Increase for windy areas
   #define RAIN_THRESHOLD 1.0              // Adjust for local conditions
   
3. Modify consecutive motion requirements:
   // In motion_filter.cpp, reduce required consecutive motions
   if (consecutiveMotions < 1) {  // Was 2, now 1
   
4. Debug motion detection:
   #define DEBUG_MOTION true
   // Enable detailed motion logging
```

## Network Communication Issues

### Problem: LoRa Network Not Working

**Symptoms:**
- No network communication between nodes
- LoRa initialization fails
- Poor signal quality or range

**Diagnostic Steps:**
```bash
1. Check LoRa module initialization
   Serial should show: "LoRa mesh network initialized - Node ID: X"
   
2. Verify antenna connection
   - SMA connector tight
   - No damage to antenna or cable
   - Correct frequency for region

3. Test signal quality
   SignalQuality quality = LoraMesh::getSignalQuality();
   // RSSI should be >-100 dBm for reliable communication
```

**Solutions:**
```bash
# Hardware verification
1. Check LoRa module connections:
   #define LORA_SCK 18      // SPI clock
   #define LORA_MISO 19     // SPI MISO  
   #define LORA_MOSI 23     // SPI MOSI
   #define LORA_CS 5        // Chip select
   #define LORA_RST 14      // Reset
   #define LORA_DIO0 26     // Interrupt

2. Verify power supply to LoRa module (3.3V)
3. Test with known good LoRa module
4. Check antenna for proper impedance (50Ω)

# Network configuration
1. Ensure all nodes use same frequency:
   #define LORA_FREQUENCY 915E6  // Match all nodes
   
2. Verify network sync word:
   #define LORA_SYNC_WORD 0x12   // Must match across network
   
3. Optimize for range vs speed:
   // For maximum range
   #define LORA_SPREADING_FACTOR 12
   #define LORA_SIGNAL_BANDWIDTH 62.5E3
   
   // For higher data rate
   #define LORA_SPREADING_FACTOR 7
   #define LORA_SIGNAL_BANDWIDTH 125E3

4. Test network step by step:
   - Start with two nodes close together (<100m)
   - Verify basic communication
   - Gradually increase distance
   - Add additional nodes one at a time

# Range optimization
1. Position antennas for line-of-sight when possible
2. Elevate antennas above ground level
3. Avoid metal structures that cause interference
4. Use higher gain antennas if legal in region
5. Increase transmission power if within legal limits:
   #define LORA_TX_POWER 20  // Maximum legal power
```

### Problem: Poor Network Range

**Symptoms:**
- Communication works at short range but fails at distance
- Intermittent network connectivity
- High packet loss rates

**Solutions:**
```bash
# Antenna improvements
1. Upgrade to higher gain antenna (3-5 dBi)
2. Position antenna vertically for omnidirectional coverage
3. Use antenna with better impedance matching
4. Check coax cable loss (keep <1 meter if possible)

# RF environment optimization
1. Survey for interference sources:
   - WiFi routers (2.4GHz can interfere with 900MHz harmonics)
   - Other LoRa devices
   - Industrial equipment
   - Power lines

2. Site selection:
   - Higher elevation for both nodes
   - Clear line of sight when possible
   - Avoid valleys and dense vegetation
   - Consider seasonal foliage changes

# Protocol optimization
1. Use acknowledgments for critical data:
   // Implement retry mechanism with exponential backoff
   
2. Optimize packet size:
   - Keep payloads small (<200 bytes)
   - Use data compression
   - Send only essential information

3. Implement mesh routing:
   // Use intermediate nodes as repeaters
   // Automatic route discovery and maintenance
```

## Environmental and Physical Issues

### Problem: Moisture in Enclosure

**Symptoms:**
- Condensation visible inside enclosure
- Corrosion on electronic components
- Intermittent failures during humid weather

**Solutions:**
```bash
# Improve sealing
1. Check O-ring condition and installation
2. Apply silicone grease to gasket
3. Verify all cable glands are tight
4. Inspect for cracks in enclosure

# Moisture management
1. Add desiccant packets inside enclosure
2. Install breathable membrane for pressure equalization
3. Create drainage path for any condensation
4. Use conformal coating on PCB if needed

# Design improvements
1. Slope enclosure bottom for drainage
2. Add ventilation with moisture trap
3. Use materials with lower thermal expansion
4. Position away from direct temperature changes
```

### Problem: Wildlife Interference

**Symptoms:**
- Camera position changed or damaged
- Cables chewed or damaged
- Enclosure showing animal damage

**Solutions:**
```bash
# Physical protection
1. Use steel cable for mounting security
2. Install at sufficient height (>3 meters)
3. Use tamper-resistant screws
4. Add protective wire mesh around cables

# Deterrent strategies
1. Use scent deterrents around installation
2. Choose mounting locations animals can't reach
3. Camouflage installation to be less obvious
4. Route cables through protective conduit

# Monitoring and alerts
1. Add tamper detection sensor
2. Set up daily status check via LoRa
3. Use GPS tracking for theft protection
4. Log all system status changes with timestamps
```

## Software and Firmware Issues

### Problem: System Crashes or Reboots

**Symptoms:**
- Unexpected restarts
- Watchdog timer resets
- System hangs requiring power cycle

**Diagnostic Steps:**
```bash
1. Check reset reason:
   esp_reset_reason_t reason = esp_reset_reason();
   // ESP_RST_POWERON, ESP_RST_WDT, ESP_RST_PANIC, etc.

2. Monitor heap memory:
   uint32_t freeHeap = ESP.getFreeHeap();
   // Should stay above 50KB for stable operation

3. Review serial output for error messages:
   - Stack overflow warnings
   - Memory allocation failures
   - Hardware communication errors
```

**Solutions:**
```bash
# Memory management
1. Always return camera frame buffers:
   camera_fb_t* fb = esp_camera_fb_get();
   // ... use image data
   esp_camera_fb_return(fb);  // Critical!

2. Use static allocation where possible:
   char buffer[1024];  // Stack allocation
   // Avoid: char* buffer = malloc(1024);

3. Monitor memory usage:
   if (ESP.getFreeHeap() < 50000) {
       DEBUG_PRINTLN("Low memory warning!");
   }

# Watchdog and stability
1. Add periodic watchdog feeding:
   esp_task_wdt_reset();  // Reset watchdog timer

2. Handle exceptions gracefully:
   try {
       riskyOperation();
   } catch (...) {
       DEBUG_PRINTLN("Operation failed, continuing...");
   }

3. Implement software reset for recovery:
   if (systemHealthCheck() == false) {
       ESP.restart();  // Clean restart
   }
```

### Problem: Firmware Upload Fails

**Symptoms:**
- "Failed to connect" errors during upload
- Compilation errors
- Boot loop after firmware update

**Solutions:**
```bash
# Upload issues
1. Hold BOOT button during upload initiation
2. Use lower upload speed: upload_speed = 115200
3. Check USB cable quality and connection
4. Try different USB port

# Compilation problems
1. Update PlatformIO platform:
   pio platform update espressif32
   
2. Clean build directory:
   pio run --target clean
   
3. Check library compatibility:
   pio lib update  // Update all libraries

# Boot loop recovery
1. Enter download mode:
   - Hold BOOT button
   - Press and release RESET button
   - Release BOOT button
   
2. Upload known good firmware
3. Check for hardware conflicts:
   - Disconnect peripherals during upload
   - Verify power supply stability
```

## Performance Optimization

### Problem: Slow Image Capture

**Symptoms:**
- Long delay between motion detection and image capture
- System appears sluggish
- High memory usage warnings

**Solutions:**
```bash
# Camera optimization
1. Use optimal frame size for memory:
   #define CAMERA_FRAME_SIZE FRAMESIZE_SVGA  // 800x600 balance
   
2. Adjust JPEG quality for speed:
   #define CAMERA_JPEG_QUALITY 15            // Faster compression
   
3. Use dual frame buffers:
   #define CAMERA_FB_COUNT 2                 // Improves capture speed

# CPU optimization
1. Increase CPU frequency during capture:
   setCpuFrequencyMhz(240);  // Maximum speed
   // ... capture image
   setCpuFrequencyMhz(80);   // Reduce for power saving

2. Optimize critical code paths:
   // Use ISR for motion detection
   IRAM_ATTR void motionISR() {
       motionDetected = true;
   }

3. Minimize processing during capture:
   // Move heavy processing to background tasks
```

## System Monitoring and Diagnostics

### Comprehensive Health Check

```cpp
void systemHealthCheck() {
    DEBUG_PRINTLN("=== SYSTEM HEALTH CHECK ===");
    
    // Power system
    float batteryV = SolarManager::getBatteryVoltage();
    int batteryPct = SolarManager::getBatteryPercentage();
    bool charging = SolarManager::isCharging();
    DEBUG_PRINTF("Battery: %.2fV (%d%%) %s\n", 
                batteryV, batteryPct, charging ? "Charging" : "Discharging");
    
    // Memory usage
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    DEBUG_PRINTF("Memory: %d/%d bytes free (%.1f%%)\n", 
                freeHeap, totalHeap, (float)freeHeap/totalHeap*100);
    
    // Storage
    size_t totalBytes = SD_MMC.totalBytes();
    size_t usedBytes = SD_MMC.usedBytes();
    DEBUG_PRINTF("Storage: %d/%d MB used (%.1f%%)\n", 
                usedBytes/1024/1024, totalBytes/1024/1024, 
                (float)usedBytes/totalBytes*100);
    
    // Network
    MeshNetworkStatus netStatus = LoraMesh::getNetworkStatus();
    DEBUG_PRINTF("Network: %d nodes, RSSI: %d dBm\n", 
                netStatus.connectedNodes, netStatus.rssi);
    
    // Camera
    CameraStatus camStatus = CameraHandler::getStatus();
    DEBUG_PRINTF("Camera: %s, %d images captured\n", 
                camStatus.initialized ? "OK" : "ERROR", 
                camStatus.imageCount);
                
    DEBUG_PRINTLN("=== END HEALTH CHECK ===\n");
}
```

### Automated Problem Detection

```cpp
void automaticDiagnostics() {
    // Battery health
    if (SolarManager::getBatteryVoltage() < BATTERY_LOW_THRESHOLD) {
        DEBUG_PRINTLN("WARNING: Low battery detected");
        enterPowerSavingMode();
    }
    
    // Memory leaks
    static uint32_t lastFreeHeap = ESP.getFreeHeap();
    uint32_t currentFreeHeap = ESP.getFreeHeap();
    if (currentFreeHeap < lastFreeHeap - 10000) {  // 10KB decrease
        DEBUG_PRINTLN("WARNING: Possible memory leak detected");
    }
    lastFreeHeap = currentFreeHeap;
    
    // Network connectivity
    MeshNetworkStatus status = LoraMesh::getNetworkStatus();
    if (millis() - status.lastMessageTime > 300000) {  // 5 minutes
        DEBUG_PRINTLN("WARNING: Network communication timeout");
        // Attempt network restart
        LoraMesh::cleanup();
        LoraMesh::init();
    }
    
    // Storage space
    if (SD_MMC.usedBytes() > SD_MMC.totalBytes() * 0.9) {  // >90% full
        DEBUG_PRINTLN("WARNING: Storage nearly full");
        // Implement automatic cleanup
        cleanupOldFiles();
    }
}
```

---

**Need More Help?**

If these solutions don't resolve your issue:

1. **Check the GitHub Issues**: https://github.com/thewriterben/ESP32WildlifeCAM/issues
2. **Post in Discussions**: https://github.com/thewriterben/ESP32WildlifeCAM/discussions  
3. **Review Documentation**: All docs are in the `/docs` folder
4. **Hardware Support**: Consult component datasheets and manufacturer support

**When Reporting Issues:**
- Include serial monitor output
- Describe hardware configuration
- Note environmental conditions
- Provide steps to reproduce problem
- Include system health check output