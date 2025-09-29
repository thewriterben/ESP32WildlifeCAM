#!/usr/bin/env python3
"""
Wildlife Classification Model Evaluation Script

This script evaluates trained wildlife classification models with comprehensive
performance metrics, visualization, and ESP32 edge deployment analysis.
"""

import os
import json
import argparse
import logging
import time
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Any
import numpy as np
import pandas as pd

# Deep Learning frameworks
import tensorflow as tf
from tensorflow import keras
from sklearn.metrics import (
    classification_report, confusion_matrix, roc_auc_score,
    precision_recall_curve, roc_curve, f1_score, accuracy_score
)

# Visualization
import matplotlib.pyplot as plt
import seaborn as sns
import plotly.graph_objects as go
import plotly.express as px
from plotly.subplots import make_subplots

# Computer Vision
import cv2
from PIL import Image

# Performance monitoring
import psutil
import memory_profiler

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class WildlifeModelEvaluator:
    """
    Comprehensive model evaluation for wildlife classification models.
    
    Features:
    - Performance metrics calculation
    - Confusion matrix and classification reports  
    - ROC curves and precision-recall analysis
    - Species-specific accuracy analysis
    - Edge device performance testing
    - Memory and inference time profiling
    """
    
    def __init__(self, config_path: Optional[str] = None):
        self.config = self._load_config(config_path)
        self.model = None
        self.class_names = []
        self.results = {}
        
        # Set up output directories
        self.output_dir = Path(self.config['output_dir'])
        self.reports_dir = self.output_dir / 'evaluation_reports'
        self.plots_dir = self.output_dir / 'plots'
        
        self.reports_dir.mkdir(parents=True, exist_ok=True)
        self.plots_dir.mkdir(parents=True, exist_ok=True)
        
        logger.info(f"Evaluator initialized with output dir: {self.output_dir}")
    
    def _load_config(self, config_path: Optional[str]) -> Dict:
        """Load evaluation configuration."""
        default_config = {
            'output_dir': './output/evaluation',
            'batch_size': 32,
            'target_accuracy': 0.85,
            'target_inference_time_ms': 2000,
            'confidence_threshold': 0.5,
            'metrics': ['accuracy', 'precision', 'recall', 'f1_score'],
            'generate_plots': True,
            'save_predictions': True,
            'edge_profiling': True
        }
        
        if config_path and os.path.exists(config_path):
            import yaml
            with open(config_path, 'r') as f:
                user_config = yaml.safe_load(f)
                if 'evaluation' in user_config:
                    default_config.update(user_config['evaluation'])
        
        return default_config
    
    def load_model(self, model_path: str) -> keras.Model:
        """Load trained model for evaluation."""
        logger.info(f"Loading model from {model_path}")
        
        if not os.path.exists(model_path):
            raise FileNotFoundError(f"Model file not found: {model_path}")
        
        self.model = keras.models.load_model(model_path)
        
        # Try to load class names from metadata
        metadata_path = Path(model_path).parent / f"{Path(model_path).stem}_metadata.json"
        if metadata_path.exists():
            with open(metadata_path, 'r') as f:
                metadata = json.load(f)
                self.class_names = metadata.get('class_names', [])
        
        if not self.class_names:
            # Generate generic class names
            num_classes = self.model.output_shape[-1]
            self.class_names = [f"class_{i}" for i in range(num_classes)]
        
        logger.info(f"Model loaded with {len(self.class_names)} classes")
        return self.model
    
    def load_test_dataset(self, data_path: str) -> tf.data.Dataset:
        """Load test dataset for evaluation."""
        logger.info(f"Loading test dataset from {data_path}")
        
        data_dir = Path(data_path)
        
        if (data_dir / 'test').exists():
            # Directory structure format
            return self._load_directory_dataset(data_dir / 'test')
        elif (data_dir / 'test_annotations.json').exists():
            # Annotation format
            return self._load_annotated_dataset(data_dir, 'test_annotations.json')
        else:
            raise ValueError(f"No test dataset found in {data_path}")
    
    def _load_directory_dataset(self, test_dir: Path) -> tf.data.Dataset:
        """Load test dataset from directory structure."""
        from tensorflow.keras.preprocessing.image import ImageDataGenerator
        
        test_datagen = ImageDataGenerator(rescale=1./255)
        
        test_generator = test_datagen.flow_from_directory(
            str(test_dir),
            target_size=(224, 224),  # Default size, should match model input
            batch_size=self.config['batch_size'],
            class_mode='categorical',
            shuffle=False
        )
        
        # Update class names from generator
        self.class_names = list(test_generator.class_indices.keys())
        
        dataset = tf.data.Dataset.from_generator(
            lambda: test_generator,
            output_signature=(
                tf.TensorSpec(shape=(None, 224, 224, 3), dtype=tf.float32),
                tf.TensorSpec(shape=(None, len(self.class_names)), dtype=tf.float32)
            )
        )
        
        return dataset
    
    def _load_annotated_dataset(self, data_dir: Path, annotation_file: str) -> tf.data.Dataset:
        """Load test dataset from annotations."""
        with open(data_dir / annotation_file, 'r') as f:
            annotations = json.load(f)
        
        # Extract image paths and labels
        image_paths = []
        labels = []
        
        for annotation in annotations['annotations']:
            image_path = data_dir / 'images' / annotation['image_name']
            if image_path.exists():
                image_paths.append(str(image_path))
                labels.append(annotation['species_id'])
        
        # Convert to categorical
        labels = tf.keras.utils.to_categorical(labels, num_classes=len(self.class_names))
        
        return self._create_dataset_from_paths(image_paths, labels)
    
    def _create_dataset_from_paths(self, image_paths: List[str], labels: np.ndarray) -> tf.data.Dataset:
        """Create tf.data.Dataset from paths and labels."""
        def load_and_preprocess_image(path, label):
            image = tf.io.read_file(path)
            image = tf.image.decode_image(image, channels=3)
            image = tf.image.resize(image, [224, 224])  # Default size
            image = tf.cast(image, tf.float32) / 255.0
            return image, label
        
        dataset = tf.data.Dataset.from_tensor_slices((image_paths, labels))
        dataset = dataset.map(load_and_preprocess_image, num_parallel_calls=tf.data.AUTOTUNE)
        dataset = dataset.batch(self.config['batch_size'])
        dataset = dataset.prefetch(tf.data.AUTOTUNE)
        
        return dataset
    
    def evaluate_model(self, test_dataset: tf.data.Dataset) -> Dict[str, Any]:
        """Comprehensive model evaluation."""
        logger.info("Starting comprehensive model evaluation...")
        
        # Basic evaluation
        basic_metrics = self._evaluate_basic_metrics(test_dataset)
        
        # Detailed predictions analysis
        prediction_analysis = self._analyze_predictions(test_dataset)
        
        # Performance profiling
        performance_metrics = self._profile_performance(test_dataset)
        
        # Species-specific analysis
        species_analysis = self._analyze_species_performance(test_dataset)
        
        # Edge deployment analysis
        edge_analysis = self._analyze_edge_compatibility()
        
        # Combine all results
        self.results = {
            'basic_metrics': basic_metrics,
            'prediction_analysis': prediction_analysis,
            'performance_metrics': performance_metrics,
            'species_analysis': species_analysis,
            'edge_analysis': edge_analysis,
            'class_names': self.class_names,
            'evaluation_config': self.config
        }
        
        # Generate reports and visualizations
        if self.config.get('generate_plots', True):
            self._generate_visualizations()
        
        # Save results
        self._save_results()
        
        logger.info("Model evaluation completed successfully")
        return self.results
    
    def _evaluate_basic_metrics(self, test_dataset: tf.data.Dataset) -> Dict[str, float]:
        """Evaluate basic model metrics."""
        logger.info("Evaluating basic metrics...")
        
        # Get model predictions
        y_true = []
        y_pred = []
        y_pred_proba = []
        
        for images, labels in test_dataset:
            predictions = self.model.predict(images, verbose=0)
            y_pred_proba.extend(predictions)
            y_pred.extend(np.argmax(predictions, axis=1))
            y_true.extend(np.argmax(labels, axis=1))
        
        y_true = np.array(y_true)
        y_pred = np.array(y_pred)
        y_pred_proba = np.array(y_pred_proba)
        
        # Calculate metrics
        accuracy = accuracy_score(y_true, y_pred)
        f1 = f1_score(y_true, y_pred, average='weighted')
        
        # Multi-class ROC AUC
        try:
            roc_auc = roc_auc_score(y_true, y_pred_proba, multi_class='ovr', average='weighted')
        except ValueError:
            roc_auc = 0.0  # In case of single class
        
        # Top-k accuracy
        top_3_accuracy = self._calculate_top_k_accuracy(y_true, y_pred_proba, k=3)
        top_5_accuracy = self._calculate_top_k_accuracy(y_true, y_pred_proba, k=5)
        
        metrics = {
            'accuracy': float(accuracy),
            'f1_score': float(f1),
            'roc_auc': float(roc_auc),
            'top_3_accuracy': float(top_3_accuracy),
            'top_5_accuracy': float(top_5_accuracy),
            'total_samples': len(y_true)
        }
        
        logger.info(f"Accuracy: {accuracy:.4f}, F1: {f1:.4f}, ROC AUC: {roc_auc:.4f}")
        return metrics
    
    def _calculate_top_k_accuracy(self, y_true, y_pred_proba, k=3):
        """Calculate top-k accuracy."""
        top_k_pred = np.argsort(y_pred_proba, axis=1)[:, -k:]
        correct = sum(y_true[i] in top_k_pred[i] for i in range(len(y_true)))
        return correct / len(y_true)
    
    def _analyze_predictions(self, test_dataset: tf.data.Dataset) -> Dict[str, Any]:
        """Analyze model predictions in detail."""
        logger.info("Analyzing predictions...")
        
        y_true = []
        y_pred = []
        y_pred_proba = []
        
        for images, labels in test_dataset:
            predictions = self.model.predict(images, verbose=0)
            y_pred_proba.extend(predictions)
            y_pred.extend(np.argmax(predictions, axis=1))
            y_true.extend(np.argmax(labels, axis=1))
        
        # Classification report
        class_report = classification_report(
            y_true, y_pred, 
            target_names=self.class_names,
            output_dict=True
        )
        
        # Confusion matrix
        cm = confusion_matrix(y_true, y_pred)
        
        # Confidence statistics
        confidences = np.max(y_pred_proba, axis=1)
        confidence_stats = {
            'mean': float(np.mean(confidences)),
            'std': float(np.std(confidences)),
            'min': float(np.min(confidences)),
            'max': float(np.max(confidences)),
            'median': float(np.median(confidences))
        }
        
        return {
            'classification_report': class_report,
            'confusion_matrix': cm.tolist(),
            'confidence_statistics': confidence_stats,
            'predictions': {
                'y_true': y_true,
                'y_pred': y_pred,
                'y_pred_proba': y_pred_proba
            }
        }
    
    def _profile_performance(self, test_dataset: tf.data.Dataset) -> Dict[str, Any]:
        """Profile model performance for inference time and memory usage."""
        logger.info("Profiling performance...")
        
        # Warm up
        for images, _ in test_dataset.take(1):
            _ = self.model.predict(images, verbose=0)
        
        # Measure inference time
        inference_times = []
        memory_usage = []
        
        for i, (images, _) in enumerate(test_dataset.take(10)):  # Sample 10 batches
            # Memory before
            process = psutil.Process()
            memory_before = process.memory_info().rss / 1024 / 1024  # MB
            
            # Time inference
            start_time = time.time()
            predictions = self.model.predict(images, verbose=0)
            end_time = time.time()
            
            # Memory after
            memory_after = process.memory_info().rss / 1024 / 1024  # MB
            
            batch_time = (end_time - start_time) * 1000  # Convert to ms
            batch_memory = memory_after - memory_before
            
            inference_times.append(batch_time)
            memory_usage.append(batch_memory)
        
        # Calculate statistics
        avg_inference_time = np.mean(inference_times)
        std_inference_time = np.std(inference_times)
        avg_memory_usage = np.mean(memory_usage)
        
        # Per-image metrics
        batch_size = self.config['batch_size']
        per_image_time = avg_inference_time / batch_size
        
        # Model size
        model_size_mb = self._get_model_size_mb()
        
        performance_metrics = {
            'avg_inference_time_ms': float(avg_inference_time),
            'std_inference_time_ms': float(std_inference_time),
            'per_image_inference_time_ms': float(per_image_time),
            'avg_memory_usage_mb': float(avg_memory_usage),
            'model_size_mb': float(model_size_mb),
            'meets_target_time': per_image_time <= self.config['target_inference_time_ms'],
            'target_time_ms': self.config['target_inference_time_ms']
        }
        
        logger.info(f"Avg inference time per image: {per_image_time:.2f}ms, Model size: {model_size_mb:.2f}MB")
        return performance_metrics
    
    def _get_model_size_mb(self) -> float:
        """Calculate model size in MB."""
        # Count parameters
        total_params = self.model.count_params()
        
        # Estimate size (assuming float32)
        # Each parameter is 4 bytes (float32)
        estimated_size_bytes = total_params * 4
        estimated_size_mb = estimated_size_bytes / (1024 * 1024)
        
        return estimated_size_mb
    
    def _analyze_species_performance(self, test_dataset: tf.data.Dataset) -> Dict[str, Any]:
        """Analyze performance per species."""
        logger.info("Analyzing species-specific performance...")
        
        y_true = []
        y_pred = []
        
        for images, labels in test_dataset:
            predictions = self.model.predict(images, verbose=0)
            y_pred.extend(np.argmax(predictions, axis=1))
            y_true.extend(np.argmax(labels, axis=1))
        
        # Per-class metrics
        species_metrics = {}
        
        for i, species_name in enumerate(self.class_names):
            # Get indices for this species
            species_indices = np.where(np.array(y_true) == i)[0]
            
            if len(species_indices) > 0:
                species_true = np.array(y_true)[species_indices]
                species_pred = np.array(y_pred)[species_indices]
                
                # Calculate metrics for this species
                species_accuracy = accuracy_score(species_true, species_pred)
                species_f1 = f1_score(species_true == i, species_pred == i)
                
                species_metrics[species_name] = {
                    'accuracy': float(species_accuracy),
                    'f1_score': float(species_f1),
                    'sample_count': len(species_indices),
                    'correct_predictions': int(np.sum(species_true == species_pred))
                }
        
        # Find best and worst performing species
        if species_metrics:
            best_species = max(species_metrics.keys(), key=lambda x: species_metrics[x]['accuracy'])
            worst_species = min(species_metrics.keys(), key=lambda x: species_metrics[x]['accuracy'])
        else:
            best_species = worst_species = None
        
        return {
            'per_species_metrics': species_metrics,
            'best_performing_species': best_species,
            'worst_performing_species': worst_species,
        }
    
    def _analyze_edge_compatibility(self) -> Dict[str, Any]:
        """Analyze compatibility with ESP32 edge deployment."""
        logger.info("Analyzing edge deployment compatibility...")
        
        # Model complexity analysis
        total_params = self.model.count_params()
        trainable_params = sum([tf.keras.backend.count_params(w) for w in self.model.trainable_weights])
        non_trainable_params = total_params - trainable_params
        
        # Estimate memory requirements
        model_size_mb = self._get_model_size_mb()
        
        # ESP32 constraints
        esp32_memory_limit = 8  # MB PSRAM
        esp32_flash_limit = 16  # MB typical
        
        # Layer analysis
        layer_types = {}
        for layer in self.model.layers:
            layer_type = type(layer).__name__
            layer_types[layer_type] = layer_types.get(layer_type, 0) + 1
        
        # Compatibility assessment
        memory_compatible = model_size_mb <= esp32_memory_limit
        complexity_score = min(1.0, esp32_memory_limit / model_size_mb) if model_size_mb > 0 else 1.0
        
        edge_analysis = {
            'total_parameters': int(total_params),
            'trainable_parameters': int(trainable_params),
            'non_trainable_parameters': int(non_trainable_params),
            'model_size_mb': float(model_size_mb),
            'esp32_memory_limit_mb': esp32_memory_limit,
            'memory_compatible': memory_compatible,
            'complexity_score': float(complexity_score),
            'layer_distribution': layer_types,
            'tflite_conversion_ready': True  # Assume compatible unless proven otherwise
        }
        
        return edge_analysis
    
    def _generate_visualizations(self):
        """Generate evaluation visualizations."""
        logger.info("Generating visualizations...")
        
        if 'prediction_analysis' not in self.results:
            return
        
        # Confusion Matrix
        self._plot_confusion_matrix()
        
        # Performance metrics
        self._plot_performance_metrics()
        
        # Species performance
        self._plot_species_performance()
        
        # ROC curves (if applicable)
        self._plot_roc_curves()
    
    def _plot_confusion_matrix(self):
        """Plot confusion matrix."""
        cm = np.array(self.results['prediction_analysis']['confusion_matrix'])
        
        plt.figure(figsize=(12, 10))
        sns.heatmap(cm, annot=True, fmt='d', cmap='Blues',
                    xticklabels=self.class_names,
                    yticklabels=self.class_names)
        plt.title('Confusion Matrix')
        plt.xlabel('Predicted')
        plt.ylabel('Actual')
        plt.xticks(rotation=45, ha='right')
        plt.yticks(rotation=0)
        plt.tight_layout()
        plt.savefig(self.plots_dir / 'confusion_matrix.png', dpi=300, bbox_inches='tight')
        plt.close()
    
    def _plot_performance_metrics(self):
        """Plot performance metrics."""
        metrics = self.results['basic_metrics']
        
        # Bar plot of metrics
        metric_names = ['Accuracy', 'F1 Score', 'ROC AUC', 'Top-3 Acc', 'Top-5 Acc']
        metric_values = [
            metrics['accuracy'], metrics['f1_score'], metrics['roc_auc'],
            metrics['top_3_accuracy'], metrics['top_5_accuracy']
        ]
        
        plt.figure(figsize=(10, 6))
        bars = plt.bar(metric_names, metric_values, color=['skyblue', 'lightgreen', 'lightcoral', 'gold', 'plum'])
        plt.ylim(0, 1)
        plt.title('Model Performance Metrics')
        plt.ylabel('Score')
        
        # Add value labels on bars
        for bar, value in zip(bars, metric_values):
            plt.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.01,
                    f'{value:.3f}', ha='center', va='bottom')
        
        # Add target line
        plt.axhline(y=self.config['target_accuracy'], color='red', linestyle='--', 
                   label=f'Target ({self.config["target_accuracy"]:.2f})')
        plt.legend()
        
        plt.xticks(rotation=45)
        plt.tight_layout()
        plt.savefig(self.plots_dir / 'performance_metrics.png', dpi=300, bbox_inches='tight')
        plt.close()
    
    def _plot_species_performance(self):
        """Plot per-species performance."""
        if 'species_analysis' not in self.results:
            return
            
        species_metrics = self.results['species_analysis']['per_species_metrics']
        
        if not species_metrics:
            return
        
        species_names = list(species_metrics.keys())
        accuracies = [species_metrics[name]['accuracy'] for name in species_names]
        sample_counts = [species_metrics[name]['sample_count'] for name in species_names]
        
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10))
        
        # Accuracy per species
        bars1 = ax1.bar(species_names, accuracies, color='lightblue')
        ax1.set_title('Accuracy per Species')
        ax1.set_ylabel('Accuracy')
        ax1.set_ylim(0, 1)
        ax1.axhline(y=self.config['target_accuracy'], color='red', linestyle='--', alpha=0.7)
        
        # Add value labels
        for bar, value in zip(bars1, accuracies):
            ax1.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.01,
                    f'{value:.3f}', ha='center', va='bottom', fontsize=8)
        
        # Sample count per species
        bars2 = ax2.bar(species_names, sample_counts, color='lightgreen')
        ax2.set_title('Sample Count per Species')
        ax2.set_ylabel('Number of Samples')
        ax2.set_xlabel('Species')
        
        # Add value labels
        for bar, value in zip(bars2, sample_counts):
            ax2.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.5,
                    str(value), ha='center', va='bottom', fontsize=8)
        
        # Rotate x-axis labels
        for ax in [ax1, ax2]:
            ax.tick_params(axis='x', rotation=45)
        
        plt.tight_layout()
        plt.savefig(self.plots_dir / 'species_performance.png', dpi=300, bbox_inches='tight')
        plt.close()
    
    def _plot_roc_curves(self):
        """Plot ROC curves for multi-class classification."""
        if 'prediction_analysis' not in self.results:
            return
            
        pred_data = self.results['prediction_analysis']['predictions']
        y_true = np.array(pred_data['y_true'])
        y_pred_proba = np.array(pred_data['y_pred_proba'])
        
        # Convert to binary format for ROC calculation
        y_true_binary = tf.keras.utils.to_categorical(y_true, num_classes=len(self.class_names))
        
        plt.figure(figsize=(10, 8))
        
        for i, class_name in enumerate(self.class_names):
            fpr, tpr, _ = roc_curve(y_true_binary[:, i], y_pred_proba[:, i])
            auc = roc_auc_score(y_true_binary[:, i], y_pred_proba[:, i])
            plt.plot(fpr, tpr, label=f'{class_name} (AUC = {auc:.3f})')
        
        plt.plot([0, 1], [0, 1], 'k--', label='Random')
        plt.xlabel('False Positive Rate')
        plt.ylabel('True Positive Rate')
        plt.title('ROC Curves - Multi-class Classification')
        plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
        plt.tight_layout()
        plt.savefig(self.plots_dir / 'roc_curves.png', dpi=300, bbox_inches='tight')
        plt.close()
    
    def _save_results(self):
        """Save evaluation results to files."""
        logger.info("Saving evaluation results...")
        
        # Save complete results as JSON
        results_file = self.reports_dir / 'evaluation_results.json'
        
        # Convert numpy arrays to lists for JSON serialization
        json_results = self._prepare_results_for_json(self.results.copy())
        
        with open(results_file, 'w') as f:
            json.dump(json_results, f, indent=2)
        
        # Save summary report
        self._generate_summary_report()
        
        logger.info(f"Results saved to {self.reports_dir}")
    
    def _prepare_results_for_json(self, results: Dict) -> Dict:
        """Prepare results for JSON serialization."""
        def convert_numpy(obj):
            if isinstance(obj, np.ndarray):
                return obj.tolist()
            elif isinstance(obj, np.integer):
                return int(obj)
            elif isinstance(obj, np.floating):
                return float(obj)
            elif isinstance(obj, dict):
                return {key: convert_numpy(value) for key, value in obj.items()}
            elif isinstance(obj, list):
                return [convert_numpy(item) for item in obj]
            else:
                return obj
        
        return convert_numpy(results)
    
    def _generate_summary_report(self):
        """Generate a human-readable summary report."""
        summary_file = self.reports_dir / 'evaluation_summary.txt'
        
        with open(summary_file, 'w') as f:
            f.write("ESP32 Wildlife Camera - Model Evaluation Summary\n")
            f.write("=" * 50 + "\n\n")
            
            # Basic metrics
            if 'basic_metrics' in self.results:
                metrics = self.results['basic_metrics']
                f.write("PERFORMANCE METRICS\n")
                f.write("-" * 20 + "\n")
                f.write(f"Accuracy: {metrics['accuracy']:.4f}\n")
                f.write(f"F1 Score: {metrics['f1_score']:.4f}\n")
                f.write(f"ROC AUC: {metrics['roc_auc']:.4f}\n")
                f.write(f"Top-3 Accuracy: {metrics['top_3_accuracy']:.4f}\n")
                f.write(f"Top-5 Accuracy: {metrics['top_5_accuracy']:.4f}\n")
                f.write(f"Total Samples: {metrics['total_samples']}\n\n")
            
            # Performance profiling
            if 'performance_metrics' in self.results:
                perf = self.results['performance_metrics']
                f.write("PERFORMANCE PROFILING\n")
                f.write("-" * 22 + "\n")
                f.write(f"Model Size: {perf['model_size_mb']:.2f} MB\n")
                f.write(f"Avg Inference Time per Image: {perf['per_image_inference_time_ms']:.2f} ms\n")
                f.write(f"Target Time: {perf['target_time_ms']} ms\n")
                f.write(f"Meets Target: {'✓' if perf['meets_target_time'] else '✗'}\n")
                f.write(f"Memory Usage: {perf['avg_memory_usage_mb']:.2f} MB\n\n")
            
            # Edge compatibility
            if 'edge_analysis' in self.results:
                edge = self.results['edge_analysis']
                f.write("ESP32 EDGE DEPLOYMENT ANALYSIS\n")
                f.write("-" * 32 + "\n")
                f.write(f"Total Parameters: {edge['total_parameters']:,}\n")
                f.write(f"Model Size: {edge['model_size_mb']:.2f} MB\n")
                f.write(f"ESP32 Memory Limit: {edge['esp32_memory_limit_mb']} MB\n")
                f.write(f"Memory Compatible: {'✓' if edge['memory_compatible'] else '✗'}\n")
                f.write(f"Complexity Score: {edge['complexity_score']:.3f}\n\n")
            
            # Species performance
            if 'species_analysis' in self.results:
                species = self.results['species_analysis']
                f.write("SPECIES PERFORMANCE\n")
                f.write("-" * 19 + "\n")
                if species['best_performing_species']:
                    f.write(f"Best Performing: {species['best_performing_species']}\n")
                if species['worst_performing_species']:
                    f.write(f"Worst Performing: {species['worst_performing_species']}\n")
                f.write("\nPer-Species Accuracy:\n")
                for species_name, metrics in species['per_species_metrics'].items():
                    f.write(f"  {species_name}: {metrics['accuracy']:.4f} ({metrics['sample_count']} samples)\n")
            
            f.write("\n" + "=" * 50 + "\n")
            f.write("Evaluation completed successfully!\n")


def main():
    parser = argparse.ArgumentParser(description='Evaluate wildlife classification model')
    parser.add_argument('--model', type=str, required=True, help='Path to trained model')
    parser.add_argument('--data', type=str, required=True, help='Path to test dataset')
    parser.add_argument('--config', type=str, help='Configuration file path')
    parser.add_argument('--output', type=str, default='./output/evaluation', help='Output directory')
    
    args = parser.parse_args()
    
    # Initialize evaluator
    evaluator = WildlifeModelEvaluator(args.config)
    
    # Override output directory if specified
    if args.output:
        evaluator.config['output_dir'] = args.output
        evaluator.output_dir = Path(args.output)
        evaluator.reports_dir = evaluator.output_dir / 'evaluation_reports'
        evaluator.plots_dir = evaluator.output_dir / 'plots'
        evaluator.reports_dir.mkdir(parents=True, exist_ok=True)
        evaluator.plots_dir.mkdir(parents=True, exist_ok=True)
    
    try:
        # Load model and test data
        model = evaluator.load_model(args.model)
        test_dataset = evaluator.load_test_dataset(args.data)
        
        # Run evaluation
        results = evaluator.evaluate_model(test_dataset)
        
        # Print summary
        print("\n" + "=" * 60)
        print("EVALUATION SUMMARY")
        print("=" * 60)
        print(f"Accuracy: {results['basic_metrics']['accuracy']:.4f}")
        print(f"F1 Score: {results['basic_metrics']['f1_score']:.4f}")
        print(f"Model Size: {results['performance_metrics']['model_size_mb']:.2f} MB")
        print(f"Inference Time: {results['performance_metrics']['per_image_inference_time_ms']:.2f} ms/image")
        print(f"ESP32 Compatible: {'✓' if results['edge_analysis']['memory_compatible'] else '✗'}")
        print(f"Target Accuracy ({evaluator.config['target_accuracy']:.2f}): {'✓' if results['basic_metrics']['accuracy'] >= evaluator.config['target_accuracy'] else '✗'}")
        print("=" * 60)
        
        logger.info("Model evaluation completed successfully!")
        
    except Exception as e:
        logger.error(f"Evaluation failed: {str(e)}")
        raise


if __name__ == "__main__":
    main()