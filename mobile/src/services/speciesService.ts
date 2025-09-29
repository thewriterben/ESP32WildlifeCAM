import apiClient from './apiClient';
import {Species, ApiResponse, PaginatedResponse} from '@types/index';

class SpeciesService {
  async getSpecies(page: number = 1, limit: number = 50): Promise<ApiResponse<PaginatedResponse<Species>>> {
    const response = await apiClient.get<PaginatedResponse<Species>>(`/species?page=${page}&limit=${limit}`);
    return response.data;
  }

  async getSpeciesById(id: number): Promise<ApiResponse<Species>> {
    const response = await apiClient.get<Species>(`/species/${id}`);
    return response.data;
  }

  async searchSpecies(query: string): Promise<ApiResponse<Species[]>> {
    const response = await apiClient.get<Species[]>(`/species/search?q=${encodeURIComponent(query)}`);
    return response.data;
  }

  async getSpeciesByRegion(region: string): Promise<ApiResponse<Species[]>> {
    const response = await apiClient.get<Species[]>(`/species/region/${region}`);
    return response.data;
  }

  async getEndangeredSpecies(): Promise<ApiResponse<Species[]>> {
    const response = await apiClient.get<Species[]>('/species/endangered');
    return response.data;
  }

  async getSpeciesStatistics(speciesId: number, timeRange: string = '30d'): Promise<ApiResponse<{
    total_detections: number;
    unique_locations: number;
    confidence_avg: number;
    recent_activity: Array<{
      date: string;
      detection_count: number;
    }>;
    behavior_breakdown: Array<{
      behavior: string;
      count: number;
      percentage: number;
    }>;
  }>> {
    const response = await apiClient.get<{
      total_detections: number;
      unique_locations: number;
      confidence_avg: number;
      recent_activity: Array<{
        date: string;
        detection_count: number;
      }>;
      behavior_breakdown: Array<{
        behavior: string;
        count: number;
        percentage: number;
      }>;
    }>(`/species/${speciesId}/statistics?timeRange=${timeRange}`);
    return response.data;
  }
}

export default new SpeciesService();