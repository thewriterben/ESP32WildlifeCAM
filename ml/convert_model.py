#!/usr/bin/env python3
"""
TensorFlow Lite Model Conversion Script for ESP32 Deployment

This script converts trained TensorFlow models to TensorFlow Lite format
optimized for ESP32 microcontrollers with quantization and optimization.
"""

import os
import json
import argparse
import logging
from pathlib import Path
import numpy as np
from typing import Dict, List, Optional, Tuple

import tensorflow as tf
from tensorflow import keras
import tensorflow_model_optimization as tfmot

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class ESP32ModelConverter:
    """
    Convert TensorFlow models to TensorFlow Lite for ESP32 deployment.
    
    Features:
    - Multiple quantization strategies
    - Model size optimization
    - Performance benchmarking  
    - ESP32-specific optimizations
    - C++ header file generation
    """
    
    def __init__(self):
        self.model = None
        self.tflite_model = None
        self.model_info = {}
    
    def load_model(self, model_path: str) -> keras.Model:
        """Load trained TensorFlow model."""
        logger.info(f"Loading model from {model_path}")
        
        if not os.path.exists(model_path):
            raise FileNotFoundError(f"Model file not found: {model_path}")
        
        # Load model
        self.model = keras.models.load_model(model_path)
        
        # Get model info
        self.model_info = {
            'input_shape': self.model.input_shape,
            'output_shape': self.model.output_shape,
            'total_params': self.model.count_params(),
            'model_size_mb': os.path.getsize(model_path) / (1024 * 1024)
        }
        
        logger.info(f"Model loaded: {self.model_info}")
        return self.model
    
    def create_representative_dataset(self, dataset_path: str = None, 
                                    num_samples: int = 100) -> callable:
        """
        Create representative dataset for quantization calibration.
        
        Args:
            dataset_path: Path to dataset for calibration
            num_samples: Number of samples to use for calibration
            
        Returns:
            Generator function for representative dataset
        """
        logger.info("Creating representative dataset for quantization...")
        
        if dataset_path and os.path.exists(dataset_path):
            # Load real data for calibration
            import glob
            from PIL import Image
            
            image_files = glob.glob(os.path.join(dataset_path, "**/*.jpg"), recursive=True)[:num_samples]
            
            def representative_data_gen():
                for image_path in image_files:
                    try:
                        # Load and preprocess image
                        image = Image.open(image_path).convert('RGB')
                        image = image.resize(self.model_info['input_shape'][1:3])
                        image = np.array(image, dtype=np.float32) / 255.0
                        image = np.expand_dims(image, axis=0)
                        yield [image]
                    except Exception as e:
                        logger.warning(f"Failed to process {image_path}: {e}")
                        continue
        else:
            # Generate synthetic data for calibration
            input_shape = self.model_info['input_shape']
            
            def representative_data_gen():
                for _ in range(num_samples):
                    # Generate random data matching input shape
                    data = np.random.random((1,) + input_shape[1:]).astype(np.float32)
                    yield [data]
        
        return representative_data_gen
    
    def convert_to_tflite(self, 
                         quantization: str = 'dynamic',
                         dataset_path: str = None,
                         optimization_target: str = 'size') -> bytes:
        """
        Convert model to TensorFlow Lite format.
        
        Args:
            quantization: Quantization strategy ('none', 'dynamic', 'int8', 'float16')
            dataset_path: Path to calibration dataset
            optimization_target: Optimization target ('size', 'latency', 'accuracy')
            
        Returns:
            TensorFlow Lite model bytes
        """
        if not self.model:
            raise ValueError("No model loaded. Call load_model() first.")
        
        logger.info(f"Converting model with {quantization} quantization...")
        
        # Initialize converter
        converter = tf.lite.TFLiteConverter.from_keras_model(self.model)
        
        # Set optimization strategy
        if optimization_target == 'size':
            converter.optimizations = [tf.lite.Optimize.DEFAULT]
        elif optimization_target == 'latency':
            converter.optimizations = [tf.lite.Optimize.OPTIMIZE_FOR_LATENCY]
        elif optimization_target == 'accuracy':
            converter.optimizations = []
        
        # Apply quantization
        if quantization == 'dynamic':
            # Dynamic range quantization (post-training)
            converter.optimizations = [tf.lite.Optimize.DEFAULT]
            
        elif quantization == 'int8':
            # Full integer quantization
            converter.optimizations = [tf.lite.Optimize.DEFAULT]
            converter.representative_dataset = self.create_representative_dataset(dataset_path)
            converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
            converter.inference_input_type = tf.int8
            converter.inference_output_type = tf.int8
            
        elif quantization == 'float16':
            # Float16 quantization
            converter.optimizations = [tf.lite.Optimize.DEFAULT]
            converter.target_spec.supported_types = [tf.float16]
            
        elif quantization == 'none':
            # No quantization
            pass
        
        # ESP32-specific optimizations
        converter.allow_custom_ops = True
        converter.experimental_new_converter = True
        
        # Convert model
        try:
            self.tflite_model = converter.convert()
            logger.info("Model conversion successful")
            
            # Update model info
            self.model_info['tflite_size_kb'] = len(self.tflite_model) / 1024
            self.model_info['compression_ratio'] = (
                self.model_info['model_size_mb'] * 1024 / self.model_info['tflite_size_kb']
            )
            
            logger.info(f"TFLite model size: {self.model_info['tflite_size_kb']:.2f} KB")
            logger.info(f"Compression ratio: {self.model_info['compression_ratio']:.2f}x")
            
        except Exception as e:
            logger.error(f"Model conversion failed: {e}")
            raise
        
        return self.tflite_model
    
    def optimize_for_esp32(self) -> bytes:
        """Apply ESP32-specific optimizations."""
        if not self.tflite_model:
            raise ValueError("No TFLite model available. Convert model first.")
        
        logger.info("Applying ESP32-specific optimizations...")
        
        # Load the model for analysis
        interpreter = tf.lite.Interpreter(model_content=self.tflite_model)
        interpreter.allocate_tensors()
        
        # Get model details
        input_details = interpreter.get_input_details()
        output_details = interpreter.get_output_details()
        
        # Check if model fits ESP32 constraints
        model_size_kb = len(self.tflite_model) / 1024
        
        # ESP32 memory constraints (approximate)
        max_model_size_kb = 1500  # Leave room for other operations
        max_tensor_arena_kb = 200
        
        if model_size_kb > max_model_size_kb:
            logger.warning(f"Model size ({model_size_kb:.1f} KB) exceeds ESP32 recommendation ({max_model_size_kb} KB)")
        
        # Estimate tensor arena size
        tensor_arena_size = self._estimate_tensor_arena_size(interpreter)
        tensor_arena_kb = tensor_arena_size / 1024
        
        if tensor_arena_kb > max_tensor_arena_kb:
            logger.warning(f"Estimated tensor arena ({tensor_arena_kb:.1f} KB) may exceed ESP32 capacity")
        
        self.model_info.update({
            'esp32_compatible': model_size_kb <= max_model_size_kb and tensor_arena_kb <= max_tensor_arena_kb,
            'estimated_tensor_arena_kb': tensor_arena_kb,
            'input_details': input_details,
            'output_details': output_details
        })
        
        return self.tflite_model
    
    def _estimate_tensor_arena_size(self, interpreter) -> int:
        """Estimate tensor arena size requirements."""
        # Get tensor details
        tensor_details = interpreter.get_tensor_details()
        
        total_size = 0
        for tensor in tensor_details:
            if tensor['shape'].size > 0:  # Skip scalar tensors
                tensor_size = np.prod(tensor['shape']) * self._get_type_size(tensor['dtype'])
                total_size += tensor_size
        
        # Add overhead (approximately 20-30%)
        total_size = int(total_size * 1.3)
        
        return total_size
    
    def _get_type_size(self, dtype) -> int:
        """Get size of data type in bytes."""
        type_sizes = {
            np.float32: 4,
            np.int8: 1,
            np.uint8: 1,
            np.int16: 2,
            np.int32: 4,
            np.float16: 2
        }
        return type_sizes.get(dtype, 4)  # Default to 4 bytes
    
    def benchmark_model(self, num_runs: int = 100) -> Dict:
        """Benchmark TFLite model performance."""
        if not self.tflite_model:
            raise ValueError("No TFLite model available. Convert model first.")
        
        logger.info(f"Benchmarking model performance ({num_runs} runs)...")
        
        # Initialize interpreter
        interpreter = tf.lite.Interpreter(model_content=self.tflite_model)
        interpreter.allocate_tensors()
        
        input_details = interpreter.get_input_details()
        output_details = interpreter.get_output_details()
        
        # Prepare random input
        input_shape = input_details[0]['shape']
        input_data = np.random.random(input_shape).astype(np.float32)
        
        # Warm up
        for _ in range(10):
            interpreter.set_tensor(input_details[0]['index'], input_data)
            interpreter.invoke()
        
        # Benchmark
        import time
        times = []
        
        for _ in range(num_runs):
            start_time = time.time()
            interpreter.set_tensor(input_details[0]['index'], input_data)
            interpreter.invoke()
            end_time = time.time()
            
            times.append((end_time - start_time) * 1000)  # Convert to milliseconds
        
        # Calculate statistics
        times = np.array(times)
        benchmark_results = {
            'avg_inference_time_ms': float(np.mean(times)),
            'min_inference_time_ms': float(np.min(times)),
            'max_inference_time_ms': float(np.max(times)),
            'std_inference_time_ms': float(np.std(times)),
            'throughput_fps': float(1000 / np.mean(times))
        }
        
        logger.info(f"Average inference time: {benchmark_results['avg_inference_time_ms']:.2f} ms")
        logger.info(f"Throughput: {benchmark_results['throughput_fps']:.2f} FPS")
        
        self.model_info['benchmark'] = benchmark_results
        
        return benchmark_results
    
    def generate_cpp_header(self, output_path: str, array_name: str = "wildlife_model") -> str:
        """Generate C++ header file for ESP32 deployment."""
        if not self.tflite_model:
            raise ValueError("No TFLite model available. Convert model first.")
        
        logger.info(f"Generating C++ header file: {output_path}")
        
        # Generate header content
        header_content = f"""/*
 * TensorFlow Lite model for ESP32 Wildlife Camera
 * Generated automatically - do not edit manually
 * 
 * Model size: {self.model_info['tflite_size_kb']:.2f} KB
 * Input shape: {self.model_info['input_shape']}
 * Output shape: {self.model_info['output_shape']}
 */

#ifndef WILDLIFE_MODEL_H
#define WILDLIFE_MODEL_H

#include <stdint.h>

// Model data
extern const unsigned char {array_name}_data[];
extern const int {array_name}_data_len;

// Model configuration 
#define MODEL_INPUT_WIDTH {self.model_info['input_shape'][2]}
#define MODEL_INPUT_HEIGHT {self.model_info['input_shape'][1]}
#define MODEL_INPUT_CHANNELS {self.model_info['input_shape'][3]}
#define MODEL_OUTPUT_SIZE {self.model_info['output_shape'][1]}
#define MODEL_TENSOR_ARENA_SIZE {int(self.model_info.get('estimated_tensor_arena_kb', 200) * 1024)}

// Model metadata
#define MODEL_VERSION "1.0.0"
#define MODEL_SIZE_BYTES {len(self.tflite_model)}

// Input/Output tensor indices (update based on actual model)
#define INPUT_TENSOR_INDEX 0
#define OUTPUT_TENSOR_INDEX 0

#endif // WILDLIFE_MODEL_H
"""
        
        # Write header file
        with open(output_path, 'w') as f:
            f.write(header_content)
        
        # Generate corresponding .cpp file with model data
        cpp_path = output_path.replace('.h', '.cpp')
        self._generate_cpp_data_file(cpp_path, array_name)
        
        return output_path
    
    def _generate_cpp_data_file(self, output_path: str, array_name: str):
        """Generate C++ data file with model bytes."""
        logger.info(f"Generating C++ data file: {output_path}")
        
        # Convert model bytes to C++ array
        model_bytes = list(self.tflite_model)
        
        cpp_content = f"""/*
 * TensorFlow Lite model data for ESP32 Wildlife Camera
 * Generated automatically - do not edit manually
 */

#include "{Path(output_path).stem.replace('_data', '')}.h"

// Model data array
const unsigned char {array_name}_data[] = {{
"""
        
        # Write bytes in rows of 12 for readability
        for i in range(0, len(model_bytes), 12):
            row = model_bytes[i:i+12]
            row_str = ', '.join(f'0x{b:02x}' for b in row)
            cpp_content += f"  {row_str},\n"
        
        cpp_content += f"""
}};

// Model data length
const int {array_name}_data_len = {len(model_bytes)};
"""
        
        # Write data file
        with open(output_path, 'w') as f:
            f.write(cpp_content)
    
    def save_model(self, output_path: str, generate_cpp: bool = True) -> str:
        """Save TFLite model and metadata."""
        if not self.tflite_model:
            raise ValueError("No TFLite model available. Convert model first.")
        
        output_path = Path(output_path)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        
        # Save TFLite model
        tflite_path = output_path.with_suffix('.tflite')
        with open(tflite_path, 'wb') as f:
            f.write(self.tflite_model)
        
        logger.info(f"TFLite model saved to: {tflite_path}")
        
        # Save model metadata
        metadata_path = output_path.with_suffix('.json')
        with open(metadata_path, 'w') as f:
            json.dump(self.model_info, f, indent=2, default=str)
        
        # Generate C++ files if requested
        if generate_cpp:
            cpp_header = output_path.with_suffix('.h')
            self.generate_cpp_header(str(cpp_header))
        
        return str(tflite_path)
    
    def validate_conversion(self, test_input: np.ndarray = None) -> Dict:
        """Validate TFLite model against original model."""
        if not self.model or not self.tflite_model:
            raise ValueError("Both original and TFLite models must be available")
        
        logger.info("Validating model conversion...")
        
        # Prepare test input
        if test_input is None:
            input_shape = self.model_info['input_shape']
            test_input = np.random.random((1,) + input_shape[1:]).astype(np.float32)
        
        # Original model prediction
        original_output = self.model.predict(test_input, verbose=0)
        
        # TFLite model prediction
        interpreter = tf.lite.Interpreter(model_content=self.tflite_model)
        interpreter.allocate_tensors()
        
        input_details = interpreter.get_input_details()
        output_details = interpreter.get_output_details()
        
        interpreter.set_tensor(input_details[0]['index'], test_input)
        interpreter.invoke()
        tflite_output = interpreter.get_tensor(output_details[0]['index'])
        
        # Compare outputs
        max_diff = np.max(np.abs(original_output - tflite_output))
        mean_diff = np.mean(np.abs(original_output - tflite_output))
        correlation = np.corrcoef(original_output.flatten(), tflite_output.flatten())[0, 1]
        
        validation_results = {
            'max_difference': float(max_diff),
            'mean_difference': float(mean_diff),
            'correlation': float(correlation),
            'validation_passed': max_diff < 0.1  # Threshold for acceptable difference
        }
        
        logger.info(f"Validation results: max_diff={max_diff:.6f}, correlation={correlation:.6f}")
        
        return validation_results


def main():
    parser = argparse.ArgumentParser(description='Convert TensorFlow model to TensorFlow Lite for ESP32')
    parser.add_argument('model_path', type=str, help='Path to trained TensorFlow model')
    parser.add_argument('--output', '-o', type=str, required=True, help='Output path for TFLite model')
    parser.add_argument('--quantization', '-q', type=str, default='dynamic',
                       choices=['none', 'dynamic', 'int8', 'float16'],
                       help='Quantization strategy')
    parser.add_argument('--dataset', '-d', type=str, help='Path to calibration dataset')
    parser.add_argument('--optimization', type=str, default='size',
                       choices=['size', 'latency', 'accuracy'],
                       help='Optimization target')
    parser.add_argument('--benchmark', action='store_true', help='Run performance benchmark')
    parser.add_argument('--cpp', action='store_true', default=True, help='Generate C++ header files')
    parser.add_argument('--validate', action='store_true', help='Validate conversion accuracy')
    
    args = parser.parse_args()
    
    try:
        # Initialize converter
        converter = ESP32ModelConverter()
        
        # Load model
        converter.load_model(args.model_path)
        
        # Convert to TFLite
        tflite_model = converter.convert_to_tflite(
            quantization=args.quantization,
            dataset_path=args.dataset,
            optimization_target=args.optimization
        )
        
        # Apply ESP32 optimizations
        converter.optimize_for_esp32()
        
        # Run benchmark if requested
        if args.benchmark:
            converter.benchmark_model()
        
        # Validate conversion if requested
        if args.validate:
            validation_results = converter.validate_conversion()
            if not validation_results['validation_passed']:
                logger.warning("Validation failed - model outputs differ significantly")
        
        # Save model
        output_path = converter.save_model(args.output, generate_cpp=args.cpp)
        
        logger.info("Model conversion completed successfully!")
        logger.info(f"Output saved to: {output_path}")
        
        # Print summary
        print("\n" + "="*50)
        print("CONVERSION SUMMARY")
        print("="*50)
        print(f"Original model size: {converter.model_info['model_size_mb']:.2f} MB")
        print(f"TFLite model size: {converter.model_info['tflite_size_kb']:.2f} KB")
        print(f"Compression ratio: {converter.model_info['compression_ratio']:.2f}x")
        print(f"ESP32 compatible: {converter.model_info['esp32_compatible']}")
        print(f"Estimated tensor arena: {converter.model_info['estimated_tensor_arena_kb']:.1f} KB")
        
        if 'benchmark' in converter.model_info:
            benchmark = converter.model_info['benchmark']
            print(f"Average inference time: {benchmark['avg_inference_time_ms']:.2f} ms")
            print(f"Throughput: {benchmark['throughput_fps']:.2f} FPS")
        
    except Exception as e:
        logger.error(f"Model conversion failed: {str(e)}")
        raise


if __name__ == "__main__":
    main()