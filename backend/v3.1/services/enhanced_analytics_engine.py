"""
Enhanced Analytics Engine for ESP32 Wildlife Camera v3.1
Implements advanced ML-based analytics including biodiversity metrics,
activity patterns, anomaly detection, and population trends.
"""

import logging
import numpy as np
import pandas as pd
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple, Any
from sqlalchemy import func, and_, or_, text
from scipy import stats
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
from sklearn.cluster import DBSCAN
import warnings
warnings.filterwarnings('ignore')

# Import models
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), '../../'))
from models import db, Camera, CameraImage, WildlifeDetection, Species, Analytics, User, Alert
from v3_1.models.multi_tenant_models import Organization, EnhancedCamera, EnhancedSpecies

logger = logging.getLogger(__name__)

class BiodiversityAnalyzer:
    """Analyzer for biodiversity metrics using ecological indices"""
    
    @staticmethod
    def calculate_shannon_index(species_counts: Dict[str, int]) -> float:
        """Calculate Shannon Diversity Index"""
        if not species_counts:
            return 0.0
        
        total = sum(species_counts.values())
        if total == 0:
            return 0.0
        
        shannon = 0.0
        for count in species_counts.values():
            if count > 0:
                proportion = count / total
                shannon -= proportion * np.log(proportion)
        
        return shannon
    
    @staticmethod
    def calculate_simpson_index(species_counts: Dict[str, int]) -> float:
        """Calculate Simpson Diversity Index"""
        if not species_counts:
            return 0.0
        
        total = sum(species_counts.values())
        if total <= 1:
            return 0.0
        
        simpson = 0.0
        for count in species_counts.values():
            if count > 0:
                simpson += count * (count - 1)
        
        simpson = simpson / (total * (total - 1))
        return 1 - simpson  # Simpson's Diversity Index (1 - D)
    
    @staticmethod
    def calculate_species_evenness(species_counts: Dict[str, int]) -> float:
        """Calculate Pielou's Evenness Index"""
        shannon = BiodiversityAnalyzer.calculate_shannon_index(species_counts)
        species_richness = len([count for count in species_counts.values() if count > 0])
        
        if species_richness <= 1:
            return 1.0
        
        max_shannon = np.log(species_richness)
        return shannon / max_shannon if max_shannon > 0 else 0.0

class ActivityPatternAnalyzer:
    """Analyzer for temporal and behavioral activity patterns"""
    
    @staticmethod
    def analyze_temporal_patterns(detections: List[WildlifeDetection]) -> Dict:
        """Analyze temporal activity patterns"""
        if not detections:
            return {}
        
        # Convert to DataFrame for easier analysis
        data = []
        for detection in detections:
            data.append({
                'species_id': detection.species_id,
                'species_name': detection.species.name if detection.species else 'Unknown',
                'datetime': detection.created_at,
                'hour': detection.created_at.hour,
                'day_of_week': detection.created_at.weekday(),
                'month': detection.created_at.month,
                'confidence': detection.confidence,
                'behavior': detection.behavior_classification or 'unknown'
            })
        
        df = pd.DataFrame(data)
        
        # Hourly activity distribution
        hourly_activity = df.groupby('hour').size().to_dict()
        
        # Peak activity hours
        top_hours = sorted(hourly_activity.items(), key=lambda x: x[1], reverse=True)[:3]
        peak_hours = [hour for hour, count in top_hours]
        
        # Activity type classification
        activity_type = ActivityPatternAnalyzer._classify_activity_type(hourly_activity)
        
        # Seasonal patterns
        seasonal_patterns = df.groupby('month').agg({
            'species_id': 'count',
            'confidence': 'mean'
        }).to_dict('index')
        
        # Behavioral patterns
        behavior_patterns = df.groupby('behavior').size().to_dict()
        
        return {
            'hourly_activity': [
                {'hour': hour, 'detection_count': hourly_activity.get(hour, 0)}
                for hour in range(24)
            ],
            'peak_activity_hours': peak_hours,
            'activity_type': activity_type,
            'seasonal_patterns': [
                {
                    'month': month,
                    'detection_count': data['species_id'],
                    'avg_confidence': data['confidence']
                }
                for month, data in seasonal_patterns.items()
            ],
            'behavior_patterns': behavior_patterns
        }
    
    @staticmethod
    def _classify_activity_type(hourly_activity: Dict[int, int]) -> str:
        """Classify activity type based on hourly distribution"""
        if not hourly_activity:
            return 'unknown'
        
        # Calculate activity percentages for different periods
        dawn_hours = [5, 6, 7]  # 5-7 AM
        day_hours = [8, 9, 10, 11, 12, 13, 14, 15, 16, 17]  # 8 AM - 5 PM
        dusk_hours = [18, 19, 20]  # 6-8 PM
        night_hours = [21, 22, 23, 0, 1, 2, 3, 4]  # 9 PM - 4 AM
        
        total_activity = sum(hourly_activity.values())
        if total_activity == 0:
            return 'unknown'
        
        dawn_activity = sum(hourly_activity.get(h, 0) for h in dawn_hours) / total_activity
        day_activity = sum(hourly_activity.get(h, 0) for h in day_hours) / total_activity
        dusk_activity = sum(hourly_activity.get(h, 0) for h in dusk_hours) / total_activity
        night_activity = sum(hourly_activity.get(h, 0) for h in night_hours) / total_activity
        
        # Classification thresholds
        if day_activity > 0.6:
            return 'diurnal'
        elif night_activity > 0.6:
            return 'nocturnal'
        elif (dawn_activity + dusk_activity) > 0.5:
            return 'crepuscular'
        else:
            return 'cathemeral'

class AnomalyDetector:
    """ML-based anomaly detection for wildlife patterns"""
    
    def __init__(self):
        self.isolation_forest = IsolationForest(contamination=0.1, random_state=42)
        self.scaler = StandardScaler()
    
    def detect_temporal_anomalies(self, detections: List[WildlifeDetection]) -> List[Dict]:
        """Detect temporal anomalies in detection patterns"""
        if len(detections) < 10:  # Need minimum data for anomaly detection
            return []
        
        # Prepare time series data
        df = pd.DataFrame([{
            'datetime': d.created_at,
            'species_id': d.species_id or 0,
            'confidence': d.confidence,
            'hour': d.created_at.hour,
            'day_of_week': d.created_at.weekday(),
            'day_of_year': d.created_at.timetuple().tm_yday
        } for d in detections])
        
        # Aggregate by day
        daily_stats = df.groupby(df['datetime'].dt.date).agg({
            'species_id': 'count',
            'confidence': 'mean',
            'hour': lambda x: x.mode().iloc[0] if len(x) > 0 else 12
        }).reset_index()
        
        if len(daily_stats) < 5:
            return []
        
        # Feature engineering
        features = daily_stats[['species_id', 'confidence', 'hour']].values
        features_scaled = self.scaler.fit_transform(features)
        
        # Detect anomalies
        anomaly_scores = self.isolation_forest.fit_predict(features_scaled)
        
        anomalies = []
        for i, (_, row) in enumerate(daily_stats.iterrows()):
            if anomaly_scores[i] == -1:  # Anomaly detected
                anomalies.append({
                    'date': row['datetime'].isoformat(),
                    'type': 'temporal',
                    'severity': 'medium',
                    'description': f'Unusual detection pattern: {row["species_id"]} detections with {row["confidence"]:.2f} avg confidence',
                    'confidence': 0.7,
                    'metadata': {
                        'detection_count': int(row['species_id']),
                        'avg_confidence': float(row['confidence']),
                        'peak_hour': int(row['hour'])
                    }
                })
        
        return anomalies
    
    def detect_species_anomalies(self, detections: List[WildlifeDetection]) -> List[Dict]:
        """Detect anomalies in species detection patterns"""
        if len(detections) < 20:
            return []
        
        # Group by species and calculate statistics
        species_stats = {}
        for detection in detections:
            species_name = detection.species.name if detection.species else 'Unknown'
            if species_name not in species_stats:
                species_stats[species_name] = {
                    'detections': [],
                    'confidences': [],
                    'locations': []
                }
            
            species_stats[species_name]['detections'].append(detection)
            species_stats[species_name]['confidences'].append(detection.confidence)
        
        anomalies = []
        
        # Detect confidence anomalies
        for species_name, stats in species_stats.items():
            if len(stats['confidences']) < 5:
                continue
            
            confidences = np.array(stats['confidences'])
            z_scores = np.abs(stats.zscore(confidences))
            
            # Flag detections with unusually low confidence
            low_confidence_threshold = 2.0  # 2 standard deviations
            low_confidence_indices = np.where(z_scores > low_confidence_threshold)[0]
            
            for idx in low_confidence_indices:
                detection = stats['detections'][idx]
                if detection.confidence < 0.3:  # Very low confidence
                    anomalies.append({
                        'id': len(anomalies) + 1,
                        'type': 'species_confidence',
                        'severity': 'high' if detection.confidence < 0.2 else 'medium',
                        'description': f'Unusually low confidence detection for {species_name}',
                        'detected_at': detection.created_at.isoformat(),
                        'species_id': detection.species_id,
                        'confidence': 0.8,
                        'metadata': {
                            'detection_confidence': detection.confidence,
                            'species_name': species_name,
                            'avg_confidence': np.mean(confidences)
                        }
                    })
        
        return anomalies

class PopulationTrendAnalyzer:
    """Analyzer for population trends and forecasting"""
    
    @staticmethod
    def analyze_population_trends(detections: List[WildlifeDetection], 
                                 time_window_days: int = 90) -> List[Dict]:
        """Analyze population trends using time series analysis"""
        if not detections:
            return []
        
        # Group detections by species and time
        species_data = {}
        for detection in detections:
            species_name = detection.species.name if detection.species else 'Unknown'
            date = detection.created_at.date()
            
            if species_name not in species_data:
                species_data[species_name] = {}
            
            if date not in species_data[species_name]:
                species_data[species_name][date] = 0
            
            species_data[species_name][date] += 1
        
        trends = []
        
        for species_name, date_counts in species_data.items():
            if len(date_counts) < 10:  # Need minimum data points
                continue
            
            # Create time series
            dates = sorted(date_counts.keys())
            counts = [date_counts[date] for date in dates]
            
            # Calculate trend using linear regression
            x = np.arange(len(dates))
            slope, intercept, r_value, p_value, std_err = stats.linregress(x, counts)
            
            # Determine trend direction
            if abs(slope) < 0.01:  # Very small slope
                trend_direction = 'stable'
                trend_percentage = 0.0
            elif slope > 0:
                trend_direction = 'increasing'
                trend_percentage = (slope * len(dates) / np.mean(counts)) * 100
            else:
                trend_direction = 'decreasing'
                trend_percentage = abs((slope * len(dates) / np.mean(counts)) * 100)
            
            # Confidence level based on R-squared and p-value
            confidence_level = min(r_value**2 * 100, 90)  # Max 90% confidence
            if p_value > 0.05:
                confidence_level *= 0.5  # Reduce confidence for non-significant trends
            
            # Generate forecast (simple linear projection)
            forecast_days = 30
            forecast_data = []
            for i in range(forecast_days):
                future_x = len(dates) + i
                predicted_count = max(0, slope * future_x + intercept)
                confidence_interval = {
                    'lower': max(0, predicted_count - 1.96 * std_err),
                    'upper': predicted_count + 1.96 * std_err
                }
                
                forecast_data.append({
                    'date': (dates[-1] + timedelta(days=i+1)).isoformat(),
                    'predicted_count': predicted_count,
                    'confidence_interval': confidence_interval
                })
            
            trends.append({
                'species_id': next((d.species_id for d in detections 
                                  if d.species and d.species.name == species_name), None),
                'species_name': species_name,
                'trend_direction': trend_direction,
                'trend_percentage': round(trend_percentage, 2),
                'confidence_level': round(confidence_level, 2),
                'forecast_data': forecast_data,
                'analysis_period': {
                    'start_date': dates[0].isoformat(),
                    'end_date': dates[-1].isoformat()
                },
                'statistical_measures': {
                    'slope': slope,
                    'r_squared': r_value**2,
                    'p_value': p_value,
                    'mean_detections': np.mean(counts),
                    'std_detections': np.std(counts)
                }
            })
        
        return trends

class ConservationAlertSystem:
    """System for generating conservation-focused alerts"""
    
    @staticmethod
    def generate_conservation_alerts(species_trends: List[Dict], 
                                   species_data: List[EnhancedSpecies]) -> List[Dict]:
        """Generate conservation alerts based on trends and species status"""
        alerts = []
        
        # Create species lookup
        species_lookup = {s.id: s for s in species_data}
        
        for trend in species_trends:
            species_id = trend.get('species_id')
            if not species_id or species_id not in species_lookup:
                continue
            
            species = species_lookup[species_id]
            
            # Alert for declining endangered species
            if (species.is_endangered and 
                trend['trend_direction'] == 'decreasing' and 
                trend['confidence_level'] > 60):
                
                alerts.append({
                    'id': len(alerts) + 1,
                    'species_id': species_id,
                    'species_name': species.name,
                    'alert_type': 'population_decline',
                    'severity': 'critical',
                    'description': f'Declining population trend detected for endangered species {species.name}',
                    'recommended_actions': [
                        'Increase monitoring frequency',
                        'Review habitat conditions',
                        'Consider intervention measures',
                        'Notify conservation authorities'
                    ],
                    'created_at': datetime.utcnow().isoformat(),
                    'trend_data': trend
                })
            
            # Alert for unusual patterns in protected species
            elif (species.is_protected and 
                  trend['trend_direction'] == 'decreasing' and 
                  trend['trend_percentage'] > 30):
                
                alerts.append({
                    'id': len(alerts) + 1,
                    'species_id': species_id,
                    'species_name': species.name,
                    'alert_type': 'behavioral_anomaly',
                    'severity': 'high',
                    'description': f'Significant decline in {species.name} detections',
                    'recommended_actions': [
                        'Investigate environmental changes',
                        'Check camera functionality',
                        'Review seasonal patterns',
                        'Consult with wildlife experts'
                    ],
                    'created_at': datetime.utcnow().isoformat(),
                    'trend_data': trend
                })
        
        return alerts

class EnhancedAnalyticsEngine:
    """Main analytics engine combining all analyzers"""
    
    def __init__(self):
        self.biodiversity_analyzer = BiodiversityAnalyzer()
        self.activity_analyzer = ActivityPatternAnalyzer()
        self.anomaly_detector = AnomalyDetector()
        self.trend_analyzer = PopulationTrendAnalyzer()
        self.conservation_system = ConservationAlertSystem()
        self.logger = logging.getLogger(__name__)
    
    def generate_comprehensive_analytics(self, 
                                       organization_id: Optional[int] = None,
                                       camera_ids: Optional[List[int]] = None,
                                       start_date: Optional[datetime] = None,
                                       end_date: Optional[datetime] = None) -> Dict[str, Any]:
        """Generate comprehensive analytics report"""
        
        # Set date range
        if not end_date:
            end_date = datetime.utcnow()
        if not start_date:
            start_date = end_date - timedelta(days=30)
        
        try:
            # Build query for detections
            query = db.session.query(WildlifeDetection).join(CameraImage).join(Camera)
            
            # Apply filters
            query = query.filter(CameraImage.timestamp >= start_date)
            query = query.filter(CameraImage.timestamp <= end_date)
            
            if organization_id:
                query = query.filter(Camera.organization_id == organization_id)
            
            if camera_ids:
                query = query.filter(Camera.id.in_(camera_ids))
            
            detections = query.all()
            
            # Get species data
            species_query = db.session.query(EnhancedSpecies)
            if organization_id:
                # Filter species relevant to organization's detections
                detected_species_ids = [d.species_id for d in detections if d.species_id]
                if detected_species_ids:
                    species_query = species_query.filter(EnhancedSpecies.id.in_(detected_species_ids))
            
            species_data = species_query.all()
            
            # Calculate biodiversity metrics
            species_counts = {}
            for detection in detections:
                species_name = detection.species.name if detection.species else 'Unknown'
                species_counts[species_name] = species_counts.get(species_name, 0) + 1
            
            biodiversity_metrics = {
                'shannon_index': self.biodiversity_analyzer.calculate_shannon_index(species_counts),
                'simpson_index': self.biodiversity_analyzer.calculate_simpson_index(species_counts),
                'species_richness': len([count for count in species_counts.values() if count > 0]),
                'species_evenness': self.biodiversity_analyzer.calculate_species_evenness(species_counts),
                'dominant_species': sorted(species_counts.items(), key=lambda x: x[1], reverse=True)[:3],
                'rare_species': [species for species, count in species_counts.items() if count == 1]
            }
            
            # Analyze activity patterns
            activity_patterns = self.activity_analyzer.analyze_temporal_patterns(detections)
            
            # Detect anomalies
            temporal_anomalies = self.anomaly_detector.detect_temporal_anomalies(detections)
            species_anomalies = self.anomaly_detector.detect_species_anomalies(detections)
            
            # Analyze population trends
            population_trends = self.trend_analyzer.analyze_population_trends(detections)
            
            # Generate conservation alerts
            conservation_alerts = self.conservation_system.generate_conservation_alerts(
                population_trends, species_data
            )
            
            return {
                'analysis_period': {
                    'start_date': start_date.isoformat(),
                    'end_date': end_date.isoformat(),
                    'total_days': (end_date - start_date).days
                },
                'summary': {
                    'total_detections': len(detections),
                    'unique_species': len(species_counts),
                    'cameras_active': len(set(d.image.camera_id for d in detections)),
                    'anomalies_detected': len(temporal_anomalies) + len(species_anomalies),
                    'conservation_alerts': len(conservation_alerts)
                },
                'biodiversity_metrics': biodiversity_metrics,
                'activity_patterns': activity_patterns,
                'anomalies': {
                    'temporal': temporal_anomalies,
                    'species': species_anomalies
                },
                'population_trends': population_trends,
                'conservation_alerts': conservation_alerts,
                'species_breakdown': [
                    {
                        'species_name': species_name,
                        'detection_count': count,
                        'percentage': (count / len(detections)) * 100 if detections else 0
                    }
                    for species_name, count in sorted(species_counts.items(), 
                                                    key=lambda x: x[1], reverse=True)
                ]
            }
            
        except Exception as e:
            self.logger.error(f"Error generating analytics: {str(e)}")
            raise e
    
    def get_real_time_insights(self, organization_id: Optional[int] = None) -> Dict[str, Any]:
        """Get real-time insights for dashboard display"""
        try:
            # Get recent detections (last 24 hours)
            start_time = datetime.utcnow() - timedelta(hours=24)
            
            query = db.session.query(WildlifeDetection).join(CameraImage).join(Camera)
            query = query.filter(CameraImage.timestamp >= start_time)
            
            if organization_id:
                query = query.filter(Camera.organization_id == organization_id)
            
            recent_detections = query.all()
            
            # Quick biodiversity calculation
            species_counts = {}
            for detection in recent_detections:
                species_name = detection.species.name if detection.species else 'Unknown'
                species_counts[species_name] = species_counts.get(species_name, 0) + 1
            
            return {
                'timestamp': datetime.utcnow().isoformat(),
                'recent_detections': len(recent_detections),
                'active_species': len(species_counts),
                'biodiversity_index': self.biodiversity_analyzer.calculate_shannon_index(species_counts),
                'top_species': sorted(species_counts.items(), key=lambda x: x[1], reverse=True)[:5],
                'activity_level': 'high' if len(recent_detections) > 10 else 'moderate' if len(recent_detections) > 3 else 'low'
            }
            
        except Exception as e:
            self.logger.error(f"Error generating real-time insights: {str(e)}")
            return {
                'timestamp': datetime.utcnow().isoformat(),
                'error': 'Failed to generate insights'
            }