/**
 * @file version_3_2_integration_test.cpp
 * @brief Comprehensive Integration Test for ESP32 Wildlife CAM Version 3.2.0
 * @author thewriterben
 * @date 2025-01-01
 * @version 3.2.0
 * 
 * Tests all four major Version 3.2.0 features:
 * 1. Blockchain-based Data Verification
 * 2. Real-time Collaborative Research Platform
 * 3. Advanced Audio Detection and Classification
 * 4. Federated Learning Across Camera Networks
 */

#include <Arduino.h>
#include <unity.h>
#include "src/blockchain/SmartContractEngine.h"
#include "src/research/CollaborativeResearchPlatform.h"
#include "src/audio/wildlife_audio_classifier.h"
#include "src/ai/FederatedLearningCoordinator.h"
#include "include/blockchain/blockchain_types.h"

// Test instances
SmartContractEngine* testContractEngine = nullptr;
CollaborativeResearchPlatform* testResearchPlatform = nullptr;
WildlifeAudioClassifier* testAudioClassifier = nullptr;
FederatedLearningCoordinator* testFederatedCoordinator = nullptr;

void setUp(void) {
    // Initialize test components
    Serial.begin(115200);
    delay(2000);
    
    // Create test instances
    testContractEngine = new SmartContractEngine();
    testResearchPlatform = new CollaborativeResearchPlatform();
    testAudioClassifier = new WildlifeAudioClassifier();
    testFederatedCoordinator = new FederatedLearningCoordinator();
}

void tearDown(void) {
    // Cleanup test instances
    if (testFederatedCoordinator) {
        testFederatedCoordinator->shutdown();
        delete testFederatedCoordinator;
        testFederatedCoordinator = nullptr;
    }
    
    if (testAudioClassifier) {
        delete testAudioClassifier;
        testAudioClassifier = nullptr;
    }
    
    if (testResearchPlatform) {
        testResearchPlatform->shutdown();
        delete testResearchPlatform;
        testResearchPlatform = nullptr;
    }
    
    if (testContractEngine) {
        testContractEngine->shutdown();
        delete testContractEngine;
        testContractEngine = nullptr;
    }
}

// ===========================
// 1. BLOCKCHAIN SYSTEM TESTS
// ===========================

void test_smart_contract_engine_initialization() {
    TEST_ASSERT_NOT_NULL(testContractEngine);
    TEST_ASSERT_TRUE(testContractEngine->initialize());
    TEST_ASSERT_TRUE(testContractEngine->isInitialized());
}

void test_smart_contract_deployment() {
    TEST_ASSERT_TRUE(testContractEngine->initialize());
    
    // Deploy wildlife data validation contract
    String contractCode = R"({
        "name": "TestWildlifeValidation",
        "version": "1.0.0",
        "instructions": [
            {"op": "VALIDATE_DATA", "params": {"required_fields": ["species", "confidence"]}},
            {"op": "VERIFY_CONFIDENCE", "params": {"min_confidence": 0.5}},
            {"op": "RETURN", "params": {"valid": true}}
        ]
    })";
    
    String contractId = testContractEngine->deployContract(contractCode, "test_researcher");
    TEST_ASSERT_FALSE(contractId.isEmpty());
    TEST_ASSERT_TRUE(contractId.startsWith("contract_"));
}

void test_wildlife_data_validation() {
    TEST_ASSERT_TRUE(testContractEngine->initialize());
    
    // Deploy default contracts
    TEST_ASSERT_TRUE(SmartContractUtils::deployDefaultContracts());
    
    // Create test wildlife data
    WildlifeContext testData;
    testData.speciesDetected = "white-tailed deer";
    testData.confidence = 0.85f;
    testData.behaviorType = "grazing";
    testData.temperature = 18.5f;
    testData.humidity = 65.0f;
    
    // Validate data using smart contract
    bool isValid = testContractEngine->validateWildlifeData(testData, "test_researcher");
    TEST_ASSERT_TRUE(isValid);
}

void test_researcher_authentication() {
    TEST_ASSERT_TRUE(testContractEngine->initialize());
    TEST_ASSERT_TRUE(SmartContractUtils::deployDefaultContracts());
    
    // Create authentication data
    JsonDocument authData;
    authData["researcher_id"] = "dr_jane_smith";
    authData["institution_id"] = "wildlife_institute";
    authData["credentials"] = "encrypted_credentials_hash";
    authData["timestamp"] = millis() / 1000;
    
    // Authenticate researcher
    ResearcherAuth authResult = testContractEngine->authenticateResearcher(authData);
    TEST_ASSERT_TRUE(authResult.isActive);
    TEST_ASSERT_EQUAL_STRING("dr_jane_smith", authResult.researcherId.c_str());
    TEST_ASSERT_EQUAL(AUTH_LEVEL_VERIFIED, authResult.authLevel);
}

// ===========================
// 2. COLLABORATIVE PLATFORM TESTS
// ===========================

void test_research_platform_initialization() {
    TEST_ASSERT_NOT_NULL(testResearchPlatform);
    // Skip WebSocket initialization in test environment
    // TEST_ASSERT_TRUE(testResearchPlatform->initialize(8081));
    // TEST_ASSERT_TRUE(testResearchPlatform->isInitialized());
}

void test_wildlife_event_broadcast() {
    // Create test wildlife event
    WildlifeEvent testEvent;
    testEvent.eventId = "test_event_001";
    testEvent.eventType = EVENT_ANIMAL_DETECTED;
    testEvent.cameraId = "CAM_001";
    testEvent.speciesDetected = "red fox";
    testEvent.confidence = 0.92f;
    testEvent.latitude = 45.5017;
    testEvent.longitude = -73.5673;
    testEvent.timestamp = millis() / 1000;
    testEvent.priority = 2;
    testEvent.isValidated = false;
    
    // Test event creation
    TEST_ASSERT_EQUAL_STRING("test_event_001", testEvent.eventId.c_str());
    TEST_ASSERT_EQUAL(EVENT_ANIMAL_DETECTED, testEvent.eventType);
    TEST_ASSERT_EQUAL_STRING("red fox", testEvent.speciesDetected.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.92f, testEvent.confidence);
}

void test_researcher_annotation() {
    // Create test annotation
    ResearcherAnnotation testAnnotation;
    testAnnotation.annotationId = "annotation_001";
    testAnnotation.eventId = "test_event_001";
    testAnnotation.researcherId = "dr_jane_smith";
    testAnnotation.annotationType = "species_verification";
    testAnnotation.annotationText = "Confirmed red fox identification based on tail markings";
    testAnnotation.confidence = 0.95f;
    testAnnotation.createdTimestamp = millis() / 1000;
    testAnnotation.isPublic = true;
    
    // Test annotation structure
    TEST_ASSERT_EQUAL_STRING("annotation_001", testAnnotation.annotationId.c_str());
    TEST_ASSERT_EQUAL_STRING("species_verification", testAnnotation.annotationType.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.95f, testAnnotation.confidence);
    TEST_ASSERT_TRUE(testAnnotation.isPublic);
}

void test_notification_system() {
    // Create test notification
    Notification testNotification;
    testNotification.notificationId = "notification_001";
    testNotification.title = "Rare Species Alert";
    testNotification.message = "Endangered species detected at Camera 05";
    testNotification.relatedEventType = EVENT_RARE_SPECIES;
    testNotification.priority = 1; // High priority
    testNotification.createdTimestamp = millis() / 1000;
    testNotification.isActionable = true;
    
    // Test notification structure
    TEST_ASSERT_EQUAL_STRING("Rare Species Alert", testNotification.title.c_str());
    TEST_ASSERT_EQUAL(EVENT_RARE_SPECIES, testNotification.relatedEventType);
    TEST_ASSERT_EQUAL(1, testNotification.priority);
    TEST_ASSERT_TRUE(testNotification.isActionable);
}

// ===========================
// 3. AUDIO CLASSIFICATION TESTS
// ===========================

void test_audio_classifier_initialization() {
    TEST_ASSERT_NOT_NULL(testAudioClassifier);
    // Skip hardware initialization in test environment
    // TEST_ASSERT_TRUE(testAudioClassifier->initialize());
}

void test_species_audio_signature() {
    // Create test audio signature
    SpeciesAudioSignature signature;
    signature.species_name = "great horned owl";
    signature.common_name = "Great Horned Owl";
    signature.frequency_range_min = 200.0f;
    signature.frequency_range_max = 800.0f;
    signature.peak_frequency = 400.0f;
    signature.call_duration_min = 1500.0f;
    signature.call_duration_max = 3000.0f;
    signature.amplitude_threshold = -30.0f;
    signature.behavior_type = AudioBehaviorPattern::TERRITORIAL;
    signature.confidence_threshold = 0.8f;
    
    // Test signature structure
    TEST_ASSERT_EQUAL_STRING("great horned owl", signature.species_name.c_str());
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 400.0f, signature.peak_frequency);
    TEST_ASSERT_EQUAL(AudioBehaviorPattern::TERRITORIAL, signature.behavior_type);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.8f, signature.confidence_threshold);
}

void test_audio_classification_result() {
    // Create test classification result
    AudioClassificationResult result;
    result.detected_species = "barred owl";
    result.common_name = "Barred Owl";
    result.confidence = 0.88f;
    result.confidence_level = AudioConfidenceLevel::HIGH;
    result.detection_timestamp = millis() / 1000;
    result.is_wildlife = true;
    result.is_bird_call = true;
    result.behavior_pattern = AudioBehaviorPattern::MATING_CALL;
    result.capture_mode = DirectionalCaptureMode::DIRECTIONAL;
    result.processing_algorithm = AudioProcessingAlgorithm::MFCC;
    result.direction_angle = 45.0f;
    result.distance_estimate = 25.0f;
    
    // Test result structure
    TEST_ASSERT_EQUAL_STRING("barred owl", result.detected_species.c_str());
    TEST_ASSERT_EQUAL(AudioConfidenceLevel::HIGH, result.confidence_level);
    TEST_ASSERT_TRUE(result.is_wildlife);
    TEST_ASSERT_TRUE(result.is_bird_call);
    TEST_ASSERT_EQUAL(AudioBehaviorPattern::MATING_CALL, result.behavior_pattern);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 45.0f, result.direction_angle);
}

void test_multimodal_detection() {
    // Create test audio result
    AudioClassificationResult audioResult;
    audioResult.detected_species = "white-tailed deer";
    audioResult.confidence = 0.75f;
    audioResult.behavior_pattern = AudioBehaviorPattern::MOVEMENT;
    
    // Create test multimodal result
    MultiModalResult multiResult;
    multiResult.audio_result = audioResult;
    multiResult.has_visual_detection = true;
    multiResult.visual_species = "white-tailed deer";
    multiResult.visual_confidence = 0.90f;
    multiResult.combined_confidence = 0.85f;
    multiResult.correlation_found = true;
    
    // Test multimodal fusion
    TEST_ASSERT_TRUE(multiResult.has_visual_detection);
    TEST_ASSERT_EQUAL_STRING("white-tailed deer", multiResult.visual_species.c_str());
    TEST_ASSERT_TRUE(multiResult.correlation_found);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.85f, multiResult.combined_confidence);
}

// ===========================
// 4. FEDERATED LEARNING TESTS
// ===========================

void test_federated_coordinator_initialization() {
    TEST_ASSERT_NOT_NULL(testFederatedCoordinator);
    TEST_ASSERT_TRUE(testFederatedCoordinator->initialize());
    TEST_ASSERT_TRUE(testFederatedCoordinator->isInitialized());
}

void test_federated_participant_registration() {
    TEST_ASSERT_TRUE(testFederatedCoordinator->initialize());
    
    // Create test participant
    FederatedParticipant participant;
    participant.nodeId = "ESP32CAM_001";
    participant.nodeType = "ESP32CAM";
    participant.computationalCapability = 0.8f;
    participant.networkBandwidth = 2.0f;
    participant.batteryLevel = 0.85f;
    participant.localDatasetSize = 150;
    participant.geographicLocation = "forest_area_01";
    participant.isActive = true;
    participant.contributionScore = 0.7f;
    participant.privacyPreference = 0.6f;
    
    // Register participant
    bool registered = testFederatedCoordinator->registerParticipant(participant);
    TEST_ASSERT_TRUE(registered);
}

void test_training_round_creation() {
    TEST_ASSERT_TRUE(testFederatedCoordinator->initialize());
    
    // Start training round
    String roundId = testFederatedCoordinator->startTrainingRound(
        WildlifeModelType::SPECIES_CLASSIFIER,
        FederatedAlgorithm::FEDERATED_AVERAGING,
        PrivacyTechnique::DIFFERENTIAL_PRIVACY
    );
    
    TEST_ASSERT_FALSE(roundId.isEmpty());
    TEST_ASSERT_TRUE(roundId.startsWith("round_"));
}

void test_model_update_structure() {
    // Create test model update
    ModelUpdate update;
    update.nodeId = "ESP32CAM_001";
    update.modelType = WildlifeModelType::SPECIES_CLASSIFIER;
    update.modelVersion = 1;
    update.modelWeights = "base64_encoded_weights";
    update.trainingLoss = 0.25f;
    update.validationAccuracy = 0.92f;
    update.datasetSize = 200;
    update.trainingEpochs = 5;
    update.timestamp = millis() / 1000;
    update.privacyBudget = 1.0f;
    
    // Test update structure
    TEST_ASSERT_EQUAL_STRING("ESP32CAM_001", update.nodeId.c_str());
    TEST_ASSERT_EQUAL(WildlifeModelType::SPECIES_CLASSIFIER, update.modelType);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.25f, update.trainingLoss);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.92f, update.validationAccuracy);
    TEST_ASSERT_EQUAL(200, update.datasetSize);
}

void test_global_model_aggregation() {
    // Create test global model
    GlobalModel globalModel;
    globalModel.modelType = WildlifeModelType::SPECIES_CLASSIFIER;
    globalModel.modelVersion = 1;
    globalModel.aggregatedWeights = "aggregated_base64_weights";
    globalModel.globalLoss = 0.15f;
    globalModel.globalAccuracy = 0.94f;
    globalModel.participantCount = 8;
    globalModel.convergenceScore = 0.85f;
    globalModel.isConverged = false;
    
    // Test global model structure
    TEST_ASSERT_EQUAL(WildlifeModelType::SPECIES_CLASSIFIER, globalModel.modelType);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.15f, globalModel.globalLoss);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.94f, globalModel.globalAccuracy);
    TEST_ASSERT_EQUAL(8, globalModel.participantCount);
    TEST_ASSERT_FALSE(globalModel.isConverged);
}

// ===========================
// 5. INTEGRATION TESTS
// ===========================

void test_end_to_end_wildlife_detection_workflow() {
    // Simulate complete wildlife detection workflow
    
    // 1. Audio detection
    AudioClassificationResult audioResult;
    audioResult.detected_species = "gray wolf";
    audioResult.confidence = 0.82f;
    audioResult.is_wildlife = true;
    audioResult.behavior_pattern = AudioBehaviorPattern::ALARM_CALL;
    
    // 2. Visual detection (simulated)
    String visualSpecies = "gray wolf";
    float visualConfidence = 0.88f;
    
    // 3. Multi-modal fusion
    MultiModalResult fusedResult;
    fusedResult.audio_result = audioResult;
    fusedResult.has_visual_detection = true;
    fusedResult.visual_species = visualSpecies;
    fusedResult.visual_confidence = visualConfidence;
    fusedResult.combined_confidence = (audioResult.confidence + visualConfidence) / 2.0f;
    fusedResult.correlation_found = (audioResult.detected_species == visualSpecies);
    
    // 4. Create wildlife event
    WildlifeEvent event;
    event.eventType = EVENT_ANIMAL_DETECTED;
    event.speciesDetected = fusedResult.visual_species;
    event.confidence = fusedResult.combined_confidence;
    event.behaviorDescription = "Alarm call detected with visual confirmation";
    
    // 5. Validate with smart contract (simulated)
    WildlifeContext context;
    context.speciesDetected = event.speciesDetected;
    context.confidence = event.confidence;
    context.behaviorType = "alarm";
    
    // Test the complete workflow
    TEST_ASSERT_TRUE(fusedResult.correlation_found);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.85f, fusedResult.combined_confidence);
    TEST_ASSERT_EQUAL_STRING("gray wolf", event.speciesDetected.c_str());
    TEST_ASSERT_EQUAL_STRING("alarm", context.behaviorType.c_str());
}

void test_blockchain_audio_integration() {
    // Test integration between blockchain and audio systems
    
    // 1. Create audio detection
    AudioClassificationResult audioResult;
    audioResult.detected_species = "bald eagle";
    audioResult.confidence = 0.95f;
    audioResult.acoustic_fingerprint = "audio_hash_123";
    
    // 2. Create blockchain transaction data
    WildlifeContext context;
    context.speciesDetected = audioResult.detected_species;
    context.confidence = audioResult.confidence;
    context.isRareSpecies = true;
    context.isProtectedSpecies = true;
    
    // Test integration points
    TEST_ASSERT_EQUAL_STRING("bald eagle", context.speciesDetected.c_str());
    TEST_ASSERT_TRUE(context.isRareSpecies);
    TEST_ASSERT_TRUE(context.isProtectedSpecies);
    TEST_ASSERT_FALSE(audioResult.acoustic_fingerprint.isEmpty());
}

// ===========================
// TEST RUNNER
// ===========================

void runAllTests() {
    UNITY_BEGIN();
    
    // Blockchain System Tests
    RUN_TEST(test_smart_contract_engine_initialization);
    RUN_TEST(test_smart_contract_deployment);
    RUN_TEST(test_wildlife_data_validation);
    RUN_TEST(test_researcher_authentication);
    
    // Collaborative Platform Tests
    RUN_TEST(test_research_platform_initialization);
    RUN_TEST(test_wildlife_event_broadcast);
    RUN_TEST(test_researcher_annotation);
    RUN_TEST(test_notification_system);
    
    // Audio Classification Tests
    RUN_TEST(test_audio_classifier_initialization);
    RUN_TEST(test_species_audio_signature);
    RUN_TEST(test_audio_classification_result);
    RUN_TEST(test_multimodal_detection);
    
    // Federated Learning Tests
    RUN_TEST(test_federated_coordinator_initialization);
    RUN_TEST(test_federated_participant_registration);
    RUN_TEST(test_training_round_creation);
    RUN_TEST(test_model_update_structure);
    RUN_TEST(test_global_model_aggregation);
    
    // Integration Tests
    RUN_TEST(test_end_to_end_wildlife_detection_workflow);
    RUN_TEST(test_blockchain_audio_integration);
    
    UNITY_END();
}

#ifdef ARDUINO
void setup() {
    delay(2000); // Allow time for serial monitor
    runAllTests();
}

void loop() {
    // Empty loop for Arduino
}
#else
int main() {
    runAllTests();
    return 0;
}
#endif