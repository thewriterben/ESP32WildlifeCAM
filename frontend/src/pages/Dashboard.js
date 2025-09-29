import React, { useState, useEffect } from 'react';
import {
  Box,
  Grid,
  Card,
  CardContent,
  Typography,
  IconButton,
  Avatar,
  LinearProgress,
  Chip,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Paper,
} from '@mui/material';
import {
  PhotoCamera,
  Pets,
  Battery90,
  SignalWifi4Bar,
  Refresh,
  Warning,
  CheckCircle,
  Error,
} from '@mui/icons-material';
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
  PieChart,
  Pie,
  Cell,
  BarChart,
  Bar,
} from 'recharts';

import analyticsService from '../services/analyticsService';
import cameraService from '../services/cameraService';
import { useAlert } from '../contexts/AlertContext';

const COLORS = ['#0088FE', '#00C49F', '#FFBB28', '#FF8042', '#8884D8'];

function Dashboard() {
  const [analytics, setAnalytics] = useState(null);
  const [cameras, setCameras] = useState([]);
  const [loading, setLoading] = useState(true);
  const { showAlert } = useAlert();

  useEffect(() => {
    const loadData = async () => {
      try {
        setLoading(true);
        
        // Load analytics and camera data in parallel
        const [analyticsResponse, camerasResponse] = await Promise.all([
          analyticsService.getDashboardAnalytics(),
          cameraService.getCameras()
        ]);
        
        setAnalytics(analyticsResponse);
        setCameras(camerasResponse.cameras || []);
      } catch (error) {
        showAlert('Failed to load dashboard data', 'error');
        console.error('Error loading dashboard data:', error);
      } finally {
        setLoading(false);
      }
    };

    loadData();
  }, [showAlert]);

  const refreshDashboardData = async () => {
    try {
      setLoading(true);
      
      // Load analytics and camera data in parallel
      const [analyticsData, camerasData] = await Promise.all([
        analyticsService.getDashboardAnalytics(),
        cameraService.getCameras(),
      ]);

      setAnalytics(analyticsData);
      setCameras(camerasData.cameras || []);
    } catch (error) {
      console.error('Failed to load dashboard data:', error);
      showAlert('Failed to load dashboard data', 'error');
    } finally {
      setLoading(false);
    }
  };

  const getHealthColor = (score) => {
    if (score >= 80) return 'success';
    if (score >= 60) return 'warning';
    return 'error';
  };

  const getHealthIcon = (score) => {
    if (score >= 80) return <CheckCircle color="success" />;
    if (score >= 60) return <Warning color="warning" />;
    return <Error color="error" />;
  };

  if (loading) {
    return (
      <Box sx={{ p: 3 }}>
        <Typography variant="h4" gutterBottom>
          Loading Dashboard...
        </Typography>
        <LinearProgress />
      </Box>
    );
  }

  if (!analytics) {
    return (
      <Box sx={{ p: 3 }}>
        <Typography variant="h4" color="error">
          Failed to load dashboard data
        </Typography>
      </Box>
    );
  }

  return (
    <Box sx={{ p: 3 }}>
      {/* Header */}
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1" gutterBottom>
          Wildlife Camera Dashboard
        </Typography>
        <IconButton onClick={refreshDashboardData} color="primary">
          <Refresh />
        </IconButton>
      </Box>

      {/* Summary Cards */}
      <Grid container spacing={3} sx={{ mb: 4 }}>
        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center' }}>
                <Avatar sx={{ bgcolor: 'primary.main', mr: 2 }}>
                  <PhotoCamera />
                </Avatar>
                <Box>
                  <Typography color="textSecondary" gutterBottom>
                    Active Cameras
                  </Typography>
                  <Typography variant="h4">
                    {analytics.summary.active_cameras}/{analytics.summary.total_cameras}
                  </Typography>
                </Box>
              </Box>
              <LinearProgress
                variant="determinate"
                value={analytics.summary.camera_health}
                color={getHealthColor(analytics.summary.camera_health)}
                sx={{ mt: 2 }}
              />
              <Typography variant="body2" color="textSecondary" sx={{ mt: 1 }}>
                {analytics.summary.camera_health}% healthy
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center' }}>
                <Avatar sx={{ bgcolor: 'secondary.main', mr: 2 }}>
                  <Pets />
                </Avatar>
                <Box>
                  <Typography color="textSecondary" gutterBottom>
                    Wildlife Detections
                  </Typography>
                  <Typography variant="h4">
                    {analytics.summary.total_detections}
                  </Typography>
                </Box>
              </Box>
              <Typography variant="body2" color="textSecondary" sx={{ mt: 2 }}>
                {analytics.summary.unique_species} unique species
              </Typography>
              <Typography variant="body2" color="textSecondary">
                {analytics.summary.detection_rate}% detection rate
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center' }}>
                <Avatar sx={{ bgcolor: 'success.main', mr: 2 }}>
                  <Battery90 />
                </Avatar>
                <Box>
                  <Typography color="textSecondary" gutterBottom>
                    System Health
                  </Typography>
                  <Typography variant="h4">
                    {analytics.system_health.overall_health_score}%
                  </Typography>
                </Box>
              </Box>
              <Box sx={{ display: 'flex', alignItems: 'center', mt: 1 }}>
                {getHealthIcon(analytics.system_health.overall_health_score)}
                <Typography variant="body2" color="textSecondary" sx={{ ml: 1 }}>
                  {analytics.system_health.system_status}
                </Typography>
              </Box>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center' }}>
                <Avatar sx={{ bgcolor: 'info.main', mr: 2 }}>
                  <SignalWifi4Bar />
                </Avatar>
                <Box>
                  <Typography color="textSecondary" gutterBottom>
                    Average Confidence
                  </Typography>
                  <Typography variant="h4">
                    {(analytics.summary.average_confidence * 100).toFixed(0)}%
                  </Typography>
                </Box>
              </Box>
              <Typography variant="body2" color="textSecondary" sx={{ mt: 2 }}>
                Detection accuracy
              </Typography>
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      {/* Charts Section */}
      <Grid container spacing={3} sx={{ mb: 4 }}>
        {/* Detection Timeline */}
        <Grid item xs={12} md={8}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Detection Timeline
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <LineChart data={analytics.detection_timeline}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="date" />
                  <YAxis />
                  <Tooltip />
                  <Line 
                    type="monotone" 
                    dataKey="detection_count" 
                    stroke="#8884d8" 
                    strokeWidth={2}
                    name="Detections"
                  />
                  <Line 
                    type="monotone" 
                    dataKey="species_count" 
                    stroke="#82ca9d" 
                    strokeWidth={2}
                    name="Species"
                  />
                </LineChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>

        {/* Species Breakdown */}
        <Grid item xs={12} md={4}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Species Distribution
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <PieChart>
                  <Pie
                    data={analytics.species_breakdown.slice(0, 5)}
                    cx="50%"
                    cy="50%"
                    labelLine={false}
                    label={({ name, value }) => `${name}: ${value}`}
                    outerRadius={80}
                    fill="#8884d8"
                    dataKey="detection_count"
                  >
                    {analytics.species_breakdown.slice(0, 5).map((entry, index) => (
                      <Cell key={`cell-${index}`} fill={COLORS[index % COLORS.length]} />
                    ))}
                  </Pie>
                  <Tooltip />
                </PieChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      {/* Activity Patterns */}
      {analytics.activity_patterns && (
        <Grid container spacing={3} sx={{ mb: 4 }}>
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Daily Activity Pattern
                </Typography>
                <ResponsiveContainer width="100%" height={250}>
                  <BarChart data={analytics.activity_patterns.hourly_pattern.hours.map((hour, index) => ({
                    hour: `${hour}:00`,
                    detections: analytics.activity_patterns.hourly_pattern.detections[index]
                  }))}>
                    <CartesianGrid strokeDasharray="3 3" />
                    <XAxis dataKey="hour" />
                    <YAxis />
                    <Tooltip />
                    <Bar dataKey="detections" fill="#8884d8" />
                  </BarChart>
                </ResponsiveContainer>
                <Typography variant="body2" color="textSecondary" sx={{ mt: 1 }}>
                  Peak activity: {analytics.activity_patterns.hourly_pattern.peak_hour}:00
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Weekly Activity Pattern
                </Typography>
                <ResponsiveContainer width="100%" height={250}>
                  <BarChart data={analytics.activity_patterns.daily_pattern.days.map((day, index) => ({
                    day: day.substring(0, 3),
                    detections: analytics.activity_patterns.daily_pattern.detections[index]
                  }))}>
                    <CartesianGrid strokeDasharray="3 3" />
                    <XAxis dataKey="day" />
                    <YAxis />
                    <Tooltip />
                    <Bar dataKey="detections" fill="#82ca9d" />
                  </BarChart>
                </ResponsiveContainer>
                <Typography variant="body2" color="textSecondary" sx={{ mt: 1 }}>
                  Peak day: {analytics.activity_patterns.daily_pattern.peak_day}
                </Typography>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {/* Camera Status Table */}
      <Grid container spacing={3}>
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Camera Status
              </Typography>
              <TableContainer component={Paper} variant="outlined">
                <Table>
                  <TableHead>
                    <TableRow>
                      <TableCell>Camera</TableCell>
                      <TableCell>Location</TableCell>
                      <TableCell>Status</TableCell>
                      <TableCell>Battery</TableCell>
                      <TableCell>Last Seen</TableCell>
                      <TableCell>Images</TableCell>
                      <TableCell>Detections</TableCell>
                    </TableRow>
                  </TableHead>
                  <TableBody>
                    {cameras.map((camera) => (
                      <TableRow key={camera.id}>
                        <TableCell>
                          <Typography variant="body2" fontWeight="medium">
                            {camera.name}
                          </Typography>
                        </TableCell>
                        <TableCell>{camera.location}</TableCell>
                        <TableCell>
                          <Chip
                            label={camera.status}
                            color={camera.status === 'online' ? 'success' : 'error'}
                            size="small"
                          />
                        </TableCell>
                        <TableCell>
                          <Box sx={{ display: 'flex', alignItems: 'center' }}>
                            <Typography variant="body2" sx={{ mr: 1 }}>
                              {camera.battery_level || 0}%
                            </Typography>
                            <LinearProgress
                              variant="determinate"
                              value={camera.battery_level || 0}
                              color={
                                (camera.battery_level || 0) >= 60
                                  ? 'success'
                                  : (camera.battery_level || 0) >= 30
                                  ? 'warning'
                                  : 'error'
                              }
                              sx={{ width: 60 }}
                            />
                          </Box>
                        </TableCell>
                        <TableCell>
                          <Typography variant="body2" color="textSecondary">
                            {camera.last_seen
                              ? new Date(camera.last_seen).toLocaleString()
                              : 'Never'}
                          </Typography>
                        </TableCell>
                        <TableCell>{camera.total_images}</TableCell>
                        <TableCell>{camera.recent_detections}</TableCell>
                      </TableRow>
                    ))}
                  </TableBody>
                </Table>
              </TableContainer>
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      {/* Conservation Alerts */}
      {analytics.conservation_alerts && analytics.conservation_alerts.length > 0 && (
        <Grid container spacing={3} sx={{ mt: 2 }}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom color="warning.main">
                  Conservation Alerts
                </Typography>
                {analytics.conservation_alerts.map((alert, index) => (
                  <Box
                    key={index}
                    sx={{
                      p: 2,
                      mb: 1,
                      border: 1,
                      borderColor: alert.severity === 'critical' ? 'error.main' : 'warning.main',
                      borderRadius: 1,
                      bgcolor: alert.severity === 'critical' ? 'error.light' : 'warning.light',
                      opacity: 0.1,
                    }}
                  >
                    <Typography variant="body1" fontWeight="medium">
                      {alert.message}
                    </Typography>
                    <Typography variant="body2" color="textSecondary">
                      Conservation Status: {alert.conservation_status}
                    </Typography>
                  </Box>
                ))}
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}
    </Box>
  );
}

export default Dashboard;