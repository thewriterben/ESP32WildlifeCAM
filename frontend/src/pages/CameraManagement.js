import React, { useState, useEffect } from 'react';
import {
  Box,
  Grid,
  Card,
  CardContent,
  CardActions,
  Typography,
  Button,
  IconButton,
  Avatar,
  Chip,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  Switch,
  FormControlLabel,
  LinearProgress,
  Tabs,
  Tab,
} from '@mui/material';
import {
  PhotoCamera,
  Settings,
  Refresh,
  Add,
  Delete,
  SignalWifi4Bar,
  Thermostat,
  Memory,
  WifiOff,
  CheckCircle,
  Error,
  Warning,
  PowerSettingsNew,
  CameraAlt,
} from '@mui/icons-material';

import cameraService from '../services/cameraService';
import wsService from '../services/websocket';
import { useAlert } from '../contexts/AlertContext';

function TabPanel({ children, value, index, ...other }) {
  return (
    <div
      role="tabpanel"
      hidden={value !== index}
      id={`camera-tabpanel-${index}`}
      aria-labelledby={`camera-tab-${index}`}
      {...other}
    >
      {value === index && <Box sx={{ p: 3 }}>{children}</Box>}
    </div>
  );
}

function CameraManagement() {
  const [cameras, setCameras] = useState([]);
  const [loading, setLoading] = useState(true);
  const [selectedCamera, setSelectedCamera] = useState(null);
  const [configDialogOpen, setConfigDialogOpen] = useState(false);
  const [addDialogOpen, setAddDialogOpen] = useState(false);
  const [tabValue, setTabValue] = useState(0);
  const [cameraConfig, setCameraConfig] = useState({
    name: '',
    location: '',
    detection_threshold: 0.7,
    motion_sensitivity: 5,
    image_quality: 'high',
    capture_interval: 30,
    battery_threshold: 20,
    storage_limit: 1000,
    night_mode: true,
    flash_enabled: false,
  });

  const { showAlert } = useAlert();

  useEffect(() => {
    const loadCameras = async () => {
      try {
        setLoading(true);
        const response = await cameraService.getCameras();
        setCameras(response.cameras || []);
      } catch (error) {
        showAlert('Failed to load cameras', 'error');
        console.error('Error loading cameras:', error);
      } finally {
        setLoading(false);
      }
    };

    const handleCameraStatusUpdate = (data) => {
      setCameras(prev => 
        prev.map(camera => 
          camera.id === data.camera_id 
            ? { ...camera, ...data.status }
            : camera
        )
      );
    };

    const handleCameraOnline = (data) => {
      setCameras(prev => 
        prev.map(camera => 
          camera.id === data.camera_id 
            ? { ...camera, status: 'online', last_seen: new Date().toISOString() }
            : camera
        )
      );
      showAlert(`Camera ${data.camera_name} is now online`, 'success');
    };

    const handleCameraOffline = (data) => {
      setCameras(prev => 
        prev.map(camera => 
          camera.id === data.camera_id 
            ? { ...camera, status: 'offline' }
            : camera
        )
      );
      showAlert(`Camera ${data.camera_name} went offline`, 'warning');
    };

    loadCameras();
    
    // Set up WebSocket listeners for real-time updates
    wsService.on('camera_status', handleCameraStatusUpdate);
    wsService.on('camera_online', handleCameraOnline);
    wsService.on('camera_offline', handleCameraOffline);

    return () => {
      wsService.off('camera_status', handleCameraStatusUpdate);
      wsService.off('camera_online', handleCameraOnline);
      wsService.off('camera_offline', handleCameraOffline);
    };
  }, [showAlert]);

  const refreshCameras = async () => {
    try {
      setLoading(true);
      const response = await cameraService.getCameras();
      setCameras(response.cameras || []);
    } catch (error) {
      showAlert('Failed to load cameras', 'error');
      console.error('Error loading cameras:', error);
    } finally {
      setLoading(false);
    }
  };



  const handleConfigureCamera = (camera) => {
    setSelectedCamera(camera);
    setCameraConfig({
      name: camera.name || '',
      location: camera.location || '',
      detection_threshold: camera.detection_threshold || 0.7,
      motion_sensitivity: camera.motion_sensitivity || 5,
      image_quality: camera.image_quality || 'high',
      capture_interval: camera.capture_interval || 30,
      battery_threshold: camera.battery_threshold || 20,
      storage_limit: camera.storage_limit || 1000,
      night_mode: camera.night_mode || true,
      flash_enabled: camera.flash_enabled || false,
    });
    setConfigDialogOpen(true);
  };

  const handleSaveConfiguration = async () => {
    try {
      await cameraService.updateCamera(selectedCamera.id, cameraConfig);
      await cameraService.sendConfiguration(selectedCamera.id, cameraConfig);
      
      setCameras(prev => 
        prev.map(camera => 
          camera.id === selectedCamera.id 
            ? { ...camera, ...cameraConfig }
            : camera
        )
      );
      
      showAlert('Camera configuration updated successfully', 'success');
      setConfigDialogOpen(false);
    } catch (error) {
      showAlert('Failed to update camera configuration', 'error');
      console.error('Error updating camera:', error);
    }
  };

  const handleAddCamera = async () => {
    try {
      const newCamera = await cameraService.registerCamera(cameraConfig);
      setCameras(prev => [...prev, newCamera.camera]);
      showAlert('Camera registered successfully', 'success');
      setAddDialogOpen(false);
      setCameraConfig({
        name: '',
        location: '',
        detection_threshold: 0.7,
        motion_sensitivity: 5,
        image_quality: 'high',
        capture_interval: 30,
        battery_threshold: 20,
        storage_limit: 1000,
        night_mode: true,
        flash_enabled: false,
      });
    } catch (error) {
      showAlert('Failed to register camera', 'error');
      console.error('Error registering camera:', error);
    }
  };

  const handleDeleteCamera = async (cameraId) => {
    if (window.confirm('Are you sure you want to delete this camera?')) {
      try {
        await cameraService.deleteCamera(cameraId);
        setCameras(prev => prev.filter(camera => camera.id !== cameraId));
        showAlert('Camera deleted successfully', 'success');
      } catch (error) {
        showAlert('Failed to delete camera', 'error');
        console.error('Error deleting camera:', error);
      }
    }
  };

  const handleRestartCamera = async (cameraId) => {
    try {
      await cameraService.restartCamera(cameraId);
      showAlert('Camera restart command sent', 'info');
    } catch (error) {
      showAlert('Failed to restart camera', 'error');
      console.error('Error restarting camera:', error);
    }
  };

  const handleTestConnection = async (cameraId) => {
    try {
      const result = await cameraService.testConnection(cameraId);
      if (result.success) {
        showAlert('Camera connection test successful', 'success');
      } else {
        showAlert('Camera connection test failed', 'error');
      }
    } catch (error) {
      showAlert('Failed to test camera connection', 'error');
      console.error('Error testing connection:', error);
    }
  };

  const getStatusColor = (status) => {
    switch (status) {
      case 'online': return 'success';
      case 'offline': return 'error';
      case 'warning': return 'warning';
      default: return 'default';
    }
  };

  const getStatusIcon = (status) => {
    switch (status) {
      case 'online': return <CheckCircle />;
      case 'offline': return <Error />;
      case 'warning': return <Warning />;
      default: return <WifiOff />;
    }
  };

  const getBatteryColor = (level) => {
    if (level > 50) return 'success';
    if (level > 20) return 'warning';
    return 'error';
  };

  const renderCameraCard = (camera) => (
    <Grid item xs={12} sm={6} md={4} key={camera.id}>
      <Card>
        <CardContent>
          <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
            <Avatar sx={{ bgcolor: 'primary.main', mr: 2 }}>
              <PhotoCamera />
            </Avatar>
            <Box sx={{ flexGrow: 1 }}>
              <Typography variant="h6" noWrap>
                {camera.name}
              </Typography>
              <Typography variant="body2" color="textSecondary" noWrap>
                {camera.location}
              </Typography>
            </Box>
            <Chip
              icon={getStatusIcon(camera.status)}
              label={camera.status}
              color={getStatusColor(camera.status)}
              size="small"
            />
          </Box>

          <Box sx={{ mb: 2 }}>
            <Typography variant="body2" gutterBottom>
              Battery Level
            </Typography>
            <Box sx={{ display: 'flex', alignItems: 'center' }}>
              <LinearProgress
                variant="determinate"
                value={camera.battery_level || 0}
                color={getBatteryColor(camera.battery_level || 0)}
                sx={{ flexGrow: 1, mr: 1 }}
              />
              <Typography variant="body2" color="textSecondary">
                {camera.battery_level || 0}%
              </Typography>
            </Box>
          </Box>

          <Grid container spacing={2}>
            <Grid item xs={6}>
              <Box sx={{ display: 'flex', alignItems: 'center' }}>
                <SignalWifi4Bar fontSize="small" sx={{ mr: 1 }} />
                <Typography variant="body2">
                  {camera.signal_strength || 0}%
                </Typography>
              </Box>
            </Grid>
            <Grid item xs={6}>
              <Box sx={{ display: 'flex', alignItems: 'center' }}>
                <Thermostat fontSize="small" sx={{ mr: 1 }} />
                <Typography variant="body2">
                  {camera.temperature || 0}°C
                </Typography>
              </Box>
            </Grid>
            <Grid item xs={6}>
              <Box sx={{ display: 'flex', alignItems: 'center' }}>
                <CameraAlt fontSize="small" sx={{ mr: 1 }} />
                <Typography variant="body2">
                  {camera.total_images || 0}
                </Typography>
              </Box>
            </Grid>
            <Grid item xs={6}>
              <Box sx={{ display: 'flex', alignItems: 'center' }}>
                <Memory fontSize="small" sx={{ mr: 1 }} />
                <Typography variant="body2">
                  {((camera.storage_used || 0) / 1024).toFixed(1)}GB
                </Typography>
              </Box>
            </Grid>
          </Grid>

          <Typography variant="body2" color="textSecondary" sx={{ mt: 2 }}>
            Last seen: {camera.last_seen ? new Date(camera.last_seen).toLocaleString() : 'Never'}
          </Typography>
        </CardContent>

        <CardActions>
          <Button size="small" onClick={() => handleConfigureCamera(camera)}>
            <Settings fontSize="small" sx={{ mr: 1 }} />
            Configure
          </Button>
          <Button size="small" onClick={() => handleTestConnection(camera.id)}>
            <Refresh fontSize="small" sx={{ mr: 1 }} />
            Test
          </Button>
          <IconButton size="small" onClick={() => handleRestartCamera(camera.id)}>
            <PowerSettingsNew />
          </IconButton>
          <IconButton size="small" onClick={() => handleDeleteCamera(camera.id)} color="error">
            <Delete />
          </IconButton>
        </CardActions>
      </Card>
    </Grid>
  );

  const renderConfigDialog = () => (
    <Dialog open={configDialogOpen} onClose={() => setConfigDialogOpen(false)} maxWidth="md" fullWidth>
      <DialogTitle>
        Configure Camera: {selectedCamera?.name}
      </DialogTitle>
      <DialogContent>
        <Tabs value={tabValue} onChange={(e, newValue) => setTabValue(newValue)}>
          <Tab label="Basic Settings" />
          <Tab label="Detection Settings" />
          <Tab label="Power & Storage" />
        </Tabs>

        <TabPanel value={tabValue} index={0}>
          <Grid container spacing={3}>
            <Grid item xs={12} md={6}>
              <TextField
                fullWidth
                label="Camera Name"
                value={cameraConfig.name}
                onChange={(e) => setCameraConfig(prev => ({ ...prev, name: e.target.value }))}
                margin="normal"
              />
            </Grid>
            <Grid item xs={12} md={6}>
              <TextField
                fullWidth
                label="Location"
                value={cameraConfig.location}
                onChange={(e) => setCameraConfig(prev => ({ ...prev, location: e.target.value }))}
                margin="normal"
              />
            </Grid>
            <Grid item xs={12} md={6}>
              <FormControl fullWidth margin="normal">
                <InputLabel>Image Quality</InputLabel>
                <Select
                  value={cameraConfig.image_quality}
                  onChange={(e) => setCameraConfig(prev => ({ ...prev, image_quality: e.target.value }))}
                  label="Image Quality"
                >
                  <MenuItem value="low">Low</MenuItem>
                  <MenuItem value="medium">Medium</MenuItem>
                  <MenuItem value="high">High</MenuItem>
                  <MenuItem value="ultra">Ultra</MenuItem>
                </Select>
              </FormControl>
            </Grid>
            <Grid item xs={12} md={6}>
              <TextField
                fullWidth
                type="number"
                label="Capture Interval (seconds)"
                value={cameraConfig.capture_interval}
                onChange={(e) => setCameraConfig(prev => ({ ...prev, capture_interval: parseInt(e.target.value) }))}
                margin="normal"
              />
            </Grid>
          </Grid>
        </TabPanel>

        <TabPanel value={tabValue} index={1}>
          <Grid container spacing={3}>
            <Grid item xs={12} md={6}>
              <Typography gutterBottom>
                Detection Threshold: {cameraConfig.detection_threshold}
              </Typography>
              <TextField
                fullWidth
                type="number"
                inputProps={{ min: 0.1, max: 1.0, step: 0.1 }}
                value={cameraConfig.detection_threshold}
                onChange={(e) => setCameraConfig(prev => ({ ...prev, detection_threshold: parseFloat(e.target.value) }))}
              />
            </Grid>
            <Grid item xs={12} md={6}>
              <Typography gutterBottom>
                Motion Sensitivity: {cameraConfig.motion_sensitivity}
              </Typography>
              <TextField
                fullWidth
                type="number"
                inputProps={{ min: 1, max: 10 }}
                value={cameraConfig.motion_sensitivity}
                onChange={(e) => setCameraConfig(prev => ({ ...prev, motion_sensitivity: parseInt(e.target.value) }))}
              />
            </Grid>
            <Grid item xs={12}>
              <FormControlLabel
                control={
                  <Switch
                    checked={cameraConfig.night_mode}
                    onChange={(e) => setCameraConfig(prev => ({ ...prev, night_mode: e.target.checked }))}
                  />
                }
                label="Night Mode"
              />
            </Grid>
            <Grid item xs={12}>
              <FormControlLabel
                control={
                  <Switch
                    checked={cameraConfig.flash_enabled}
                    onChange={(e) => setCameraConfig(prev => ({ ...prev, flash_enabled: e.target.checked }))}
                  />
                }
                label="Flash Enabled"
              />
            </Grid>
          </Grid>
        </TabPanel>

        <TabPanel value={tabValue} index={2}>
          <Grid container spacing={3}>
            <Grid item xs={12} md={6}>
              <TextField
                fullWidth
                type="number"
                label="Battery Threshold (%)"
                value={cameraConfig.battery_threshold}
                onChange={(e) => setCameraConfig(prev => ({ ...prev, battery_threshold: parseInt(e.target.value) }))}
                margin="normal"
              />
            </Grid>
            <Grid item xs={12} md={6}>
              <TextField
                fullWidth
                type="number"
                label="Storage Limit (MB)"
                value={cameraConfig.storage_limit}
                onChange={(e) => setCameraConfig(prev => ({ ...prev, storage_limit: parseInt(e.target.value) }))}
                margin="normal"
              />
            </Grid>
          </Grid>
        </TabPanel>
      </DialogContent>
      <DialogActions>
        <Button onClick={() => setConfigDialogOpen(false)}>
          Cancel
        </Button>
        <Button onClick={handleSaveConfiguration} variant="contained">
          Save Configuration
        </Button>
      </DialogActions>
    </Dialog>
  );

  const renderAddDialog = () => (
    <Dialog open={addDialogOpen} onClose={() => setAddDialogOpen(false)} maxWidth="sm" fullWidth>
      <DialogTitle>Add New Camera</DialogTitle>
      <DialogContent>
        <TextField
          fullWidth
          label="Camera Name"
          value={cameraConfig.name}
          onChange={(e) => setCameraConfig(prev => ({ ...prev, name: e.target.value }))}
          margin="normal"
        />
        <TextField
          fullWidth
          label="Location"
          value={cameraConfig.location}
          onChange={(e) => setCameraConfig(prev => ({ ...prev, location: e.target.value }))}
          margin="normal"
        />
      </DialogContent>
      <DialogActions>
        <Button onClick={() => setAddDialogOpen(false)}>
          Cancel
        </Button>
        <Button onClick={handleAddCamera} variant="contained">
          Add Camera
        </Button>
      </DialogActions>
    </Dialog>
  );

  if (loading) {
    return (
      <Box sx={{ p: 3 }}>
        <LinearProgress />
        <Typography sx={{ mt: 2 }}>Loading cameras...</Typography>
      </Box>
    );
  }

  return (
    <Box sx={{ p: 3 }}>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Camera Management
        </Typography>
        <Box>
          <Button
            variant="outlined"
            startIcon={<Refresh />}
            onClick={refreshCameras}
            sx={{ mr: 2 }}
          >
            Refresh
          </Button>
          <Button
            variant="contained"
            startIcon={<Add />}
            onClick={() => setAddDialogOpen(true)}
          >
            Add Camera
          </Button>
        </Box>
      </Box>

      <Grid container spacing={3}>
        {cameras.map(renderCameraCard)}
      </Grid>

      {cameras.length === 0 && (
        <Box sx={{ textAlign: 'center', py: 8 }}>
          <PhotoCamera sx={{ fontSize: 64, color: 'text.secondary', mb: 2 }} />
          <Typography variant="h6" color="textSecondary" gutterBottom>
            No cameras registered
          </Typography>
          <Typography variant="body2" color="textSecondary" sx={{ mb: 3 }}>
            Add your first wildlife camera to start monitoring
          </Typography>
          <Button
            variant="contained"
            startIcon={<Add />}
            onClick={() => setAddDialogOpen(true)}
          >
            Add Camera
          </Button>
        </Box>
      )}

      {renderConfigDialog()}
      {renderAddDialog()}
    </Box>
  );
}

export default CameraManagement;