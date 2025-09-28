import React from 'react';
import { Box, Typography, Paper } from '@mui/material';

const CameraDetail = () => {
  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" component="h1" gutterBottom>
        Camera Details
      </Typography>
      <Paper sx={{ p: 3 }}>
        <Typography variant="body1">
          Detailed camera view coming soon. This will include:
        </Typography>
        <ul>
          <li>Live camera feed</li>
          <li>Recent captures</li>
          <li>Configuration settings</li>
          <li>Performance metrics</li>
          <li>Remote control capabilities</li>
        </ul>
      </Paper>
    </Box>
  );
};

export default CameraDetail;