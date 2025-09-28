import React, { useState, useEffect } from 'react';
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import { ThemeProvider, createTheme } from '@mui/material/styles';
import CssBaseline from '@mui/material/CssBaseline';
import { Box, Alert, Snackbar } from '@mui/material';

// Components
import Navigation from './components/Navigation';
import ProtectedRoute from './components/ProtectedRoute';

// Pages
import Login from './pages/Login';
import Dashboard from './pages/Dashboard';
import CameraList from './pages/CameraList';
import CameraDetail from './pages/CameraDetail';
import ImageGallery from './pages/ImageGallery';
import Analytics from './pages/Analytics';
import SpeciesLibrary from './pages/SpeciesLibrary';
import Settings from './pages/Settings';

// Services
import authService from './services/authService';
import { AlertProvider, useAlert } from './contexts/AlertContext';
import { AuthProvider, useAuth } from './contexts/AuthContext';

// Create theme
const theme = createTheme({
  palette: {
    mode: 'light',
    primary: {
      main: '#2e7d32', // Forest green
      light: '#60ad5e',
      dark: '#005005',
    },
    secondary: {
      main: '#8bc34a', // Light green
      light: '#bef67a',
      dark: '#5a9216',
    },
    background: {
      default: '#f1f8e9',
      paper: '#ffffff',
    },
    success: {
      main: '#4caf50',
    },
    warning: {
      main: '#ff9800',
    },
    error: {
      main: '#f44336',
    },
  },
  typography: {
    fontFamily: '"Roboto", "Helvetica", "Arial", sans-serif',
    h4: {
      fontWeight: 600,
    },
    h5: {
      fontWeight: 500,
    },
    h6: {
      fontWeight: 500,
    },
  },
  components: {
    MuiCard: {
      styleOverrides: {
        root: {
          boxShadow: '0 2px 8px rgba(0,0,0,0.1)',
          borderRadius: 12,
        },
      },
    },
    MuiButton: {
      styleOverrides: {
        root: {
          borderRadius: 8,
          textTransform: 'none',
          fontWeight: 500,
        },
      },
    },
  },
});

function AppContent() {
  const { user } = useAuth();
  const { alert, hideAlert } = useAlert();

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <Router>
        <Box sx={{ display: 'flex', minHeight: '100vh' }}>
          {user && <Navigation />}
          
          <Box component="main" sx={{ flexGrow: 1, bgcolor: 'background.default' }}>
            <Routes>
              <Route 
                path="/login" 
                element={user ? <Navigate to="/dashboard" /> : <Login />} 
              />
              
              <Route path="/" element={<Navigate to="/dashboard" />} />
              
              <Route 
                path="/dashboard" 
                element={
                  <ProtectedRoute>
                    <Dashboard />
                  </ProtectedRoute>
                } 
              />
              
              <Route 
                path="/cameras" 
                element={
                  <ProtectedRoute>
                    <CameraList />
                  </ProtectedRoute>
                } 
              />
              
              <Route 
                path="/cameras/:id" 
                element={
                  <ProtectedRoute>
                    <CameraDetail />
                  </ProtectedRoute>
                } 
              />
              
              <Route 
                path="/gallery" 
                element={
                  <ProtectedRoute>
                    <ImageGallery />
                  </ProtectedRoute>
                } 
              />
              
              <Route 
                path="/analytics" 
                element={
                  <ProtectedRoute>
                    <Analytics />
                  </ProtectedRoute>
                } 
              />
              
              <Route 
                path="/species" 
                element={
                  <ProtectedRoute>
                    <SpeciesLibrary />
                  </ProtectedRoute>
                } 
              />
              
              <Route 
                path="/settings" 
                element={
                  <ProtectedRoute>
                    <Settings />
                  </ProtectedRoute>
                } 
              />
              
              <Route path="*" element={<Navigate to="/dashboard" />} />
            </Routes>
          </Box>
        </Box>
        
        {/* Global alert snackbar */}
        <Snackbar
          open={!!alert}
          autoHideDuration={6000}
          onClose={hideAlert}
          anchorOrigin={{ vertical: 'top', horizontal: 'right' }}
        >
          {alert && (
            <Alert 
              onClose={hideAlert} 
              severity={alert.type} 
              sx={{ width: '100%' }}
            >
              {alert.message}
            </Alert>
          )}
        </Snackbar>
      </Router>
    </ThemeProvider>
  );
}

function App() {
  return (
    <AlertProvider>
      <AuthProvider>
        <AppContent />
      </AuthProvider>
    </AlertProvider>
  );
}

export default App;