import React, {useState} from 'react';
import {View, StyleSheet} from 'react-native';
import {Text, TextInput, Button, Card, useTheme, Snackbar} from 'react-native-paper';
import {SafeAreaView} from 'react-native-safe-area-context';
import {useNavigation} from '@react-navigation/native';
import authService from '@services/authService';

const ForgotPasswordScreen: React.FC = () => {
  const theme = useTheme();
  const navigation = useNavigation();
  
  const [email, setEmail] = useState('');
  const [isLoading, setIsLoading] = useState(false);
  const [message, setMessage] = useState('');
  const [isSuccess, setIsSuccess] = useState(false);

  const handleResetPassword = async () => {
    if (!email) return;

    setIsLoading(true);
    try {
      await authService.forgotPassword(email);
      setMessage('Password reset link sent to your email');
      setIsSuccess(true);
    } catch (error: any) {
      setMessage(error.response?.data?.message || 'Failed to send reset email');
      setIsSuccess(false);
    } finally {
      setIsLoading(false);
    }
  };

  return (
    <SafeAreaView style={[styles.container, {backgroundColor: theme.colors.background}]}>
      <View style={styles.content}>
        <View style={styles.header}>
          <Text variant="headlineMedium" style={[styles.title, {color: theme.colors.primary}]}>
            Reset Password
          </Text>
          <Text variant="bodyMedium" style={[styles.subtitle, {color: theme.colors.onSurfaceVariant}]}>
            Enter your email address and we'll send you a link to reset your password
          </Text>
        </View>

        <Card style={styles.card}>
          <Card.Content>
            <TextInput
              label="Email"
              value={email}
              onChangeText={setEmail}
              mode="outlined"
              style={styles.input}
              keyboardType="email-address"
              autoCapitalize="none"
              autoComplete="email"
            />

            <Button
              mode="contained"
              onPress={handleResetPassword}
              loading={isLoading}
              disabled={!email || isLoading}
              style={styles.button}
              contentStyle={styles.buttonContent}
            >
              Send Reset Link
            </Button>

            <Button
              mode="text"
              onPress={() => navigation.navigate('Login' as never)}
              style={styles.backButton}
            >
              Back to Sign In
            </Button>
          </Card.Content>
        </Card>
      </View>

      <Snackbar
        visible={!!message}
        onDismiss={() => setMessage('')}
        duration={4000}
        style={isSuccess ? {backgroundColor: theme.colors.primary} : undefined}
      >
        {message}
      </Snackbar>
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
    justifyContent: 'center',
  },
  header: {
    alignItems: 'center',
    marginBottom: 32,
  },
  title: {
    fontWeight: 'bold',
    marginBottom: 8,
  },
  subtitle: {
    textAlign: 'center',
  },
  card: {
    marginBottom: 24,
  },
  input: {
    marginBottom: 24,
  },
  button: {
    marginBottom: 16,
  },
  buttonContent: {
    paddingVertical: 8,
  },
  backButton: {},
});

export default ForgotPasswordScreen;