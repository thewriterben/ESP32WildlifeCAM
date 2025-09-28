/**
 * Camera service for ESP32 Wildlife Camera frontend
 */

import axios from 'axios';

const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:5000/api';

class CameraService {
  /**
   * Get all cameras for the current user
   */
  async getCameras() {
    try {
      const response = await axios.get(`${API_BASE_URL}/cameras`);
      return response.data;
    } catch (error) {
      console.error('Failed to fetch cameras:', error);
      throw error;
    }
  }
  
  /**
   * Get camera details by ID
   */
  async getCamera(cameraId) {
    try {
      const response = await axios.get(`${API_BASE_URL}/cameras/${cameraId}`);
      return response.data;
    } catch (error) {
      console.error(`Failed to fetch camera ${cameraId}:`, error);
      throw error;
    }
  }
  
  /**
   * Register a new camera
   */
  async registerCamera(cameraData) {
    try {
      const response = await axios.post(`${API_BASE_URL}/cameras`, cameraData);
      return response.data;
    } catch (error) {
      console.error('Failed to register camera:', error);
      throw error;
    }
  }
  
  /**
   * Update camera configuration
   */
  async updateCamera(cameraId, updates) {
    try {
      const response = await axios.put(`${API_BASE_URL}/cameras/${cameraId}`, updates);
      return response.data;
    } catch (error) {
      console.error(`Failed to update camera ${cameraId}:`, error);
      throw error;
    }
  }
  
  /**
   * Delete camera
   */
  async deleteCamera(cameraId) {
    try {
      const response = await axios.delete(`${API_BASE_URL}/cameras/${cameraId}`);
      return response.data;
    } catch (error) {
      console.error(`Failed to delete camera ${cameraId}:`, error);
      throw error;
    }
  }
  
  /**
   * Update camera status (heartbeat)
   */
  async updateCameraStatus(cameraId, statusData) {
    try {
      const response = await axios.post(`${API_BASE_URL}/cameras/${cameraId}/status`, statusData);
      return response.data;
    } catch (error) {
      console.error(`Failed to update camera status ${cameraId}:`, error);
      throw error;
    }
  }
  
  /**
   * Upload image to camera
   */
  async uploadImage(cameraId, imageFile, metadata = {}) {
    try {
      const formData = new FormData();
      formData.append('image', imageFile);
      formData.append('metadata', JSON.stringify(metadata));
      
      const response = await axios.post(
        `${API_BASE_URL}/cameras/${cameraId}/upload`,
        formData,
        {
          headers: {
            'Content-Type': 'multipart/form-data',
          },
        }
      );
      
      return response.data;
    } catch (error) {
      console.error(`Failed to upload image to camera ${cameraId}:`, error);
      throw error;
    }
  }
  
  /**
   * Get camera images
   */
  async getCameraImages(cameraId, params = {}) {
    try {
      const queryParams = new URLSearchParams({
        camera_id: cameraId,
        ...params
      }).toString();
      
      const response = await axios.get(`${API_BASE_URL}/images?${queryParams}`);
      return response.data;
    } catch (error) {
      console.error(`Failed to fetch images for camera ${cameraId}:`, error);
      throw error;
    }
  }
  
  /**
   * Get camera statistics
   */
  async getCameraStats(cameraId, days = 7) {
    try {
      const response = await axios.get(
        `${API_BASE_URL}/analytics/cameras/${cameraId}/performance?days=${days}`
      );
      return response.data;
    } catch (error) {
      console.error(`Failed to fetch stats for camera ${cameraId}:`, error);
      throw error;
    }
  }
  
  /**
   * Send configuration to camera
   */
  async sendConfiguration(cameraId, config) {
    try {
      const response = await axios.post(
        `${API_BASE_URL}/cameras/${cameraId}/configure`,
        config
      );
      return response.data;
    } catch (error) {
      console.error(`Failed to send configuration to camera ${cameraId}:`, error);
      throw error;
    }
  }
  
  /**
   * Test camera connection
   */
  async testConnection(cameraId) {
    try {
      const response = await axios.post(`${API_BASE_URL}/cameras/${cameraId}/test`);
      return response.data;
    } catch (error) {
      console.error(`Failed to test connection to camera ${cameraId}:`, error);
      throw error;
    }
  }
  
  /**
   * Get camera logs
   */
  async getCameraLogs(cameraId, limit = 100) {
    try {
      const response = await axios.get(
        `${API_BASE_URL}/cameras/${cameraId}/logs?limit=${limit}`
      );
      return response.data;
    } catch (error) {
      console.error(`Failed to fetch logs for camera ${cameraId}:`, error);
      throw error;
    }
  }
  
  /**
   * Restart camera
   */
  async restartCamera(cameraId) {
    try {
      const response = await axios.post(`${API_BASE_URL}/cameras/${cameraId}/restart`);
      return response.data;
    } catch (error) {
      console.error(`Failed to restart camera ${cameraId}:`, error);
      throw error;
    }
  }
  
  /**
   * Update camera firmware
   */
  async updateFirmware(cameraId, firmwareFile) {
    try {
      const formData = new FormData();
      formData.append('firmware', firmwareFile);
      
      const response = await axios.post(
        `${API_BASE_URL}/cameras/${cameraId}/firmware-update`,
        formData,
        {
          headers: {
            'Content-Type': 'multipart/form-data',
          },
        }
      );
      
      return response.data;
    } catch (error) {
      console.error(`Failed to update firmware for camera ${cameraId}:`, error);
      throw error;
    }
  }
}

// Create singleton instance
const cameraService = new CameraService();

export default cameraService;