import apiClient from './apiClient';
import {DashboardData, BiodiversityMetrics, ActivityPattern, PopulationTrend, ConservationAlert, ApiResponse} from '@types/index';

class AnalyticsService {
  async getDashboardData(timeRange: string = '7d'): Promise<ApiResponse<DashboardData>> {
    const response = await apiClient.get<DashboardData>(`/analytics/dashboard?timeRange=${timeRange}`);
    return response.data;
  }

  async getBiodiversityMetrics(params: {
    startDate: string;
    endDate: string;
    cameraIds?: number[];
  }): Promise<ApiResponse<BiodiversityMetrics>> {
    const response = await apiClient.post<BiodiversityMetrics>('/analytics/biodiversity', params);
    return response.data;
  }

  async getActivityPatterns(params: {
    speciesId?: number;
    timeRange: string;
    cameraIds?: number[];
  }): Promise<ApiResponse<ActivityPattern[]>> {
    const response = await apiClient.get<ActivityPattern[]>(`/analytics/activity?${new URLSearchParams(params as any).toString()}`);
    return response.data;
  }

  async getPopulationTrends(params: {
    speciesId?: number;
    timeRange: string;
    cameraIds?: number[];
  }): Promise<ApiResponse<PopulationTrend[]>> {
    const response = await apiClient.get<PopulationTrend[]>(`/analytics/population-trends?${new URLSearchParams(params as any).toString()}`);
    return response.data;
  }

  async getConservationAlerts(): Promise<ApiResponse<ConservationAlert[]>> {
    const response = await apiClient.get<ConservationAlert[]>('/analytics/conservation-alerts');
    return response.data;
  }

  async getSpeciesDistribution(params: {
    timeRange: string;
    cameraIds?: number[];
  }): Promise<ApiResponse<Array<{
    species_id: number;
    species_name: string;
    detection_count: number;
    camera_count: number;
    average_confidence: number;
    locations: Array<{
      camera_id: number;
      camera_name: string;
      latitude: number;
      longitude: number;
      detection_count: number;
    }>;
  }>>> {
    const response = await apiClient.get<Array<{
      species_id: number;
      species_name: string;
      detection_count: number;
      camera_count: number;
      average_confidence: number;
      locations: Array<{
        camera_id: number;
        camera_name: string;
        latitude: number;
        longitude: number;
        detection_count: number;
      }>;
    }>>(`/analytics/species-distribution?${new URLSearchParams(params as any).toString()}`);
    return response.data;
  }

  async getAnomalyDetections(params: {
    timeRange: string;
    threshold?: number;
  }): Promise<ApiResponse<Array<{
    id: number;
    type: 'temporal' | 'spatial' | 'behavioral' | 'population';
    severity: 'low' | 'medium' | 'high';
    description: string;
    detected_at: string;
    species_id?: number;
    camera_id?: number;
    confidence: number;
    metadata: any;
  }>>> {
    const response = await apiClient.get<Array<{
      id: number;
      type: 'temporal' | 'spatial' | 'behavioral' | 'population';
      severity: 'low' | 'medium' | 'high';
      description: string;
      detected_at: string;
      species_id?: number;
      camera_id?: number;
      confidence: number;
      metadata: any;
    }>>(`/analytics/anomalies?${new URLSearchParams(params as any).toString()}`);
    return response.data;
  }

  async exportAnalytics(params: {
    type: 'dashboard' | 'biodiversity' | 'activity' | 'population' | 'all';
    format: 'json' | 'csv' | 'pdf';
    timeRange: string;
    includeCharts?: boolean;
  }): Promise<ApiResponse<{download_url: string}>> {
    const response = await apiClient.post<{download_url: string}>('/analytics/export', params);
    return response.data;
  }

  async generateReport(params: {
    type: 'weekly' | 'monthly' | 'quarterly' | 'annual';
    organizationId?: number;
    cameraIds?: number[];
    includeRecommendations?: boolean;
  }): Promise<ApiResponse<{report_id: string; download_url: string}>> {
    const response = await apiClient.post<{report_id: string; download_url: string}>('/analytics/reports', params);
    return response.data;
  }
}

export default new AnalyticsService();