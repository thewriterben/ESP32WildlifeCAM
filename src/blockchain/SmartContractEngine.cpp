/**
 * @file SmartContractEngine.cpp
 * @brief Smart Contract Engine Implementation
 * @author thewriterben
 * @date 2025-01-01
 * @version 3.2.0
 */

#include "SmartContractEngine.h"
#include "../utils/logger.h"
#include <MD5Builder.h>

// Global smart contract engine instance
SmartContractEngine* g_smartContractEngine = nullptr;

SmartContractEngine::SmartContractEngine() 
    : initialized_(false), totalExecutions_(0), successfulExecutions_(0),
      failedExecutions_(0), totalGasUsed_(0) {
}

SmartContractEngine::~SmartContractEngine() {
    shutdown();
}

bool SmartContractEngine::initialize() {
    if (initialized_) {
        return true;
    }

    Logger::info("Initializing Smart Contract Engine v3.2.0");

    // Initialize contract storage
    deployedContracts_.clear();
    
    // Reset statistics
    totalExecutions_ = 0;
    successfulExecutions_ = 0;
    failedExecutions_ = 0;
    totalGasUsed_ = 0;

    // Deploy default contracts
    if (!SmartContractUtils::deployDefaultContracts()) {
        Logger::error("Failed to deploy default smart contracts");
        return false;
    }

    initialized_ = true;
    Logger::info("Smart Contract Engine initialized successfully");
    return true;
}

void SmartContractEngine::shutdown() {
    if (!initialized_) {
        return;
    }

    Logger::info("Shutting down Smart Contract Engine");
    
    // Clear deployed contracts
    deployedContracts_.clear();
    
    initialized_ = false;
    Logger::info("Smart Contract Engine shutdown complete");
}

String SmartContractEngine::deployContract(const String& contractCode, const String& createdBy) {
    if (!initialized_) {
        Logger::error("Smart Contract Engine not initialized");
        return "";
    }

    // Validate contract code
    if (!validateContractCode(contractCode)) {
        Logger::error("Invalid contract code provided");
        return "";
    }

    // Generate unique contract ID
    String contractId = generateContractId();

    // Create smart contract structure
    SmartContract contract;
    contract.contractId = contractId;
    contract.contractCode = contractCode;
    contract.state = CONTRACT_STATE_DEPLOYED;
    contract.createdTimestamp = millis() / 1000;
    contract.createdBy = createdBy;

    // Add to deployed contracts
    deployedContracts_.push_back(contract);

    Logger::info("Smart contract deployed: %s by %s", contractId.c_str(), createdBy.c_str());
    return contractId;
}

ContractExecutionContext SmartContractEngine::executeContract(const String& contractId, 
                                                            const JsonDocument& inputData,
                                                            const String& executorId) {
    ContractExecutionContext context;
    context.contractId = contractId;
    context.executorId = executorId;
    context.inputData = inputData;
    context.executionStartTime = millis();

    totalExecutions_++;

    if (!initialized_) {
        context.errorMessage = "Smart Contract Engine not initialized";
        failedExecutions_++;
        return context;
    }

    // Find contract
    SmartContract* contract = findContract(contractId);
    if (!contract) {
        context.errorMessage = "Contract not found: " + contractId;
        failedExecutions_++;
        return context;
    }

    // Update contract state
    contract->state = CONTRACT_STATE_EXECUTING;
    contract->lastExecutionTimestamp = context.executionStartTime / 1000;

    // Parse contract code
    JsonDocument contractDoc;
    DeserializationError error = deserializeJson(contractDoc, contract->contractCode);
    if (error) {
        context.errorMessage = "Failed to parse contract code: " + String(error.c_str());
        contract->state = CONTRACT_STATE_FAILED;
        failedExecutions_++;
        return context;
    }

    // Execute contract instructions
    JsonArray instructions = contractDoc["instructions"];
    for (JsonObject instruction : instructions) {
        String opName = instruction["op"];
        JsonObject params = instruction["params"];

        ContractOpcode opcode = OP_NOP;
        if (opName == "VALIDATE_DATA") opcode = OP_VALIDATE_DATA;
        else if (opName == "AUTH_RESEARCHER") opcode = OP_AUTH_RESEARCHER;
        else if (opName == "CHECK_SPECIES") opcode = OP_CHECK_SPECIES;
        else if (opName == "VERIFY_CONFIDENCE") opcode = OP_VERIFY_CONFIDENCE;
        else if (opName == "CHECK_TIMESTAMP") opcode = OP_CHECK_TIMESTAMP;
        else if (opName == "VALIDATE_PROVENANCE") opcode = OP_VALIDATE_PROVENANCE;
        else if (opName == "CONSENSUS_CHECK") opcode = OP_CONSENSUS_CHECK;
        else if (opName == "IMMUTABLE_CHECK") opcode = OP_IMMUTABLE_CHECK;
        else if (opName == "RETURN") opcode = OP_RETURN;

        if (!executeInstruction(opcode, params, context)) {
            contract->state = CONTRACT_STATE_FAILED;
            contract->executionError = context.errorMessage;
            failedExecutions_++;
            return context;
        }

        // Check for early return
        if (opcode == OP_RETURN) {
            break;
        }

        // Check gas limit
        if (context.gasUsed >= context.gasLimit) {
            context.errorMessage = "Gas limit exceeded";
            contract->state = CONTRACT_STATE_FAILED;
            failedExecutions_++;
            return context;
        }
    }

    // Execution completed successfully
    context.executionComplete = true;
    contract->state = CONTRACT_STATE_COMPLETED;
    contract->gasUsed = context.gasUsed;
    totalGasUsed_ += context.gasUsed;
    successfulExecutions_++;

    logExecutionEvent(contractId, "Contract executed successfully", context);
    return context;
}

bool SmartContractEngine::validateWildlifeData(const WildlifeContext& wildlifeData, 
                                             const String& researcherId) {
    // Create input data for validation contract
    JsonDocument inputData;
    inputData["species"] = wildlifeData.speciesDetected;
    inputData["confidence"] = wildlifeData.confidence;
    inputData["timestamp"] = millis() / 1000;
    inputData["researcher_id"] = researcherId;
    inputData["behavior"] = wildlifeData.behaviorType;
    inputData["is_rare"] = wildlifeData.isRareSpecies;
    inputData["is_protected"] = wildlifeData.isProtectedSpecies;

    // Find wildlife data validation contract
    for (const auto& contract : deployedContracts_) {
        JsonDocument contractDoc;
        deserializeJson(contractDoc, contract.contractCode);
        if (contractDoc["name"] == "WildlifeDataValidation") {
            ContractExecutionContext result = executeContract(contract.contractId, inputData, researcherId);
            return result.executionComplete && !result.errorMessage.length() > 0;
        }
    }

    Logger::warning("Wildlife data validation contract not found");
    return false;
}

ResearcherAuth SmartContractEngine::authenticateResearcher(const JsonDocument& authData) {
    ResearcherAuth auth;
    
    // Find researcher authentication contract
    for (const auto& contract : deployedContracts_) {
        JsonDocument contractDoc;
        deserializeJson(contractDoc, contract.contractCode);
        if (contractDoc["name"] == "ResearcherAuthentication") {
            String researcherId = authData["researcher_id"];
            ContractExecutionContext result = executeContract(contract.contractId, authData, researcherId);
            
            if (result.executionComplete && result.errorMessage.length() == 0) {
                auth.researcherId = researcherId;
                auth.institutionId = authData["institution_id"];
                auth.authLevel = AUTH_LEVEL_VERIFIED;
                auth.authTimestamp = millis() / 1000;
                auth.expirationTimestamp = auth.authTimestamp + (24 * 3600); // 24 hours
                auth.isActive = true;
                auth.permissions = "{\"read\": true, \"write\": true, \"validate\": true}";
            }
            break;
        }
    }

    return auth;
}

bool SmartContractEngine::executeInstruction(ContractOpcode opcode, 
                                           const JsonDocument& params,
                                           ContractExecutionContext& context) {
    // Consume base gas for instruction execution
    if (!consumeGas(10, context)) {
        return false;
    }

    switch (opcode) {
        case OP_NOP:
            // No operation
            break;
            
        case OP_VALIDATE_DATA: {
            JsonArray requiredFields = params["required_fields"];
            for (JsonVariant field : requiredFields) {
                if (!context.inputData.containsKey(field.as<String>())) {
                    context.errorMessage = "Missing required field: " + field.as<String>();
                    return false;
                }
            }
            consumeGas(50, context);
            break;
        }
        
        case OP_VERIFY_CONFIDENCE: {
            float minConfidence = params["min_confidence"];
            float actualConfidence = context.inputData["confidence"];
            if (actualConfidence < minConfidence) {
                context.errorMessage = "Confidence too low: " + String(actualConfidence) + " < " + String(minConfidence);
                return false;
            }
            consumeGas(20, context);
            break;
        }
        
        case OP_CHECK_TIMESTAMP: {
            uint32_t maxAgeHours = params["max_age_hours"];
            uint32_t timestamp = context.inputData["timestamp"];
            uint32_t currentTime = millis() / 1000;
            uint32_t ageSeconds = currentTime - timestamp;
            
            if (ageSeconds > (maxAgeHours * 3600)) {
                context.errorMessage = "Data too old: " + String(ageSeconds) + " seconds";
                return false;
            }
            consumeGas(15, context);
            break;
        }
        
        case OP_CHECK_SPECIES: {
            JsonArray allowedSpecies = params["allowed_species"];
            String species = context.inputData["species"];
            bool found = false;
            
            for (JsonVariant allowedSpecies : allowedSpecies) {
                if (allowedSpecies.as<String>() == species) {
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                context.errorMessage = "Species not in allowed list: " + species;
                return false;
            }
            consumeGas(30, context);
            break;
        }
        
        case OP_AUTH_RESEARCHER: {
            bool requireInstitution = params["require_institution"];
            if (requireInstitution && !context.inputData.containsKey("institution_id")) {
                context.errorMessage = "Institution ID required for authentication";
                return false;
            }
            consumeGas(100, context);
            break;
        }
        
        case OP_VALIDATE_PROVENANCE: {
            bool requireSignature = params["require_signature"];
            if (requireSignature && !context.inputData.containsKey("signature")) {
                context.errorMessage = "Digital signature required for provenance validation";
                return false;
            }
            consumeGas(75, context);
            break;
        }
        
        case OP_CONSENSUS_CHECK:
            // Placeholder for consensus checking
            consumeGas(200, context);
            break;
            
        case OP_IMMUTABLE_CHECK:
            // Placeholder for immutability checking
            consumeGas(50, context);
            break;
            
        case OP_RETURN: {
            // Copy return parameters to output
            for (JsonPair param : params.as<JsonObject>()) {
                context.outputData[param.key()] = param.value();
            }
            break;
        }
        
        default:
            context.errorMessage = "Unknown opcode: " + String(static_cast<int>(opcode));
            return false;
    }

    return true;
}

bool SmartContractEngine::validateContractCode(const String& contractCode) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, contractCode);
    if (error) {
        return false;
    }

    // Check required fields
    if (!doc.containsKey("name") || !doc.containsKey("version") || !doc.containsKey("instructions")) {
        return false;
    }

    // Validate contract size
    if (contractCode.length() > BLOCKCHAIN_MAX_CONTRACT_SIZE) {
        return false;
    }

    return true;
}

String SmartContractEngine::generateContractId() {
    MD5Builder md5;
    md5.begin();
    md5.add(String(millis()));
    md5.add(String(ESP.getChipId()));
    md5.add(String(deployedContracts_.size()));
    md5.calculate();
    return "contract_" + md5.toString().substring(0, 8);
}

SmartContract* SmartContractEngine::findContract(const String& contractId) {
    for (auto& contract : deployedContracts_) {
        if (contract.contractId == contractId) {
            return &contract;
        }
    }
    return nullptr;
}

bool SmartContractEngine::consumeGas(uint32_t gasAmount, ContractExecutionContext& context) {
    if (context.gasUsed + gasAmount > context.gasLimit) {
        context.errorMessage = "Insufficient gas";
        return false;
    }
    context.gasUsed += gasAmount;
    return true;
}

void SmartContractEngine::logExecutionEvent(const String& contractId, const String& event,
                                          const ContractExecutionContext& context) {
    Logger::info("Contract %s: %s (Gas: %d, Executor: %s)", 
                contractId.c_str(), event.c_str(), context.gasUsed, context.executorId.c_str());
}

String SmartContractEngine::getStatistics() {
    JsonDocument stats;
    stats["total_executions"] = totalExecutions_;
    stats["successful_executions"] = successfulExecutions_;
    stats["failed_executions"] = failedExecutions_;
    stats["success_rate"] = totalExecutions_ > 0 ? (float)successfulExecutions_ / totalExecutions_ : 0.0;
    stats["total_gas_used"] = totalGasUsed_;
    stats["deployed_contracts"] = deployedContracts_.size();
    stats["initialized"] = initialized_;

    String result;
    serializeJson(stats, result);
    return result;
}

// Utility functions implementation
namespace SmartContractUtils {
    bool initializeGlobalEngine() {
        if (!g_smartContractEngine) {
            g_smartContractEngine = new SmartContractEngine();
        }
        return g_smartContractEngine->initialize();
    }

    bool quickValidateWildlifeData(const WildlifeContext& wildlifeData,
                                  const String& researcherId) {
        if (!g_smartContractEngine || !g_smartContractEngine->isInitialized()) {
            return false;
        }
        return g_smartContractEngine->validateWildlifeData(wildlifeData, researcherId);
    }

    ResearcherAuth quickAuthenticateResearcher(const String& researcherId,
                                              const String& credentials) {
        ResearcherAuth auth;
        if (!g_smartContractEngine || !g_smartContractEngine->isInitialized()) {
            return auth;
        }

        JsonDocument authData;
        authData["researcher_id"] = researcherId;
        authData["credentials"] = credentials;
        authData["timestamp"] = millis() / 1000;

        return g_smartContractEngine->authenticateResearcher(authData);
    }

    bool deployDefaultContracts() {
        if (!g_smartContractEngine) {
            return false;
        }

        // Deploy wildlife data validation contract
        String validationContractId = g_smartContractEngine->deployContract(
            WildlifeDataValidationContract::CONTRACT_CODE, "system");
        if (validationContractId.isEmpty()) {
            Logger::error("Failed to deploy wildlife data validation contract");
            return false;
        }

        // Deploy researcher authentication contract
        String authContractId = g_smartContractEngine->deployContract(
            ResearcherAuthContract::CONTRACT_CODE, "system");
        if (authContractId.isEmpty()) {
            Logger::error("Failed to deploy researcher authentication contract");
            return false;
        }

        Logger::info("Default smart contracts deployed successfully");
        return true;
    }
}