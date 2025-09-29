import {DefaultTheme} from 'react-native-paper';

export const theme = {
  ...DefaultTheme,
  colors: {
    ...DefaultTheme.colors,
    primary: '#2c5530', // Forest green
    primaryVariant: '#1e3a21',
    secondary: '#4a7856',
    secondaryVariant: '#3d6047',
    background: '#f5f7f5',
    surface: '#ffffff',
    error: '#d32f2f',
    warning: '#ff9800',
    success: '#4caf50',
    info: '#2196f3',
    onPrimary: '#ffffff',
    onSecondary: '#ffffff',
    onBackground: '#1c1c1c',
    onSurface: '#1c1c1c',
    onError: '#ffffff',
    text: '#1c1c1c',
    placeholder: '#666666',
    disabled: '#cccccc',
    backdrop: 'rgba(0, 0, 0, 0.5)',
  },
  spacing: {
    xs: 4,
    sm: 8,
    md: 16,
    lg: 24,
    xl: 32,
    xxl: 48,
  },
  borderRadius: {
    sm: 4,
    md: 8,
    lg: 12,
    xl: 16,
    round: 50,
  },
  typography: {
    h1: {
      fontSize: 28,
      fontWeight: '700',
      lineHeight: 34,
    },
    h2: {
      fontSize: 24,
      fontWeight: '600',
      lineHeight: 30,
    },
    h3: {
      fontSize: 20,
      fontWeight: '600',
      lineHeight: 26,
    },
    h4: {
      fontSize: 18,
      fontWeight: '500',
      lineHeight: 24,
    },
    h5: {
      fontSize: 16,
      fontWeight: '500',
      lineHeight: 22,
    },
    h6: {
      fontSize: 14,
      fontWeight: '500',
      lineHeight: 20,
    },
    body1: {
      fontSize: 16,
      fontWeight: '400',
      lineHeight: 22,
    },
    body2: {
      fontSize: 14,
      fontWeight: '400',
      lineHeight: 20,
    },
    caption: {
      fontSize: 12,
      fontWeight: '400',
      lineHeight: 16,
    },
    button: {
      fontSize: 14,
      fontWeight: '500',
      lineHeight: 20,
      textTransform: 'uppercase',
    },
  },
  shadows: {
    sm: {
      shadowColor: '#000',
      shadowOffset: {
        width: 0,
        height: 1,
      },
      shadowOpacity: 0.22,
      shadowRadius: 2.22,
      elevation: 3,
    },
    md: {
      shadowColor: '#000',
      shadowOffset: {
        width: 0,
        height: 2,
      },
      shadowOpacity: 0.25,
      shadowRadius: 3.84,
      elevation: 5,
    },
    lg: {
      shadowColor: '#000',
      shadowOffset: {
        width: 0,
        height: 4,
      },
      shadowOpacity: 0.30,
      shadowRadius: 4.65,
      elevation: 8,
    },
  },
};

export const darkTheme = {
  ...theme,
  colors: {
    ...theme.colors,
    primary: '#4a7856',
    primaryVariant: '#3d6047',
    background: '#121212',
    surface: '#1e1e1e',
    onBackground: '#ffffff',
    onSurface: '#ffffff',
    text: '#ffffff',
    placeholder: '#aaaaaa',
    disabled: '#555555',
  },
};