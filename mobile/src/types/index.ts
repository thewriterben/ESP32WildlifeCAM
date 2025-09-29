// Core types for ESP32 Wildlife CAM Mobile App v3.1

export interface User {
  id: number;
  username: string;
  email: string;
  role: 'admin' | 'manager' | 'researcher' | 'viewer';
  organizationId?: number;
  created_at: string;
  last_login?: string;
  is_active: boolean;
}

export interface Organization {
  id: number;
  name: string;
  description?: string;
  subscription_tier: 'basic' | 'professional' | 'enterprise';
  settings: OrganizationSettings;
  created_at: string;
  updated_at: string;
}

export interface OrganizationSettings {
  max_cameras: number;
  analytics_retention_days: number;
  enable_ai_features: boolean;
  custom_branding: boolean;
  api_access_enabled: boolean;
}

export interface Camera {
  id: number;
  name: string;
  device_id: string;
  location: string;
  latitude?: number;
  longitude?: number;
  altitude?: number;
  status: 'online' | 'offline' | 'maintenance';
  last_seen?: string;
  battery_level?: number;
  signal_strength?: number;
  configuration: CameraConfiguration;
  firmware_version?: string;
  user_id: number;
  organization_id?: number;
  created_at: string;
  updated_at: string;
}

export interface CameraConfiguration {
  resolution: string;
  frame_rate: number;
  motion_sensitivity: number;
  capture_mode: 'motion' | 'scheduled' | 'manual';
  night_mode: boolean;
  burst_mode: boolean;
  burst_count: number;
  ai_detection_enabled: boolean;
  notification_enabled: boolean;
}

export interface Species {
  id: number;
  name: string;
  scientific_name?: string;
  common_names?: string[];
  conservation_status?: string;
  description?: string;
  kingdom?: string;
  phylum?: string;
  class_name?: string;
  order?: string;
  family?: string;
  genus?: string;
  habitat_types?: string[];
  activity_patterns?: string[];
  diet_type?: string;
  average_size?: string;
  is_endangered: boolean;
  is_protected: boolean;
  research_priority?: 'high' | 'medium' | 'low';
  created_at: string;
}

export interface CameraImage {
  id: number;
  filename: string;
  filepath: string;
  file_size: number;
  width: number;
  height: number;
  format: string;
  metadata?: ImageMetadata;
  processed: boolean;
  processing_started_at?: string;
  processing_completed_at?: string;
  processing_error?: string;
  timestamp: string;
  uploaded_at: string;
  camera_id: number;
  detections: WildlifeDetection[];
}

export interface ImageMetadata {
  motion_trigger: boolean;
  weather_data?: WeatherData;
  battery_level?: number;
  signal_strength?: number;
  camera_settings?: Partial<CameraConfiguration>;
}

export interface WeatherData {
  temperature: number;
  humidity: number;
  pressure?: number;
  wind_speed?: number;
  wind_direction?: number;
  visibility?: number;
}

export interface WildlifeDetection {
  id: number;
  confidence: number;
  bounding_box: BoundingBox;
  behavior_classification?: string;
  group_size: number;
  age_class?: 'adult' | 'juvenile' | 'unknown';
  sex?: 'male' | 'female' | 'unknown';
  verified: boolean;
  verified_by?: number;
  verification_notes?: string;
  model_version: string;
  detection_method: string;
  created_at: string;
  image_id: number;
  species_id?: number;
  species?: Species;
}

export interface BoundingBox {
  x: number;
  y: number;
  width: number;
  height: number;
}

export interface Alert {
  id: number;
  type: 'species_detection' | 'system_error' | 'low_battery' | 'conservation_alert';
  severity: 'critical' | 'warning' | 'info';
  title: string;
  message: string;
  data?: any;
  read: boolean;
  acknowledged: boolean;
  resolved: boolean;
  created_at: string;
  resolved_at?: string;
  camera_id?: number;
  user_id: number;
  detection_id?: number;
  camera?: Camera;
  detection?: WildlifeDetection;
}

export interface Analytics {
  id: number;
  metric_name: string;
  metric_type: 'count' | 'average' | 'percentage';
  time_period: 'hourly' | 'daily' | 'weekly' | 'monthly';
  value: number;
  data?: any;
  period_start: string;
  period_end: string;
  created_at: string;
  camera_id?: number;
  user_id?: number;
}

export interface DashboardData {
  summary: {
    total_cameras: number;
    online_cameras: number;
    total_detections: number;
    recent_detections: number;
    species_count: number;
    conservation_alerts: number;
  };
  recent_detections: WildlifeDetection[];
  camera_status: Camera[];
  analytics: Analytics[];
  alerts: Alert[];
}

export interface BiodiversityMetrics {
  shannon_index: number;
  simpson_index: number;
  species_richness: number;
  species_evenness: number;
  dominant_species: Species[];
  rare_species: Species[];
  temporal_diversity: TemporalDiversity[];
}

export interface TemporalDiversity {
  period: string;
  shannon_index: number;
  species_count: number;
  detection_count: number;
}

export interface ActivityPattern {
  species_id: number;
  species_name: string;
  hourly_activity: HourlyActivity[];
  peak_activity_hours: number[];
  activity_type: 'diurnal' | 'nocturnal' | 'crepuscular' | 'cathemeral';
  seasonal_patterns: SeasonalPattern[];
}

export interface HourlyActivity {
  hour: number;
  detection_count: number;
  confidence_avg: number;
}

export interface SeasonalPattern {
  month: number;
  detection_count: number;
  avg_group_size: number;
  behavior_patterns: string[];
}

export interface PopulationTrend {
  species_id: number;
  species_name: string;
  trend_direction: 'increasing' | 'decreasing' | 'stable' | 'unknown';
  trend_percentage: number;
  confidence_level: number;
  forecast_data: ForecastPoint[];
  analysis_period: {
    start_date: string;
    end_date: string;
  };
}

export interface ForecastPoint {
  date: string;
  predicted_count: number;
  confidence_interval: {
    lower: number;
    upper: number;
  };
}

export interface ConservationAlert {
  id: number;
  species_id: number;
  species_name: string;
  alert_type: 'population_decline' | 'habitat_change' | 'behavioral_anomaly';
  severity: 'critical' | 'high' | 'medium';
  description: string;
  recommended_actions: string[];
  created_at: string;
  camera_locations: string[];
}

// Navigation types
export type RootStackParamList = {
  Login: undefined;
  Dashboard: undefined;
  CameraList: undefined;
  CameraDetail: {cameraId: number};
  SpeciesGallery: undefined;
  SpeciesDetail: {speciesId: number};
  ImageGallery: {cameraId?: number};
  ImageDetail: {imageId: number};
  Analytics: undefined;
  Settings: undefined;
  Profile: undefined;
  Notifications: undefined;
  CameraConfiguration: {cameraId: number};
  OrganizationSettings: undefined;
};

// API Response types
export interface ApiResponse<T> {
  data: T;
  message: string;
  success: boolean;
  timestamp: string;
}

export interface PaginatedResponse<T> {
  data: T[];
  pagination: {
    page: number;
    limit: number;
    total: number;
    total_pages: number;
  };
}

// WebSocket message types
export interface WebSocketMessage {
  type: string;
  data: any;
  timestamp: string;
}

export interface RealtimeDetection {
  detection: WildlifeDetection;
  image: CameraImage;
  camera: Camera;
}

export interface SystemStatus {
  camera_id: number;
  status: Camera['status'];
  battery_level?: number;
  signal_strength?: number;
  last_heartbeat: string;
}

// Local storage types
export interface CachedData {
  cameras: Camera[];
  recent_images: CameraImage[];
  species_library: Species[];
  user_settings: UserSettings;
  last_sync: string;
}

export interface UserSettings {
  notifications_enabled: boolean;
  push_notifications: boolean;
  email_notifications: boolean;
  offline_sync: boolean;
  image_quality: 'high' | 'medium' | 'low';
  auto_sync: boolean;
  dark_mode: boolean;
  language: string;
}

// Authentication types
export interface AuthState {
  user: User | null;
  organization: Organization | null;
  token: string | null;
  isAuthenticated: boolean;
  isLoading: boolean;
  error: string | null;
}

export interface LoginCredentials {
  email: string;
  password: string;
  organization_code?: string;
}

export interface RegisterData {
  username: string;
  email: string;
  password: string;
  organization_code?: string;
  role?: string;
}