#!/usr/bin/env python3
"""
Automatic C++ Header Generator for ESP32WildlifeCAM v3.1
Generates C++ header files with quantized TensorFlow Lite models for ESP32 deployment
"""

import os
import json
import argparse
import logging
from pathlib import Path
from typing import Dict, List, Optional, Any
from datetime import datetime

logger = logging.getLogger(__name__)

class ESP32HeaderGenerator:
    """Generates C++ header files with embedded TensorFlow Lite models"""
    
    def __init__(self):
        self.template_dir = Path(__file__).parent / 'templates'
        
    def load_model_metadata(self, metadata_path: str) -> Dict[str, Any]:
        """Load model metadata from JSON file"""
        with open(metadata_path, 'r') as f:
            return json.load(f)
    
    def convert_tflite_to_array(self, tflite_path: str) -> Tuple[List[int], int]:
        """Convert TensorFlow Lite model to C++ byte array"""
        with open(tflite_path, 'rb') as f:
            model_data = f.read()
        
        # Convert to list of integers
        byte_array = list(model_data)
        model_size = len(byte_array)
        
        return byte_array, model_size
    
    def generate_species_header(self, 
                              model_path: str,
                              metadata_path: str,
                              output_path: str,
                              region: str) -> str:
        """Generate species classification header file"""
        
        # Load model data and metadata
        byte_array, model_size = self.convert_tflite_to_array(model_path)
        metadata = self.load_model_metadata(metadata_path)
        
        # Extract species information
        species_list = metadata['species_metadata']['species_list']
        species_mapping = metadata['species_metadata']['species_mapping']
        num_species = len(species_list)
        
        # Generate header content
        header_content = self._generate_species_header_content(
            byte_array=byte_array,
            model_size=model_size,
            species_list=species_list,
            species_mapping=species_mapping,
            num_species=num_species,
            region=region,
            metadata=metadata
        )
        
        # Write to file
        output_path = Path(output_path)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(output_path, 'w') as f:
            f.write(header_content)
        
        logger.info(f"Generated species header: {output_path} ({model_size} bytes)")
        return str(output_path)
    
    def _generate_species_header_content(self,
                                       byte_array: List[int],
                                       model_size: int,
                                       species_list: List[str],
                                       species_mapping: Dict[str, int],
                                       num_species: int,
                                       region: str,
                                       metadata: Dict[str, Any]) -> str:
        """Generate the actual header file content"""
        
        # Format byte array for C++
        byte_array_str = self._format_byte_array(byte_array)
        
        # Generate species name array
        species_names_array = self._generate_species_names_array(species_list)
        
        # Generate species mapping constants
        species_constants = self._generate_species_constants(species_mapping)
        
        # Generate conservation status mapping if available
        conservation_mapping = ""
        if 'conservation_classes' in metadata['species_metadata']:
            conservation_mapping = self._generate_conservation_mapping(
                metadata['species_metadata']['conservation_classes']
            )
        
        # Get current timestamp
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        # Generate header content
        header_content = f"""/**
 * ESP32 Wildlife Camera - Enhanced Species Classification Model
 * Region: {region.upper()}
 * Generated on: {timestamp}
 * Model size: {model_size:,} bytes
 * Number of species: {num_species}
 * 
 * This file contains the quantized TensorFlow Lite model for wildlife species
 * classification optimized for ESP32 deployment.
 */

#ifndef ENHANCED_SPECIES_MODEL_{region.upper()}_H
#define ENHANCED_SPECIES_MODEL_{region.upper()}_H

#include <Arduino.h>

// Model configuration
#define SPECIES_MODEL_REGION "{region}"
#define SPECIES_MODEL_VERSION "3.1.0"
#define SPECIES_MODEL_SIZE {model_size}
#define SPECIES_NUM_CLASSES {num_species}
#define SPECIES_INPUT_WIDTH {metadata['input_shape'][1]}
#define SPECIES_INPUT_HEIGHT {metadata['input_shape'][0]}
#define SPECIES_INPUT_CHANNELS {metadata['input_shape'][2]}

// Model accuracy metrics
#define SPECIES_MODEL_ACCURACY {metadata.get('evaluation_metrics', {}).get('species_output_accuracy', 0.0):.4f}
#define SPECIES_MODEL_TOP3_ACCURACY {metadata.get('evaluation_metrics', {}).get('species_output_top_3_categorical_accuracy', 0.0):.4f}

// Memory requirements (estimated)
#define SPECIES_TENSOR_ARENA_SIZE (128 * 1024)  // 128KB tensor arena
#define SPECIES_MODEL_MEMORY_USAGE (SPECIES_MODEL_SIZE + SPECIES_TENSOR_ARENA_SIZE)

{species_constants}

// Species names array
const char* const SPECIES_NAMES[SPECIES_NUM_CLASSES] = {{
{species_names_array}
}};

{conservation_mapping}

// Quantized TensorFlow Lite model data
const unsigned char SPECIES_MODEL_DATA[SPECIES_MODEL_SIZE] = {{
{byte_array_str}
}};

// Model configuration structure
struct SpeciesModelConfig {{
    const char* region;
    const char* version;
    int num_classes;
    int input_width;
    int input_height;
    int input_channels;
    float accuracy;
    int model_size;
    int tensor_arena_size;
    const unsigned char* model_data;
    const char* const* species_names;
}};

// Global model configuration instance
const SpeciesModelConfig SPECIES_MODEL_CONFIG = {{
    SPECIES_MODEL_REGION,
    SPECIES_MODEL_VERSION,
    SPECIES_NUM_CLASSES,
    SPECIES_INPUT_WIDTH,
    SPECIES_INPUT_HEIGHT,
    SPECIES_INPUT_CHANNELS,
    SPECIES_MODEL_ACCURACY,
    SPECIES_MODEL_SIZE,
    SPECIES_TENSOR_ARENA_SIZE,
    SPECIES_MODEL_DATA,
    SPECIES_NAMES
}};

// Utility functions
inline const char* getSpeciesName(int class_id) {{
    if (class_id >= 0 && class_id < SPECIES_NUM_CLASSES) {{
        return SPECIES_NAMES[class_id];
    }}
    return "unknown";
}}

inline int getSpeciesId(const char* species_name) {{
    for (int i = 0; i < SPECIES_NUM_CLASSES; i++) {{
        if (strcmp(SPECIES_NAMES[i], species_name) == 0) {{
            return i;
        }}
    }}
    return -1;
}}

// Model validation
inline bool validateModelIntegrity() {{
    // Basic model size check
    return sizeof(SPECIES_MODEL_DATA) == SPECIES_MODEL_SIZE;
}}

// Regional species information
struct RegionalSpeciesInfo {{
    const char* region_name;
    const char* const* countries;
    int num_countries;
    bool is_endangered;
    bool is_protected;
    const char* conservation_status;
}};

// Performance optimization hints
#define SPECIES_INFERENCE_BATCH_SIZE 1
#define SPECIES_CONFIDENCE_THRESHOLD 0.6f
#define SPECIES_TOP_K_RESULTS 3

// Error codes
#define SPECIES_MODEL_OK 0
#define SPECIES_MODEL_ERROR_INIT -1
#define SPECIES_MODEL_ERROR_INPUT -2
#define SPECIES_MODEL_ERROR_INFERENCE -3
#define SPECIES_MODEL_ERROR_OUTPUT -4

#endif // ENHANCED_SPECIES_MODEL_{region.upper()}_H
"""
        
        return header_content
    
    def _format_byte_array(self, byte_array: List[int], bytes_per_line: int = 16) -> str:
        """Format byte array for C++ header"""
        lines = []
        for i in range(0, len(byte_array), bytes_per_line):
            chunk = byte_array[i:i + bytes_per_line]
            hex_values = [f"0x{b:02x}" for b in chunk]
            line = "    " + ", ".join(hex_values)
            if i + bytes_per_line < len(byte_array):
                line += ","
            lines.append(line)
        
        return "\n".join(lines)
    
    def _generate_species_names_array(self, species_list: List[str]) -> str:
        """Generate species names array for C++"""
        lines = []
        for i, species in enumerate(species_list):
            line = f'    "{species}"'
            if i < len(species_list) - 1:
                line += ","
            lines.append(line)
        
        return "\n".join(lines)
    
    def _generate_species_constants(self, species_mapping: Dict[str, int]) -> str:
        """Generate species ID constants"""
        lines = ["// Species ID constants"]
        
        for species_name, species_id in sorted(species_mapping.items(), key=lambda x: x[1]):
            constant_name = f"SPECIES_{species_name.upper().replace(' ', '_')}"
            lines.append(f"#define {constant_name} {species_id}")
        
        return "\n".join(lines)
    
    def _generate_conservation_mapping(self, conservation_classes: List[str]) -> str:
        """Generate conservation status mapping"""
        lines = ["// Conservation status constants"]
        
        status_map = {
            'LC': 'LEAST_CONCERN',
            'NT': 'NEAR_THREATENED', 
            'VU': 'VULNERABLE',
            'EN': 'ENDANGERED',
            'CR': 'CRITICALLY_ENDANGERED',
            'EW': 'EXTINCT_IN_WILD',
            'EX': 'EXTINCT',
            'DD': 'DATA_DEFICIENT'
        }
        
        for i, status_code in enumerate(conservation_classes):
            if status_code in status_map:
                constant_name = f"CONSERVATION_{status_map[status_code]}"
                lines.append(f"#define {constant_name} {i}")
        
        return "\n".join(lines)
    
    def generate_model_manager_header(self, 
                                    models_info: List[Dict[str, Any]], 
                                    output_path: str) -> str:
        """Generate a model manager header that handles multiple regional models"""
        
        # Generate includes for all regional models
        includes = []
        model_configs = []
        region_enum = []
        
        for i, model_info in enumerate(models_info):
            region = model_info['region']
            includes.append(f'#include "enhanced_species_model_{region}.h"')
            model_configs.append(f"    &SPECIES_MODEL_CONFIG_{region.upper()}")
            region_enum.append(f"    REGION_{region.upper()} = {i}")
        
        includes_str = "\n".join(includes)
        model_configs_str = ",\n".join(model_configs)
        region_enum_str = ",\n".join(region_enum)
        
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        header_content = f"""/**
 * ESP32 Wildlife Camera - Enhanced Model Manager
 * Generated on: {timestamp}
 * 
 * This file manages multiple regional species classification models
 * and provides a unified interface for model selection and inference.
 */

#ifndef ENHANCED_MODEL_MANAGER_H
#define ENHANCED_MODEL_MANAGER_H

#include <Arduino.h>

// Include all regional models
{includes_str}

// Region enumeration
enum WildlifeRegion {{
{region_enum_str},
    REGION_COUNT
}};

// Global model configuration array
const SpeciesModelConfig* REGIONAL_MODELS[REGION_COUNT] = {{
{model_configs_str}
}};

// Model manager class
class EnhancedModelManager {{
private:
    WildlifeRegion current_region;
    const SpeciesModelConfig* current_config;
    
public:
    EnhancedModelManager() : current_region(REGION_NORTH_AMERICA) {{
        current_config = REGIONAL_MODELS[current_region];
    }}
    
    // Set active region
    bool setRegion(WildlifeRegion region) {{
        if (region >= 0 && region < REGION_COUNT) {{
            current_region = region;
            current_config = REGIONAL_MODELS[region];
            return true;
        }}
        return false;
    }}
    
    // Get current region
    WildlifeRegion getCurrentRegion() const {{
        return current_region;
    }}
    
    // Get current model configuration
    const SpeciesModelConfig* getCurrentConfig() const {{
        return current_config;
    }}
    
    // Get species name by ID for current region
    const char* getSpeciesName(int class_id) const {{
        if (current_config && class_id >= 0 && class_id < current_config->num_classes) {{
            return current_config->species_names[class_id];
        }}
        return "unknown";
    }}
    
    // Get total memory usage for current model
    int getMemoryUsage() const {{
        if (current_config) {{
            return current_config->model_size + current_config->tensor_arena_size;
        }}
        return 0;
    }}
    
    // Validate all models
    bool validateAllModels() const {{
        for (int i = 0; i < REGION_COUNT; i++) {{
            const SpeciesModelConfig* config = REGIONAL_MODELS[i];
            if (!config || config->model_size <= 0) {{
                return false;
            }}
        }}
        return true;
    }}
    
    // Get region name string
    const char* getRegionName(WildlifeRegion region) const {{
        if (region >= 0 && region < REGION_COUNT) {{
            return REGIONAL_MODELS[region]->region;
        }}
        return "unknown";
    }}
    
    // Auto-select region based on GPS coordinates (placeholder)
    WildlifeRegion suggestRegion(float latitude, float longitude) const {{
        // Simple geographic region detection
        // In production, this would use more sophisticated geographic boundaries
        
        if (latitude >= 15.0f && latitude <= 75.0f && 
            longitude >= -170.0f && longitude <= -50.0f) {{
            return REGION_NORTH_AMERICA;
        }}
        else if (latitude >= 35.0f && latitude <= 70.0f && 
                 longitude >= -15.0f && longitude <= 45.0f) {{
            return REGION_EUROPE;
        }}
        else if (latitude >= -45.0f && latitude <= 80.0f && 
                 longitude >= 25.0f && longitude <= 180.0f) {{
            return REGION_ASIA;
        }}
        else if (latitude >= -35.0f && latitude <= 40.0f && 
                 longitude >= -20.0f && longitude <= 55.0f) {{
            return REGION_AFRICA;
        }}
        else if (latitude >= -45.0f && latitude <= -10.0f && 
                 longitude >= 110.0f && longitude <= 180.0f) {{
            return REGION_AUSTRALIA;
        }}
        
        return REGION_NORTH_AMERICA; // Default
    }}
}};

// Global model manager instance
extern EnhancedModelManager g_model_manager;

#endif // ENHANCED_MODEL_MANAGER_H
"""
        
        # Write to file
        output_path = Path(output_path)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(output_path, 'w') as f:
            f.write(header_content)
        
        logger.info(f"Generated model manager header: {output_path}")
        return str(output_path)

def main():
    """Main function for C++ header generation"""
    parser = argparse.ArgumentParser(description='Generate C++ headers for ESP32 deployment')
    
    parser.add_argument('--model', type=str, required=True,
                       help='Path to quantized TensorFlow Lite model (.tflite)')
    parser.add_argument('--metadata', type=str, required=True,
                       help='Path to model metadata JSON file')
    parser.add_argument('--output', type=str, required=True,
                       help='Output path for C++ header file')
    parser.add_argument('--region', type=str, required=True,
                       help='Model region (e.g., north_america, europe)')
    parser.add_argument('--generate-manager', action='store_true',
                       help='Generate model manager header for multiple regions')
    
    args = parser.parse_args()
    
    # Initialize generator
    generator = ESP32HeaderGenerator()
    
    # Generate species header
    header_path = generator.generate_species_header(
        model_path=args.model,
        metadata_path=args.metadata,
        output_path=args.output,
        region=args.region
    )
    
    logger.info(f"Successfully generated C++ header: {header_path}")
    
    # Optionally generate model manager
    if args.generate_manager:
        manager_output = str(Path(args.output).parent / "enhanced_model_manager.h")
        
        # For demo, create a single model info
        models_info = [{'region': args.region}]
        
        manager_path = generator.generate_model_manager_header(
            models_info=models_info,
            output_path=manager_output
        )
        
        logger.info(f"Generated model manager header: {manager_path}")

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    main()