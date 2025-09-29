import AsyncStorage from '@react-native-async-storage/async-storage';
import apiClient from './apiClient';
import {LoginCredentials, RegisterData, User, Organization, ApiResponse} from '@types/index';

class AuthService {
  async login(credentials: LoginCredentials): Promise<ApiResponse<{user: User; organization?: Organization; token: string; refresh_token: string}>> {
    const response = await apiClient.post<{user: User; organization?: Organization; token: string; refresh_token: string}>('/auth/login', credentials);
    
    if (response.data.data.token) {
      await AsyncStorage.setItem('auth_token', response.data.data.token);
      await AsyncStorage.setItem('refresh_token', response.data.data.refresh_token);
      await AsyncStorage.setItem('user', JSON.stringify(response.data.data.user));
      
      if (response.data.data.organization) {
        await AsyncStorage.setItem('organization', JSON.stringify(response.data.data.organization));
      }
    }
    
    return response.data;
  }

  async register(userData: RegisterData): Promise<ApiResponse<{user: User; organization?: Organization; token: string; refresh_token: string}>> {
    const response = await apiClient.post<{user: User; organization?: Organization; token: string; refresh_token: string}>('/auth/register', userData);
    
    if (response.data.data.token) {
      await AsyncStorage.setItem('auth_token', response.data.data.token);
      await AsyncStorage.setItem('refresh_token', response.data.data.refresh_token);
      await AsyncStorage.setItem('user', JSON.stringify(response.data.data.user));
      
      if (response.data.data.organization) {
        await AsyncStorage.setItem('organization', JSON.stringify(response.data.data.organization));
      }
    }
    
    return response.data;
  }

  async logout(): Promise<void> {
    try {
      await apiClient.post('/auth/logout');
    } catch (error) {
      // Continue with logout even if API call fails
      console.warn('Logout API call failed:', error);
    } finally {
      await AsyncStorage.multiRemove([
        'auth_token',
        'refresh_token',
        'user',
        'organization',
      ]);
    }
  }

  async refreshToken(): Promise<ApiResponse<{user: User; organization?: Organization; token: string}>> {
    const refreshToken = await AsyncStorage.getItem('refresh_token');
    
    if (!refreshToken) {
      throw new Error('No refresh token available');
    }

    const response = await apiClient.post<{user: User; organization?: Organization; token: string}>('/auth/refresh', {
      refresh_token: refreshToken,
    });
    
    if (response.data.data.token) {
      await AsyncStorage.setItem('auth_token', response.data.data.token);
      await AsyncStorage.setItem('user', JSON.stringify(response.data.data.user));
      
      if (response.data.data.organization) {
        await AsyncStorage.setItem('organization', JSON.stringify(response.data.data.organization));
      }
    }
    
    return response.data;
  }

  async getCurrentUser(): Promise<ApiResponse<{user: User; organization?: Organization}>> {
    const response = await apiClient.get<{user: User; organization?: Organization}>('/auth/me');
    
    await AsyncStorage.setItem('user', JSON.stringify(response.data.data.user));
    
    if (response.data.data.organization) {
      await AsyncStorage.setItem('organization', JSON.stringify(response.data.data.organization));
    }
    
    return response.data;
  }

  async updateProfile(userData: Partial<User>): Promise<ApiResponse<User>> {
    const response = await apiClient.put<User>('/auth/profile', userData);
    
    await AsyncStorage.setItem('user', JSON.stringify(response.data.data));
    
    return response.data;
  }

  async changePassword(currentPassword: string, newPassword: string): Promise<ApiResponse<null>> {
    return await apiClient.post<null>('/auth/change-password', {
      current_password: currentPassword,
      new_password: newPassword,
    }).then(response => response.data);
  }

  async forgotPassword(email: string): Promise<ApiResponse<null>> {
    return await apiClient.post<null>('/auth/forgot-password', {email}).then(response => response.data);
  }

  async resetPassword(token: string, newPassword: string): Promise<ApiResponse<null>> {
    return await apiClient.post<null>('/auth/reset-password', {
      token,
      new_password: newPassword,
    }).then(response => response.data);
  }

  async getStoredToken(): Promise<string | null> {
    return await AsyncStorage.getItem('auth_token');
  }

  async getStoredUser(): Promise<User | null> {
    const userStr = await AsyncStorage.getItem('user');
    return userStr ? JSON.parse(userStr) : null;
  }

  async getStoredOrganization(): Promise<Organization | null> {
    const orgStr = await AsyncStorage.getItem('organization');
    return orgStr ? JSON.parse(orgStr) : null;
  }

  async isAuthenticated(): Promise<boolean> {
    const token = await AsyncStorage.getItem('auth_token');
    return !!token;
  }
}

export default new AuthService();