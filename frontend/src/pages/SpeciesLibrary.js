import React from 'react';
import { Box, Typography, Paper } from '@mui/material';

const SpeciesLibrary = () => {
  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" component="h1" gutterBottom>
        Species Library
      </Typography>
      <Paper sx={{ p: 3 }}>
        <Typography variant="body1">
          Species identification library coming soon. This will include:
        </Typography>
        <ul>
          <li>Complete species database</li>
          <li>Conservation status information</li>
          <li>Identification guides</li>
          <li>Behavioral patterns</li>
          <li>Distribution maps</li>
        </ul>
      </Paper>
    </Box>
  );
};

export default SpeciesLibrary;