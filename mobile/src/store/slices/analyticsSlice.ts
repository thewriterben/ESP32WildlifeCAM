import {createSlice, createAsyncThunk} from '@reduxjs/toolkit';
import {DashboardData, BiodiversityMetrics, ActivityPattern, PopulationTrend, ConservationAlert} from '@types/index';
import analyticsService from '@services/analyticsService';

interface AnalyticsState {
  dashboardData: DashboardData | null;
  biodiversityMetrics: BiodiversityMetrics | null;
  activityPatterns: ActivityPattern[];
  populationTrends: PopulationTrend[];
  conservationAlerts: ConservationAlert[];
  isLoading: boolean;
  error: string | null;
  lastUpdate: string | null;
}

const initialState: AnalyticsState = {
  dashboardData: null,
  biodiversityMetrics: null,
  activityPatterns: [],
  populationTrends: [],
  conservationAlerts: [],
  isLoading: false,
  error: null,
  lastUpdate: null,
};

export const fetchDashboardData = createAsyncThunk(
  'analytics/fetchDashboardData',
  async (timeRange: string = '7d', {rejectWithValue}) => {
    try {
      const response = await analyticsService.getDashboardData(timeRange);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to fetch dashboard data');
    }
  },
);

export const fetchBiodiversityMetrics = createAsyncThunk(
  'analytics/fetchBiodiversityMetrics',
  async (params: {startDate: string; endDate: string}, {rejectWithValue}) => {
    try {
      const response = await analyticsService.getBiodiversityMetrics(params);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to fetch biodiversity metrics');
    }
  },
);

export const fetchActivityPatterns = createAsyncThunk(
  'analytics/fetchActivityPatterns',
  async (params: {speciesId?: number; timeRange: string}, {rejectWithValue}) => {
    try {
      const response = await analyticsService.getActivityPatterns(params);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to fetch activity patterns');
    }
  },
);

export const fetchPopulationTrends = createAsyncThunk(
  'analytics/fetchPopulationTrends',
  async (params: {speciesId?: number; timeRange: string}, {rejectWithValue}) => {
    try {
      const response = await analyticsService.getPopulationTrends(params);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to fetch population trends');
    }
  },
);

export const fetchConservationAlerts = createAsyncThunk(
  'analytics/fetchConservationAlerts',
  async (_, {rejectWithValue}) => {
    try {
      const response = await analyticsService.getConservationAlerts();
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to fetch conservation alerts');
    }
  },
);

const analyticsSlice = createSlice({
  name: 'analytics',
  initialState,
  reducers: {
    clearError: state => {
      state.error = null;
    },
    clearAnalytics: state => {
      state.dashboardData = null;
      state.biodiversityMetrics = null;
      state.activityPatterns = [];
      state.populationTrends = [];
      state.conservationAlerts = [];
      state.lastUpdate = null;
    },
  },
  extraReducers: builder => {
    builder
      .addCase(fetchDashboardData.pending, state => {
        state.isLoading = true;
        state.error = null;
      })
      .addCase(fetchDashboardData.fulfilled, (state, action) => {
        state.isLoading = false;
        state.dashboardData = action.payload;
        state.lastUpdate = new Date().toISOString();
        state.error = null;
      })
      .addCase(fetchDashboardData.rejected, (state, action) => {
        state.isLoading = false;
        state.error = action.payload as string;
      })
      .addCase(fetchBiodiversityMetrics.fulfilled, (state, action) => {
        state.biodiversityMetrics = action.payload;
      })
      .addCase(fetchActivityPatterns.fulfilled, (state, action) => {
        state.activityPatterns = action.payload;
      })
      .addCase(fetchPopulationTrends.fulfilled, (state, action) => {
        state.populationTrends = action.payload;
      })
      .addCase(fetchConservationAlerts.fulfilled, (state, action) => {
        state.conservationAlerts = action.payload;
      });
  },
});

export const {clearError, clearAnalytics} = analyticsSlice.actions;
export default analyticsSlice.reducer;