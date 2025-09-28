# ESP32 Wildlife Camera - Complete Setup Guide

This comprehensive guide will walk you through setting up the complete ESP32 Wildlife Camera ecosystem from scratch.

## Table of Contents

1. [Overview](#overview)
2. [System Requirements](#system-requirements)
3. [Quick Start](#quick-start)
4. [Detailed Installation](#detailed-installation)
5. [Hardware Setup](#hardware-setup)
6. [Software Configuration](#software-configuration)
7. [Development Environment](#development-environment)
8. [Production Deployment](#production-deployment)
9. [Troubleshooting](#troubleshooting)

## Overview

The ESP32 Wildlife Camera ecosystem consists of:

- **ESP32 Firmware**: C/C++ firmware for camera operation and AI processing
- **Backend API**: Python Flask server for data management and processing
- **Web Dashboard**: React frontend for monitoring and management
- **Machine Learning Pipeline**: Python scripts for model training and optimization
- **Documentation**: Comprehensive guides and API documentation

## System Requirements

### Hardware Requirements

#### ESP32 Camera Module (Choose one)
- **AI-Thinker ESP32-CAM** (Recommended for beginners)
- **ESP32-S3-CAM** (Enhanced AI capabilities)
- **TTGO T-Camera** (Built-in display)
- **ESP32-C3-CAM** (Ultra-low power)
- **ESP32-C6-CAM** (WiFi 6 support)

#### Additional Components
- MicroSD card (32GB+ Class 10)
- PIR motion sensor (HC-SR501)
- Solar panel (5W minimum) + LiPo battery (3000mAh+)
- Weatherproof enclosure
- LoRa module (optional, for mesh networking)

#### Development Hardware
- USB cable for programming
- Breadboard and jumper wires
- 3D printer (optional, for custom enclosures)

### Software Requirements

#### Development Environment
- **Operating System**: Windows 10+, macOS 10.15+, or Ubuntu 18.04+
- **Python**: 3.8 or higher
- **Node.js**: 16.0 or higher
- **Git**: Latest version

#### Optional but Recommended
- **Docker**: For containerized deployment
- **PlatformIO IDE**: For ESP32 development
- **Visual Studio Code**: With PlatformIO extension

## Quick Start

### Automated Setup

The fastest way to get started is using our automated setup script:

```bash
# Clone the repository
git clone https://github.com/thewriterben/ESP32WildlifeCAM.git
cd ESP32WildlifeCAM

# Run automated setup
chmod +x tools/setup.sh
./tools/setup.sh
```

This script will:
- Check system requirements
- Install necessary dependencies
- Set up Python and Node.js environments
- Create configuration files
- Download required models

### Manual Quick Start

If you prefer manual setup:

```bash
# 1. Install Python dependencies
python3 -m venv venv
source venv/bin/activate  # Linux/Mac
# venv\Scripts\activate   # Windows
pip install -r backend/requirements.txt

# 2. Install Node.js dependencies
cd frontend
npm install
cd ..

# 3. Install PlatformIO
python3 -m pip install platformio

# 4. Build firmware
platformio run

# 5. Start services
# Terminal 1: Backend
cd backend
python app.py

# Terminal 2: Frontend
cd frontend
npm start
```

## Detailed Installation

### 1. Repository Setup

```bash
# Clone the repository
git clone https://github.com/thewriterben/ESP32WildlifeCAM.git
cd ESP32WildlifeCAM

# Create necessary directories
mkdir -p uploads/thumbnails uploads/processed
mkdir -p models
mkdir -p datasets
```

### 2. Backend Setup

#### Python Environment

```bash
# Create virtual environment
python3 -m venv venv

# Activate virtual environment
source venv/bin/activate  # Linux/Mac
# venv\Scripts\activate   # Windows

# Upgrade pip
pip install --upgrade pip

# Install dependencies
pip install -r backend/requirements.txt
```

#### Database Setup

For development, you can use Docker:

```bash
# Start PostgreSQL and Redis
docker-compose up -d db redis

# Wait for services to start
sleep 10

# Initialize database
cd backend
flask db init
flask db migrate -m "Initial migration"
flask db upgrade
cd ..
```

For production PostgreSQL installation, see [Database Setup Guide](DATABASE_SETUP.md).

#### Environment Configuration

Create `backend/.env`:

```env
# Database
DATABASE_URL=postgresql://wildlife:wildlife123@localhost:5432/wildlife_camera

# Security
JWT_SECRET_KEY=your-super-secret-key-change-in-production

# File Storage
UPLOAD_FOLDER=./uploads
MAX_CONTENT_LENGTH=52428800

# Celery
CELERY_BROKER_URL=redis://localhost:6379/0
CELERY_RESULT_BACKEND=redis://localhost:6379/0

# AI Models
MEGADETECTOR_MODEL_PATH=./models/megadetector_v5a.pb
MEGADETECTOR_CONFIDENCE_THRESHOLD=0.2

# Development
DEBUG=true
FLASK_ENV=development
```

### 3. Frontend Setup

```bash
cd frontend

# Install dependencies
npm install

# Create environment file
cat > .env << EOF
REACT_APP_API_URL=http://localhost:5000/api
REACT_APP_WEBSOCKET_URL=ws://localhost:5000
GENERATE_SOURCEMAP=false
EOF

cd ..
```

### 4. Firmware Setup

#### PlatformIO Installation

```bash
# Install PlatformIO Core
python3 -m pip install platformio

# Add to PATH (if needed)
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

#### Firmware Configuration

Edit `firmware/include/config.h` for your specific setup:

```cpp
// WiFi Configuration
#define WIFI_SSID "YourWiFiNetwork"
#define WIFI_PASSWORD "YourWiFiPassword"

// Camera Configuration
#define CAMERA_MODEL_AI_THINKER  // Change based on your board

// API Configuration
#define API_SERVER_URL "http://192.168.1.100:5000/api"
#define DEVICE_ID "wildlife_cam_001"

// Feature Configuration
#define ENABLE_AI_DETECTION true
#define ENABLE_LORA_MESH false
#define ENABLE_SOLAR_MANAGEMENT true
```

#### Build and Upload

```bash
# Build firmware
platformio run

# Upload to ESP32 (connect via USB)
platformio run --target upload

# Monitor serial output
platformio device monitor
```

### 5. Machine Learning Setup

#### Model Training Environment

```bash
# Install ML dependencies
pip install tensorflow scikit-learn opencv-python-headless

# Download sample datasets (optional)
mkdir -p datasets/wildlife/train
mkdir -p datasets/wildlife/validation
mkdir -p datasets/wildlife/test
```

#### MegaDetector Integration

```bash
# Download MegaDetector model
mkdir -p models
cd models

# Download MegaDetector v5a
curl -L "https://github.com/microsoft/CameraTraps/releases/download/v5.0/md_v5a.0.0.pt" -o megadetector_v5a.pt

cd ..
```

## Hardware Setup

### ESP32 Camera Wiring

#### AI-Thinker ESP32-CAM

```
ESP32-CAM Pin Connections:
- VCC -> 5V
- GND -> GND
- U0R -> FTDI RX (for programming)
- U0T -> FTDI TX (for programming)
- GPIO0 -> GND (for programming mode)
- RST -> FTDI RST (optional)

PIR Sensor Connections:
- VCC -> 3.3V
- GND -> GND
- OUT -> GPIO13
```

#### Power Management

```
Solar Panel -> Charge Controller -> LiPo Battery -> ESP32
                                              -> Load Switch -> External Components

Voltage Monitoring:
- Battery voltage -> GPIO35 (through voltage divider)
- Solar voltage -> GPIO34 (through voltage divider)
```

### Enclosure Assembly

1. **3D Print Enclosure**: Use files in `hardware/stl_files/`
2. **Weatherproofing**: Apply silicone sealant to joints
3. **Camera Positioning**: Ensure clear view, no obstructions
4. **Ventilation**: Include breather vents to prevent condensation
5. **Mounting**: Use appropriate brackets for installation

## Software Configuration

### Backend Configuration

#### Species Database

Initialize with sample species:

```bash
cd backend
python -c "
from app import app
from models import db, Species
import json

with app.app_context():
    # Load sample species data
    with open('sample_species.json', 'r') as f:
        species_data = json.load(f)
    
    for species_info in species_data:
        species = Species(**species_info)
        db.session.add(species)
    
    db.session.commit()
    print('Sample species data loaded')
"
```

#### API Key Configuration

For production, configure API keys:

```env
# Cloud Storage
AWS_ACCESS_KEY_ID=your_aws_access_key
AWS_SECRET_ACCESS_KEY=your_aws_secret_key
AWS_S3_BUCKET=wildlife-camera-images

# Notification Services
PUSHOVER_TOKEN=your_pushover_token
PUSHOVER_USER=your_pushover_user

# External APIs
INATURALIST_API_KEY=your_inaturalist_key
WILDLIFE_INSIGHTS_KEY=your_wildlife_insights_key
```

### Frontend Configuration

#### Map Integration

Add Mapbox token for map features:

```env
REACT_APP_MAPBOX_TOKEN=your_mapbox_token
```

#### Real-time Features

Configure WebSocket for real-time updates:

```env
REACT_APP_WEBSOCKET_URL=ws://your-domain.com
REACT_APP_ENABLE_REALTIME=true
```

## Development Environment

### Running Development Servers

#### Backend Development

```bash
# Activate virtual environment
source venv/bin/activate

# Start Flask development server
cd backend
export FLASK_ENV=development
export FLASK_DEBUG=1
python app.py

# In another terminal, start Celery worker
celery -A app.celery worker --loglevel=info
```

#### Frontend Development

```bash
cd frontend
npm start
```

This starts the React development server with hot reload.

#### Firmware Development

```bash
# Build and upload
platformio run --target upload

# Monitor serial output
platformio device monitor --baud 115200

# For debugging with JTAG
platformio debug
```

### Development Tools

#### Code Formatting

```bash
# Python code formatting
black backend/
isort backend/

# JavaScript code formatting
cd frontend
npm run format
```

#### Testing

```bash
# Backend tests
cd backend
pytest

# Frontend tests
cd frontend
npm test
```

## Production Deployment

### Docker Deployment

#### Single Server Deployment

```bash
# Build and start all services
docker-compose --profile production up -d

# Check service status
docker-compose ps

# View logs
docker-compose logs -f
```

#### Kubernetes Deployment

```bash
# Deploy to Kubernetes cluster
kubectl apply -f kubernetes/

# Check deployment status
kubectl get pods
kubectl get services
```

### Database Migration

For production database:

```bash
# Backup existing database
pg_dump wildlife_camera > backup_$(date +%Y%m%d).sql

# Run migrations
cd backend
flask db upgrade

# Verify migration
flask db current
```

### SSL Configuration

#### Let's Encrypt Setup

```bash
# Install Certbot
sudo apt-get install certbot python3-certbot-nginx

# Obtain SSL certificate
sudo certbot --nginx -d your-domain.com

# Auto-renewal
sudo crontab -e
# Add: 0 12 * * * /usr/bin/certbot renew --quiet
```

### Monitoring Setup

#### Prometheus and Grafana

```bash
# Start monitoring stack
docker-compose --profile monitoring up -d

# Access Grafana
# http://your-domain:3001
# Default: admin/admin
```

## Troubleshooting

### Common Issues

#### ESP32 Connection Issues

```bash
# Check serial port
ls /dev/ttyUSB*  # Linux
ls /dev/cu.*     # macOS

# Flash ESP32 with specific settings
platformio run --target upload --upload-port /dev/ttyUSB0
```

#### Database Connection Issues

```bash
# Check database status
docker-compose ps db

# Connect to database directly
docker-compose exec db psql -U wildlife -d wildlife_camera

# Reset database
docker-compose down -v
docker-compose up -d db
```

#### Frontend Build Issues

```bash
# Clear node modules and reinstall
cd frontend
rm -rf node_modules package-lock.json
npm install

# Clear npm cache
npm cache clean --force
```

#### Backend Import Issues

```bash
# Check Python environment
which python
pip list

# Reinstall dependencies
pip install --force-reinstall -r requirements.txt
```

### Performance Optimization

#### Backend Optimization

```python
# Enable query optimization
SQLALCHEMY_ECHO = False
SQLALCHEMY_TRACK_MODIFICATIONS = False

# Configure connection pooling
SQLALCHEMY_ENGINE_OPTIONS = {
    'pool_size': 10,
    'pool_recycle': 120,
    'pool_pre_ping': True
}
```

#### Frontend Optimization

```bash
# Build optimized production bundle
npm run build

# Analyze bundle size
npm install -g webpack-bundle-analyzer
npx webpack-bundle-analyzer build/static/js/*.js
```

#### Firmware Optimization

```cpp
// Enable compiler optimizations
build_flags = -O2 -DNDEBUG

// Reduce memory usage
#define CONFIG_SPIRAM_CACHE_WORKAROUND
#define CONFIG_SPIRAM_BANKSWITCH_ENABLE
```

### Getting Help

- **Documentation**: [docs/README.md](README.md)
- **API Reference**: [docs/API_DOCS.md](API_DOCS.md)
- **Hardware Guide**: [docs/HARDWARE_GUIDE.md](HARDWARE_GUIDE.md)
- **Issues**: [GitHub Issues](https://github.com/thewriterben/ESP32WildlifeCAM/issues)
- **Discussions**: [GitHub Discussions](https://github.com/thewriterben/ESP32WildlifeCAM/discussions)

---

## Next Steps

After completing the setup:

1. **Test the system** with sample images
2. **Configure your first camera** deployment
3. **Set up monitoring** and alerts
4. **Customize species detection** for your region
5. **Integrate with research platforms** like Wildlife Insights

For advanced features and customization, see the [Advanced Configuration Guide](ADVANCED_CONFIG.md).