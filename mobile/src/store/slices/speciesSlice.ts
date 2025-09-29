import {createSlice, createAsyncThunk, PayloadAction} from '@reduxjs/toolkit';
import {Species} from '@types/index';
import speciesService from '@services/speciesService';

interface SpeciesState {
  species: Species[];
  selectedSpecies: Species | null;
  isLoading: boolean;
  error: string | null;
  lastSync: string | null;
}

const initialState: SpeciesState = {
  species: [],
  selectedSpecies: null,
  isLoading: false,
  error: null,
  lastSync: null,
};

export const fetchSpecies = createAsyncThunk(
  'species/fetchSpecies',
  async (_, {rejectWithValue}) => {
    try {
      const response = await speciesService.getSpecies();
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to fetch species');
    }
  },
);

export const fetchSpeciesById = createAsyncThunk(
  'species/fetchSpeciesById',
  async (speciesId: number, {rejectWithValue}) => {
    try {
      const response = await speciesService.getSpeciesById(speciesId);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to fetch species');
    }
  },
);

const speciesSlice = createSlice({
  name: 'species',
  initialState,
  reducers: {
    clearError: state => {
      state.error = null;
    },
    setSelectedSpecies: (state, action: PayloadAction<Species | null>) => {
      state.selectedSpecies = action.payload;
    },
    clearSpecies: state => {
      state.species = [];
      state.selectedSpecies = null;
      state.lastSync = null;
    },
  },
  extraReducers: builder => {
    builder
      .addCase(fetchSpecies.pending, state => {
        state.isLoading = true;
        state.error = null;
      })
      .addCase(fetchSpecies.fulfilled, (state, action) => {
        state.isLoading = false;
        state.species = action.payload;
        state.lastSync = new Date().toISOString();
        state.error = null;
      })
      .addCase(fetchSpecies.rejected, (state, action) => {
        state.isLoading = false;
        state.error = action.payload as string;
      })
      .addCase(fetchSpeciesById.fulfilled, (state, action) => {
        const existingIndex = state.species.findIndex(s => s.id === action.payload.id);
        if (existingIndex >= 0) {
          state.species[existingIndex] = action.payload;
        } else {
          state.species.push(action.payload);
        }
        state.selectedSpecies = action.payload;
      });
  },
});

export const {clearError, setSelectedSpecies, clearSpecies} = speciesSlice.actions;
export default speciesSlice.reducer;