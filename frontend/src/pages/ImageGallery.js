import React, { useState, useEffect } from 'react';
import {
  Box,
  Grid,
  Card,
  CardMedia,
  CardContent,
  Typography,
  IconButton,
  Dialog,
  DialogContent,
  DialogTitle,
  DialogActions,
  Button,
  Chip,
  TextField,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  Pagination,
} from '@mui/material';
import {
  ZoomIn,
  Download,
  FilterList,
  Close,
} from '@mui/icons-material';

import cameraService from '../services/cameraService';
import { useAlert } from '../contexts/AlertContext';

const ImageGallery = () => {
  const [images, setImages] = useState([]);
  const [cameras, setCameras] = useState([]);
  const [loading, setLoading] = useState(true);
  const [selectedImage, setSelectedImage] = useState(null);
  const [filters, setFilters] = useState({
    camera_id: '',
    start_date: '',
    end_date: '',
  });
  const [pagination, setPagination] = useState({
    page: 1,
    per_page: 20,
    total: 0,
    pages: 0,
  });

  const { showAlert } = useAlert();

  useEffect(() => {
    loadCameras();
    loadImages();
  }, [pagination.page, filters]);

  const loadCameras = async () => {
    try {
      const data = await cameraService.getCameras();
      setCameras(data.cameras || []);
    } catch (error) {
      console.error('Failed to load cameras:', error);
    }
  };

  const loadImages = async () => {
    try {
      setLoading(true);
      
      // Mock implementation - replace with actual API call
      const mockImages = [
        {
          id: 1,
          filename: 'deer_001.jpg',
          camera_name: 'Forest Trail Camera 1',
          timestamp: '2023-10-15T14:35:00Z',
          detections: [
            { species: 'White-tailed Deer', confidence: 0.92 }
          ],
          thumbnail_url: '/api/images/1/thumbnail'
        },
        {
          id: 2,
          filename: 'bear_001.jpg',
          camera_name: 'Forest Trail Camera 1',
          timestamp: '2023-10-15T08:22:00Z',
          detections: [
            { species: 'Black Bear', confidence: 0.87 }
          ],
          thumbnail_url: '/api/images/2/thumbnail'
        },
        // Add more mock data as needed
      ];

      setImages(mockImages);
      setPagination(prev => ({
        ...prev,
        total: mockImages.length,
        pages: Math.ceil(mockImages.length / prev.per_page)
      }));
    } catch (error) {
      console.error('Failed to load images:', error);
      showAlert('Failed to load images', 'error');
    } finally {
      setLoading(false);
    }
  };

  const handleImageClick = (image) => {
    setSelectedImage(image);
  };

  const handleCloseDialog = () => {
    setSelectedImage(null);
  };

  const handleFilterChange = (e) => {
    setFilters({
      ...filters,
      [e.target.name]: e.target.value,
    });
    setPagination(prev => ({ ...prev, page: 1 }));
  };

  const handlePageChange = (event, value) => {
    setPagination(prev => ({ ...prev, page: value }));
  };

  const handleDownload = (image) => {
    // Create download link
    const link = document.createElement('a');
    link.href = `/api/images/${image.id}`;
    link.download = image.filename;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  };

  const getConfidenceColor = (confidence) => {
    if (confidence >= 0.8) return 'success';
    if (confidence >= 0.6) return 'warning';
    return 'error';
  };

  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" component="h1" gutterBottom>
        Image Gallery
      </Typography>

      {/* Filters */}
      <Card sx={{ p: 2, mb: 3 }}>
        <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
          <FilterList sx={{ mr: 1 }} />
          <Typography variant="h6">Filters</Typography>
        </Box>
        
        <Grid container spacing={2}>
          <Grid item xs={12} sm={6} md={3}>
            <FormControl fullWidth>
              <InputLabel>Camera</InputLabel>
              <Select
                name="camera_id"
                value={filters.camera_id}
                onChange={handleFilterChange}
                label="Camera"
              >
                <MenuItem value="">All Cameras</MenuItem>
                {cameras.map((camera) => (
                  <MenuItem key={camera.id} value={camera.id}>
                    {camera.name}
                  </MenuItem>
                ))}
              </Select>
            </FormControl>
          </Grid>
          
          <Grid item xs={12} sm={6} md={3}>
            <TextField
              fullWidth
              name="start_date"
              label="Start Date"
              type="date"
              value={filters.start_date}
              onChange={handleFilterChange}
              InputLabelProps={{ shrink: true }}
            />
          </Grid>
          
          <Grid item xs={12} sm={6} md={3}>
            <TextField
              fullWidth
              name="end_date"
              label="End Date"
              type="date"
              value={filters.end_date}
              onChange={handleFilterChange}
              InputLabelProps={{ shrink: true }}
            />
          </Grid>
        </Grid>
      </Card>

      {/* Image Grid */}
      <Grid container spacing={2}>
        {images.map((image) => (
          <Grid item xs={12} sm={6} md={4} lg={3} key={image.id}>
            <Card>
              <CardMedia
                component="img"
                height="200"
                image={`https://via.placeholder.com/300x200?text=${image.filename}`}
                alt={image.filename}
                sx={{ cursor: 'pointer' }}
                onClick={() => handleImageClick(image)}
              />
              <CardContent>
                <Typography variant="subtitle2" noWrap gutterBottom>
                  {image.filename}
                </Typography>
                
                <Typography variant="body2" color="text.secondary" gutterBottom>
                  {image.camera_name}
                </Typography>
                
                <Typography variant="caption" display="block" gutterBottom>
                  {new Date(image.timestamp).toLocaleString()}
                </Typography>
                
                {image.detections && image.detections.length > 0 && (
                  <Box sx={{ mb: 1 }}>
                    {image.detections.map((detection, index) => (
                      <Chip
                        key={index}
                        label={`${detection.species} (${(detection.confidence * 100).toFixed(0)}%)`}
                        size="small"
                        color={getConfidenceColor(detection.confidence)}
                        sx={{ mr: 0.5, mb: 0.5 }}
                      />
                    ))}
                  </Box>
                )}
                
                <Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
                  <IconButton 
                    size="small" 
                    onClick={() => handleImageClick(image)}
                    color="primary"
                  >
                    <ZoomIn />
                  </IconButton>
                  <IconButton 
                    size="small" 
                    onClick={() => handleDownload(image)}
                    color="primary"
                  >
                    <Download />
                  </IconButton>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        ))}
      </Grid>

      {/* Pagination */}
      {pagination.pages > 1 && (
        <Box sx={{ display: 'flex', justifyContent: 'center', mt: 3 }}>
          <Pagination
            count={pagination.pages}
            page={pagination.page}
            onChange={handlePageChange}
            color="primary"
          />
        </Box>
      )}

      {/* Image Detail Dialog */}
      <Dialog
        open={!!selectedImage}
        onClose={handleCloseDialog}
        maxWidth="lg"
        fullWidth
      >
        {selectedImage && (
          <>
            <DialogTitle>
              <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                <Typography variant="h6">{selectedImage.filename}</Typography>
                <IconButton onClick={handleCloseDialog}>
                  <Close />
                </IconButton>
              </Box>
            </DialogTitle>
            
            <DialogContent>
              <Grid container spacing={2}>
                <Grid item xs={12} md={8}>
                  <img
                    src={`https://via.placeholder.com/800x600?text=${selectedImage.filename}`}
                    alt={selectedImage.filename}
                    style={{ width: '100%', height: 'auto', borderRadius: 8 }}
                  />
                </Grid>
                
                <Grid item xs={12} md={4}>
                  <Typography variant="h6" gutterBottom>
                    Image Details
                  </Typography>
                  
                  <Typography variant="body2" gutterBottom>
                    <strong>Camera:</strong> {selectedImage.camera_name}
                  </Typography>
                  
                  <Typography variant="body2" gutterBottom>
                    <strong>Timestamp:</strong> {new Date(selectedImage.timestamp).toLocaleString()}
                  </Typography>
                  
                  <Typography variant="body2" gutterBottom>
                    <strong>Filename:</strong> {selectedImage.filename}
                  </Typography>
                  
                  {selectedImage.detections && selectedImage.detections.length > 0 && (
                    <>
                      <Typography variant="h6" sx={{ mt: 2, mb: 1 }}>
                        Wildlife Detections
                      </Typography>
                      {selectedImage.detections.map((detection, index) => (
                        <Box key={index} sx={{ mb: 1 }}>
                          <Chip
                            label={detection.species}
                            color={getConfidenceColor(detection.confidence)}
                            sx={{ mr: 1 }}
                          />
                          <Typography variant="body2" component="span">
                            Confidence: {(detection.confidence * 100).toFixed(1)}%
                          </Typography>
                        </Box>
                      ))}
                    </>
                  )}
                </Grid>
              </Grid>
            </DialogContent>
            
            <DialogActions>
              <Button onClick={() => handleDownload(selectedImage)} startIcon={<Download />}>
                Download
              </Button>
              <Button onClick={handleCloseDialog}>Close</Button>
            </DialogActions>
          </>
        )}
      </Dialog>
    </Box>
  );
};

export default ImageGallery;