#!/usr/bin/env python3
"""
Data Preprocessing Pipeline for ESP32 Wildlife Camera

This module provides comprehensive data preprocessing utilities for wildlife
classification, including image preprocessing, data augmentation, normalization,
and batch processing utilities.
"""

import os
import json
import logging
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Union, Callable
import numpy as np
import pandas as pd
from concurrent.futures import ThreadPoolExecutor, as_completed

# Image processing
import cv2
from PIL import Image, ImageEnhance, ImageFilter
import albumentations as A
from albumentations import pytorch as AT

# Deep learning
import tensorflow as tf
from tensorflow.keras.preprocessing.image import ImageDataGenerator

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class WildlifeImagePreprocessor:
    """
    Comprehensive image preprocessing pipeline for wildlife classification.
    
    Features:
    - Image resizing and normalization
    - Advanced data augmentation
    - Batch processing utilities
    - Quality assessment and filtering
    - Format conversion and optimization
    """
    
    def __init__(self, config: Optional[Dict] = None):
        self.config = self._load_config(config)
        self.target_size = tuple(self.config['target_size'])
        self.augmentor = self._setup_augmentation()
        
        logger.info(f"Preprocessor initialized with target size: {self.target_size}")
    
    def _load_config(self, config: Optional[Dict]) -> Dict:
        """Load preprocessing configuration."""
        default_config = {
            'target_size': [224, 224],
            'normalize': True,
            'resize_method': 'bilinear',
            'maintain_aspect_ratio': True,
            'center_crop': True,
            'quality_threshold': 0.7,
            'augmentation': {
                'enabled': True,
                'rotation_range': 15,
                'zoom_range': 0.15,
                'brightness_range': 0.2,
                'contrast_range': 0.2,
                'saturation_range': 0.15,
                'hue_range': 0.1,
                'horizontal_flip': True,
                'vertical_flip': False,
                'noise_probability': 0.1,
                'blur_probability': 0.05
            },
            'preprocessing': {
                'gaussian_blur': False,
                'histogram_equalization': False,
                'edge_enhancement': False,
                'noise_reduction': True
            }
        }
        
        if config:
            default_config.update(config)
        
        return default_config
    
    def _setup_augmentation(self) -> A.Compose:
        """Set up albumentations augmentation pipeline."""
        aug_config = self.config['augmentation']
        
        if not aug_config['enabled']:
            return A.Compose([])
        
        transforms = []
        
        # Geometric transforms
        if aug_config['rotation_range'] > 0:
            transforms.append(A.Rotate(
                limit=aug_config['rotation_range'],
                interpolation=cv2.INTER_LINEAR,
                border_mode=cv2.BORDER_REFLECT,
                p=0.7
            ))
        
        if aug_config['zoom_range'] > 0:
            zoom = aug_config['zoom_range']
            transforms.append(A.RandomScale(
                scale_limit=(-zoom, zoom),
                interpolation=cv2.INTER_LINEAR,
                p=0.5
            ))
        
        # Flipping
        if aug_config['horizontal_flip']:
            transforms.append(A.HorizontalFlip(p=0.5))
        
        if aug_config['vertical_flip']:
            transforms.append(A.VerticalFlip(p=0.2))
        
        # Color transforms
        if aug_config['brightness_range'] > 0:
            brightness = aug_config['brightness_range']
            transforms.append(A.RandomBrightnessContrast(
                brightness_limit=brightness,
                contrast_limit=0,
                p=0.6
            ))
        
        if aug_config['contrast_range'] > 0:
            contrast = aug_config['contrast_range']
            transforms.append(A.RandomBrightnessContrast(
                brightness_limit=0,
                contrast_limit=contrast,
                p=0.6
            ))
        
        if aug_config['saturation_range'] > 0:
            transforms.append(A.HueSaturationValue(
                hue_shift_limit=0,
                sat_shift_limit=int(aug_config['saturation_range'] * 255),
                val_shift_limit=0,
                p=0.5
            ))
        
        if aug_config['hue_range'] > 0:
            transforms.append(A.HueSaturationValue(
                hue_shift_limit=int(aug_config['hue_range'] * 180),
                sat_shift_limit=0,
                val_shift_limit=0,
                p=0.3
            ))
        
        # Noise and blur
        if aug_config['noise_probability'] > 0:
            transforms.append(A.GaussNoise(
                var_limit=(10.0, 50.0),
                p=aug_config['noise_probability']
            ))
        
        if aug_config['blur_probability'] > 0:
            transforms.append(A.OneOf([
                A.MotionBlur(blur_limit=3, p=0.5),
                A.MedianBlur(blur_limit=3, p=0.3),
                A.GaussianBlur(blur_limit=3, p=0.2)
            ], p=aug_config['blur_probability']))
        
        # Weather effects
        transforms.extend([
            A.OneOf([
                A.RandomRain(brightness_coefficient=0.9, drop_width=1, blur_value=1, p=0.3),
                A.RandomSunFlare(flare_roi=(0, 0, 1, 0.5), angle_lower=0, p=0.2),
                A.RandomShadow(shadow_roi=(0, 0.5, 1, 1), num_shadows_lower=1, p=0.2)
            ], p=0.15),
            
            # Final normalization
            A.Normalize(
                mean=[0.485, 0.456, 0.406],  # ImageNet means
                std=[0.229, 0.224, 0.225],   # ImageNet stds
                max_pixel_value=255.0
            )
        ])
        
        return A.Compose(transforms)
    
    def preprocess_image(self, image_path: Union[str, Path], 
                        apply_augmentation: bool = False) -> np.ndarray:
        """
        Preprocess a single image.
        
        Args:
            image_path: Path to input image
            apply_augmentation: Whether to apply data augmentation
            
        Returns:
            Preprocessed image as numpy array
        """
        # Load image
        image = self._load_image(image_path)
        
        # Quality check
        if not self._check_image_quality(image):
            logger.warning(f"Low quality image detected: {image_path}")
        
        # Resize image
        image = self._resize_image(image)
        
        # Apply preprocessing
        image = self._apply_preprocessing(image)
        
        # Apply augmentation if requested
        if apply_augmentation and self.config['augmentation']['enabled']:
            try:
                augmented = self.augmentor(image=image)
                image = augmented['image']
            except Exception as e:
                logger.warning(f"Augmentation failed for {image_path}: {e}")
        
        # Ensure correct format
        if not self.config['augmentation']['enabled'] or not apply_augmentation:
            image = self._normalize_image(image)
        
        return image
    
    def _load_image(self, image_path: Union[str, Path]) -> np.ndarray:
        """Load image from file."""
        try:
            # Use OpenCV for loading (BGR format)
            image = cv2.imread(str(image_path))
            if image is None:
                raise ValueError(f"Could not load image: {image_path}")
            
            # Convert BGR to RGB
            image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
            return image
            
        except Exception as e:
            logger.error(f"Error loading image {image_path}: {e}")
            raise
    
    def _check_image_quality(self, image: np.ndarray) -> bool:
        """Check if image meets quality requirements."""
        # Check resolution
        height, width = image.shape[:2]
        if min(height, width) < 100:  # Too small
            return False
        
        # Check if image is too dark or too bright
        mean_brightness = np.mean(image)
        if mean_brightness < 30 or mean_brightness > 225:
            return False
        
        # Check for blur (Laplacian variance)
        gray = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
        laplacian_var = cv2.Laplacian(gray, cv2.CV_64F).var()
        
        if laplacian_var < 100:  # Likely blurred
            return False
        
        return True
    
    def _resize_image(self, image: np.ndarray) -> np.ndarray:
        """Resize image to target size."""
        height, width = image.shape[:2]
        target_width, target_height = self.target_size
        
        if self.config['maintain_aspect_ratio']:
            # Calculate scaling factor
            scale = min(target_width / width, target_height / height)
            new_width = int(width * scale)
            new_height = int(height * scale)
            
            # Resize
            if self.config['resize_method'] == 'bilinear':
                image = cv2.resize(image, (new_width, new_height), interpolation=cv2.INTER_LINEAR)
            elif self.config['resize_method'] == 'cubic':
                image = cv2.resize(image, (new_width, new_height), interpolation=cv2.INTER_CUBIC)
            else:
                image = cv2.resize(image, (new_width, new_height), interpolation=cv2.INTER_AREA)
            
            # Pad or crop to exact target size
            if self.config['center_crop'] and (new_width != target_width or new_height != target_height):
                # Center crop/pad
                image = self._center_crop_pad(image, (target_width, target_height))
        else:
            # Direct resize without maintaining aspect ratio
            if self.config['resize_method'] == 'bilinear':
                image = cv2.resize(image, (target_width, target_height), interpolation=cv2.INTER_LINEAR)
            else:
                image = cv2.resize(image, (target_width, target_height), interpolation=cv2.INTER_AREA)
        
        return image
    
    def _center_crop_pad(self, image: np.ndarray, target_size: Tuple[int, int]) -> np.ndarray:
        """Center crop or pad image to target size."""
        height, width = image.shape[:2]
        target_width, target_height = target_size
        
        if width > target_width or height > target_height:
            # Crop
            start_x = (width - target_width) // 2
            start_y = (height - target_height) // 2
            
            end_x = start_x + target_width
            end_y = start_y + target_height
            
            image = image[start_y:end_y, start_x:end_x]
        elif width < target_width or height < target_height:
            # Pad
            pad_x = (target_width - width) // 2
            pad_y = (target_height - height) // 2
            
            image = cv2.copyMakeBorder(
                image, pad_y, target_height - height - pad_y,
                pad_x, target_width - width - pad_x,
                cv2.BORDER_REFLECT
            )
        
        return image
    
    def _apply_preprocessing(self, image: np.ndarray) -> np.ndarray:
        """Apply preprocessing operations."""
        prep_config = self.config['preprocessing']
        
        # Noise reduction
        if prep_config.get('noise_reduction', True):
            image = cv2.bilateralFilter(image, 9, 75, 75)
        
        # Gaussian blur (mild)
        if prep_config.get('gaussian_blur', False):
            image = cv2.GaussianBlur(image, (3, 3), 0)
        
        # Histogram equalization
        if prep_config.get('histogram_equalization', False):
            # Apply to each channel
            for i in range(3):
                image[:, :, i] = cv2.equalizeHist(image[:, :, i])
        
        # Edge enhancement
        if prep_config.get('edge_enhancement', False):
            # Subtle sharpening
            kernel = np.array([[0, -1, 0], [-1, 5, -1], [0, -1, 0]])
            image = cv2.filter2D(image, -1, kernel)
        
        return image
    
    def _normalize_image(self, image: np.ndarray) -> np.ndarray:
        """Normalize image values."""
        if self.config['normalize']:
            # Convert to float and normalize to [0, 1]
            image = image.astype(np.float32) / 255.0
            
            # Apply ImageNet normalization
            mean = np.array([0.485, 0.456, 0.406])
            std = np.array([0.229, 0.224, 0.225])
            image = (image - mean) / std
        
        return image
    
    def batch_preprocess(self, image_paths: List[Union[str, Path]], 
                        output_dir: Optional[Union[str, Path]] = None,
                        apply_augmentation: bool = False,
                        num_workers: int = 4) -> List[np.ndarray]:
        """
        Preprocess multiple images in parallel.
        
        Args:
            image_paths: List of image paths
            output_dir: Optional directory to save preprocessed images
            apply_augmentation: Whether to apply augmentation
            num_workers: Number of worker threads
            
        Returns:
            List of preprocessed images
        """
        logger.info(f"Preprocessing {len(image_paths)} images with {num_workers} workers...")
        
        preprocessed_images = []
        
        def preprocess_single(image_path):
            try:
                return self.preprocess_image(image_path, apply_augmentation)
            except Exception as e:
                logger.error(f"Failed to preprocess {image_path}: {e}")
                return None
        
        with ThreadPoolExecutor(max_workers=num_workers) as executor:
            # Submit all tasks
            future_to_path = {
                executor.submit(preprocess_single, path): path 
                for path in image_paths
            }
            
            # Collect results
            for future in as_completed(future_to_path):
                image_path = future_to_path[future]
                try:
                    result = future.result()
                    if result is not None:
                        preprocessed_images.append(result)
                        
                        # Save if output directory specified
                        if output_dir:
                            self._save_preprocessed_image(result, image_path, output_dir)
                            
                except Exception as e:
                    logger.error(f"Error processing {image_path}: {e}")
        
        logger.info(f"Successfully preprocessed {len(preprocessed_images)}/{len(image_paths)} images")
        return preprocessed_images
    
    def _save_preprocessed_image(self, image: np.ndarray, original_path: Union[str, Path], 
                               output_dir: Union[str, Path]):
        """Save preprocessed image to output directory."""
        output_dir = Path(output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
        
        # Generate output filename
        original_name = Path(original_path).stem
        output_path = output_dir / f"{original_name}_preprocessed.jpg"
        
        # Denormalize if needed
        if self.config['normalize']:
            # Reverse ImageNet normalization
            mean = np.array([0.485, 0.456, 0.406])
            std = np.array([0.229, 0.224, 0.225])
            image = image * std + mean
            image = np.clip(image * 255, 0, 255).astype(np.uint8)
        
        # Convert RGB to BGR for OpenCV
        image_bgr = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
        cv2.imwrite(str(output_path), image_bgr)


class DatasetProcessor:
    """
    Dataset management and processing utilities.
    
    Features:
    - Dataset organization and validation
    - Label management and encoding
    - Dataset splitting utilities
    - Annotation format conversion
    - Data loading and batching
    """
    
    def __init__(self, config: Optional[Dict] = None):
        self.config = config or {}
        self.preprocessor = WildlifeImagePreprocessor(config)
        
    def organize_dataset(self, source_dir: Union[str, Path], 
                        output_dir: Union[str, Path],
                        species_mapping: Optional[Dict[str, int]] = None) -> Dict:
        """
        Organize dataset into structured format.
        
        Args:
            source_dir: Source directory with images
            output_dir: Output directory for organized dataset
            species_mapping: Optional mapping of species names to indices
            
        Returns:
            Dataset organization summary
        """
        source_dir = Path(source_dir)
        output_dir = Path(output_dir)
        
        logger.info(f"Organizing dataset from {source_dir} to {output_dir}")
        
        # Create output structure
        train_dir = output_dir / 'train'
        val_dir = output_dir / 'validation'
        test_dir = output_dir / 'test'
        
        for split_dir in [train_dir, val_dir, test_dir]:
            split_dir.mkdir(parents=True, exist_ok=True)
        
        # Scan source directory for images and labels
        image_files = []
        for ext in ['*.jpg', '*.jpeg', '*.png', '*.bmp']:
            image_files.extend(source_dir.glob(f'**/{ext}'))
            image_files.extend(source_dir.glob(f'**/{ext.upper()}'))
        
        logger.info(f"Found {len(image_files)} image files")
        
        # Organize by species (assuming directory structure or filename contains species)
        species_images = {}
        unknown_images = []
        
        for image_file in image_files:
            species = self._extract_species_from_path(image_file, species_mapping)
            if species:
                if species not in species_images:
                    species_images[species] = []
                species_images[species].append(image_file)
            else:
                unknown_images.append(image_file)
        
        # Split and copy files
        dataset_summary = {
            'total_images': len(image_files),
            'species_count': len(species_images),
            'unknown_images': len(unknown_images),
            'splits': {'train': {}, 'validation': {}, 'test': {}}
        }
        
        for species, images in species_images.items():
            # Create species directories
            for split_dir in [train_dir, val_dir, test_dir]:
                (split_dir / species).mkdir(exist_ok=True)
            
            # Split images
            np.random.shuffle(images)
            
            train_split = int(len(images) * 0.7)
            val_split = int(len(images) * 0.2)
            
            train_images = images[:train_split]
            val_images = images[train_split:train_split + val_split]
            test_images = images[train_split + val_split:]
            
            # Copy files
            self._copy_images(train_images, train_dir / species)
            self._copy_images(val_images, val_dir / species)
            self._copy_images(test_images, test_dir / species)
            
            # Update summary
            dataset_summary['splits']['train'][species] = len(train_images)
            dataset_summary['splits']['validation'][species] = len(val_images)
            dataset_summary['splits']['test'][species] = len(test_images)
        
        # Save summary
        with open(output_dir / 'dataset_summary.json', 'w') as f:
            json.dump(dataset_summary, f, indent=2)
        
        logger.info(f"Dataset organization completed. Summary: {dataset_summary}")
        return dataset_summary
    
    def _extract_species_from_path(self, image_path: Path, 
                                  species_mapping: Optional[Dict] = None) -> Optional[str]:
        """Extract species name from image path."""
        # Try parent directory name first
        parent_name = image_path.parent.name.lower()
        
        # Common species names
        common_species = [
            'deer', 'bear', 'fox', 'wolf', 'raccoon', 'rabbit', 'bird', 
            'squirrel', 'coyote', 'bobcat', 'elk', 'moose', 'wild_boar'
        ]
        
        for species in common_species:
            if species in parent_name:
                return species
        
        # Try filename
        filename = image_path.stem.lower()
        for species in common_species:
            if species in filename:
                return species
        
        # Try species mapping if provided
        if species_mapping:
            for species_name in species_mapping.keys():
                if species_name.lower() in parent_name or species_name.lower() in filename:
                    return species_name
        
        return None
    
    def _copy_images(self, image_paths: List[Path], destination_dir: Path):
        """Copy images to destination directory."""
        import shutil
        
        for image_path in image_paths:
            destination_path = destination_dir / image_path.name
            try:
                shutil.copy2(str(image_path), str(destination_path))
            except Exception as e:
                logger.error(f"Failed to copy {image_path}: {e}")
    
    def validate_dataset(self, dataset_dir: Union[str, Path]) -> Dict:
        """
        Validate dataset structure and content.
        
        Args:
            dataset_dir: Path to dataset directory
            
        Returns:
            Validation report
        """
        dataset_dir = Path(dataset_dir)
        logger.info(f"Validating dataset at {dataset_dir}")
        
        validation_report = {
            'valid': True,
            'errors': [],
            'warnings': [],
            'statistics': {}
        }
        
        # Check directory structure
        required_dirs = ['train', 'validation', 'test']
        for dir_name in required_dirs:
            dir_path = dataset_dir / dir_name
            if not dir_path.exists():
                validation_report['errors'].append(f"Missing directory: {dir_name}")
                validation_report['valid'] = False
        
        # Validate each split
        for split in required_dirs:
            split_dir = dataset_dir / split
            if split_dir.exists():
                split_stats = self._validate_split_directory(split_dir)
                validation_report['statistics'][split] = split_stats
                
                if split_stats['corrupted_images'] > 0:
                    validation_report['warnings'].append(
                        f"{split} split has {split_stats['corrupted_images']} corrupted images"
                    )
        
        return validation_report
    
    def _validate_split_directory(self, split_dir: Path) -> Dict:
        """Validate a single split directory."""
        stats = {
            'total_images': 0,
            'species_count': 0,
            'corrupted_images': 0,
            'species_distribution': {}
        }
        
        # Get species directories
        species_dirs = [d for d in split_dir.iterdir() if d.is_dir()]
        stats['species_count'] = len(species_dirs)
        
        for species_dir in species_dirs:
            species_name = species_dir.name
            image_files = []
            
            for ext in ['*.jpg', '*.jpeg', '*.png', '*.bmp']:
                image_files.extend(species_dir.glob(ext))
                image_files.extend(species_dir.glob(ext.upper()))
            
            valid_images = 0
            for image_file in image_files:
                try:
                    # Try to load image
                    img = cv2.imread(str(image_file))
                    if img is not None:
                        valid_images += 1
                    else:
                        stats['corrupted_images'] += 1
                except:
                    stats['corrupted_images'] += 1
            
            stats['species_distribution'][species_name] = valid_images
            stats['total_images'] += valid_images
        
        return stats
    
    def create_tf_dataset(self, dataset_dir: Union[str, Path], 
                         split: str = 'train',
                         batch_size: int = 32,
                         shuffle: bool = True,
                         apply_augmentation: bool = True) -> tf.data.Dataset:
        """
        Create TensorFlow dataset from organized directory structure.
        
        Args:
            dataset_dir: Path to dataset directory
            split: Dataset split ('train', 'validation', 'test')
            batch_size: Batch size
            shuffle: Whether to shuffle data
            apply_augmentation: Whether to apply augmentation
            
        Returns:
            TensorFlow dataset
        """
        split_dir = Path(dataset_dir) / split
        
        if not split_dir.exists():
            raise ValueError(f"Split directory not found: {split_dir}")
        
        # Get all image paths and labels
        image_paths = []
        labels = []
        class_names = []
        
        species_dirs = sorted([d for d in split_dir.iterdir() if d.is_dir()])
        
        for class_idx, species_dir in enumerate(species_dirs):
            species_name = species_dir.name
            class_names.append(species_name)
            
            # Get image files
            species_images = []
            for ext in ['*.jpg', '*.jpeg', '*.png', '*.bmp']:
                species_images.extend(species_dir.glob(ext))
                species_images.extend(species_dir.glob(ext.upper()))
            
            for image_path in species_images:
                image_paths.append(str(image_path))
                labels.append(class_idx)
        
        # Convert labels to categorical
        num_classes = len(class_names)
        labels = tf.keras.utils.to_categorical(labels, num_classes)
        
        # Create preprocessing function
        def preprocess_fn(image_path, label):
            # Load and preprocess image
            image_str = tf.py_function(
                lambda path: self.preprocessor.preprocess_image(
                    path.numpy().decode('utf-8'),
                    apply_augmentation=(split == 'train' and apply_augmentation)
                ),
                [image_path],
                tf.float32
            )
            image_str.set_shape([self.preprocessor.target_size[1], 
                               self.preprocessor.target_size[0], 3])
            return image_str, label
        
        # Create dataset
        dataset = tf.data.Dataset.from_tensor_slices((image_paths, labels))
        
        if shuffle:
            dataset = dataset.shuffle(buffer_size=1000)
        
        dataset = dataset.map(preprocess_fn, num_parallel_calls=tf.data.AUTOTUNE)
        dataset = dataset.batch(batch_size)
        dataset = dataset.prefetch(tf.data.AUTOTUNE)
        
        logger.info(f"Created {split} dataset with {len(image_paths)} images, {num_classes} classes")
        return dataset


def main():
    """Example usage of preprocessing utilities."""
    import argparse
    
    parser = argparse.ArgumentParser(description='Preprocess wildlife dataset')
    parser.add_argument('--input', type=str, required=True, help='Input dataset directory')
    parser.add_argument('--output', type=str, required=True, help='Output directory')
    parser.add_argument('--organize', action='store_true', help='Organize dataset structure')
    parser.add_argument('--validate', action='store_true', help='Validate dataset')
    parser.add_argument('--preprocess', action='store_true', help='Preprocess images')
    
    args = parser.parse_args()
    
    # Initialize processor
    processor = DatasetProcessor()
    
    if args.organize:
        logger.info("Organizing dataset...")
        summary = processor.organize_dataset(args.input, args.output)
        print(f"Dataset organized: {summary}")
    
    if args.validate:
        logger.info("Validating dataset...")
        report = processor.validate_dataset(args.output if args.organize else args.input)
        print(f"Validation report: {report}")
    
    if args.preprocess:
        logger.info("Preprocessing images...")
        # This would preprocess all images in the dataset
        # Implementation depends on specific requirements


if __name__ == "__main__":
    main()