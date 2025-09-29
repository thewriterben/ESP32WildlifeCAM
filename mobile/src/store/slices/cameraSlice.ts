import {createSlice, createAsyncThunk, PayloadAction} from '@reduxjs/toolkit';
import {Camera, CameraConfiguration} from '@types/index';
import cameraService from '@services/cameraService';

interface CameraState {
  cameras: Camera[];
  selectedCamera: Camera | null;
  isLoading: boolean;
  error: string | null;
  lastSync: string | null;
}

const initialState: CameraState = {
  cameras: [],
  selectedCamera: null,
  isLoading: false,
  error: null,
  lastSync: null,
};

// Async thunks
export const fetchCameras = createAsyncThunk(
  'cameras/fetchCameras',
  async (_, {rejectWithValue}) => {
    try {
      const response = await cameraService.getCameras();
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to fetch cameras');
    }
  },
);

export const fetchCameraById = createAsyncThunk(
  'cameras/fetchCameraById',
  async (cameraId: number, {rejectWithValue}) => {
    try {
      const response = await cameraService.getCameraById(cameraId);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to fetch camera');
    }
  },
);

export const createCamera = createAsyncThunk(
  'cameras/createCamera',
  async (cameraData: Partial<Camera>, {rejectWithValue}) => {
    try {
      const response = await cameraService.createCamera(cameraData);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to create camera');
    }
  },
);

export const updateCamera = createAsyncThunk(
  'cameras/updateCamera',
  async ({id, data}: {id: number; data: Partial<Camera>}, {rejectWithValue}) => {
    try {
      const response = await cameraService.updateCamera(id, data);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to update camera');
    }
  },
);

export const deleteCamera = createAsyncThunk(
  'cameras/deleteCamera',
  async (cameraId: number, {rejectWithValue}) => {
    try {
      await cameraService.deleteCamera(cameraId);
      return cameraId;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to delete camera');
    }
  },
);

export const updateCameraConfiguration = createAsyncThunk(
  'cameras/updateCameraConfiguration',
  async ({id, config}: {id: number; config: Partial<CameraConfiguration>}, {rejectWithValue}) => {
    try {
      const response = await cameraService.updateCameraConfiguration(id, config);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to update camera configuration');
    }
  },
);

export const triggerCameraCapture = createAsyncThunk(
  'cameras/triggerCameraCapture',
  async (cameraId: number, {rejectWithValue}) => {
    try {
      const response = await cameraService.triggerCapture(cameraId);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to trigger camera capture');
    }
  },
);

const cameraSlice = createSlice({
  name: 'cameras',
  initialState,
  reducers: {
    clearError: state => {
      state.error = null;
    },
    setSelectedCamera: (state, action: PayloadAction<Camera | null>) => {
      state.selectedCamera = action.payload;
    },
    updateCameraStatus: (state, action: PayloadAction<{id: number; status: Camera['status']}>) => {
      const camera = state.cameras.find(c => c.id === action.payload.id);
      if (camera) {
        camera.status = action.payload.status;
        camera.last_seen = new Date().toISOString();
      }
      if (state.selectedCamera?.id === action.payload.id) {
        state.selectedCamera.status = action.payload.status;
        state.selectedCamera.last_seen = new Date().toISOString();
      }
    },
    updateCameraBattery: (state, action: PayloadAction<{id: number; battery_level: number}>) => {
      const camera = state.cameras.find(c => c.id === action.payload.id);
      if (camera) {
        camera.battery_level = action.payload.battery_level;
      }
      if (state.selectedCamera?.id === action.payload.id) {
        state.selectedCamera.battery_level = action.payload.battery_level;
      }
    },
    updateCameraSignal: (state, action: PayloadAction<{id: number; signal_strength: number}>) => {
      const camera = state.cameras.find(c => c.id === action.payload.id);
      if (camera) {
        camera.signal_strength = action.payload.signal_strength;
      }
      if (state.selectedCamera?.id === action.payload.id) {
        state.selectedCamera.signal_strength = action.payload.signal_strength;
      }
    },
    clearCameras: state => {
      state.cameras = [];
      state.selectedCamera = null;
      state.lastSync = null;
    },
  },
  extraReducers: builder => {
    builder
      // Fetch cameras
      .addCase(fetchCameras.pending, state => {
        state.isLoading = true;
        state.error = null;
      })
      .addCase(fetchCameras.fulfilled, (state, action) => {
        state.isLoading = false;
        state.cameras = action.payload;
        state.lastSync = new Date().toISOString();
        state.error = null;
      })
      .addCase(fetchCameras.rejected, (state, action) => {
        state.isLoading = false;
        state.error = action.payload as string;
      })
      // Fetch camera by ID
      .addCase(fetchCameraById.fulfilled, (state, action) => {
        const existingIndex = state.cameras.findIndex(c => c.id === action.payload.id);
        if (existingIndex >= 0) {
          state.cameras[existingIndex] = action.payload;
        } else {
          state.cameras.push(action.payload);
        }
        state.selectedCamera = action.payload;
      })
      // Create camera
      .addCase(createCamera.fulfilled, (state, action) => {
        state.cameras.push(action.payload);
      })
      // Update camera
      .addCase(updateCamera.fulfilled, (state, action) => {
        const index = state.cameras.findIndex(c => c.id === action.payload.id);
        if (index >= 0) {
          state.cameras[index] = action.payload;
        }
        if (state.selectedCamera?.id === action.payload.id) {
          state.selectedCamera = action.payload;
        }
      })
      // Delete camera
      .addCase(deleteCamera.fulfilled, (state, action) => {
        state.cameras = state.cameras.filter(c => c.id !== action.payload);
        if (state.selectedCamera?.id === action.payload) {
          state.selectedCamera = null;
        }
      })
      // Update camera configuration
      .addCase(updateCameraConfiguration.fulfilled, (state, action) => {
        const index = state.cameras.findIndex(c => c.id === action.payload.id);
        if (index >= 0) {
          state.cameras[index] = action.payload;
        }
        if (state.selectedCamera?.id === action.payload.id) {
          state.selectedCamera = action.payload;
        }
      });
  },
});

export const {
  clearError,
  setSelectedCamera,
  updateCameraStatus,
  updateCameraBattery,
  updateCameraSignal,
  clearCameras,
} = cameraSlice.actions;

export default cameraSlice.reducer;