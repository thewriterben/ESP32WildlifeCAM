# Migration Guide - Upgrading to ESP32WildlifeCAM v3.1

*Last Updated: December 1, 2025*  
*Target Version: 3.1.0*  
*Migration Difficulty: Medium*  
*Estimated Time: 2-4 hours*

## 🎯 Overview

This guide provides comprehensive instructions for upgrading from ESP32WildlifeCAM v2.5.x to v3.1.0. Version 3.1 introduces major new features including enhanced AI pipeline, federated learning, mobile applications, and professional analytics.

## 📋 Pre-Migration Assessment

### Current Version Identification

Check your current installation:

```bash
# Check version in your config files
grep -r "VERSION.*2\.5" . | head -5

# Check implemented features
ls firmware/src/ | grep -E "(ai|federated|mobile|analytics)"
```

### Migration Complexity by Current Version

| Current Version | Migration Complexity | Estimated Time | Key Changes |
|----------------|---------------------|----------------|-------------|
| v2.5.x (Recent) | Medium | 2-3 hours | New features, API updates |
| v2.0-2.4 (Intermediate) | High | 3-4 hours | Configuration changes, new dependencies |
| v1.x (Legacy) | Very High | 4-6 hours+ | Complete system overhaul |

## 🔧 Step-by-Step Migration Process

### Phase 1: Environment Preparation (45 minutes)

1. **Backup Current Installation:**
```bash
# Create complete backup
cp -r esp32wildcam_current esp32wildcam_v25_backup
cd esp32wildcam_v25_backup
git tag backup-v25-$(date +%Y%m%d)
```

2. **Update Development Environment:**
```bash
# Update PlatformIO
pio upgrade
pio platform update espressif32

# Install new dependencies for v3.1
pio lib install "AsyncTCP@^1.1.1"
pio lib install "ArduinoJson@^6.21.3"
pip install fastapi uvicorn redis
npm install -g @react-native-community/cli
```

3. **Download v3.1 Release:**
```bash
git clone https://github.com/thewriterben/ESP32WildlifeCAM.git esp32wildcam_v31
cd esp32wildcam_v31
git checkout v3.1-release
```

### Phase 2: Configuration Migration (60 minutes)

1. **Update Version Configuration:**
```c
// Update in config.h files
#define VERSION "3.1.0"
#define VERSION_MAJOR 3
#define VERSION_MINOR 1
#define VERSION_PATCH 0
```

2. **New Configuration Options for v3.1:**
```c
// Add to config.h
// Mobile app integration
#define ENABLE_MOBILE_API 1
#define MOBILE_API_PORT 8080

// Federated learning
#define ENABLE_FEDERATED_LEARNING 1
#define FL_UPDATE_INTERVAL_HOURS 24

// Enhanced analytics
#define ENABLE_PROFESSIONAL_ANALYTICS 1
#define ANALYTICS_BUFFER_SIZE 1024

// Multi-tenant support
#define ENABLE_MULTI_TENANT 1
#define MAX_ORGANIZATIONS 100
```

3. **Database Migration (New in v3.1):**
```bash
# Set up PostgreSQL database
createdb wildlife_cam_v31
psql wildlife_cam_v31 < schema/v31_initial.sql
psql wildlife_cam_v31 < schema/v31_migration_from_v25.sql
```

4. **Redis Configuration (New in v3.1):**
```bash
# Install and configure Redis for caching
sudo apt-get install redis-server
redis-server --daemonize yes
```

### Phase 3: Code Integration (90 minutes)

1. **AI Pipeline Upgrade:**
```cpp
// Update AI model loading in main.cpp
#include "ai/enhanced_species_classifier.h"
#include "ai/federated_learning_client.h"

// Initialize enhanced AI pipeline
EnhancedSpeciesClassifier classifier;
FederatedLearningClient fl_client;
```

2. **Mobile API Integration:**
```cpp
// Add mobile API endpoints
#include "api/mobile_endpoints.h"
#include "api/real_time_streaming.h"

// Initialize mobile API server
MobileAPIServer mobile_api(MOBILE_API_PORT);
mobile_api.start();
```

3. **Analytics System Setup:**
```cpp
// Add professional analytics
#include "analytics/behavior_analyzer.h"
#include "analytics/report_generator.h"

BehaviorAnalyzer analyzer;
ReportGenerator reporter;
```

### Phase 4: Mobile App Setup (60 minutes)

1. **React Native Application:**
```bash
# Navigate to mobile app directory
cd mobile_app/WildlifeCamApp

# Install dependencies
npm install
cd ios && pod install && cd ..

# Configure for your backend
cp config.example.js config.js
# Edit config.js with your camera IP addresses and API endpoints
```

2. **Build Mobile App:**
```bash
# For Android
npx react-native run-android

# For iOS
npx react-native run-ios
```

### Phase 5: Testing & Validation (60 minutes)

1. **Run Migration Validation:**
```bash
# Execute v3.1 validation script
python3 validate_v31_migration.py

# Test new features
python3 test_federated_learning.py
python3 test_mobile_api.py
python3 test_analytics.py
```

2. **Hardware Validation:**
```bash
# Test enhanced AI performance
./test_enhanced_ai.sh

# Validate power optimization
./test_30day_battery.sh

# Test LoRa mesh networking
./test_lora_mesh.sh
```

## 💾 Data Migration Strategy

### Camera Configuration Migration
```bash
# Migrate existing camera configurations
python3 scripts/migrate_camera_configs.py \
  --source backup/v25_configs \
  --target configs/v31 \
  --format v31
```

### Image Data Migration
```bash
# Migrate existing images with new metadata
python3 scripts/migrate_image_data.py \
  --source /sdcard/WILDLIFE/IMAGES \
  --target /sdcard/WILDLIFE_V31/IMAGES \
  --add-ai-metadata
```

### Feature Configuration Mapping

| v2.5 Feature | v3.1 Equivalent | Migration Action |
|-------------|-----------------|------------------|
| Basic AI Detection | Enhanced AI Pipeline | Automatic upgrade |
| WiFi Management | Enhanced Connectivity | Configuration merge |
| Simple Analytics | Professional Analytics | Data structure upgrade |
| Local Storage | Multi-tier Storage | Automatic migration |

## 🆕 New Features in v3.1

### Enhanced AI Pipeline
- **95%+ Species Accuracy** - Improved classification models
- **25+ Species Support** - Expanded recognition capabilities
- **Real-time Processing** - Edge AI optimization

### Federated Learning System
- **Distributed Training** - Collaborative model improvement
- **Privacy-preserving** - Local data stays local
- **Automatic Updates** - Seamless model updates

### Mobile Application
- **Cross-platform** - iOS and Android support
- **Real-time Monitoring** - Live camera feeds
- **Remote Configuration** - Update settings remotely

### Professional Analytics
- **Behavior Analysis** - Advanced pattern recognition
- **Custom Reports** - Automated report generation
- **Multi-site Analytics** - Cross-deployment insights

## ⚠️ Breaking Changes

### API Changes
- **New Endpoints** - Mobile API endpoints added
- **Authentication** - Enhanced security requirements
- **Data Format** - JSON schema updates

### Configuration Changes
- **New Parameters** - Additional configuration options
- **File Structure** - Reorganized configuration files
- **Database Schema** - PostgreSQL integration required

### Dependencies
- **New Libraries** - React Native, FastAPI, Redis
- **Python Packages** - Additional ML libraries
- **System Requirements** - Increased memory requirements

## 🔧 Troubleshooting

### Common Issues

**1. Build Errors:**
```bash
# Clear build cache
pio run --target clean
rm -rf .pio/build/

# Rebuild with verbose output
pio run --verbose
```

**2. Database Connection Issues:**
```bash
# Check PostgreSQL status
sudo systemctl status postgresql

# Test database connection
psql -h localhost -U wildlife_user -d wildlife_cam_v31
```

**3. Mobile App Issues:**
```bash
# Clear React Native cache
npx react-native start --reset-cache

# Check device connectivity
adb devices  # For Android
xcrun simctl list devices  # For iOS
```

### Performance Optimization

**1. Memory Management:**
```c
// Update heap allocation for v3.1
#define HEAP_SIZE_KB 512  // Increased from 256KB
#define AI_MODEL_BUFFER_SIZE 2048  // Increased for enhanced models
```

**2. Network Optimization:**
```c
// Optimize for mobile connectivity
#define MOBILE_API_TIMEOUT_MS 5000
#define WEBSOCKET_KEEPALIVE_INTERVAL 30
```

## 📚 Post-Migration Resources

### Updated Documentation
- [Version 3.1 Feature Guide](docs/v3.1/README.md)
- [Enhanced AI Pipeline Guide](docs/v3.1/ai_pipeline.md)
- [Mobile App User Guide](docs/v3.1/mobile_app.md)
- [Federated Learning Guide](docs/v3.1/federated_learning.md)
- [Professional Analytics Guide](docs/v3.1/analytics.md)

### Testing Resources
- [v3.1 Test Suite](tests/v31_test_suite.py)
- [Performance Benchmarks](tests/v31_benchmarks.py)
- [Integration Tests](tests/v31_integration_tests.py)

### Support Channels
- **GitHub Issues** - Report migration-specific problems
- **Documentation** - Comprehensive guides for new features
- **Community Forums** - User experiences and troubleshooting
- **Mobile App Support** - Dedicated mobile app assistance

### Continuous Updates
- Monitor releases for v3.1.x patches and improvements
- Subscribe to release notifications for security updates
- Participate in community testing for future versions

## 🎉 Post-Migration Verification

### Success Checklist
- [ ] All version numbers updated to 3.1.0
- [ ] Enhanced AI pipeline functioning (95%+ accuracy)
- [ ] Mobile app connected and operational
- [ ] Federated learning system active
- [ ] Professional analytics generating reports
- [ ] Database migration completed successfully
- [ ] All hardware tests passing
- [ ] Network connectivity stable
- [ ] Power optimization active (30+ day battery life)
- [ ] LoRa mesh networking operational

### Performance Validation
```bash
# Run comprehensive v3.1 validation
python3 validate_v31_complete.py --comprehensive

# Generate migration report
python3 generate_migration_report.py --version 3.1.0
```

---

*This migration guide ensures a smooth transition to ESP32WildlifeCAM v3.1 while preserving your existing configuration and data. Take time to test each phase thoroughly before proceeding to the next. For complex deployments, consider staging the migration in a test environment first.*

**📞 Support**: For migration assistance, create an issue with the label `migration-v3.1` on the project GitHub repository.