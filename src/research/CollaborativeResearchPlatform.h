/**
 * @file CollaborativeResearchPlatform.h
 * @brief Real-time Collaborative Research Platform for ESP32 Wildlife CAM
 * @author thewriterben
 * @date 2025-01-01
 * @version 3.2.0
 * 
 * Implements a comprehensive real-time collaborative research platform with
 * multi-user support, live data streaming, notification systems, and
 * collaborative annotation tools for wildlife researchers.
 */

#ifndef COLLABORATIVE_RESEARCH_PLATFORM_H
#define COLLABORATIVE_RESEARCH_PLATFORM_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebSocketsServer.h>
#include <vector>
#include <map>
#include "../../include/blockchain/blockchain_types.h"

/**
 * @brief Real-time event types for wildlife monitoring
 */
enum WildlifeEventType {
    EVENT_ANIMAL_DETECTED,               // Animal detected by camera
    EVENT_RARE_SPECIES,                  // Rare species detected
    EVENT_BEHAVIORAL_CHANGE,             // Behavioral pattern change detected
    EVENT_ENVIRONMENTAL_ALERT,           // Environmental condition alert
    EVENT_SYSTEM_STATUS,                 // System status update
    EVENT_RESEARCHER_ANNOTATION,         // Researcher added annotation
    EVENT_DATA_VALIDATION,               // Data validation event
    EVENT_CAMERA_MALFUNCTION,            // Camera hardware issue
    EVENT_POWER_ALERT,                   // Power system alert
    EVENT_NETWORK_STATUS                 // Network connectivity change
};

/**
 * @brief User roles for research platform access control
 */
enum ResearcherRole {
    ROLE_FIELD_RESEARCHER = 1,           // Field researcher with basic access
    ROLE_DATA_ANALYST = 2,               // Data analyst with analysis tools
    ROLE_PROJECT_COORDINATOR = 3,        // Project coordinator with management access
    ROLE_SYSTEM_ADMINISTRATOR = 4,       // Full system administrative access
    ROLE_GUEST_OBSERVER = 5              // Read-only guest access
};

/**
 * @brief Real-time wildlife event structure
 */
struct WildlifeEvent {
    String eventId;                      // Unique event identifier
    WildlifeEventType eventType;         // Type of wildlife event
    String cameraId;                     // Camera that triggered the event
    String speciesDetected;              // Detected species (if applicable)
    float confidence;                    // AI detection confidence
    float latitude;                      // GPS latitude
    float longitude;                     // GPS longitude
    uint32_t timestamp;                  // Event timestamp
    String imageFilename;                // Associated image file
    String audioFilename;                // Associated audio file (if any)
    String behaviorDescription;          // Behavior description
    String environmentalConditions;      // Environmental conditions
    int priority;                        // Event priority (1-5)
    String metadata;                     // Additional JSON metadata
    bool isValidated;                    // Researcher validation status
    String validatedBy;                  // Researcher who validated
    
    WildlifeEvent() : eventId(""), eventType(EVENT_SYSTEM_STATUS), cameraId(""),
        speciesDetected(""), confidence(0.0), latitude(0.0), longitude(0.0),
        timestamp(0), imageFilename(""), audioFilename(""), behaviorDescription(""),
        environmentalConditions(""), priority(3), metadata("{}"), isValidated(false),
        validatedBy("") {}
};

/**
 * @brief Researcher annotation structure
 */
struct ResearcherAnnotation {
    String annotationId;                 // Unique annotation identifier
    String eventId;                      // Associated event ID
    String researcherId;                 // Researcher who created annotation
    String annotationType;               // Type of annotation (classification, behavior, note)
    String annotationText;               // Annotation text content
    String tags;                         // JSON array of tags
    float confidence;                    // Researcher confidence in annotation
    uint32_t createdTimestamp;           // Creation timestamp
    uint32_t modifiedTimestamp;          // Last modification timestamp
    bool isPublic;                       // Public visibility flag
    String attachments;                  // JSON array of attachment filenames
    
    ResearcherAnnotation() : annotationId(""), eventId(""), researcherId(""),
        annotationType("note"), annotationText(""), tags("[]"), confidence(1.0),
        createdTimestamp(0), modifiedTimestamp(0), isPublic(true), attachments("[]") {}
};

/**
 * @brief Active researcher session information
 */
struct ActiveResearcher {
    String researcherId;                 // Unique researcher identifier
    String displayName;                  // Display name
    ResearcherRole role;                 // Access role
    String institution;                  // Research institution
    String currentProject;               // Current project ID
    uint32_t sessionStartTime;           // Session start timestamp
    uint32_t lastActivityTime;           // Last activity timestamp
    String currentCamera;                // Currently viewing camera
    bool isOnline;                       // Online status
    String ipAddress;                    // Client IP address
    uint8_t websocketId;                 // WebSocket connection ID
    String permissions;                  // JSON permissions object
    
    ActiveResearcher() : researcherId(""), displayName(""), role(ROLE_GUEST_OBSERVER),
        institution(""), currentProject(""), sessionStartTime(0), lastActivityTime(0),
        currentCamera(""), isOnline(false), ipAddress(""), websocketId(0),
        permissions("{}") {}
};

/**
 * @brief Research project workspace
 */
struct ResearchProject {
    String projectId;                    // Unique project identifier
    String projectName;                  // Project display name
    String description;                  // Project description
    String principalInvestigator;        // Principal investigator ID
    String collaborators;                // JSON array of collaborator IDs
    String targetSpecies;                // JSON array of target species
    String studyArea;                    // Study area description
    float studyAreaLatitude;             // Study area center latitude
    float studyAreaLongitude;            // Study area center longitude
    float studyAreaRadius;               // Study area radius (km)
    uint32_t startDate;                  // Project start date
    uint32_t endDate;                    // Project end date (0 = ongoing)
    String objectives;                   // Project objectives
    String methodology;                  // Research methodology
    bool isActive;                       // Project active status
    String dataAccessPolicy;             // Data access policy
    
    ResearchProject() : projectId(""), projectName(""), description(""),
        principalInvestigator(""), collaborators("[]"), targetSpecies("[]"),
        studyArea(""), studyAreaLatitude(0.0), studyAreaLongitude(0.0),
        studyAreaRadius(0.0), startDate(0), endDate(0), objectives(""),
        methodology(""), isActive(true), dataAccessPolicy("open") {}
};

/**
 * @brief Real-time notification structure
 */
struct Notification {
    String notificationId;               // Unique notification identifier
    String recipientId;                  // Recipient researcher ID (empty = broadcast)
    String title;                        // Notification title
    String message;                      // Notification message
    WildlifeEventType relatedEventType;  // Related event type
    String relatedEventId;               // Related event ID
    int priority;                        // Priority level (1-5)
    uint32_t createdTimestamp;           // Creation timestamp
    uint32_t expirationTimestamp;        // Expiration timestamp (0 = never)
    bool isRead;                         // Read status
    bool isActionable;                   // Requires user action
    String actionUrl;                    // Action URL or endpoint
    String metadata;                     // Additional JSON metadata
    
    Notification() : notificationId(""), recipientId(""), title(""), message(""),
        relatedEventType(EVENT_SYSTEM_STATUS), relatedEventId(""), priority(3),
        createdTimestamp(0), expirationTimestamp(0), isRead(false),
        isActionable(false), actionUrl(""), metadata("{}") {}
};

/**
 * @brief Collaborative Research Platform Class
 * 
 * Manages real-time collaborative research functionality including live data
 * streaming, multi-user sessions, notification systems, and annotation tools.
 */
class CollaborativeResearchPlatform {
public:
    CollaborativeResearchPlatform();
    ~CollaborativeResearchPlatform();

    /**
     * @brief Initialize the collaborative research platform
     * @param websocketPort Port for WebSocket server
     * @return true if initialization successful
     */
    bool initialize(uint16_t websocketPort = 81);

    /**
     * @brief Shutdown the platform
     */
    void shutdown();

    /**
     * @brief Main update loop - call regularly
     */
    void update();

    // Event Management
    /**
     * @brief Broadcast wildlife event to all connected researchers
     * @param event Wildlife event to broadcast
     * @return true if broadcast successful
     */
    bool broadcastWildlifeEvent(const WildlifeEvent& event);

    /**
     * @brief Add researcher annotation to an event
     * @param annotation Researcher annotation
     * @return true if annotation added successfully
     */
    bool addAnnotation(const ResearcherAnnotation& annotation);

    /**
     * @brief Get annotations for a specific event
     * @param eventId Event identifier
     * @return Vector of annotations
     */
    std::vector<ResearcherAnnotation> getAnnotations(const String& eventId);

    // User Session Management
    /**
     * @brief Authenticate and register researcher session
     * @param researcherId Researcher identifier
     * @param credentials Encrypted credentials
     * @param websocketId WebSocket connection ID
     * @return true if authentication successful
     */
    bool authenticateResearcher(const String& researcherId, 
                               const String& credentials,
                               uint8_t websocketId);

    /**
     * @brief Get active researchers
     * @return Vector of active researcher sessions
     */
    std::vector<ActiveResearcher> getActiveResearchers();

    /**
     * @brief Update researcher activity timestamp
     * @param researcherId Researcher identifier
     */
    void updateResearcherActivity(const String& researcherId);

    /**
     * @brief Disconnect researcher session
     * @param researcherId Researcher identifier
     */
    void disconnectResearcher(const String& researcherId);

    // Notification System
    /**
     * @brief Send notification to specific researcher
     * @param notification Notification to send
     * @return true if notification sent successfully
     */
    bool sendNotification(const Notification& notification);

    /**
     * @brief Broadcast notification to all researchers
     * @param notification Notification to broadcast
     * @return true if broadcast successful
     */
    bool broadcastNotification(const Notification& notification);

    /**
     * @brief Get unread notifications for researcher
     * @param researcherId Researcher identifier
     * @return Vector of unread notifications
     */
    std::vector<Notification> getUnreadNotifications(const String& researcherId);

    /**
     * @brief Mark notification as read
     * @param notificationId Notification identifier
     * @param researcherId Researcher identifier
     * @return true if marked successfully
     */
    bool markNotificationRead(const String& notificationId, const String& researcherId);

    // Project Management
    /**
     * @brief Create new research project
     * @param project Project information
     * @return true if project created successfully
     */
    bool createProject(const ResearchProject& project);

    /**
     * @brief Get project information
     * @param projectId Project identifier
     * @return Project information
     */
    ResearchProject getProject(const String& projectId);

    /**
     * @brief List projects accessible to researcher
     * @param researcherId Researcher identifier
     * @return Vector of accessible projects
     */
    std::vector<ResearchProject> getAccessibleProjects(const String& researcherId);

    // Real-time Data Streaming
    /**
     * @brief Start live data stream for researcher
     * @param researcherId Researcher identifier
     * @param cameraId Camera identifier (empty = all cameras)
     * @return true if stream started successfully
     */
    bool startLiveDataStream(const String& researcherId, const String& cameraId = "");

    /**
     * @brief Stop live data stream for researcher
     * @param researcherId Researcher identifier
     * @return true if stream stopped successfully
     */
    bool stopLiveDataStream(const String& researcherId);

    // Chat/Messaging System
    /**
     * @brief Send chat message
     * @param senderId Sender researcher ID
     * @param recipientId Recipient researcher ID (empty = broadcast)
     * @param message Chat message
     * @return true if message sent successfully
     */
    bool sendChatMessage(const String& senderId, const String& recipientId, 
                        const String& message);

    // Map Visualization
    /**
     * @brief Get real-time camera map data
     * @return JSON string with camera locations and recent detections
     */
    String getRealtimeMapData();

    // Statistics and Analytics
    /**
     * @brief Get platform statistics
     * @return JSON string with platform statistics
     */
    String getPlatformStatistics();

    /**
     * @brief Check if platform is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return initialized_; }

private:
    bool initialized_;
    WebSocketsServer* webSocketServer_;
    uint16_t websocketPort_;
    
    // Data storage
    std::vector<WildlifeEvent> recentEvents_;
    std::vector<ResearcherAnnotation> annotations_;
    std::vector<ActiveResearcher> activeResearchers_;
    std::vector<ResearchProject> projects_;
    std::vector<Notification> notifications_;
    
    // Statistics
    uint32_t totalEventsProcessed_;
    uint32_t totalAnnotationsCreated_;
    uint32_t totalNotificationsSent_;
    uint32_t totalResearcherSessions_;

    /**
     * @brief Handle WebSocket events
     * @param num WebSocket connection number
     * @param type Event type
     * @param payload Event payload
     * @param length Payload length
     */
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);

    /**
     * @brief Process incoming WebSocket message
     * @param researcherId Researcher identifier
     * @param message JSON message
     */
    void processWebSocketMessage(const String& researcherId, const JsonDocument& message);

    /**
     * @brief Send WebSocket message to specific researcher
     * @param researcherId Researcher identifier
     * @param message JSON message
     * @return true if sent successfully
     */
    bool sendWebSocketMessage(const String& researcherId, const JsonDocument& message);

    /**
     * @brief Broadcast WebSocket message to all researchers
     * @param message JSON message
     * @return true if broadcast successful
     */
    bool broadcastWebSocketMessage(const JsonDocument& message);

    /**
     * @brief Find active researcher by ID
     * @param researcherId Researcher identifier
     * @return Pointer to researcher or nullptr if not found
     */
    ActiveResearcher* findActiveResearcher(const String& researcherId);

    /**
     * @brief Find active researcher by WebSocket ID
     * @param websocketId WebSocket connection ID
     * @return Pointer to researcher or nullptr if not found
     */
    ActiveResearcher* findActiveResearcherByWebSocket(uint8_t websocketId);

    /**
     * @brief Generate unique event ID
     * @return Unique event identifier
     */
    String generateEventId();

    /**
     * @brief Generate unique annotation ID
     * @return Unique annotation identifier
     */
    String generateAnnotationId();

    /**
     * @brief Generate unique notification ID
     * @return Unique notification identifier
     */
    String generateNotificationId();

    /**
     * @brief Clean up expired notifications
     */
    void cleanupExpiredNotifications();

    /**
     * @brief Clean up inactive researcher sessions
     */
    void cleanupInactiveResearchers();

    /**
     * @brief Log platform event
     * @param event Event description
     * @param researcherId Associated researcher ID (if applicable)
     */
    void logPlatformEvent(const String& event, const String& researcherId = "");
};

// Global collaborative research platform instance
extern CollaborativeResearchPlatform* g_researchPlatform;

// Utility functions for easy integration
namespace ResearchPlatformUtils {
    /**
     * @brief Initialize global research platform
     * @param websocketPort WebSocket server port
     * @return true if successful
     */
    bool initializeGlobalPlatform(uint16_t websocketPort = 81);

    /**
     * @brief Quick wildlife event broadcast
     * @param eventType Event type
     * @param cameraId Camera identifier
     * @param speciesDetected Detected species
     * @param confidence AI confidence
     * @return true if broadcast successful
     */
    bool quickBroadcastEvent(WildlifeEventType eventType, const String& cameraId,
                            const String& speciesDetected, float confidence);

    /**
     * @brief Quick notification broadcast
     * @param title Notification title
     * @param message Notification message
     * @param priority Priority level
     * @return true if broadcast successful
     */
    bool quickBroadcastNotification(const String& title, const String& message, int priority = 3);

    /**
     * @brief Get platform status summary
     * @return JSON string with status summary
     */
    String getPlatformStatusSummary();
}

#endif // COLLABORATIVE_RESEARCH_PLATFORM_H