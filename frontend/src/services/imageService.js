/**
 * Image service for ESP32 Wildlife Camera frontend
 */

import axios from 'axios';

const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:5000/api';

class ImageService {
  /**
   * Get images with filtering and pagination
   */
  async getImages(params = {}) {
    try {
      const queryParams = new URLSearchParams(params).toString();
      const response = await axios.get(`${API_BASE_URL}/images${queryParams ? '?' + queryParams : ''}`);
      return response.data;
    } catch (error) {
      console.error('Failed to fetch images:', error);
      throw error;
    }
  }
  
  /**
   * Get image details by ID
   */
  async getImage(imageId) {
    try {
      const response = await axios.get(`${API_BASE_URL}/images/${imageId}`);
      return response.data;
    } catch (error) {
      console.error(`Failed to fetch image ${imageId}:`, error);
      throw error;
    }
  }
  
  /**
   * Delete image
   */
  async deleteImage(imageId) {
    try {
      const response = await axios.delete(`${API_BASE_URL}/images/${imageId}`);
      return response.data;
    } catch (error) {
      console.error(`Failed to delete image ${imageId}:`, error);
      throw error;
    }
  }
  
  /**
   * Update image metadata
   */
  async updateImage(imageId, metadata) {
    try {
      const response = await axios.put(`${API_BASE_URL}/images/${imageId}`, metadata);
      return response.data;
    } catch (error) {
      console.error(`Failed to update image ${imageId}:`, error);
      throw error;
    }
  }
  
  /**
   * Get image detections
   */
  async getImageDetections(imageId) {
    try {
      const response = await axios.get(`${API_BASE_URL}/images/${imageId}/detections`);
      return response.data;
    } catch (error) {
      console.error(`Failed to fetch detections for image ${imageId}:`, error);
      throw error;
    }
  }
  
  /**
   * Verify detection
   */
  async verifyDetection(imageId, detectionId, verified) {
    try {
      const response = await axios.post(
        `${API_BASE_URL}/images/${imageId}/detections/${detectionId}/verify`,
        { verified }
      );
      return response.data;
    } catch (error) {
      console.error(`Failed to verify detection ${detectionId}:`, error);
      throw error;
    }
  }
  
  /**
   * Export images
   */
  async exportImages(params = {}) {
    try {
      const queryParams = new URLSearchParams(params).toString();
      const response = await axios.get(
        `${API_BASE_URL}/images/export${queryParams ? '?' + queryParams : ''}`,
        { responseType: 'blob' }
      );
      
      // Create download link
      const url = window.URL.createObjectURL(new Blob([response.data]));
      const link = document.createElement('a');
      link.href = url;
      link.setAttribute('download', `wildlife_images_${new Date().getTime()}.zip`);
      document.body.appendChild(link);
      link.click();
      link.remove();
      window.URL.revokeObjectURL(url);
      
      return { success: true };
    } catch (error) {
      console.error('Failed to export images:', error);
      throw error;
    }
  }
  
  /**
   * Search images by species
   */
  async searchBySpecies(species, params = {}) {
    try {
      const queryParams = new URLSearchParams({
        species,
        ...params
      }).toString();
      
      const response = await axios.get(`${API_BASE_URL}/images/search?${queryParams}`);
      return response.data;
    } catch (error) {
      console.error(`Failed to search images by species ${species}:`, error);
      throw error;
    }
  }
  
  /**
   * Get image tags
   */
  async getImageTags(imageId) {
    try {
      const response = await axios.get(`${API_BASE_URL}/images/${imageId}/tags`);
      return response.data;
    } catch (error) {
      console.error(`Failed to fetch tags for image ${imageId}:`, error);
      throw error;
    }
  }
  
  /**
   * Add tag to image
   */
  async addImageTag(imageId, tag) {
    try {
      const response = await axios.post(`${API_BASE_URL}/images/${imageId}/tags`, { tag });
      return response.data;
    } catch (error) {
      console.error(`Failed to add tag to image ${imageId}:`, error);
      throw error;
    }
  }
  
  /**
   * Remove tag from image
   */
  async removeImageTag(imageId, tagId) {
    try {
      const response = await axios.delete(`${API_BASE_URL}/images/${imageId}/tags/${tagId}`);
      return response.data;
    } catch (error) {
      console.error(`Failed to remove tag from image ${imageId}:`, error);
      throw error;
    }
  }
}

// Create singleton instance
const imageService = new ImageService();

export default imageService;