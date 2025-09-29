import {configureStore} from '@reduxjs/toolkit';
import {persistStore, persistReducer} from 'redux-persist';
import AsyncStorage from '@react-native-async-storage/async-storage';
import {combineReducers} from '@reduxjs/toolkit';

import authSlice from './slices/authSlice';
import cameraSlice from './slices/cameraSlice';
import speciesSlice from './slices/speciesSlice';
import imageSlice from './slices/imageSlice';
import analyticsSlice from './slices/analyticsSlice';
import alertSlice from './slices/alertSlice';
import settingsSlice from './slices/settingsSlice';

const persistConfig = {
  key: 'root',
  storage: AsyncStorage,
  whitelist: ['auth', 'settings', 'cameras', 'species'], // What to persist
  blacklist: ['images', 'analytics', 'alerts'], // What not to persist (real-time data)
};

const rootReducer = combineReducers({
  auth: authSlice,
  cameras: cameraSlice,
  species: speciesSlice,
  images: imageSlice,
  analytics: analyticsSlice,
  alerts: alertSlice,
  settings: settingsSlice,
});

const persistedReducer = persistReducer(persistConfig, rootReducer);

export const store = configureStore({
  reducer: persistedReducer,
  middleware: getDefaultMiddleware =>
    getDefaultMiddleware({
      serializableCheck: {
        ignoredActions: ['persist/PERSIST', 'persist/REHYDRATE'],
      },
    }),
  devTools: __DEV__,
});

export const persistor = persistStore(store);

export type RootState = ReturnType<typeof store.getState>;
export type AppDispatch = typeof store.dispatch;