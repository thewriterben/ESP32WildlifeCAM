"""
Enhanced database models for ESP32 Wildlife Camera Backend v3.1
Implements multi-tenant architecture with Row Level Security (RLS)
"""

from flask_sqlalchemy import SQLAlchemy
from datetime import datetime
import json
from sqlalchemy.dialects.postgresql import UUID, JSONB
from sqlalchemy import text, event
import uuid
from enum import Enum
from sqlalchemy.ext.declarative import declared_attr

# Import base models from existing system
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), '../../'))
from models import db

class OrganizationSubscriptionTier(Enum):
    BASIC = "basic"
    PROFESSIONAL = "professional"
    ENTERPRISE = "enterprise"

class UserRole(Enum):
    ADMIN = "admin"
    MANAGER = "manager"
    RESEARCHER = "researcher"
    VIEWER = "viewer"

class MultiTenantMixin:
    """Base mixin for multi-tenant models with organization_id"""
    
    @declared_attr
    def organization_id(cls):
        return db.Column(db.Integer, db.ForeignKey('organizations.id'), nullable=True, index=True)

class Organization(db.Model):
    """Enhanced organization model for multi-tenant support"""
    __tablename__ = 'organizations'
    
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(200), nullable=False)
    display_name = db.Column(db.String(200))
    description = db.Column(db.Text)
    
    # Organization identification
    code = db.Column(db.String(50), unique=True, nullable=False, index=True)
    domain = db.Column(db.String(100))  # email domain for auto-assignment
    
    # Subscription management
    subscription_tier = db.Column(db.Enum(OrganizationSubscriptionTier), 
                                 default=OrganizationSubscriptionTier.BASIC)
    subscription_status = db.Column(db.String(20), default='active')  # active, suspended, cancelled
    subscription_started_at = db.Column(db.DateTime, default=datetime.utcnow)
    subscription_expires_at = db.Column(db.DateTime)
    
    # Organization settings and limits
    settings = db.Column(JSONB, default=lambda: {
        'max_cameras': 5,
        'max_users': 10,
        'analytics_retention_days': 90,
        'enable_ai_features': True,
        'custom_branding': False,
        'api_access_enabled': True,
        'webhook_enabled': False,
        'export_enabled': True,
    })
    
    # Branding and customization
    branding = db.Column(JSONB, default=lambda: {
        'logo_url': None,
        'primary_color': '#2c5530',
        'secondary_color': '#4a7856',
        'custom_domain': None,
    })
    
    # Contact and billing information
    contact_email = db.Column(db.String(120))
    contact_name = db.Column(db.String(100))
    billing_address = db.Column(JSONB)
    
    # Status and metadata
    is_active = db.Column(db.Boolean, default=True)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    
    # Relationships
    users = db.relationship('User', backref='organization', lazy=True,
                           foreign_keys='User.organization_id')
    cameras = db.relationship('Camera', backref='organization', lazy=True,
                             foreign_keys='Camera.organization_id')
    invitations = db.relationship('OrganizationInvitation', backref='organization', lazy=True)
    subscriptions = db.relationship('OrganizationSubscription', backref='organization', lazy=True)
    
    def get_usage_stats(self):
        """Get current usage statistics for the organization"""
        return {
            'active_cameras': len([c for c in self.cameras if c.status == 'online']),
            'total_cameras': len(self.cameras),
            'active_users': len([u for u in self.users if u.is_active]),
            'total_users': len(self.users),
            'storage_used_mb': sum([c.get_storage_usage() for c in self.cameras]),
        }
    
    def is_within_limits(self, resource_type: str, requested_amount: int = 1):
        """Check if organization is within subscription limits"""
        current_usage = self.get_usage_stats()
        limits = self.settings
        
        if resource_type == 'cameras':
            return current_usage['total_cameras'] + requested_amount <= limits.get('max_cameras', 5)
        elif resource_type == 'users':
            return current_usage['total_users'] + requested_amount <= limits.get('max_users', 10)
        
        return True
    
    def __repr__(self):
        return f'<Organization {self.name}>'

class OrganizationInvitation(db.Model):
    """Model for managing organization invitations"""
    __tablename__ = 'organization_invitations'
    
    id = db.Column(db.Integer, primary_key=True)
    token = db.Column(db.String(255), unique=True, nullable=False, default=lambda: str(uuid.uuid4()))
    
    # Invitation details
    email = db.Column(db.String(120), nullable=False)
    role = db.Column(db.Enum(UserRole), default=UserRole.VIEWER)
    
    # Status tracking
    status = db.Column(db.String(20), default='pending')  # pending, accepted, expired, cancelled
    invited_by = db.Column(db.Integer, db.ForeignKey('users.id'))
    invited_at = db.Column(db.DateTime, default=datetime.utcnow)
    expires_at = db.Column(db.DateTime)
    accepted_at = db.Column(db.DateTime)
    accepted_by = db.Column(db.Integer, db.ForeignKey('users.id'))
    
    # Foreign keys
    organization_id = db.Column(db.Integer, db.ForeignKey('organizations.id'), nullable=False)
    
    # Relationships
    inviter = db.relationship('User', foreign_keys=[invited_by])
    accepter = db.relationship('User', foreign_keys=[accepted_by])
    
    def is_valid(self):
        """Check if invitation is still valid"""
        return (self.status == 'pending' and 
                self.expires_at and 
                self.expires_at > datetime.utcnow())
    
    def __repr__(self):
        return f'<OrganizationInvitation {self.email} -> {self.organization.name}>'

class OrganizationSubscription(db.Model):
    """Model for tracking organization subscription history"""
    __tablename__ = 'organization_subscriptions'
    
    id = db.Column(db.Integer, primary_key=True)
    
    # Subscription details
    tier = db.Column(db.Enum(OrganizationSubscriptionTier), nullable=False)
    status = db.Column(db.String(20), nullable=False)  # active, cancelled, expired
    
    # Billing information
    stripe_subscription_id = db.Column(db.String(255))
    amount = db.Column(db.Numeric(10, 2))
    currency = db.Column(db.String(3), default='USD')
    billing_interval = db.Column(db.String(20))  # monthly, yearly
    
    # Dates
    started_at = db.Column(db.DateTime, nullable=False)
    ends_at = db.Column(db.DateTime)
    cancelled_at = db.Column(db.DateTime)
    
    # Foreign keys
    organization_id = db.Column(db.Integer, db.ForeignKey('organizations.id'), nullable=False)
    
    def __repr__(self):
        return f'<OrganizationSubscription {self.organization.name} - {self.tier.value}>'

# Enhanced User model with multi-tenant support
class EnhancedUser(db.Model):
    """Enhanced user model with organization and role support"""
    __tablename__ = 'users_v31'
    
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    email = db.Column(db.String(120), unique=True, nullable=False)
    password_hash = db.Column(db.String(255), nullable=False)
    
    # Enhanced role and organization support
    role = db.Column(db.Enum(UserRole), default=UserRole.VIEWER)
    organization_id = db.Column(db.Integer, db.ForeignKey('organizations.id'), nullable=True)
    
    # Profile information
    first_name = db.Column(db.String(50))
    last_name = db.Column(db.String(50))
    title = db.Column(db.String(100))
    bio = db.Column(db.Text)
    avatar_url = db.Column(db.String(500))
    
    # Contact and notification preferences
    phone = db.Column(db.String(20))
    timezone = db.Column(db.String(50), default='UTC')
    language = db.Column(db.String(10), default='en')
    
    # Notification settings
    notification_preferences = db.Column(JSONB, default=lambda: {
        'email_notifications': True,
        'push_notifications': True,
        'wildlife_detections': True,
        'system_alerts': True,
        'conservation_alerts': True,
        'weekly_reports': True,
    })
    
    # Security and access
    two_factor_enabled = db.Column(db.Boolean, default=False)
    two_factor_secret = db.Column(db.String(32))
    last_login_ip = db.Column(db.String(45))
    login_count = db.Column(db.Integer, default=0)
    
    # Status and timestamps
    is_active = db.Column(db.Boolean, default=True)
    is_verified = db.Column(db.Boolean, default=False)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    last_login = db.Column(db.DateTime)
    last_seen = db.Column(db.DateTime)
    
    def get_full_name(self):
        """Get user's full name"""
        if self.first_name and self.last_name:
            return f"{self.first_name} {self.last_name}"
        return self.username
    
    def has_permission(self, permission: str):
        """Check if user has specific permission based on role"""
        role_permissions = {
            UserRole.ADMIN: ['*'],  # All permissions
            UserRole.MANAGER: [
                'cameras.read', 'cameras.write', 'cameras.delete',
                'users.read', 'users.invite',
                'analytics.read', 'analytics.export',
                'images.read', 'images.verify',
                'species.read', 'alerts.read', 'alerts.manage'
            ],
            UserRole.RESEARCHER: [
                'cameras.read', 'cameras.write',
                'analytics.read', 'analytics.export',
                'images.read', 'images.verify',
                'species.read', 'alerts.read'
            ],
            UserRole.VIEWER: [
                'cameras.read', 'analytics.read',
                'images.read', 'species.read', 'alerts.read'
            ]
        }
        
        user_permissions = role_permissions.get(self.role, [])
        return '*' in user_permissions or permission in user_permissions
    
    def __repr__(self):
        return f'<EnhancedUser {self.username}>'

# Enhanced Camera model with organization support
class EnhancedCamera(MultiTenantMixin, db.Model):
    """Enhanced camera model with multi-tenant and advanced features"""
    __tablename__ = 'cameras_v31'
    
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), nullable=False)
    device_id = db.Column(db.String(50), unique=True, nullable=False)
    
    # Location and environment
    location = db.Column(db.String(200), nullable=False)
    latitude = db.Column(db.Float)
    longitude = db.Column(db.Float)
    altitude = db.Column(db.Float)
    timezone = db.Column(db.String(50), default='UTC')
    
    # Enhanced status tracking
    status = db.Column(db.String(20), default='offline')  # online, offline, maintenance, error
    health_score = db.Column(db.Float, default=100.0)  # 0-100 health indicator
    last_seen = db.Column(db.DateTime)
    last_heartbeat = db.Column(db.DateTime)
    
    # Power and connectivity
    battery_level = db.Column(db.Integer)  # Percentage
    charging_status = db.Column(db.String(20))  # charging, discharging, full
    solar_voltage = db.Column(db.Float)
    signal_strength = db.Column(db.Integer)  # dBm or percentage
    connection_type = db.Column(db.String(20))  # wifi, lora, cellular
    
    # Storage and performance
    storage_total_mb = db.Column(db.Integer)
    storage_used_mb = db.Column(db.Integer)
    temperature = db.Column(db.Float)
    humidity = db.Column(db.Float)
    
    # Enhanced configuration
    configuration = db.Column(JSONB, default=lambda: {
        'resolution': '1024x768',
        'frame_rate': 10,
        'motion_sensitivity': 50,
        'capture_mode': 'motion',
        'night_mode': True,
        'burst_mode': False,
        'burst_count': 3,
        'ai_detection_enabled': True,
        'notification_enabled': True,
        'schedule_enabled': False,
        'schedule': [],
        'image_quality': 80,
        'video_enabled': False,
        'audio_enabled': False,
    })
    
    # AI and detection settings
    ai_models_enabled = db.Column(JSONB, default=lambda: ['wildlife_v3', 'species_classifier'])
    detection_confidence_threshold = db.Column(db.Float, default=0.6)
    conservation_alerts_enabled = db.Column(db.Boolean, default=True)
    
    # Deployment information
    deployment_type = db.Column(db.String(50))  # permanent, temporary, mobile
    deployment_purpose = db.Column(db.String(100))  # research, monitoring, security
    deployment_notes = db.Column(db.Text)
    
    # Hardware information
    firmware_version = db.Column(db.String(20))
    hardware_version = db.Column(db.String(20))
    manufacturer = db.Column(db.String(50))
    model = db.Column(db.String(50))
    
    # Maintenance and support
    installation_date = db.Column(db.Date)
    last_maintenance = db.Column(db.Date)
    next_maintenance_due = db.Column(db.Date)
    warranty_expires = db.Column(db.Date)
    
    # Timestamps
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    
    # Foreign keys
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    
    def get_storage_usage(self):
        """Get storage usage in MB"""
        return self.storage_used_mb or 0
    
    def get_storage_percentage(self):
        """Get storage usage as percentage"""
        if not self.storage_total_mb:
            return 0
        return (self.storage_used_mb or 0) / self.storage_total_mb * 100
    
    def is_healthy(self):
        """Check if camera is in healthy state"""
        return (self.status == 'online' and 
                self.health_score >= 70 and
                (self.battery_level is None or self.battery_level > 20))
    
    def __repr__(self):
        return f'<EnhancedCamera {self.name}>'

# Enhanced Species model with conservation data
class EnhancedSpecies(db.Model):
    """Enhanced species model with detailed conservation and regional data"""
    __tablename__ = 'species_v31'
    
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), unique=True, nullable=False)
    scientific_name = db.Column(db.String(150))
    common_names = db.Column(JSONB)  # Multiple language support
    
    # Classification hierarchy
    kingdom = db.Column(db.String(50))
    phylum = db.Column(db.String(50))
    class_name = db.Column('class', db.String(50))
    order = db.Column(db.String(50))
    family = db.Column(db.String(50))
    genus = db.Column(db.String(50))
    species = db.Column(db.String(50))
    
    # Conservation status (IUCN Red List)
    conservation_status = db.Column(db.String(50))  # LC, NT, VU, EN, CR, EW, EX
    conservation_trend = db.Column(db.String(20))  # increasing, stable, decreasing, unknown
    population_estimate = db.Column(db.String(100))
    
    # Regional information
    regions = db.Column(JSONB)  # Regions where species is found
    native_regions = db.Column(JSONB)
    introduced_regions = db.Column(JSONB)
    
    # Behavioral and ecological data
    habitat_types = db.Column(JSONB)
    activity_patterns = db.Column(JSONB)  # diurnal, nocturnal, crepuscular, cathemeral
    diet_type = db.Column(db.String(50))
    diet_details = db.Column(JSONB)
    
    # Physical characteristics
    size_range = db.Column(JSONB)  # {min: 10, max: 30, unit: 'cm'}
    weight_range = db.Column(JSONB)
    lifespan = db.Column(JSONB)  # {min: 5, max: 15, unit: 'years'}
    
    # Breeding and reproduction
    breeding_season = db.Column(JSONB)
    gestation_period = db.Column(db.String(50))
    litter_size = db.Column(JSONB)
    
    # Research and monitoring data
    research_priority = db.Column(db.String(20))  # critical, high, medium, low
    research_notes = db.Column(db.Text)
    monitoring_difficulty = db.Column(db.String(20))  # easy, moderate, difficult
    
    # Detection and AI model information
    ai_model_accuracy = db.Column(db.Float)  # Model accuracy for this species
    detection_difficulty = db.Column(db.String(20))  # easy, moderate, difficult
    similar_species = db.Column(JSONB)  # List of species IDs that look similar
    
    # Status flags
    is_endangered = db.Column(db.Boolean, default=False)
    is_protected = db.Column(db.Boolean, default=False)
    is_endemic = db.Column(db.Boolean, default=False)
    is_invasive = db.Column(db.Boolean, default=False)
    
    # Media and documentation
    description = db.Column(db.Text)
    identification_notes = db.Column(db.Text)
    reference_image_urls = db.Column(JSONB)
    reference_sounds = db.Column(JSONB)
    external_links = db.Column(JSONB)
    
    # Timestamps
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    
    def get_conservation_urgency(self):
        """Get conservation urgency score based on status"""
        urgency_scores = {
            'CR': 100,  # Critically Endangered
            'EN': 80,   # Endangered
            'VU': 60,   # Vulnerable
            'NT': 40,   # Near Threatened
            'LC': 20,   # Least Concern
        }
        return urgency_scores.get(self.conservation_status, 0)
    
    def __repr__(self):
        return f'<EnhancedSpecies {self.name}>'

# Row Level Security setup
@event.listens_for(db.Model, 'after_configured', once=True)
def setup_row_level_security():
    """Setup Row Level Security policies for multi-tenant models"""
    
    # Enable RLS on tables
    tables_with_rls = [
        'cameras_v31',
        'users_v31',
        'camera_images',
        'wildlife_detections',
        'alerts',
        'analytics'
    ]
    
    for table in tables_with_rls:
        db.session.execute(text(f'ALTER TABLE {table} ENABLE ROW LEVEL SECURITY;'))
    
    # Create RLS policies
    # Policy for cameras - users can only see cameras in their organization
    db.session.execute(text('''
        CREATE POLICY cameras_organization_policy ON cameras_v31
        USING (organization_id = current_setting('app.current_organization_id')::int);
    '''))
    
    # Policy for users - users can only see other users in their organization
    db.session.execute(text('''
        CREATE POLICY users_organization_policy ON users_v31
        USING (organization_id = current_setting('app.current_organization_id')::int);
    '''))
    
    db.session.commit()

def set_organization_context(organization_id: int):
    """Set the current organization context for RLS"""
    db.session.execute(text('SET app.current_organization_id = :org_id'), {'org_id': organization_id})

def clear_organization_context():
    """Clear the organization context"""
    db.session.execute(text('RESET app.current_organization_id'))