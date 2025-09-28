import React, { useState, useEffect } from 'react';
import {
  Box,
  Grid,
  Card,
  CardContent,
  Typography,
  IconButton,
  Button,
  Chip,
  LinearProgress,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
} from '@mui/material';
import {
  Add,
  Edit,
  Delete,
  Refresh,
  LocationOn,
  Battery4Bar,
  SignalWifi4Bar,
  PhotoCamera,
} from '@mui/icons-material';

import cameraService from '../services/cameraService';
import { useAlert } from '../contexts/AlertContext';

const CameraList = () => {
  const [cameras, setCameras] = useState([]);
  const [loading, setLoading] = useState(true);
  const [openDialog, setOpenDialog] = useState(false);
  const [editingCamera, setEditingCamera] = useState(null);
  const [formData, setFormData] = useState({
    name: '',
    location: '',
    latitude: '',
    longitude: '',
    device_id: '',
    configuration: {},
  });

  const { showAlert } = useAlert();

  useEffect(() => {
    loadCameras();
  }, []);

  const loadCameras = async () => {
    try {
      setLoading(true);
      const data = await cameraService.getCameras();
      setCameras(data.cameras || []);
    } catch (error) {
      console.error('Failed to load cameras:', error);
      showAlert('Failed to load cameras', 'error');
    } finally {
      setLoading(false);
    }
  };

  const handleOpenDialog = (camera = null) => {
    if (camera) {
      setEditingCamera(camera);
      setFormData({
        name: camera.name,
        location: camera.location,
        latitude: camera.latitude || '',
        longitude: camera.longitude || '',
        device_id: camera.device_id,
        configuration: camera.configuration || {},
      });
    } else {
      setEditingCamera(null);
      setFormData({
        name: '',
        location: '',
        latitude: '',
        longitude: '',
        device_id: '',
        configuration: {},
      });
    }
    setOpenDialog(true);
  };

  const handleCloseDialog = () => {
    setOpenDialog(false);
    setEditingCamera(null);
  };

  const handleFormChange = (e) => {
    setFormData({
      ...formData,
      [e.target.name]: e.target.value,
    });
  };

  const handleSubmit = async () => {
    try {
      const cameraData = {
        ...formData,
        latitude: formData.latitude ? parseFloat(formData.latitude) : null,
        longitude: formData.longitude ? parseFloat(formData.longitude) : null,
      };

      if (editingCamera) {
        await cameraService.updateCamera(editingCamera.id, cameraData);
        showAlert('Camera updated successfully', 'success');
      } else {
        await cameraService.registerCamera(cameraData);
        showAlert('Camera registered successfully', 'success');
      }

      handleCloseDialog();
      loadCameras();
    } catch (error) {
      console.error('Failed to save camera:', error);
      showAlert('Failed to save camera', 'error');
    }
  };

  const handleDelete = async (cameraId) => {
    if (window.confirm('Are you sure you want to delete this camera?')) {
      try {
        await cameraService.deleteCamera(cameraId);
        showAlert('Camera deleted successfully', 'success');
        loadCameras();
      } catch (error) {
        console.error('Failed to delete camera:', error);
        showAlert('Failed to delete camera', 'error');
      }
    }
  };

  const getStatusColor = (status) => {
    switch (status) {
      case 'online':
        return 'success';
      case 'offline':
        return 'error';
      case 'maintenance':
        return 'warning';
      default:
        return 'default';
    }
  };

  const getBatteryColor = (level) => {
    if (level >= 60) return 'success';
    if (level >= 30) return 'warning';
    return 'error';
  };

  if (loading) {
    return (
      <Box sx={{ p: 3 }}>
        <Typography variant="h4" gutterBottom>
          Loading Cameras...
        </Typography>
        <LinearProgress />
      </Box>
    );
  }

  return (
    <Box sx={{ p: 3 }}>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Wildlife Cameras
        </Typography>
        <Box>
          <IconButton onClick={loadCameras} color="primary">
            <Refresh />
          </IconButton>
          <Button
            variant="contained"
            startIcon={<Add />}
            onClick={() => handleOpenDialog()}
            sx={{ ml: 1 }}
          >
            Add Camera
          </Button>
        </Box>
      </Box>

      <Grid container spacing={3}>
        {cameras.map((camera) => (
          <Grid item xs={12} sm={6} md={4} lg={3} key={camera.id}>
            <Card>
              <CardContent>
                <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', mb: 2 }}>
                  <Typography variant="h6" component="h3">
                    {camera.name}
                  </Typography>
                  <Box>
                    <IconButton 
                      size="small" 
                      onClick={() => handleOpenDialog(camera)}
                      color="primary"
                    >
                      <Edit />
                    </IconButton>
                    <IconButton 
                      size="small" 
                      onClick={() => handleDelete(camera.id)}
                      color="error"
                    >
                      <Delete />
                    </IconButton>
                  </Box>
                </Box>

                <Box sx={{ display: 'flex', alignItems: 'center', mb: 1 }}>
                  <LocationOn sx={{ fontSize: 16, mr: 0.5, color: 'text.secondary' }} />
                  <Typography variant="body2" color="text.secondary">
                    {camera.location}
                  </Typography>
                </Box>

                <Box sx={{ mb: 2 }}>
                  <Chip
                    label={camera.status}
                    color={getStatusColor(camera.status)}
                    size="small"
                  />
                </Box>

                <Box sx={{ mb: 1 }}>
                  <Box sx={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
                    <Box sx={{ display: 'flex', alignItems: 'center' }}>
                      <Battery4Bar sx={{ fontSize: 16, mr: 0.5 }} />
                      <Typography variant="body2">
                        Battery
                      </Typography>
                    </Box>
                    <Typography variant="body2">
                      {camera.battery_level || 0}%
                    </Typography>
                  </Box>
                  <LinearProgress
                    variant="determinate"
                    value={camera.battery_level || 0}
                    color={getBatteryColor(camera.battery_level || 0)}
                    sx={{ mt: 0.5 }}
                  />
                </Box>

                <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 1 }}>
                  <Box sx={{ display: 'flex', alignItems: 'center' }}>
                    <SignalWifi4Bar sx={{ fontSize: 16, mr: 0.5 }} />
                    <Typography variant="body2">Signal</Typography>
                  </Box>
                  <Typography variant="body2">
                    {camera.signal_strength || 0} dBm
                  </Typography>
                </Box>

                <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                  <Box sx={{ display: 'flex', alignItems: 'center' }}>
                    <PhotoCamera sx={{ fontSize: 16, mr: 0.5 }} />
                    <Typography variant="body2">Images</Typography>
                  </Box>
                  <Typography variant="body2">
                    {camera.total_images || 0}
                  </Typography>
                </Box>

                {camera.last_seen && (
                  <Typography variant="caption" color="text.secondary" sx={{ mt: 1, display: 'block' }}>
                    Last seen: {new Date(camera.last_seen).toLocaleString()}
                  </Typography>
                )}
              </CardContent>
            </Card>
          </Grid>
        ))}

        {cameras.length === 0 && (
          <Grid item xs={12}>
            <Box sx={{ textAlign: 'center', py: 8 }}>
              <PhotoCamera sx={{ fontSize: 64, color: 'text.secondary', mb: 2 }} />
              <Typography variant="h6" color="text.secondary" gutterBottom>
                No cameras registered
              </Typography>
              <Typography variant="body2" color="text.secondary" sx={{ mb: 3 }}>
                Add your first wildlife camera to start monitoring
              </Typography>
              <Button
                variant="contained"
                startIcon={<Add />}
                onClick={() => handleOpenDialog()}
              >
                Add Camera
              </Button>
            </Box>
          </Grid>
        )}
      </Grid>

      {/* Camera Dialog */}
      <Dialog open={openDialog} onClose={handleCloseDialog} maxWidth="sm" fullWidth>
        <DialogTitle>
          {editingCamera ? 'Edit Camera' : 'Add New Camera'}
        </DialogTitle>
        <DialogContent>
          <TextField
            autoFocus
            margin="dense"
            name="name"
            label="Camera Name"
            fullWidth
            variant="outlined"
            value={formData.name}
            onChange={handleFormChange}
            sx={{ mb: 2 }}
          />
          
          <TextField
            margin="dense"
            name="location"
            label="Location"
            fullWidth
            variant="outlined"
            value={formData.location}
            onChange={handleFormChange}
            sx={{ mb: 2 }}
          />

          <Box sx={{ display: 'flex', gap: 2, mb: 2 }}>
            <TextField
              margin="dense"
              name="latitude"
              label="Latitude"
              type="number"
              inputProps={{ step: 'any' }}
              variant="outlined"
              value={formData.latitude}
              onChange={handleFormChange}
              sx={{ flex: 1 }}
            />
            <TextField
              margin="dense"
              name="longitude"
              label="Longitude"
              type="number"
              inputProps={{ step: 'any' }}
              variant="outlined"
              value={formData.longitude}
              onChange={handleFormChange}
              sx={{ flex: 1 }}
            />
          </Box>

          <TextField
            margin="dense"
            name="device_id"
            label="Device ID"
            fullWidth
            variant="outlined"
            value={formData.device_id}
            onChange={handleFormChange}
            helperText="Unique identifier for the ESP32 device"
          />
        </DialogContent>
        <DialogActions>
          <Button onClick={handleCloseDialog}>Cancel</Button>
          <Button onClick={handleSubmit} variant="contained">
            {editingCamera ? 'Update' : 'Register'}
          </Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default CameraList;