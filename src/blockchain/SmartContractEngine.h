/**
 * @file SmartContractEngine.h
 * @brief Smart Contract Engine for ESP32 Wildlife CAM Blockchain
 * @author thewriterben
 * @date 2025-01-01
 * @version 3.2.0
 * 
 * Implements a lightweight smart contract execution engine optimized for ESP32
 * constraints. Supports data validation contracts, researcher authentication,
 * and wildlife observation verification protocols.
 */

#ifndef SMART_CONTRACT_ENGINE_H
#define SMART_CONTRACT_ENGINE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "../../include/blockchain/blockchain_types.h"
#include "../../include/blockchain/blockchain_config.h"

/**
 * @brief Smart contract instruction opcodes
 */
enum ContractOpcode {
    OP_NOP = 0x00,                      // No operation
    OP_VALIDATE_DATA = 0x01,            // Validate wildlife data
    OP_AUTH_RESEARCHER = 0x02,          // Authenticate researcher
    OP_CHECK_SPECIES = 0x03,            // Check species classification
    OP_VERIFY_CONFIDENCE = 0x04,        // Verify AI confidence threshold
    OP_CHECK_TIMESTAMP = 0x05,          // Check timestamp validity
    OP_VALIDATE_PROVENANCE = 0x06,      // Validate data provenance
    OP_CONSENSUS_CHECK = 0x07,          // Check consensus requirements
    OP_IMMUTABLE_CHECK = 0x08,          // Check immutability constraints
    OP_RETURN = 0xFF                    // Return from contract
};

/**
 * @brief Contract execution context
 */
struct ContractExecutionContext {
    String contractId;                   // Contract identifier
    String executorId;                   // Executor (researcher) identifier
    JsonDocument inputData;              // Input data for contract
    JsonDocument outputData;             // Output data from contract
    uint32_t gasLimit;                   // Gas limit for execution
    uint32_t gasUsed;                    // Gas consumed
    bool executionComplete;              // Execution completion flag
    String errorMessage;                 // Error message if failed
    uint32_t executionStartTime;         // Execution start timestamp
    
    ContractExecutionContext() : contractId(""), executorId(""), 
        gasLimit(BLOCKCHAIN_CONTRACT_GAS_LIMIT), gasUsed(0), executionComplete(false),
        errorMessage(""), executionStartTime(0) {}
};

/**
 * @brief Predefined smart contract for wildlife data validation
 */
struct WildlifeDataValidationContract {
    static constexpr const char* CONTRACT_CODE = R"({
        "name": "WildlifeDataValidation",
        "version": "1.0.0",
        "description": "Validates wildlife observation data integrity",
        "instructions": [
            {"op": "VALIDATE_DATA", "params": {"required_fields": ["species", "confidence", "timestamp"]}},
            {"op": "VERIFY_CONFIDENCE", "params": {"min_confidence": 0.7}},
            {"op": "CHECK_TIMESTAMP", "params": {"max_age_hours": 24}},
            {"op": "CHECK_SPECIES", "params": {"allowed_species": ["deer", "bear", "wolf", "eagle", "fox"]}},
            {"op": "RETURN", "params": {"valid": true}}
        ]
    })";
};

/**
 * @brief Predefined smart contract for researcher authentication
 */
struct ResearcherAuthContract {
    static constexpr const char* CONTRACT_CODE = R"({
        "name": "ResearcherAuthentication",
        "version": "1.0.0", 
        "description": "Authenticates researcher credentials and permissions",
        "instructions": [
            {"op": "AUTH_RESEARCHER", "params": {"require_institution": true}},
            {"op": "CHECK_TIMESTAMP", "params": {"max_age_hours": 1}},
            {"op": "VALIDATE_PROVENANCE", "params": {"require_signature": true}},
            {"op": "RETURN", "params": {"authenticated": true}}
        ]
    })";
};

/**
 * @brief Smart Contract Engine Class
 * 
 * Lightweight smart contract execution engine designed for ESP32 constraints.
 * Executes validation contracts for wildlife data and researcher authentication.
 */
class SmartContractEngine {
public:
    SmartContractEngine();
    ~SmartContractEngine();

    /**
     * @brief Initialize the smart contract engine
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Shutdown the smart contract engine
     */
    void shutdown();

    /**
     * @brief Deploy a smart contract
     * @param contractCode JSON contract code
     * @param createdBy Researcher ID who created the contract
     * @return Contract ID if successful, empty string if failed
     */
    String deployContract(const String& contractCode, const String& createdBy);

    /**
     * @brief Execute a smart contract
     * @param contractId Contract identifier
     * @param inputData Input data for contract execution
     * @param executorId Executor (researcher) identifier
     * @return Execution context with results
     */
    ContractExecutionContext executeContract(const String& contractId, 
                                           const JsonDocument& inputData,
                                           const String& executorId);

    /**
     * @brief Execute wildlife data validation contract
     * @param wildlifeData Wildlife observation data
     * @param researcherId Researcher requesting validation
     * @return true if data is valid
     */
    bool validateWildlifeData(const WildlifeContext& wildlifeData, 
                             const String& researcherId);

    /**
     * @brief Execute researcher authentication contract
     * @param authData Authentication data
     * @return Authentication result
     */
    ResearcherAuth authenticateResearcher(const JsonDocument& authData);

    /**
     * @brief Get contract information
     * @param contractId Contract identifier
     * @return Smart contract structure
     */
    SmartContract getContract(const String& contractId);

    /**
     * @brief List all deployed contracts
     * @return Vector of contract IDs
     */
    std::vector<String> listContracts();

    /**
     * @brief Remove a deployed contract
     * @param contractId Contract identifier
     * @param requesterId Requester identifier (must be creator or admin)
     * @return true if removal successful
     */
    bool removeContract(const String& contractId, const String& requesterId);

    /**
     * @brief Get engine statistics
     * @return JSON string with statistics
     */
    String getStatistics();

    /**
     * @brief Check if engine is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return initialized_; }

private:
    bool initialized_;
    std::vector<SmartContract> deployedContracts_;
    uint32_t totalExecutions_;
    uint32_t successfulExecutions_;
    uint32_t failedExecutions_;
    uint32_t totalGasUsed_;

    /**
     * @brief Execute a single contract instruction
     * @param opcode Instruction opcode
     * @param params Instruction parameters
     * @param context Execution context
     * @return true if execution successful
     */
    bool executeInstruction(ContractOpcode opcode, 
                          const JsonDocument& params,
                          ContractExecutionContext& context);

    /**
     * @brief Validate contract code
     * @param contractCode JSON contract code
     * @return true if valid
     */
    bool validateContractCode(const String& contractCode);

    /**
     * @brief Generate unique contract ID
     * @return Unique contract identifier
     */
    String generateContractId();

    /**
     * @brief Find contract by ID
     * @param contractId Contract identifier
     * @return Pointer to contract or nullptr if not found
     */
    SmartContract* findContract(const String& contractId);

    /**
     * @brief Consume gas during execution
     * @param gasAmount Amount of gas to consume
     * @param context Execution context
     * @return true if gas available, false if limit exceeded
     */
    bool consumeGas(uint32_t gasAmount, ContractExecutionContext& context);

    /**
     * @brief Log contract execution event
     * @param contractId Contract identifier
     * @param event Event description
     * @param context Execution context
     */
    void logExecutionEvent(const String& contractId, const String& event,
                          const ContractExecutionContext& context);
};

// Global smart contract engine instance
extern SmartContractEngine* g_smartContractEngine;

// Utility functions for easy integration
namespace SmartContractUtils {
    /**
     * @brief Initialize global smart contract engine
     * @return true if successful
     */
    bool initializeGlobalEngine();

    /**
     * @brief Quick wildlife data validation
     * @param wildlifeData Wildlife data to validate
     * @param researcherId Researcher ID
     * @return true if valid
     */
    bool quickValidateWildlifeData(const WildlifeContext& wildlifeData,
                                  const String& researcherId);

    /**
     * @brief Quick researcher authentication
     * @param researcherId Researcher identifier
     * @param credentials Encrypted credentials
     * @return Authentication result
     */
    ResearcherAuth quickAuthenticateResearcher(const String& researcherId,
                                              const String& credentials);

    /**
     * @brief Deploy default validation contracts
     * @return true if successful
     */
    bool deployDefaultContracts();
}

#endif // SMART_CONTRACT_ENGINE_H