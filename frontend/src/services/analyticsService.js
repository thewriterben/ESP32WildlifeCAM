/**
 * Analytics service for ESP32 Wildlife Camera frontend
 */

import axios from 'axios';

const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:5000/api';

class AnalyticsService {
  /**
   * Get dashboard analytics
   */
  async getDashboardAnalytics(days = 7) {
    try {
      const response = await axios.get(`${API_BASE_URL}/analytics/dashboard?days=${days}`);
      return response.data;
    } catch (error) {
      console.error('Failed to fetch dashboard analytics:', error);
      throw error;
    }
  }
  
  /**
   * Get species report
   */
  async getSpeciesReport(speciesId, startDate = null, endDate = null) {
    try {
      const params = new URLSearchParams();
      if (startDate) params.append('start_date', startDate);
      if (endDate) params.append('end_date', endDate);
      
      const queryString = params.toString();
      const url = `${API_BASE_URL}/analytics/species/${speciesId}${queryString ? '?' + queryString : ''}`;
      
      const response = await axios.get(url);
      return response.data;
    } catch (error) {
      console.error(`Failed to fetch species report for ${speciesId}:`, error);
      throw error;
    }
  }
  
  /**
   * Get camera performance analytics
   */
  async getCameraPerformance(cameraId, days = 30) {
    try {
      const response = await axios.get(
        `${API_BASE_URL}/analytics/cameras/${cameraId}/performance?days=${days}`
      );
      return response.data;
    } catch (error) {
      console.error(`Failed to fetch camera performance for ${cameraId}:`, error);
      throw error;
    }
  }
  
  /**
   * Get detection trends
   */
  async getDetectionTrends(params = {}) {
    try {
      const queryParams = new URLSearchParams(params).toString();
      const response = await axios.get(
        `${API_BASE_URL}/analytics/trends${queryParams ? '?' + queryParams : ''}`
      );
      return response.data;
    } catch (error) {
      console.error('Failed to fetch detection trends:', error);
      throw error;
    }
  }
  
  /**
   * Get activity patterns
   */
  async getActivityPatterns(params = {}) {
    try {
      const queryParams = new URLSearchParams(params).toString();
      const response = await axios.get(
        `${API_BASE_URL}/analytics/activity${queryParams ? '?' + queryParams : ''}`
      );
      return response.data;
    } catch (error) {
      console.error('Failed to fetch activity patterns:', error);
      throw error;
    }
  }
  
  /**
   * Get conservation alerts
   */
  async getConservationAlerts() {
    try {
      const response = await axios.get(`${API_BASE_URL}/analytics/conservation-alerts`);
      return response.data;
    } catch (error) {
      console.error('Failed to fetch conservation alerts:', error);
      throw error;
    }
  }
  
  /**
   * Get biodiversity metrics
   */
  async getBiodiversityMetrics(params = {}) {
    try {
      const queryParams = new URLSearchParams(params).toString();
      const response = await axios.get(
        `${API_BASE_URL}/analytics/biodiversity${queryParams ? '?' + queryParams : ''}`
      );
      return response.data;
    } catch (error) {
      console.error('Failed to fetch biodiversity metrics:', error);
      throw error;
    }
  }
  
  /**
   * Get system health metrics
   */
  async getSystemHealth() {
    try {
      const response = await axios.get(`${API_BASE_URL}/analytics/system-health`);
      return response.data;
    } catch (error) {
      console.error('Failed to fetch system health:', error);
      throw error;
    }
  }
  
  /**
   * Export analytics data
   */
  async exportData(type, params = {}) {
    try {
      const queryParams = new URLSearchParams({
        type,
        ...params
      }).toString();
      
      const response = await axios.get(
        `${API_BASE_URL}/analytics/export?${queryParams}`,
        { responseType: 'blob' }
      );
      
      // Create download link
      const url = window.URL.createObjectURL(new Blob([response.data]));
      const link = document.createElement('a');
      link.href = url;
      link.setAttribute('download', `wildlife_analytics_${type}_${new Date().getTime()}.csv`);
      document.body.appendChild(link);
      link.click();
      link.remove();
      window.URL.revokeObjectURL(url);
      
      return { success: true };
    } catch (error) {
      console.error('Failed to export analytics data:', error);
      throw error;
    }
  }
  
  /**
   * Get monthly summary
   */
  async getMonthlySummary(year, month) {
    try {
      const response = await axios.get(
        `${API_BASE_URL}/analytics/monthly-summary?year=${year}&month=${month}`
      );
      return response.data;
    } catch (error) {
      console.error('Failed to fetch monthly summary:', error);
      throw error;
    }
  }
  
  /**
   * Get comparison data
   */
  async getComparisonData(compareType, period1, period2) {
    try {
      const response = await axios.post(`${API_BASE_URL}/analytics/compare`, {
        type: compareType,
        period1,
        period2
      });
      return response.data;
    } catch (error) {
      console.error('Failed to fetch comparison data:', error);
      throw error;
    }
  }
  
  /**
   * Get real-time metrics
   */
  async getRealTimeMetrics() {
    try {
      const response = await axios.get(`${API_BASE_URL}/analytics/realtime`);
      return response.data;
    } catch (error) {
      console.error('Failed to fetch real-time metrics:', error);
      throw error;
    }
  }
  
  /**
   * Get custom report
   */
  async getCustomReport(reportConfig) {
    try {
      const response = await axios.post(`${API_BASE_URL}/analytics/custom-report`, reportConfig);
      return response.data;
    } catch (error) {
      console.error('Failed to generate custom report:', error);
      throw error;
    }
  }
}

// Create singleton instance
const analyticsService = new AnalyticsService();

export default analyticsService;