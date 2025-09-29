import React from 'react';
import {View, StyleSheet} from 'react-native';
import {Text, useTheme} from 'react-native-paper';
import {SafeAreaView} from 'react-native-safe-area-context';

const CameraListScreen: React.FC = () => {
  const theme = useTheme();

  return (
    <SafeAreaView style={[styles.container, {backgroundColor: theme.colors.background}]}>
      <View style={styles.content}>
        <Text variant="headlineMedium">Camera List</Text>
        <Text variant="bodyMedium">Camera management interface coming soon!</Text>
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
    padding: 16,
    justifyContent: 'center',
    alignItems: 'center',
  },
});

export default CameraListScreen;