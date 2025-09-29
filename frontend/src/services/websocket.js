/**
 * WebSocket service for ESP32 Wildlife Camera frontend
 * Handles real-time communication with the backend
 */

import { io } from 'socket.io-client';
import authService from './authService';

const WS_BASE_URL = process.env.REACT_APP_WS_URL || 'http://localhost:5000';

class WebSocketService {
  constructor() {
    this.socket = null;
    this.isConnected = false;
    this.reconnectAttempts = 0;
    this.maxReconnectAttempts = 5;
    this.reconnectDelay = 1000;
    this.listeners = new Map();
    this.connectionPromise = null;
  }

  /**
   * Connect to WebSocket server
   */
  async connect() {
    if (this.connectionPromise) {
      return this.connectionPromise;
    }

    this.connectionPromise = this._connect();
    return this.connectionPromise;
  }

  async _connect() {
    return new Promise((resolve, reject) => {
      try {
        const token = authService.getToken();
        
        this.socket = io(WS_BASE_URL, {
          auth: {
            token: token,
          },
          transports: ['websocket', 'polling'],
          timeout: 20000,
        });

        this.socket.on('connect', () => {
          console.log('WebSocket connected');
          this.isConnected = true;
          this.reconnectAttempts = 0;
          this.reconnectDelay = 1000;
          resolve();
        });

        this.socket.on('disconnect', (reason) => {
          console.log('WebSocket disconnected:', reason);
          this.isConnected = false;
          this.connectionPromise = null;
          
          if (reason === 'io server disconnect') {
            // Server initiated disconnect, don't reconnect
            return;
          }
          
          this._scheduleReconnect();
        });

        this.socket.on('connect_error', (error) => {
          console.error('WebSocket connection error:', error);
          this.isConnected = false;
          this.connectionPromise = null;
          
          if (this.reconnectAttempts === 0) {
            reject(error);
          }
          
          this._scheduleReconnect();
        });

        // Set up message handlers
        this._setupMessageHandlers();

      } catch (error) {
        console.error('Failed to create WebSocket connection:', error);
        this.connectionPromise = null;
        reject(error);
      }
    });
  }

  /**
   * Set up default message handlers
   */
  _setupMessageHandlers() {
    // Camera status updates
    this.socket.on('camera_status', (data) => {
      this._emit('camera_status', data);
    });

    // New detection events
    this.socket.on('new_detection', (data) => {
      this._emit('new_detection', data);
    });

    // System health updates
    this.socket.on('system_health', (data) => {
      this._emit('system_health', data);
    });

    // Real-time analytics
    this.socket.on('analytics_update', (data) => {
      this._emit('analytics_update', data);
    });

    // Conservation alerts
    this.socket.on('conservation_alert', (data) => {
      this._emit('conservation_alert', data);
    });

    // Camera online/offline status
    this.socket.on('camera_online', (data) => {
      this._emit('camera_online', data);
    });

    this.socket.on('camera_offline', (data) => {
      this._emit('camera_offline', data);
    });

    // Image processing updates
    this.socket.on('image_processed', (data) => {
      this._emit('image_processed', data);
    });

    // Error events
    this.socket.on('error', (error) => {
      console.error('WebSocket error:', error);
      this._emit('error', error);
    });
  }

  /**
   * Schedule reconnection attempt
   */
  _scheduleReconnect() {
    if (this.reconnectAttempts >= this.maxReconnectAttempts) {
      console.error('Max reconnection attempts reached');
      return;
    }

    this.reconnectAttempts++;
    const delay = this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1);

    console.log(`Attempting to reconnect in ${delay}ms (attempt ${this.reconnectAttempts})`);

    setTimeout(() => {
      this.connect().catch(console.error);
    }, delay);
  }

  /**
   * Disconnect from WebSocket server
   */
  disconnect() {
    if (this.socket) {
      this.socket.disconnect();
      this.socket = null;
    }
    this.isConnected = false;
    this.connectionPromise = null;
    this.listeners.clear();
  }

  /**
   * Send message to server
   */
  emit(event, data) {
    if (this.socket && this.isConnected) {
      this.socket.emit(event, data);
    } else {
      console.warn('WebSocket not connected, cannot send message:', event);
    }
  }

  /**
   * Subscribe to events
   */
  on(event, callback) {
    if (!this.listeners.has(event)) {
      this.listeners.set(event, new Set());
    }
    this.listeners.get(event).add(callback);
  }

  /**
   * Unsubscribe from events
   */
  off(event, callback) {
    if (this.listeners.has(event)) {
      this.listeners.get(event).delete(callback);
    }
  }

  /**
   * Emit to local listeners
   */
  _emit(event, data) {
    if (this.listeners.has(event)) {
      this.listeners.get(event).forEach(callback => {
        try {
          callback(data);
        } catch (error) {
          console.error(`Error in ${event} listener:`, error);
        }
      });
    }
  }

  /**
   * Join a room (e.g., for camera-specific updates)
   */
  joinRoom(room) {
    this.emit('join_room', { room });
  }

  /**
   * Leave a room
   */
  leaveRoom(room) {
    this.emit('leave_room', { room });
  }

  /**
   * Request real-time data
   */
  requestUpdate(type = 'all') {
    this.emit('request_update', { type });
  }

  /**
   * Subscribe to camera updates
   */
  subscribeCameraUpdates(cameraId) {
    this.joinRoom(`camera_${cameraId}`);
  }

  /**
   * Unsubscribe from camera updates
   */
  unsubscribeCameraUpdates(cameraId) {
    this.leaveRoom(`camera_${cameraId}`);
  }

  /**
   * Get connection status
   */
  getConnectionStatus() {
    return {
      connected: this.isConnected,
      reconnectAttempts: this.reconnectAttempts,
      maxReconnectAttempts: this.maxReconnectAttempts,
    };
  }
}

// Create singleton instance
const wsService = new WebSocketService();

export default wsService;