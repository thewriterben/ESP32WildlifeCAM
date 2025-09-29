import apiClient from './apiClient';
import {Alert, ApiResponse, PaginatedResponse} from '@types/index';

class AlertService {
  async getAlerts(params: {
    page?: number;
    limit?: number;
    read?: boolean;
    severity?: 'critical' | 'warning' | 'info';
    type?: string;
  } = {}): Promise<ApiResponse<Alert[]>> {
    const queryParams = new URLSearchParams();
    
    Object.entries(params).forEach(([key, value]) => {
      if (value !== undefined) {
        queryParams.append(key, value.toString());
      }
    });

    const response = await apiClient.get<Alert[]>(`/alerts?${queryParams.toString()}`);
    return response.data;
  }

  async getAlertById(id: number): Promise<ApiResponse<Alert>> {
    const response = await apiClient.get<Alert>(`/alerts/${id}`);
    return response.data;
  }

  async markAsRead(alertId: number): Promise<ApiResponse<Alert>> {
    const response = await apiClient.patch<Alert>(`/alerts/${alertId}/read`);
    return response.data;
  }

  async markAllAsRead(): Promise<ApiResponse<{updated_count: number}>> {
    const response = await apiClient.patch<{updated_count: number}>('/alerts/read-all');
    return response.data;
  }

  async acknowledgeAlert(alertId: number): Promise<ApiResponse<Alert>> {
    const response = await apiClient.patch<Alert>(`/alerts/${alertId}/acknowledge`);
    return response.data;
  }

  async resolveAlert(alertId: number, resolution_notes?: string): Promise<ApiResponse<Alert>> {
    const response = await apiClient.patch<Alert>(`/alerts/${alertId}/resolve`, {
      resolution_notes,
    });
    return response.data;
  }

  async deleteAlert(alertId: number): Promise<ApiResponse<null>> {
    const response = await apiClient.delete<null>(`/alerts/${alertId}`);
    return response.data;
  }

  async getUnreadCount(): Promise<ApiResponse<{unread_count: number}>> {
    const response = await apiClient.get<{unread_count: number}>('/alerts/unread-count');
    return response.data;
  }

  async createAlert(alertData: {
    type: string;
    severity: 'critical' | 'warning' | 'info';
    title: string;
    message: string;
    camera_id?: number;
    detection_id?: number;
    data?: any;
  }): Promise<ApiResponse<Alert>> {
    const response = await apiClient.post<Alert>('/alerts', alertData);
    return response.data;
  }

  async getAlertSettings(): Promise<ApiResponse<{
    email_notifications: boolean;
    push_notifications: boolean;
    alert_types: Array<{
      type: string;
      enabled: boolean;
      severity_threshold: 'critical' | 'warning' | 'info';
    }>;
  }>> {
    const response = await apiClient.get<{
      email_notifications: boolean;
      push_notifications: boolean;
      alert_types: Array<{
        type: string;
        enabled: boolean;
        severity_threshold: 'critical' | 'warning' | 'info';
      }>;
    }>('/alerts/settings');
    return response.data;
  }

  async updateAlertSettings(settings: {
    email_notifications?: boolean;
    push_notifications?: boolean;
    alert_types?: Array<{
      type: string;
      enabled: boolean;
      severity_threshold: 'critical' | 'warning' | 'info';
    }>;
  }): Promise<ApiResponse<{message: string}>> {
    const response = await apiClient.put<{message: string}>('/alerts/settings', settings);
    return response.data;
  }
}

export default new AlertService();