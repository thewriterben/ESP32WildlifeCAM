import apiClient from './apiClient';
import {CameraImage, WildlifeDetection, ApiResponse, PaginatedResponse} from '@types/index';

class ImageService {
  async getImages(params: {
    page?: number;
    limit?: number;
    cameraId?: number;
    startDate?: string;
    endDate?: string;
    hasDetections?: boolean;
    verified?: boolean;
  } = {}): Promise<ApiResponse<PaginatedResponse<CameraImage>>> {
    const queryParams = new URLSearchParams();
    
    Object.entries(params).forEach(([key, value]) => {
      if (value !== undefined) {
        queryParams.append(key, value.toString());
      }
    });

    const response = await apiClient.get<PaginatedResponse<CameraImage>>(`/images?${queryParams.toString()}`);
    return response.data;
  }

  async getImageById(id: number): Promise<ApiResponse<CameraImage>> {
    const response = await apiClient.get<CameraImage>(`/images/${id}`);
    return response.data;
  }

  async getImagesByCamera(cameraId: number, page: number = 1, limit: number = 20): Promise<ApiResponse<PaginatedResponse<CameraImage>>> {
    const response = await apiClient.get<PaginatedResponse<CameraImage>>(`/cameras/${cameraId}/images?page=${page}&limit=${limit}`);
    return response.data;
  }

  async verifyDetection(detectionId: number, verified: boolean, notes?: string): Promise<ApiResponse<WildlifeDetection>> {
    const response = await apiClient.post<WildlifeDetection>(`/detections/${detectionId}/verify`, {
      verified,
      verification_notes: notes,
    });
    return response.data;
  }

  async deleteImage(id: number): Promise<ApiResponse<null>> {
    const response = await apiClient.delete<null>(`/images/${id}`);
    return response.data;
  }

  async getImageUrl(filename: string, size: 'thumbnail' | 'medium' | 'full' = 'medium'): string {
    return `${apiClient.getBaseURL()}/images/file/${filename}?size=${size}`;
  }

  async downloadImage(filename: string, size: 'thumbnail' | 'medium' | 'full' = 'full'): Promise<string> {
    const response = await apiClient.get(`/images/file/${filename}?size=${size}`, {
      responseType: 'blob',
    });
    
    // Convert blob to local file URL for React Native
    return URL.createObjectURL(response.data);
  }

  async getRecentDetections(limit: number = 50): Promise<ApiResponse<Array<{
    detection: WildlifeDetection;
    image: CameraImage;
  }>>> {
    const response = await apiClient.get<Array<{
      detection: WildlifeDetection;
      image: CameraImage;
    }>>(`/detections/recent?limit=${limit}`);
    return response.data;
  }

  async getDetectionsBySpecies(speciesId: number, page: number = 1, limit: number = 20): Promise<ApiResponse<PaginatedResponse<{
    detection: WildlifeDetection;
    image: CameraImage;
  }>>> {
    const response = await apiClient.get<PaginatedResponse<{
      detection: WildlifeDetection;
      image: CameraImage;
    }>>(`/species/${speciesId}/detections?page=${page}&limit=${limit}`);
    return response.data;
  }

  async exportImages(params: {
    cameraId?: number;
    startDate?: string;
    endDate?: string;
    format: 'json' | 'csv';
    includeImages?: boolean;
  }): Promise<ApiResponse<{download_url: string}>> {
    const response = await apiClient.post<{download_url: string}>('/images/export', params);
    return response.data;
  }
}

export default new ImageService();