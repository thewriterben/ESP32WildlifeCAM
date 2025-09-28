"""
Database models for ESP32 Wildlife Camera Backend
"""

from flask_sqlalchemy import SQLAlchemy
from datetime import datetime
import json
from sqlalchemy.dialects.postgresql import UUID, JSONB
import uuid

db = SQLAlchemy()

class User(db.Model):
    """User model for authentication and authorization."""
    __tablename__ = 'users'
    
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    email = db.Column(db.String(120), unique=True, nullable=False)
    password_hash = db.Column(db.String(255), nullable=False)
    role = db.Column(db.String(20), default='user')  # user, admin, researcher
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    last_login = db.Column(db.DateTime)
    is_active = db.Column(db.Boolean, default=True)
    
    # Relationships
    cameras = db.relationship('Camera', backref='user', lazy=True)
    
    def __repr__(self):
        return f'<User {self.username}>'

class Camera(db.Model):
    """Camera model for tracking deployed wildlife cameras."""
    __tablename__ = 'cameras'
    
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), nullable=False)
    device_id = db.Column(db.String(50), unique=True, nullable=False)
    location = db.Column(db.String(200), nullable=False)
    latitude = db.Column(db.Float)
    longitude = db.Column(db.Float)
    altitude = db.Column(db.Float)
    
    # Status tracking
    status = db.Column(db.String(20), default='offline')  # online, offline, maintenance
    last_seen = db.Column(db.DateTime)
    battery_level = db.Column(db.Integer)  # Percentage
    signal_strength = db.Column(db.Integer)  # dBm or percentage
    
    # Configuration
    configuration = db.Column(JSONB)
    firmware_version = db.Column(db.String(20))
    
    # Timestamps
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    
    # Foreign keys
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    
    # Relationships
    images = db.relationship('CameraImage', backref='camera', lazy=True, cascade='all, delete-orphan')
    alerts = db.relationship('Alert', backref='camera', lazy=True)
    
    def recent_detections_count(self, hours=24):
        """Get count of recent wildlife detections."""
        from datetime import timedelta
        since = datetime.utcnow() - timedelta(hours=hours)
        
        return WildlifeDetection.query.join(CameraImage).filter(
            CameraImage.camera_id == self.id,
            CameraImage.timestamp >= since
        ).count()
    
    def __repr__(self):
        return f'<Camera {self.name}>'

class Species(db.Model):
    """Species model for wildlife classification."""
    __tablename__ = 'species'
    
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), unique=True, nullable=False)
    scientific_name = db.Column(db.String(150))
    common_names = db.Column(JSONB)  # Array of alternative names
    conservation_status = db.Column(db.String(50))  # IUCN status
    description = db.Column(db.Text)
    
    # Classification hierarchy
    kingdom = db.Column(db.String(50))
    phylum = db.Column(db.String(50))
    class_name = db.Column('class', db.String(50))
    order = db.Column(db.String(50))
    family = db.Column(db.String(50))
    genus = db.Column(db.String(50))
    
    # Behavioral and ecological data
    habitat_types = db.Column(JSONB)
    activity_patterns = db.Column(JSONB)  # diurnal, nocturnal, crepuscular
    diet_type = db.Column(db.String(50))
    average_size = db.Column(db.String(100))
    
    # Research and conservation data
    is_endangered = db.Column(db.Boolean, default=False)
    is_protected = db.Column(db.Boolean, default=False)
    research_priority = db.Column(db.String(20))  # high, medium, low
    
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    # Relationships
    detections = db.relationship('WildlifeDetection', backref='species', lazy=True)
    
    def __repr__(self):
        return f'<Species {self.name}>'

class CameraImage(db.Model):
    """Model for storing camera image metadata."""
    __tablename__ = 'camera_images'
    
    id = db.Column(db.Integer, primary_key=True)
    filename = db.Column(db.String(255), nullable=False)
    filepath = db.Column(db.String(500), nullable=False)
    file_size = db.Column(db.BigInteger)
    
    # Image metadata
    width = db.Column(db.Integer)
    height = db.Column(db.Integer)
    format = db.Column(db.String(10))
    
    # Camera metadata from ESP32
    metadata = db.Column(JSONB)  # Motion trigger, weather, battery level, etc.
    
    # Processing status
    processed = db.Column(db.Boolean, default=False)
    processing_started_at = db.Column(db.DateTime)
    processing_completed_at = db.Column(db.DateTime)
    processing_error = db.Column(db.Text)
    
    # Timestamps
    timestamp = db.Column(db.DateTime, default=datetime.utcnow)  # When captured
    uploaded_at = db.Column(db.DateTime, default=datetime.utcnow)  # When uploaded
    
    # Foreign keys
    camera_id = db.Column(db.Integer, db.ForeignKey('cameras.id'), nullable=False)
    
    # Relationships
    detections = db.relationship('WildlifeDetection', backref='image', lazy=True, cascade='all, delete-orphan')
    
    def __repr__(self):
        return f'<CameraImage {self.filename}>'

class WildlifeDetection(db.Model):
    """Model for wildlife detection results."""
    __tablename__ = 'wildlife_detections'
    
    id = db.Column(db.Integer, primary_key=True)
    
    # Detection results
    confidence = db.Column(db.Float, nullable=False)
    bounding_box = db.Column(JSONB)  # {x, y, width, height}
    
    # Additional AI analysis
    behavior_classification = db.Column(db.String(50))  # feeding, resting, alert, etc.
    group_size = db.Column(db.Integer, default=1)
    age_class = db.Column(db.String(20))  # adult, juvenile, unknown
    sex = db.Column(db.String(10))  # male, female, unknown
    
    # Verification and validation
    verified = db.Column(db.Boolean, default=False)
    verified_by = db.Column(db.Integer, db.ForeignKey('users.id'))
    verification_notes = db.Column(db.Text)
    
    # Model information
    model_version = db.Column(db.String(20))
    detection_method = db.Column(db.String(50))  # megadetector, custom_model, manual
    
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    # Foreign keys
    image_id = db.Column(db.Integer, db.ForeignKey('camera_images.id'), nullable=False)
    species_id = db.Column(db.Integer, db.ForeignKey('species.id'))
    
    # Relationships
    verifier = db.relationship('User', foreign_keys=[verified_by])
    
    def __repr__(self):
        return f'<WildlifeDetection {self.species.name if self.species else "Unknown"}>'

class Alert(db.Model):
    """Model for system alerts and notifications."""
    __tablename__ = 'alerts'
    
    id = db.Column(db.Integer, primary_key=True)
    type = db.Column(db.String(50), nullable=False)  # species_detection, system_error, low_battery
    severity = db.Column(db.String(20), default='info')  # critical, warning, info
    title = db.Column(db.String(200), nullable=False)
    message = db.Column(db.Text, nullable=False)
    
    # Alert data
    data = db.Column(JSONB)  # Additional alert-specific data
    
    # Status tracking
    read = db.Column(db.Boolean, default=False)
    acknowledged = db.Column(db.Boolean, default=False)
    resolved = db.Column(db.Boolean, default=False)
    
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    resolved_at = db.Column(db.DateTime)
    
    # Foreign keys
    camera_id = db.Column(db.Integer, db.ForeignKey('cameras.id'))
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    detection_id = db.Column(db.Integer, db.ForeignKey('wildlife_detections.id'))
    
    # Relationships
    detection = db.relationship('WildlifeDetection', backref='alerts')
    
    def __repr__(self):
        return f'<Alert {self.title}>'

class Analytics(db.Model):
    """Model for storing analytics data."""
    __tablename__ = 'analytics'
    
    id = db.Column(db.Integer, primary_key=True)
    
    # Analytics metadata
    metric_name = db.Column(db.String(100), nullable=False)
    metric_type = db.Column(db.String(50), nullable=False)  # count, average, percentage
    time_period = db.Column(db.String(20))  # hourly, daily, weekly, monthly
    
    # Data
    value = db.Column(db.Float, nullable=False)
    data = db.Column(JSONB)  # Additional metric data
    
    # Time information
    period_start = db.Column(db.DateTime, nullable=False)
    period_end = db.Column(db.DateTime, nullable=False)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    # Foreign keys (optional - for camera-specific metrics)
    camera_id = db.Column(db.Integer, db.ForeignKey('cameras.id'))
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'))
    
    def __repr__(self):
        return f'<Analytics {self.metric_name}: {self.value}>'

class SystemConfig(db.Model):
    """Model for system configuration settings."""
    __tablename__ = 'system_config'
    
    id = db.Column(db.Integer, primary_key=True)
    key = db.Column(db.String(100), unique=True, nullable=False)
    value = db.Column(JSONB, nullable=False)
    description = db.Column(db.Text)
    
    # Metadata
    category = db.Column(db.String(50))  # detection, alerts, system
    data_type = db.Column(db.String(20))  # string, number, boolean, object
    
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    updated_by = db.Column(db.Integer, db.ForeignKey('users.id'))
    
    def __repr__(self):
        return f'<SystemConfig {self.key}>'

class ResearchProject(db.Model):
    """Model for research project management."""
    __tablename__ = 'research_projects'
    
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(200), nullable=False)
    description = db.Column(db.Text)
    
    # Project details
    principal_investigator = db.Column(db.String(100))
    institution = db.Column(db.String(200))
    funding_source = db.Column(db.String(200))
    
    # Study parameters
    study_area = db.Column(db.String(200))
    target_species = db.Column(JSONB)  # Array of species IDs
    study_objectives = db.Column(JSONB)
    
    # Timeline
    start_date = db.Column(db.Date)
    end_date = db.Column(db.Date)
    status = db.Column(db.String(20), default='active')  # active, completed, suspended
    
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    # Foreign keys
    owner_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    
    def __repr__(self):
        return f'<ResearchProject {self.name}>'