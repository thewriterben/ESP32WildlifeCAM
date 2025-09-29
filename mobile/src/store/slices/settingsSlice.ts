import {createSlice, PayloadAction} from '@reduxjs/toolkit';
import {UserSettings} from '@types/index';

const initialState: UserSettings = {
  notifications_enabled: true,
  push_notifications: true,
  email_notifications: true,
  offline_sync: true,
  image_quality: 'high',
  auto_sync: true,
  dark_mode: false,
  language: 'en',
};

const settingsSlice = createSlice({
  name: 'settings',
  initialState,
  reducers: {
    updateSetting: (state, action: PayloadAction<Partial<UserSettings>>) => {
      return {...state, ...action.payload};
    },
    resetSettings: () => initialState,
    toggleNotifications: state => {
      state.notifications_enabled = !state.notifications_enabled;
    },
    togglePushNotifications: state => {
      state.push_notifications = !state.push_notifications;
    },
    toggleEmailNotifications: state => {
      state.email_notifications = !state.email_notifications;
    },
    toggleOfflineSync: state => {
      state.offline_sync = !state.offline_sync;
    },
    toggleAutoSync: state => {
      state.auto_sync = !state.auto_sync;
    },
    toggleDarkMode: state => {
      state.dark_mode = !state.dark_mode;
    },
    setImageQuality: (state, action: PayloadAction<'high' | 'medium' | 'low'>) => {
      state.image_quality = action.payload;
    },
    setLanguage: (state, action: PayloadAction<string>) => {
      state.language = action.payload;
    },
  },
});

export const {
  updateSetting,
  resetSettings,
  toggleNotifications,
  togglePushNotifications,
  toggleEmailNotifications,
  toggleOfflineSync,
  toggleAutoSync,
  toggleDarkMode,
  setImageQuality,
  setLanguage,
} = settingsSlice.actions;

export default settingsSlice.reducer;