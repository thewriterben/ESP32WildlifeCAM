#!/usr/bin/env python3
"""
ESP32 Wildlife Camera Backend API Server
A comprehensive Flask-based backend for wildlife monitoring camera networks.

Features:
- Camera registration and management
- Image upload and processing
- Species detection using Microsoft MegaDetector
- Analytics and reporting
- Real-time monitoring
- Multi-camera deployment support
"""

from flask import Flask, request, jsonify, send_file
from flask_sqlalchemy import SQLAlchemy
from flask_migrate import Migrate
from flask_cors import CORS
from flask_jwt_extended import JWTManager, create_access_token, jwt_required, get_jwt_identity
from werkzeug.utils import secure_filename
from werkzeug.security import generate_password_hash, check_password_hash
from celery import Celery
import os
import json
import uuid
from datetime import datetime, timedelta
import logging
from pathlib import Path
import PIL.Image
import numpy as np
from typing import Dict, List, Optional, Tuple
import requests

# Import custom modules
from models import (
    db, Camera, CameraImage, WildlifeDetection, Species, 
    User, SystemConfig, Analytics, Alert
)
from wildlife_detection import WildlifeDetector
from image_processor import ImageProcessor
from analytics_engine import AnalyticsEngine

# Application configuration
class Config:
    # Database configuration
    SQLALCHEMY_DATABASE_URI = os.environ.get('DATABASE_URL') or \
        'postgresql://wildlife:wildlife123@localhost/wildlife_camera'
    SQLALCHEMY_TRACK_MODIFICATIONS = False
    
    # File upload configuration
    MAX_CONTENT_LENGTH = 50 * 1024 * 1024  # 50MB max file size
    UPLOAD_FOLDER = os.environ.get('UPLOAD_FOLDER') or './uploads'
    ALLOWED_EXTENSIONS = {'jpg', 'jpeg', 'png', 'gif', 'mp4', 'avi'}
    
    # Security configuration
    JWT_SECRET_KEY = os.environ.get('JWT_SECRET_KEY') or 'wildlife-camera-secret-key'
    JWT_ACCESS_TOKEN_EXPIRES = timedelta(hours=24)
    
    # Celery configuration
    CELERY_BROKER_URL = os.environ.get('CELERY_BROKER_URL') or 'redis://localhost:6379/0'
    CELERY_RESULT_BACKEND = os.environ.get('CELERY_RESULT_BACKEND') or 'redis://localhost:6379/0'
    
    # Microsoft MegaDetector configuration
    MEGADETECTOR_MODEL_PATH = os.environ.get('MEGADETECTOR_MODEL_PATH') or './models/megadetector_v5a.pb'
    MEGADETECTOR_CONFIDENCE_THRESHOLD = float(os.environ.get('MEGADETECTOR_CONFIDENCE_THRESHOLD', '0.2'))
    
    # Cloud storage configuration
    AWS_ACCESS_KEY_ID = os.environ.get('AWS_ACCESS_KEY_ID')
    AWS_SECRET_ACCESS_KEY = os.environ.get('AWS_SECRET_ACCESS_KEY')
    AWS_S3_BUCKET = os.environ.get('AWS_S3_BUCKET', 'wildlife-camera-images')
    AWS_REGION = os.environ.get('AWS_REGION', 'us-west-2')

# Initialize Flask application
app = Flask(__name__)
app.config.from_object(Config)

# Initialize extensions
db.init_app(app)
migrate = Migrate(app, db)
cors = CORS(app)
jwt = JWTManager(app)

# Initialize Celery
def make_celery(app):
    celery = Celery(
        app.import_name,
        backend=app.config['CELERY_RESULT_BACKEND'],
        broker=app.config['CELERY_BROKER_URL']
    )
    celery.conf.update(app.config)
    return celery

celery = make_celery(app)

# Initialize custom services
wildlife_detector = WildlifeDetector(app.config['MEGADETECTOR_MODEL_PATH'])
image_processor = ImageProcessor(app.config['UPLOAD_FOLDER'])
analytics_engine = AnalyticsEngine()

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Utility functions
def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in app.config['ALLOWED_EXTENSIONS']

def generate_unique_filename(original_filename):
    """Generate a unique filename while preserving the extension."""
    extension = original_filename.rsplit('.', 1)[1].lower()
    unique_id = str(uuid.uuid4())
    return f"{unique_id}.{extension}"

# Authentication routes
@app.route('/api/auth/register', methods=['POST'])
def register():
    """Register a new user."""
    try:
        data = request.get_json()
        
        if User.query.filter_by(username=data['username']).first():
            return jsonify({'error': 'Username already exists'}), 400
        
        if User.query.filter_by(email=data['email']).first():
            return jsonify({'error': 'Email already registered'}), 400
        
        user = User(
            username=data['username'],
            email=data['email'],
            password_hash=generate_password_hash(data['password']),
            role=data.get('role', 'user')
        )
        
        db.session.add(user)
        db.session.commit()
        
        access_token = create_access_token(identity=user.id)
        
        return jsonify({
            'message': 'User registered successfully',
            'access_token': access_token,
            'user': {
                'id': user.id,
                'username': user.username,
                'email': user.email,
                'role': user.role
            }
        }), 201
        
    except Exception as e:
        logger.error(f"Registration error: {str(e)}")
        return jsonify({'error': 'Registration failed'}), 500

@app.route('/api/auth/login', methods=['POST'])
def login():
    """Authenticate user and return access token."""
    try:
        data = request.get_json()
        user = User.query.filter_by(username=data['username']).first()
        
        if user and check_password_hash(user.password_hash, data['password']):
            access_token = create_access_token(identity=user.id)
            
            return jsonify({
                'message': 'Login successful',
                'access_token': access_token,
                'user': {
                    'id': user.id,
                    'username': user.username,
                    'email': user.email,
                    'role': user.role
                }
            }), 200
        
        return jsonify({'error': 'Invalid credentials'}), 401
        
    except Exception as e:
        logger.error(f"Login error: {str(e)}")
        return jsonify({'error': 'Login failed'}), 500

# Camera management routes
@app.route('/api/cameras', methods=['GET'])
@jwt_required()
def get_cameras():
    """Get all cameras for the current user."""
    try:
        user_id = get_jwt_identity()
        cameras = Camera.query.filter_by(user_id=user_id).all()
        
        camera_list = []
        for camera in cameras:
            camera_data = {
                'id': camera.id,
                'name': camera.name,
                'location': camera.location,
                'latitude': camera.latitude,
                'longitude': camera.longitude,
                'status': camera.status,
                'last_seen': camera.last_seen.isoformat() if camera.last_seen else None,
                'battery_level': camera.battery_level,
                'signal_strength': camera.signal_strength,
                'total_images': camera.images.count(),
                'recent_detections': camera.recent_detections_count()
            }
            camera_list.append(camera_data)
        
        return jsonify({'cameras': camera_list}), 200
        
    except Exception as e:
        logger.error(f"Get cameras error: {str(e)}")
        return jsonify({'error': 'Failed to retrieve cameras'}), 500

@app.route('/api/cameras', methods=['POST'])
@jwt_required()
def register_camera():
    """Register a new camera."""
    try:
        user_id = get_jwt_identity()
        data = request.get_json()
        
        camera = Camera(
            name=data['name'],
            location=data['location'],
            latitude=data.get('latitude'),
            longitude=data.get('longitude'),
            user_id=user_id,
            device_id=data.get('device_id', str(uuid.uuid4())),
            configuration=data.get('configuration', {})
        )
        
        db.session.add(camera)
        db.session.commit()
        
        return jsonify({
            'message': 'Camera registered successfully',
            'camera': {
                'id': camera.id,
                'name': camera.name,
                'device_id': camera.device_id,
                'location': camera.location
            }
        }), 201
        
    except Exception as e:
        logger.error(f"Camera registration error: {str(e)}")
        return jsonify({'error': 'Camera registration failed'}), 500

# Health check endpoint
@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint for monitoring."""
    try:
        # Check database connection
        db.session.execute('SELECT 1')
        
        return jsonify({
            'status': 'healthy',
            'timestamp': datetime.utcnow().isoformat(),
            'version': '1.0.0'
        }), 200
        
    except Exception as e:
        logger.error(f"Health check failed: {str(e)}")
        return jsonify({
            'status': 'unhealthy',
            'error': str(e),
            'timestamp': datetime.utcnow().isoformat()
        }), 500

# Error handlers
@app.errorhandler(404)
def not_found(error):
    return jsonify({'error': 'Endpoint not found'}), 404

@app.errorhandler(500)
def internal_error(error):
    return jsonify({'error': 'Internal server error'}), 500

if __name__ == '__main__':
    # Create upload directory
    os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)
    
    # Run application
    app.run(
        host=os.environ.get('HOST', '0.0.0.0'),
        port=int(os.environ.get('PORT', 5000)),
        debug=os.environ.get('DEBUG', 'False').lower() == 'true'
    )