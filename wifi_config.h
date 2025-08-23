/**
 * @file wifi_config.h
 * @brief WiFi and connectivity configuration template
 * 
 * This file contains WiFi credentials and network settings for the
 * ESP32 Wildlife Camera system. Copy this file and update with your
 * specific network configuration.
 * 
 * Security Note: Never commit this file with real credentials to version control.
 * Add 'wifi_config.h' to your .gitignore file.
 */

#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// ===========================
// WiFi CONFIGURATION
// ===========================

// Primary WiFi network
#define WIFI_SSID "YourWiFiNetwork"
#define WIFI_PASSWORD "YourWiFiPassword"

// Backup WiFi network (optional)
#define WIFI_SSID_BACKUP "BackupNetwork"
#define WIFI_PASSWORD_BACKUP "BackupPassword"

// WiFi connection settings
#define WIFI_CONNECT_TIMEOUT_MS 20000   // 20 seconds timeout
#define WIFI_RETRY_ATTEMPTS 3           // Number of connection retries
#define WIFI_RETRY_DELAY_MS 5000        // Delay between retries

// Access Point settings (for configuration mode)
#define AP_SSID "WildlifeCAM_Config"
#define AP_PASSWORD "wildlife123"
#define AP_CHANNEL 6
#define AP_MAX_CONNECTIONS 2

// ===========================
// TIME SYNCHRONIZATION
// ===========================

// NTP servers for time synchronization
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.nist.gov"
#define NTP_SERVER_3 "time.google.com"

// Timezone configuration
#define TIMEZONE_OFFSET_HOURS 0         // UTC offset in hours
#define DAYLIGHT_SAVINGS_OFFSET 3600    // Daylight savings offset in seconds

// ===========================
// REMOTE MONITORING
// ===========================

// MQTT broker settings (optional)
#define MQTT_ENABLED false
#define MQTT_BROKER "your-mqtt-broker.com"
#define MQTT_PORT 1883
#define MQTT_USERNAME "wildlife_cam"
#define MQTT_PASSWORD "mqtt_password"
#define MQTT_CLIENT_ID "esp32_wildlife_cam"

// MQTT topics
#define MQTT_TOPIC_STATUS "wildlife/camera/status"
#define MQTT_TOPIC_IMAGES "wildlife/camera/images"
#define MQTT_TOPIC_MOTION "wildlife/camera/motion"
#define MQTT_TOPIC_POWER "wildlife/camera/power"
#define MQTT_TOPIC_CONFIG "wildlife/camera/config"

// ===========================
// CLOUD STORAGE (OPTIONAL)
// ===========================

// Enable cloud upload
#define CLOUD_UPLOAD_ENABLED false

// Cloud service selection (uncomment one)
// #define USE_AWS_S3
// #define USE_GOOGLE_CLOUD
// #define USE_AZURE_BLOB
// #define USE_CUSTOM_SERVER

// AWS S3 configuration
#ifdef USE_AWS_S3
#define AWS_S3_BUCKET "your-wildlife-bucket"
#define AWS_ACCESS_KEY_ID "your-access-key"
#define AWS_SECRET_ACCESS_KEY "your-secret-key"
#define AWS_REGION "us-west-2"
#endif

// Custom server configuration
#ifdef USE_CUSTOM_SERVER
#define UPLOAD_SERVER_URL "https://your-server.com/upload"
#define UPLOAD_API_KEY "your-api-key"
#define UPLOAD_USERNAME "username"
#define UPLOAD_PASSWORD "password"
#endif

// ===========================
// WEB INTERFACE
// ===========================

// Web server settings
#define WEB_SERVER_PORT 80
#define WEB_USERNAME "admin"
#define WEB_PASSWORD "wildlife2024"

// Enable HTTPS (requires SSL certificates)
#define ENABLE_HTTPS false
#define HTTPS_PORT 443

// ===========================
// NOTIFICATION SETTINGS
// ===========================

// Email notifications (requires SMTP server)
#define EMAIL_ENABLED false
#define SMTP_SERVER "smtp.gmail.com"
#define SMTP_PORT 587
#define SMTP_USERNAME "your-email@gmail.com"
#define SMTP_PASSWORD "your-app-password"
#define NOTIFICATION_EMAIL "recipient@example.com"

// Push notifications (optional service)
#define PUSH_NOTIFICATIONS_ENABLED false
#define PUSHOVER_TOKEN "your-pushover-token"
#define PUSHOVER_USER "your-pushover-user"

// ===========================
// TELEMETRY AND MONITORING
// ===========================

// Remote telemetry
#define TELEMETRY_ENABLED false
#define TELEMETRY_SERVER "telemetry.your-domain.com"
#define TELEMETRY_INTERVAL_S 300        // 5 minutes

// Device identification
#define DEVICE_NAME "WildlifeCAM_01"
#define DEVICE_LOCATION "Forest_Station_A"
#define DEVICE_OWNER "Wildlife_Research_Team"

// ===========================
// SECURITY SETTINGS
// ===========================

// Enable WPA2 Enterprise (for enterprise networks)
#define WPA2_ENTERPRISE_ENABLED false
#define WPA2_IDENTITY "device_identity"
#define WPA2_USERNAME "device_username"
#define WPA2_PASSWORD "device_password"

// Certificate validation
#define VALIDATE_SSL_CERTIFICATES true

// Firmware update settings
#define OTA_UPDATE_ENABLED true
#define OTA_UPDATE_URL "https://updates.your-domain.com/firmware"
#define OTA_UPDATE_CHECK_INTERVAL_H 24  // Check every 24 hours

// ===========================
// ADVANCED NETWORK SETTINGS
// ===========================

// Static IP configuration (optional)
#define USE_STATIC_IP false
#define STATIC_IP "192.168.1.100"
#define GATEWAY_IP "192.168.1.1"
#define SUBNET_MASK "255.255.255.0"
#define DNS_PRIMARY "8.8.8.8"
#define DNS_SECONDARY "8.8.4.4"

// WiFi power management
#define WIFI_POWER_SAVE_MODE WIFI_PS_MIN_MODEM  // Power saving mode
#define WIFI_TX_POWER WIFI_POWER_19_5dBm        // Transmission power

// Connection quality monitoring
#define RSSI_THRESHOLD -70                      // Minimum signal strength
#define CONNECTION_CHECK_INTERVAL_S 60          // Check connection every minute

// ===========================
// EXAMPLE CONFIGURATIONS
// ===========================

#if 0  // Set to 1 to enable example configurations

// Example: Home network
#define WIFI_SSID "HomeNetwork"
#define WIFI_PASSWORD "homepassword123"

// Example: Research station
#define WIFI_SSID "ResearchStation_WiFi"
#define WIFI_PASSWORD "research2024!"
#define MQTT_ENABLED true
#define MQTT_BROKER "192.168.1.50"
#define TELEMETRY_ENABLED true

// Example: Remote location with cellular
#define WIFI_SSID ""  // No WiFi available
#define USE_CELLULAR_BACKUP true
#define CELLULAR_APN "internet"

#endif

// ===========================
// VALIDATION AND WARNINGS
// ===========================

// Compile-time validation
#if defined(WIFI_SSID) && (strlen(WIFI_SSID) == 0)
#warning "WiFi SSID not configured - WiFi will be disabled"
#endif

#if MQTT_ENABLED && !defined(MQTT_BROKER)
#error "MQTT enabled but broker not configured"
#endif

#if CLOUD_UPLOAD_ENABLED && !defined(USE_AWS_S3) && !defined(USE_GOOGLE_CLOUD) && !defined(USE_AZURE_BLOB) && !defined(USE_CUSTOM_SERVER)
#error "Cloud upload enabled but no service configured"
#endif

// Security warnings
#if defined(WIFI_PASSWORD) && (strlen(WIFI_PASSWORD) < 8)
#warning "WiFi password is less than 8 characters - consider using a stronger password"
#endif

#if defined(WEB_PASSWORD) && (strcmp(WEB_PASSWORD, "wildlife2024") == 0)
#warning "Using default web password - change for production use"
#endif

#endif // WIFI_CONFIG_H

/**
 * USAGE INSTRUCTIONS:
 * 
 * 1. Copy this file to your project directory
 * 2. Update the WiFi credentials with your network information
 * 3. Configure optional services as needed (MQTT, cloud storage, etc.)
 * 4. Add 'wifi_config.h' to your .gitignore file
 * 5. Compile and upload to your ESP32 device
 * 
 * SECURITY NOTES:
 * 
 * - Never commit real credentials to version control
 * - Use strong passwords for all accounts
 * - Enable HTTPS for production deployments
 * - Regularly update passwords and API keys
 * - Monitor device access logs
 * 
 * TROUBLESHOOTING:
 * 
 * - If WiFi connection fails, check SSID and password
 * - Verify signal strength at installation location
 * - Check firewall settings for MQTT and web server ports
 * - Ensure time synchronization is working for SSL certificates
 */