/**
 * Authentication service for ESP32 Wildlife Camera frontend
 */

import axios from 'axios';

const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:5000/api';

class AuthService {
  constructor() {
    this.token = localStorage.getItem('wildlife_token');
    this.user = JSON.parse(localStorage.getItem('wildlife_user') || 'null');
    
    // Setup axios interceptor for automatic token inclusion
    axios.interceptors.request.use(
      (config) => {
        if (this.token) {
          config.headers.Authorization = `Bearer ${this.token}`;
        }
        return config;
      },
      (error) => {
        return Promise.reject(error);
      }
    );
    
    // Setup response interceptor for automatic logout on 401
    axios.interceptors.response.use(
      (response) => response,
      (error) => {
        if (error.response?.status === 401) {
          this.logout();
          window.location.href = '/login';
        }
        return Promise.reject(error);
      }
    );
  }
  
  /**
   * Login user with username and password
   */
  async login(username, password) {
    try {
      const response = await axios.post(`${API_BASE_URL}/auth/login`, {
        username,
        password
      });
      
      const { access_token, user } = response.data;
      
      // Store token and user info
      this.token = access_token;
      this.user = user;
      
      localStorage.setItem('wildlife_token', access_token);
      localStorage.setItem('wildlife_user', JSON.stringify(user));
      
      return { success: true, user };
    } catch (error) {
      console.error('Login failed:', error);
      return {
        success: false,
        error: error.response?.data?.error || 'Login failed'
      };
    }
  }
  
  /**
   * Register new user
   */
  async register(userData) {
    try {
      const response = await axios.post(`${API_BASE_URL}/auth/register`, userData);
      
      const { access_token, user } = response.data;
      
      // Auto-login after registration
      this.token = access_token;
      this.user = user;
      
      localStorage.setItem('wildlife_token', access_token);
      localStorage.setItem('wildlife_user', JSON.stringify(user));
      
      return { success: true, user };
    } catch (error) {
      console.error('Registration failed:', error);
      return {
        success: false,
        error: error.response?.data?.error || 'Registration failed'
      };
    }
  }
  
  /**
   * Logout user
   */
  logout() {
    this.token = null;
    this.user = null;
    
    localStorage.removeItem('wildlife_token');
    localStorage.removeItem('wildlife_user');
  }
  
  /**
   * Check if user is authenticated
   */
  isAuthenticated() {
    return !!this.token;
  }
  
  /**
   * Get current user
   */
  getCurrentUser() {
    return this.user;
  }
  
  /**
   * Get current token
   */
  getToken() {
    return this.token;
  }
  
  /**
   * Refresh token
   */
  async refreshToken() {
    try {
      const response = await axios.post(`${API_BASE_URL}/auth/refresh`);
      
      const { access_token } = response.data;
      
      this.token = access_token;
      localStorage.setItem('wildlife_token', access_token);
      
      return { success: true };
    } catch (error) {
      console.error('Token refresh failed:', error);
      this.logout();
      return { success: false };
    }
  }
}

// Create singleton instance
const authService = new AuthService();

export default authService;