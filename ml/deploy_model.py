#!/usr/bin/env python3
"""
Model Deployment Utilities for ESP32 Wildlife Camera

This script provides utilities for deploying trained wildlife classification
models to ESP32 devices, including firmware integration, over-the-air updates,
and deployment validation.
"""

import os
import json
import logging
import subprocess
import tempfile
import shutil
from pathlib import Path
from typing import Dict, List, Optional, Any, Tuple
import hashlib
import zipfile
import requests

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class ESP32ModelDeployer:
    """
    ESP32 model deployment and management utilities.
    
    Features:
    - TensorFlow Lite model packaging
    - C++ header generation for firmware
    - Over-the-air (OTA) deployment
    - Deployment validation and testing
    - Model versioning and rollback
    - Firmware integration utilities
    """
    
    def __init__(self, config: Optional[Dict] = None):
        self.config = self._load_config(config)
        self.deployment_dir = Path(self.config['deployment_dir'])
        self.firmware_dir = Path(self.config['firmware_dir'])
        
        # Create directories
        self.deployment_dir.mkdir(parents=True, exist_ok=True)
        
        logger.info(f"Model deployer initialized. Deployment dir: {self.deployment_dir}")
    
    def _load_config(self, config: Optional[Dict]) -> Dict:
        """Load deployment configuration."""
        default_config = {
            'deployment_dir': './deployments',
            'firmware_dir': '../firmware',
            'model_header_template': 'model_data.h.template',
            'max_model_size_kb': 2048,  # 2MB max model size
            'deployment_validation': True,
            'ota_enabled': True,
            'version_format': 'v{major}.{minor}.{patch}',
            'backup_count': 3,
            'esp32_settings': {
                'board': 'esp32cam',
                'flash_size': '4MB',
                'partition_scheme': 'huge_app',
                'upload_speed': 921600,
                'monitor_speed': 115200
            }
        }
        
        if config:
            default_config.update(config)
        
        return default_config
    
    def prepare_model_for_deployment(self, tflite_model_path: str, 
                                   model_name: str = None,
                                   version: str = None) -> Dict[str, Any]:
        """
        Prepare TensorFlow Lite model for ESP32 deployment.
        
        Args:
            tflite_model_path: Path to TensorFlow Lite model
            model_name: Name for the deployed model
            version: Model version string
            
        Returns:
            Deployment package information
        """
        tflite_path = Path(tflite_model_path)
        
        if not tflite_path.exists():
            raise FileNotFoundError(f"TensorFlow Lite model not found: {tflite_path}")
        
        if not model_name:
            model_name = tflite_path.stem
        
        if not version:
            version = self._generate_version()
        
        logger.info(f"Preparing model '{model_name}' version {version} for deployment...")
        
        # Create deployment package directory
        package_dir = self.deployment_dir / f"{model_name}_{version}"
        package_dir.mkdir(parents=True, exist_ok=True)
        
        # Validate model size
        model_size_kb = tflite_path.stat().st_size / 1024
        if model_size_kb > self.config['max_model_size_kb']:
            raise ValueError(f"Model too large: {model_size_kb:.1f}KB > {self.config['max_model_size_kb']}KB")
        
        # Copy model to package
        model_file = package_dir / f"{model_name}.tflite"
        shutil.copy2(str(tflite_path), str(model_file))
        
        # Generate C++ header file
        header_file = self._generate_cpp_header(model_file, model_name)
        
        # Generate model metadata
        metadata = self._generate_model_metadata(model_file, model_name, version)
        
        # Create deployment manifest
        manifest = self._create_deployment_manifest(
            model_name, version, model_file, header_file, metadata
        )
        
        # Package everything
        package_info = self._create_deployment_package(package_dir, manifest)
        
        logger.info(f"Model deployment package created: {package_info['package_path']}")
        return package_info
    
    def _generate_version(self) -> str:
        """Generate version string."""
        import datetime
        
        # Use timestamp-based versioning
        timestamp = datetime.datetime.now()
        major = timestamp.year % 100  # 2-digit year
        minor = timestamp.month
        patch = timestamp.day * 100 + timestamp.hour
        
        return f"v{major}.{minor}.{patch}"
    
    def _generate_cpp_header(self, model_file: Path, model_name: str) -> Path:
        """Generate C++ header file for firmware integration."""
        logger.info(f"Generating C++ header for {model_name}...")
        
        # Read model data
        with open(model_file, 'rb') as f:
            model_data = f.read()
        
        # Generate header content
        header_content = self._create_cpp_header_content(model_data, model_name)
        
        # Write header file
        header_file = model_file.parent / f"{model_name}_model_data.h"
        with open(header_file, 'w') as f:
            f.write(header_content)
        
        logger.info(f"C++ header generated: {header_file}")
        return header_file
    
    def _create_cpp_header_content(self, model_data: bytes, model_name: str) -> str:
        """Create C++ header file content."""
        # Convert model data to C++ array
        data_array = ', '.join(f'0x{byte:02x}' for byte in model_data)
        
        # Create header guard
        guard_name = f"{model_name.upper()}_MODEL_DATA_H"
        
        header_content = f"""// Auto-generated model data for {model_name}
// Generated by ESP32 Wildlife Camera ML Pipeline
// DO NOT EDIT - This file is auto-generated

#ifndef {guard_name}
#define {guard_name}

#include <stdint.h>

// Model information
#define {model_name.upper()}_MODEL_SIZE {len(model_data)}
#define {model_name.upper()}_MODEL_VERSION_MAJOR 1
#define {model_name.upper()}_MODEL_VERSION_MINOR 0
#define {model_name.upper()}_MODEL_VERSION_PATCH 0

// Model data array
extern const unsigned char {model_name}_model_data[{len(model_data)}];
extern const int {model_name}_model_data_len;

// Model data definition
const unsigned char {model_name}_model_data[{len(model_data)}] = {{
    {data_array}
}};

const int {model_name}_model_data_len = {len(model_data)};

// Model configuration
struct {model_name.capitalize()}ModelConfig {{
    const unsigned char* model_data;
    int model_size;
    int input_width;
    int input_height;
    int input_channels;
    int num_classes;
    float inference_threshold;
}};

// Default model configuration
const {model_name.capitalize()}ModelConfig {model_name}_model_config = {{
    .model_data = {model_name}_model_data,
    .model_size = {model_name.upper()}_MODEL_SIZE,
    .input_width = 224,      // Default input width
    .input_height = 224,     // Default input height  
    .input_channels = 3,     // RGB channels
    .num_classes = 16,       // Number of wildlife species
    .inference_threshold = 0.7f  // Confidence threshold
}};

#endif // {guard_name}
"""
        
        return header_content
    
    def _generate_model_metadata(self, model_file: Path, model_name: str, version: str) -> Dict:
        """Generate model metadata."""
        import datetime
        import tensorflow as tf
        
        # Basic metadata
        metadata = {
            'model_name': model_name,
            'version': version,
            'file_size_bytes': model_file.stat().st_size,
            'file_size_kb': model_file.stat().st_size / 1024,
            'creation_date': datetime.datetime.now().isoformat(),
            'checksum': self._calculate_file_hash(model_file)
        }
        
        # Analyze TFLite model
        try:
            interpreter = tf.lite.Interpreter(model_path=str(model_file))
            interpreter.allocate_tensors()
            
            input_details = interpreter.get_input_details()
            output_details = interpreter.get_output_details()
            
            if input_details:
                input_shape = input_details[0]['shape']
                metadata['input_shape'] = input_shape.tolist()
                metadata['input_dtype'] = str(input_details[0]['dtype'])
            
            if output_details:
                output_shape = output_details[0]['shape']
                metadata['output_shape'] = output_shape.tolist()
                metadata['output_dtype'] = str(output_details[0]['dtype'])
            
            # Get tensor details
            tensor_details = interpreter.get_tensor_details()
            metadata['tensor_count'] = len(tensor_details)
            
            # Estimate memory requirements
            total_tensor_size = sum(
                np.prod([dim for dim in tensor['shape'] if dim > 0]) * 
                self._get_dtype_size(tensor['dtype'])
                for tensor in tensor_details
            )
            metadata['estimated_memory_kb'] = total_tensor_size / 1024
            
        except Exception as e:
            logger.warning(f"Could not analyze TFLite model: {e}")
            metadata['analysis_error'] = str(e)
        
        return metadata
    
    def _get_dtype_size(self, dtype) -> int:
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
            return 4  # Default
    
    def _calculate_file_hash(self, file_path: Path) -> str:
        """Calculate SHA256 hash of file."""
        sha256_hash = hashlib.sha256()
        with open(file_path, 'rb') as f:
            for chunk in iter(lambda: f.read(4096), b""):
                sha256_hash.update(chunk)
        return sha256_hash.hexdigest()
    
    def _create_deployment_manifest(self, model_name: str, version: str,
                                  model_file: Path, header_file: Path,
                                  metadata: Dict) -> Dict:
        """Create deployment manifest."""
        import datetime
        
        manifest = {
            'deployment_info': {
                'model_name': model_name,
                'version': version,
                'created_at': datetime.datetime.now().isoformat(),
                'deployment_type': 'esp32_tflite'
            },
            'files': {
                'model_file': {
                    'path': model_file.name,
                    'size_bytes': model_file.stat().st_size,
                    'checksum': self._calculate_file_hash(model_file),
                    'type': 'tflite_model'
                },
                'header_file': {
                    'path': header_file.name,
                    'size_bytes': header_file.stat().st_size,
                    'checksum': self._calculate_file_hash(header_file),
                    'type': 'cpp_header'
                }
            },
            'model_metadata': metadata,
            'esp32_config': {
                'max_memory_kb': self.config['max_model_size_kb'],
                'target_board': self.config['esp32_settings']['board'],
                'flash_size': self.config['esp32_settings']['flash_size']
            },
            'deployment_instructions': {
                'firmware_integration': [
                    f"1. Copy {header_file.name} to firmware/src/models/",
                    f"2. Include header in main.cpp: #include \"models/{header_file.name}\"",
                    "3. Initialize model in setup(): load_wildlife_model()",
                    "4. Use model in inference: run_inference(image_data)"
                ],
                'build_flags': [
                    "-DUSE_TFLITE_MODEL",
                    f"-DMODEL_SIZE={metadata['file_size_bytes']}",
                    "-DTENSOR_ARENA_SIZE=2048000"  # 2MB tensor arena
                ]
            }
        }
        
        # Save manifest
        manifest_file = model_file.parent / 'deployment_manifest.json'
        with open(manifest_file, 'w') as f:
            json.dump(manifest, f, indent=2)
        
        return manifest
    
    def _create_deployment_package(self, package_dir: Path, manifest: Dict) -> Dict:
        """Create deployment package archive."""
        package_name = f"{manifest['deployment_info']['model_name']}_{manifest['deployment_info']['version']}.zip"
        package_path = self.deployment_dir / package_name
        
        # Create ZIP archive
        with zipfile.ZipFile(package_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            # Add all files in package directory
            for file_path in package_dir.rglob('*'):
                if file_path.is_file():
                    arcname = file_path.relative_to(package_dir)
                    zipf.write(str(file_path), str(arcname))
        
        package_info = {
            'package_name': package_name,
            'package_path': str(package_path),
            'package_size_bytes': package_path.stat().st_size,
            'package_size_kb': package_path.stat().st_size / 1024,
            'manifest': manifest
        }
        
        return package_info
    
    def deploy_to_firmware(self, package_path: str, 
                          firmware_dir: Optional[str] = None) -> bool:
        """
        Deploy model package to firmware directory.
        
        Args:
            package_path: Path to deployment package
            firmware_dir: Optional firmware directory override
            
        Returns:
            True if successful, False otherwise
        """
        package_path = Path(package_path)
        
        if not package_path.exists():
            logger.error(f"Package not found: {package_path}")
            return False
        
        if firmware_dir:
            target_firmware_dir = Path(firmware_dir)
        else:
            target_firmware_dir = self.firmware_dir
        
        if not target_firmware_dir.exists():
            logger.error(f"Firmware directory not found: {target_firmware_dir}")
            return False
        
        logger.info(f"Deploying package to firmware: {package_path}")
        
        try:
            # Extract package to temporary directory
            with tempfile.TemporaryDirectory() as temp_dir:
                temp_path = Path(temp_dir)
                
                with zipfile.ZipFile(package_path, 'r') as zipf:
                    zipf.extractall(temp_path)
                
                # Load manifest
                manifest_file = temp_path / 'deployment_manifest.json'
                if not manifest_file.exists():
                    logger.error("Deployment manifest not found in package")
                    return False
                
                with open(manifest_file, 'r') as f:
                    manifest = json.load(f)
                
                # Copy files to firmware
                models_dir = target_firmware_dir / 'src' / 'models'
                models_dir.mkdir(parents=True, exist_ok=True)
                
                # Copy header file
                header_file = manifest['files']['header_file']['path']
                src_header = temp_path / header_file
                dst_header = models_dir / header_file
                
                if src_header.exists():
                    shutil.copy2(str(src_header), str(dst_header))
                    logger.info(f"Copied header file: {dst_header}")
                else:
                    logger.error(f"Header file not found in package: {header_file}")
                    return False
                
                # Copy model file (optional, for reference)
                model_file = manifest['files']['model_file']['path']
                src_model = temp_path / model_file
                dst_model = models_dir / model_file
                
                if src_model.exists():
                    shutil.copy2(str(src_model), str(dst_model))
                    logger.info(f"Copied model file: {dst_model}")
                
                # Update firmware configuration if needed
                self._update_firmware_config(target_firmware_dir, manifest)
                
                logger.info("Model deployment to firmware completed successfully")
                return True
                
        except Exception as e:
            logger.error(f"Failed to deploy to firmware: {e}")
            return False
    
    def _update_firmware_config(self, firmware_dir: Path, manifest: Dict):
        """Update firmware configuration files."""
        # Update platformio.ini if it exists
        platformio_ini = firmware_dir / 'platformio.ini'
        
        if platformio_ini.exists():
            logger.info("Updating platformio.ini with model configuration...")
            
            build_flags = manifest['deployment_instructions']['build_flags']
            
            # Read current config
            with open(platformio_ini, 'r') as f:
                config_content = f.read()
            
            # Add build flags if not already present
            for flag in build_flags:
                if flag not in config_content:
                    # Find build_flags section and add flag
                    if 'build_flags =' in config_content:
                        config_content = config_content.replace(
                            'build_flags =',
                            f'build_flags =\n    {flag}'
                        )
                    else:
                        # Add build_flags section
                        config_content += f"\nbuild_flags =\n    {flag}\n"
            
            # Write updated config
            with open(platformio_ini, 'w') as f:
                f.write(config_content)
    
    def validate_deployment(self, firmware_dir: Optional[str] = None) -> Dict[str, Any]:
        """
        Validate model deployment in firmware.
        
        Args:
            firmware_dir: Optional firmware directory override
            
        Returns:
            Validation results
        """
        if firmware_dir:
            target_firmware_dir = Path(firmware_dir)
        else:
            target_firmware_dir = self.firmware_dir
        
        logger.info(f"Validating deployment in: {target_firmware_dir}")
        
        validation_results = {
            'firmware_dir_exists': target_firmware_dir.exists(),
            'models_dir_exists': False,
            'header_files_found': [],
            'model_files_found': [],
            'build_config_valid': False,
            'compilation_test': False,
            'overall_valid': False
        }
        
        try:
            # Check models directory
            models_dir = target_firmware_dir / 'src' / 'models'
            validation_results['models_dir_exists'] = models_dir.exists()
            
            if models_dir.exists():
                # Find header files
                header_files = list(models_dir.glob('*_model_data.h'))
                validation_results['header_files_found'] = [f.name for f in header_files]
                
                # Find model files
                model_files = list(models_dir.glob('*.tflite'))
                validation_results['model_files_found'] = [f.name for f in model_files]
            
            # Check build configuration
            platformio_ini = target_firmware_dir / 'platformio.ini'
            if platformio_ini.exists():
                with open(platformio_ini, 'r') as f:
                    config_content = f.read()
                
                required_flags = ['-DUSE_TFLITE_MODEL', '-DTENSOR_ARENA_SIZE']
                validation_results['build_config_valid'] = all(
                    flag in config_content for flag in required_flags
                )
            
            # Test compilation (if PlatformIO is available)
            if self._check_platformio_available():
                validation_results['compilation_test'] = self._test_compilation(target_firmware_dir)
            
            # Overall validation
            validation_results['overall_valid'] = (
                validation_results['firmware_dir_exists'] and
                validation_results['models_dir_exists'] and
                len(validation_results['header_files_found']) > 0 and
                validation_results['build_config_valid']
            )
            
        except Exception as e:
            logger.error(f"Validation failed: {e}")
            validation_results['error'] = str(e)
        
        return validation_results
    
    def _check_platformio_available(self) -> bool:
        """Check if PlatformIO is available."""
        try:
            result = subprocess.run(['pio', '--version'], 
                                  capture_output=True, text=True, timeout=10)
            return result.returncode == 0
        except:
            return False
    
    def _test_compilation(self, firmware_dir: Path) -> bool:
        """Test firmware compilation."""
        try:
            logger.info("Testing firmware compilation...")
            
            # Run PlatformIO build check
            result = subprocess.run(
                ['pio', 'run', '--target', 'checkprogsize'],
                cwd=str(firmware_dir),
                capture_output=True,
                text=True,
                timeout=300  # 5 minute timeout
            )
            
            if result.returncode == 0:
                logger.info("Compilation test passed")
                return True
            else:
                logger.warning(f"Compilation test failed: {result.stderr}")
                return False
                
        except subprocess.TimeoutExpired:
            logger.warning("Compilation test timed out")
            return False
        except Exception as e:
            logger.warning(f"Compilation test error: {e}")
            return False
    
    def create_ota_package(self, package_path: str, 
                          server_url: Optional[str] = None) -> Dict[str, Any]:
        """
        Create over-the-air (OTA) update package.
        
        Args:
            package_path: Path to deployment package
            server_url: Optional OTA server URL
            
        Returns:
            OTA package information
        """
        package_path = Path(package_path)
        
        if not package_path.exists():
            raise FileNotFoundError(f"Package not found: {package_path}")
        
        logger.info(f"Creating OTA package from: {package_path}")
        
        # Load package manifest
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_path = Path(temp_dir)
            
            with zipfile.ZipFile(package_path, 'r') as zipf:
                zipf.extractall(temp_path)
            
            manifest_file = temp_path / 'deployment_manifest.json'
            with open(manifest_file, 'r') as f:
                manifest = json.load(f)
        
        # Create OTA manifest
        ota_manifest = {
            'ota_info': {
                'package_name': package_path.name,
                'version': manifest['deployment_info']['version'],
                'model_name': manifest['deployment_info']['model_name'],
                'package_size': package_path.stat().st_size,
                'checksum': self._calculate_file_hash(package_path),
                'created_at': manifest['deployment_info']['created_at']
            },
            'deployment_requirements': {
                'min_flash_size': manifest['esp32_config']['flash_size'],
                'min_memory_kb': manifest['esp32_config']['max_memory_kb'],
                'compatible_boards': [manifest['esp32_config']['target_board']]
            },
            'update_instructions': {
                'download_url': f"{server_url}/{package_path.name}" if server_url else None,
                'install_steps': [
                    "1. Download deployment package",
                    "2. Verify package checksum",
                    "3. Extract model files",
                    "4. Update firmware with new model",
                    "5. Restart ESP32 device"
                ]
            }
        }
        
        # Save OTA manifest
        ota_manifest_file = package_path.parent / f"{package_path.stem}_ota_manifest.json"
        with open(ota_manifest_file, 'w') as f:
            json.dump(ota_manifest, f, indent=2)
        
        ota_info = {
            'ota_manifest_path': str(ota_manifest_file),
            'package_path': str(package_path),
            'manifest': ota_manifest
        }
        
        logger.info(f"OTA package created: {ota_manifest_file}")
        return ota_info
    
    def list_deployments(self) -> List[Dict]:
        """List all available deployments."""
        deployments = []
        
        # Find all deployment packages
        for package_file in self.deployment_dir.glob('*.zip'):
            try:
                # Extract and read manifest
                with tempfile.TemporaryDirectory() as temp_dir:
                    temp_path = Path(temp_dir)
                    
                    with zipfile.ZipFile(package_file, 'r') as zipf:
                        zipf.extractall(temp_path)
                    
                    manifest_file = temp_path / 'deployment_manifest.json'
                    if manifest_file.exists():
                        with open(manifest_file, 'r') as f:
                            manifest = json.load(f)
                        
                        deployment_info = {
                            'package_file': str(package_file),
                            'model_name': manifest['deployment_info']['model_name'],
                            'version': manifest['deployment_info']['version'],
                            'created_at': manifest['deployment_info']['created_at'],
                            'size_kb': package_file.stat().st_size / 1024,
                            'model_size_kb': manifest['model_metadata']['file_size_kb']
                        }
                        deployments.append(deployment_info)
                        
            except Exception as e:
                logger.warning(f"Could not read deployment package {package_file}: {e}")
        
        # Sort by creation date (newest first)
        deployments.sort(key=lambda x: x['created_at'], reverse=True)
        
        return deployments


def main():
    """Main function for command-line usage."""
    import argparse
    
    parser = argparse.ArgumentParser(description='Deploy models to ESP32 devices')
    parser.add_argument('--prepare', type=str, help='Prepare TFLite model for deployment')
    parser.add_argument('--deploy', type=str, help='Deploy package to firmware directory')
    parser.add_argument('--validate', action='store_true', help='Validate deployment')
    parser.add_argument('--list', action='store_true', help='List available deployments')
    parser.add_argument('--ota', type=str, help='Create OTA package from deployment')
    parser.add_argument('--model-name', type=str, help='Model name for deployment')
    parser.add_argument('--version', type=str, help='Model version')
    parser.add_argument('--firmware-dir', type=str, help='Firmware directory path')
    parser.add_argument('--server-url', type=str, help='OTA server URL')
    
    args = parser.parse_args()
    
    # Initialize deployer
    config = {}
    if args.firmware_dir:
        config['firmware_dir'] = args.firmware_dir
    
    deployer = ESP32ModelDeployer(config)
    
    try:
        if args.prepare:
            package_info = deployer.prepare_model_for_deployment(
                args.prepare, args.model_name, args.version
            )
            print(f"Deployment package created: {package_info['package_path']}")
            print(f"Package size: {package_info['package_size_kb']:.1f} KB")
        
        elif args.deploy:
            success = deployer.deploy_to_firmware(args.deploy, args.firmware_dir)
            print(f"Deployment: {'Success' if success else 'Failed'}")
        
        elif args.validate:
            results = deployer.validate_deployment(args.firmware_dir)
            print(f"Validation Results:")
            for key, value in results.items():
                print(f"  {key}: {value}")
        
        elif args.list:
            deployments = deployer.list_deployments()
            print(f"Available Deployments ({len(deployments)}):")
            for deployment in deployments:
                print(f"  {deployment['model_name']} {deployment['version']}")
                print(f"    Size: {deployment['size_kb']:.1f} KB")
                print(f"    Created: {deployment['created_at']}")
                print()
        
        elif args.ota:
            ota_info = deployer.create_ota_package(args.ota, args.server_url)
            print(f"OTA package created: {ota_info['ota_manifest_path']}")
        
        else:
            print("No action specified. Use --help for available options.")
            
    except Exception as e:
        logger.error(f"Operation failed: {str(e)}")
        raise


if __name__ == "__main__":
    main()