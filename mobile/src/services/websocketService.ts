import {store} from '@store/index';
import {updateCameraStatus, updateCameraBattery, updateCameraSignal} from '@store/slices/cameraSlice';
import {addRealtimeImage} from '@store/slices/imageSlice';
import {addRealtimeAlert} from '@store/slices/alertSlice';
import {WebSocketMessage, RealtimeDetection, SystemStatus, Alert, CameraImage} from '@types/index';
import AsyncStorage from '@react-native-async-storage/async-storage';

class WebSocketService {
  private ws: WebSocket | null = null;
  private reconnectAttempts = 0;
  private maxReconnectAttempts = 5;
  private reconnectDelay = 1000;
  private pingInterval: NodeJS.Timeout | null = null;
  private isConnecting = false;
  private listeners: Map<string, Function[]> = new Map();

  async connect(): Promise<void> {
    if (this.ws?.readyState === WebSocket.OPEN || this.isConnecting) {
      return;
    }

    try {
      this.isConnecting = true;
      const token = await AsyncStorage.getItem('auth_token');
      
      if (!token) {
        console.warn('No auth token available for WebSocket connection');
        this.isConnecting = false;
        return;
      }

      const wsUrl = __DEV__ 
        ? 'ws://10.0.2.2:5000/ws' 
        : 'wss://your-production-api.com/ws';

      this.ws = new WebSocket(`${wsUrl}?token=${token}`);

      this.ws.onopen = () => {
        console.log('WebSocket connected');
        this.isConnecting = false;
        this.reconnectAttempts = 0;
        this.startPingInterval();
        this.emit('connected');
      };

      this.ws.onmessage = (event) => {
        try {
          const message: WebSocketMessage = JSON.parse(event.data);
          this.handleMessage(message);
        } catch (error) {
          console.error('Error parsing WebSocket message:', error);
        }
      };

      this.ws.onclose = (event) => {
        console.log('WebSocket disconnected:', event.code, event.reason);
        this.isConnecting = false;
        this.stopPingInterval();
        this.emit('disconnected');
        
        if (!event.wasClean && this.reconnectAttempts < this.maxReconnectAttempts) {
          this.scheduleReconnect();
        }
      };

      this.ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        this.isConnecting = false;
        this.emit('error', error);
      };

    } catch (error) {
      console.error('Error connecting to WebSocket:', error);
      this.isConnecting = false;
      this.scheduleReconnect();
    }
  }

  disconnect(): void {
    this.stopPingInterval();
    
    if (this.ws) {
      this.ws.close(1000, 'Client disconnecting');
      this.ws = null;
    }
    
    this.reconnectAttempts = 0;
    this.emit('disconnected');
  }

  private handleMessage(message: WebSocketMessage): void {
    console.log('WebSocket message received:', message.type);

    switch (message.type) {
      case 'realtime_detection':
        this.handleRealtimeDetection(message.data as RealtimeDetection);
        break;
        
      case 'system_status':
        this.handleSystemStatus(message.data as SystemStatus);
        break;
        
      case 'new_alert':
        this.handleNewAlert(message.data as Alert);
        break;
        
      case 'new_image':
        this.handleNewImage(message.data as CameraImage);
        break;
        
      case 'camera_offline':
      case 'camera_online':
        this.handleCameraStatusChange(message.data);
        break;
        
      case 'pong':
        // Handle ping response
        break;
        
      default:
        console.log('Unknown WebSocket message type:', message.type);
    }

    this.emit(message.type, message.data);
  }

  private handleRealtimeDetection(data: RealtimeDetection): void {
    // Add the new image with detection to the store
    store.dispatch(addRealtimeImage(data.image));
    
    // Create an alert for the detection
    const alert: Alert = {
      id: Date.now(), // Temporary ID
      type: 'species_detection',
      severity: data.detection.species?.is_endangered ? 'critical' : 'info',
      title: `${data.detection.species?.name || 'Unknown Species'} Detected`,
      message: `Detected at ${data.camera.name} with ${Math.round(data.detection.confidence * 100)}% confidence`,
      data: {
        detection_id: data.detection.id,
        species_name: data.detection.species?.name,
        confidence: data.detection.confidence,
      },
      read: false,
      acknowledged: false,
      resolved: false,
      created_at: new Date().toISOString(),
      camera_id: data.camera.id,
      user_id: 0, // Will be updated by server
      detection_id: data.detection.id,
      camera: data.camera,
      detection: data.detection,
    };
    
    store.dispatch(addRealtimeAlert(alert));
  }

  private handleSystemStatus(data: SystemStatus): void {
    store.dispatch(updateCameraStatus({
      id: data.camera_id,
      status: data.status,
    }));

    if (data.battery_level !== undefined) {
      store.dispatch(updateCameraBattery({
        id: data.camera_id,
        battery_level: data.battery_level,
      }));
    }

    if (data.signal_strength !== undefined) {
      store.dispatch(updateCameraSignal({
        id: data.camera_id,
        signal_strength: data.signal_strength,
      }));
    }
  }

  private handleNewAlert(data: Alert): void {
    store.dispatch(addRealtimeAlert(data));
  }

  private handleNewImage(data: CameraImage): void {
    store.dispatch(addRealtimeImage(data));
  }

  private handleCameraStatusChange(data: {camera_id: number; status: string}): void {
    store.dispatch(updateCameraStatus({
      id: data.camera_id,
      status: data.status as any,
    }));
  }

  private startPingInterval(): void {
    this.pingInterval = setInterval(() => {
      if (this.ws?.readyState === WebSocket.OPEN) {
        this.send('ping', {timestamp: new Date().toISOString()});
      }
    }, 30000); // Ping every 30 seconds
  }

  private stopPingInterval(): void {
    if (this.pingInterval) {
      clearInterval(this.pingInterval);
      this.pingInterval = null;
    }
  }

  private scheduleReconnect(): void {
    if (this.reconnectAttempts >= this.maxReconnectAttempts) {
      console.log('Max reconnection attempts reached');
      return;
    }

    this.reconnectAttempts++;
    const delay = this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1);
    
    console.log(`Scheduling reconnection attempt ${this.reconnectAttempts} in ${delay}ms`);
    
    setTimeout(() => {
      this.connect();
    }, delay);
  }

  send(type: string, data: any): void {
    if (this.ws?.readyState === WebSocket.OPEN) {
      const message: WebSocketMessage = {
        type,
        data,
        timestamp: new Date().toISOString(),
      };
      
      this.ws.send(JSON.stringify(message));
    } else {
      console.warn('WebSocket not connected, cannot send message:', type);
    }
  }

  // Event listener system
  on(event: string, callback: Function): void {
    if (!this.listeners.has(event)) {
      this.listeners.set(event, []);
    }
    this.listeners.get(event)!.push(callback);
  }

  off(event: string, callback: Function): void {
    const listeners = this.listeners.get(event);
    if (listeners) {
      const index = listeners.indexOf(callback);
      if (index > -1) {
        listeners.splice(index, 1);
      }
    }
  }

  private emit(event: string, data?: any): void {
    const listeners = this.listeners.get(event);
    if (listeners) {
      listeners.forEach(callback => {
        try {
          callback(data);
        } catch (error) {
          console.error('Error in WebSocket event listener:', error);
        }
      });
    }
  }

  isConnected(): boolean {
    return this.ws?.readyState === WebSocket.OPEN;
  }

  getConnectionState(): 'connecting' | 'open' | 'closing' | 'closed' {
    if (!this.ws) return 'closed';
    
    switch (this.ws.readyState) {
      case WebSocket.CONNECTING: return 'connecting';
      case WebSocket.OPEN: return 'open';
      case WebSocket.CLOSING: return 'closing';
      case WebSocket.CLOSED: return 'closed';
      default: return 'closed';
    }
  }
}

export default new WebSocketService();