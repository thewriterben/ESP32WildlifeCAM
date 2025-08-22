/**
 * @file board_node.h
 * @brief Node role implementation for multi-board systems
 * 
 * Implements camera board node functionality for participating in
 * a coordinated wildlife monitoring network under coordinator management.
 */

#ifndef BOARD_NODE_H
#define BOARD_NODE_H

#include <Arduino.h>
#include <vector>
#include "message_protocol.h"
#include "discovery_protocol.h"

// Node states
enum NodeState {
    NODE_INACTIVE = 0,
    NODE_INITIALIZING = 1,
    NODE_SEEKING_COORDINATOR = 2,
    NODE_ACTIVE = 3,
    NODE_TASK_EXECUTING = 4,
    NODE_STANDALONE = 5
};

// Task execution status
enum TaskStatus {
    TASK_PENDING = 0,
    TASK_RUNNING = 1,
    TASK_COMPLETED = 2,
    TASK_FAILED = 3,
    TASK_TIMEOUT = 4
};

// Node task execution structure
struct NodeTask {
    int taskId;
    String taskType;
    JsonObject parameters;
    unsigned long deadline;
    int priority;
    TaskStatus status;
    unsigned long startTime;
    unsigned long completionTime;
    String errorMessage;
};

// Node configuration
struct NodeConfig {
    unsigned long heartbeatInterval;
    unsigned long coordinatorTimeout;
    unsigned long taskTimeout;
    int maxRetries;
    bool enableAutonomousMode;
    bool enableTaskExecution;
};

class BoardNode {
public:
    BoardNode();
    ~BoardNode();
    
    /**
     * Initialize node
     */
    bool init(int nodeId, BoardRole preferredRole = ROLE_NODE);
    
    /**
     * Start node operation
     */
    bool startNode();
    
    /**
     * Stop node operation
     */
    void stopNode();
    
    /**
     * Process node tasks and messages
     */
    void process();
    
    /**
     * Handle incoming messages
     */
    void handleMessage(const MultiboardMessage& msg);
    
    /**
     * Set assigned role from coordinator
     */
    void setAssignedRole(BoardRole role);
    
    /**
     * Get current role
     */
    BoardRole getCurrentRole() const;
    
    /**
     * Get preferred role
     */
    BoardRole getPreferredRole() const;
    
    /**
     * Get node state
     */
    NodeState getState() const;
    
    /**
     * Get coordinator node ID
     */
    int getCoordinatorNode() const;
    
    /**
     * Get active tasks
     */
    const std::vector<NodeTask>& getActiveTasks() const;
    
    /**
     * Get completed tasks
     */
    const std::vector<NodeTask>& getCompletedTasks() const;
    
    /**
     * Send status update to coordinator
     */
    bool sendStatusUpdate();
    
    /**
     * Send heartbeat to network
     */
    bool sendHeartbeat();
    
    /**
     * Execute specific task
     */
    bool executeTask(const NodeTask& task);
    
    /**
     * Report task completion
     */
    bool reportTaskCompletion(int taskId, bool success, const String& result = "");
    
    /**
     * Get node statistics
     */
    struct NodeStats {
        unsigned long uptimeMs;
        int tasksCompleted;
        int tasksFailed;
        int coordinatorChanges;
        unsigned long lastCoordinatorContact;
        float taskSuccessRate;
        unsigned long totalTaskTime;
    };
    NodeStats getStats() const;
    
    /**
     * Set node configuration
     */
    void setNodeConfig(const NodeConfig& config);
    
    /**
     * Get node configuration
     */
    const NodeConfig& getNodeConfig() const;
    
    /**
     * Enable/disable autonomous mode
     */
    void setAutonomousMode(bool enabled);
    
    /**
     * Check if coordinator is available
     */
    bool isCoordinatorAvailable() const;
    
    /**
     * Force coordinator search
     */
    void seekCoordinator();
    
private:
    // Core state
    int nodeId_;
    BoardRole preferredRole_;
    BoardRole currentRole_;
    NodeState state_;
    BoardCapabilities capabilities_;
    DiscoveryProtocol* discoveryProtocol_;
    bool initialized_;
    
    // Network state
    int coordinatorNode_;
    unsigned long lastCoordinatorContact_;
    
    // Task management
    std::vector<NodeTask> activeTasks_;
    std::vector<NodeTask> completedTasks_;
    NodeConfig nodeConfig_;
    
    // Timing
    unsigned long startTime_;
    unsigned long lastHeartbeat_;
    unsigned long lastStatusUpdate_;
    
    // Statistics
    int tasksCompleted_;
    int tasksFailed_;
    int coordinatorChanges_;
    unsigned long totalTaskTime_;
    
    // Internal methods
    void processSeekingCoordinator();
    void processActiveNode();
    void processTaskExecution();
    void processStandaloneMode();
    
    void handleRoleAssignment(const MultiboardMessage& msg);
    void handleTaskAssignment(const MultiboardMessage& msg);
    void handleConfigUpdate(const MultiboardMessage& msg);
    void handleCoordinatorHeartbeat(const MultiboardMessage& msg);
    
    bool executeImageCaptureTask(const NodeTask& task);
    bool executeMotionDetectionTask(const NodeTask& task);
    bool executeDataTransmissionTask(const NodeTask& task);
    bool executeSystemStatusTask(const NodeTask& task);
    bool executeAIAnalysisTask(const NodeTask& task);
    
    void updateTaskStatus(int taskId, TaskStatus status, const String& error = "");
    void cleanupCompletedTasks();
    void checkTaskTimeouts();
    
    void switchToStandaloneMode();
    void performStandaloneTasks();
    
    // Default configuration
    static const NodeConfig DEFAULT_CONFIG;
    static const unsigned long HEARTBEAT_INTERVAL = 60000;      // 1 minute
    static const unsigned long STATUS_UPDATE_INTERVAL = 300000; // 5 minutes
    static const unsigned long COORDINATOR_TIMEOUT = 600000;    // 10 minutes
};

#endif // BOARD_NODE_H