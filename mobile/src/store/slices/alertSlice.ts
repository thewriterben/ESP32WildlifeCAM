import {createSlice, createAsyncThunk, PayloadAction} from '@reduxjs/toolkit';
import {Alert} from '@types/index';
import alertService from '@services/alertService';

interface AlertState {
  alerts: Alert[];
  unreadCount: number;
  isLoading: boolean;
  error: string | null;
}

const initialState: AlertState = {
  alerts: [],
  unreadCount: 0,
  isLoading: false,
  error: null,
};

export const fetchAlerts = createAsyncThunk(
  'alerts/fetchAlerts',
  async (params: {read?: boolean; limit?: number}, {rejectWithValue}) => {
    try {
      const response = await alertService.getAlerts(params);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to fetch alerts');
    }
  },
);

export const markAlertAsRead = createAsyncThunk(
  'alerts/markAsRead',
  async (alertId: number, {rejectWithValue}) => {
    try {
      const response = await alertService.markAsRead(alertId);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to mark alert as read');
    }
  },
);

export const acknowledgeAlert = createAsyncThunk(
  'alerts/acknowledge',
  async (alertId: number, {rejectWithValue}) => {
    try {
      const response = await alertService.acknowledgeAlert(alertId);
      return response.data;
    } catch (error: any) {
      return rejectWithValue(error.response?.data?.message || 'Failed to acknowledge alert');
    }
  },
);

const alertSlice = createSlice({
  name: 'alerts',
  initialState,
  reducers: {
    clearError: state => {
      state.error = null;
    },
    addRealtimeAlert: (state, action: PayloadAction<Alert>) => {
      state.alerts.unshift(action.payload);
      if (!action.payload.read) {
        state.unreadCount += 1;
      }
    },
    clearAlerts: state => {
      state.alerts = [];
      state.unreadCount = 0;
    },
  },
  extraReducers: builder => {
    builder
      .addCase(fetchAlerts.pending, state => {
        state.isLoading = true;
        state.error = null;
      })
      .addCase(fetchAlerts.fulfilled, (state, action) => {
        state.isLoading = false;
        state.alerts = action.payload;
        state.unreadCount = action.payload.filter((alert: Alert) => !alert.read).length;
        state.error = null;
      })
      .addCase(fetchAlerts.rejected, (state, action) => {
        state.isLoading = false;
        state.error = action.payload as string;
      })
      .addCase(markAlertAsRead.fulfilled, (state, action) => {
        const alert = state.alerts.find(a => a.id === action.payload.id);
        if (alert && !alert.read) {
          alert.read = true;
          state.unreadCount -= 1;
        }
      })
      .addCase(acknowledgeAlert.fulfilled, (state, action) => {
        const alert = state.alerts.find(a => a.id === action.payload.id);
        if (alert) {
          alert.acknowledged = true;
        }
      });
  },
});

export const {clearError, addRealtimeAlert, clearAlerts} = alertSlice.actions;
export default alertSlice.reducer;