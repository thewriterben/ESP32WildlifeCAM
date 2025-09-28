#!/bin/bash
# ESP32 Wildlife Camera - Automated Setup Script
# This script sets up the complete development environment

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check system requirements
check_system_requirements() {
    log_info "Checking system requirements..."
    
    # Check for required commands
    local required_commands=("python3" "node" "npm" "git" "curl")
    local missing_commands=()
    
    for cmd in "${required_commands[@]}"; do
        if ! command_exists "$cmd"; then
            missing_commands+=("$cmd")
        fi
    done
    
    if [ ${#missing_commands[@]} -ne 0 ]; then
        log_error "Missing required commands: ${missing_commands[*]}"
        log_info "Please install the missing commands and run this script again."
        exit 1
    fi
    
    # Check Python version
    python_version=$(python3 --version 2>&1 | cut -d' ' -f2)
    python_major=$(echo "$python_version" | cut -d'.' -f1)
    python_minor=$(echo "$python_version" | cut -d'.' -f2)
    
    if [ "$python_major" -lt 3 ] || ([ "$python_major" -eq 3 ] && [ "$python_minor" -lt 8 ]); then
        log_error "Python 3.8+ is required. Found: $python_version"
        exit 1
    fi
    
    # Check Node.js version
    node_version=$(node --version | sed 's/v//')
    node_major=$(echo "$node_version" | cut -d'.' -f1)
    
    if [ "$node_major" -lt 16 ]; then
        log_error "Node.js 16+ is required. Found: $node_version"
        exit 1
    fi
    
    log_success "System requirements check passed"
}

# Install PlatformIO
install_platformio() {
    log_info "Installing PlatformIO..."
    
    if ! command_exists pio; then
        # Install PlatformIO Core
        python3 -m pip install --user platformio
        
        # Add to PATH if not already there
        if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
            echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
            export PATH="$HOME/.local/bin:$PATH"
        fi
        
        log_success "PlatformIO installed successfully"
    else
        log_success "PlatformIO is already installed"
    fi
}

# Setup Python virtual environment
setup_python_env() {
    log_info "Setting up Python environment..."
    
    # Create virtual environment if it doesn't exist
    if [ ! -d "venv" ]; then
        python3 -m venv venv
        log_success "Python virtual environment created"
    fi
    
    # Activate virtual environment
    source venv/bin/activate
    
    # Upgrade pip
    pip install --upgrade pip
    
    # Install backend requirements
    if [ -f "backend/requirements.txt" ]; then
        log_info "Installing Python dependencies..."
        pip install -r backend/requirements.txt
        log_success "Python dependencies installed"
    fi
}

# Setup Node.js environment
setup_node_env() {
    log_info "Setting up Node.js environment..."
    
    # Install frontend dependencies
    if [ -d "frontend" ] && [ -f "frontend/package.json" ]; then
        cd frontend
        log_info "Installing frontend dependencies..."
        npm install
        log_success "Frontend dependencies installed"
        cd ..
    fi
}

# Main setup function
main() {
    echo "=================================================="
    echo "ESP32 Wildlife Camera - Setup Script"
    echo "=================================================="
    echo
    
    # Check system requirements
    check_system_requirements
    
    # Install dependencies
    install_platformio
    
    # Setup environments
    setup_python_env
    setup_node_env
    
    echo
    log_success "Setup completed successfully!"
    echo
    echo "Next steps:"
    echo "1. Review configuration files"
    echo "2. Build firmware: platformio run"
    echo "3. Start backend: cd backend && python app.py"
    echo "4. Start frontend: cd frontend && npm start"
    echo
}

# Run main function
main "$@"