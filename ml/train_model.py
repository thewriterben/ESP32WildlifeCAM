#!/usr/bin/env python3
"""
Wildlife Species Classification Model Training Script

This script trains custom wildlife detection and classification models
for the ESP32 Wildlife Camera system, integrating with existing datasets
and optimizing for edge deployment.
"""

import os
import json
import argparse
import logging
from datetime import datetime
from pathlib import Path
import numpy as np
import pandas as pd
from typing import Dict, List, Tuple, Optional

# Deep Learning frameworks
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers, models, optimizers, callbacks
from tensorflow.keras.preprocessing.image import ImageDataGenerator
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report, confusion_matrix
import matplotlib.pyplot as plt
import seaborn as sns

# Computer Vision
import cv2
from PIL import Image

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class WildlifeModelTrainer:
    """
    Wildlife classification model trainer with support for:
    - Custom dataset integration
    - Transfer learning from pre-trained models
    - Edge optimization for ESP32 deployment
    - Integration with Microsoft MegaDetector
    """
    
    def __init__(self, config_path: str = None):
        self.config = self._load_config(config_path)
        self.model = None
        self.history = None
        
        # Set up paths
        self.data_dir = Path(self.config['data_dir'])
        self.output_dir = Path(self.config['output_dir'])
        self.models_dir = self.output_dir / 'models'
        self.logs_dir = self.output_dir / 'logs'
        
        # Create directories
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.models_dir.mkdir(parents=True, exist_ok=True)
        self.logs_dir.mkdir(parents=True, exist_ok=True)
        
        # Model parameters
        self.input_shape = tuple(self.config['input_shape'])
        self.num_classes = self.config['num_classes'] 
        self.batch_size = self.config['batch_size']
        self.epochs = self.config['epochs']
        
        logger.info(f"Initialized trainer with config: {self.config}")
    
    def _load_config(self, config_path: str) -> Dict:
        """Load training configuration."""
        default_config = {
            'data_dir': './datasets/wildlife',
            'output_dir': './output',
            'input_shape': [224, 224, 3],
            'num_classes': 10,
            'batch_size': 32,
            'epochs': 50,
            'learning_rate': 0.001,
            'validation_split': 0.2,
            'test_split': 0.1,
            'use_transfer_learning': True,
            'base_model': 'MobileNetV2',
            'fine_tune_layers': 50,
            'data_augmentation': True,
            'class_weights': 'balanced',
            'early_stopping_patience': 10,
            'reduce_lr_patience': 5,
            'species_mapping': {
                'deer': 0,
                'bear': 1,
                'fox': 2,
                'wolf': 3,
                'raccoon': 4,
                'rabbit': 5,
                'bird': 6,
                'squirrel': 7,
                'coyote': 8,
                'unknown': 9
            }
        }
        
        if config_path and os.path.exists(config_path):
            with open(config_path, 'r') as f:
                user_config = json.load(f)
                default_config.update(user_config)
        
        return default_config
    
    def prepare_dataset(self) -> Tuple[tf.data.Dataset, tf.data.Dataset, tf.data.Dataset]:
        """
        Prepare training, validation, and test datasets.
        Supports both directory structure and annotation files.
        """
        logger.info("Preparing dataset...")
        
        # Check if we have a structured dataset directory
        if (self.data_dir / 'train').exists():
            return self._prepare_directory_dataset()
        elif (self.data_dir / 'annotations.json').exists():
            return self._prepare_annotated_dataset()
        else:
            return self._prepare_wildlife_insights_dataset()
    
    def _prepare_directory_dataset(self) -> Tuple[tf.data.Dataset, tf.data.Dataset, tf.data.Dataset]:
        """Prepare dataset from directory structure (train/val/test folders)."""
        
        # Data generators with augmentation
        train_datagen = ImageDataGenerator(
            rescale=1./255,
            rotation_range=20,
            width_shift_range=0.2,
            height_shift_range=0.2,
            horizontal_flip=True,
            zoom_range=0.2,
            shear_range=0.1,
            fill_mode='nearest'
        ) if self.config['data_augmentation'] else ImageDataGenerator(rescale=1./255)
        
        val_test_datagen = ImageDataGenerator(rescale=1./255)
        
        # Load datasets
        train_generator = train_datagen.flow_from_directory(
            self.data_dir / 'train',
            target_size=self.input_shape[:2],
            batch_size=self.batch_size,
            class_mode='categorical'
        )
        
        validation_generator = val_test_datagen.flow_from_directory(
            self.data_dir / 'validation',
            target_size=self.input_shape[:2],
            batch_size=self.batch_size,
            class_mode='categorical'
        )
        
        test_generator = val_test_datagen.flow_from_directory(
            self.data_dir / 'test',
            target_size=self.input_shape[:2],
            batch_size=self.batch_size,
            class_mode='categorical',
            shuffle=False
        )
        
        # Convert to tf.data.Dataset
        train_dataset = tf.data.Dataset.from_generator(
            lambda: train_generator,
            output_signature=(
                tf.TensorSpec(shape=(None,) + self.input_shape, dtype=tf.float32),
                tf.TensorSpec(shape=(None, self.num_classes), dtype=tf.float32)
            )
        )
        
        val_dataset = tf.data.Dataset.from_generator(
            lambda: validation_generator,
            output_signature=(
                tf.TensorSpec(shape=(None,) + self.input_shape, dtype=tf.float32),
                tf.TensorSpec(shape=(None, self.num_classes), dtype=tf.float32)
            )
        )
        
        test_dataset = tf.data.Dataset.from_generator(
            lambda: test_generator,
            output_signature=(
                tf.TensorSpec(shape=(None,) + self.input_shape, dtype=tf.float32),
                tf.TensorSpec(shape=(None, self.num_classes), dtype=tf.float32)
            )
        )
        
        # Store class indices for later use
        self.class_indices = train_generator.class_indices
        self.class_names = list(self.class_indices.keys())
        
        return train_dataset, val_dataset, test_dataset
    
    def _prepare_annotated_dataset(self) -> Tuple[tf.data.Dataset, tf.data.Dataset, tf.data.Dataset]:
        """Prepare dataset from annotation file (COCO-style or custom format)."""
        logger.info("Loading annotated dataset...")
        
        # Load annotations
        with open(self.data_dir / 'annotations.json', 'r') as f:
            annotations = json.load(f)
        
        # Extract image paths and labels
        image_paths = []
        labels = []
        
        for annotation in annotations['annotations']:
            image_path = self.data_dir / 'images' / annotation['image_name']
            if image_path.exists():
                image_paths.append(str(image_path))
                labels.append(annotation['species_id'])
        
        # Convert labels to categorical
        labels = tf.keras.utils.to_categorical(labels, num_classes=self.num_classes)
        
        # Split dataset
        X_train, X_temp, y_train, y_temp = train_test_split(
            image_paths, labels, 
            test_size=self.config['validation_split'] + self.config['test_split'],
            stratify=labels.argmax(axis=1),
            random_state=42
        )
        
        val_size = self.config['validation_split'] / (self.config['validation_split'] + self.config['test_split'])
        X_val, X_test, y_val, y_test = train_test_split(
            X_temp, y_temp,
            test_size=1-val_size,
            stratify=y_temp.argmax(axis=1),
            random_state=42
        )
        
        # Create tf.data datasets
        train_dataset = self._create_dataset_from_paths(X_train, y_train, training=True)
        val_dataset = self._create_dataset_from_paths(X_val, y_val, training=False)
        test_dataset = self._create_dataset_from_paths(X_test, y_test, training=False)
        
        return train_dataset, val_dataset, test_dataset
    
    def _prepare_wildlife_insights_dataset(self) -> Tuple[tf.data.Dataset, tf.data.Dataset, tf.data.Dataset]:
        """Prepare dataset compatible with Wildlife Insights and iNaturalist formats."""
        logger.info("Preparing Wildlife Insights compatible dataset...")
        
        # This would integrate with Wildlife Insights API or local data
        # For now, create a placeholder that can be extended
        
        # Look for CSV metadata file
        metadata_file = self.data_dir / 'metadata.csv'
        if metadata_file.exists():
            df = pd.read_csv(metadata_file)
            
            # Extract image paths and species
            image_paths = [str(self.data_dir / 'images' / fname) for fname in df['filename']]
            species = df['species'].values
            
            # Map species to indices
            unique_species = sorted(df['species'].unique())
            species_to_idx = {species: idx for idx, species in enumerate(unique_species)}
            labels = [species_to_idx[s] for s in species]
            
            # Convert to categorical
            labels = tf.keras.utils.to_categorical(labels, num_classes=len(unique_species))
            
            # Update config
            self.num_classes = len(unique_species)
            self.class_names = unique_species
            
            # Split and create datasets
            X_train, X_temp, y_train, y_temp = train_test_split(
                image_paths, labels, test_size=0.3, stratify=labels.argmax(axis=1), random_state=42
            )
            X_val, X_test, y_val, y_test = train_test_split(
                X_temp, y_temp, test_size=0.5, stratify=y_temp.argmax(axis=1), random_state=42
            )
            
            train_dataset = self._create_dataset_from_paths(X_train, y_train, training=True)
            val_dataset = self._create_dataset_from_paths(X_val, y_val, training=False)
            test_dataset = self._create_dataset_from_paths(X_test, y_test, training=False)
            
            return train_dataset, val_dataset, test_dataset
        
        else:
            raise ValueError("No suitable dataset format found. Please provide structured directories, annotations.json, or metadata.csv")
    
    def _create_dataset_from_paths(self, image_paths: List[str], labels: np.ndarray, training: bool = False) -> tf.data.Dataset:
        """Create tf.data.Dataset from image paths and labels."""
        
        def load_and_preprocess_image(path, label):
            image = tf.io.read_file(path)
            image = tf.image.decode_image(image, channels=3)
            image = tf.image.resize(image, self.input_shape[:2])
            image = tf.cast(image, tf.float32) / 255.0
            
            if training and self.config['data_augmentation']:
                # Apply random augmentations
                image = tf.image.random_flip_left_right(image)
                image = tf.image.random_brightness(image, 0.2)
                image = tf.image.random_contrast(image, 0.8, 1.2)
                image = tf.image.random_saturation(image, 0.8, 1.2)
            
            return image, label
        
        dataset = tf.data.Dataset.from_tensor_slices((image_paths, labels))
        dataset = dataset.map(load_and_preprocess_image, num_parallel_calls=tf.data.AUTOTUNE)
        dataset = dataset.batch(self.batch_size)
        dataset = dataset.prefetch(tf.data.AUTOTUNE)
        
        if training:
            dataset = dataset.shuffle(1000)
        
        return dataset
    
    def build_model(self) -> keras.Model:
        """Build the classification model with transfer learning."""
        logger.info("Building model...")
        
        if self.config['use_transfer_learning']:
            # Use pre-trained model as base
            base_model_name = self.config['base_model']
            
            if base_model_name == 'MobileNetV2':
                base_model = keras.applications.MobileNetV2(
                    input_shape=self.input_shape,
                    include_top=False,
                    weights='imagenet'
                )
            elif base_model_name == 'EfficientNetB0':
                base_model = keras.applications.EfficientNetB0(
                    input_shape=self.input_shape,
                    include_top=False,
                    weights='imagenet'
                )
            elif base_model_name == 'ResNet50':
                base_model = keras.applications.ResNet50(
                    input_shape=self.input_shape,
                    include_top=False,
                    weights='imagenet'
                )
            else:
                raise ValueError(f"Unsupported base model: {base_model_name}")
            
            # Freeze base model initially
            base_model.trainable = False
            
            # Add custom classifier head
            model = keras.Sequential([
                base_model,
                layers.GlobalAveragePooling2D(),
                layers.Dropout(0.3),
                layers.Dense(128, activation='relu'),
                layers.Dropout(0.2),
                layers.Dense(self.num_classes, activation='softmax')
            ])
            
        else:
            # Build custom CNN from scratch
            model = keras.Sequential([
                layers.Conv2D(32, 3, activation='relu', input_shape=self.input_shape),
                layers.MaxPooling2D(),
                layers.Conv2D(64, 3, activation='relu'),
                layers.MaxPooling2D(),
                layers.Conv2D(128, 3, activation='relu'),
                layers.MaxPooling2D(),
                layers.Flatten(),
                layers.Dense(128, activation='relu'),
                layers.Dropout(0.5),
                layers.Dense(self.num_classes, activation='softmax')
            ])
        
        # Compile model
        model.compile(
            optimizer=optimizers.Adam(learning_rate=self.config['learning_rate']),
            loss='categorical_crossentropy',
            metrics=['accuracy', 'top_3_accuracy']
        )
        
        self.model = model
        logger.info(f"Model built with {model.count_params()} parameters")
        
        return model
    
    def train(self, train_dataset: tf.data.Dataset, val_dataset: tf.data.Dataset) -> keras.callbacks.History:
        """Train the model."""
        logger.info("Starting training...")
        
        # Prepare callbacks
        callbacks_list = [
            callbacks.EarlyStopping(
                monitor='val_loss',
                patience=self.config['early_stopping_patience'],
                restore_best_weights=True
            ),
            callbacks.ReduceLROnPlateau(
                monitor='val_loss',
                factor=0.2,
                patience=self.config['reduce_lr_patience'],
                min_lr=1e-7
            ),
            callbacks.ModelCheckpoint(
                str(self.models_dir / 'best_model.h5'),
                monitor='val_accuracy',
                save_best_only=True,
                save_weights_only=False
            ),
            callbacks.TensorBoard(
                log_dir=str(self.logs_dir),
                histogram_freq=1,
                write_graph=True,
                write_images=True
            )
        ]
        
        # Train model
        history = self.model.fit(
            train_dataset,
            validation_data=val_dataset,
            epochs=self.epochs,
            callbacks=callbacks_list,
            verbose=1
        )
        
        self.history = history
        
        # Fine-tune if using transfer learning
        if self.config['use_transfer_learning'] and self.config['fine_tune_layers'] > 0:
            logger.info("Starting fine-tuning phase...")
            self._fine_tune_model(train_dataset, val_dataset)
        
        return history
    
    def _fine_tune_model(self, train_dataset: tf.data.Dataset, val_dataset: tf.data.Dataset):
        """Fine-tune the pre-trained base model."""
        # Unfreeze the base model
        base_model = self.model.layers[0]
        base_model.trainable = True
        
        # Freeze early layers, fine-tune later layers
        fine_tune_at = len(base_model.layers) - self.config['fine_tune_layers']
        for layer in base_model.layers[:fine_tune_at]:
            layer.trainable = False
        
        # Recompile with lower learning rate
        self.model.compile(
            optimizer=optimizers.Adam(learning_rate=self.config['learning_rate'] / 10),
            loss='categorical_crossentropy',
            metrics=['accuracy', 'top_3_accuracy']
        )
        
        # Continue training
        fine_tune_history = self.model.fit(
            train_dataset,
            validation_data=val_dataset,
            epochs=self.epochs // 2,  # Fewer epochs for fine-tuning
            initial_epoch=len(self.history.history['loss']),
            callbacks=[
                callbacks.EarlyStopping(monitor='val_loss', patience=5, restore_best_weights=True),
                callbacks.ModelCheckpoint(
                    str(self.models_dir / 'fine_tuned_model.h5'),
                    monitor='val_accuracy',
                    save_best_only=True
                )
            ],
            verbose=1
        )
        
        # Merge histories
        for key in self.history.history:
            self.history.history[key].extend(fine_tune_history.history[key])
    
    def evaluate(self, test_dataset: tf.data.Dataset) -> Dict:
        """Evaluate the trained model."""
        logger.info("Evaluating model...")
        
        # Get test accuracy
        test_loss, test_accuracy, test_top3_accuracy = self.model.evaluate(test_dataset, verbose=0)
        
        # Get predictions for detailed analysis
        y_pred = []
        y_true = []
        
        for images, labels in test_dataset:
            predictions = self.model.predict(images, verbose=0)
            y_pred.extend(np.argmax(predictions, axis=1))
            y_true.extend(np.argmax(labels, axis=1))
        
        # Generate classification report
        class_report = classification_report(
            y_true, y_pred, 
            target_names=self.class_names,
            output_dict=True
        )
        
        # Generate confusion matrix
        cm = confusion_matrix(y_true, y_pred)
        
        results = {
            'test_loss': test_loss,
            'test_accuracy': test_accuracy,
            'test_top3_accuracy': test_top3_accuracy,
            'classification_report': class_report,
            'confusion_matrix': cm.tolist(),
            'class_names': self.class_names
        }
        
        # Save results
        with open(self.output_dir / 'evaluation_results.json', 'w') as f:
            json.dump(results, f, indent=2)
        
        logger.info(f"Test accuracy: {test_accuracy:.4f}")
        logger.info(f"Test top-3 accuracy: {test_top3_accuracy:.4f}")
        
        return results
    
    def save_model(self, model_name: str = None):
        """Save the trained model in multiple formats."""
        if not model_name:
            model_name = f"wildlife_classifier_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        
        # Save full model
        model_path = self.models_dir / f"{model_name}.h5"
        self.model.save(str(model_path))
        logger.info(f"Model saved to {model_path}")
        
        # Save model architecture and weights separately
        with open(self.models_dir / f"{model_name}_architecture.json", 'w') as f:
            f.write(self.model.to_json())
        
        self.model.save_weights(str(self.models_dir / f"{model_name}_weights.h5"))
        
        # Save metadata
        metadata = {
            'model_name': model_name,
            'input_shape': self.input_shape,
            'num_classes': self.num_classes,
            'class_names': self.class_names,
            'training_config': self.config,
            'training_date': datetime.now().isoformat()
        }
        
        with open(self.models_dir / f"{model_name}_metadata.json", 'w') as f:
            json.dump(metadata, f, indent=2)
        
        return str(model_path)
    
    def plot_training_history(self):
        """Plot training history."""
        if not self.history:
            logger.warning("No training history available")
            return
        
        fig, axes = plt.subplots(2, 2, figsize=(15, 10))
        
        # Accuracy
        axes[0, 0].plot(self.history.history['accuracy'], label='Training Accuracy')
        axes[0, 0].plot(self.history.history['val_accuracy'], label='Validation Accuracy')
        axes[0, 0].set_title('Model Accuracy')
        axes[0, 0].set_xlabel('Epoch')
        axes[0, 0].set_ylabel('Accuracy')
        axes[0, 0].legend()
        
        # Loss
        axes[0, 1].plot(self.history.history['loss'], label='Training Loss')
        axes[0, 1].plot(self.history.history['val_loss'], label='Validation Loss')
        axes[0, 1].set_title('Model Loss')
        axes[0, 1].set_xlabel('Epoch')
        axes[0, 1].set_ylabel('Loss')
        axes[0, 1].legend()
        
        # Top-3 Accuracy
        if 'top_3_accuracy' in self.history.history:
            axes[1, 0].plot(self.history.history['top_3_accuracy'], label='Training Top-3 Accuracy')
            axes[1, 0].plot(self.history.history['val_top_3_accuracy'], label='Validation Top-3 Accuracy')
            axes[1, 0].set_title('Model Top-3 Accuracy')
            axes[1, 0].set_xlabel('Epoch')
            axes[1, 0].set_ylabel('Top-3 Accuracy')
            axes[1, 0].legend()
        
        # Learning Rate
        if hasattr(self.model.optimizer, 'learning_rate'):
            lr_history = [float(self.model.optimizer.learning_rate) for _ in range(len(self.history.history['loss']))]
            axes[1, 1].plot(lr_history, label='Learning Rate')
            axes[1, 1].set_title('Learning Rate')
            axes[1, 1].set_xlabel('Epoch')
            axes[1, 1].set_ylabel('Learning Rate')
            axes[1, 1].set_yscale('log')
            axes[1, 1].legend()
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'training_history.png', dpi=300, bbox_inches='tight')
        plt.show()


def main():
    parser = argparse.ArgumentParser(description='Train wildlife classification model')
    parser.add_argument('--config', type=str, help='Configuration file path')
    parser.add_argument('--data-dir', type=str, help='Dataset directory')
    parser.add_argument('--output-dir', type=str, help='Output directory')
    parser.add_argument('--epochs', type=int, default=50, help='Number of training epochs')
    parser.add_argument('--batch-size', type=int, default=32, help='Batch size')
    parser.add_argument('--model-name', type=str, help='Model name for saving')
    
    args = parser.parse_args()
    
    # Update config with command line arguments
    config_updates = {}
    if args.data_dir:
        config_updates['data_dir'] = args.data_dir
    if args.output_dir:
        config_updates['output_dir'] = args.output_dir
    if args.epochs:
        config_updates['epochs'] = args.epochs
    if args.batch_size:
        config_updates['batch_size'] = args.batch_size
    
    # Initialize trainer
    trainer = WildlifeModelTrainer(args.config)
    
    # Update config with CLI args
    trainer.config.update(config_updates)
    
    try:
        # Prepare dataset
        train_dataset, val_dataset, test_dataset = trainer.prepare_dataset()
        
        # Build and train model
        model = trainer.build_model()
        history = trainer.train(train_dataset, val_dataset)
        
        # Evaluate model
        results = trainer.evaluate(test_dataset)
        
        # Save model
        model_path = trainer.save_model(args.model_name)
        
        # Plot training history
        trainer.plot_training_history()
        
        logger.info("Training completed successfully!")
        logger.info(f"Model saved to: {model_path}")
        logger.info(f"Test accuracy: {results['test_accuracy']:.4f}")
        
    except Exception as e:
        logger.error(f"Training failed: {str(e)}")
        raise


if __name__ == "__main__":
    main()