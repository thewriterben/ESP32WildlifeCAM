#!/usr/bin/env python3
"""
ML Pipeline Validation Script

End-to-end validation of the wildlife classification ML pipeline,
including data loading, model training, conversion, and deployment testing.
"""

import os
import sys
import json
import logging
import tempfile
from pathlib import Path
from typing import Dict, List, Optional, Any
import numpy as np

# Add current directory to path for local imports
sys.path.append(str(Path(__file__).parent))

from train_model import WildlifeModelTrainer
from convert_model import ESP32ModelConverter
from evaluate_model import WildlifeModelEvaluator
from edge_optimization import ESP32ModelOptimizer
from preprocessing import WildlifeImagePreprocessor, DatasetProcessor

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class MLPipelineValidator:
    """
    Comprehensive ML pipeline validation.
    
    Features:
    - End-to-end pipeline testing
    - Component integration validation
    - Performance benchmarking
    - ESP32 deployment readiness check
    - Automated smoke tests
    """
    
    def __init__(self, config_path: Optional[str] = None):
        self.config_path = config_path
        self.validation_results = {}
        self.temp_dir = None
        
        # Initialize temporary directory
        self.temp_dir = Path(tempfile.mkdtemp(prefix='ml_pipeline_validation_'))
        logger.info(f"Using temporary directory: {self.temp_dir}")
    
    def run_full_validation(self) -> Dict[str, Any]:
        """Run complete pipeline validation."""
        logger.info("Starting full ML pipeline validation...")
        
        validation_steps = [
            ('environment_check', self._validate_environment),
            ('preprocessing_test', self._test_preprocessing),
            ('training_smoke_test', self._test_training_smoke),
            ('conversion_test', self._test_model_conversion),
            ('evaluation_test', self._test_model_evaluation),
            ('optimization_test', self._test_edge_optimization),
            ('integration_test', self._test_end_to_end_integration),
            ('performance_benchmark', self._benchmark_pipeline_performance)
        ]
        
        overall_success = True
        
        for step_name, step_function in validation_steps:
            try:
                logger.info(f"Running validation step: {step_name}")
                result = step_function()
                self.validation_results[step_name] = {
                    'success': result,
                    'status': 'PASS' if result else 'FAIL'
                }
                
                if not result:
                    overall_success = False
                    logger.error(f"Validation step failed: {step_name}")
                else:
                    logger.info(f"Validation step passed: {step_name}")
                    
            except Exception as e:
                logger.error(f"Error in validation step {step_name}: {e}")
                self.validation_results[step_name] = {
                    'success': False,
                    'status': 'ERROR',
                    'error': str(e)
                }
                overall_success = False
        
        self.validation_results['overall_success'] = overall_success
        self.validation_results['overall_status'] = 'PASS' if overall_success else 'FAIL'
        
        # Generate report
        self._generate_validation_report()
        
        logger.info(f"Pipeline validation completed. Overall status: {'PASS' if overall_success else 'FAIL'}")
        return self.validation_results
    
    def _validate_environment(self) -> bool:
        """Validate environment and dependencies."""
        logger.info("Validating environment...")
        
        try:
            # Check Python version
            import sys
            python_version = sys.version_info
            if python_version.major != 3 or python_version.minor < 8:
                logger.error(f"Python 3.8+ required, found {python_version.major}.{python_version.minor}")
                return False
            
            # Check critical dependencies
            critical_deps = [
                'tensorflow', 'numpy', 'opencv-python', 'scikit-learn',
                'matplotlib', 'pandas', 'PIL', 'yaml'
            ]
            
            missing_deps = []
            for dep in critical_deps:
                try:
                    if dep == 'opencv-python':
                        import cv2
                    elif dep == 'PIL':
                        from PIL import Image
                    elif dep == 'yaml':
                        import yaml
                    else:
                        __import__(dep)
                except ImportError:
                    missing_deps.append(dep)
            
            if missing_deps:
                logger.error(f"Missing dependencies: {missing_deps}")
                return False
            
            # Check TensorFlow functionality
            import tensorflow as tf
            tf_version = tf.__version__
            logger.info(f"TensorFlow version: {tf_version}")
            
            # Test basic TF operations
            test_tensor = tf.constant([1, 2, 3])
            test_result = tf.reduce_sum(test_tensor)
            
            if test_result.numpy() != 6:
                logger.error("TensorFlow basic operation test failed")
                return False
            
            # Check GPU availability (optional)
            gpus = tf.config.list_physical_devices('GPU')
            logger.info(f"GPUs available: {len(gpus)}")
            
            logger.info("Environment validation passed")
            return True
            
        except Exception as e:
            logger.error(f"Environment validation failed: {e}")
            return False
    
    def _test_preprocessing(self) -> bool:
        """Test preprocessing functionality."""
        logger.info("Testing preprocessing...")
        
        try:
            # Create synthetic test images
            test_images_dir = self.temp_dir / 'test_images'
            test_images_dir.mkdir(exist_ok=True)
            
            # Generate test images
            import cv2
            test_species = ['deer', 'bear', 'fox']
            
            for species in test_species:
                species_dir = test_images_dir / species
                species_dir.mkdir(exist_ok=True)
                
                for i in range(5):
                    # Create synthetic image
                    img = np.random.randint(0, 255, (300, 400, 3), dtype=np.uint8)
                    img_path = species_dir / f"{species}_{i:03d}.jpg"
                    cv2.imwrite(str(img_path), img)
            
            # Test preprocessor
            preprocessor = WildlifeImagePreprocessor()
            
            # Test single image preprocessing
            test_image_path = test_images_dir / 'deer' / 'deer_001.jpg'
            processed_img = preprocessor.preprocess_image(test_image_path)
            
            if processed_img is None:
                logger.error("Image preprocessing returned None")
                return False
            
            # Check output shape
            expected_shape = (224, 224, 3)  # Default target size
            if processed_img.shape != expected_shape:
                logger.error(f"Unexpected output shape: {processed_img.shape}, expected: {expected_shape}")
                return False
            
            # Test batch preprocessing
            image_paths = list(test_images_dir.rglob('*.jpg'))[:10]
            processed_batch = preprocessor.batch_preprocess(image_paths[:3])
            
            if len(processed_batch) != 3:
                logger.error(f"Batch preprocessing failed: expected 3, got {len(processed_batch)}")
                return False
            
            # Test dataset processor
            dataset_processor = DatasetProcessor()
            
            # Test dataset organization
            organized_dir = self.temp_dir / 'organized_dataset'
            summary = dataset_processor.organize_dataset(test_images_dir, organized_dir)
            
            if not summary or summary['total_images'] == 0:
                logger.error("Dataset organization failed")
                return False
            
            # Test dataset validation
            validation_report = dataset_processor.validate_dataset(organized_dir)
            
            if not validation_report['valid']:
                logger.error(f"Dataset validation failed: {validation_report['errors']}")
                return False
            
            logger.info("Preprocessing tests passed")
            return True
            
        except Exception as e:
            logger.error(f"Preprocessing test failed: {e}")
            return False
    
    def _test_training_smoke(self) -> bool:
        """Run a minimal training smoke test."""
        logger.info("Running training smoke test...")
        
        try:
            # Create minimal training config
            train_config = {
                'data_dir': str(self.temp_dir / 'organized_dataset'),
                'output_dir': str(self.temp_dir / 'training_output'),
                'input_shape': [64, 64, 3],  # Small for quick testing
                'num_classes': 3,
                'batch_size': 2,
                'epochs': 2,  # Minimal epochs
                'learning_rate': 0.01,
                'use_transfer_learning': False,  # Faster without transfer learning
                'data_augmentation': False  # Disable for speed
            }
            
            # Initialize trainer
            trainer = WildlifeModelTrainer()
            trainer.config.update(train_config)
            trainer.input_shape = tuple(train_config['input_shape'])
            trainer.num_classes = train_config['num_classes']
            trainer.batch_size = train_config['batch_size']
            trainer.epochs = train_config['epochs']
            
            # Setup directories
            trainer.output_dir = Path(train_config['output_dir'])
            trainer.models_dir = trainer.output_dir / 'models'
            trainer.logs_dir = trainer.output_dir / 'logs'
            
            trainer.output_dir.mkdir(parents=True, exist_ok=True)
            trainer.models_dir.mkdir(parents=True, exist_ok=True)
            trainer.logs_dir.mkdir(parents=True, exist_ok=True)
            
            # Create synthetic dataset for training
            import tensorflow as tf
            
            # Generate synthetic training data
            num_samples = 10
            X_train = np.random.random((num_samples, 64, 64, 3)).astype(np.float32)
            y_train = tf.keras.utils.to_categorical(
                np.random.randint(0, 3, num_samples), 
                num_classes=3
            )
            
            X_val = np.random.random((6, 64, 64, 3)).astype(np.float32)
            y_val = tf.keras.utils.to_categorical(
                np.random.randint(0, 3, 6), 
                num_classes=3
            )
            
            # Create datasets
            train_dataset = tf.data.Dataset.from_tensor_slices((X_train, y_train)).batch(2)
            val_dataset = tf.data.Dataset.from_tensor_slices((X_val, y_val)).batch(2)
            
            # Build and train model
            model = trainer.build_model()
            
            if model is None:
                logger.error("Model building failed")
                return False
            
            # Quick training
            history = trainer.train(train_dataset, val_dataset)
            
            if history is None:
                logger.error("Training failed")
                return False
            
            # Check if model learned something
            if len(history.history['loss']) != train_config['epochs']:
                logger.error(f"Training history incomplete: {len(history.history['loss'])} vs {train_config['epochs']}")
                return False
            
            # Save model
            model_path = trainer.save_model('smoke_test_model')
            
            if not Path(model_path).exists():
                logger.error("Model saving failed")
                return False
            
            logger.info("Training smoke test passed")
            return True
            
        except Exception as e:
            logger.error(f"Training smoke test failed: {e}")
            return False
    
    def _test_model_conversion(self) -> bool:
        """Test TensorFlow Lite model conversion."""
        logger.info("Testing model conversion...")
        
        try:
            # Find the smoke test model
            model_path = self.temp_dir / 'training_output' / 'models' / 'smoke_test_model.h5'
            
            if not model_path.exists():
                logger.error("Smoke test model not found for conversion testing")
                return False
            
            # Initialize converter
            converter = ESP32ModelConverter()
            
            # Load model
            model = converter.load_model(str(model_path))
            
            if model is None:
                logger.error("Model loading for conversion failed")
                return False
            
            # Test different quantization strategies
            strategies = ['none', 'dynamic']  # Test basic strategies
            
            for strategy in strategies:
                try:
                    logger.info(f"Testing {strategy} quantization...")
                    
                    tflite_model = converter.convert_to_tflite(
                        quantization=strategy,
                        optimization_target='size'
                    )
                    
                    if tflite_model is None or len(tflite_model) == 0:
                        logger.error(f"Conversion failed for {strategy} quantization")
                        return False
                    
                    # Test converted model
                    import tensorflow as tf
                    interpreter = tf.lite.Interpreter(model_content=tflite_model)
                    interpreter.allocate_tensors()
                    
                    input_details = interpreter.get_input_details()
                    output_details = interpreter.get_output_details()
                    
                    if not input_details or not output_details:
                        logger.error(f"Invalid converted model for {strategy}")
                        return False
                    
                    # Test inference
                    input_shape = input_details[0]['shape']
                    test_input = np.random.random(input_shape).astype(np.float32)
                    
                    interpreter.set_tensor(input_details[0]['index'], test_input)
                    interpreter.invoke()
                    output = interpreter.get_tensor(output_details[0]['index'])
                    
                    if output is None:
                        logger.error(f"Inference failed for {strategy} quantized model")
                        return False
                    
                    logger.info(f"{strategy} quantization test passed")
                    
                except Exception as e:
                    logger.error(f"Conversion test failed for {strategy}: {e}")
                    return False
            
            logger.info("Model conversion tests passed")
            return True
            
        except Exception as e:
            logger.error(f"Model conversion test failed: {e}")
            return False
    
    def _test_model_evaluation(self) -> bool:
        """Test model evaluation functionality.""" 
        logger.info("Testing model evaluation...")
        
        try:
            # Use the smoke test model
            model_path = self.temp_dir / 'training_output' / 'models' / 'smoke_test_model.h5'
            
            if not model_path.exists():
                logger.error("Smoke test model not found for evaluation testing")
                return False
            
            # Initialize evaluator
            evaluator = WildlifeModelEvaluator()
            
            # Load model
            model = evaluator.load_model(str(model_path))
            
            if model is None:
                logger.error("Model loading for evaluation failed")
                return False
            
            # Create synthetic test dataset
            import tensorflow as tf
            
            num_test_samples = 12
            X_test = np.random.random((num_test_samples, 64, 64, 3)).astype(np.float32)
            y_test = tf.keras.utils.to_categorical(
                np.random.randint(0, 3, num_test_samples), 
                num_classes=3
            )
            
            test_dataset = tf.data.Dataset.from_tensor_slices((X_test, y_test)).batch(4)
            
            # Run evaluation
            evaluator.config['generate_plots'] = False  # Skip plots for testing
            evaluator.output_dir = self.temp_dir / 'evaluation_output'
            evaluator.reports_dir = evaluator.output_dir / 'evaluation_reports'
            evaluator.plots_dir = evaluator.output_dir / 'plots'
            
            evaluator.reports_dir.mkdir(parents=True, exist_ok=True)
            evaluator.plots_dir.mkdir(parents=True, exist_ok=True)
            
            results = evaluator.evaluate_model(test_dataset)
            
            if not results:
                logger.error("Model evaluation returned no results")
                return False
            
            # Check required result components
            required_keys = ['basic_metrics', 'prediction_analysis', 'performance_metrics']
            for key in required_keys:
                if key not in results:
                    logger.error(f"Missing result component: {key}")
                    return False
            
            # Check basic metrics
            basic_metrics = results['basic_metrics']
            if 'accuracy' not in basic_metrics:
                logger.error("Missing accuracy in basic metrics")
                return False
            
            if not (0 <= basic_metrics['accuracy'] <= 1):
                logger.error(f"Invalid accuracy value: {basic_metrics['accuracy']}")
                return False
            
            logger.info("Model evaluation tests passed")
            return True
            
        except Exception as e:
            logger.error(f"Model evaluation test failed: {e}")
            return False
    
    def _test_edge_optimization(self) -> bool:
        """Test edge optimization functionality."""
        logger.info("Testing edge optimization...")
        
        try:
            # Use the smoke test model
            model_path = self.temp_dir / 'training_output' / 'models' / 'smoke_test_model.h5'
            
            if not model_path.exists():
                logger.error("Smoke test model not found for optimization testing")
                return False
            
            # Initialize optimizer
            optimizer = ESP32ModelOptimizer()
            
            # Load model
            model = optimizer.load_model(str(model_path))
            
            if model is None:
                logger.error("Model loading for optimization failed")
                return False
            
            # Test complexity analysis
            complexity_analysis = optimizer.analyze_model_complexity()
            
            if not complexity_analysis:
                logger.error("Complexity analysis failed")
                return False
            
            required_keys = ['parameters', 'memory_usage', 'esp32_compatibility']
            for key in required_keys:
                if key not in complexity_analysis:
                    logger.error(f"Missing complexity analysis component: {key}")
                    return False
            
            # Test tensor arena estimation
            arena_analysis = optimizer.estimate_tensor_arena_size()
            
            if not arena_analysis:
                logger.error("Arena analysis failed")
                return False
            
            if 'estimated_arena_size_kb' not in arena_analysis:
                logger.error("Missing arena size estimation")
                return False
            
            # Test performance profiling
            performance_profile = optimizer.profile_inference_performance(num_runs=5)
            
            if not performance_profile:
                logger.error("Performance profiling failed")
                return False
            
            if 'host_performance' not in performance_profile:
                logger.error("Missing host performance metrics")
                return False
            
            logger.info("Edge optimization tests passed")
            return True
            
        except Exception as e:
            logger.error(f"Edge optimization test failed: {e}")
            return False
    
    def _test_end_to_end_integration(self) -> bool:
        """Test end-to-end pipeline integration."""
        logger.info("Testing end-to-end integration...")
        
        try:
            # This is a comprehensive test that runs through the entire pipeline
            # with minimal data to ensure all components work together
            
            # 1. Create test data
            test_data_dir = self.temp_dir / 'integration_test_data'
            self._create_minimal_test_dataset(test_data_dir)
            
            # 2. Test preprocessing -> training -> conversion -> evaluation flow
            integration_output_dir = self.temp_dir / 'integration_output'
            integration_output_dir.mkdir(exist_ok=True)
            
            # Create integration config
            integration_config = {
                'data_dir': str(test_data_dir),
                'output_dir': str(integration_output_dir),
                'input_shape': [64, 64, 3],
                'num_classes': 2,
                'batch_size': 2,
                'epochs': 1,
                'use_transfer_learning': False
            }
            
            # Initialize all components
            trainer = WildlifeModelTrainer()
            trainer.config.update(integration_config)
            trainer._setup_directories()
            
            converter = ESP32ModelConverter()
            evaluator = WildlifeModelEvaluator()
            optimizer = ESP32ModelOptimizer()
            
            # Run pipeline steps
            
            # Step 1: Prepare dataset (simplified)
            train_data, val_data, test_data = self._create_synthetic_datasets()
            
            # Step 2: Build and train model
            model = trainer.build_model()
            history = trainer.train(train_data, val_data)
            model_path = trainer.save_model('integration_test_model')
            
            # Step 3: Convert model
            converter.load_model(model_path)
            tflite_model = converter.convert_to_tflite(quantization='dynamic')
            
            # Step 4: Evaluate model
            evaluator.load_model(model_path)
            evaluation_results = evaluator.evaluate_model(test_data)
            
            # Step 5: Optimize for ESP32
            optimizer.load_model(model_path)
            optimization_results = optimizer.analyze_model_complexity()
            
            # Verify all steps completed successfully
            checks = [
                (model is not None, "Model building"),
                (history is not None, "Model training"),
                (Path(model_path).exists(), "Model saving"),
                (tflite_model is not None, "Model conversion"),
                (evaluation_results is not None, "Model evaluation"),
                (optimization_results is not None, "Model optimization")
            ]
            
            for check, description in checks:
                if not check:
                    logger.error(f"Integration test failed at: {description}")
                    return False
            
            logger.info("End-to-end integration test passed")
            return True
            
        except Exception as e:
            logger.error(f"End-to-end integration test failed: {e}")
            return False
    
    def _create_minimal_test_dataset(self, output_dir: Path):
        """Create minimal test dataset for integration testing."""
        output_dir.mkdir(parents=True, exist_ok=True)
        
        # Create directory structure
        for split in ['train', 'validation', 'test']:
            split_dir = output_dir / split
            split_dir.mkdir(exist_ok=True)
            
            for species in ['deer', 'bear']:
                species_dir = split_dir / species
                species_dir.mkdir(exist_ok=True)
                
                # Create synthetic images
                import cv2
                for i in range(3):  # 3 images per species per split
                    img = np.random.randint(0, 255, (64, 64, 3), dtype=np.uint8)
                    img_path = species_dir / f"{species}_{i:03d}.jpg"
                    cv2.imwrite(str(img_path), img)
    
    def _create_synthetic_datasets(self):
        """Create synthetic TensorFlow datasets for testing."""
        import tensorflow as tf
        
        # Training data
        X_train = np.random.random((8, 64, 64, 3)).astype(np.float32)
        y_train = tf.keras.utils.to_categorical(np.random.randint(0, 2, 8), 2)
        train_data = tf.data.Dataset.from_tensor_slices((X_train, y_train)).batch(2)
        
        # Validation data
        X_val = np.random.random((4, 64, 64, 3)).astype(np.float32)
        y_val = tf.keras.utils.to_categorical(np.random.randint(0, 2, 4), 2)
        val_data = tf.data.Dataset.from_tensor_slices((X_val, y_val)).batch(2)
        
        # Test data
        X_test = np.random.random((6, 64, 64, 3)).astype(np.float32)
        y_test = tf.keras.utils.to_categorical(np.random.randint(0, 2, 6), 2)
        test_data = tf.data.Dataset.from_tensor_slices((X_test, y_test)).batch(2)
        
        return train_data, val_data, test_data
    
    def _benchmark_pipeline_performance(self) -> bool:
        """Benchmark overall pipeline performance."""
        logger.info("Benchmarking pipeline performance...")
        
        try:
            # This would run performance benchmarks
            # For now, we'll do a basic timing test
            
            import time
            import tempfile
            
            # Time key operations
            benchmark_results = {}
            
            # Benchmark preprocessing
            start_time = time.time()
            preprocessor = WildlifeImagePreprocessor()
            # Create and process test image
            test_img = np.random.randint(0, 255, (300, 400, 3), dtype=np.uint8)
            with tempfile.NamedTemporaryFile(suffix='.jpg', delete=False) as tmp_file:
                import cv2
                cv2.imwrite(tmp_file.name, test_img)
                processed = preprocessor.preprocess_image(tmp_file.name)
                os.unlink(tmp_file.name)
            
            benchmark_results['preprocessing_time_ms'] = (time.time() - start_time) * 1000
            
            # Benchmark model inference (using existing model)
            model_path = self.temp_dir / 'integration_output' / 'models' / 'integration_test_model.h5'
            if model_path.exists():
                import tensorflow as tf
                model = tf.keras.models.load_model(str(model_path))
                
                start_time = time.time()
                test_input = np.random.random((1, 64, 64, 3)).astype(np.float32)
                prediction = model.predict(test_input, verbose=0)
                benchmark_results['inference_time_ms'] = (time.time() - start_time) * 1000
            
            # Log benchmark results
            logger.info(f"Benchmark results: {benchmark_results}")
            
            # Check if performance is reasonable
            if 'preprocessing_time_ms' in benchmark_results:
                if benchmark_results['preprocessing_time_ms'] > 5000:  # 5 seconds
                    logger.warning("Preprocessing time is high")
            
            if 'inference_time_ms' in benchmark_results:
                if benchmark_results['inference_time_ms'] > 2000:  # 2 seconds
                    logger.warning("Inference time is high")
            
            logger.info("Pipeline performance benchmark completed")
            return True
            
        except Exception as e:
            logger.error(f"Pipeline performance benchmark failed: {e}")
            return False
    
    def _generate_validation_report(self):
        """Generate comprehensive validation report."""
        report_path = self.temp_dir / 'validation_report.json'
        
        report = {
            'validation_summary': {
                'overall_status': self.validation_results.get('overall_status', 'UNKNOWN'),
                'total_tests': len(self.validation_results) - 2,  # Exclude overall_* keys
                'passed_tests': sum(1 for k, v in self.validation_results.items() 
                                  if k not in ['overall_success', 'overall_status'] and v.get('success', False)),
                'failed_tests': sum(1 for k, v in self.validation_results.items() 
                                  if k not in ['overall_success', 'overall_status'] and not v.get('success', True))
            },
            'detailed_results': self.validation_results,
            'recommendations': self._generate_recommendations()
        }
        
        with open(report_path, 'w') as f:
            json.dump(report, f, indent=2)
        
        logger.info(f"Validation report saved to: {report_path}")
    
    def _generate_recommendations(self) -> List[str]:
        """Generate recommendations based on validation results."""
        recommendations = []
        
        for test_name, result in self.validation_results.items():
            if test_name in ['overall_success', 'overall_status']:
                continue
                
            if not result.get('success', True):
                if test_name == 'environment_check':
                    recommendations.append("Fix environment setup - check Python version and install missing dependencies")
                elif test_name == 'preprocessing_test':
                    recommendations.append("Fix preprocessing issues - check image processing libraries")
                elif test_name == 'training_smoke_test':
                    recommendations.append("Fix training pipeline - check TensorFlow setup and model configuration")
                elif test_name == 'conversion_test':
                    recommendations.append("Fix model conversion - check TensorFlow Lite compatibility")
                elif test_name == 'evaluation_test':
                    recommendations.append("Fix evaluation pipeline - check metrics calculation")
                elif test_name == 'optimization_test':
                    recommendations.append("Fix ESP32 optimization - check edge deployment utilities")
                elif test_name == 'integration_test':
                    recommendations.append("Fix pipeline integration - check component compatibility")
        
        if not recommendations:
            recommendations.append("All validation tests passed - pipeline is ready for production use")
        
        return recommendations
    
    def cleanup(self):
        """Clean up temporary files."""
        if self.temp_dir and self.temp_dir.exists():
            import shutil
            try:
                shutil.rmtree(str(self.temp_dir))
                logger.info(f"Cleaned up temporary directory: {self.temp_dir}")
            except Exception as e:
                logger.warning(f"Failed to cleanup temporary directory: {e}")


def main():
    """Main function for command-line usage."""
    import argparse
    
    parser = argparse.ArgumentParser(description='Validate ML pipeline')
    parser.add_argument('--config', type=str, help='Configuration file path')
    parser.add_argument('--quick', action='store_true', help='Run quick validation (skip intensive tests)')
    parser.add_argument('--output', type=str, help='Output directory for validation results')
    
    args = parser.parse_args()
    
    # Initialize validator
    validator = MLPipelineValidator(args.config)
    
    try:
        # Run validation
        results = validator.run_full_validation()
        
        # Print summary
        print("\n" + "=" * 60)
        print("ML PIPELINE VALIDATION SUMMARY")
        print("=" * 60)
        
        summary = results.get('validation_summary', {})
        if 'validation_summary' not in results:
            # Calculate summary
            summary = {
                'overall_status': results.get('overall_status', 'UNKNOWN'),
                'total_tests': len(results) - 2,
                'passed_tests': sum(1 for k, v in results.items() 
                                  if k not in ['overall_success', 'overall_status'] and v.get('success', False)),
                'failed_tests': sum(1 for k, v in results.items() 
                                  if k not in ['overall_success', 'overall_status'] and not v.get('success', True))
            }
        
        print(f"Overall Status: {summary['overall_status']}")
        print(f"Tests Passed: {summary['passed_tests']}/{summary['total_tests']}")
        
        print("\nDetailed Results:")
        for test_name, result in results.items():
            if test_name in ['overall_success', 'overall_status']:
                continue
            status = result.get('status', 'UNKNOWN')
            print(f"  {test_name}: {status}")
            if 'error' in result:
                print(f"    Error: {result['error']}")
        
        print("=" * 60)
        
        # Copy results to output directory if specified
        if args.output:
            output_dir = Path(args.output)
            output_dir.mkdir(parents=True, exist_ok=True)
            
            # Copy validation report
            import shutil
            src_report = validator.temp_dir / 'validation_report.json'
            if src_report.exists():
                dst_report = output_dir / 'validation_report.json'
                shutil.copy2(str(src_report), str(dst_report))
                print(f"Validation report copied to: {dst_report}")
        
        # Return appropriate exit code
        sys.exit(0 if results.get('overall_success', False) else 1)
        
    except KeyboardInterrupt:
        logger.info("Validation interrupted by user")
        sys.exit(130)
    except Exception as e:
        logger.error(f"Validation failed with unexpected error: {e}")
        sys.exit(1)
    finally:
        # Cleanup
        validator.cleanup()


if __name__ == "__main__":
    main()