import React from 'react';
import { Box, Typography, Paper } from '@mui/material';

const Analytics = () => {
  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" component="h1" gutterBottom>
        Analytics
      </Typography>
      <Paper sx={{ p: 3 }}>
        <Typography variant="body1">
          Advanced analytics dashboard coming soon. This will include:
        </Typography>
        <ul>
          <li>Species population trends</li>
          <li>Activity pattern analysis</li>
          <li>Camera performance metrics</li>
          <li>Conservation insights</li>
          <li>Export capabilities</li>
        </ul>
      </Paper>
    </Box>
  );
};

export default Analytics;