"""
Analytics engine for ESP32 Wildlife Camera system.
Generates insights, reports, and metrics from wildlife detection data.
"""

import logging
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple
from sqlalchemy import func, and_, or_
from models import (
    db, Camera, CameraImage, WildlifeDetection, Species, 
    Analytics, User, Alert
)

logger = logging.getLogger(__name__)

class AnalyticsEngine:
    """
    Analytics engine for generating wildlife monitoring insights.
    """
    
    def __init__(self):
        self.logger = logging.getLogger(__name__)
    
    def generate_dashboard_analytics(self, user_id: int, start_date: datetime = None, 
                                   end_date: datetime = None) -> Dict:
        """
        Generate comprehensive analytics for dashboard display.
        
        Args:
            user_id: User ID to generate analytics for
            start_date: Start date for analytics period
            end_date: End date for analytics period
            
        Returns:
            Dictionary containing dashboard analytics
        """
        try:
            # Default to last 7 days if no dates provided
            if not end_date:
                end_date = datetime.utcnow()
            if not start_date:
                start_date = end_date - timedelta(days=7)
            
            analytics = {
                'period': {
                    'start_date': start_date.isoformat(),
                    'end_date': end_date.isoformat(),
                    'days': (end_date - start_date).days
                },
                'summary': self._generate_summary_stats(user_id, start_date, end_date),
                'species_breakdown': self._generate_species_breakdown(user_id, start_date, end_date),
                'detection_timeline': self._generate_detection_timeline(user_id, start_date, end_date),
                'camera_performance': self._generate_camera_performance(user_id, start_date, end_date),
                'activity_patterns': self._generate_activity_patterns(user_id, start_date, end_date),
                'conservation_alerts': self._generate_conservation_alerts(user_id, start_date, end_date),
                'system_health': self._generate_system_health(user_id)
            }
            
            return analytics
            
        except Exception as e:
            self.logger.error(f"Failed to generate dashboard analytics: {str(e)}")
            return {}
    
    def _generate_summary_stats(self, user_id: int, start_date: datetime, 
                               end_date: datetime) -> Dict:
        """Generate high-level summary statistics."""
        try:
            # Total cameras
            total_cameras = Camera.query.filter_by(user_id=user_id).count()
            
            # Active cameras (seen in last 24 hours)
            active_threshold = datetime.utcnow() - timedelta(hours=24)
            active_cameras = Camera.query.filter(
                Camera.user_id == user_id,
                Camera.last_seen >= active_threshold
            ).count()
            
            # Total images in period
            total_images = CameraImage.query.join(Camera).filter(
                Camera.user_id == user_id,
                CameraImage.timestamp >= start_date,
                CameraImage.timestamp <= end_date
            ).count()
            
            # Total detections in period
            total_detections = WildlifeDetection.query.join(CameraImage).join(Camera).filter(
                Camera.user_id == user_id,
                CameraImage.timestamp >= start_date,
                CameraImage.timestamp <= end_date
            ).count()
            
            # Unique species detected
            unique_species = db.session.query(func.count(func.distinct(WildlifeDetection.species_id))).join(
                CameraImage
            ).join(Camera).filter(
                Camera.user_id == user_id,
                CameraImage.timestamp >= start_date,
                CameraImage.timestamp <= end_date
            ).scalar()
            
            # Average detection confidence
            avg_confidence = db.session.query(func.avg(WildlifeDetection.confidence)).join(
                CameraImage
            ).join(Camera).filter(
                Camera.user_id == user_id,
                CameraImage.timestamp >= start_date,
                CameraImage.timestamp <= end_date
            ).scalar()
            
            return {
                'total_cameras': total_cameras,
                'active_cameras': active_cameras,
                'camera_health': round((active_cameras / total_cameras * 100) if total_cameras > 0 else 0, 1),
                'total_images': total_images,
                'total_detections': total_detections,
                'detection_rate': round((total_detections / total_images * 100) if total_images > 0 else 0, 1),
                'unique_species': unique_species or 0,
                'average_confidence': round(float(avg_confidence or 0), 2)
            }
            
        except Exception as e:
            self.logger.error(f"Failed to generate summary stats: {str(e)}")
            return {}
    
    def _generate_species_breakdown(self, user_id: int, start_date: datetime, 
                                   end_date: datetime) -> List[Dict]:
        """Generate species detection breakdown."""
        try:
            species_stats = db.session.query(
                Species.name,
                Species.scientific_name,
                Species.conservation_status,
                func.count(WildlifeDetection.id).label('detection_count'),
                func.avg(WildlifeDetection.confidence).label('avg_confidence'),
                func.max(CameraImage.timestamp).label('last_seen')
            ).join(WildlifeDetection).join(CameraImage).join(Camera).filter(
                Camera.user_id == user_id,
                CameraImage.timestamp >= start_date,
                CameraImage.timestamp <= end_date
            ).group_by(
                Species.id, Species.name, Species.scientific_name, Species.conservation_status
            ).order_by(func.count(WildlifeDetection.id).desc()).all()
            
            breakdown = []
            for stat in species_stats:
                breakdown.append({
                    'species': stat.name,
                    'scientific_name': stat.scientific_name,
                    'conservation_status': stat.conservation_status,
                    'detection_count': stat.detection_count,
                    'average_confidence': round(float(stat.avg_confidence), 2),
                    'last_seen': stat.last_seen.isoformat() if stat.last_seen else None,
                    'is_endangered': stat.conservation_status in ['Endangered', 'Critically Endangered'],
                    'is_protected': stat.conservation_status in ['Vulnerable', 'Endangered', 'Critically Endangered']
                })
            
            return breakdown
            
        except Exception as e:
            self.logger.error(f"Failed to generate species breakdown: {str(e)}")
            return []
    
    def _generate_detection_timeline(self, user_id: int, start_date: datetime, 
                                    end_date: datetime) -> List[Dict]:
        """Generate detection timeline data."""
        try:
            # Group detections by day
            timeline_data = db.session.query(
                func.date(CameraImage.timestamp).label('date'),
                func.count(WildlifeDetection.id).label('detection_count'),
                func.count(func.distinct(WildlifeDetection.species_id)).label('species_count')
            ).join(CameraImage).join(Camera).filter(
                Camera.user_id == user_id,
                CameraImage.timestamp >= start_date,
                CameraImage.timestamp <= end_date
            ).group_by(func.date(CameraImage.timestamp)).order_by(
                func.date(CameraImage.timestamp)
            ).all()
            
            timeline = []
            for data in timeline_data:
                timeline.append({
                    'date': data.date.isoformat(),
                    'detection_count': data.detection_count,
                    'species_count': data.species_count
                })
            
            return timeline
            
        except Exception as e:
            self.logger.error(f"Failed to generate detection timeline: {str(e)}")
            return []
    
    def _generate_camera_performance(self, user_id: int, start_date: datetime, 
                                    end_date: datetime) -> List[Dict]:
        """Generate camera performance metrics."""
        try:
            camera_stats = db.session.query(
                Camera.id,
                Camera.name,
                Camera.location,
                Camera.status,
                Camera.battery_level,
                Camera.last_seen,
                func.count(CameraImage.id).label('image_count'),
                func.count(WildlifeDetection.id).label('detection_count')
            ).outerjoin(CameraImage).outerjoin(WildlifeDetection).filter(
                Camera.user_id == user_id,
                or_(
                    CameraImage.timestamp.is_(None),
                    and_(
                        CameraImage.timestamp >= start_date,
                        CameraImage.timestamp <= end_date
                    )
                )
            ).group_by(
                Camera.id, Camera.name, Camera.location, Camera.status, 
                Camera.battery_level, Camera.last_seen
            ).all()
            
            performance = []
            for stat in camera_stats:
                # Calculate health score
                health_score = 100
                if stat.battery_level and stat.battery_level < 20:
                    health_score -= 40
                elif stat.battery_level and stat.battery_level < 50:
                    health_score -= 20
                
                if stat.last_seen:
                    hours_since_seen = (datetime.utcnow() - stat.last_seen).total_seconds() / 3600
                    if hours_since_seen > 48:
                        health_score -= 50
                    elif hours_since_seen > 24:
                        health_score -= 30
                else:
                    health_score = 0
                
                performance.append({
                    'camera_id': stat.id,
                    'name': stat.name,
                    'location': stat.location,
                    'status': stat.status,
                    'battery_level': stat.battery_level,
                    'last_seen': stat.last_seen.isoformat() if stat.last_seen else None,
                    'image_count': stat.image_count or 0,
                    'detection_count': stat.detection_count or 0,
                    'detection_rate': round((stat.detection_count or 0) / (stat.image_count or 1) * 100, 1),
                    'health_score': max(0, health_score)
                })
            
            return performance
            
        except Exception as e:
            self.logger.error(f"Failed to generate camera performance: {str(e)}")
            return []
    
    def _generate_activity_patterns(self, user_id: int, start_date: datetime, 
                                   end_date: datetime) -> Dict:
        """Generate wildlife activity pattern analysis."""
        try:
            # Hourly activity pattern
            hourly_activity = db.session.query(
                func.extract('hour', CameraImage.timestamp).label('hour'),
                func.count(WildlifeDetection.id).label('detection_count')
            ).join(CameraImage).join(Camera).filter(
                Camera.user_id == user_id,
                CameraImage.timestamp >= start_date,
                CameraImage.timestamp <= end_date
            ).group_by(func.extract('hour', CameraImage.timestamp)).all()
            
            # Daily activity pattern
            daily_activity = db.session.query(
                func.extract('dow', CameraImage.timestamp).label('day_of_week'),
                func.count(WildlifeDetection.id).label('detection_count')
            ).join(CameraImage).join(Camera).filter(
                Camera.user_id == user_id,
                CameraImage.timestamp >= start_date,
                CameraImage.timestamp <= end_date
            ).group_by(func.extract('dow', CameraImage.timestamp)).all()
            
            # Convert to lists
            hourly_data = [0] * 24
            for activity in hourly_activity:
                hourly_data[int(activity.hour)] = activity.detection_count
            
            daily_data = [0] * 7
            day_names = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday']
            for activity in daily_activity:
                daily_data[int(activity.day_of_week)] = activity.detection_count
            
            return {
                'hourly_pattern': {
                    'hours': list(range(24)),
                    'detections': hourly_data,
                    'peak_hour': hourly_data.index(max(hourly_data)) if max(hourly_data) > 0 else 0
                },
                'daily_pattern': {
                    'days': day_names,
                    'detections': daily_data,
                    'peak_day': day_names[daily_data.index(max(daily_data))] if max(daily_data) > 0 else 'Unknown'
                }
            }
            
        except Exception as e:
            self.logger.error(f"Failed to generate activity patterns: {str(e)}")
            return {}
    
    def _generate_conservation_alerts(self, user_id: int, start_date: datetime, 
                                     end_date: datetime) -> List[Dict]:
        """Generate conservation-related alerts and insights."""
        try:
            # Find endangered species detections
            endangered_detections = db.session.query(
                Species.name,
                Species.conservation_status,
                func.count(WildlifeDetection.id).label('detection_count'),
                func.max(CameraImage.timestamp).label('last_seen')
            ).join(WildlifeDetection).join(CameraImage).join(Camera).filter(
                Camera.user_id == user_id,
                CameraImage.timestamp >= start_date,
                CameraImage.timestamp <= end_date,
                Species.conservation_status.in_(['Vulnerable', 'Endangered', 'Critically Endangered'])
            ).group_by(Species.name, Species.conservation_status).all()
            
            alerts = []
            for detection in endangered_detections:
                severity = 'critical' if detection.conservation_status == 'Critically Endangered' else 'warning'
                
                alerts.append({
                    'type': 'conservation',
                    'severity': severity,
                    'species': detection.name,
                    'conservation_status': detection.conservation_status,
                    'detection_count': detection.detection_count,
                    'last_seen': detection.last_seen.isoformat() if detection.last_seen else None,
                    'message': f"{detection.name} ({detection.conservation_status}) detected {detection.detection_count} times"
                })
            
            return alerts
            
        except Exception as e:
            self.logger.error(f"Failed to generate conservation alerts: {str(e)}")
            return []
    
    def _generate_system_health(self, user_id: int) -> Dict:
        """Generate system health overview."""
        try:
            # Camera health
            total_cameras = Camera.query.filter_by(user_id=user_id).count()
            online_cameras = Camera.query.filter_by(user_id=user_id, status='online').count()
            
            # Low battery cameras
            low_battery_cameras = Camera.query.filter(
                Camera.user_id == user_id,
                Camera.battery_level < 20
            ).count()
            
            # Recent alerts
            recent_alerts = Alert.query.filter(
                Alert.user_id == user_id,
                Alert.created_at >= datetime.utcnow() - timedelta(hours=24),
                Alert.severity.in_(['critical', 'warning'])
            ).count()
            
            # Processing backlog
            unprocessed_images = CameraImage.query.join(Camera).filter(
                Camera.user_id == user_id,
                CameraImage.processed == False
            ).count()
            
            # Overall system health score
            health_score = 100
            if total_cameras > 0:
                camera_health_ratio = online_cameras / total_cameras
                if camera_health_ratio < 0.5:
                    health_score -= 40
                elif camera_health_ratio < 0.8:
                    health_score -= 20
            
            if low_battery_cameras > 0:
                health_score -= min(30, low_battery_cameras * 10)
            
            if recent_alerts > 0:
                health_score -= min(20, recent_alerts * 5)
            
            if unprocessed_images > 100:
                health_score -= 10
            
            return {
                'overall_health_score': max(0, health_score),
                'total_cameras': total_cameras,
                'online_cameras': online_cameras,
                'offline_cameras': total_cameras - online_cameras,
                'low_battery_cameras': low_battery_cameras,
                'recent_critical_alerts': recent_alerts,
                'processing_backlog': unprocessed_images,
                'system_status': 'healthy' if health_score >= 80 else 'warning' if health_score >= 60 else 'critical'
            }
            
        except Exception as e:
            self.logger.error(f"Failed to generate system health: {str(e)}")
            return {}
    
    def generate_species_report(self, user_id: int, species_id: int, 
                               start_date: datetime = None, end_date: datetime = None) -> Dict:
        """Generate detailed report for a specific species."""
        try:
            # Default to last 30 days if no dates provided  
            if not end_date:
                end_date = datetime.utcnow()
            if not start_date:
                start_date = end_date - timedelta(days=30)
            
            species = Species.query.get(species_id)
            if not species:
                return {}
            
            # Get all detections for this species
            detections = WildlifeDetection.query.join(CameraImage).join(Camera).filter(
                Camera.user_id == user_id,
                WildlifeDetection.species_id == species_id,
                CameraImage.timestamp >= start_date,
                CameraImage.timestamp <= end_date
            ).order_by(CameraImage.timestamp.desc()).all()
            
            # Generate detailed analytics
            report = {
                'species': {
                    'name': species.name,
                    'scientific_name': species.scientific_name,
                    'conservation_status': species.conservation_status,
                    'description': species.description
                },
                'period': {
                    'start_date': start_date.isoformat(),
                    'end_date': end_date.isoformat()
                },
                'summary': {
                    'total_detections': len(detections),
                    'unique_cameras': len(set(d.image.camera_id for d in detections)),
                    'average_confidence': round(sum(d.confidence for d in detections) / len(detections), 2) if detections else 0,
                    'first_seen': min(d.image.timestamp for d in detections).isoformat() if detections else None,
                    'last_seen': max(d.image.timestamp for d in detections).isoformat() if detections else None
                },
                'detection_timeline': self._generate_species_timeline(detections),
                'camera_locations': self._generate_species_camera_data(detections),
                'behavior_analysis': self._analyze_species_behavior(detections)
            }
            
            return report
            
        except Exception as e:
            self.logger.error(f"Failed to generate species report: {str(e)}")
            return {}
    
    def _generate_species_timeline(self, detections: List) -> List[Dict]:
        """Generate timeline data for species detections."""
        timeline_data = {}
        
        for detection in detections:
            date_key = detection.image.timestamp.date().isoformat()
            if date_key not in timeline_data:
                timeline_data[date_key] = 0
            timeline_data[date_key] += 1
        
        return [{'date': date, 'count': count} for date, count in sorted(timeline_data.items())]
    
    def _generate_species_camera_data(self, detections: List) -> List[Dict]:
        """Generate camera location data for species detections."""
        camera_data = {}
        
        for detection in detections:
            camera = detection.image.camera
            if camera.id not in camera_data:
                camera_data[camera.id] = {
                    'camera_id': camera.id,
                    'name': camera.name,
                    'location': camera.location,
                    'latitude': camera.latitude,
                    'longitude': camera.longitude,
                    'detection_count': 0
                }
            camera_data[camera.id]['detection_count'] += 1
        
        return list(camera_data.values())
    
    def _analyze_species_behavior(self, detections: List) -> Dict:
        """Analyze behavior patterns from detections."""
        if not detections:
            return {}
        
        # Analyze activity by hour
        hourly_activity = [0] * 24
        for detection in detections:
            hour = detection.image.timestamp.hour
            hourly_activity[hour] += 1
        
        # Find peak activity hours
        peak_hour = hourly_activity.index(max(hourly_activity))
        
        # Determine activity pattern
        night_activity = sum(hourly_activity[22:24] + hourly_activity[0:6])
        day_activity = sum(hourly_activity[6:18])
        evening_activity = sum(hourly_activity[18:22])
        
        total_activity = night_activity + day_activity + evening_activity
        
        if total_activity == 0:
            activity_pattern = 'unknown'
        elif night_activity / total_activity > 0.6:
            activity_pattern = 'nocturnal'
        elif day_activity / total_activity > 0.6:
            activity_pattern = 'diurnal'
        else:
            activity_pattern = 'crepuscular'
        
        return {
            'peak_activity_hour': peak_hour,
            'activity_pattern': activity_pattern,
            'night_activity_percentage': round(night_activity / total_activity * 100, 1) if total_activity > 0 else 0,
            'day_activity_percentage': round(day_activity / total_activity * 100, 1) if total_activity > 0 else 0,
            'evening_activity_percentage': round(evening_activity / total_activity * 100, 1) if total_activity > 0 else 0
        }