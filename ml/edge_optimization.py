#!/usr/bin/env python3
"""
ESP32 Edge Optimization Module

This module provides ESP32-specific optimizations for wildlife classification models,
including memory optimization, quantization analysis, inference profiling, and
performance benchmarking tools.
"""

import os
import json
import logging
import time
import psutil
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Any
import numpy as np

# Deep Learning frameworks
import tensorflow as tf
from tensorflow import keras
import tensorflow_model_optimization as tfmot

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class ESP32ModelOptimizer:
    """
    ESP32-specific model optimization for wildlife classification.
    
    Features:
    - Memory arena sizing and optimization  
    - Quantization strategy analysis
    - Inference profiling and benchmarking
    - Model size reduction techniques
    - Performance optimization utilities
    """
    
    def __init__(self, config: Optional[Dict] = None):
        self.config = self._load_config(config)
        self.model = None
        self.tflite_model = None
        self.optimization_results = {}
        
        # ESP32 constraints
        self.esp32_constraints = {
            'psram_size_mb': 8,      # 8MB PSRAM
            'flash_size_mb': 16,     # Typical flash size
            'heap_size_kb': 320,     # Available heap
            'cpu_freq_mhz': 240,     # CPU frequency
            'inference_target_ms': 2000,  # 2 second target
            'power_budget_mw': 1000  # 1W power budget
        }
        
        logger.info("ESP32 Model Optimizer initialized")
    
    def _load_config(self, config: Optional[Dict]) -> Dict:
        """Load optimization configuration."""
        default_config = {
            'target_device': 'ESP32',
            'memory_limit_mb': 8,
            'quantization_strategy': 'int8',
            'optimization_target': 'size',  # 'size', 'latency', 'accuracy'
            'enable_xnnpack': True,
            'tensor_arena_size_kb': 2048,
            'micro_allocator': True,
            'optimize_for_size': True,
            'benchmarking': {
                'num_runs': 100,
                'warmup_runs': 10,
                'profile_memory': True
            }
        }
        
        if config:
            default_config.update(config)
        
        return default_config
    
    def load_model(self, model_path: str) -> keras.Model:
        """Load model for optimization."""
        logger.info(f"Loading model from {model_path}")
        
        if not os.path.exists(model_path):
            raise FileNotFoundError(f"Model file not found: {model_path}")
        
        self.model = keras.models.load_model(model_path)
        logger.info(f"Model loaded with {self.model.count_params()} parameters")
        
        return self.model
    
    def analyze_model_complexity(self) -> Dict[str, Any]:
        """Analyze model complexity for ESP32 deployment."""
        if not self.model:
            raise ValueError("No model loaded. Call load_model() first.")
        
        logger.info("Analyzing model complexity...")
        
        # Parameter analysis
        total_params = self.model.count_params()
        trainable_params = sum([tf.keras.backend.count_params(w) for w in self.model.trainable_weights])
        non_trainable_params = total_params - trainable_params
        
        # Memory estimation
        model_size_bytes = self._estimate_model_size_bytes()
        model_size_mb = model_size_bytes / (1024 * 1024)
        
        # Layer analysis
        layer_analysis = self._analyze_layers()
        
        # Compute complexity score
        complexity_score = self._calculate_complexity_score(total_params, model_size_mb)
        
        analysis = {
            'parameters': {
                'total': int(total_params),
                'trainable': int(trainable_params),
                'non_trainable': int(non_trainable_params)
            },
            'memory_usage': {
                'model_size_bytes': int(model_size_bytes),
                'model_size_mb': float(model_size_mb),
                'estimated_ram_mb': float(model_size_mb * 1.5)  # Include intermediate tensors
            },
            'layer_analysis': layer_analysis,
            'complexity_score': float(complexity_score),
            'esp32_compatibility': {
                'memory_fits': model_size_mb <= self.esp32_constraints['psram_size_mb'],
                'complexity_rating': self._rate_complexity(complexity_score),
                'optimization_needed': complexity_score > 0.7
            }
        }
        
        self.optimization_results['complexity_analysis'] = analysis
        return analysis
    
    def _estimate_model_size_bytes(self) -> int:
        """Estimate model size in bytes."""
        total_params = self.model.count_params()
        
        # Assume float32 parameters (4 bytes each)
        param_size = total_params * 4
        
        # Add overhead for model structure (estimated 10%)
        overhead = param_size * 0.1
        
        return int(param_size + overhead)
    
    def _analyze_layers(self) -> Dict[str, Any]:
        """Analyze model layers for optimization opportunities."""
        layer_types = {}
        layer_details = []
        total_ops = 0
        
        for i, layer in enumerate(self.model.layers):
            layer_type = type(layer).__name__
            layer_types[layer_type] = layer_types.get(layer_type, 0) + 1
            
            # Estimate operations for this layer
            layer_ops = self._estimate_layer_operations(layer)
            total_ops += layer_ops
            
            layer_info = {
                'index': i,
                'name': layer.name,
                'type': layer_type,
                'output_shape': str(layer.output_shape) if hasattr(layer, 'output_shape') else 'N/A',
                'parameters': layer.count_params(),
                'estimated_ops': layer_ops
            }
            
            # Add layer-specific information
            if hasattr(layer, 'filters'):
                layer_info['filters'] = layer.filters
            if hasattr(layer, 'kernel_size'):
                layer_info['kernel_size'] = layer.kernel_size
            if hasattr(layer, 'strides'):
                layer_info['strides'] = layer.strides
            
            layer_details.append(layer_info)
        
        return {
            'layer_types': layer_types,
            'layer_details': layer_details,
            'total_estimated_ops': total_ops,
            'optimization_opportunities': self._identify_optimization_opportunities(layer_details)
        }
    
    def _estimate_layer_operations(self, layer) -> int:
        """Estimate computational operations for a layer."""
        layer_type = type(layer).__name__
        
        if hasattr(layer, 'output_shape') and layer.output_shape:
            try:
                output_size = np.prod([dim for dim in layer.output_shape[1:] if dim is not None])
            except:
                output_size = 1000  # Default estimate
        else:
            output_size = 1000
        
        # Operation estimates based on layer type
        if 'Conv' in layer_type:
            if hasattr(layer, 'kernel_size') and hasattr(layer, 'filters'):
                kernel_ops = np.prod(layer.kernel_size) * layer.filters
                return int(output_size * kernel_ops)
        elif 'Dense' in layer_type:
            if hasattr(layer, 'units'):
                return int(output_size * layer.units)
        elif 'BatchNorm' in layer_type:
            return int(output_size * 4)  # Scale, shift, mean, variance
        
        return int(output_size)  # Default estimate
    
    def _identify_optimization_opportunities(self, layer_details: List[Dict]) -> List[str]:
        """Identify optimization opportunities in the model."""
        opportunities = []
        
        # Check for large dense layers
        for layer in layer_details:
            if 'Dense' in layer['type'] and layer['parameters'] > 100000:
                opportunities.append(f"Large dense layer '{layer['name']}' with {layer['parameters']} parameters - consider pruning")
        
        # Check for many convolutional layers
        conv_layers = [l for l in layer_details if 'Conv' in l['type']]
        if len(conv_layers) > 10:
            opportunities.append(f"Many conv layers ({len(conv_layers)}) - consider depthwise separable convolutions")
        
        # Check for lack of regularization
        dropout_layers = [l for l in layer_details if 'Dropout' in l['type']]
        if len(dropout_layers) < 2:
            opportunities.append("Few dropout layers - model might benefit from regularization")
        
        return opportunities
    
    def _calculate_complexity_score(self, total_params: int, model_size_mb: float) -> float:
        """Calculate complexity score (0-1, higher = more complex)."""
        # Normalize based on ESP32 constraints
        param_score = min(1.0, total_params / 1000000)  # 1M params = score 1.0
        size_score = min(1.0, model_size_mb / self.esp32_constraints['psram_size_mb'])
        
        # Weighted average
        complexity_score = 0.6 * param_score + 0.4 * size_score
        return complexity_score
    
    def _rate_complexity(self, score: float) -> str:
        """Rate complexity score."""
        if score < 0.3:
            return "Low"
        elif score < 0.6:
            return "Medium"
        elif score < 0.8:
            return "High"
        else:
            return "Very High"
    
    def optimize_model_architecture(self) -> keras.Model:
        """Optimize model architecture for ESP32 deployment."""
        if not self.model:
            raise ValueError("No model loaded. Call load_model() first.")
        
        logger.info("Optimizing model architecture...")
        
        # Clone the model
        optimized_model = keras.models.clone_model(self.model)
        optimized_model.set_weights(self.model.get_weights())
        
        # Apply optimizations based on configuration
        if self.config['optimize_for_size']:
            optimized_model = self._apply_size_optimizations(optimized_model)
        
        # Apply pruning if beneficial
        if self._should_apply_pruning():
            optimized_model = self._apply_pruning(optimized_model)
        
        logger.info("Model architecture optimization completed")
        return optimized_model
    
    def _apply_size_optimizations(self, model: keras.Model) -> keras.Model:
        """Apply size-focused optimizations."""
        # This is a placeholder for architecture modifications
        # In practice, you might replace layers with more efficient variants
        logger.info("Applying size optimizations...")
        
        # For now, return the model as-is
        # Real implementations might:
        # - Replace Conv2D with DepthwiseConv2D + Conv2D
        # - Reduce filter counts in conv layers
        # - Replace dense layers with global average pooling
        
        return model
    
    def _should_apply_pruning(self) -> bool:
        """Determine if pruning would be beneficial."""
        if 'complexity_analysis' not in self.optimization_results:
            self.analyze_model_complexity()
        
        complexity = self.optimization_results['complexity_analysis']
        return complexity['complexity_score'] > 0.6
    
    def _apply_pruning(self, model: keras.Model) -> keras.Model:
        """Apply magnitude-based pruning to the model."""
        logger.info("Applying magnitude-based pruning...")
        
        # Define pruning parameters
        pruning_params = {
            'pruning_schedule': tfmot.sparsity.keras.PolynomialDecay(
                initial_sparsity=0.0,
                final_sparsity=0.5,  # Remove 50% of weights
                begin_step=0,
                end_step=1000
            )
        }
        
        # Apply pruning to dense and conv layers
        def apply_pruning_to_layer(layer):
            if isinstance(layer, (keras.layers.Dense, keras.layers.Conv2D)):
                return tfmot.sparsity.keras.prune_low_magnitude(layer, **pruning_params)
            return layer
        
        # Clone and modify model
        pruned_model = keras.models.clone_model(
            model,
            clone_function=apply_pruning_to_layer
        )
        
        return pruned_model
    
    def estimate_tensor_arena_size(self, tflite_model_path: Optional[str] = None) -> Dict[str, Any]:
        """Estimate tensor arena size requirements for ESP32."""
        logger.info("Estimating tensor arena size...")
        
        if tflite_model_path and os.path.exists(tflite_model_path):
            # Load TFLite model
            interpreter = tf.lite.Interpreter(model_path=tflite_model_path)
        elif self.tflite_model:
            # Use in-memory TFLite model
            interpreter = tf.lite.Interpreter(model_content=self.tflite_model)
        else:
            # Convert current model to TFLite first
            if not self.model:
                raise ValueError("No model available for arena estimation")
            
            converter = tf.lite.TFLiteConverter.from_keras_model(self.model)
            tflite_model = converter.convert()
            interpreter = tf.lite.Interpreter(model_content=tflite_model)
        
        interpreter.allocate_tensors()
        
        # Get tensor information
        input_details = interpreter.get_input_details()
        output_details = interpreter.get_output_details()
        tensor_details = interpreter.get_tensor_details()
        
        # Calculate memory requirements
        total_tensor_size = 0
        input_size = 0
        output_size = 0
        intermediate_size = 0
        
        for tensor in tensor_details:
            tensor_size = np.prod(tensor['shape']) * self._get_type_size(tensor['dtype'])
            total_tensor_size += tensor_size
            
            if tensor['index'] in [d['index'] for d in input_details]:
                input_size += tensor_size
            elif tensor['index'] in [d['index'] for d in output_details]:
                output_size += tensor_size
            else:
                intermediate_size += tensor_size
        
        # Estimate arena size (add overhead)
        estimated_arena_size = int(total_tensor_size * 1.2)  # 20% overhead
        arena_size_kb = estimated_arena_size / 1024
        
        arena_analysis = {
            'estimated_arena_size_bytes': estimated_arena_size,
            'estimated_arena_size_kb': arena_size_kb,
            'tensor_breakdown': {
                'input_size_bytes': int(input_size),
                'output_size_bytes': int(output_size),
                'intermediate_size_bytes': int(intermediate_size),
                'total_tensor_size_bytes': int(total_tensor_size)
            },
            'esp32_compatibility': {
                'fits_in_psram': arena_size_kb <= (self.esp32_constraints['psram_size_mb'] * 1024 * 0.8),  # Use 80% of PSRAM
                'recommended_arena_kb': max(int(arena_size_kb * 1.1), 1024),  # At least 1MB
                'memory_efficiency': min(1.0, (self.esp32_constraints['psram_size_mb'] * 1024) / arena_size_kb)
            },
            'tensor_count': len(tensor_details),
            'input_tensors': len(input_details),
            'output_tensors': len(output_details)
        }
        
        self.optimization_results['arena_analysis'] = arena_analysis
        logger.info(f"Estimated tensor arena size: {arena_size_kb:.1f} KB")
        
        return arena_analysis
    
    def _get_type_size(self, dtype) -> int:
        """Get size in bytes for tensor dtype."""
        if dtype == np.float32:
            return 4
        elif dtype == np.float16:
            return 2
        elif dtype == np.int8:
            return 1
        elif dtype == np.int16:
            return 2
        elif dtype == np.int32:
            return 4
        elif dtype == np.uint8:
            return 1
        else:
            return 4  # Default to 4 bytes
    
    def profile_inference_performance(self, test_data: Optional[np.ndarray] = None,
                                    num_runs: int = 100) -> Dict[str, Any]:
        """Profile inference performance on current hardware (simulating ESP32)."""
        if not self.model:
            raise ValueError("No model loaded. Call load_model() first.")
        
        logger.info(f"Profiling inference performance ({num_runs} runs)...")
        
        # Generate test data if not provided
        if test_data is None:
            input_shape = self.model.input_shape[1:]  # Remove batch dimension
            test_data = np.random.random((1,) + input_shape).astype(np.float32)
        
        # Warm-up runs
        warmup_runs = min(10, num_runs // 10)
        for _ in range(warmup_runs):
            _ = self.model.predict(test_data, verbose=0)
        
        # Timed runs
        inference_times = []
        memory_usage = []
        
        process = psutil.Process()
        
        for i in range(num_runs):
            # Memory before inference
            memory_before = process.memory_info().rss
            
            # Time inference
            start_time = time.perf_counter()
            predictions = self.model.predict(test_data, verbose=0)
            end_time = time.perf_counter()
            
            # Memory after inference
            memory_after = process.memory_info().rss
            
            inference_time_ms = (end_time - start_time) * 1000
            memory_delta_mb = (memory_after - memory_before) / (1024 * 1024)
            
            inference_times.append(inference_time_ms)
            memory_usage.append(memory_delta_mb)
        
        # Calculate statistics
        mean_time = np.mean(inference_times)
        std_time = np.std(inference_times)
        min_time = np.min(inference_times)
        max_time = np.max(inference_times)
        median_time = np.median(inference_times)
        
        mean_memory = np.mean(memory_usage)
        
        # Estimate ESP32 performance (scaling factor based on CPU differences)
        esp32_scaling_factor = 2.5  # Rough estimate: ESP32 is ~2.5x slower
        estimated_esp32_time = mean_time * esp32_scaling_factor
        
        performance_profile = {
            'host_performance': {
                'mean_inference_time_ms': float(mean_time),
                'std_inference_time_ms': float(std_time),
                'min_inference_time_ms': float(min_time),
                'max_inference_time_ms': float(max_time),
                'median_inference_time_ms': float(median_time),
                'mean_memory_usage_mb': float(mean_memory)
            },
            'estimated_esp32_performance': {
                'estimated_inference_time_ms': float(estimated_esp32_time),
                'meets_target': estimated_esp32_time <= self.esp32_constraints['inference_target_ms'],
                'performance_ratio': float(self.esp32_constraints['inference_target_ms'] / estimated_esp32_time),
                'optimization_needed': estimated_esp32_time > self.esp32_constraints['inference_target_ms'] * 0.8
            },
            'test_parameters': {
                'num_runs': num_runs,
                'warmup_runs': warmup_runs,
                'input_shape': list(test_data.shape),
                'scaling_factor_used': esp32_scaling_factor
            }
        }
        
        self.optimization_results['performance_profile'] = performance_profile
        
        logger.info(f"Mean inference time: {mean_time:.2f}ms (estimated ESP32: {estimated_esp32_time:.2f}ms)")
        return performance_profile
    
    def benchmark_quantization_strategies(self, test_data: Optional[np.ndarray] = None) -> Dict[str, Any]:
        """Benchmark different quantization strategies."""
        if not self.model:
            raise ValueError("No model loaded. Call load_model() first.")
        
        logger.info("Benchmarking quantization strategies...")
        
        # Generate test data if not provided
        if test_data is None:
            input_shape = self.model.input_shape[1:]
            test_data = np.random.random((10,) + input_shape).astype(np.float32)
        
        quantization_results = {}
        
        # Test different quantization strategies
        strategies = ['none', 'dynamic', 'int8', 'float16']
        
        for strategy in strategies:
            logger.info(f"Testing {strategy} quantization...")
            
            try:
                # Convert model
                converter = tf.lite.TFLiteConverter.from_keras_model(self.model)
                
                if strategy == 'dynamic':
                    converter.optimizations = [tf.lite.Optimize.DEFAULT]
                elif strategy == 'int8':
                    converter.optimizations = [tf.lite.Optimize.DEFAULT]
                    converter.target_spec.supported_types = [tf.int8]
                    # Would need representative dataset for full int8 quantization
                elif strategy == 'float16':
                    converter.optimizations = [tf.lite.Optimize.DEFAULT]
                    converter.target_spec.supported_types = [tf.float16]
                
                tflite_model = converter.convert()
                
                # Test inference
                interpreter = tf.lite.Interpreter(model_content=tflite_model)
                interpreter.allocate_tensors()
                
                input_details = interpreter.get_input_details()
                output_details = interpreter.get_output_details()
                
                # Profile inference
                inference_times = []
                for i in range(10):  # Small number for benchmarking
                    interpreter.set_tensor(input_details[0]['index'], test_data[:1])
                    
                    start_time = time.perf_counter()
                    interpreter.invoke()
                    end_time = time.perf_counter()
                    
                    inference_times.append((end_time - start_time) * 1000)
                
                # Calculate metrics
                model_size_bytes = len(tflite_model)
                model_size_kb = model_size_bytes / 1024
                mean_inference_time = np.mean(inference_times)
                
                quantization_results[strategy] = {
                    'model_size_bytes': model_size_bytes,
                    'model_size_kb': model_size_kb,
                    'mean_inference_time_ms': float(mean_inference_time),
                    'compression_ratio': float(self._estimate_model_size_bytes() / model_size_bytes),
                    'fits_esp32': model_size_kb <= (self.esp32_constraints['psram_size_mb'] * 1024 * 0.6)
                }
                
            except Exception as e:
                logger.warning(f"Failed to test {strategy} quantization: {e}")
                quantization_results[strategy] = {'error': str(e)}
        
        # Find best strategy
        valid_strategies = {k: v for k, v in quantization_results.items() if 'error' not in v}
        
        if valid_strategies:
            # Score based on size and performance
            best_strategy = min(valid_strategies.keys(), 
                              key=lambda x: valid_strategies[x]['model_size_kb'] + 
                                          valid_strategies[x]['mean_inference_time_ms'] * 0.1)
            
            quantization_results['recommended_strategy'] = best_strategy
            quantization_results['recommendation_reason'] = f"Best balance of size and performance"
        
        self.optimization_results['quantization_benchmark'] = quantization_results
        return quantization_results
    
    def generate_optimization_report(self, output_path: Optional[str] = None) -> Dict[str, Any]:
        """Generate comprehensive optimization report."""
        logger.info("Generating optimization report...")
        
        # Ensure all analyses are complete
        if 'complexity_analysis' not in self.optimization_results:
            self.analyze_model_complexity()
        
        # Compile report
        report = {
            'model_info': {
                'parameters': self.model.count_params() if self.model else 0,
                'input_shape': list(self.model.input_shape[1:]) if self.model else [],
                'output_shape': list(self.model.output_shape[1:]) if self.model else []
            },
            'esp32_constraints': self.esp32_constraints,
            'optimization_results': self.optimization_results,
            'recommendations': self._generate_recommendations()
        }
        
        # Save report if path provided
        if output_path:
            with open(output_path, 'w') as f:
                json.dump(report, f, indent=2)
            logger.info(f"Optimization report saved to {output_path}")
        
        return report
    
    def _generate_recommendations(self) -> List[str]:
        """Generate optimization recommendations based on analysis."""
        recommendations = []
        
        if 'complexity_analysis' in self.optimization_results:
            complexity = self.optimization_results['complexity_analysis']
            
            if not complexity['esp32_compatibility']['memory_fits']:
                recommendations.append("Model too large for ESP32 PSRAM - apply aggressive quantization or pruning")
            
            if complexity['complexity_score'] > 0.8:
                recommendations.append("High model complexity - consider architecture simplification")
            
            if complexity['esp32_compatibility']['optimization_needed']:
                recommendations.append("Model optimization recommended for ESP32 deployment")
        
        if 'performance_profile' in self.optimization_results:
            perf = self.optimization_results['performance_profile']
            
            if not perf['estimated_esp32_performance']['meets_target']:
                recommendations.append("Inference time exceeds target - apply performance optimizations")
        
        if 'quantization_benchmark' in self.optimization_results:
            quant = self.optimization_results['quantization_benchmark']
            
            if 'recommended_strategy' in quant:
                strategy = quant['recommended_strategy']
                recommendations.append(f"Use {strategy} quantization for optimal ESP32 performance")
        
        if not recommendations:
            recommendations.append("Model appears suitable for ESP32 deployment")
        
        return recommendations


def main():
    """Example usage of ESP32 optimization utilities."""
    import argparse
    
    parser = argparse.ArgumentParser(description='Optimize model for ESP32 deployment')
    parser.add_argument('--model', type=str, required=True, help='Path to trained model')
    parser.add_argument('--output', type=str, default='./optimization_report.json', help='Output report path')
    parser.add_argument('--profile', action='store_true', help='Profile inference performance')
    parser.add_argument('--benchmark', action='store_true', help='Benchmark quantization strategies')
    
    args = parser.parse_args()
    
    # Initialize optimizer
    optimizer = ESP32ModelOptimizer()
    
    try:
        # Load model
        model = optimizer.load_model(args.model)
        
        # Run analyses
        complexity = optimizer.analyze_model_complexity()
        arena_analysis = optimizer.estimate_tensor_arena_size()
        
        if args.profile:
            performance = optimizer.profile_inference_performance()
        
        if args.benchmark:
            quantization = optimizer.benchmark_quantization_strategies()
        
        # Generate report
        report = optimizer.generate_optimization_report(args.output)
        
        # Print summary
        print("\n" + "=" * 60)
        print("ESP32 OPTIMIZATION SUMMARY")
        print("=" * 60)
        print(f"Model Parameters: {complexity['parameters']['total']:,}")
        print(f"Model Size: {complexity['memory_usage']['model_size_mb']:.2f} MB")
        print(f"ESP32 Compatible: {'✓' if complexity['esp32_compatibility']['memory_fits'] else '✗'}")
        print(f"Complexity Rating: {complexity['esp32_compatibility']['complexity_rating']}")
        print(f"Tensor Arena: {arena_analysis['estimated_arena_size_kb']:.1f} KB")
        
        if args.profile:
            perf = optimizer.optimization_results['performance_profile']
            est_time = perf['estimated_esp32_performance']['estimated_inference_time_ms']
            print(f"Est. ESP32 Inference: {est_time:.0f} ms")
            print(f"Meets Target: {'✓' if perf['estimated_esp32_performance']['meets_target'] else '✗'}")
        
        print("\nRecommendations:")
        for rec in report['recommendations']:
            print(f"  • {rec}")
        print("=" * 60)
        
        logger.info("ESP32 optimization analysis completed successfully!")
        
    except Exception as e:
        logger.error(f"Optimization failed: {str(e)}")
        raise


if __name__ == "__main__":
    main()