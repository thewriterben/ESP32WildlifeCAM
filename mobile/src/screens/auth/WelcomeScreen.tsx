import React from 'react';
import {View, StyleSheet, Image} from 'react-native';
import {Text, Button, useTheme} from 'react-native-paper';
import {SafeAreaView} from 'react-native-safe-area-context';
import {useNavigation} from '@react-navigation/native';

const WelcomeScreen: React.FC = () => {
  const theme = useTheme();
  const navigation = useNavigation();

  return (
    <SafeAreaView style={[styles.container, {backgroundColor: theme.colors.primary}]}>
      <View style={styles.content}>
        <View style={styles.logoContainer}>
          <Text variant="headlineLarge" style={[styles.title, {color: theme.colors.onPrimary}]}>
            ESP32 Wildlife CAM
          </Text>
          <Text variant="bodyLarge" style={[styles.subtitle, {color: theme.colors.onPrimary}]}>
            Monitor. Protect. Preserve.
          </Text>
        </View>

        <View style={styles.features}>
          <Text variant="bodyMedium" style={[styles.feature, {color: theme.colors.onPrimary}]}>
            🔍 Real-time wildlife monitoring
          </Text>
          <Text variant="bodyMedium" style={[styles.feature, {color: theme.colors.onPrimary}]}>
            🤖 AI-powered species identification
          </Text>
          <Text variant="bodyMedium" style={[styles.feature, {color: theme.colors.onPrimary}]}>
            📊 Advanced analytics and insights
          </Text>
          <Text variant="bodyMedium" style={[styles.feature, {color: theme.colors.onPrimary}]}>
            🌍 Multi-camera management
          </Text>
        </View>

        <View style={styles.actions}>
          <Button
            mode="contained"
            onPress={() => navigation.navigate('Login' as never)}
            style={[styles.button, {backgroundColor: theme.colors.surface}]}
            contentStyle={styles.buttonContent}
            labelStyle={{color: theme.colors.primary}}
          >
            Sign In
          </Button>
          
          <Button
            mode="outlined"
            onPress={() => navigation.navigate('Register' as never)}
            style={[styles.button, styles.outlinedButton]}
            contentStyle={styles.buttonContent}
            labelStyle={{color: theme.colors.onPrimary}}
          >
            Create Account
          </Button>
        </View>
      </View>
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
  content: {
    flex: 1,
    padding: 24,
    justifyContent: 'space-between',
  },
  logoContainer: {
    alignItems: 'center',
    marginTop: 60,
  },
  title: {
    textAlign: 'center',
    fontWeight: 'bold',
    marginBottom: 8,
  },
  subtitle: {
    textAlign: 'center',
    opacity: 0.8,
  },
  features: {
    alignItems: 'center',
  },
  feature: {
    marginBottom: 16,
    textAlign: 'center',
  },
  actions: {
    marginBottom: 40,
  },
  button: {
    marginBottom: 16,
    borderRadius: 8,
  },
  outlinedButton: {
    borderWidth: 2,
    borderColor: 'rgba(255, 255, 255, 0.5)',
  },
  buttonContent: {
    paddingVertical: 8,
  },
});

export default WelcomeScreen;