/**
 * @file FederatedLearningCoordinator.h
 * @brief Version 3.2 Federated Learning Coordinator for ESP32 Wildlife CAM
 * @author thewriterben
 * @date 2025-01-01
 * @version 3.2.0
 * 
 * Implements comprehensive federated learning coordination across wildlife
 * camera networks with privacy-preserving algorithms, model aggregation,
 * adaptive learning, and network resilience features.
 */

#ifndef FEDERATED_LEARNING_COORDINATOR_H
#define FEDERATED_LEARNING_COORDINATOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "../../firmware/src/mesh/federated_mesh.h"
#include "../../include/blockchain/blockchain_types.h"

/**
 * @brief Wildlife model types for federated learning
 */
enum class WildlifeModelType {
    SPECIES_CLASSIFIER,         // Species identification model
    BEHAVIOR_ANALYZER,          // Behavior pattern analysis model
    MOTION_DETECTOR,            // Motion detection model
    AUDIO_CLASSIFIER,           // Audio classification model
    ENVIRONMENTAL_PREDICTOR,    // Environmental condition predictor
    HYBRID_MULTIMODAL          // Combined audio-visual model
};

/**
 * @brief Federated learning algorithms
 */
enum class FederatedAlgorithm {
    FEDERATED_AVERAGING,        // Standard FedAvg algorithm
    FEDERATED_PROX,            // FedProx with proximal term
    FEDERATED_NOVA,            // FedNova with normalized averaging
    DIFFERENTIAL_PRIVACY,       // DP-FedAvg with differential privacy
    SECURE_AGGREGATION,        // Secure multi-party computation
    ADAPTIVE_FEDERATED         // Adaptive algorithm selection
};

/**
 * @brief Privacy preservation techniques
 */
enum class PrivacyTechnique {
    NONE,                      // No privacy preservation
    DIFFERENTIAL_PRIVACY,      // Differential privacy with noise
    HOMOMORPHIC_ENCRYPTION,    // Homomorphic encryption (limited)
    SECURE_MULTIPARTY,         // Secure multi-party computation
    GRADIENT_COMPRESSION,      // Gradient compression and quantization
    LOCAL_DIFFERENTIAL        // Local differential privacy
};

/**
 * @brief Model update structure for federated learning
 */
struct ModelUpdate {
    String nodeId;                      // Node identifier
    WildlifeModelType modelType;        // Type of model
    uint32_t modelVersion;              // Model version number
    String modelWeights;                // Serialized model weights (base64)
    float trainingLoss;                 // Training loss on local data
    float validationAccuracy;           // Validation accuracy
    uint32_t datasetSize;               // Local dataset size
    uint32_t trainingEpochs;            // Number of training epochs
    uint32_t timestamp;                 // Update timestamp
    String dataDistribution;            // JSON of local data distribution
    float privacyBudget;                // Privacy budget consumed
    String cryptographicHash;           // Hash for integrity verification
    
    ModelUpdate() : nodeId(""), modelType(WildlifeModelType::SPECIES_CLASSIFIER),
        modelVersion(0), modelWeights(""), trainingLoss(1.0f), validationAccuracy(0.0f),
        datasetSize(0), trainingEpochs(0), timestamp(0), dataDistribution("{}"),
        privacyBudget(0.0f), cryptographicHash("") {}
};

/**
 * @brief Aggregated global model structure
 */
struct GlobalModel {
    WildlifeModelType modelType;        // Type of model
    uint32_t modelVersion;              // Global model version
    String aggregatedWeights;           // Aggregated model weights
    float globalLoss;                   // Global loss estimate
    float globalAccuracy;               // Global accuracy estimate
    uint32_t participantCount;          // Number of participants
    uint32_t aggregationTimestamp;      // Aggregation timestamp
    String modelMetadata;               // JSON metadata
    float convergenceScore;             // Convergence measurement
    bool isConverged;                   // Convergence status
    String distributionSignature;       // Data distribution signature
    
    GlobalModel() : modelType(WildlifeModelType::SPECIES_CLASSIFIER), modelVersion(0),
        aggregatedWeights(""), globalLoss(1.0f), globalAccuracy(0.0f),
        participantCount(0), aggregationTimestamp(0), modelMetadata("{}"),
        convergenceScore(0.0f), isConverged(false), distributionSignature("") {}
};

/**
 * @brief Federated learning participant information
 */
struct FederatedParticipant {
    String nodeId;                      // Unique node identifier
    String nodeType;                    // Node hardware type
    float computationalCapability;      // Computational power score
    float networkBandwidth;             // Available bandwidth (Mbps)
    float batteryLevel;                 // Battery level (0.0-1.0)
    uint32_t localDatasetSize;          // Size of local dataset
    String geographicLocation;          // Geographic location
    uint32_t lastParticipationTime;     // Last participation timestamp
    bool isActive;                      // Active participation status
    float contributionScore;            // Contribution quality score
    String supportedModels;             // JSON array of supported model types
    float privacyPreference;            // Privacy preference level (0.0-1.0)
    
    FederatedParticipant() : nodeId(""), nodeType("ESP32CAM"), computationalCapability(1.0f),
        networkBandwidth(1.0f), batteryLevel(1.0f), localDatasetSize(0),
        geographicLocation("unknown"), lastParticipationTime(0), isActive(false),
        contributionScore(0.5f), supportedModels("[]"), privacyPreference(0.5f) {}
};

/**
 * @brief Federated learning training round
 */
struct TrainingRound {
    String roundId;                     // Unique round identifier
    WildlifeModelType modelType;        // Model type for this round
    FederatedAlgorithm algorithm;       // Algorithm used
    PrivacyTechnique privacyTechnique;  // Privacy technique applied
    uint32_t startTimestamp;            // Round start timestamp
    uint32_t endTimestamp;              // Round end timestamp (0 = ongoing)
    std::vector<String> participants;   // Participant node IDs
    GlobalModel currentModel;           // Current global model
    float targetAccuracy;               // Target accuracy threshold
    uint32_t maxRounds;                 // Maximum rounds before termination
    uint32_t currentRoundNumber;        // Current round number
    bool isCompleted;                   // Round completion status
    String coordinatorNodeId;           // Coordinator node ID
    
    TrainingRound() : roundId(""), modelType(WildlifeModelType::SPECIES_CLASSIFIER),
        algorithm(FederatedAlgorithm::FEDERATED_AVERAGING),
        privacyTechnique(PrivacyTechnique::NONE), startTimestamp(0), endTimestamp(0),
        targetAccuracy(0.95f), maxRounds(100), currentRoundNumber(0),
        isCompleted(false), coordinatorNodeId("") {}
};

/**
 * @brief Federated Learning Coordinator Class
 * 
 * Manages federated learning operations across wildlife camera networks
 * with comprehensive support for model aggregation, privacy preservation,
 * and adaptive learning strategies.
 */
class FederatedLearningCoordinator {
public:
    FederatedLearningCoordinator();
    ~FederatedLearningCoordinator();

    /**
     * @brief Initialize the federated learning coordinator
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Shutdown the coordinator
     */
    void shutdown();

    /**
     * @brief Main update loop - call regularly
     */
    void update();

    // Training Round Management
    /**
     * @brief Start a new federated learning training round
     * @param modelType Type of model to train
     * @param algorithm Federated learning algorithm to use
     * @param privacyTechnique Privacy preservation technique
     * @return Training round ID if successful, empty string if failed
     */
    String startTrainingRound(WildlifeModelType modelType, 
                             FederatedAlgorithm algorithm = FederatedAlgorithm::FEDERATED_AVERAGING,
                             PrivacyTechnique privacyTechnique = PrivacyTechnique::NONE);

    /**
     * @brief Join an existing training round
     * @param roundId Training round identifier
     * @return true if successfully joined
     */
    bool joinTrainingRound(const String& roundId);

    /**
     * @brief Submit model update to training round
     * @param roundId Training round identifier
     * @param modelUpdate Local model update
     * @return true if update submitted successfully
     */
    bool submitModelUpdate(const String& roundId, const ModelUpdate& modelUpdate);

    /**
     * @brief Get current global model for a training round
     * @param roundId Training round identifier
     * @return Global model structure
     */
    GlobalModel getGlobalModel(const String& roundId);

    // Participant Management
    /**
     * @brief Register as a federated learning participant
     * @param participant Participant information
     * @return true if registration successful
     */
    bool registerParticipant(const FederatedParticipant& participant);

    /**
     * @brief Update participant information
     * @param nodeId Node identifier
     * @param participant Updated participant information
     * @return true if update successful
     */
    bool updateParticipant(const String& nodeId, const FederatedParticipant& participant);

    /**
     * @brief Get active participants
     * @return Vector of active participants
     */
    std::vector<FederatedParticipant> getActiveParticipants();

    /**
     * @brief Select optimal participants for training round
     * @param modelType Model type
     * @param maxParticipants Maximum number of participants
     * @return Vector of selected participant IDs
     */
    std::vector<String> selectOptimalParticipants(WildlifeModelType modelType, 
                                                 uint32_t maxParticipants);

    // Model Aggregation
    /**
     * @brief Aggregate model updates using specified algorithm
     * @param roundId Training round identifier
     * @param modelUpdates Vector of model updates
     * @param algorithm Aggregation algorithm
     * @return Aggregated global model
     */
    GlobalModel aggregateModelUpdates(const String& roundId,
                                    const std::vector<ModelUpdate>& modelUpdates,
                                    FederatedAlgorithm algorithm);

    /**
     * @brief Apply privacy preservation to model update
     * @param modelUpdate Model update to protect
     * @param privacyTechnique Privacy technique to apply
     * @param privacyBudget Privacy budget allocation
     * @return Privacy-preserved model update
     */
    ModelUpdate applyPrivacyPreservation(const ModelUpdate& modelUpdate,
                                        PrivacyTechnique privacyTechnique,
                                        float privacyBudget);

    // Adaptive Learning
    /**
     * @brief Adapt learning parameters based on network conditions
     * @param roundId Training round identifier
     * @return true if adaptation applied
     */
    bool adaptLearningParameters(const String& roundId);

    /**
     * @brief Detect and handle network partitions
     * @return true if network is healthy
     */
    bool handleNetworkPartitions();

    /**
     * @brief Optimize for intermittent connectivity
     * @param roundId Training round identifier
     * @return true if optimization applied
     */
    bool optimizeForIntermittentConnectivity(const String& roundId);

    // Edge Computing Optimization
    /**
     * @brief Optimize model for edge deployment
     * @param model Global model to optimize
     * @param targetDevice Target device capabilities
     * @return Optimized model
     */
    GlobalModel optimizeModelForEdge(const GlobalModel& model, 
                                   const String& targetDevice);

    /**
     * @brief Perform model compression
     * @param modelWeights Model weights to compress
     * @param compressionRatio Target compression ratio
     * @return Compressed model weights
     */
    String compressModelWeights(const String& modelWeights, float compressionRatio);

    /**
     * @brief Quantize model for reduced memory usage
     * @param modelWeights Model weights to quantize
     * @param quantizationBits Number of bits for quantization
     * @return Quantized model weights
     */
    String quantizeModelWeights(const String& modelWeights, uint8_t quantizationBits);

    // Statistics and Monitoring
    /**
     * @brief Get federated learning statistics
     * @return JSON string with statistics
     */
    String getStatistics();

    /**
     * @brief Get training round status
     * @param roundId Training round identifier
     * @return JSON string with round status
     */
    String getTrainingRoundStatus(const String& roundId);

    /**
     * @brief Check if coordinator is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return initialized_; }

private:
    bool initialized_;
    FederatedMesh* federatedMesh_;
    
    // Data storage
    std::vector<TrainingRound> activeRounds_;
    std::vector<FederatedParticipant> participants_;
    std::map<String, std::vector<ModelUpdate>> roundUpdates_;
    
    // Statistics
    uint32_t totalRoundsCompleted_;
    uint32_t totalModelUpdatesProcessed_;
    uint32_t totalParticipantsRegistered_;
    float averageConvergenceTime_;

    /**
     * @brief Find training round by ID
     * @param roundId Training round identifier
     * @return Pointer to training round or nullptr if not found
     */
    TrainingRound* findTrainingRound(const String& roundId);

    /**
     * @brief Find participant by node ID
     * @param nodeId Node identifier
     * @return Pointer to participant or nullptr if not found
     */
    FederatedParticipant* findParticipant(const String& nodeId);

    /**
     * @brief Generate unique round ID
     * @return Unique training round identifier
     */
    String generateRoundId();

    /**
     * @brief Validate model update
     * @param modelUpdate Model update to validate
     * @return true if valid
     */
    bool validateModelUpdate(const ModelUpdate& modelUpdate);

    /**
     * @brief Calculate convergence score
     * @param previousModel Previous global model
     * @param currentModel Current global model
     * @return Convergence score (0.0-1.0)
     */
    float calculateConvergenceScore(const GlobalModel& previousModel,
                                   const GlobalModel& currentModel);

    /**
     * @brief Apply differential privacy noise
     * @param modelWeights Model weights to add noise to
     * @param epsilon Privacy parameter epsilon
     * @return Noisy model weights
     */
    String applyDifferentialPrivacy(const String& modelWeights, float epsilon);

    /**
     * @brief Evaluate participant contribution quality
     * @param nodeId Participant node ID
     * @param modelUpdate Model update from participant
     * @return Contribution quality score (0.0-1.0)
     */
    float evaluateContributionQuality(const String& nodeId, const ModelUpdate& modelUpdate);

    /**
     * @brief Log federated learning event
     * @param event Event description
     * @param roundId Associated round ID (if applicable)
     */
    void logFederatedEvent(const String& event, const String& roundId = "");
};

// Global federated learning coordinator instance
extern FederatedLearningCoordinator* g_federatedCoordinator;

// Utility functions for easy integration
namespace FederatedLearningUtils {
    /**
     * @brief Initialize global coordinator
     * @return true if successful
     */
    bool initializeGlobalCoordinator();

    /**
     * @brief Quick start training round for species classification
     * @return Training round ID if successful
     */
    String quickStartSpeciesClassification();

    /**
     * @brief Quick participant registration with current device
     * @return true if successful
     */
    bool quickRegisterCurrentDevice();

    /**
     * @brief Get coordinator status summary
     * @return JSON string with status summary
     */
    String getCoordinatorStatusSummary();
}

#endif // FEDERATED_LEARNING_COORDINATOR_H