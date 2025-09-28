import React from 'react';
import { Box, Typography, Paper } from '@mui/material';

const Settings = () => {
  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" component="h1" gutterBottom>
        Settings
      </Typography>
      <Paper sx={{ p: 3 }}>
        <Typography variant="body1">
          System settings coming soon. This will include:
        </Typography>
        <ul>
          <li>User profile management</li>
          <li>Notification preferences</li>
          <li>System configuration</li>
          <li>API keys and integrations</li>
          <li>Backup and restore</li>
        </ul>
      </Paper>
    </Box>
  );
};

export default Settings;