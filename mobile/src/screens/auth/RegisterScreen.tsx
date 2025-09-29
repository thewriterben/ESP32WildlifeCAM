import React, {useState} from 'react';
import {View, StyleSheet, KeyboardAvoidingView, Platform, ScrollView} from 'react-native';
import {Text, TextInput, Button, Card, useTheme, Snackbar} from 'react-native-paper';
import {SafeAreaView} from 'react-native-safe-area-context';
import {useNavigation} from '@react-navigation/native';
import {useDispatch, useSelector} from 'react-redux';
import {register} from '@store/slices/authSlice';
import {RootState, AppDispatch} from '@store/index';

const RegisterScreen: React.FC = () => {
  const theme = useTheme();
  const navigation = useNavigation();
  const dispatch = useDispatch<AppDispatch>();
  const {isLoading, error} = useSelector((state: RootState) => state.auth);

  const [formData, setFormData] = useState({
    username: '',
    email: '',
    password: '',
    confirmPassword: '',
    organizationCode: '',
  });
  const [showPassword, setShowPassword] = useState(false);
  const [showError, setShowError] = useState(false);

  const handleRegister = async () => {
    if (formData.password !== formData.confirmPassword) {
      setShowError(true);
      return;
    }

    try {
      await dispatch(register({
        username: formData.username,
        email: formData.email,
        password: formData.password,
        organization_code: formData.organizationCode || undefined,
      })).unwrap();
    } catch (err) {
      setShowError(true);
    }
  };

  const isFormValid = formData.username && formData.email && formData.password && 
                     formData.confirmPassword && formData.password === formData.confirmPassword;

  return (
    <SafeAreaView style={[styles.container, {backgroundColor: theme.colors.background}]}>
      <KeyboardAvoidingView 
        style={styles.keyboardAvoid}
        behavior={Platform.OS === 'ios' ? 'padding' : 'height'}
      >
        <ScrollView contentContainerStyle={styles.scrollContent}>
          <View style={styles.header}>
            <Text variant="headlineMedium" style={[styles.title, {color: theme.colors.primary}]}>
              Create Account
            </Text>
            <Text variant="bodyMedium" style={[styles.subtitle, {color: theme.colors.onSurfaceVariant}]}>
              Join the Wildlife CAM community
            </Text>
          </View>

          <Card style={styles.card}>
            <Card.Content>
              <TextInput
                label="Username"
                value={formData.username}
                onChangeText={(username) => setFormData({...formData, username})}
                mode="outlined"
                style={styles.input}
                autoCapitalize="none"
              />

              <TextInput
                label="Email"
                value={formData.email}
                onChangeText={(email) => setFormData({...formData, email})}
                mode="outlined"
                style={styles.input}
                keyboardType="email-address"
                autoCapitalize="none"
                autoComplete="email"
              />

              <TextInput
                label="Password"
                value={formData.password}
                onChangeText={(password) => setFormData({...formData, password})}
                mode="outlined"
                style={styles.input}
                secureTextEntry={!showPassword}
                autoCapitalize="none"
                right={
                  <TextInput.Icon
                    icon={showPassword ? 'eye-off' : 'eye'}
                    onPress={() => setShowPassword(!showPassword)}
                  />
                }
              />

              <TextInput
                label="Confirm Password"
                value={formData.confirmPassword}
                onChangeText={(confirmPassword) => setFormData({...formData, confirmPassword})}
                mode="outlined"
                style={styles.input}
                secureTextEntry={!showPassword}
                autoCapitalize="none"
                error={formData.confirmPassword && formData.password !== formData.confirmPassword}
              />

              <TextInput
                label="Organization Code (Optional)"
                value={formData.organizationCode}
                onChangeText={(organizationCode) => setFormData({...formData, organizationCode})}
                mode="outlined"
                style={styles.input}
                autoCapitalize="characters"
              />

              <Button
                mode="contained"
                onPress={handleRegister}
                loading={isLoading}
                disabled={!isFormValid || isLoading}
                style={styles.registerButton}
                contentStyle={styles.buttonContent}
              >
                Create Account
              </Button>
            </Card.Content>
          </Card>

          <View style={styles.footer}>
            <Text variant="bodyMedium" style={{color: theme.colors.onSurfaceVariant}}>
              Already have an account?{' '}
            </Text>
            <Button
              mode="text"
              onPress={() => navigation.navigate('Login' as never)}
              compact
            >
              Sign In
            </Button>
          </View>
        </ScrollView>
      </KeyboardAvoidingView>

      <Snackbar
        visible={showError && (!!error || formData.password !== formData.confirmPassword)}
        onDismiss={() => setShowError(false)}
        duration={4000}
      >
        {error || (formData.password !== formData.confirmPassword ? 'Passwords do not match' : 'Registration failed')}
      </Snackbar>
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
  keyboardAvoid: {
    flex: 1,
  },
  scrollContent: {
    flexGrow: 1,
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
    marginBottom: 16,
  },
  registerButton: {
    marginTop: 8,
  },
  buttonContent: {
    paddingVertical: 8,
  },
  footer: {
    flexDirection: 'row',
    justifyContent: 'center',
    alignItems: 'center',
  },
});

export default RegisterScreen;