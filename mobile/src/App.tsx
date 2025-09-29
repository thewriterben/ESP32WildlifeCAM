/**
 * ESP32 Wildlife CAM Mobile App v3.1
 * React Native TypeScript Implementation
 */

import React, {useEffect} from 'react';
import {Provider} from 'react-redux';
import {PersistGate} from 'redux-persist/integration/react';
import {NavigationContainer} from '@react-navigation/native';
import {Provider as PaperProvider} from 'react-native-paper';
import {SafeAreaProvider} from 'react-native-safe-area-context';
import {StatusBar} from 'react-native';
import 'react-native-gesture-handler';

import {store, persistor} from '@store/index';
import AppNavigator from '@navigation/AppNavigator';
import {theme} from '@utils/theme';
import LoadingScreen from '@components/LoadingScreen';
import ErrorBoundary from '@components/ErrorBoundary';
import {initializeApp} from '@services/appService';

const App: React.FC = () => {
  useEffect(() => {
    initializeApp();
  }, []);

  return (
    <ErrorBoundary>
      <Provider store={store}>
        <PersistGate loading={<LoadingScreen />} persistor={persistor}>
          <PaperProvider theme={theme}>
            <SafeAreaProvider>
              <NavigationContainer>
                <StatusBar
                  barStyle="light-content"
                  backgroundColor={theme.colors.primary}
                />
                <AppNavigator />
              </NavigationContainer>
            </SafeAreaProvider>
          </PaperProvider>
        </PersistGate>
      </Provider>
    </ErrorBoundary>
  );
};

export default App;