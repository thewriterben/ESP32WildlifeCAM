import React, { useState, useEffect } from 'react';
import {
  Box,
  Typography,
  Grid,
  Card,
  CardContent,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  Button,
  Tabs,
  Tab,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Chip,
  LinearProgress,
} from '@mui/material';
import {
  TrendingUp,
  Assessment,
  Download,
  Refresh,
  Timeline,
  PieChart,
} from '@mui/icons-material';
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
  PieChart as RechartsPieChart,
  Pie,
  Cell,
  AreaChart,
  Area,
} from 'recharts';

import analyticsService from '../services/analyticsService';
import { useAlert } from '../contexts/AlertContext';

const COLORS = ['#0088FE', '#00C49F', '#FFBB28', '#FF8042', '#8884D8', '#82ca9d'];

function TabPanel({ children, value, index }) {
  return (
    <div
      role="tabpanel"
      hidden={value !== index}
      id={`analytics-tabpanel-${index}`}
      aria-labelledby={`analytics-tab-${index}`}
    >
      {value === index && <Box sx={{ p: 3 }}>{children}</Box>}
    </div>
  );
}

const Analytics = () => {
  const [loading, setLoading] = useState(true);
  const [tabValue, setTabValue] = useState(0);
  const [timeRange, setTimeRange] = useState('7days');
  const [detectionTrends, setDetectionTrends] = useState([]);
  const [speciesData, setSpeciesData] = useState([]);
  const [activityPatterns, setActivityPatterns] = useState([]);
  const [biodiversityMetrics, setBiodiversityMetrics] = useState({});
  const [cameraPerformance, setCameraPerformance] = useState([]);
  const [conservationAlerts, setConservationAlerts] = useState([]);

  const { showAlert } = useAlert();

  useEffect(() => {
    const loadData = async () => {
      try {
        setLoading(true);

        const params = { time_range: timeRange };

        const [
          trendsData,
          activityData,
          biodiversityData,
          alertsData
        ] = await Promise.all([
          analyticsService.getDetectionTrends(params),
          analyticsService.getActivityPatterns(params),
          analyticsService.getBiodiversityMetrics(params),
          analyticsService.getConservationAlerts()
        ]);

        setDetectionTrends(trendsData.trends || []);
        setSpeciesData(trendsData.species_breakdown || []);
        setActivityPatterns(activityData.patterns || []);
        setBiodiversityMetrics(biodiversityData);
        setConservationAlerts(alertsData.alerts || []);

        // Mock camera performance data
        setCameraPerformance([
          { camera: 'Forest Trail 1', uptime: 98.5, detections: 156, battery: 85 },
          { camera: 'River Crossing', uptime: 95.2, detections: 203, battery: 72 },
          { camera: 'Mountain View', uptime: 92.1, detections: 89, battery: 91 },
          { camera: 'Valley Bottom', uptime: 99.1, detections: 178, battery: 68 },
        ]);

      } catch (error) {
        showAlert('Failed to load analytics data', 'error');
        console.error('Error loading analytics:', error);
      } finally {
        setLoading(false);
      }
    };

    loadData();
  }, [timeRange, showAlert]);

  const refreshAnalyticsData = async () => {
    try {
      setLoading(true);

      const params = { time_range: timeRange };

      const [
        trendsData,
        activityData,
        biodiversityData,
        alertsData
      ] = await Promise.all([
        analyticsService.getDetectionTrends(params),
        analyticsService.getActivityPatterns(params),
        analyticsService.getBiodiversityMetrics(params),
        analyticsService.getConservationAlerts()
      ]);

      setDetectionTrends(trendsData.trends || []);
      setSpeciesData(trendsData.species_breakdown || []);
      setActivityPatterns(activityData.patterns || []);
      setBiodiversityMetrics(biodiversityData);
      setConservationAlerts(alertsData.alerts || []);

      // Mock camera performance data
      setCameraPerformance([
        { camera: 'Forest Trail 1', uptime: 98.5, detections: 156, battery: 85 },
        { camera: 'River Crossing', uptime: 95.2, detections: 203, battery: 72 },
        { camera: 'Mountain View', uptime: 92.1, detections: 89, battery: 91 },
        { camera: 'Valley Bottom', uptime: 99.1, detections: 178, battery: 68 },
      ]);

    } catch (error) {
      showAlert('Failed to load analytics data', 'error');
      console.error('Error loading analytics:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleExportData = async (type) => {
    try {
      await analyticsService.exportData(type, { time_range: timeRange });
      showAlert('Export started - check your downloads', 'success');
    } catch (error) {
      showAlert('Failed to export data', 'error');
      console.error('Export error:', error);
    }
  };

  const renderDetectionTrends = () => (
    <Grid container spacing={3}>
      <Grid item xs={12}>
        <Card>
          <CardContent>
            <Typography variant="h6" gutterBottom>
              Detection Trends Over Time
            </Typography>
            <ResponsiveContainer width="100%" height={300}>
              <LineChart data={detectionTrends}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="date" />
                <YAxis />
                <Tooltip />
                <Line type="monotone" dataKey="detections" stroke="#8884d8" strokeWidth={2} />
                <Line type="monotone" dataKey="unique_species" stroke="#82ca9d" strokeWidth={2} />
              </LineChart>
            </ResponsiveContainer>
          </CardContent>
        </Card>
      </Grid>

      <Grid item xs={12} md={6}>
        <Card>
          <CardContent>
            <Typography variant="h6" gutterBottom>
              Species Distribution
            </Typography>
            <ResponsiveContainer width="100%" height={300}>
              <RechartsPieChart>
                <Pie
                  data={speciesData}
                  cx="50%"
                  cy="50%"
                  labelLine={false}
                  label={({ name, percent }) => `${name} ${(percent * 100).toFixed(0)}%`}
                  outerRadius={80}
                  fill="#8884d8"
                  dataKey="count"
                >
                  {speciesData.map((entry, index) => (
                    <Cell key={`cell-${index}`} fill={COLORS[index % COLORS.length]} />
                  ))}
                </Pie>
                <Tooltip />
              </RechartsPieChart>
            </ResponsiveContainer>
          </CardContent>
        </Card>
      </Grid>

      <Grid item xs={12} md={6}>
        <Card>
          <CardContent>
            <Typography variant="h6" gutterBottom>
              Biodiversity Metrics
            </Typography>
            <Box sx={{ mb: 2 }}>
              <Typography variant="body2" color="textSecondary">
                Species Count
              </Typography>
              <Typography variant="h4">
                {biodiversityMetrics.total_species || 0}
              </Typography>
            </Box>
            <Box sx={{ mb: 2 }}>
              <Typography variant="body2" color="textSecondary">
                Shannon Diversity Index
              </Typography>
              <Typography variant="h5">
                {biodiversityMetrics.shannon_index?.toFixed(2) || '0.00'}
              </Typography>
            </Box>
            <Box>
              <Typography variant="body2" color="textSecondary">
                Simpson Index
              </Typography>
              <Typography variant="h5">
                {biodiversityMetrics.simpson_index?.toFixed(2) || '0.00'}
              </Typography>
            </Box>
          </CardContent>
        </Card>
      </Grid>
    </Grid>
  );

  const renderActivityPatterns = () => (
    <Grid container spacing={3}>
      <Grid item xs={12}>
        <Card>
          <CardContent>
            <Typography variant="h6" gutterBottom>
              Daily Activity Patterns
            </Typography>
            <ResponsiveContainer width="100%" height={300}>
              <AreaChart data={activityPatterns}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="hour" />
                <YAxis />
                <Tooltip />
                <Area type="monotone" dataKey="activity" stroke="#8884d8" fill="#8884d8" />
              </AreaChart>
            </ResponsiveContainer>
          </CardContent>
        </Card>
      </Grid>
    </Grid>
  );

  const renderCameraPerformance = () => (
    <Grid container spacing={3}>
      <Grid item xs={12}>
        <Card>
          <CardContent>
            <Typography variant="h6" gutterBottom>
              Camera Performance Metrics
            </Typography>
            <TableContainer>
              <Table>
                <TableHead>
                  <TableRow>
                    <TableCell>Camera</TableCell>
                    <TableCell align="right">Uptime %</TableCell>
                    <TableCell align="right">Detections</TableCell>
                    <TableCell align="right">Battery %</TableCell>
                    <TableCell align="right">Status</TableCell>
                  </TableRow>
                </TableHead>
                <TableBody>
                  {cameraPerformance.map((camera) => (
                    <TableRow key={camera.camera}>
                      <TableCell component="th" scope="row">
                        {camera.camera}
                      </TableCell>
                      <TableCell align="right">
                        <Box sx={{ display: 'flex', alignItems: 'center' }}>
                          <LinearProgress
                            variant="determinate"
                            value={camera.uptime}
                            sx={{ width: 80, mr: 1 }}
                          />
                          {camera.uptime}%
                        </Box>
                      </TableCell>
                      <TableCell align="right">{camera.detections}</TableCell>
                      <TableCell align="right">
                        <Box sx={{ display: 'flex', alignItems: 'center' }}>
                          <LinearProgress
                            variant="determinate"
                            value={camera.battery}
                            color={camera.battery > 50 ? 'success' : camera.battery > 20 ? 'warning' : 'error'}
                            sx={{ width: 80, mr: 1 }}
                          />
                          {camera.battery}%
                        </Box>
                      </TableCell>
                      <TableCell align="right">
                        <Chip
                          label={camera.uptime > 95 ? 'Excellent' : camera.uptime > 90 ? 'Good' : 'Needs Attention'}
                          color={camera.uptime > 95 ? 'success' : camera.uptime > 90 ? 'primary' : 'warning'}
                          size="small"
                        />
                      </TableCell>
                    </TableRow>
                  ))}
                </TableBody>
              </Table>
            </TableContainer>
          </CardContent>
        </Card>
      </Grid>
    </Grid>
  );

  const renderConservationInsights = () => (
    <Grid container spacing={3}>
      <Grid item xs={12}>
        <Card>
          <CardContent>
            <Typography variant="h6" gutterBottom>
              Conservation Alerts
            </Typography>
            {conservationAlerts.length === 0 ? (
              <Typography variant="body2" color="textSecondary">
                No active conservation alerts
              </Typography>
            ) : (
              conservationAlerts.map((alert, index) => (
                <Box
                  key={index}
                  sx={{
                    p: 2,
                    mb: 1,
                    border: 1,
                    borderColor: alert.severity === 'critical' ? 'error.main' : 'warning.main',
                    borderRadius: 1,
                    bgcolor: alert.severity === 'critical' ? 'error.light' : 'warning.light',
                  }}
                >
                  <Typography variant="subtitle1" color={alert.severity === 'critical' ? 'error' : 'warning'}>
                    {alert.title}
                  </Typography>
                  <Typography variant="body2">
                    {alert.description}
                  </Typography>
                  <Typography variant="caption" color="textSecondary">
                    {new Date(alert.timestamp).toLocaleString()}
                  </Typography>
                </Box>
              ))
            )}
          </CardContent>
        </Card>
      </Grid>
    </Grid>
  );

  if (loading) {
    return (
      <Box sx={{ p: 3 }}>
        <LinearProgress />
        <Typography sx={{ mt: 2 }}>Loading analytics...</Typography>
      </Box>
    );
  }

  return (
    <Box sx={{ p: 3 }}>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Wildlife Analytics
        </Typography>
        
        <Box sx={{ display: 'flex', gap: 2 }}>
          <FormControl size="small" sx={{ minWidth: 120 }}>
            <InputLabel>Time Range</InputLabel>
            <Select
              value={timeRange}
              label="Time Range"
              onChange={(e) => setTimeRange(e.target.value)}
            >
              <MenuItem value="24hours">24 Hours</MenuItem>
              <MenuItem value="7days">7 Days</MenuItem>
              <MenuItem value="30days">30 Days</MenuItem>
              <MenuItem value="90days">90 Days</MenuItem>
              <MenuItem value="1year">1 Year</MenuItem>
            </Select>
          </FormControl>
          
          <Button
            variant="outlined"
            startIcon={<Refresh />}
            onClick={refreshAnalyticsData}
          >
            Refresh
          </Button>
          
          <Button
            variant="contained"
            startIcon={<Download />}
            onClick={() => handleExportData('all')}
          >
            Export Data
          </Button>
        </Box>
      </Box>

      <Box sx={{ borderBottom: 1, borderColor: 'divider', mb: 3 }}>
        <Tabs value={tabValue} onChange={(e, newValue) => setTabValue(newValue)}>
          <Tab label="Detection Trends" icon={<TrendingUp />} />
          <Tab label="Activity Patterns" icon={<Timeline />} />
          <Tab label="Camera Performance" icon={<Assessment />} />
          <Tab label="Conservation Insights" icon={<PieChart />} />
        </Tabs>
      </Box>

      <TabPanel value={tabValue} index={0}>
        {renderDetectionTrends()}
      </TabPanel>

      <TabPanel value={tabValue} index={1}>
        {renderActivityPatterns()}
      </TabPanel>

      <TabPanel value={tabValue} index={2}>
        {renderCameraPerformance()}
      </TabPanel>

      <TabPanel value={tabValue} index={3}>
        {renderConservationInsights()}
      </TabPanel>
    </Box>
  );
};

export default Analytics;