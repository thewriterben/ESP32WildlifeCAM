import React from 'react';
import {createBottomTabNavigator} from '@react-navigation/bottom-tabs';
import {createStackNavigator} from '@react-navigation/stack';
import Icon from 'react-native-vector-icons/MaterialIcons';
import {useTheme} from 'react-native-paper';
import {RootStackParamList} from '@types/index';

// Screen imports
import DashboardScreen from '@screens/dashboard/DashboardScreen';
import CameraListScreen from '@screens/cameras/CameraListScreen';
import CameraDetailScreen from '@screens/cameras/CameraDetailScreen';
import CameraConfigurationScreen from '@screens/cameras/CameraConfigurationScreen';
import SpeciesGalleryScreen from '@screens/species/SpeciesGalleryScreen';
import SpeciesDetailScreen from '@screens/species/SpeciesDetailScreen';
import ImageGalleryScreen from '@screens/images/ImageGalleryScreen';
import ImageDetailScreen from '@screens/images/ImageDetailScreen';
import AnalyticsScreen from '@screens/analytics/AnalyticsScreen';
import SettingsScreen from '@screens/settings/SettingsScreen';
import ProfileScreen from '@screens/settings/ProfileScreen';
import NotificationsScreen from '@screens/notifications/NotificationsScreen';
import OrganizationSettingsScreen from '@screens/settings/OrganizationSettingsScreen';

const Tab = createBottomTabNavigator();
const Stack = createStackNavigator<RootStackParamList>();

const DashboardStack = () => (
  <Stack.Navigator>
    <Stack.Screen 
      name="Dashboard" 
      component={DashboardScreen}
      options={{title: 'Wildlife Monitor'}}
    />
    <Stack.Screen 
      name="Notifications" 
      component={NotificationsScreen}
      options={{title: 'Notifications'}}
    />
  </Stack.Navigator>
);

const CameraStack = () => (
  <Stack.Navigator>
    <Stack.Screen 
      name="CameraList" 
      component={CameraListScreen}
      options={{title: 'My Cameras'}}
    />
    <Stack.Screen 
      name="CameraDetail" 
      component={CameraDetailScreen}
      options={({route}) => ({title: route.params?.cameraName || 'Camera Details'})}
    />
    <Stack.Screen 
      name="CameraConfiguration" 
      component={CameraConfigurationScreen}
      options={{title: 'Camera Settings'}}
    />
  </Stack.Navigator>
);

const SpeciesStack = () => (
  <Stack.Navigator>
    <Stack.Screen 
      name="SpeciesGallery" 
      component={SpeciesGalleryScreen}
      options={{title: 'Species Library'}}
    />
    <Stack.Screen 
      name="SpeciesDetail" 
      component={SpeciesDetailScreen}
      options={({route}) => ({title: route.params?.speciesName || 'Species Details'})}
    />
  </Stack.Navigator>
);

const ImageStack = () => (
  <Stack.Navigator>
    <Stack.Screen 
      name="ImageGallery" 
      component={ImageGalleryScreen}
      options={{title: 'Image Gallery'}}
    />
    <Stack.Screen 
      name="ImageDetail" 
      component={ImageDetailScreen}
      options={{title: 'Image Details'}}
    />
  </Stack.Navigator>
);

const AnalyticsStack = () => (
  <Stack.Navigator>
    <Stack.Screen 
      name="Analytics" 
      component={AnalyticsScreen}
      options={{title: 'Analytics'}}
    />
  </Stack.Navigator>
);

const SettingsStack = () => (
  <Stack.Navigator>
    <Stack.Screen 
      name="Settings" 
      component={SettingsScreen}
      options={{title: 'Settings'}}
    />
    <Stack.Screen 
      name="Profile" 
      component={ProfileScreen}
      options={{title: 'Profile'}}
    />
    <Stack.Screen 
      name="OrganizationSettings" 
      component={OrganizationSettingsScreen}
      options={{title: 'Organization Settings'}}
    />
  </Stack.Navigator>
);

const MainNavigator: React.FC = () => {
  const theme = useTheme();

  return (
    <Tab.Navigator
      screenOptions={({route}) => ({
        tabBarIcon: ({focused, color, size}) => {
          let iconName: string;

          switch (route.name) {
            case 'DashboardTab':
              iconName = 'dashboard';
              break;
            case 'CamerasTab':
              iconName = 'photo-camera';
              break;
            case 'SpeciesTab':
              iconName = 'pets';
              break;
            case 'ImagesTab':
              iconName = 'photo-library';
              break;
            case 'AnalyticsTab':
              iconName = 'analytics';
              break;
            case 'SettingsTab':
              iconName = 'settings';
              break;
            default:
              iconName = 'help';
          }

          return <Icon name={iconName} size={size} color={color} />;
        },
        tabBarActiveTintColor: theme.colors.primary,
        tabBarInactiveTintColor: 'gray',
        headerShown: false,
        tabBarStyle: {
          backgroundColor: theme.colors.surface,
          borderTopWidth: 1,
          borderTopColor: theme.colors.outline,
        },
      })}
    >
      <Tab.Screen 
        name="DashboardTab" 
        component={DashboardStack}
        options={{tabBarLabel: 'Dashboard'}}
      />
      <Tab.Screen 
        name="CamerasTab" 
        component={CameraStack}
        options={{tabBarLabel: 'Cameras'}}
      />
      <Tab.Screen 
        name="SpeciesTab" 
        component={SpeciesStack}
        options={{tabBarLabel: 'Species'}}
      />
      <Tab.Screen 
        name="ImagesTab" 
        component={ImageStack}
        options={{tabBarLabel: 'Images'}}
      />
      <Tab.Screen 
        name="AnalyticsTab" 
        component={AnalyticsStack}
        options={{tabBarLabel: 'Analytics'}}
      />
      <Tab.Screen 
        name="SettingsTab" 
        component={SettingsStack}
        options={{tabBarLabel: 'Settings'}}
      />
    </Tab.Navigator>
  );
};

export default MainNavigator;