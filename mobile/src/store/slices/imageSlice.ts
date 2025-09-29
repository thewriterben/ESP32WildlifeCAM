import {createSlice, createAsyncThunk, PayloadAction} from '@reduxjs/toolkit';
import {CameraImage, WildlifeDetection, PaginatedResponse} from '@types/index';
import imageService from '@services/imageService';

interface ImageState {
  images: CameraImage[];
  selectedImage: CameraImage | null;
  isLoading: boolean;
  error: string | null;
  pagination: {
    page: number;
    limit: number;
    total: number;
    total_pages: number;
  } | null;
}

const initialState: ImageState = {
  images: [],
  selectedImage: null,
  isLoading: false,
  error: null,
  pagination: null,
};

export const fetchImages = createAsyncThunk(
  'images/fetchImages',
  async (params: {page?: number; limit?: number; cameraId?: number}, {rejectWithValue}) => {
    try {
      const response = await imageService.getImages(params);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to fetch images');
    }
  },
);

export const fetchImageById = createAsyncThunk(
  'images/fetchImageById',
  async (imageId: number, {rejectWithValue}) => {
    try {
      const response = await imageService.getImageById(imageId);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to fetch image');
    }
  },
);

export const verifyDetection = createAsyncThunk(
  'images/verifyDetection',
  async ({detectionId, verified, notes}: {detectionId: number; verified: boolean; notes?: string}, {rejectWithValue}) => {
    try {
      const response = await imageService.verifyDetection(detectionId, verified, notes);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to verify detection');
    }
  },
);

const imageSlice = createSlice({
  name: 'images',
  initialState,
  reducers: {
    clearError: state => {
      state.error = null;
    },
    setSelectedImage: (state, action: PayloadAction<CameraImage | null>) => {
      state.selectedImage = action.payload;
    },
    clearImages: state => {
      state.images = [];
      state.selectedImage = null;
      state.pagination = null;
    },
    addRealtimeImage: (state, action: PayloadAction<CameraImage>) => {
      state.images.unshift(action.payload);
      // Keep only the latest 100 images in memory
      if (state.images.length > 100) {
        state.images = state.images.slice(0, 100);
      }
    },
  },
  extraReducers: builder => {
    builder
      .addCase(fetchImages.pending, state => {
        state.isLoading = true;
        state.error = null;
      })
      .addCase(fetchImages.fulfilled, (state, action) => {
        state.isLoading = false;
        const response = action.payload as PaginatedResponse<CameraImage>;
        
        if (response.pagination.page === 1) {
          state.images = response.data;
        } else {
          state.images = [...state.images, ...response.data];
        }
        
        state.pagination = response.pagination;
        state.error = null;
      })
      .addCase(fetchImages.rejected, (state, action) => {
        state.isLoading = false;
        state.error = action.payload as string;
      })
      .addCase(fetchImageById.fulfilled, (state, action) => {
        const existingIndex = state.images.findIndex(img => img.id === action.payload.id);
        if (existingIndex >= 0) {
          state.images[existingIndex] = action.payload;
        } else {
          state.images.unshift(action.payload);
        }
        state.selectedImage = action.payload;
      })
      .addCase(verifyDetection.fulfilled, (state, action) => {
        const detection = action.payload as WildlifeDetection;
        const image = state.images.find(img => 
          img.detections.some(d => d.id === detection.id)
        );
        
        if (image) {
          const detectionIndex = image.detections.findIndex(d => d.id === detection.id);
          if (detectionIndex >= 0) {
            image.detections[detectionIndex] = detection;
          }
        }
        
        if (state.selectedImage?.detections.some(d => d.id === detection.id)) {
          const detectionIndex = state.selectedImage.detections.findIndex(d => d.id === detection.id);
          if (detectionIndex >= 0) {
            state.selectedImage.detections[detectionIndex] = detection;
          }
        }
      });
  },
});

export const {clearError, setSelectedImage, clearImages, addRealtimeImage} = imageSlice.actions;
export default imageSlice.reducer;