import React, {Component, ReactNode} from 'react';
import {View, StyleSheet} from 'react-native';
import {Text, Button, Card} from 'react-native-paper';

interface Props {
  children: ReactNode;
}

interface State {
  hasError: boolean;
  error?: Error;
}

class ErrorBoundary extends Component<Props, State> {
  constructor(props: Props) {
    super(props);
    this.state = {hasError: false};
  }

  static getDerivedStateFromError(error: Error): State {
    return {hasError: true, error};
  }

  componentDidCatch(error: Error, errorInfo: any) {
    console.error('Error Boundary caught an error:', error, errorInfo);
    // TODO: Log error to crash reporting service
  }

  handleRetry = () => {
    this.setState({hasError: false, error: undefined});
  };

  render() {
    if (this.state.hasError) {
      return (
        <View style={styles.container}>
          <Card style={styles.card}>
            <Card.Content>
              <Text variant="headlineSmall" style={styles.title}>
                Oops! Something went wrong
              </Text>
              <Text variant="bodyMedium" style={styles.message}>
                The app encountered an unexpected error. Please try again.
              </Text>
              {__DEV__ && this.state.error && (
                <Text variant="bodySmall" style={styles.errorDetails}>
                  {this.state.error.toString()}
                </Text>
              )}
            </Card.Content>
            <Card.Actions>
              <Button mode="contained" onPress={this.handleRetry}>
                Try Again
              </Button>
            </Card.Actions>
          </Card>
        </View>
      );
    }

    return this.props.children;
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    padding: 20,
    backgroundColor: '#f5f5f5',
  },
  card: {
    width: '100%',
    maxWidth: 400,
  },
  title: {
    textAlign: 'center',
    marginBottom: 16,
  },
  message: {
    textAlign: 'center',
    marginBottom: 16,
  },
  errorDetails: {
    textAlign: 'center',
    color: 'red',
    fontFamily: 'monospace',
    backgroundColor: '#f0f0f0',
    padding: 8,
    borderRadius: 4,
  },
});

export default ErrorBoundary;