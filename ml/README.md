# ESP32 Wildlife Camera ML Pipeline

This directory contains the complete machine learning pipeline for the ESP32 Wildlife Camera project, including model training, evaluation, optimization, and deployment tools.

## 🚀 Quick Start

### Prerequisites

- Python 3.8+
- TensorFlow 2.13+
- NVIDIA GPU (recommended for training)
- Docker (optional)

### Installation

1. **Install dependencies:**
```bash
pip install -r requirements.txt
```

2. **Or use Docker:**
```bash
docker build -t wildlife-ml .
docker run --gpus all -v $(pwd):/app -it wildlife-ml
```

### Basic Usage

1. **Download datasets:**
```bash
python download_datasets.py --download-all --extract --organize
```

2. **Train a model:**
```bash
python train_model.py --data-dir ./datasets/combined_wildlife --epochs 50
```

3. **Convert for ESP32:**
```bash
python convert_model.py --model ./output/models/wildlife_classifier.h5 --quantization int8
```

4. **Deploy to firmware:**
```bash
python deploy_model.py --prepare ./output/models/wildlife_classifier.tflite --deploy
```

## 📁 Pipeline Components

### Core Scripts

| Script | Purpose | Usage |
|--------|---------|-------|
| `train_model.py` | Model training with transfer learning | `python train_model.py --config config.yaml` |
| `convert_model.py` | TensorFlow Lite conversion | `python convert_model.py --model model.h5` |
| `evaluate_model.py` | Model performance evaluation | `python evaluate_model.py --model model.h5 --data test_data/` |
| `edge_optimization.py` | ESP32-specific optimizations | `python edge_optimization.py --model model.h5` |
| `preprocessing.py` | Data preprocessing utilities | Used by other scripts |
| `download_datasets.py` | Dataset download and management | `python download_datasets.py --list` |
| `deploy_model.py` | Model deployment utilities | `python deploy_model.py --prepare model.tflite` |
| `benchmark_model.py` | Performance benchmarking | `python benchmark_model.py --model model.h5` |
| `validate_pipeline.py` | End-to-end validation | `python validate_pipeline.py` |

### Configuration Files

- **`config.yaml`**: Main configuration for training, model settings, and deployment
- **`requirements.txt`**: Python dependencies
- **`Dockerfile`**: Container setup for reproducible training environment

### Directory Structure

```
ml/
├── train_model.py              # Model training script
├── convert_model.py            # TFLite conversion
├── evaluate_model.py           # Model evaluation
├── edge_optimization.py        # ESP32 optimizations
├── preprocessing.py            # Data preprocessing
├── download_datasets.py        # Dataset management
├── deploy_model.py             # Deployment utilities
├── benchmark_model.py          # Performance benchmarking
├── validate_pipeline.py        # Pipeline validation
├── config.yaml                 # Configuration file
├── requirements.txt            # Dependencies
├── Dockerfile                  # Container setup
├── README.md                   # This file
└── datasets/                   # Dataset storage
    ├── wildlife/               # Raw datasets
    ├── custom/                 # Custom datasets
    ├── annotations/            # Annotation files
    └── metadata/               # Dataset metadata
```

## 🏋️ Model Training

### Supported Architectures

- **MobileNetV2** (recommended for ESP32)
- **EfficientNetB0/B1**
- **ResNet50**
- Custom CNN architectures

### Training Features

- **Transfer Learning**: Pre-trained ImageNet weights
- **Data Augmentation**: Advanced augmentation pipeline
- **Multi-GPU Support**: Distributed training
- **Hyperparameter Tuning**: Automated optimization
- **Early Stopping**: Prevent overfitting
- **Learning Rate Scheduling**: Adaptive learning rates

### Example Training Command

```bash
python train_model.py \
    --config config.yaml \
    --data-dir ./datasets/wildlife \
    --output-dir ./output \
    --epochs 100 \
    --batch-size 32 \
    --model-name wildlife_classifier_v1
```

## 🔄 Model Conversion

### TensorFlow Lite Conversion

Convert trained models for ESP32 deployment:

```bash
python convert_model.py \
    --model ./output/models/wildlife_classifier.h5 \
    --quantization int8 \
    --optimization-target size
```

### Quantization Options

- **None**: No quantization (largest, most accurate)
- **Dynamic**: Runtime quantization (good balance)
- **INT8**: Full integer quantization (smallest, fastest)
- **Float16**: Half-precision (moderate compression)

### ESP32 Optimizations

```bash
python edge_optimization.py \
    --model ./output/models/wildlife_classifier.h5 \
    --profile \
    --benchmark
```

## 📊 Model Evaluation

### Comprehensive Metrics

- **Accuracy**: Overall and per-species
- **Precision/Recall**: Class-specific performance
- **Confusion Matrix**: Detailed error analysis
- **ROC Curves**: Binary classification performance
- **Inference Time**: Speed analysis
- **Memory Usage**: Resource profiling

### Evaluation Command

```bash
python evaluate_model.py \
    --model ./output/models/wildlife_classifier.h5 \
    --data ./datasets/wildlife/test \
    --output ./evaluation_results
```

## 🚀 Model Deployment

### Deployment Pipeline

1. **Model Preparation**: Convert to TFLite and generate C++ headers
2. **Firmware Integration**: Deploy to ESP32 firmware
3. **Validation**: Test deployment
4. **OTA Updates**: Over-the-air model updates

### Deployment Commands

```bash
# Prepare deployment package
python deploy_model.py \
    --prepare ./output/models/wildlife_classifier.tflite \
    --model-name wildlife_v1 \
    --version v1.0.0

# Deploy to firmware
python deploy_model.py \
    --deploy ./deployments/wildlife_v1_v1.0.0.zip \
    --firmware-dir ../firmware

# Validate deployment
python deploy_model.py --validate
```

## 📈 Performance Benchmarking

### Benchmark Single Model

```bash
python benchmark_model.py \
    --model ./output/models/wildlife_classifier.h5 \
    --output ./benchmark_results
```

### Compare Multiple Models

```bash
python benchmark_model.py \
    --models model1.h5 model2.tflite model3.h5 \
    --names "Original" "Quantized" "Optimized" \
    --output ./comparison_results
```

## 🧪 Pipeline Validation

Validate the entire ML pipeline:

```bash
python validate_pipeline.py --output ./validation_results
```

This runs:
- Environment checks
- Component integration tests
- End-to-end pipeline validation
- Performance benchmarking
- ESP32 compatibility tests

## 🐾 Supported Wildlife Species

The pipeline supports classification of 15+ wildlife species:

- **Mammals**: Deer, Bear, Fox, Wolf, Raccoon, Rabbit, Squirrel, Coyote, Bobcat, Elk, Moose
- **Others**: Wild Boar, Mountain Lion, Lynx, Birds
- **Unknown**: Fallback category

## 📋 Model Requirements

### ESP32 Constraints

- **Model Size**: ≤ 2MB (recommended)
- **Memory Usage**: ≤ 8MB PSRAM
- **Inference Time**: ≤ 2 seconds per image
- **Accuracy Target**: ≥ 85% for supported species

### Performance Targets

- **Accuracy**: 85%+ on test dataset
- **Inference Speed**: < 2s per image on ESP32
- **Model Size**: < 2MB for deployment
- **Memory Efficient**: < 8MB RAM usage

## 🔧 Configuration

### Main Configuration (`config.yaml`)

Key configuration sections:

```yaml
# Dataset configuration
dataset:
  data_dir: "./datasets/wildlife"
  species: ["deer", "bear", "fox", ...]
  train_split: 0.7
  validation_split: 0.2
  test_split: 0.1

# Model configuration
model:
  architecture: "MobileNetV2"
  input_shape: [224, 224, 3]
  num_classes: 16
  use_transfer_learning: true

# Training configuration
training:
  batch_size: 32
  epochs: 100
  learning_rate: 0.001
  data_augmentation:
    enabled: true
    rotation_range: 20

# Edge deployment
edge_deployment:
  target_device: "ESP32"
  quantization: "int8"
  optimization_target: "size"
```

## 🐳 Docker Usage

### Build Container

```bash
docker build -t wildlife-ml .
```

### Run Training

```bash
docker run --gpus all \
    -v $(pwd)/datasets:/app/datasets \
    -v $(pwd)/output:/app/output \
    wildlife-ml \
    python train_model.py --config config.yaml
```

### Interactive Development

```bash
docker run --gpus all \
    -v $(pwd):/app \
    -p 6006:6006 \
    -it wildlife-ml bash
```

## 📊 Monitoring and Visualization

### TensorBoard

Monitor training progress:

```bash
tensorboard --logdir ./output/logs --host 0.0.0.0
```

### Weights & Biases (Optional)

Configure in `config.yaml`:

```yaml
logging:
  wandb:
    enabled: true
    project: "esp32-wildlife-cam"
    entity: "your-team"
```

## 🚨 Troubleshooting

### Common Issues

1. **GPU Out of Memory**
   - Reduce batch size
   - Use model parallelism
   - Enable mixed precision

2. **Slow Training**
   - Check GPU utilization
   - Increase batch size
   - Use multiple workers

3. **Poor Accuracy**
   - Increase training data
   - Adjust learning rate
   - Try different architectures

4. **Large Model Size**
   - Use quantization
   - Prune unnecessary layers
   - Apply compression techniques

### Performance Optimization

1. **Training Speed**
   ```bash
   # Use mixed precision
   export TF_ENABLE_AUTO_MIXED_PRECISION=1
   
   # Optimize data pipeline
   export TF_DATA_EXPERIMENTAL_OPTIMIZATION=1
   ```

2. **Memory Usage**
   ```bash
   # Limit GPU memory growth
   export TF_FORCE_GPU_ALLOW_GROWTH=true
   ```

3. **Reproducibility**
   ```bash
   # Set random seeds
   export PYTHONHASHSEED=0
   export TF_DETERMINISTIC_OPS=1
   ```

## 🤝 Contributing

1. Follow the existing code structure
2. Add tests for new functionality
3. Update documentation
4. Run validation pipeline before submitting

## 📄 License

This project is part of the ESP32 Wildlife Camera system. See the main project LICENSE for details.

## 🆘 Support

For questions and support:

1. Check the troubleshooting section
2. Run the validation pipeline
3. Review logs in `./output/logs/`
4. Open an issue in the main repository

## 🔄 Updates and Versioning

Models are versioned using semantic versioning (vX.Y.Z):
- **X**: Major architecture changes
- **Y**: Performance improvements
- **Z**: Bug fixes and minor updates

Latest stable version: Check deployment manifests in `./deployments/`