import {Platform, PermissionsAndroid} from 'react-native';
import PushNotification from 'react-native-push-notification';
import Geolocation from 'react-native-geolocation-service';
import {store} from '@store/index';
import {loadUser} from '@store/slices/authSlice';
import websocketService from './websocketService';
import authService from './authService';

class AppService {
  private initialized = false;

  async initializeApp(): Promise<void> {
    if (this.initialized) return;

    console.log('Initializing ESP32 Wildlife CAM Mobile App...');

    try {
      // Initialize push notifications
      await this.initializePushNotifications();

      // Request permissions
      await this.requestPermissions();

      // Check if user is authenticated
      const isAuthenticated = await authService.isAuthenticated();
      
      if (isAuthenticated) {
        // Load user data
        try {
          await store.dispatch(loadUser()).unwrap();
          
          // Connect to WebSocket
          await websocketService.connect();
        } catch (error) {
          console.error('Error loading user or connecting WebSocket:', error);
        }
      }

      this.initialized = true;
      console.log('App initialization completed');

    } catch (error) {
      console.error('Error during app initialization:', error);
      throw error;
    }
  }

  private async initializePushNotifications(): Promise<void> {
    return new Promise((resolve, reject) => {
      PushNotification.configure({
        onRegister: (token) => {
          console.log('Push notification token:', token.token);
          // Store token for sending to backend
          this.storePushToken(token.token);
          resolve();
        },

        onNotification: (notification) => {
          console.log('Push notification received:', notification);
          
          // Handle notification tap
          if (notification.userInteraction) {
            this.handleNotificationTap(notification);
          }
        },

        onAction: (notification) => {
          console.log('Push notification action:', notification.action);
        },

        onRegistrationError: (err) => {
          console.error('Push notification registration error:', err);
          reject(err);
        },

        permissions: {
          alert: true,
          badge: true,
          sound: true,
        },

        popInitialNotification: true,
        requestPermissions: Platform.OS === 'ios',
      });

      // Create notification channels for Android
      if (Platform.OS === 'android') {
        PushNotification.createChannel(
          {
            channelId: 'wildlife-detections',
            channelName: 'Wildlife Detections',
            channelDescription: 'Notifications for new wildlife detections',
            playSound: true,
            soundName: 'default',
            importance: 4,
            vibrate: true,
          },
          (created) => console.log('Wildlife detections channel created:', created)
        );

        PushNotification.createChannel(
          {
            channelId: 'system-alerts',
            channelName: 'System Alerts',
            channelDescription: 'System status and error notifications',
            playSound: true,
            soundName: 'default',
            importance: 4,
            vibrate: true,
          },
          (created) => console.log('System alerts channel created:', created)
        );

        PushNotification.createChannel(
          {
            channelId: 'conservation-alerts',
            channelName: 'Conservation Alerts',
            channelDescription: 'Important conservation and endangered species alerts',
            playSound: true,
            soundName: 'default',
            importance: 5,
            vibrate: true,
          },
          (created) => console.log('Conservation alerts channel created:', created)
        );
      }
    });
  }

  private async requestPermissions(): Promise<void> {
    if (Platform.OS === 'android') {
      try {
        const permissions = [
          PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
          PermissionsAndroid.PERMISSIONS.ACCESS_COARSE_LOCATION,
          PermissionsAndroid.PERMISSIONS.CAMERA,
          PermissionsAndroid.PERMISSIONS.WRITE_EXTERNAL_STORAGE,
          PermissionsAndroid.PERMISSIONS.READ_EXTERNAL_STORAGE,
        ];

        const granted = await PermissionsAndroid.requestMultiple(permissions);
        
        Object.entries(granted).forEach(([permission, result]) => {
          if (result === PermissionsAndroid.RESULTS.GRANTED) {
            console.log(`${permission} permission granted`);
          } else {
            console.warn(`${permission} permission denied`);
          }
        });

      } catch (error) {
        console.error('Error requesting permissions:', error);
      }
    }
  }

  private async storePushToken(token: string): Promise<void> {
    try {
      // Store locally
      await AsyncStorage.setItem('push_token', token);
      
      // Send to backend if authenticated
      const isAuthenticated = await authService.isAuthenticated();
      if (isAuthenticated) {
        // TODO: Send token to backend
        // await apiClient.post('/users/push-token', { token });
      }
    } catch (error) {
      console.error('Error storing push token:', error);
    }
  }

  private handleNotificationTap(notification: any): void {
    // Handle different notification types and navigate accordingly
    const {data} = notification;
    
    if (data?.type === 'wildlife_detection' && data?.camera_id) {
      // Navigate to camera detail screen
      // This would typically use navigation service
      console.log('Navigate to camera:', data.camera_id);
    } else if (data?.type === 'system_alert' && data?.camera_id) {
      // Navigate to system alerts
      console.log('Navigate to alerts');
    }
  }

  async getCurrentLocation(): Promise<{latitude: number; longitude: number} | null> {
    return new Promise((resolve) => {
      Geolocation.getCurrentPosition(
        (position) => {
          resolve({
            latitude: position.coords.latitude,
            longitude: position.coords.longitude,
          });
        },
        (error) => {
          console.error('Error getting location:', error);
          resolve(null);
        },
        {
          enableHighAccuracy: true,
          timeout: 15000,
          maximumAge: 10000,
        }
      );
    });
  }

  showLocalNotification(params: {
    title: string;
    message: string;
    channelId?: string;
    data?: any;
  }): void {
    PushNotification.localNotification({
      title: params.title,
      message: params.message,
      channelId: params.channelId || 'wildlife-detections',
      userInfo: params.data,
      playSound: true,
      soundName: 'default',
      vibrate: true,
    });
  }

  async cleanup(): Promise<void> {
    try {
      // Disconnect WebSocket
      websocketService.disconnect();
      
      // Clear notifications
      PushNotification.cancelAllLocalNotifications();
      
      console.log('App cleanup completed');
    } catch (error) {
      console.error('Error during app cleanup:', error);
    }
  }
}

export const initializeApp = async (): Promise<void> => {
  const appService = new AppService();
  await appService.initializeApp();
};

export default new AppService();