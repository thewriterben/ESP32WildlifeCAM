# ESP32WildlifeCAM Version 3.1 Documentation

## Overview

ESP32WildlifeCAM v3.1 represents a major evolution of the wildlife monitoring system, introducing enterprise-level features including multi-tenant architecture, advanced ML analytics, React Native mobile applications, and enhanced species classification models supporting 50+ species with regional variants.

## New Features

### 1. React Native Mobile Application

A comprehensive mobile application built with React Native and TypeScript, providing:

- **Real-time Wildlife Monitoring Dashboard**: Live data synchronization with WebSocket connections
- **Camera Management**: Complete camera configuration and control from mobile devices
- **Species Identification Gallery**: AI-powered species identification with confidence scores
- **Push Notifications**: Real-time alerts for wildlife detections and system events
- **Offline Data Caching**: Field researcher support with offline capabilities
- **GPS Integration**: Camera location mapping and geographic analytics
- **Multi-tenant Support**: Organization-based access control and data isolation

#### Technical Stack
- React Native 0.72+ with TypeScript
- Redux Toolkit for state management
- React Native Paper for Material Design UI
- WebSocket integration for real-time data
- AsyncStorage for offline capabilities
- Push notification services (FCM/APNs)

#### Installation
```bash
cd mobile/
npm install
# For iOS
cd ios && pod install && cd ..
npx react-native run-ios

# For Android
npx react-native run-android
```

### 2. Advanced Analytics with Machine Learning Insights

Enhanced analytics engine implementing cutting-edge ecological analysis:

#### Biodiversity Metrics
- **Shannon Diversity Index**: Measures species diversity in community
- **Simpson Diversity Index**: Probability-based diversity measurement
- **Species Richness**: Total number of different species
- **Species Evenness**: Distribution equality among species

#### Activity Pattern Analysis
- **Temporal Analysis**: Hourly, daily, and seasonal activity patterns
- **Behavioral Classification**: Automatic classification of animal behaviors
- **Peak Activity Detection**: Identification of optimal monitoring times
- **Activity Type Classification**: Diurnal, nocturnal, crepuscular patterns

#### Anomaly Detection
- **Temporal Anomalies**: Detection of unusual timing patterns
- **Population Anomalies**: Identification of population changes
- **Behavioral Anomalies**: Detection of unusual animal behaviors
- **System Anomalies**: Camera and equipment malfunction detection

#### Population Trend Forecasting
- **Time Series Analysis**: Statistical trend analysis of species populations
- **Population Forecasting**: ML-based population predictions
- **Confidence Intervals**: Statistical confidence measures for predictions
- **Trend Significance Testing**: Statistical validation of observed trends

#### Conservation Alerts
- **Endangered Species Monitoring**: Automatic alerts for endangered species
- **Population Decline Warnings**: Early warning system for population changes
- **Habitat Change Detection**: Environmental change notifications
- **Research Recommendations**: Automated research priority suggestions

### 3. Multi-tenant Architecture for Organizations

Enterprise-level multi-tenancy with complete data isolation:

#### Organization Management
- **Organization Profiles**: Complete organization setup and configuration
- **Subscription Management**: Tiered subscription plans (Basic, Professional, Enterprise)
- **Billing Integration**: Stripe integration for subscription billing
- **Usage Tracking**: Detailed usage analytics and reporting

#### Role-Based Access Control (RBAC)
- **Admin Role**: Complete system administration capabilities
- **Manager Role**: Organization management and user administration
- **Researcher Role**: Data analysis and research capabilities  
- **Viewer Role**: Read-only access to data and analytics

#### Data Isolation
- **Row Level Security (RLS)**: PostgreSQL-based data isolation
- **Organization Context**: Automatic data filtering by organization
- **API Isolation**: Complete API-level data separation
- **Audit Logging**: Complete activity tracking and audit trails

#### User Management
- **Invitation System**: Email-based user invitations
- **User Onboarding**: Guided setup for new users
- **Profile Management**: Complete user profile customization
- **Two-Factor Authentication**: Enhanced security with 2FA support

### 4. Enhanced Species Classification Models

Dramatically expanded AI capabilities with regional specialization:

#### Extended Species Support
- **50+ Species**: Expanded from 15+ to 50+ supported species
- **Regional Models**: Specialized models for different geographic regions
- **High Accuracy**: Targeting 92%+ classification accuracy
- **Confidence Scoring**: Detailed confidence metrics for each detection

#### Regional Model Variants
- **North America**: 26 species including deer, bear, coyote, elk, moose
- **Europe**: 25 species including red deer, wild boar, brown bear, lynx
- **Asia**: 25 species including tiger, leopard, elephant, red panda
- **Africa**: 29 species including lion, elephant, giraffe, zebra
- **Australia**: 20 species including kangaroo, koala, wombat, dingo

#### Multi-task Learning
- **Species Classification**: Primary species identification task
- **Behavior Recognition**: Secondary behavior classification
- **Conservation Status**: Auxiliary conservation status prediction
- **Confidence Calibration**: Improved confidence score reliability

#### ESP32 Optimization
- **Model Quantization**: INT8 quantization for ESP32 deployment
- **Memory Optimization**: Optimized for ESP32 memory constraints
- **C++ Header Generation**: Automatic embedded code generation
- **Performance Optimization**: Inference time optimization for real-time use

#### Transfer Learning with MobileNetV3
- **Pre-trained Backbone**: MobileNetV3 architecture for efficiency
- **Fine-tuning**: Task-specific fine-tuning for wildlife detection
- **Architecture Variants**: Large and Small variants for different use cases
- **Optimization Strategies**: Multiple optimization approaches for deployment

## Architecture Overview

### System Components

```
ESP32WildlifeCAM v3.1 Architecture

┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  Mobile App     │    │   Web Frontend  │    │   ESP32 Devices │
│  (React Native) │    │    (React)      │    │   (Firmware)    │
└─────────┬───────┘    └─────────┬───────┘    └─────────┬───────┘
          │                      │                      │
          └──────────────────────┼──────────────────────┘
                                 │
                    ┌─────────────┴───────────┐
                    │     Load Balancer      │
                    │       (Nginx)          │
                    └─────────────┬───────────┘
                                 │
          ┌──────────────────────┼──────────────────────┐
          │                      │                      │
    ┌─────┴─────┐    ┌───────────┴──────────┐    ┌─────┴─────┐
    │WebSocket  │    │  Backend API v3.1    │    │    ML     │
    │ Server    │    │     (Flask)          │    │ Services  │
    └───────────┘    └───────────┬──────────┘    └───────────┘
                                 │
              ┌──────────────────┼──────────────────┐
              │                  │                  │
        ┌─────┴─────┐    ┌───────┴────────┐    ┌───┴────────┐
        │PostgreSQL │    │     Redis      │    │ Analytics  │
        │    DB     │    │    Cache       │    │  Engine    │
        └───────────┘    └────────────────┘    └────────────┘
```

### Database Schema

The v3.1 database schema includes:

- **Organizations**: Multi-tenant organization management
- **Enhanced Users**: Extended user profiles with RBAC
- **Enhanced Cameras**: Advanced camera management with health monitoring
- **Enhanced Species**: Detailed species information with conservation data
- **Enhanced Detection**: Multi-task detection results
- **Analytics**: Pre-computed analytics and insights
- **Subscriptions**: Billing and subscription management
- **Audit Logs**: Complete activity tracking

### API Endpoints

#### Authentication & Organizations
```
POST   /api/v3.1/auth/login
POST   /api/v3.1/auth/register  
POST   /api/v3.1/auth/refresh
GET    /api/v3.1/organizations
POST   /api/v3.1/organizations
PUT    /api/v3.1/organizations/{id}
POST   /api/v3.1/organizations/{id}/invite
```

#### Enhanced Analytics
```
GET    /api/v3.1/analytics/dashboard
GET    /api/v3.1/analytics/biodiversity
GET    /api/v3.1/analytics/activity-patterns
GET    /api/v3.1/analytics/population-trends
GET    /api/v3.1/analytics/anomalies
GET    /api/v3.1/analytics/conservation-alerts
POST   /api/v3.1/analytics/export
```

#### Mobile-Optimized Endpoints
```
GET    /api/v3.1/mobile/dashboard
GET    /api/v3.1/mobile/cameras/nearby
POST   /api/v3.1/mobile/cameras/{id}/capture
GET    /api/v3.1/mobile/species/identify
POST   /api/v3.1/mobile/offline/sync
```

## Deployment

### Production Deployment

1. **Environment Setup**
```bash
# Copy environment template
cp .env.example .env.production

# Configure production variables
vim .env.production
```

2. **Deploy with Docker Compose**
```bash
# Deploy full production stack
docker-compose -f docker-compose.v3.1.yml --profile production up -d

# Deploy with monitoring
docker-compose -f docker-compose.v3.1.yml --profile production --profile monitoring up -d
```

3. **Initialize Database**
```bash
# Run database migrations
docker-compose -f docker-compose.v3.1.yml exec backend_v31 flask db upgrade

# Load initial data
docker-compose -f docker-compose.v3.1.yml exec backend_v31 python scripts/load_initial_data.py
```

### Development Setup

1. **Local Development**
```bash
# Start development services
docker-compose -f docker-compose.v3.1.yml --profile development up -d

# Install mobile app dependencies
cd mobile/ && npm install

# Start mobile development
npm run start
```

### Scaling and High Availability

#### Horizontal Scaling
- **Backend Services**: Multiple backend instances behind load balancer
- **Worker Scaling**: Celery workers can be scaled independently
- **Database Scaling**: Read replicas for analytics workloads
- **Caching**: Redis cluster for high availability

#### Performance Optimization
- **Database Indexing**: Optimized indexes for multi-tenant queries
- **API Caching**: Redis-based API response caching
- **CDN Integration**: Static asset delivery optimization
- **Connection Pooling**: Optimized database connection management

## Migration from v3.0

### Database Migration
```bash
# Backup existing database
pg_dump wildlife_camera > backup_v30.sql

# Run migration scripts
python scripts/migrate_v30_to_v31.py

# Verify migration
python scripts/verify_migration.py
```

### Configuration Updates
- Update environment variables for multi-tenant support
- Configure organization settings
- Update ESP32 firmware to support new model formats
- Configure mobile app push notification services

## Security Considerations

### Multi-tenant Security
- **Row Level Security (RLS)**: Database-level data isolation
- **API Authentication**: JWT-based authentication with organization context
- **Input Validation**: Comprehensive input sanitization
- **Rate Limiting**: API rate limiting per organization

### Data Privacy
- **Data Encryption**: Encryption at rest and in transit
- **Audit Logging**: Complete activity tracking
- **GDPR Compliance**: Data privacy regulation compliance
- **Access Controls**: Fine-grained permission system

## Monitoring and Observability

### Metrics Collection
- **Application Metrics**: Custom business metrics
- **System Metrics**: Infrastructure monitoring
- **ML Model Metrics**: Model performance tracking
- **User Activity Metrics**: Usage analytics

### Alerting
- **System Alerts**: Infrastructure issues
- **Business Alerts**: Conservation and wildlife alerts
- **Performance Alerts**: Application performance issues
- **Security Alerts**: Security incident notifications

## Support and Maintenance

### Backup Strategy
- **Database Backups**: Automated daily backups
- **File Backups**: Media and model file backups
- **Configuration Backups**: System configuration backups
- **Recovery Testing**: Regular recovery procedure testing

### Update Procedures
- **Rolling Updates**: Zero-downtime deployment procedures
- **Database Migrations**: Safe migration procedures
- **Model Updates**: ML model update procedures
- **Mobile App Updates**: App store deployment procedures

## Contributing

See [CONTRIBUTING.md](../../CONTRIBUTING.md) for development guidelines and contribution procedures.

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](../../LICENSE) file for details.