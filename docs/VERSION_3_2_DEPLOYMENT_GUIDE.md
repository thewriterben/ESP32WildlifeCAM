# ESP32 Wildlife CAM Version 3.2.0 Deployment Guide

## Overview

Version 3.2.0 transforms the ESP32 Wildlife CAM into a next-generation wildlife monitoring platform with four major new capabilities:

1. **Blockchain-based Data Verification**
2. **Real-time Collaborative Research Platform**
3. **Advanced Audio Detection and Classification**
4. **Federated Learning Across Camera Networks**

## Prerequisites

### Hardware Requirements

- ESP32-CAM or compatible board (ESP32-S3-CAM recommended)
- MicroSD card (16GB minimum, 32GB recommended)
- I2S digital microphone (INMP441 or compatible)
- External antenna for improved connectivity
- Adequate power supply (solar panel + battery recommended)

### Software Dependencies

- PlatformIO Core 6.1.18+
- Arduino ESP32 Core 2.0.11+
- Required libraries (automatically installed via platformio.ini):
  - ArduinoJson 6.21.3+
  - TensorFlow Lite for Microcontrollers 2.4.0+
  - WebSockets library for real-time communication

## Installation Steps

### 1. Clone and Setup

```bash
git clone https://github.com/thewriterben/ESP32WildlifeCAM.git
cd ESP32WildlifeCAM
```

### 2. Configure PlatformIO

```bash
# Install PlatformIO
pip install platformio

# Install dependencies
pio pkg install
```

### 3. Hardware Configuration

#### Basic ESP32-CAM Setup
```cpp
// In config.h, ensure these are set:
#define CAMERA_MODEL_AI_THINKER
#define BOARD_HAS_PSRAM
#define VERSION_3_2_ENABLED true
```

#### Audio Hardware Setup
```cpp
// I2S Microphone Configuration
#define I2S_MIC_SCK_PIN     26  // Serial Clock
#define I2S_MIC_WS_PIN      25  // Word Select
#define I2S_MIC_SD_PIN      22  // Serial Data
#define ENABLE_AUDIO_DETECTION true
```

### 4. Network Configuration

#### WiFi Setup
```cpp
// In wifi_config.h
#define WIFI_SSID "YourNetworkName"
#define WIFI_PASSWORD "YourNetworkPassword"
#define ENABLE_MESH_NETWORKING true
```

#### Blockchain Configuration
```cpp
// Enable blockchain features
#define BLOCKCHAIN_ENABLE_SMART_CONTRACTS true
#define BLOCKCHAIN_RESEARCHER_AUTH true
#define BLOCKCHAIN_ENABLE_PROVENANCE true
```

### 5. Build and Flash

```bash
# Build for ESP32-CAM
pio run -e esp32cam

# Flash to device
pio run -e esp32cam -t upload

# Monitor serial output
pio device monitor
```

## Version 3.2.0 Feature Configuration

### 1. Smart Contract Engine

The smart contract engine validates wildlife data and authenticates researchers:

```cpp
// Initialize in setup()
SmartContractEngine* contractEngine = new SmartContractEngine();
contractEngine->initialize();

// Deploy validation contracts
SmartContractUtils::deployDefaultContracts();
```

### 2. Collaborative Research Platform

Real-time multi-user research platform with WebSocket communication:

```cpp
// Initialize research platform
CollaborativeResearchPlatform* platform = new CollaborativeResearchPlatform();
platform->initialize(81); // WebSocket on port 81

// Access web interface at: http://[device-ip]/research_dashboard.html
```

### 3. Audio Classification System

Enhanced audio detection with species identification:

```cpp
// Initialize audio classifier
WildlifeAudioClassifier* audioClassifier = new WildlifeAudioClassifier();
audioClassifier->initialize(MicrophoneType::I2S_DIGITAL);

// Enable directional capture
audioClassifier->setDirectionalMode(DirectionalCaptureMode::BEAMFORMING);
```

### 4. Federated Learning Coordinator

Distributed machine learning across camera networks:

```cpp
// Initialize federated learning
FederatedLearningCoordinator* coordinator = new FederatedLearningCoordinator();
coordinator->initialize();

// Start species classification training
String roundId = coordinator->startTrainingRound(
    WildlifeModelType::SPECIES_CLASSIFIER,
    FederatedAlgorithm::FEDERATED_AVERAGING
);
```

## Web Interface Access

### Research Dashboard

1. Connect to device WiFi network
2. Open browser to `http://[device-ip]/research_dashboard.html`
3. Log in with researcher credentials
4. Access real-time data, collaboration tools, and analytics

### API Endpoints

- `GET /api/wildlife/detections` - Recent wildlife detections
- `GET /api/blockchain/status` - Blockchain integrity status
- `GET /api/federated/stats` - Federated learning statistics
- `POST /api/research/annotate` - Add researcher annotations
- `WebSocket ws://[device-ip]:81` - Real-time data stream

## Advanced Configuration

### Blockchain Settings

```cpp
// Adjust blockchain parameters in blockchain_config.h
#define BLOCKCHAIN_MAX_BLOCKS_IN_MEMORY 10
#define BLOCKCHAIN_CONTRACT_GAS_LIMIT 15000
#define BLOCKCHAIN_WITNESS_NODES 5
```

### Audio Processing

```cpp
// Configure audio processing algorithms
audioClassifier->setProcessingAlgorithm(AudioProcessingAlgorithm::MFCC);
audioClassifier->setSensitivity(0.8f);
audioClassifier->setNoiseFiltering(true);
```

### Federated Learning

```cpp
// Configure privacy-preserving techniques
coordinator->setPrivacyTechnique(PrivacyTechnique::DIFFERENTIAL_PRIVACY);
coordinator->setPrivacyBudget(1.0f);
```

## Troubleshooting

### Common Issues

1. **Memory Issues**
   - Reduce `BLOCKCHAIN_MAX_BLOCKS_IN_MEMORY`
   - Enable `BLOCKCHAIN_LAZY_LOADING`
   - Use external PSRAM

2. **Network Connectivity**
   - Check WiFi credentials
   - Verify mesh network configuration
   - Monitor signal strength

3. **Audio Detection**
   - Verify I2S microphone connections
   - Adjust sensitivity settings
   - Check for hardware conflicts

4. **Smart Contract Failures**
   - Increase gas limit
   - Validate contract code
   - Check researcher authentication

### Debug Output

Enable verbose logging:

```cpp
#define BLOCKCHAIN_DEBUG_ENABLED true
#define BLOCKCHAIN_VERBOSE_LOGGING true
#define RESEARCH_PLATFORM_DEBUG true
#define FEDERATED_LEARNING_DEBUG true
```

### Performance Monitoring

```cpp
// Get system statistics
String stats = g_researchPlatform->getPlatformStatistics();
String blockchainStats = g_smartContractEngine->getStatistics();
String federatedStats = g_federatedCoordinator->getStatistics();
```

## Security Considerations

### Researcher Authentication

- Enable strong password policies
- Use institutional email verification
- Implement session timeouts
- Enable two-factor authentication (future enhancement)

### Data Protection

- All wildlife observations are cryptographically hashed
- Blockchain provides tamper-evident data storage
- Smart contracts validate data integrity
- Federated learning preserves privacy

### Network Security

- WPA2/WPA3 encryption for WiFi
- TLS encryption for web interface
- Secure WebSocket connections
- Network access control lists

## Maintenance

### Regular Tasks

1. **Monitor System Health**
   - Check blockchain integrity
   - Verify federated learning convergence
   - Monitor audio classification accuracy

2. **Update Management**
   - Regular firmware updates via OTA
   - Smart contract updates
   - ML model updates through federated learning

3. **Data Management**
   - Periodic blockchain verification
   - Audio signature updates
   - Research collaboration cleanup

### Backup Procedures

- Automatic blockchain backup to cloud storage
- Research data synchronization
- Configuration backup and restore

## Support

For technical support and advanced configuration:

- GitHub Issues: https://github.com/thewriterben/ESP32WildlifeCAM/issues
- Documentation: See `/docs` directory
- Community Forum: [Wildlife Tech Community]
- Email: support@wildlifecam.tech

## License

This project is licensed under the MIT License. See LICENSE file for details.

---

**ESP32 Wildlife CAM Version 3.2.0 - Advanced Wildlife Monitoring Platform**
*Transforming wildlife research through blockchain, AI, and collaborative technology*