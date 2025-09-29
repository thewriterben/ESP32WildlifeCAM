# ESP32 Wildlife Camera Backend API

## Overview

Complete Flask-based backend infrastructure for the ESP32 Wildlife Camera project, providing RESTful API endpoints, real-time WebSocket communication, and wildlife detection processing.

## Features

### Core Functionality
- **User Authentication**: JWT-based authentication system
- **Camera Management**: Device registration and monitoring
- **Image Processing**: Upload, storage, and wildlife detection
- **Species Classification**: Integration with Microsoft MegaDetector
- **Analytics Engine**: Biodiversity metrics and reporting
- **Real-time Updates**: WebSocket integration for live notifications

### API Endpoints

#### Authentication
- `POST /api/auth/register` - User registration
- `POST /api/auth/login` - User authentication

#### Camera Management
- `GET /api/cameras` - List user's cameras
- `POST /api/cameras` - Register new camera
- `GET /api/cameras/{id}` - Get camera details
- `PUT /api/cameras/{id}` - Update camera configuration
- `DELETE /api/cameras/{id}` - Delete camera

#### Image Management
- `GET /api/images` - List camera images (with pagination)
- `POST /api/images` - Upload camera image
- `GET /api/images/{id}` - Get image details
- `GET /api/images/{id}/file` - Download image file

#### Wildlife Detection
- `GET /api/detections` - List wildlife detections (with filtering)
- `GET /api/detections/{id}` - Get detection details
- `PUT /api/detections/{id}/verify` - Verify/correct detection

#### Species Management
- `GET /api/species` - List all species
- `GET /api/species/{id}` - Get species details

#### Analytics
- `GET /api/analytics/dashboard` - Dashboard analytics
- `GET /api/analytics/species/{id}` - Species-specific analytics
- `GET /api/analytics/cameras/{id}/performance` - Camera performance
- `GET /api/analytics/trends` - Detection trends

#### System
- `GET /api/health` - Health check endpoint
- `GET /api/docs` - API documentation

### WebSocket Events
- `connect/disconnect` - Connection management
- `join_camera_room/leave_camera_room` - Camera-specific updates
- `camera_registered` - New camera notifications
- `image_uploaded` - Image upload notifications
- `detection_complete` - Wildlife detection completed

## Quick Start

### Prerequisites
- Python 3.8+
- PostgreSQL database
- Redis server (for Celery and rate limiting)

### Installation

1. **Install dependencies:**
```bash
pip install -r requirements.txt
```

2. **Configure environment:**
```bash
cp .env.example .env
# Edit .env with your configuration
```

3. **Initialize database:**
```bash
flask db upgrade
```

4. **Start the application:**

**Development:**
```bash
python app.py
```

**Production:**
```bash
gunicorn --bind 0.0.0.0:5000 --workers 4 app:app
```

**With Docker:**
```bash
docker build -t wildlife-camera-backend .
docker run -p 5000:5000 wildlife-camera-backend
```

### Background Tasks

Start Celery worker for image processing:
```bash
celery -A app.celery worker --loglevel=info
```

## Configuration

Key environment variables (see `.env.example` for complete list):

- `DATABASE_URL` - PostgreSQL connection string
- `JWT_SECRET_KEY` - JWT token secret
- `CELERY_BROKER_URL` - Redis URL for Celery
- `MEGADETECTOR_MODEL_PATH` - Path to MegaDetector model
- `UPLOAD_FOLDER` - Image storage directory

## Security Features

- JWT token authentication
- Rate limiting (configurable per endpoint)
- Input validation
- CORS configuration
- Secure file upload handling
- SQL injection protection

## Integration

### ESP32 Firmware
The API is designed to work with ESP32 cameras:
- Image upload endpoint accepts multipart/form-data
- Camera registration with device_id
- Status updates and battery monitoring

### Frontend
RESTful API with JSON responses:
- Comprehensive error handling
- Pagination support
- Real-time WebSocket updates

## Architecture

```
┌─────────────────┐    ┌──────────────┐    ┌─────────────────┐
│   ESP32 Camera  │────│  Flask API   │────│   PostgreSQL    │
└─────────────────┘    └──────────────┘    └─────────────────┘
                              │
                       ┌──────────────┐
                       │    Redis     │
                       │ (Celery +    │
                       │Rate Limiting)│
                       └──────────────┘
                              │
                       ┌──────────────┐
                       │   Celery     │
                       │  Worker      │
                       └──────────────┘
```

## Model Schema

### Key Models
- **User**: Authentication and authorization
- **Camera**: Wildlife camera device management
- **CameraImage**: Image metadata and storage
- **WildlifeDetection**: AI detection results
- **Species**: Wildlife species classification
- **Analytics**: Metrics and reporting data

## Development

### Code Structure
```
backend/
├── app.py                 # Main Flask application
├── models.py              # Database models
├── wildlife_detection.py  # AI detection engine
├── analytics_engine.py    # Analytics and reporting
├── image_processor.py     # Image processing utilities
├── requirements.txt       # Python dependencies
├── Dockerfile            # Container configuration
├── .env.example          # Environment template
└── README.md             # This file
```

### Testing
```bash
# Syntax validation
python -m py_compile *.py

# Run with debug mode
DEBUG=true python app.py
```

## License

This project is part of the ESP32WildlifeCAM open-source wildlife monitoring system.