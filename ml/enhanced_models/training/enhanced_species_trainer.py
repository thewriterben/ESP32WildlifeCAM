#!/usr/bin/env python3
"""
Enhanced Species Classification Model Trainer for ESP32WildlifeCAM v3.1

This module implements training for enhanced species classification models with:
- Support for 50+ species
- Regional model variants
- Multi-task learning (species + behavior + conservation status)
- Transfer learning with MobileNetV3
- Model quantization for ESP32 deployment
"""

import os
import json
import logging
import argparse
import numpy as np
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Any
from datetime import datetime

import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers, models, optimizers, callbacks
from tensorflow.keras.applications import MobileNetV3Large, MobileNetV3Small
import tensorflow_model_optimization as tfmot
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report, confusion_matrix
from sklearn.preprocessing import LabelEncoder
import albumentations as A
import cv2
import pandas as pd

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class RegionalSpeciesDatabase:
    """Manages regional species databases and metadata"""
    
    REGIONS = {
        'north_america': {
            'name': 'North America',
            'countries': ['US', 'CA', 'MX'],
            'primary_species': [
                'white_tailed_deer', 'black_bear', 'raccoon', 'wild_turkey',
                'coyote', 'gray_wolf', 'mountain_lion', 'bobcat', 'red_fox',
                'gray_fox', 'elk', 'moose', 'bighorn_sheep', 'pronghorn',
                'wild_boar', 'cottontail_rabbit', 'snowshoe_hare', 'beaver',
                'river_otter', 'muskrat', 'opossum', 'skunk', 'porcupine',
                'groundhog', 'chipmunk', 'squirrel'
            ]
        },
        'europe': {
            'name': 'Europe',
            'countries': ['GB', 'FR', 'DE', 'IT', 'ES', 'PL', 'RO', 'NL', 'GR', 'PT'],
            'primary_species': [
                'red_deer', 'roe_deer', 'wild_boar', 'brown_bear', 'gray_wolf',
                'red_fox', 'european_badger', 'pine_marten', 'european_otter',
                'eurasian_lynx', 'wild_cat', 'hare', 'rabbit', 'hedgehog',
                'red_squirrel', 'gray_squirrel', 'stoat', 'weasel', 'polecat',
                'mink', 'beaver', 'chamois', 'ibex', 'mouflon', 'fallow_deer'
            ]
        },
        'asia': {
            'name': 'Asia',
            'countries': ['CN', 'IN', 'JP', 'KR', 'TH', 'MY', 'ID', 'PH', 'VN', 'MM'],
            'primary_species': [
                'tiger', 'leopard', 'snow_leopard', 'asiatic_black_bear',
                'sun_bear', 'sloth_bear', 'red_panda', 'giant_panda',
                'asian_elephant', 'rhino', 'wild_boar', 'sambar_deer',
                'chital', 'muntjac', 'gaur', 'water_buffalo', 'macaque',
                'langur', 'gibbon', 'orangutan', 'civet', 'binturong',
                'pangolin', 'tapir', 'serow'
            ]
        },
        'africa': {
            'name': 'Africa',
            'countries': ['ZA', 'KE', 'TZ', 'BW', 'ZW', 'ZM', 'NA', 'MW', 'UG', 'RW'],
            'primary_species': [
                'african_elephant', 'lion', 'leopard', 'cheetah', 'african_buffalo',
                'rhinoceros', 'giraffe', 'zebra', 'hippo', 'warthog', 'impala',
                'kudu', 'waterbuck', 'hartebeest', 'wildebeest', 'gazelle',
                'duiker', 'bushbuck', 'eland', 'sable_antelope', 'nyala',
                'baboon', 'vervet_monkey', 'hyena', 'wild_dog', 'caracal',
                'serval', 'aardvark', 'pangolin', 'honey_badger'
            ]
        },
        'australia': {
            'name': 'Australia & Oceania',
            'countries': ['AU', 'NZ', 'PG'],
            'primary_species': [
                'kangaroo', 'wallaby', 'koala', 'wombat', 'echidna', 'platypus',
                'tasmanian_devil', 'dingo', 'feral_pig', 'feral_goat',
                'red_deer', 'fallow_deer', 'sambar_deer', 'possum', 'glider',
                'bandicoot', 'quoll', 'bilby', 'bettong', 'potoroo'
            ]
        }
    }
    
    CONSERVATION_STATUS_MAP = {
        'CR': 'critically_endangered',
        'EN': 'endangered', 
        'VU': 'vulnerable',
        'NT': 'near_threatened',
        'LC': 'least_concern',
        'DD': 'data_deficient',
        'EX': 'extinct',
        'EW': 'extinct_in_wild'
    }
    
    BEHAVIOR_CLASSES = [
        'feeding', 'drinking', 'resting', 'alert', 'grooming',
        'playing', 'mating', 'territorial', 'fleeing', 'hunting',
        'foraging', 'traveling', 'socializing', 'unknown'
    ]
    
    def __init__(self, region: str = 'north_america'):
        self.region = region
        self.species_list = self.REGIONS[region]['primary_species']
        self.num_species = len(self.species_list)
        self.species_to_idx = {species: idx for idx, species in enumerate(self.species_list)}
        self.idx_to_species = {idx: species for species, idx in self.species_to_idx.items()}
    
    def get_species_metadata(self) -> Dict[str, Any]:
        """Get comprehensive species metadata for the region"""
        return {
            'region': self.region,
            'region_name': self.REGIONS[self.region]['name'],
            'num_species': self.num_species,
            'species_list': self.species_list,
            'species_mapping': self.species_to_idx,
            'conservation_classes': list(self.CONSERVATION_STATUS_MAP.keys()),
            'behavior_classes': self.BEHAVIOR_CLASSES,
            'countries': self.REGIONS[self.region]['countries']
        }

class EnhancedDataAugmentation:
    """Advanced data augmentation pipeline for wildlife images"""
    
    def __init__(self, input_size: Tuple[int, int] = (224, 224)):
        self.input_size = input_size
        
        # Training augmentations
        self.train_transform = A.Compose([
            A.Resize(height=input_size[0], width=input_size[1]),
            A.HorizontalFlip(p=0.5),
            A.RandomRotate90(p=0.3),
            A.Rotate(limit=15, p=0.4),
            A.RandomBrightnessContrast(
                brightness_limit=0.2, 
                contrast_limit=0.2, 
                p=0.5
            ),
            A.HueSaturationValue(
                hue_shift_limit=10,
                sat_shift_limit=20,
                val_shift_limit=10,
                p=0.3
            ),
            A.GaussianBlur(blur_limit=(1, 3), p=0.2),
            A.GaussNoise(var_limit=(10.0, 50.0), p=0.2),
            A.CoarseDropout(
                max_holes=8, 
                max_height=16, 
                max_width=16, 
                p=0.3
            ),
            A.RandomShadow(p=0.2),
            A.RandomFog(p=0.1),
            A.Normalize(
                mean=[0.485, 0.456, 0.406],
                std=[0.229, 0.224, 0.225]
            )
        ])
        
        # Validation/test augmentations
        self.val_transform = A.Compose([
            A.Resize(height=input_size[0], width=input_size[1]),
            A.Normalize(
                mean=[0.485, 0.456, 0.406],
                std=[0.229, 0.224, 0.225]
            )
        ])
    
    def augment_image(self, image: np.ndarray, is_training: bool = True) -> np.ndarray:
        """Apply augmentation to image"""
        transform = self.train_transform if is_training else self.val_transform
        return transform(image=image)['image']

class MultiTaskModel:
    """Multi-task learning model for species, behavior, and conservation status"""
    
    def __init__(self, 
                 num_species: int,
                 num_behaviors: int = 14,
                 num_conservation_classes: int = 8,
                 input_shape: Tuple[int, int, int] = (224, 224, 3),
                 backbone: str = 'mobilenetv3_large'):
        
        self.num_species = num_species
        self.num_behaviors = num_behaviors
        self.num_conservation_classes = num_conservation_classes
        self.input_shape = input_shape
        self.backbone = backbone
        
        self.model = self._build_model()
    
    def _build_model(self) -> keras.Model:
        """Build multi-task model with shared backbone"""
        
        # Input layer
        inputs = keras.Input(shape=self.input_shape, name='image_input')
        
        # Backbone (feature extractor)
        if self.backbone == 'mobilenetv3_large':
            backbone = MobileNetV3Large(
                input_shape=self.input_shape,
                include_top=False,
                weights='imagenet',
                pooling='avg'
            )
        elif self.backbone == 'mobilenetv3_small':
            backbone = MobileNetV3Small(
                input_shape=self.input_shape,
                include_top=False,
                weights='imagenet',
                pooling='avg'
            )
        else:
            raise ValueError(f"Unsupported backbone: {self.backbone}")
        
        # Make backbone layers trainable (fine-tuning)
        backbone.trainable = True
        
        # Freeze early layers for stable training
        for layer in backbone.layers[:-50]:
            layer.trainable = False
        
        # Extract features
        features = backbone(inputs, training=True)
        
        # Shared dense layers
        x = layers.Dense(512, activation='relu', name='shared_dense_1')(features)
        x = layers.Dropout(0.3, name='shared_dropout_1')(x)
        x = layers.Dense(256, activation='relu', name='shared_dense_2')(x)
        x = layers.Dropout(0.2, name='shared_dropout_2')(x)
        
        # Task-specific heads
        
        # Species classification head
        species_head = layers.Dense(128, activation='relu', name='species_dense')(x)
        species_head = layers.Dropout(0.2, name='species_dropout')(species_head)
        species_output = layers.Dense(
            self.num_species, 
            activation='softmax', 
            name='species_output'
        )(species_head)
        
        # Behavior classification head
        behavior_head = layers.Dense(64, activation='relu', name='behavior_dense')(x)
        behavior_head = layers.Dropout(0.2, name='behavior_dropout')(behavior_head)
        behavior_output = layers.Dense(
            self.num_behaviors, 
            activation='softmax', 
            name='behavior_output'
        )(behavior_head)
        
        # Conservation status head
        conservation_head = layers.Dense(32, activation='relu', name='conservation_dense')(x)
        conservation_head = layers.Dropout(0.2, name='conservation_dropout')(conservation_head)
        conservation_output = layers.Dense(
            self.num_conservation_classes, 
            activation='softmax', 
            name='conservation_output'
        )(conservation_head)
        
        # Create model
        model = keras.Model(
            inputs=inputs,
            outputs=[species_output, behavior_output, conservation_output],
            name='enhanced_wildlife_classifier'
        )
        
        return model
    
    def compile_model(self, learning_rate: float = 0.001):
        """Compile the multi-task model"""
        
        # Loss weights for balancing tasks
        loss_weights = {
            'species_output': 1.0,      # Primary task
            'behavior_output': 0.5,     # Secondary task
            'conservation_output': 0.3   # Auxiliary task
        }
        
        # Compile with multiple losses
        self.model.compile(
            optimizer=optimizers.Adam(learning_rate=learning_rate),
            loss={
                'species_output': 'sparse_categorical_crossentropy',
                'behavior_output': 'sparse_categorical_crossentropy',
                'conservation_output': 'sparse_categorical_crossentropy'
            },
            loss_weights=loss_weights,
            metrics={
                'species_output': ['accuracy', 'top_3_categorical_accuracy'],
                'behavior_output': ['accuracy'],
                'conservation_output': ['accuracy']
            }
        )
    
    def get_model(self) -> keras.Model:
        """Get the compiled model"""
        return self.model

class EnhancedSpeciesTrainer:
    """Main trainer class for enhanced species classification"""
    
    def __init__(self, 
                 region: str = 'north_america',
                 input_size: Tuple[int, int] = (224, 224),
                 backbone: str = 'mobilenetv3_large',
                 batch_size: int = 32):
        
        self.region = region
        self.input_size = input_size
        self.backbone = backbone
        self.batch_size = batch_size
        
        # Initialize components
        self.species_db = RegionalSpeciesDatabase(region)
        self.augmentation = EnhancedDataAugmentation(input_size)
        self.model_builder = MultiTaskModel(
            num_species=self.species_db.num_species,
            input_shape=(input_size[0], input_size[1], 3),
            backbone=backbone
        )
        
        self.model = None
        self.history = None
        
        logger.info(f"Initialized trainer for {region} with {self.species_db.num_species} species")
    
    def load_dataset(self, dataset_path: str) -> Tuple[np.ndarray, Dict[str, np.ndarray]]:
        """Load and preprocess dataset"""
        dataset_path = Path(dataset_path)
        
        if not dataset_path.exists():
            raise FileNotFoundError(f"Dataset path {dataset_path} does not exist")
        
        # Load dataset metadata
        metadata_file = dataset_path / 'metadata.json'
        if metadata_file.exists():
            with open(metadata_file, 'r') as f:
                metadata = json.load(f)
        else:
            raise FileNotFoundError("Dataset metadata.json not found")
        
        # Load images and labels
        images = []
        species_labels = []
        behavior_labels = []
        conservation_labels = []
        
        logger.info("Loading dataset...")
        
        for species_dir in dataset_path.iterdir():
            if not species_dir.is_dir() or species_dir.name == 'metadata':
                continue
            
            species_name = species_dir.name
            if species_name not in self.species_db.species_to_idx:
                logger.warning(f"Species {species_name} not in region {self.region}, skipping")
                continue
            
            species_idx = self.species_db.species_to_idx[species_name]
            
            # Load images from species directory
            for img_file in species_dir.glob('*.jpg'):
                try:
                    image = cv2.imread(str(img_file))
                    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
                    
                    # Apply validation transform (resize + normalize)
                    image = self.augmentation.augment_image(image, is_training=False)
                    
                    images.append(image)
                    species_labels.append(species_idx)
                    
                    # Generate synthetic behavior and conservation labels
                    # In real implementation, these would come from annotations
                    behavior_labels.append(np.random.randint(0, self.species_db.BEHAVIOR_CLASSES.__len__()))
                    conservation_labels.append(np.random.randint(0, len(self.species_db.CONSERVATION_STATUS_MAP)))
                    
                except Exception as e:
                    logger.warning(f"Error loading image {img_file}: {e}")
                    continue
        
        if not images:
            raise ValueError("No valid images found in dataset")
        
        # Convert to numpy arrays
        X = np.array(images, dtype=np.float32)
        y = {
            'species_output': np.array(species_labels, dtype=np.int32),
            'behavior_output': np.array(behavior_labels, dtype=np.int32),
            'conservation_output': np.array(conservation_labels, dtype=np.int32)
        }
        
        logger.info(f"Loaded {len(images)} images for {len(set(species_labels))} species")
        
        return X, y
    
    def train(self, 
              dataset_path: str, 
              epochs: int = 50,
              validation_split: float = 0.2,
              learning_rate: float = 0.001,
              output_dir: str = './models') -> Dict[str, Any]:
        """Train the enhanced species classification model"""
        
        output_dir = Path(output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
        
        try:
            # Load dataset
            X, y = self.load_dataset(dataset_path)
            
            # Split data
            X_train, X_val, y_train_dict, y_val_dict = train_test_split(
                X, 
                {k: v for k, v in y.items()},
                test_size=validation_split,
                random_state=42,
                stratify=y['species_output']
            )
            
            # Build and compile model
            self.model_builder.compile_model(learning_rate=learning_rate)
            self.model = self.model_builder.get_model()
            
            # Print model summary
            self.model.summary()
            
            # Setup callbacks
            callbacks_list = [
                keras.callbacks.ModelCheckpoint(
                    filepath=str(output_dir / f'best_model_{self.region}.h5'),
                    monitor='val_species_output_accuracy',
                    save_best_only=True,
                    save_weights_only=False,
                    verbose=1
                ),
                keras.callbacks.ReduceLROnPlateau(
                    monitor='val_species_output_loss',
                    factor=0.5,
                    patience=5,
                    min_lr=1e-7,
                    verbose=1
                ),
                keras.callbacks.EarlyStopping(
                    monitor='val_species_output_accuracy',
                    patience=10,
                    restore_best_weights=True,
                    verbose=1
                ),
                keras.callbacks.CSVLogger(
                    str(output_dir / f'training_log_{self.region}.csv')
                )
            ]
            
            # Train model
            logger.info("Starting training...")
            
            self.history = self.model.fit(
                X_train,
                y_train_dict,
                batch_size=self.batch_size,
                epochs=epochs,
                validation_data=(X_val, y_val_dict),
                callbacks=callbacks_list,
                verbose=1
            )
            
            # Evaluate model
            logger.info("Evaluating model...")
            evaluation = self.model.evaluate(X_val, y_val_dict, verbose=0)
            
            # Save model metadata
            metadata = {
                'region': self.region,
                'num_species': self.species_db.num_species,
                'species_mapping': self.species_db.species_to_idx,
                'input_shape': self.model.input_shape[1:],
                'backbone': self.backbone,
                'training_date': datetime.now().isoformat(),
                'training_samples': len(X_train),
                'validation_samples': len(X_val),
                'evaluation_metrics': dict(zip(self.model.metrics_names, evaluation)),
                'species_metadata': self.species_db.get_species_metadata()
            }
            
            with open(output_dir / f'model_metadata_{self.region}.json', 'w') as f:
                json.dump(metadata, f, indent=2)
            
            logger.info(f"Training completed. Model saved to {output_dir}")
            
            return {
                'model': self.model,
                'history': self.history.history,
                'metadata': metadata,
                'output_dir': str(output_dir)
            }
            
        except Exception as e:
            logger.error(f"Training failed: {str(e)}")
            raise e
    
    def quantize_for_esp32(self, model_path: str, output_path: str) -> str:
        """Quantize model for ESP32 deployment using TensorFlow Lite"""
        
        if not self.model:
            self.model = keras.models.load_model(model_path)
        
        def representative_dataset():
            """Generate representative dataset for quantization"""
            # Use a subset of validation data
            for i in range(100):
                # Generate sample data (in real implementation, use actual validation set)
                sample = np.random.random((1, *self.input_size, 3)).astype(np.float32)
                yield [sample]
        
        # Convert to TensorFlow Lite with quantization
        converter = tf.lite.TFLiteConverter.from_keras_model(self.model)
        
        # Enable optimizations
        converter.optimizations = [tf.lite.Optimize.DEFAULT]
        
        # Enable integer quantization
        converter.representative_dataset = representative_dataset
        converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
        converter.inference_input_type = tf.int8
        converter.inference_output_type = tf.int8
        
        # Convert model
        quantized_model = converter.convert()
        
        # Save quantized model
        output_path = Path(output_path)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(output_path, 'wb') as f:
            f.write(quantized_model)
        
        logger.info(f"Quantized model saved to {output_path}")
        logger.info(f"Model size: {len(quantized_model) / 1024:.1f} KB")
        
        return str(output_path)

def main():
    """Main training function"""
    parser = argparse.ArgumentParser(description='Train Enhanced Species Classification Model')
    
    parser.add_argument('--region', type=str, default='north_america',
                       choices=['north_america', 'europe', 'asia', 'africa', 'australia'],
                       help='Geographic region for species model')
    parser.add_argument('--dataset', type=str, required=True,
                       help='Path to dataset directory')
    parser.add_argument('--output', type=str, default='./models',
                       help='Output directory for trained models')
    parser.add_argument('--epochs', type=int, default=50,
                       help='Number of training epochs')
    parser.add_argument('--batch-size', type=int, default=32,
                       help='Training batch size')
    parser.add_argument('--backbone', type=str, default='mobilenetv3_large',
                       choices=['mobilenetv3_large', 'mobilenetv3_small'],
                       help='Backbone architecture')
    parser.add_argument('--quantize', action='store_true',
                       help='Generate quantized model for ESP32')
    
    args = parser.parse_args()
    
    # Initialize trainer
    trainer = EnhancedSpeciesTrainer(
        region=args.region,
        backbone=args.backbone,
        batch_size=args.batch_size
    )
    
    # Train model
    results = trainer.train(
        dataset_path=args.dataset,
        epochs=args.epochs,
        output_dir=args.output
    )
    
    # Generate quantized model if requested
    if args.quantize:
        model_path = Path(args.output) / f'best_model_{args.region}.h5'
        quantized_path = Path(args.output) / f'quantized_model_{args.region}.tflite'
        trainer.quantize_for_esp32(str(model_path), str(quantized_path))
    
    logger.info("Training pipeline completed successfully!")

if __name__ == "__main__":
    main()