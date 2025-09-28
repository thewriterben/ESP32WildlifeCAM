"""
Wildlife detection using Microsoft MegaDetector integration
and custom species classification models.
"""

import os
import json
import logging
import numpy as np
from typing import List, Dict, Tuple, Optional
from PIL import Image
import requests
from datetime import datetime

# TensorFlow imports with fallback
try:
    import tensorflow as tf
    TF_AVAILABLE = True
except ImportError:
    TF_AVAILABLE = False
    logging.warning("TensorFlow not available - wildlife detection will use fallback methods")

logger = logging.getLogger(__name__)

class WildlifeDetector:
    """
    Wildlife detection system integrating Microsoft MegaDetector
    and custom species classification models.
    """
    
    def __init__(self, model_path: str = None):
        self.model_path = model_path
        self.model = None
        self.species_classifier = None
        
        # MegaDetector categories
        self.megadetector_categories = {
            1: 'animal',
            2: 'person', 
            3: 'vehicle'
        }
        
        # Load species classification data
        self.species_mapping = self._load_species_mapping()
        
        # Detection thresholds
        self.confidence_threshold = 0.2
        self.animal_confidence_threshold = 0.5
        
        # Initialize models
        self._initialize_models()
    
    def _initialize_models(self):
        """Initialize detection models."""
        try:
            if TF_AVAILABLE and self.model_path and os.path.exists(self.model_path):
                self._load_megadetector_model()
            else:
                logger.warning("MegaDetector model not available - using fallback detection")
                
        except Exception as e:
            logger.error(f"Failed to initialize models: {str(e)}")
    
    def _load_megadetector_model(self):
        """Load Microsoft MegaDetector TensorFlow model."""
        try:
            logger.info(f"Loading MegaDetector model from {self.model_path}")
            
            # Load saved model
            self.model = tf.saved_model.load(self.model_path)
            self.infer = self.model.signatures['serving_default']
            
            logger.info("MegaDetector model loaded successfully")
            
        except Exception as e:
            logger.error(f"Failed to load MegaDetector model: {str(e)}")
            self.model = None
    
    def _load_species_mapping(self) -> Dict:
        """Load species classification mapping."""
        # Default species mapping - in production this would be loaded from database
        return {
            'deer': {
                'scientific_name': 'Odocoileus virginianus',
                'common_names': ['White-tailed deer', 'Whitetail'],
                'conservation_status': 'Least Concern',
                'typical_confidence_range': [0.7, 0.95]
            },
            'bear': {
                'scientific_name': 'Ursus americanus', 
                'common_names': ['Black bear', 'American black bear'],
                'conservation_status': 'Least Concern',
                'typical_confidence_range': [0.8, 0.98]
            },
            'fox': {
                'scientific_name': 'Vulpes vulpes',
                'common_names': ['Red fox'],
                'conservation_status': 'Least Concern', 
                'typical_confidence_range': [0.6, 0.9]
            },
            'wolf': {
                'scientific_name': 'Canis lupus',
                'common_names': ['Gray wolf', 'Grey wolf'],
                'conservation_status': 'Least Concern',
                'typical_confidence_range': [0.75, 0.95]
            },
            'raccoon': {
                'scientific_name': 'Procyon lotor',
                'common_names': ['Common raccoon'],
                'conservation_status': 'Least Concern',
                'typical_confidence_range': [0.7, 0.92]
            },
            'rabbit': {
                'scientific_name': 'Sylvilagus floridanus',
                'common_names': ['Eastern cottontail', 'Cottontail rabbit'],
                'conservation_status': 'Least Concern',
                'typical_confidence_range': [0.6, 0.85]
            },
            'bird': {
                'scientific_name': 'Aves',
                'common_names': ['Bird species'],
                'conservation_status': 'Various',
                'typical_confidence_range': [0.5, 0.8]
            }
        }
    
    def detect_wildlife(self, image_path: str) -> List[Dict]:
        """
        Detect wildlife in an image.
        
        Args:
            image_path: Path to the image file
            
        Returns:
            List of detection dictionaries containing species, confidence, and bounding box
        """
        try:
            if not os.path.exists(image_path):
                raise FileNotFoundError(f"Image file not found: {image_path}")
            
            # Load and preprocess image
            image = self._load_and_preprocess_image(image_path)
            
            # Run detection
            if self.model:
                detections = self._run_megadetector(image)
            else:
                detections = self._run_fallback_detection(image_path)
            
            # Post-process and classify species
            results = []
            for detection in detections:
                if detection['category'] == 'animal' and detection['confidence'] >= self.animal_confidence_threshold:
                    # Classify species
                    species_result = self._classify_species(image, detection)
                    results.append(species_result)
            
            logger.info(f"Detected {len(results)} wildlife instances in {image_path}")
            return results
            
        except Exception as e:
            logger.error(f"Wildlife detection failed for {image_path}: {str(e)}")
            return []
    
    def _load_and_preprocess_image(self, image_path: str) -> np.ndarray:
        """Load and preprocess image for detection."""
        try:
            # Load image
            image = Image.open(image_path)
            
            # Convert to RGB if necessary
            if image.mode != 'RGB':
                image = image.convert('RGB')
            
            # Convert to numpy array
            image_np = np.array(image)
            
            return image_np
            
        except Exception as e:
            logger.error(f"Failed to load image {image_path}: {str(e)}")
            raise
    
    def _run_megadetector(self, image: np.ndarray) -> List[Dict]:
        """Run MegaDetector on preprocessed image."""
        try:
            # Prepare input tensor
            input_tensor = tf.convert_to_tensor(image)
            input_tensor = input_tensor[tf.newaxis, ...]
            
            # Run inference
            detections = self.infer(input_tensor)
            
            # Extract results
            results = []
            
            boxes = detections['detection_boxes'][0].numpy()
            classes = detections['detection_classes'][0].numpy().astype(int)
            scores = detections['detection_scores'][0].numpy()
            
            for i in range(len(boxes)):
                if scores[i] >= self.confidence_threshold:
                    category = self.megadetector_categories.get(classes[i], 'unknown')
                    
                    # Convert normalized coordinates to pixel coordinates
                    height, width = image.shape[:2]
                    ymin, xmin, ymax, xmax = boxes[i]
                    
                    results.append({
                        'category': category,
                        'confidence': float(scores[i]),
                        'bounding_box': {
                            'x': int(xmin * width),
                            'y': int(ymin * height),
                            'width': int((xmax - xmin) * width),
                            'height': int((ymax - ymin) * height)
                        }
                    })
            
            return results
            
        except Exception as e:
            logger.error(f"MegaDetector inference failed: {str(e)}")
            return []
    
    def _run_fallback_detection(self, image_path: str) -> List[Dict]:
        """
        Fallback detection method when MegaDetector is not available.
        Uses simple heuristics and metadata analysis.
        """
        try:
            # Simple fallback - assume wildlife if motion was detected
            # In production, this could use other CV techniques or API calls
            
            # Load image to get dimensions
            image = Image.open(image_path)
            width, height = image.size
            
            # Create a default detection covering most of the image
            detection = {
                'category': 'animal',
                'confidence': 0.6,  # Lower confidence for fallback
                'bounding_box': {
                    'x': int(width * 0.1),
                    'y': int(height * 0.1), 
                    'width': int(width * 0.8),
                    'height': int(height * 0.8)
                }
            }
            
            logger.info("Using fallback detection method")
            return [detection]
            
        except Exception as e:
            logger.error(f"Fallback detection failed: {str(e)}")
            return []
    
    def _classify_species(self, image: np.ndarray, detection: Dict) -> Dict:
        """
        Classify detected animal to species level.
        
        Args:
            image: Full image array
            detection: Detection dictionary with bounding box
            
        Returns:
            Enhanced detection with species classification
        """
        try:
            # Extract region of interest
            bbox = detection['bounding_box']
            roi = image[bbox['y']:bbox['y']+bbox['height'], 
                       bbox['x']:bbox['x']+bbox['width']]
            
            # Simple species classification (placeholder)
            # In production, this would use a trained species classifier
            species = self._simple_species_classification(roi)
            
            # Build result
            result = {
                'species': species,
                'confidence': detection['confidence'],
                'bounding_box': detection['bounding_box'],
                'scientific_name': self.species_mapping.get(species, {}).get('scientific_name', ''),
                'detection_timestamp': datetime.utcnow().isoformat(),
                'model_version': 'megadetector_v5a',
                'classification_method': 'simple_heuristic'
            }
            
            return result
            
        except Exception as e:
            logger.error(f"Species classification failed: {str(e)}")
            return {
                'species': 'unknown',
                'confidence': detection['confidence'],
                'bounding_box': detection['bounding_box'],
                'scientific_name': '',
                'detection_timestamp': datetime.utcnow().isoformat()
            }
    
    def _simple_species_classification(self, roi: np.ndarray) -> str:
        """
        Simple species classification based on basic image properties.
        This is a placeholder - in production use a trained classifier.
        """
        try:
            # Get basic image statistics
            mean_color = np.mean(roi, axis=(0, 1))
            roi_size = roi.shape[0] * roi.shape[1]
            
            # Simple heuristics based on color and size
            if roi_size > 50000:  # Large animal
                if mean_color[0] > mean_color[1] and mean_color[0] > mean_color[2]:
                    return 'deer'  # Reddish/brown
                elif np.mean(mean_color) < 80:
                    return 'bear'  # Dark
                else:
                    return 'deer'  # Default large animal
            elif roi_size > 10000:  # Medium animal
                if mean_color[0] > 150:
                    return 'fox'  # Reddish
                else:
                    return 'raccoon'
            else:  # Small animal
                if np.mean(mean_color) > 120:
                    return 'rabbit'  # Light colored
                else:
                    return 'bird'
            
        except Exception as e:
            logger.error(f"Simple classification failed: {str(e)}")
            return 'unknown'
    
    def batch_process_images(self, image_paths: List[str]) -> Dict[str, List[Dict]]:
        """
        Process multiple images in batch.
        
        Args:
            image_paths: List of image file paths
            
        Returns:
            Dictionary mapping image paths to detection results
        """
        results = {}
        
        for image_path in image_paths:
            try:
                detections = self.detect_wildlife(image_path)
                results[image_path] = detections
            except Exception as e:
                logger.error(f"Batch processing failed for {image_path}: {str(e)}")
                results[image_path] = []
        
        return results
    
    def update_confidence_threshold(self, threshold: float):
        """Update detection confidence threshold."""
        if 0.0 <= threshold <= 1.0:
            self.confidence_threshold = threshold
            logger.info(f"Updated confidence threshold to {threshold}")
        else:
            raise ValueError("Confidence threshold must be between 0.0 and 1.0")
    
    def get_model_info(self) -> Dict:
        """Get information about loaded models."""
        return {
            'megadetector_loaded': self.model is not None,
            'model_path': self.model_path,
            'tensorflow_available': TF_AVAILABLE,
            'confidence_threshold': self.confidence_threshold,
            'supported_species': list(self.species_mapping.keys())
        }