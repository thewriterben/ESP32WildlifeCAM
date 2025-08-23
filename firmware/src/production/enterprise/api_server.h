#ifndef API_SERVER_H
#define API_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "../../config.h"

/**
 * @brief RESTful API Server for Enterprise Integration
 * 
 * Provides comprehensive REST API for external system integration,
 * data access, and remote management of wildlife camera networks.
 */
class APIServer {
public:
    /**
     * @brief API endpoint types
     */
    enum class EndpointType {
        GET = 0,
        POST = 1,
        PUT = 2,
        DELETE = 3,
        PATCH = 4
    };

    /**
     * @brief API authentication methods
     */
    enum class AuthMethod {
        NONE = 0,
        API_KEY = 1,
        BEARER_TOKEN = 2,
        BASIC_AUTH = 3,
        OAUTH = 4
    };

    /**
     * @brief API configuration
     */
    struct APIConfig {
        uint16_t port;
        bool enable_cors;
        bool enable_authentication;
        AuthMethod auth_method;
        String api_key;
        uint32_t rate_limit_requests_per_minute;
        bool enable_logging;
        bool enable_ssl;
        String ssl_cert_path;
        String ssl_key_path;
    };

    /**
     * @brief API endpoint definition
     */
    struct APIEndpoint {
        String path;
        EndpointType method;
        String description;
        bool requires_auth;
        std::function<void(void)> handler;
        String content_type;
        std::vector<String> required_parameters;
    };

    /**
     * @brief Initialize API server
     * @param config API server configuration
     * @return true if initialization successful
     */
    static bool initialize(const APIConfig& config);

    /**
     * @brief Start API server
     * @return true if server started successfully
     */
    static bool start();

    /**
     * @brief Stop API server
     */
    static void stop();

    /**
     * @brief Process API server (call in main loop)
     */
    static void process();

    /**
     * @brief Register custom API endpoint
     * @param endpoint Endpoint definition
     * @return true if endpoint registered successfully
     */
    static bool registerEndpoint(const APIEndpoint& endpoint);

    /**
     * @brief Get API server statistics
     * @return JSON object with server statistics
     */
    static DynamicJsonDocument getServerStatistics();

    /**
     * @brief Set API authentication key
     * @param api_key New API key
     */
    static void setAPIKey(const String& api_key);

    /**
     * @brief Enable/disable API endpoint
     * @param path Endpoint path
     * @param enabled Enable/disable endpoint
     */
    static void setEndpointEnabled(const String& path, bool enabled);

private:
    static APIConfig config_;
    static WebServer* server_;
    static std::vector<APIEndpoint> endpoints_;
    static std::map<String, uint32_t> rate_limit_tracker_;
    static bool initialized_;
    static bool running_;
    
    // Built-in API endpoints
    static void handleStatus();
    static void handleSystemInfo();
    static void handleDetections();
    static void handleImages();
    static void handleConfiguration();
    static void handleAlerts();
    static void handleStatistics();
    static void handleHealthCheck();
    
    // Utility methods
    static bool authenticateRequest();
    static bool checkRateLimit(const String& client_ip);
    static void sendJSONResponse(int status_code, const DynamicJsonDocument& data);
    static void sendErrorResponse(int status_code, const String& message);
    static void logAPIRequest(const String& method, const String& path, const String& client_ip);
    static void setupCORS();
    static void registerBuiltinEndpoints();
};

#endif // API_SERVER_H