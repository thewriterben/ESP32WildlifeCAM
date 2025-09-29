# ESP32 Wildlife Camera ML Training Container
FROM tensorflow/tensorflow:2.13.0-gpu

LABEL maintainer="ESP32 Wildlife Camera Team"
LABEL description="Machine Learning training environment for wildlife classification models"

# Set working directory
WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    libgl1-mesa-glx \
    libglib2.0-0 \
    libsm6 \
    libxext6 \
    libxrender-dev \
    libgomp1 \
    wget \
    curl \
    git \
    && rm -rf /var/lib/apt/lists/*

# Copy requirements first for better caching
COPY requirements.txt .

# Install Python dependencies
RUN pip install --no-cache-dir -r requirements.txt

# Copy ML pipeline code
COPY . .

# Create necessary directories
RUN mkdir -p /app/datasets /app/output /app/models /app/logs

# Set environment variables
ENV PYTHONPATH=/app
ENV TF_CPP_MIN_LOG_LEVEL=2
ENV CUDA_VISIBLE_DEVICES=0

# Expose port for TensorBoard
EXPOSE 6006

# Default command
CMD ["python", "train_model.py", "--help"]

# Health check
HEALTHCHECK --interval=30s --timeout=30s --start-period=5s --retries=3 \
    CMD python -c "import tensorflow as tf; print('TensorFlow version:', tf.__version__)"

# Useful aliases and scripts
RUN echo 'alias ll="ls -la"' >> ~/.bashrc && \
    echo 'alias tb="tensorboard --logdir=/app/logs --host=0.0.0.0"' >> ~/.bashrc

# Create entrypoint script
RUN cat > /app/entrypoint.sh << 'EOF'
#!/bin/bash
set -e

echo "ESP32 Wildlife Camera ML Training Environment"
echo "============================================="
echo "TensorFlow version: $(python -c 'import tensorflow as tf; print(tf.__version__)')"
echo "GPU available: $(python -c 'import tensorflow as tf; print(len(tf.config.list_physical_devices("GPU")) > 0)')"
echo "Working directory: $(pwd)"
echo ""

# Execute the command
exec "$@"
EOF

RUN chmod +x /app/entrypoint.sh

ENTRYPOINT ["/app/entrypoint.sh"]