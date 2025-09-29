import React, {useEffect} from 'react';
import {View, StyleSheet, ScrollView, RefreshControl} from 'react-native';
import {Text, Card, useTheme, IconButton} from 'react-native-paper';
import {SafeAreaView} from 'react-native-safe-area-context';
import {useDispatch, useSelector} from 'react-redux';
import {useNavigation} from '@react-navigation/native';
import {RootState, AppDispatch} from '@store/index';
import {fetchDashboardData} from '@store/slices/analyticsSlice';
import {fetchAlerts} from '@store/slices/alertSlice';
import LoadingScreen from '@components/LoadingScreen';

const DashboardScreen: React.FC = () => {
  const theme = useTheme();
  const navigation = useNavigation();
  const dispatch = useDispatch<AppDispatch>();
  
  const {dashboardData, isLoading} = useSelector((state: RootState) => state.analytics);
  const {unreadCount} = useSelector((state: RootState) => state.alerts);
  const {user, organization} = useSelector((state: RootState) => state.auth);

  const [refreshing, setRefreshing] = React.useState(false);

  useEffect(() => {
    loadDashboardData();
  }, []);

  const loadDashboardData = async () => {
    try {
      await Promise.all([
        dispatch(fetchDashboardData('7d')),
        dispatch(fetchAlerts({limit: 10})),
      ]);
    } catch (error) {
      console.error('Error loading dashboard data:', error);
    }
  };

  const handleRefresh = async () => {
    setRefreshing(true);
    await loadDashboardData();
    setRefreshing(false);
  };

  if (isLoading && !dashboardData) {
    return <LoadingScreen message="Loading dashboard..." />;
  }

  return (
    <SafeAreaView style={[styles.container, {backgroundColor: theme.colors.background}]}>
      <View style={styles.header}>
        <View style={styles.headerContent}>
          <Text variant="headlineSmall" style={{color: theme.colors.onBackground}}>
            Welcome back, {user?.username}
          </Text>
          {organization && (
            <Text variant="bodyMedium" style={{color: theme.colors.onSurfaceVariant}}>
              {organization.name}
            </Text>
          )}
        </View>
        <IconButton
          icon="bell"
          size={24}
          onPress={() => navigation.navigate('Notifications' as never)}
          style={unreadCount > 0 ? {backgroundColor: theme.colors.errorContainer} : undefined}
        />
      </View>

      <ScrollView
        contentContainerStyle={styles.scrollContent}
        refreshControl={
          <RefreshControl refreshing={refreshing} onRefresh={handleRefresh} />
        }
      >
        {/* Summary Cards */}
        <View style={styles.summaryGrid}>
          <Card style={[styles.summaryCard, styles.halfWidth]}>
            <Card.Content>
              <Text variant="bodySmall" style={{color: theme.colors.onSurfaceVariant}}>
                Total Cameras
              </Text>
              <Text variant="headlineMedium" style={{color: theme.colors.primary}}>
                {dashboardData?.summary.total_cameras || 0}
              </Text>
              <Text variant="bodySmall" style={{color: theme.colors.primary}}>
                {dashboardData?.summary.online_cameras || 0} online
              </Text>
            </Card.Content>
          </Card>

          <Card style={[styles.summaryCard, styles.halfWidth]}>
            <Card.Content>
              <Text variant="bodySmall" style={{color: theme.colors.onSurfaceVariant}}>
                Recent Detections
              </Text>
              <Text variant="headlineMedium" style={{color: theme.colors.secondary}}>
                {dashboardData?.summary.recent_detections || 0}
              </Text>
              <Text variant="bodySmall" style={{color: theme.colors.onSurfaceVariant}}>
                Last 24 hours
              </Text>
            </Card.Content>
          </Card>

          <Card style={[styles.summaryCard, styles.halfWidth]}>
            <Card.Content>
              <Text variant="bodySmall" style={{color: theme.colors.onSurfaceVariant}}>
                Species Count
              </Text>
              <Text variant="headlineMedium" style={{color: theme.colors.tertiary}}>
                {dashboardData?.summary.species_count || 0}
              </Text>
              <Text variant="bodySmall" style={{color: theme.colors.onSurfaceVariant}}>
                Unique species
              </Text>
            </Card.Content>
          </Card>

          <Card style={[styles.summaryCard, styles.halfWidth]}>
            <Card.Content>
              <Text variant="bodySmall" style={{color: theme.colors.onSurfaceVariant}}>
                Conservation Alerts
              </Text>
              <Text variant="headlineMedium" style={{
                color: dashboardData?.summary.conservation_alerts ? theme.colors.error : theme.colors.onSurfaceVariant
              }}>
                {dashboardData?.summary.conservation_alerts || 0}
              </Text>
              <Text variant="bodySmall" style={{color: theme.colors.onSurfaceVariant}}>
                Active alerts
              </Text>
            </Card.Content>
          </Card>
        </View>

        {/* Recent Activity */}
        <Card style={styles.activityCard}>
          <Card.Title title="Recent Activity" />
          <Card.Content>
            {dashboardData?.recent_detections && dashboardData.recent_detections.length > 0 ? (
              dashboardData.recent_detections.slice(0, 5).map((detection, index) => (
                <View key={detection.id} style={styles.activityItem}>
                  <Text variant="bodyMedium">
                    {detection.species?.name || 'Unknown Species'}
                  </Text>
                  <Text variant="bodySmall" style={{color: theme.colors.onSurfaceVariant}}>
                    {Math.round(detection.confidence * 100)}% confidence
                  </Text>
                </View>
              ))
            ) : (
              <Text variant="bodyMedium" style={{color: theme.colors.onSurfaceVariant}}>
                No recent detections
              </Text>
            )}
          </Card.Content>
        </Card>

        {/* Quick Actions */}
        <Card style={styles.actionsCard}>
          <Card.Title title="Quick Actions" />
          <Card.Content>
            <View style={styles.actionButtons}>
              <Text variant="bodyMedium">Coming soon: Quick camera controls, species identification, and more!</Text>
            </View>
          </Card.Content>
        </Card>
      </ScrollView>
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
  header: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    padding: 16,
    borderBottomWidth: 1,
    borderBottomColor: 'rgba(0,0,0,0.1)',
  },
  headerContent: {
    flex: 1,
  },
  scrollContent: {
    padding: 16,
  },
  summaryGrid: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    marginBottom: 16,
    gap: 8,
  },
  summaryCard: {
    marginBottom: 8,
  },
  halfWidth: {
    width: '48.5%',
  },
  activityCard: {
    marginBottom: 16,
  },
  activityItem: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    paddingVertical: 8,
    borderBottomWidth: 1,
    borderBottomColor: 'rgba(0,0,0,0.1)',
  },
  actionsCard: {
    marginBottom: 16,
  },
  actionButtons: {
    padding: 16,
  },
});

export default DashboardScreen;