/**
 * @file board_node.cpp
 * @brief Implementation of node role for multi-board systems
 */

#include "board_node.h"
#include "../lora_mesh.h"
#include "../camera_handler.h"
#include "../config.h"

// Default node configuration
const NodeConfig BoardNode::DEFAULT_CONFIG = {
    .heartbeatInterval = 60000,      // 1 minute
    .coordinatorTimeout = 600000,    // 10 minutes
    .taskTimeout = 300000,           // 5 minutes
    .maxRetries = 3,
    .enableAutonomousMode = true,
    .enableTaskExecution = true
};

BoardNode::BoardNode() :
    nodeId_(0),
    preferredRole_(ROLE_NODE),
    currentRole_(ROLE_NODE),
    state_(NODE_INACTIVE),
    discoveryProtocol_(nullptr),
    initialized_(false),
    coordinatorNode_(-1),
    lastCoordinatorContact_(0),
    nodeConfig_(DEFAULT_CONFIG),
    startTime_(0),
    lastHeartbeat_(0),
    lastStatusUpdate_(0),
    tasksCompleted_(0),
    tasksFailed_(0),
    coordinatorChanges_(0),
    totalTaskTime_(0) {
}

BoardNode::~BoardNode() {
    stopNode();
    if (discoveryProtocol_) {
        delete discoveryProtocol_;
    }
}

bool BoardNode::init(int nodeId, BoardRole preferredRole) {
    nodeId_ = nodeId;
    preferredRole_ = preferredRole;
    currentRole_ = preferredRole;
    capabilities_ = MessageProtocol::getCurrentCapabilities();
    
    // Initialize discovery protocol
    discoveryProtocol_ = new DiscoveryProtocol();
    if (!discoveryProtocol_->init(nodeId, preferredRole)) {
        Serial.println("Failed to initialize discovery protocol");
        delete discoveryProtocol_;
        discoveryProtocol_ = nullptr;
        return false;
    }
    
    state_ = NODE_INACTIVE;
    initialized_ = true;
    
    Serial.printf("Board node initialized: Node %d, Preferred role: %s\n",
                  nodeId, MessageProtocol::roleToString(preferredRole));
    
    return true;
}

bool BoardNode::startNode() {
    if (!initialized_) {
        Serial.println("Node not initialized");
        return false;
    }
    
    state_ = NODE_INITIALIZING;
    startTime_ = millis();
    lastHeartbeat_ = millis();
    lastStatusUpdate_ = millis();
    
    Serial.println("Starting node operation...");
    
    // Start discovery to find coordinator
    if (discoveryProtocol_->startDiscovery()) {
        state_ = NODE_SEEKING_COORDINATOR;
        Serial.println("Node seeking coordinator...");
        return true;
    }
    
    return false;
}

void BoardNode::stopNode() {
    if (discoveryProtocol_) {
        discoveryProtocol_->stopDiscovery();
    }
    
    state_ = NODE_INACTIVE;
    activeTasks_.clear();
    coordinatorNode_ = -1;
    
    Serial.println("Node stopped");
}

void BoardNode::process() {
    if (!initialized_ || state_ == NODE_INACTIVE) {
        return;
    }
    
    unsigned long now = millis();
    
    // Process discovery protocol
    if (discoveryProtocol_) {
        discoveryProtocol_->processMessages();
    }
    
    switch (state_) {
        case NODE_SEEKING_COORDINATOR:
            processSeekingCoordinator();
            break;
            
        case NODE_ACTIVE:
            processActiveNode();
            break;
            
        case NODE_TASK_EXECUTING:
            processTaskExecution();
            break;
            
        case NODE_STANDALONE:
            processStandaloneMode();
            break;
            
        default:
            break;
    }
    
    // Send periodic heartbeat
    if (now - lastHeartbeat_ >= nodeConfig_.heartbeatInterval) {
        sendHeartbeat();
        lastHeartbeat_ = now;
    }
    
    // Send periodic status update
    if (now - lastStatusUpdate_ >= STATUS_UPDATE_INTERVAL) {
        sendStatusUpdate();
        lastStatusUpdate_ = now;
    }
    
    // Check for task timeouts
    checkTaskTimeouts();
    
    // Check coordinator availability
    if (coordinatorNode_ > 0 && 
        (now - lastCoordinatorContact_) > nodeConfig_.coordinatorTimeout) {
        Serial.println("Coordinator timeout, switching to standalone mode");
        switchToStandaloneMode();
    }
}

void BoardNode::handleMessage(const MultiboardMessage& msg) {
    switch (msg.type) {
        case MSG_DISCOVERY:
            if (discoveryProtocol_) {
                discoveryProtocol_->handleDiscoveryMessage(msg);
            }
            break;
            
        case MSG_ROLE_ASSIGNMENT:
            handleRoleAssignment(msg);
            break;
            
        case MSG_TASK_ASSIGNMENT:
            handleTaskAssignment(msg);
            break;
            
        case MSG_CONFIG_UPDATE:
            handleConfigUpdate(msg);
            break;
            
        case MSG_HEARTBEAT:
            if (msg.sourceRole == ROLE_COORDINATOR) {
                handleCoordinatorHeartbeat(msg);
            }
            break;
            
        default:
            Serial.printf("Node received unhandled message type: %s\n",
                         MessageProtocol::messageTypeToString(msg.type));
            break;
    }
}

void BoardNode::setAssignedRole(BoardRole role) {
    if (role != currentRole_) {
        Serial.printf("Role changed from %s to %s\n",
                     MessageProtocol::roleToString(currentRole_),
                     MessageProtocol::roleToString(role));
        currentRole_ = role;
    }
}

BoardRole BoardNode::getCurrentRole() const {
    return currentRole_;
}

BoardRole BoardNode::getPreferredRole() const {
    return preferredRole_;
}

NodeState BoardNode::getState() const {
    return state_;
}

int BoardNode::getCoordinatorNode() const {
    return coordinatorNode_;
}

const std::vector<NodeTask>& BoardNode::getActiveTasks() const {
    return activeTasks_;
}

const std::vector<NodeTask>& BoardNode::getCompletedTasks() const {
    return completedTasks_;
}

bool BoardNode::sendStatusUpdate() {
    DynamicJsonDocument doc(1024);
    doc["type"] = MSG_STATUS;
    doc["source_node"] = nodeId_;
    doc["target_node"] = coordinatorNode_ > 0 ? coordinatorNode_ : 0; // Broadcast if no coordinator
    doc["timestamp"] = millis();
    doc["hop_count"] = 0;
    
    JsonObject data = doc.createNestedObject("data");
    data["role"] = currentRole_;
    data["state"] = state_;
    data["active_tasks"] = activeTasks_.size();
    data["completed_tasks"] = tasksCompleted_;
    data["failed_tasks"] = tasksFailed_;
    data["uptime"] = millis() - startTime_;
    data["battery_level"] = capabilities_.batteryLevel;
    data["free_storage"] = capabilities_.availableStorage;
    
    String message;
    serializeJson(doc, message);
    
    return LoraMesh::queueMessage(message);
}

bool BoardNode::sendHeartbeat() {
    DynamicJsonDocument doc(512);
    doc["type"] = MSG_HEARTBEAT;
    doc["source_node"] = nodeId_;
    doc["target_node"] = 0; // Broadcast
    doc["timestamp"] = millis();
    doc["hop_count"] = 0;
    
    JsonObject data = doc.createNestedObject("data");
    data["role"] = currentRole_;
    data["state"] = state_;
    data["coordinator"] = coordinatorNode_;
    
    String message;
    serializeJson(doc, message);
    
    return LoraMesh::queueMessage(message);
}

bool BoardNode::executeTask(const NodeTask& task) {
    if (!nodeConfig_.enableTaskExecution) {
        return false;
    }
    
    Serial.printf("Executing task %d: %s\n", task.taskId, task.taskType.c_str());
    
    bool success = false;
    
    if (task.taskType == "image_capture") {
        success = executeImageCaptureTask(task);
    } else if (task.taskType == "motion_detection") {
        success = executeMotionDetectionTask(task);
    } else if (task.taskType == "data_transmission") {
        success = executeDataTransmissionTask(task);
    } else if (task.taskType == "system_status") {
        success = executeSystemStatusTask(task);
    } else if (task.taskType == "ai_analysis") {
        success = executeAIAnalysisTask(task);
    } else {
        Serial.printf("Unknown task type: %s\n", task.taskType.c_str());
        return false;
    }
    
    return success;
}

bool BoardNode::reportTaskCompletion(int taskId, bool success, const String& result) {
    DynamicJsonDocument doc(1024);
    doc["type"] = MSG_DATA;
    doc["source_node"] = nodeId_;
    doc["target_node"] = coordinatorNode_;
    doc["timestamp"] = millis();
    doc["hop_count"] = 0;
    
    JsonObject data = doc.createNestedObject("data");
    data["task_id"] = taskId;
    data["success"] = success;
    data["result"] = result;
    data["completion_time"] = millis();
    
    String message;
    serializeJson(doc, message);
    
    return LoraMesh::queueMessage(message);
}

BoardNode::NodeStats BoardNode::getStats() const {
    NodeStats stats = {};
    
    stats.uptimeMs = millis() - startTime_;
    stats.tasksCompleted = tasksCompleted_;
    stats.tasksFailed = tasksFailed_;
    stats.coordinatorChanges = coordinatorChanges_;
    stats.lastCoordinatorContact = lastCoordinatorContact_;
    stats.totalTaskTime = totalTaskTime_;
    
    int totalTasks = tasksCompleted_ + tasksFailed_;
    stats.taskSuccessRate = totalTasks > 0 ? 
        (float)tasksCompleted_ / totalTasks : 0.0;
    
    return stats;
}

void BoardNode::setNodeConfig(const NodeConfig& config) {
    nodeConfig_ = config;
}

const NodeConfig& BoardNode::getNodeConfig() const {
    return nodeConfig_;
}

void BoardNode::setAutonomousMode(bool enabled) {
    nodeConfig_.enableAutonomousMode = enabled;
    
    if (enabled && state_ == NODE_ACTIVE && coordinatorNode_ == -1) {
        switchToStandaloneMode();
    }
}

bool BoardNode::isCoordinatorAvailable() const {
    return coordinatorNode_ > 0 && 
           (millis() - lastCoordinatorContact_) < nodeConfig_.coordinatorTimeout;
}

void BoardNode::seekCoordinator() {
    state_ = NODE_SEEKING_COORDINATOR;
    coordinatorNode_ = -1;
    
    if (discoveryProtocol_) {
        discoveryProtocol_->startDiscovery();
    }
}

void BoardNode::processSeekingCoordinator() {
    if (!discoveryProtocol_) return;
    
    if (discoveryProtocol_->isDiscoveryComplete()) {
        coordinatorNode_ = discoveryProtocol_->getCoordinatorNode();
        
        if (coordinatorNode_ > 0) {
            state_ = NODE_ACTIVE;
            lastCoordinatorContact_ = millis();
            Serial.printf("Found coordinator: Node %d\n", coordinatorNode_);
        } else if (nodeConfig_.enableAutonomousMode) {
            switchToStandaloneMode();
        }
    }
}

void BoardNode::processActiveNode() {
    // Process any active tasks
    if (!activeTasks_.empty()) {
        state_ = NODE_TASK_EXECUTING;
    }
    
    // Regular node operation
    cleanupCompletedTasks();
}

void BoardNode::processTaskExecution() {
    bool hasActiveTasks = false;
    
    for (auto& task : activeTasks_) {
        if (task.status == TASK_PENDING) {
            task.status = TASK_RUNNING;
            task.startTime = millis();
            
            bool success = executeTask(task);
            
            if (success) {
                task.status = TASK_COMPLETED;
                task.completionTime = millis();
                tasksCompleted_++;
                totalTaskTime_ += (task.completionTime - task.startTime);
                reportTaskCompletion(task.taskId, true);
            } else {
                task.status = TASK_FAILED;
                task.completionTime = millis();
                tasksFailed_++;
                reportTaskCompletion(task.taskId, false, task.errorMessage);
            }
        } else if (task.status == TASK_RUNNING) {
            hasActiveTasks = true;
        }
    }
    
    if (!hasActiveTasks) {
        state_ = NODE_ACTIVE;
    }
}

void BoardNode::processStandaloneMode() {
    if (nodeConfig_.enableAutonomousMode) {
        performStandaloneTasks();
    }
    
    // Periodically check for coordinator
    if (discoveryProtocol_ && 
        (millis() - lastCoordinatorContact_) > nodeConfig_.coordinatorTimeout / 2) {
        seekCoordinator();
    }
}

void BoardNode::handleRoleAssignment(const MultiboardMessage& msg) {
    BoardRole assignedRole = static_cast<BoardRole>(msg.data["assigned_role"].as<int>());
    setAssignedRole(assignedRole);
    
    Serial.printf("Received role assignment: %s\n",
                  MessageProtocol::roleToString(assignedRole));
}

void BoardNode::handleTaskAssignment(const MultiboardMessage& msg) {
    NodeTask task;
    task.taskId = msg.data["task_id"];
    task.taskType = msg.data["task_type"].as<String>();
    task.deadline = msg.data["deadline"];
    task.priority = msg.data["priority"];
    task.status = TASK_PENDING;
    task.startTime = 0;
    task.completionTime = 0;
    
    // Copy task parameters
    task.parameters = msg.data["parameters"];
    
    activeTasks_.push_back(task);
    
    Serial.printf("Received task assignment: %s (ID: %d)\n",
                  task.taskType.c_str(), task.taskId);
}

void BoardNode::handleConfigUpdate(const MultiboardMessage& msg) {
    Serial.println("Received configuration update");
    // TODO: Update configuration based on received data
}

void BoardNode::handleCoordinatorHeartbeat(const MultiboardMessage& msg) {
    coordinatorNode_ = msg.sourceNode;
    lastCoordinatorContact_ = millis();
    
    if (state_ == NODE_SEEKING_COORDINATOR || state_ == NODE_STANDALONE) {
        state_ = NODE_ACTIVE;
        Serial.printf("Reconnected to coordinator: Node %d\n", coordinatorNode_);
        coordinatorChanges_++;
    }
}

bool BoardNode::executeImageCaptureTask(const NodeTask& task) {
    Serial.println("Executing image capture task using CameraHandler");
    
    // Initialize camera if not already done
    if (!cameraHandler_.isInitialized()) {
        Serial.println("Initializing camera for image capture task...");
        if (!cameraHandler_.init()) {
            Serial.println("Camera initialization failed");
            return false;
        }
    }
    
    // Extract parameters from task
    uint32_t timeout_ms = 5000; // Default 5 second timeout
    if (task.parameters.containsKey("timeout_ms")) {
        timeout_ms = task.parameters["timeout_ms"];
    }
    
    String save_folder = "/wildlife_images";
    if (task.parameters.containsKey("folder")) {
        save_folder = task.parameters["folder"].as<String>();
    }
    
    // Capture frame with timeout
    esp_err_t result = cameraHandler_.captureFrame(timeout_ms);
    if (result != ESP_OK) {
        Serial.printf("Frame capture failed with error: 0x%x\n", result);
        return false;
    }
    
    // Get the captured frame
    camera_fb_t* fb = cameraHandler_.getFrameBuffer();
    if (!fb) {
        Serial.println("Failed to get frame buffer");
        return false;
    }
    
    // Save the image
    String filename = cameraHandler_.saveImage(fb, save_folder.c_str());
    
    // Return frame buffer to system
    cameraHandler_.returnFrameBuffer(fb);
    
    if (filename.length() > 0) {
        Serial.printf("Image capture successful: %s\n", filename.c_str());
        return true;
    } else {
        Serial.println("Image save failed");
        return false;
    }
}

bool BoardNode::executeMotionDetectionTask(const NodeTask& task) {
    Serial.println("Executing motion detection task using MotionDetectionManager");
    
    // Initialize camera manager for motion detection if not already done
    if (!cameraManager_.isReady()) {
        Serial.println("Initializing camera manager for motion detection...");
        if (!cameraManager_.initialize()) {
            Serial.println("Camera manager initialization failed");
            return false;
        }
    }
    
    // Initialize motion detection manager if not already done
    static bool motionInitialized = false;
    if (!motionInitialized) {
        Serial.println("Initializing motion detection manager...");
        if (!motionManager_.initialize(&cameraManager_, MotionDetectionManager::DetectionSystem::ENHANCED_HYBRID)) {
            Serial.println("Motion detection manager initialization failed");
            return false;
        }
        
        // Configure enhanced features including PIR sensor on GPIO 1
        if (!motionManager_.configureEnhancedFeatures(true, true, true)) {
            Serial.println("Enhanced motion detection features configuration failed");
            return false;
        }
        
        motionInitialized = true;
        Serial.println("Motion detection system initialized successfully");
    }
    
    // Perform motion detection
    auto motionResult = motionManager_.detectMotion();
    
    if (motionResult.motionDetected) {
        Serial.printf("Motion detected! Confidence: %.2f\n", motionResult.confidenceScore);
        
        // If motion detected, trigger image capture and storage
        if (motionResult.hasEnhancedData) {
            Serial.printf("Enhanced data: Zones=%d, Speed=%.1f, Direction=%.1f°, ML=%.2f\n",
                         motionResult.activeZoneCount,
                         motionResult.motionSpeed,
                         motionResult.motionDirection * 180 / PI,
                         motionResult.mlConfidence);
        }
        
        // Capture image when motion is detected
        String motionFolder = "/wildlife_motion";
        auto captureResult = cameraManager_.captureImage(motionFolder);
        
        if (captureResult.success) {
            Serial.printf("Motion-triggered image captured: %s (%.2f KB)\n", 
                         captureResult.filename.c_str(), 
                         captureResult.imageSize / 1024.0);
            return true;
        } else {
            Serial.println("Motion-triggered image capture failed");
            return false;
        }
    } else {
        Serial.println("No motion detected");
        return true; // Not detecting motion is still a successful task execution
    }
}

bool BoardNode::executeDataTransmissionTask(const NodeTask& task) {
    // TODO: Implement data transmission
    Serial.println("Executing data transmission task");
    delay(2000); // Simulate transmission time
    return true;
}

bool BoardNode::executeSystemStatusTask(const NodeTask& task) {
    Serial.println("Executing system status task");
    sendStatusUpdate();
    return true;
}

bool BoardNode::executeAIAnalysisTask(const NodeTask& task) {
    // TODO: Implement AI analysis
    Serial.println("Executing AI analysis task");
    delay(3000); // Simulate analysis time
    return true;
}

void BoardNode::checkTaskTimeouts() {
    unsigned long now = millis();
    
    for (auto& task : activeTasks_) {
        if (task.status == TASK_RUNNING && now > task.deadline) {
            task.status = TASK_TIMEOUT;
            task.completionTime = now;
            tasksFailed_++;
            Serial.printf("Task %d timed out\n", task.taskId);
        }
    }
}

void BoardNode::cleanupCompletedTasks() {
    activeTasks_.erase(
        std::remove_if(activeTasks_.begin(), activeTasks_.end(),
                      [this](const NodeTask& task) {
                          if (task.status == TASK_COMPLETED || 
                              task.status == TASK_FAILED ||
                              task.status == TASK_TIMEOUT) {
                              completedTasks_.push_back(task);
                              return true;
                          }
                          return false;
                      }),
        activeTasks_.end()
    );
}

void BoardNode::switchToStandaloneMode() {
    state_ = NODE_STANDALONE;
    coordinatorNode_ = -1;
    Serial.println("Switched to standalone mode");
}

void BoardNode::performStandaloneTasks() {
    // Perform basic wildlife monitoring tasks independently
    static unsigned long lastStandaloneTask = 0;
    unsigned long now = millis();
    
    if (now - lastStandaloneTask > 300000) { // Every 5 minutes
        Serial.println("Performing standalone monitoring task");
        // TODO: Implement standalone image capture and basic analysis
        lastStandaloneTask = now;
    }
}