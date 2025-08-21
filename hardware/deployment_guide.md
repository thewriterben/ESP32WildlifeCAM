# Wildlife Monitoring Deployment Guide

## Deployment Scenarios and Board Selection

This guide helps you choose the right ESP32 camera board and configuration for your specific wildlife monitoring needs.

## Scenario 1: Budget Trail Camera

**Recommended Board**: ESP32-CAM (AI-Thinker)  
**Target Animals**: Small to medium mammals, birds  
**Deployment Duration**: 1-3 months  
**Power Source**: Battery pack with solar panel  

### Configuration
```cpp
// platformio.ini environment
[env:budget_trail_cam]
extends = env:esp32cam_wildlife
build_flags = 
    ${env:esp32cam_wildlife.build_flags}
    -DBUDGET_TRAIL_CAM_MODE=1
    -DCAPTURE_INTERVAL_HOURS=2
    -DLOW_POWER_MODE=1
```

### Hardware Setup
- ESP32-CAM board ($8-12)
- OV2640 camera sensor (included)
- 18650 battery holder + 2x 18650 batteries
- 6V 1W solar panel
- TP4056 charging module
- Waterproof enclosure (IP65)

### Expected Performance
- **Battery Life**: 2-4 weeks without solar
- **Image Quality**: 1600x1200 JPEG, adequate for species ID
- **Storage**: 200-500 images per day
- **Cost**: ~$25 total

## Scenario 2: Professional Research Station

**Recommended Board**: ESP32-S3-EYE or ESP32-S3-CAM  
**Target Animals**: All wildlife types, detailed behavioral studies  
**Deployment Duration**: 6-12 months  
**Power Source**: Solar with large battery bank  

### Configuration
```cpp
// platformio.ini environment
[env:research_station]
extends = env:esp32s3cam_wildlife
build_flags = 
    ${env:esp32s3cam_wildlife.build_flags}
    -DRESEARCH_STATION_MODE=1
    -DHIGH_QUALITY_CAPTURE=1
    -DDATA_LOGGING_ENABLED=1
    -DREMOTE_ACCESS_ENABLED=1
```

### Hardware Setup
- ESP32-S3-CAM or ESP-EYE board ($25-40)
- OV5640 camera sensor (5MP, autofocus)
- 32GB SD card for local storage
- SIM800L cellular module for data transmission
- 12V 20W solar panel
- 12V 7Ah sealed lead acid battery
- Buck converter (12V to 5V)
- Professional weatherproof enclosure

### Expected Performance
- **Battery Life**: Indefinite with proper solar sizing
- **Image Quality**: 2592x1944 JPEG, research grade
- **Storage**: 1000+ images per day with metadata
- **Cost**: ~$150-200 total

## Scenario 3: Remote Forest Monitoring

**Recommended Board**: M5Stack Timer Camera  
**Target Animals**: Large mammals (deer, bears, elk)  
**Deployment Duration**: 3-6 months  
**Power Source**: Built-in battery + solar charging  

### Configuration
```cpp
// platformio.ini environment
[env:remote_forest]
extends = env:m5stack_timercam_wildlife
build_flags = 
    ${env:m5stack_timercam_wildlife.build_flags}
    -DREMOTE_FOREST_MODE=1
    -DMOTION_TRIGGERED_ONLY=1
    -DLORA_TRANSMISSION=1
    -DULTRA_LOW_POWER=1
```

### Hardware Setup
- M5Stack Timer Camera ($35-45)
- Built-in OV3660 sensor
- Built-in PIR motion sensor
- Built-in 270mAh battery
- External 6V 2W solar panel
- LoRa module (SX1276/78) for long-range communication
- Tree mounting bracket
- Camouflage enclosure

### Expected Performance
- **Battery Life**: 1-2 months with motion triggering
- **Image Quality**: 2048x1536 JPEG, excellent detail
- **Transmission Range**: 1-5km LoRa range
- **Cost**: ~$80-100 total

## Scenario 4: Urban Wildlife Study

**Recommended Board**: TTGO T-Camera  
**Target Animals**: Urban birds, small mammals  
**Deployment Duration**: Permanent installation  
**Power Source**: Mains power with battery backup  

### Configuration
```cpp
// platformio.ini environment
[env:urban_wildlife]
extends = env:ttgo_tcamera_wildlife
build_flags = 
    ${env:ttgo_tcamera_wildlife.build_flags}
    -DURBAN_WILDLIFE_MODE=1
    -DCONTINUOUS_MONITORING=1
    -DWIFI_ENABLED=1
    -DTIME_LAPSE_MODE=1
```

### Hardware Setup
- TTGO T-Camera board ($20-30)
- Built-in OV2640 sensor
- Built-in 1.3" TFT display for status
- 32GB SD card
- WiFi connectivity to local network
- 5V power adapter with UPS backup
- Discrete mounting enclosure

### Expected Performance
- **Power**: Continuous operation on mains power
- **Image Quality**: 1600x1200 JPEG with live preview
- **Storage**: Unlimited with WiFi upload
- **Cost**: ~$60-80 total

## Scenario 5: Compact Sensor Network

**Recommended Board**: XIAO ESP32S3 Sense  
**Target Animals**: Small birds, insects  
**Deployment Duration**: 1-6 months  
**Power Source**: Small solar + battery  

### Configuration
```cpp
// platformio.ini environment
[env:sensor_network]
extends = env:xiao_esp32s3_sense_wildlife
build_flags = 
    ${env:xiao_esp32s3_sense_wildlife.build_flags}
    -DSENSOR_NETWORK_MODE=1
    -DMESH_NETWORKING=1
    -DMICRO_POWER_MODE=1
    -DCOMPACT_IMAGES=1
```

### Hardware Setup
- XIAO ESP32S3 Sense ($15-20)
- Built-in OV2640 sensor
- MicroSD card (16GB)
- 3.7V 1000mAh LiPo battery
- 3W mini solar panel
- Ultra-compact enclosure
- Multiple units for network deployment

### Expected Performance
- **Battery Life**: 2-6 weeks per unit
- **Image Quality**: 640x480 to 1280x1024 JPEG
- **Network**: Mesh communication between units
- **Cost**: ~$30-40 per unit

## Power Consumption Comparison

| Board | Scenario | Active Power | Sleep Power | Battery Life |
|-------|----------|--------------|-------------|--------------|
| ESP32-CAM | Budget Trail | 250mW | 10µW | 2-4 weeks |
| ESP32-S3-CAM | Research | 400mW | 15µW | Indefinite* |
| M5Stack Timer | Remote Forest | 300mW | 5µW | 1-2 months |
| TTGO T-Camera | Urban Study | 350mW | - | Mains powered |
| XIAO ESP32S3 | Sensor Network | 150mW | 3µW | 2-6 weeks |

*With adequate solar sizing

## Solar Panel Sizing Guide

### Basic Calculation
```
Daily Energy = (Active Power × Active Hours) + (Sleep Power × Sleep Hours)
Solar Panel = Daily Energy × Safety Factor (3-5x) / Peak Sun Hours
```

### Example Sizing
**ESP32-CAM Budget Trail Camera:**
- Active: 250mW × 0.1 hours = 25mWh
- Sleep: 0.01mW × 23.9 hours = 0.24mWh
- Daily Total: ~25mWh
- Solar Panel: 25mWh × 4 / 4 hours = 25mW minimum
- Recommended: 1W panel (40x safety margin for cloudy days)

## Connectivity Options by Scenario

### WiFi Connectivity
```cpp
// Urban/suburban deployments with WiFi access
#define WIFI_SSID "wildlife_network"
#define WIFI_PASSWORD "your_password"
#define UPLOAD_INTERVAL_HOURS 6
```

### Cellular Connectivity
```cpp
// Remote areas with cellular coverage
#define USE_SIM800L 1
#define APN "your_carrier_apn"
#define CELLULAR_UPLOAD_BATCH_SIZE 10
```

### LoRa Long Range
```cpp
// Very remote areas, long range communication
#define LORA_FREQUENCY 915000000  // US frequency
#define LORA_SPREAD_FACTOR 12     // Maximum range
#define LORA_BANDWIDTH 125000     // Standard bandwidth
```

### Satellite Communication
```cpp
// Extremely remote areas (requires specialized hardware)
#define USE_SATELLITE_MODEM 1
#define SATELLITE_UPLOAD_COMPRESSED 1
#define MAX_DAILY_SATELLITE_USAGE_KB 100
```

## Environmental Considerations

### Weatherproofing
- **IP65 minimum** for outdoor deployments
- **Desiccant packets** to prevent condensation
- **Cable glands** for all external connections
- **UV-resistant materials** for extended sun exposure

### Temperature Management
```cpp
// Temperature compensation for different climates
#ifdef ARCTIC_DEPLOYMENT
    #define MIN_OPERATING_TEMP -40
    #define BATTERY_HEATER_ENABLED 1
#endif

#ifdef DESERT_DEPLOYMENT
    #define MAX_OPERATING_TEMP 60
    #define THERMAL_SHUTDOWN_ENABLED 1
#endif
```

### Wildlife Safety
- **No sharp edges** or small parts that could harm animals
- **Secure mounting** to prevent animals from dislodging
- **Neutral colors** to blend with environment
- **IR flash only** for night photography to avoid disturbing nocturnal animals

## Deployment Checklist

### Pre-Deployment Testing
- [ ] Board detection and configuration verification
- [ ] Camera capture test in various lighting conditions
- [ ] Power consumption measurement
- [ ] Connectivity range testing
- [ ] Weatherproof enclosure test
- [ ] SD card formatting and capacity test

### Installation
- [ ] Site survey for optimal camera placement
- [ ] Solar panel orientation and tilt optimization
- [ ] Secure mounting to prevent theft/damage
- [ ] Camouflage application if required
- [ ] Initial system test after installation
- [ ] GPS coordinates recorded for future maintenance

### Maintenance Schedule
- **Weekly**: Remote status check via connectivity
- **Monthly**: Physical inspection if accessible
- **Quarterly**: Battery and solar panel cleaning
- **Annually**: Complete system service and upgrade

This deployment guide ensures optimal wildlife monitoring system performance across diverse environments and requirements.