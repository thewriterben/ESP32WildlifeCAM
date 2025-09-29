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
from flask_socketio import SocketIO, emit, join_room, leave_room
from flask_limiter import Limiter
from flask_limiter.util import get_remote_address
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
from functools import wraps

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
    MEGADETECTOR_API_URL = os.environ.get('MEGADETECTOR_API_URL') or 'https://api.lila.science/batch/v1/detect'
    
    # CORS configuration
    CORS_ORIGINS = os.environ.get('CORS_ORIGINS', 'http://localhost:3000').split(',')
    
    # Rate limiting configuration
    RATELIMIT_STORAGE_URL = os.environ.get('RATE_LIMIT_STORAGE_URL') or 'redis://localhost:6379/1'
    RATELIMIT_DEFAULT = os.environ.get('RATE_LIMIT_DEFAULT') or '1000/hour'
    
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
cors = CORS(app, origins=app.config['CORS_ORIGINS'])
jwt = JWTManager(app)
socketio = SocketIO(app, cors_allowed_origins=app.config['CORS_ORIGINS'])
limiter = Limiter(
    app,
    key_func=get_remote_address,
    storage_uri=app.config['RATELIMIT_STORAGE_URL'],
    default_limits=[app.config['RATELIMIT_DEFAULT']]
)

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

# Celery tasks
@celery.task
def process_image_task(image_id):
    """Background task to process uploaded image for wildlife detection."""
    try:
        image = CameraImage.query.get(image_id)
        if not image:
            logger.error(f"Image {image_id} not found for processing")
            return
        
        # Run wildlife detection
        detections = wildlife_detector.detect_wildlife(image.file_path)
        
        # Save detections to database
        for detection in detections:
            wildlife_detection = WildlifeDetection(
                image_id=image_id,
                confidence=detection['confidence'],
                bounding_box=detection.get('bounding_box'),
                species_id=detection.get('species_id'),
                behavior_classification=detection.get('behavior'),
                group_size=detection.get('group_size', 1),
                model_version=detection.get('model_version', 'megadetector_v5a'),
                detection_method=detection.get('method', 'megadetector')
            )
            db.session.add(wildlife_detection)
        
        db.session.commit()
        
        # Emit real-time update
        socketio.emit('detection_complete', {
            'image_id': image_id,
            'detection_count': len(detections)
        }, room=f'user_{image.camera.user_id}')
        
        logger.info(f"Processed image {image_id}, found {len(detections)} detections")
        
    except Exception as e:
        logger.error(f"Error processing image {image_id}: {str(e)}")

# Initialize custom services
wildlife_detector = WildlifeDetector(app.config['MEGADETECTOR_MODEL_PATH'])
image_processor = ImageProcessor(app.config['UPLOAD_FOLDER'])
analytics_engine = AnalyticsEngine()

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# WebSocket event handlers
@socketio.on('connect')
@jwt_required()
def handle_connect():
    """Handle client connection."""
    try:
        user_id = get_jwt_identity()
        join_room(f'user_{user_id}')
        emit('connected', {'message': 'Connected to wildlife camera system'})
        logger.info(f"User {user_id} connected to WebSocket")
    except Exception as e:
        logger.error(f"WebSocket connection error: {str(e)}")
        emit('error', {'message': 'Connection failed'})

@socketio.on('disconnect')
@jwt_required()
def handle_disconnect():
    """Handle client disconnection."""
    try:
        user_id = get_jwt_identity()
        leave_room(f'user_{user_id}')
        logger.info(f"User {user_id} disconnected from WebSocket")
    except Exception as e:
        logger.error(f"WebSocket disconnection error: {str(e)}")

@socketio.on('join_camera_room')
@jwt_required()
def handle_join_camera_room(data):
    """Join a specific camera room for real-time updates."""
    try:
        user_id = get_jwt_identity()
        camera_id = data.get('camera_id')
        
        # Verify camera ownership
        camera = Camera.query.filter_by(id=camera_id, user_id=user_id).first()
        if camera:
            join_room(f'camera_{camera_id}')
            emit('joined_camera_room', {'camera_id': camera_id})
        else:
            emit('error', {'message': 'Camera not found or access denied'})
    except Exception as e:
        logger.error(f"Join camera room error: {str(e)}")
        emit('error', {'message': 'Failed to join camera room'})

@socketio.on('leave_camera_room')
@jwt_required()
def handle_leave_camera_room(data):
    """Leave a specific camera room."""
    try:
        camera_id = data.get('camera_id')
        leave_room(f'camera_{camera_id}')
        emit('left_camera_room', {'camera_id': camera_id})
    except Exception as e:
        logger.error(f"Leave camera room error: {str(e)}")
        emit('error', {'message': 'Failed to leave camera room'})

# Utility functions
def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in app.config['ALLOWED_EXTENSIONS']

def generate_unique_filename(original_filename):
    """Generate a unique filename while preserving the extension."""
    extension = original_filename.rsplit('.', 1)[1].lower()
    unique_id = str(uuid.uuid4())
    return f"{unique_id}.{extension}"

def validate_json_input(required_fields):
    """Decorator to validate JSON input fields."""
    def decorator(f):
        @wraps(f)
        def wrapper(*args, **kwargs):
            if not request.is_json:
                return jsonify({'error': 'Content-Type must be application/json'}), 400
            
            data = request.get_json()
            if not data:
                return jsonify({'error': 'No JSON data provided'}), 400
            
            missing_fields = [field for field in required_fields if field not in data]
            if missing_fields:
                return jsonify({'error': f'Missing required fields: {", ".join(missing_fields)}'}), 400
                
            return f(*args, **kwargs)
        return wrapper
    return decorator

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
@validate_json_input(['name', 'location'])
@limiter.limit("10/minute")
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
        
        # Emit real-time update
        socketio.emit('camera_registered', {
            'camera_id': camera.id,
            'name': camera.name,
            'user_id': user_id
        }, room=f'user_{user_id}')
        
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

@app.route('/api/cameras/<int:camera_id>', methods=['GET'])
@jwt_required()
def get_camera(camera_id):
    """Get details for a specific camera."""
    try:
        user_id = get_jwt_identity()
        camera = Camera.query.filter_by(id=camera_id, user_id=user_id).first()
        
        if not camera:
            return jsonify({'error': 'Camera not found'}), 404
        
        camera_data = {
            'id': camera.id,
            'name': camera.name,
            'device_id': camera.device_id,
            'location': camera.location,
            'latitude': camera.latitude,
            'longitude': camera.longitude,
            'status': camera.status,
            'last_seen': camera.last_seen.isoformat() if camera.last_seen else None,
            'battery_level': camera.battery_level,
            'signal_strength': camera.signal_strength,
            'configuration': camera.configuration,
            'total_images': camera.images.count(),
            'recent_detections': camera.recent_detections_count(),
            'created_at': camera.created_at.isoformat()
        }
        
        return jsonify({'camera': camera_data}), 200
        
    except Exception as e:
        logger.error(f"Get camera error: {str(e)}")
        return jsonify({'error': 'Failed to retrieve camera'}), 500

@app.route('/api/cameras/<int:camera_id>', methods=['PUT'])
@jwt_required()
@limiter.limit("20/minute")
def update_camera(camera_id):
    """Update camera configuration."""
    try:
        user_id = get_jwt_identity()
        camera = Camera.query.filter_by(id=camera_id, user_id=user_id).first()
        
        if not camera:
            return jsonify({'error': 'Camera not found'}), 404
        
        data = request.get_json()
        
        # Update allowed fields
        if 'name' in data:
            camera.name = data['name']
        if 'location' in data:
            camera.location = data['location']
        if 'latitude' in data:
            camera.latitude = data['latitude']
        if 'longitude' in data:
            camera.longitude = data['longitude']
        if 'configuration' in data:
            camera.configuration = data['configuration']
        
        db.session.commit()
        
        return jsonify({'message': 'Camera updated successfully'}), 200
        
    except Exception as e:
        logger.error(f"Camera update error: {str(e)}")
        return jsonify({'error': 'Failed to update camera'}), 500

@app.route('/api/cameras/<int:camera_id>', methods=['DELETE'])
@jwt_required()
@limiter.limit("5/minute")
def delete_camera(camera_id):
    """Delete a camera and all associated data."""
    try:
        user_id = get_jwt_identity()
        camera = Camera.query.filter_by(id=camera_id, user_id=user_id).first()
        
        if not camera:
            return jsonify({'error': 'Camera not found'}), 404
        
        db.session.delete(camera)
        db.session.commit()
        
        return jsonify({'message': 'Camera deleted successfully'}), 200
        
    except Exception as e:
        logger.error(f"Camera deletion error: {str(e)}")
        return jsonify({'error': 'Failed to delete camera'}), 500

# Image management routes
@app.route('/api/images', methods=['POST'])
@jwt_required()
@limiter.limit("100/hour")
def upload_image():
    """Upload and process camera image."""
    try:
        user_id = get_jwt_identity()
        
        if 'image' not in request.files:
            return jsonify({'error': 'No image file provided'}), 400
        
        file = request.files['image']
        camera_id = request.form.get('camera_id')
        
        if not camera_id:
            return jsonify({'error': 'Camera ID required'}), 400
        
        # Verify camera ownership
        camera = Camera.query.filter_by(id=camera_id, user_id=user_id).first()
        if not camera:
            return jsonify({'error': 'Camera not found'}), 404
        
        if file.filename == '':
            return jsonify({'error': 'No file selected'}), 400
        
        if not allowed_file(file.filename):
            return jsonify({'error': 'File type not allowed'}), 400
        
        # Generate unique filename and save
        filename = generate_unique_filename(file.filename)
        file_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
        file.save(file_path)
        
        # Create image record
        image_record = CameraImage(
            camera_id=camera_id,
            filename=filename,
            file_path=file_path,
            file_size=os.path.getsize(file_path),
            metadata=request.form.get('metadata', '{}')
        )
        
        db.session.add(image_record)
        db.session.commit()
        
        # Queue for wildlife detection processing
        process_image_task.delay(image_record.id)
        
        # Emit real-time update
        socketio.emit('image_uploaded', {
            'image_id': image_record.id,
            'camera_id': camera_id,
            'filename': filename
        }, room=f'user_{user_id}')
        
        return jsonify({
            'message': 'Image uploaded successfully',
            'image': {
                'id': image_record.id,
                'filename': filename,
                'camera_id': camera_id
            }
        }), 201
        
    except Exception as e:
        logger.error(f"Image upload error: {str(e)}")
        return jsonify({'error': 'Image upload failed'}), 500

@app.route('/api/images', methods=['GET'])
@jwt_required()
def get_images():
    """Get images for user's cameras."""
    try:
        user_id = get_jwt_identity()
        page = request.args.get('page', 1, type=int)
        per_page = min(request.args.get('per_page', 20, type=int), 100)
        camera_id = request.args.get('camera_id', type=int)
        
        # Base query for user's images
        query = db.session.query(CameraImage).join(Camera).filter(Camera.user_id == user_id)
        
        if camera_id:
            query = query.filter(CameraImage.camera_id == camera_id)
        
        # Paginate results
        images = query.order_by(CameraImage.created_at.desc()).paginate(
            page=page, per_page=per_page, error_out=False
        )
        
        image_list = []
        for image in images.items:
            image_data = {
                'id': image.id,
                'filename': image.filename,
                'camera_id': image.camera_id,
                'camera_name': image.camera.name,
                'file_size': image.file_size,
                'created_at': image.created_at.isoformat(),
                'detection_count': len(image.detections)
            }
            image_list.append(image_data)
        
        return jsonify({
            'images': image_list,
            'pagination': {
                'page': page,
                'per_page': per_page,
                'total': images.total,
                'pages': images.pages
            }
        }), 200
        
    except Exception as e:
        logger.error(f"Get images error: {str(e)}")
        return jsonify({'error': 'Failed to retrieve images'}), 500

@app.route('/api/images/<int:image_id>', methods=['GET'])
@jwt_required()
def get_image(image_id):
    """Get specific image details."""
    try:
        user_id = get_jwt_identity()
        image = db.session.query(CameraImage).join(Camera).filter(
            CameraImage.id == image_id,
            Camera.user_id == user_id
        ).first()
        
        if not image:
            return jsonify({'error': 'Image not found'}), 404
        
        image_data = {
            'id': image.id,
            'filename': image.filename,
            'camera_id': image.camera_id,
            'camera_name': image.camera.name,
            'file_size': image.file_size,
            'metadata': image.metadata,
            'created_at': image.created_at.isoformat(),
            'detections': [{
                'id': d.id,
                'species_name': d.species.name if d.species else 'Unknown',
                'confidence': d.confidence,
                'bounding_box': d.bounding_box
            } for d in image.detections]
        }
        
        return jsonify({'image': image_data}), 200
        
    except Exception as e:
        logger.error(f"Get image error: {str(e)}")
        return jsonify({'error': 'Failed to retrieve image'}), 500

@app.route('/api/images/<int:image_id>/file', methods=['GET'])
@jwt_required()
def serve_image(image_id):
    """Serve image file."""
    try:
        user_id = get_jwt_identity()
        image = db.session.query(CameraImage).join(Camera).filter(
            CameraImage.id == image_id,
            Camera.user_id == user_id
        ).first()
        
        if not image:
            return jsonify({'error': 'Image not found'}), 404
        
        if not os.path.exists(image.file_path):
            return jsonify({'error': 'Image file not found'}), 404
        
        return send_file(image.file_path)
        
    except Exception as e:
        logger.error(f"Serve image error: {str(e)}")
        return jsonify({'error': 'Failed to serve image'}), 500

# Wildlife detection routes
@app.route('/api/detections', methods=['GET'])
@jwt_required()
def get_detections():
    """Get wildlife detections for user's cameras."""
    try:
        user_id = get_jwt_identity()
        page = request.args.get('page', 1, type=int)
        per_page = min(request.args.get('per_page', 20, type=int), 100)
        camera_id = request.args.get('camera_id', type=int)
        species_id = request.args.get('species_id', type=int)
        confidence_threshold = request.args.get('confidence_threshold', 0.0, type=float)
        start_date = request.args.get('start_date')
        end_date = request.args.get('end_date')
        
        # Base query for user's detections
        query = db.session.query(WildlifeDetection).join(CameraImage).join(Camera).filter(
            Camera.user_id == user_id
        )
        
        # Apply filters
        if camera_id:
            query = query.filter(CameraImage.camera_id == camera_id)
        if species_id:
            query = query.filter(WildlifeDetection.species_id == species_id)
        if confidence_threshold:
            query = query.filter(WildlifeDetection.confidence >= confidence_threshold)
        if start_date:
            start_dt = datetime.fromisoformat(start_date)
            query = query.filter(WildlifeDetection.created_at >= start_dt)
        if end_date:
            end_dt = datetime.fromisoformat(end_date)
            query = query.filter(WildlifeDetection.created_at <= end_dt)
        
        # Paginate results
        detections = query.order_by(WildlifeDetection.created_at.desc()).paginate(
            page=page, per_page=per_page, error_out=False
        )
        
        detection_list = []
        for detection in detections.items:
            detection_data = {
                'id': detection.id,
                'species_name': detection.species.name if detection.species else 'Unknown',
                'species_id': detection.species_id,
                'confidence': detection.confidence,
                'bounding_box': detection.bounding_box,
                'behavior_classification': detection.behavior_classification,
                'group_size': detection.group_size,
                'verified': detection.verified,
                'image_id': detection.image_id,
                'camera_name': detection.image.camera.name,
                'created_at': detection.created_at.isoformat()
            }
            detection_list.append(detection_data)
        
        return jsonify({
            'detections': detection_list,
            'pagination': {
                'page': page,
                'per_page': per_page,
                'total': detections.total,
                'pages': detections.pages
            }
        }), 200
        
    except Exception as e:
        logger.error(f"Get detections error: {str(e)}")
        return jsonify({'error': 'Failed to retrieve detections'}), 500

@app.route('/api/detections/<int:detection_id>', methods=['GET'])
@jwt_required()
def get_detection(detection_id):
    """Get specific detection details."""
    try:
        user_id = get_jwt_identity()
        detection = db.session.query(WildlifeDetection).join(CameraImage).join(Camera).filter(
            WildlifeDetection.id == detection_id,
            Camera.user_id == user_id
        ).first()
        
        if not detection:
            return jsonify({'error': 'Detection not found'}), 404
        
        detection_data = {
            'id': detection.id,
            'species_name': detection.species.name if detection.species else 'Unknown',
            'species_id': detection.species_id,
            'confidence': detection.confidence,
            'bounding_box': detection.bounding_box,
            'behavior_classification': detection.behavior_classification,
            'group_size': detection.group_size,
            'age_class': detection.age_class,
            'sex': detection.sex,
            'verified': detection.verified,
            'verified_by': detection.verified_by,
            'verification_notes': detection.verification_notes,
            'model_version': detection.model_version,
            'detection_method': detection.detection_method,
            'image_id': detection.image_id,
            'camera_name': detection.image.camera.name,
            'created_at': detection.created_at.isoformat()
        }
        
        return jsonify({'detection': detection_data}), 200
        
    except Exception as e:
        logger.error(f"Get detection error: {str(e)}")
        return jsonify({'error': 'Failed to retrieve detection'}), 500

@app.route('/api/detections/<int:detection_id>/verify', methods=['PUT'])
@jwt_required()
@validate_json_input(['verified'])
@limiter.limit("50/minute")
def verify_detection(detection_id):
    """Verify or correct a wildlife detection."""
    try:
        user_id = get_jwt_identity()
        detection = db.session.query(WildlifeDetection).join(CameraImage).join(Camera).filter(
            WildlifeDetection.id == detection_id,
            Camera.user_id == user_id
        ).first()
        
        if not detection:
            return jsonify({'error': 'Detection not found'}), 404
        
        data = request.get_json()
        
        detection.verified = data['verified']
        detection.verified_by = user_id
        detection.verification_notes = data.get('notes', '')
        
        # Update species if corrected
        if 'correct_species_id' in data:
            detection.species_id = data['correct_species_id']
        
        db.session.commit()
        
        return jsonify({'message': 'Detection verification updated'}), 200
        
    except Exception as e:
        logger.error(f"Verify detection error: {str(e)}")
        return jsonify({'error': 'Failed to verify detection'}), 500

# Species management routes
@app.route('/api/species', methods=['GET'])
def get_species():
    """Get list of all species."""
    try:
        species_list = []
        species = Species.query.all()
        
        for s in species:
            species_data = {
                'id': s.id,
                'name': s.name,
                'scientific_name': s.scientific_name,
                'conservation_status': s.conservation_status,
                'description': s.description,
                'is_endangered': s.is_endangered,
                'is_protected': s.is_protected
            }
            species_list.append(species_data)
        
        return jsonify({'species': species_list}), 200
        
    except Exception as e:
        logger.error(f"Get species error: {str(e)}")
        return jsonify({'error': 'Failed to retrieve species'}), 500

@app.route('/api/species/<int:species_id>', methods=['GET'])
def get_species_details(species_id):
    """Get detailed information about a specific species."""
    try:
        species = Species.query.get(species_id)
        
        if not species:
            return jsonify({'error': 'Species not found'}), 404
        
        # Get recent detection stats
        recent_detections = WildlifeDetection.query.filter_by(species_id=species_id).filter(
            WildlifeDetection.created_at >= datetime.utcnow() - timedelta(days=30)
        ).count()
        
        species_data = {
            'id': species.id,
            'name': species.name,
            'scientific_name': species.scientific_name,
            'conservation_status': species.conservation_status,
            'description': species.description,
            'habitat': species.habitat,
            'behavior': species.behavior,
            'diet': species.diet,
            'average_size': species.average_size,
            'is_endangered': species.is_endangered,
            'is_protected': species.is_protected,
            'research_priority': species.research_priority,
            'recent_detections': recent_detections,
            'total_detections': len(species.detections)
        }
        
        return jsonify({'species': species_data}), 200
        
    except Exception as e:
        logger.error(f"Get species details error: {str(e)}")
        return jsonify({'error': 'Failed to retrieve species details'}), 500

# Analytics routes
@app.route('/api/analytics/dashboard', methods=['GET'])
@jwt_required()
def get_dashboard_analytics():
    """Get dashboard analytics for the user."""
    try:
        user_id = get_jwt_identity()
        days = request.args.get('days', 7, type=int)
        
        end_date = datetime.utcnow()
        start_date = end_date - timedelta(days=days)
        
        analytics = analytics_engine.generate_dashboard_analytics(
            user_id=user_id,
            start_date=start_date,
            end_date=end_date
        )
        
        return jsonify(analytics), 200
        
    except Exception as e:
        logger.error(f"Dashboard analytics error: {str(e)}")
        return jsonify({'error': 'Failed to generate analytics'}), 500

@app.route('/api/analytics/species/<int:species_id>', methods=['GET'])
@jwt_required()
def get_species_analytics(species_id):
    """Get detailed analytics for a specific species."""
    try:
        user_id = get_jwt_identity()
        start_date = request.args.get('start_date')
        end_date = request.args.get('end_date')
        
        if start_date:
            start_date = datetime.fromisoformat(start_date)
        if end_date:
            end_date = datetime.fromisoformat(end_date)
        
        report = analytics_engine.generate_species_report(
            user_id=user_id,
            species_id=species_id,
            start_date=start_date,
            end_date=end_date
        )
        
        return jsonify(report), 200
        
    except Exception as e:
        logger.error(f"Species analytics error: {str(e)}")
        return jsonify({'error': 'Failed to generate species analytics'}), 500

@app.route('/api/analytics/cameras/<int:camera_id>/performance', methods=['GET'])
@jwt_required()
def get_camera_performance(camera_id):
    """Get performance analytics for a specific camera."""
    try:
        user_id = get_jwt_identity()
        days = request.args.get('days', 30, type=int)
        
        # Verify camera ownership
        camera = Camera.query.filter_by(id=camera_id, user_id=user_id).first()
        if not camera:
            return jsonify({'error': 'Camera not found'}), 404
        
        end_date = datetime.utcnow()
        start_date = end_date - timedelta(days=days)
        
        performance_data = analytics_engine._generate_camera_performance(
            user_id=user_id,
            start_date=start_date,
            end_date=end_date
        )
        
        # Filter for specific camera
        camera_performance = next((p for p in performance_data if p['camera_id'] == camera_id), None)
        
        if not camera_performance:
            camera_performance = {
                'camera_id': camera_id,
                'camera_name': camera.name,
                'image_count': 0,
                'detection_count': 0,
                'detection_rate': 0,
                'health_score': 0
            }
        
        return jsonify({'performance': camera_performance}), 200
        
    except Exception as e:
        logger.error(f"Camera performance error: {str(e)}")
        return jsonify({'error': 'Failed to generate camera performance analytics'}), 500

@app.route('/api/analytics/trends', methods=['GET'])
@jwt_required()
def get_detection_trends():
    """Get detection trends analytics."""
    try:
        user_id = get_jwt_identity()
        days = request.args.get('days', 30, type=int)
        species_id = request.args.get('species_id', type=int)
        
        end_date = datetime.utcnow()
        start_date = end_date - timedelta(days=days)
        
        analytics = analytics_engine.generate_dashboard_analytics(
            user_id=user_id,
            start_date=start_date,
            end_date=end_date
        )
        
        trends_data = {
            'detection_timeline': analytics.get('detection_timeline', []),
            'species_breakdown': analytics.get('species_breakdown', []),
            'activity_patterns': analytics.get('activity_patterns', {}),
            'period': analytics.get('period', {})
        }
        
        return jsonify(trends_data), 200
        
    except Exception as e:
        logger.error(f"Detection trends error: {str(e)}")
        return jsonify({'error': 'Failed to generate trends analytics'}), 500

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

@app.errorhandler(400)
def bad_request(error):
    return jsonify({'error': 'Bad request'}), 400

@app.errorhandler(401)
def unauthorized(error):
    return jsonify({'error': 'Unauthorized'}), 401

@app.errorhandler(403)
def forbidden(error):
    return jsonify({'error': 'Forbidden'}), 403

@app.errorhandler(413)
def request_entity_too_large(error):
    return jsonify({'error': 'File too large'}), 413

@app.errorhandler(429)
def ratelimit_handler(e):
    return jsonify({'error': 'Rate limit exceeded', 'message': str(e.description)}), 429

if __name__ == '__main__':
    # Create upload directory
    os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)
    
    # Run application with SocketIO
    socketio.run(
        app,
        host=os.environ.get('HOST', '0.0.0.0'),
        port=int(os.environ.get('PORT', 5000)),
        debug=os.environ.get('DEBUG', 'False').lower() == 'true'
    )