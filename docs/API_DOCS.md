# ESP32 Wildlife Camera API Documentation

Complete API documentation for the ESP32 Wildlife Camera backend server.

## Table of Contents

1. [Overview](#overview)
2. [Authentication](#authentication)
3. [Base URLs](#base-urls)
4. [Response Format](#response-format)
5. [Error Handling](#error-handling)
6. [Rate Limiting](#rate-limiting)
7. [API Endpoints](#api-endpoints)
8. [WebSocket Events](#websocket-events)
9. [Examples](#examples)

## Overview

The ESP32 Wildlife Camera API is a RESTful web service that provides access to:

- Camera management and registration
- Image upload and processing
- Wildlife detection and classification
- Analytics and reporting
- User management and authentication
- Real-time monitoring

## Authentication

The API uses JWT (JSON Web Token) based authentication.

### Getting an Access Token

```http
POST /api/auth/login
Content-Type: application/json

{
  "username": "your_username",
  "password": "your_password"
}
```

**Response:**
```json
{
  "message": "Login successful",
  "access_token": "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9...",
  "user": {
    "id": 1,
    "username": "your_username",
    "email": "user@example.com",
    "role": "user"
  }
}
```

### Using the Token

Include the token in the Authorization header:

```http
Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9...
```

## Base URLs

- **Development**: `http://localhost:5000/api`
- **Production**: `https://your-domain.com/api`

## Response Format

All API responses follow a consistent format:

### Success Response
```json
{
  "success": true,
  "data": {
    // Response data
  },
  "message": "Operation completed successfully"
}
```

### Error Response
```json
{
  "success": false,
  "error": {
    "code": "ERROR_CODE",
    "message": "Human readable error message",
    "details": {
      // Additional error details
    }
  }
}
```

## Error Handling

### HTTP Status Codes

- `200` - Success
- `201` - Created
- `400` - Bad Request
- `401` - Unauthorized
- `403` - Forbidden
- `404` - Not Found
- `413` - Payload Too Large
- `422` - Unprocessable Entity
- `429` - Too Many Requests
- `500` - Internal Server Error

### Error Codes

- `INVALID_CREDENTIALS` - Authentication failed
- `INVALID_TOKEN` - JWT token is invalid or expired
- `VALIDATION_ERROR` - Request validation failed
- `RESOURCE_NOT_FOUND` - Requested resource not found
- `PERMISSION_DENIED` - Insufficient permissions
- `RATE_LIMIT_EXCEEDED` - Too many requests
- `FILE_TOO_LARGE` - Uploaded file exceeds size limit
- `PROCESSING_ERROR` - Error during image processing

## Rate Limiting

The API implements rate limiting to prevent abuse:

- **General endpoints**: 100 requests per minute per IP
- **Authentication**: 10 requests per minute per IP
- **Image upload**: 20 requests per minute per user
- **Analytics**: 30 requests per minute per user

Rate limit headers are included in responses:
```http
X-RateLimit-Limit: 100
X-RateLimit-Remaining: 95
X-RateLimit-Reset: 1634567890
```

## API Endpoints

### Authentication

#### Register User
```http
POST /api/auth/register
```

**Request:**
```json
{
  "username": "new_user",
  "email": "user@example.com",
  "password": "secure_password",
  "role": "user"
}
```

**Response:**
```json
{
  "message": "User registered successfully",
  "access_token": "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9...",
  "user": {
    "id": 2,
    "username": "new_user",
    "email": "user@example.com",
    "role": "user"
  }
}
```

#### Login
```http
POST /api/auth/login
```

**Request:**
```json
{
  "username": "your_username",
  "password": "your_password"
}
```

#### Refresh Token
```http
POST /api/auth/refresh
Authorization: Bearer <token>
```

#### Logout
```http
POST /api/auth/logout
Authorization: Bearer <token>
```

### Camera Management

#### List Cameras
```http
GET /api/cameras
Authorization: Bearer <token>
```

**Response:**
```json
{
  "cameras": [
    {
      "id": 1,
      "name": "Forest Trail Camera 1",
      "location": "North Forest Trail",
      "latitude": 45.5236,
      "longitude": -122.6750,
      "status": "online",
      "last_seen": "2023-10-15T14:30:00Z",
      "battery_level": 85,
      "signal_strength": -67,
      "total_images": 1250,
      "recent_detections": 15
    }
  ]
}
```

#### Register Camera
```http
POST /api/cameras
Authorization: Bearer <token>
```

**Request:**
```json
{
  "name": "Forest Trail Camera 2",
  "location": "South Forest Trail",
  "latitude": 45.5200,
  "longitude": -122.6800,
  "device_id": "ESP32_CAM_002",
  "configuration": {
    "resolution": "1600x1200",
    "sensitivity": 0.8,
    "night_mode": true
  }
}
```

#### Get Camera Details
```http
GET /api/cameras/{camera_id}
Authorization: Bearer <token>
```

#### Update Camera
```http
PUT /api/cameras/{camera_id}
Authorization: Bearer <token>
```

#### Delete Camera
```http
DELETE /api/cameras/{camera_id}
Authorization: Bearer <token>
```

#### Update Camera Status (Heartbeat)
```http
POST /api/cameras/{camera_id}/status
Authorization: Bearer <token>
```

**Request:**
```json
{
  "status": "online",
  "battery_level": 82,
  "signal_strength": -65,
  "timestamp": "2023-10-15T14:35:00Z",
  "metadata": {
    "temperature": 22.5,
    "humidity": 65,
    "free_memory": 45000
  }
}
```

### Image Management

#### Upload Image
```http
POST /api/cameras/{camera_id}/upload
Authorization: Bearer <token>
Content-Type: multipart/form-data
```

**Request:**
- `image` (file): Image file (JPEG, PNG)
- `metadata` (string): JSON metadata

**Metadata example:**
```json
{
  "timestamp": "2023-10-15T14:35:00Z",
  "trigger_type": "motion",
  "battery_level": 82,
  "temperature": 22.5,
  "location": {
    "latitude": 45.5236,
    "longitude": -122.6750
  }
}
```

**Response:**
```json
{
  "message": "Image uploaded successfully",
  "image_id": 12345,
  "filename": "550e8400-e29b-41d4-a716-446655440000.jpg",
  "processing_queued": true
}
```

#### Get Image
```http
GET /api/images/{image_id}
Authorization: Bearer <token>
```

Returns the original image file.

#### Get Image Thumbnail
```http
GET /api/images/{image_id}/thumbnail
Authorization: Bearer <token>
```

Returns a thumbnail version of the image.

#### List Images
```http
GET /api/images?camera_id={camera_id}&start_date={date}&end_date={date}&page={page}&per_page={per_page}
Authorization: Bearer <token>
```

**Query Parameters:**
- `camera_id` (optional): Filter by camera
- `start_date` (optional): Start date (ISO 8601)
- `end_date` (optional): End date (ISO 8601)
- `page` (optional): Page number (default: 1)
- `per_page` (optional): Items per page (default: 20, max: 100)

### Wildlife Detection

#### List Detections
```http
GET /api/detections
Authorization: Bearer <token>
```

**Query Parameters:**
- `camera_id` (optional): Filter by camera
- `species_id` (optional): Filter by species
- `start_date` (optional): Start date
- `end_date` (optional): End date
- `confidence_threshold` (optional): Minimum confidence (0.0-1.0)
- `page` (optional): Page number
- `per_page` (optional): Items per page

**Response:**
```json
{
  "detections": [
    {
      "id": 1001,
      "species": "White-tailed Deer",
      "confidence": 0.92,
      "bounding_box": {
        "x": 120,
        "y": 80,
        "width": 200,
        "height": 180
      },
      "image_id": 12345,
      "camera_name": "Forest Trail Camera 1",
      "timestamp": "2023-10-15T14:35:00Z",
      "thumbnail_url": "/api/images/12345/thumbnail"
    }
  ],
  "pagination": {
    "page": 1,
    "per_page": 20,
    "total": 150,
    "pages": 8,
    "has_prev": false,
    "has_next": true
  }
}
```

#### Get Detection Details
```http
GET /api/detections/{detection_id}
Authorization: Bearer <token>
```

#### Verify Detection
```http
PUT /api/detections/{detection_id}/verify
Authorization: Bearer <token>
```

**Request:**
```json
{
  "verified": true,
  "correct_species_id": 5,
  "notes": "Correct identification"
}
```

### Species Management

#### List Species
```http
GET /api/species
```

**Response:**
```json
{
  "species": [
    {
      "id": 1,
      "name": "White-tailed Deer",
      "scientific_name": "Odocoileus virginianus",
      "conservation_status": "Least Concern",
      "description": "Common North American deer species"
    }
  ]
}
```

#### Get Species Details
```http
GET /api/species/{species_id}
```

#### Add Species (Admin only)
```http
POST /api/species
Authorization: Bearer <admin_token>
```

### Analytics

#### Dashboard Analytics
```http
GET /api/analytics/dashboard?days={days}
Authorization: Bearer <token>
```

**Query Parameters:**
- `days` (optional): Number of days to include (default: 7)

**Response:**
```json
{
  "period": {
    "start_date": "2023-10-08T00:00:00Z",
    "end_date": "2023-10-15T23:59:59Z",
    "days": 7
  },
  "summary": {
    "total_cameras": 5,
    "active_cameras": 4,
    "camera_health": 80.0,
    "total_images": 342,
    "total_detections": 89,
    "detection_rate": 26.0,
    "unique_species": 8,
    "average_confidence": 0.82
  },
  "species_breakdown": [
    {
      "species": "White-tailed Deer",
      "scientific_name": "Odocoileus virginianus",
      "detection_count": 35,
      "average_confidence": 0.89,
      "last_seen": "2023-10-15T14:35:00Z"
    }
  ],
  "detection_timeline": [
    {
      "date": "2023-10-15",
      "detection_count": 12,
      "species_count": 4
    }
  ],
  "activity_patterns": {
    "hourly_pattern": {
      "hours": [0, 1, 2, ..., 23],
      "detections": [2, 1, 0, ..., 5],
      "peak_hour": 6
    },
    "daily_pattern": {
      "days": ["Sunday", "Monday", ..., "Saturday"],
      "detections": [15, 12, 8, 10, 14, 18, 22],
      "peak_day": "Saturday"
    }
  }
}
```

#### Species Report
```http
GET /api/analytics/species/{species_id}?start_date={date}&end_date={date}
Authorization: Bearer <token>
```

#### Camera Performance
```http
GET /api/analytics/cameras/{camera_id}/performance
Authorization: Bearer <token>
```

### Alerts

#### List Alerts
```http
GET /api/alerts
Authorization: Bearer <token>
```

#### Mark Alert as Read
```http
PUT /api/alerts/{alert_id}/read
Authorization: Bearer <token>
```

#### Create Alert
```http
POST /api/alerts
Authorization: Bearer <token>
```

### Health Check

#### System Health
```http
GET /api/health
```

**Response:**
```json
{
  "status": "healthy",
  "timestamp": "2023-10-15T14:35:00Z",
  "version": "1.0.0",
  "services": {
    "database": "healthy",
    "redis": "healthy",
    "storage": "healthy"
  }
}
```

## WebSocket Events

The API supports WebSocket connections for real-time updates.

### Connection
```javascript
const ws = new WebSocket('ws://localhost:5000/ws');
```

### Authentication
Send JWT token after connection:
```javascript
ws.send(JSON.stringify({
  type: 'auth',
  token: 'your_jwt_token'
}));
```

### Events

#### New Detection
```json
{
  "type": "new_detection",
  "data": {
    "detection_id": 1001,
    "camera_id": 1,
    "species": "White-tailed Deer",
    "confidence": 0.92,
    "timestamp": "2023-10-15T14:35:00Z",
    "image_url": "/api/images/12345/thumbnail"
  }
}
```

#### Camera Status Update
```json
{
  "type": "camera_status",
  "data": {
    "camera_id": 1,
    "status": "online",
    "battery_level": 82,
    "timestamp": "2023-10-15T14:35:00Z"
  }
}
```

#### System Alert
```json
{
  "type": "system_alert",
  "data": {
    "alert_id": 5001,
    "type": "low_battery",
    "severity": "warning",
    "camera_id": 3,
    "message": "Camera battery level is below 20%"
  }
}
```

## Examples

### JavaScript/Node.js

```javascript
const axios = require('axios');

// Login and get token
async function login(username, password) {
  const response = await axios.post('http://localhost:5000/api/auth/login', {
    username,
    password
  });
  return response.data.access_token;
}

// Upload image
async function uploadImage(token, cameraId, imageFile, metadata) {
  const formData = new FormData();
  formData.append('image', imageFile);
  formData.append('metadata', JSON.stringify(metadata));
  
  const response = await axios.post(
    `http://localhost:5000/api/cameras/${cameraId}/upload`,
    formData,
    {
      headers: {
        'Authorization': `Bearer ${token}`,
        'Content-Type': 'multipart/form-data'
      }
    }
  );
  
  return response.data;
}

// Get analytics
async function getAnalytics(token, days = 7) {
  const response = await axios.get(
    `http://localhost:5000/api/analytics/dashboard?days=${days}`,
    {
      headers: {
        'Authorization': `Bearer ${token}`
      }
    }
  );
  
  return response.data;
}
```

### Python

```python
import requests
import json

class WildlifeCameraAPI:
    def __init__(self, base_url):
        self.base_url = base_url
        self.token = None
    
    def login(self, username, password):
        response = requests.post(f'{self.base_url}/auth/login', json={
            'username': username,
            'password': password
        })
        
        if response.status_code == 200:
            self.token = response.json()['access_token']
            return True
        return False
    
    def get_headers(self):
        return {'Authorization': f'Bearer {self.token}'}
    
    def get_cameras(self):
        response = requests.get(
            f'{self.base_url}/cameras',
            headers=self.get_headers()
        )
        return response.json()
    
    def upload_image(self, camera_id, image_path, metadata=None):
        files = {'image': open(image_path, 'rb')}
        data = {}
        
        if metadata:
            data['metadata'] = json.dumps(metadata)
        
        response = requests.post(
            f'{self.base_url}/cameras/{camera_id}/upload',
            files=files,
            data=data,
            headers=self.get_headers()
        )
        
        return response.json()

# Usage
api = WildlifeCameraAPI('http://localhost:5000/api')
api.login('username', 'password')
cameras = api.get_cameras()
```

### ESP32 Arduino

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class WildlifeCameraAPI {
private:
    String baseUrl;
    String token;
    
public:
    WildlifeCameraAPI(String url) : baseUrl(url) {}
    
    bool login(String username, String password) {
        HTTPClient http;
        http.begin(baseUrl + "/auth/login");
        http.addHeader("Content-Type", "application/json");
        
        DynamicJsonDocument doc(1024);
        doc["username"] = username;
        doc["password"] = password;
        
        String requestBody;
        serializeJson(doc, requestBody);
        
        int httpResponseCode = http.POST(requestBody);
        
        if (httpResponseCode == 200) {
            String response = http.getString();
            DynamicJsonDocument responseDoc(2048);
            deserializeJson(responseDoc, response);
            
            token = responseDoc["access_token"].as<String>();
            return true;
        }
        
        return false;
    }
    
    bool updateCameraStatus(int cameraId, int batteryLevel, int signalStrength) {
        HTTPClient http;
        http.begin(baseUrl + "/cameras/" + String(cameraId) + "/status");
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", "Bearer " + token);
        
        DynamicJsonDocument doc(1024);
        doc["status"] = "online";
        doc["battery_level"] = batteryLevel;
        doc["signal_strength"] = signalStrength;
        doc["timestamp"] = "2023-10-15T14:35:00Z";
        
        String requestBody;
        serializeJson(doc, requestBody);
        
        int httpResponseCode = http.POST(requestBody);
        
        return httpResponseCode == 200;
    }
};
```

---

For more detailed examples and integration guides, see:
- [Frontend Integration Guide](FRONTEND_INTEGRATION.md)
- [ESP32 Integration Guide](ESP32_API_INTEGRATION.md)
- [Third-party Integrations](THIRD_PARTY_INTEGRATIONS.md)