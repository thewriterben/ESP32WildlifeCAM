import apiClient from './apiClient';
import {Camera, CameraConfiguration, ApiResponse, PaginatedResponse} from '@types/index';

class CameraService {
  async getCameras(page: number = 1, limit: number = 20): Promise<ApiResponse<PaginatedResponse<Camera>>> {
    const response = await apiClient.get<PaginatedResponse<Camera>>(`/cameras?page=${page}&limit=${limit}`);
    return response.data;
  }

  async getCameraById(id: number): Promise<ApiResponse<Camera>> {
    const response = await apiClient.get<Camera>(`/cameras/${id}`);
    return response.data;
  }

  async createCamera(cameraData: Partial<Camera>): Promise<ApiResponse<Camera>> {
    const response = await apiClient.post<Camera>('/cameras', cameraData);
    return response.data;
  }

  async updateCamera(id: number, cameraData: Partial<Camera>): Promise<ApiResponse<Camera>> {
    const response = await apiClient.put<Camera>(`/cameras/${id}`, cameraData);
    return response.data;
  }

  async deleteCamera(id: number): Promise<ApiResponse<null>> {
    const response = await apiClient.delete<null>(`/cameras/${id}`);
    return response.data;
  }

  async updateCameraConfiguration(id: number, config: Partial<CameraConfiguration>): Promise<ApiResponse<Camera>> {
    const response = await apiClient.put<Camera>(`/cameras/${id}/configuration`, {configuration: config});
    return response.data;
  }

  async triggerCapture(id: number, mode: 'single' | 'burst' = 'single'): Promise<ApiResponse<{message: string}>> {
    const response = await apiClient.post<{message: string}>(`/cameras/${id}/capture`, {mode});
    return response.data;
  }

  async getCameraStatus(id: number): Promise<ApiResponse<{
    status: Camera['status'];
    battery_level?: number;
    signal_strength?: number;
    last_heartbeat: string;
    storage_available: number;
    temperature?: number;
  }>> {
    const response = await apiClient.get<{
      status: Camera['status'];
      battery_level?: number;
      signal_strength?: number;
      last_heartbeat: string;
      storage_available: number;
      temperature?: number;
    }>(`/cameras/${id}/status`);
    return response.data;
  }

  async updateFirmware(id: number, firmwareFile: string): Promise<ApiResponse<{message: string; progress?: number}>> {
    const formData = new FormData();
    formData.append('firmware', {
      uri: firmwareFile,
      type: 'application/octet-stream',
      name: 'firmware.bin',
    } as any);

    const response = await apiClient.upload<{message: string; progress?: number}>(`/cameras/${id}/firmware`, formData);
    return response.data;
  }

  async restartCamera(id: number): Promise<ApiResponse<{message: string}>> {
    const response = await apiClient.post<{message: string}>(`/cameras/${id}/restart`);
    return response.data;
  }

  async getCameraLogs(id: number, level: 'info' | 'warning' | 'error' = 'info', limit: number = 100): Promise<ApiResponse<{
    logs: Array<{
      timestamp: string;
      level: string;
      message: string;
      component?: string;
    }>;
  }>> {
    const response = await apiClient.get<{
      logs: Array<{
        timestamp: string;
        level: string;
        message: string;
        component?: string;
      }>;
    }>(`/cameras/${id}/logs?level=${level}&limit=${limit}`);
    return response.data;
  }

  async testConnection(id: number): Promise<ApiResponse<{
    connected: boolean;
    latency?: number;
    signal_strength?: number;
    last_response: string;
  }>> {
    const response = await apiClient.post<{
      connected: boolean;
      latency?: number;
      signal_strength?: number;
      last_response: string;
    }>(`/cameras/${id}/test-connection`);
    return response.data;
  }

  async syncTime(id: number): Promise<ApiResponse<{message: string}>> {
    const response = await apiClient.post<{message: string}>(`/cameras/${id}/sync-time`);
    return response.data;
  }

  async getCamerasByLocation(latitude: number, longitude: number, radius: number = 10): Promise<ApiResponse<Camera[]>> {
    const response = await apiClient.get<Camera[]>(`/cameras/nearby?lat=${latitude}&lng=${longitude}&radius=${radius}`);
    return response.data;
  }

  async getCamerasForOrganization(organizationId: number): Promise<ApiResponse<Camera[]>> {
    const response = await apiClient.get<Camera[]>(`/organizations/${organizationId}/cameras`);
    return response.data;
  }
}

export default new CameraService();