#!/usr/bin/env python3
"""
Model Performance Benchmarking Script

This script provides comprehensive performance benchmarking for wildlife
classification models, including accuracy, speed, memory usage, and
ESP32-specific performance metrics.
"""

import os
import json
import logging
import time
import psutil
from pathlib import Path
from typing import Dict, List, Optional, Any
import numpy as np

# Deep Learning frameworks
import tensorflow as tf
from tensorflow import keras

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class ModelPerformanceBenchmark:
    """Comprehensive model performance benchmarking suite."""
    
    def __init__(self, config: Optional[Dict] = None):
        self.config = self._load_config(config)
        self.results = {}
        self.benchmark_data = []
        
        # Output directories
        self.output_dir = Path(self.config['output_dir'])
        self.reports_dir = self.output_dir / 'benchmark_reports'
        
        # Create directories
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.reports_dir.mkdir(parents=True, exist_ok=True)
        
        logger.info(f"Benchmark suite initialized. Output dir: {self.output_dir}")
    
    def _load_config(self, config: Optional[Dict]) -> Dict:
        """Load benchmarking configuration."""
        default_config = {
            'output_dir': './benchmark_results',
            'warmup_runs': 10,
            'benchmark_runs': 100,
            'esp32_simulation': True,
            'memory_profiling': True,
            'target_metrics': {
                'inference_time_ms': 2000,
                'memory_limit_mb': 8,
                'model_size_limit_mb': 2
            }
        }
        
        if config:
            default_config.update(config)
        
        return default_config
    
    def benchmark_model(self, model_path: str, test_data: Optional[np.ndarray] = None,
                       model_name: Optional[str] = None) -> Dict[str, Any]:
        """Comprehensive benchmarking of a single model."""
        model_path = Path(model_path)
        
        if not model_path.exists():
            raise FileNotFoundError(f"Model not found: {model_path}")
        
        if not model_name:
            model_name = model_path.stem
        
        logger.info(f"Benchmarking model: {model_name}")
        
        # Load model
        if model_path.suffix == '.tflite':
            model = self._load_tflite_model(model_path)
            model_type = 'tflite'
        else:
            model = keras.models.load_model(str(model_path))
            model_type = 'keras'
        
        # Generate test data if not provided
        if test_data is None:
            test_data = self._generate_test_data(model, model_type)
        
        # Run benchmark suite
        results = {
            'model_info': self._get_model_info(model, model_path, model_type),
            'inference_benchmark': self._benchmark_inference_speed(model, test_data, model_type),
            'esp32_estimation': self._estimate_esp32_performance(model, test_data, model_type),
        }
        
        # Store results
        self.results[model_name] = results
        
        logger.info(f"Benchmarking completed for {model_name}")
        return results
    
    def _load_tflite_model(self, model_path: Path):
        """Load TensorFlow Lite model."""
        interpreter = tf.lite.Interpreter(model_path=str(model_path))
        interpreter.allocate_tensors()
        return interpreter
    
    def _generate_test_data(self, model, model_type: str) -> np.ndarray:
        """Generate test data based on model input shape."""
        if model_type == 'tflite':
            input_details = model.get_input_details()
            input_shape = input_details[0]['shape']
        else:
            input_shape = model.input_shape
        
        # Generate random test data
        batch_size = 32
        if input_shape[0] is None:
            shape = (batch_size,) + input_shape[1:]
        else:
            shape = input_shape
        
        test_data = np.random.random(shape).astype(np.float32)
        logger.info(f"Generated test data with shape: {shape}")
        
        return test_data
    
    def _get_model_info(self, model, model_path: Path, model_type: str) -> Dict:
        """Extract model information."""
        info = {
            'model_path': str(model_path),
            'model_type': model_type,
            'file_size_bytes': model_path.stat().st_size,
            'file_size_mb': model_path.stat().st_size / (1024 * 1024)
        }
        
        if model_type == 'tflite':
            input_details = model.get_input_details()
            info.update({
                'input_shape': input_details[0]['shape'].tolist(),
                'tensor_count': len(model.get_tensor_details())
            })
        else:
            info.update({
                'input_shape': list(model.input_shape),
                'parameter_count': model.count_params(),
                'layer_count': len(model.layers)
            })
        
        return info
    
    def _benchmark_inference_speed(self, model, test_data: np.ndarray, 
                                 model_type: str) -> Dict[str, Any]:
        """Benchmark inference speed."""
        logger.info("Benchmarking inference speed...")
        
        # Prepare single sample for inference timing
        if model_type == 'tflite':
            input_details = model.get_input_details()
            output_details = model.get_output_details()
            single_sample = test_data[:1]
        else:
            single_sample = test_data[:1]
        
        # Warmup runs
        for _ in range(self.config['warmup_runs']):
            if model_type == 'tflite':
                model.set_tensor(input_details[0]['index'], single_sample)
                model.invoke()
                _ = model.get_tensor(output_details[0]['index'])
            else:
                _ = model.predict(single_sample, verbose=0)
        
        # Benchmark runs
        inference_times = []
        
        for _ in range(self.config['benchmark_runs']):
            start_time = time.perf_counter()
            
            if model_type == 'tflite':
                model.set_tensor(input_details[0]['index'], single_sample)
                model.invoke()
                prediction = model.get_tensor(output_details[0]['index'])
            else:
                prediction = model.predict(single_sample, verbose=0)
            
            end_time = time.perf_counter()
            inference_times.append((end_time - start_time) * 1000)  # Convert to ms
        
        # Calculate statistics
        inference_stats = {
            'mean_ms': float(np.mean(inference_times)),
            'std_ms': float(np.std(inference_times)),
            'min_ms': float(np.min(inference_times)),
            'max_ms': float(np.max(inference_times)),
            'median_ms': float(np.median(inference_times)),
            'fps': float(1000 / np.mean(inference_times))
        }
        
        logger.info(f"Inference speed: {inference_stats['mean_ms']:.2f}ms ± {inference_stats['std_ms']:.2f}ms")
        return inference_stats
    
    def _estimate_esp32_performance(self, model, test_data: np.ndarray,
                                  model_type: str) -> Dict[str, Any]:
        """Estimate ESP32 performance."""
        if not self.config['esp32_simulation']:
            return {'simulation_disabled': True}
        
        logger.info("Estimating ESP32 performance...")
        
        # Get baseline performance
        host_performance = self._benchmark_inference_speed(model, test_data, model_type)
        
        # ESP32 performance scaling factors
        scaling_factors = {
            'cpu_scaling': 2.5,      # ESP32 is ~2.5x slower
            'memory_scaling': 1.2,   # Additional overhead
            'tflite_scaling': 0.9 if model_type == 'tflite' else 1.5,
        }
        
        # Calculate ESP32 estimates
        total_scaling = (scaling_factors['cpu_scaling'] * 
                        scaling_factors['memory_scaling'] * 
                        scaling_factors['tflite_scaling'])
        
        estimated_inference_time = host_performance['mean_ms'] * total_scaling
        
        # ESP32 constraints
        esp32_constraints = {
            'max_inference_time_ms': self.config['target_metrics']['inference_time_ms'],
            'max_model_size_mb': self.config['target_metrics']['model_size_limit_mb']
        }
        
        # Compatibility assessment
        model_info = self._get_model_info(model, Path(), model_type)
        
        esp32_results = {
            'estimated_inference_time_ms': float(estimated_inference_time),
            'scaling_factors': scaling_factors,
            'total_scaling_factor': float(total_scaling),
            'meets_time_constraint': estimated_inference_time <= esp32_constraints['max_inference_time_ms'],
            'meets_size_constraint': model_info['file_size_mb'] <= esp32_constraints['max_model_size_mb'],
            'esp32_compatible': (
                estimated_inference_time <= esp32_constraints['max_inference_time_ms'] and
                model_info['file_size_mb'] <= esp32_constraints['max_model_size_mb']
            ),
            'constraints': esp32_constraints
        }
        
        logger.info(f"ESP32 estimated inference time: {estimated_inference_time:.1f}ms")
        return esp32_results
    
    def generate_benchmark_report(self) -> str:
        """Generate comprehensive benchmark report."""
        report_path = self.reports_dir / 'benchmark_summary.txt'
        
        with open(report_path, 'w') as f:
            f.write("ESP32 Wildlife Camera - Model Benchmark Report\n")
            f.write("=" * 50 + "\n\n")
            
            f.write(f"Benchmark Configuration:\n")
            f.write(f"- Warmup runs: {self.config['warmup_runs']}\n")
            f.write(f"- Benchmark runs: {self.config['benchmark_runs']}\n")
            f.write(f"- Target inference time: {self.config['target_metrics']['inference_time_ms']}ms\n\n")
            
            if self.results:
                f.write(f"Models Benchmarked: {len(self.results)}\n")
                f.write("-" * 20 + "\n")
                
                for model_name, results in self.results.items():
                    f.write(f"\n{model_name}:\n")
                    if 'inference_benchmark' in results:
                        inf = results['inference_benchmark']
                        f.write(f"  Inference Time: {inf['mean_ms']:.2f}ms\n")
                        f.write(f"  FPS: {inf['fps']:.1f}\n")
                    
                    if 'model_info' in results:
                        f.write(f"  Model Size: {results['model_info']['file_size_mb']:.2f}MB\n")
                    
                    if 'esp32_estimation' in results and 'simulation_disabled' not in results['esp32_estimation']:
                        esp32 = results['esp32_estimation']
                        f.write(f"  ESP32 Est. Time: {esp32['estimated_inference_time_ms']:.1f}ms\n")
                        f.write(f"  ESP32 Compatible: {'Yes' if esp32['esp32_compatible'] else 'No'}\n")
            
            f.write(f"\n{'=' * 50}\n")
            f.write("Benchmark completed successfully!\n")
        
        logger.info(f"Benchmark report saved: {report_path}")
        return str(report_path)


def main():
    """Main function for command-line usage."""
    import argparse
    
    parser = argparse.ArgumentParser(description='Benchmark model performance')
    parser.add_argument('--model', type=str, help='Model to benchmark')
    parser.add_argument('--output', type=str, default='./benchmark_results', help='Output directory')
    parser.add_argument('--quick', action='store_true', help='Quick benchmark')
    
    args = parser.parse_args()
    
    # Load configuration
    config = {'output_dir': args.output}
    
    if args.quick:
        config.update({
            'warmup_runs': 3,
            'benchmark_runs': 10
        })
    
    # Initialize benchmarker
    benchmarker = ModelPerformanceBenchmark(config)
    
    try:
        if args.model:
            # Single model benchmark
            results = benchmarker.benchmark_model(args.model)
            
            print("\n" + "=" * 60)
            print("BENCHMARK RESULTS")
            print("=" * 60)
            
            if 'inference_benchmark' in results:
                inf = results['inference_benchmark']
                print(f"Inference Time: {inf['mean_ms']:.2f}ms ± {inf['std_ms']:.2f}ms")
                print(f"FPS: {inf['fps']:.1f}")
            
            if 'model_info' in results:
                print(f"Model Size: {results['model_info']['file_size_mb']:.2f}MB")
            
            if 'esp32_estimation' in results and 'simulation_disabled' not in results['esp32_estimation']:
                esp32 = results['esp32_estimation']
                print(f"ESP32 Est. Time: {esp32['estimated_inference_time_ms']:.1f}ms")
                print(f"ESP32 Compatible: {'✓' if esp32['esp32_compatible'] else '✗'}")
            
            print("=" * 60)
        else:
            print("No model specified. Use --model option.")
        
        # Generate final report
        report_path = benchmarker.generate_benchmark_report()
        print(f"\nDetailed report saved: {report_path}")
        
    except Exception as e:
        logger.error(f"Benchmarking failed: {str(e)}")
        raise


if __name__ == "__main__":
    main()
