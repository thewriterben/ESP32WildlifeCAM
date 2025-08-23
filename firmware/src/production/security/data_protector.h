#ifndef DATA_PROTECTOR_H
#define DATA_PROTECTOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <mbedtls/aes.h>
#include <mbedtls/sha256.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include "../../config.h"

/**
 * @brief Data Protection and Encryption System for Wildlife Camera Security
 * 
 * Provides comprehensive data protection including encryption, access control,
 * data anonymization, and privacy protection for sensitive wildlife data.
 */
class DataProtector {
public:
    /**
     * @brief Encryption algorithms supported
     */
    enum class EncryptionAlgorithm {
        AES_128_CBC = 0,
        AES_256_CBC = 1,
        AES_128_GCM = 2,
        AES_256_GCM = 3
    };

    /**
     * @brief Data classification levels
     */
    enum class DataClassification {
        PUBLIC = 0,         // Public research data
        INTERNAL = 1,       // Internal use only
        CONFIDENTIAL = 2,   // Confidential wildlife data
        RESTRICTED = 3,     // Restricted access (endangered species)
        TOP_SECRET = 4      // Highly sensitive conservation data
    };

    /**
     * @brief Access permission levels
     */
    enum class AccessLevel {
        NONE = 0,
        READ_ONLY = 1,
        READ_WRITE = 2,
        ADMIN = 3,
        SYSTEM = 4
    };

    /**
     * @brief Data anonymization methods
     */
    enum class AnonymizationMethod {
        LOCATION_BLUR = 0,      // Blur precise GPS coordinates
        TIMESTAMP_BLUR = 1,     // Blur precise timestamps
        IMAGE_REDACTION = 2,    // Redact sensitive image areas
        AUDIO_FILTERING = 3,    // Filter sensitive audio
        METADATA_REMOVAL = 4,   // Remove identifying metadata
        FULL_ANONYMIZATION = 5  // Complete anonymization
    };

    /**
     * @brief Encryption context information
     */
    struct EncryptionContext {
        EncryptionAlgorithm algorithm;
        String key_id;
        String initialization_vector;
        uint32_t timestamp;
        String device_id;
        DataClassification classification;
    };

    /**
     * @brief Access control entry
     */
    struct AccessControlEntry {
        String user_id;
        String role;
        AccessLevel access_level;
        std::vector<String> permitted_data_types;
        std::vector<String> permitted_locations;
        uint32_t expiration_timestamp;
        bool is_active;
        String granted_by;
        String reason;
    };

    /**
     * @brief Data protection policy
     */
    struct ProtectionPolicy {
        String policy_name;
        DataClassification classification;
        bool encryption_required;
        EncryptionAlgorithm encryption_algorithm;
        bool anonymization_required;
        std::vector<AnonymizationMethod> anonymization_methods;
        uint32_t retention_period_days;
        bool audit_required;
        std::vector<String> permitted_operations;
        float privacy_level; // 0.0 = none, 1.0 = maximum
    };

    /**
     * @brief Audit log entry
     */
    struct AuditLogEntry {
        String entry_id;
        uint32_t timestamp;
        String user_id;
        String operation;
        String data_type;
        String data_id;
        bool success;
        String result_message;
        String source_ip;
        String device_id;
        DataClassification data_classification;
    };

    /**
     * @brief Initialize data protector
     * @param enable_hardware_security Enable hardware security features if available
     * @return true if initialization successful
     */
    static bool initialize(bool enable_hardware_security = true);

    /**
     * @brief Encrypt data with specified algorithm
     * @param data Data to encrypt
     * @param classification Data classification level
     * @param algorithm Encryption algorithm to use
     * @return Encrypted data with context information
     */
    static String encryptData(const String& data, DataClassification classification, EncryptionAlgorithm algorithm = EncryptionAlgorithm::AES_256_GCM);

    /**
     * @brief Decrypt data using context information
     * @param encrypted_data Encrypted data with context
     * @param user_id User requesting decryption
     * @return Decrypted data (empty if access denied)
     */
    static String decryptData(const String& encrypted_data, const String& user_id = "system");

    /**
     * @brief Encrypt image data with location privacy protection
     * @param image_data Pointer to image data
     * @param image_size Size of image data
     * @param location_lat GPS latitude
     * @param location_lon GPS longitude
     * @param blur_location Apply location blurring
     * @return Encrypted image data with privacy protection
     */
    static std::vector<uint8_t> encryptImageData(const uint8_t* image_data, size_t image_size, 
                                                float location_lat, float location_lon, bool blur_location = true);

    /**
     * @brief Anonymize wildlife data for research sharing
     * @param data_json Wildlife data in JSON format
     * @param methods Anonymization methods to apply
     * @param privacy_level Privacy level (0.0 - 1.0)
     * @return Anonymized data suitable for research sharing
     */
    static DynamicJsonDocument anonymizeData(const DynamicJsonDocument& data_json, 
                                            const std::vector<AnonymizationMethod>& methods, 
                                            float privacy_level = 0.8f);

    /**
     * @brief Check access permissions for data operation
     * @param user_id User requesting access
     * @param operation Operation being requested
     * @param data_type Type of data being accessed
     * @param data_classification Classification of the data
     * @return true if access is permitted
     */
    static bool checkAccess(const String& user_id, const String& operation, 
                           const String& data_type, DataClassification data_classification);

    /**
     * @brief Add access control entry
     * @param entry Access control entry to add
     * @param granted_by User granting the access
     * @return true if entry added successfully
     */
    static bool addAccessControlEntry(const AccessControlEntry& entry, const String& granted_by);

    /**
     * @brief Revoke access for user
     * @param user_id User to revoke access for
     * @param revoked_by User revoking the access
     * @param reason Reason for revocation
     * @return true if access revoked successfully
     */
    static bool revokeAccess(const String& user_id, const String& revoked_by, const String& reason);

    /**
     * @brief Create data protection policy
     * @param policy Protection policy to create
     * @return true if policy created successfully
     */
    static bool createProtectionPolicy(const ProtectionPolicy& policy);

    /**
     * @brief Apply protection policy to data
     * @param data Data to protect
     * @param policy_name Name of policy to apply
     * @param user_id User applying the policy
     * @return Protected data according to policy
     */
    static String applyProtectionPolicy(const String& data, const String& policy_name, const String& user_id);

    /**
     * @brief Log data access or operation for audit
     * @param user_id User performing operation
     * @param operation Operation being performed
     * @param data_type Type of data involved
     * @param data_id Unique identifier of data
     * @param success Whether operation was successful
     * @param message Additional information
     */
    static void logAuditEvent(const String& user_id, const String& operation, 
                             const String& data_type, const String& data_id, 
                             bool success, const String& message = "");

    /**
     * @brief Get audit log entries
     * @param start_timestamp Start time for log query
     * @param end_timestamp End time for log query
     * @param user_id Filter by specific user (empty for all)
     * @return Vector of audit log entries
     */
    static std::vector<AuditLogEntry> getAuditLog(uint32_t start_timestamp, uint32_t end_timestamp, const String& user_id = "");

    /**
     * @brief Generate secure hash of data
     * @param data Data to hash
     * @param include_salt Include random salt in hash
     * @return SHA-256 hash of data
     */
    static String generateSecureHash(const String& data, bool include_salt = true);

    /**
     * @brief Verify data integrity using hash
     * @param data Original data
     * @param hash Hash to verify against
     * @return true if data integrity is verified
     */
    static bool verifyDataIntegrity(const String& data, const String& hash);

    /**
     * @brief Securely delete sensitive data
     * @param data_path Path to data file
     * @param overwrite_passes Number of overwrite passes
     * @return true if secure deletion successful
     */
    static bool secureDelete(const String& data_path, uint8_t overwrite_passes = 3);

    /**
     * @brief Export encrypted backup of protected data
     * @param backup_path Path to save backup
     * @param master_key Master encryption key for backup
     * @return true if backup created successfully
     */
    static bool createEncryptedBackup(const String& backup_path, const String& master_key);

    /**
     * @brief Restore from encrypted backup
     * @param backup_path Path to backup file
     * @param master_key Master decryption key
     * @return true if restore successful
     */
    static bool restoreFromBackup(const String& backup_path, const String& master_key);

    /**
     * @brief Get data protection statistics
     * @return JSON object with protection statistics
     */
    static DynamicJsonDocument getProtectionStatistics();

    /**
     * @brief Process data protection tasks (call in main loop)
     */
    static void process();

    /**
     * @brief Generate random encryption key
     * @param key_length Length of key in bytes
     * @return Randomly generated encryption key
     */
    static String generateRandomKey(uint16_t key_length = 32);

    /**
     * @brief Rotate encryption keys
     * @param old_key_id ID of key to rotate
     * @return ID of new key
     */
    static String rotateEncryptionKey(const String& old_key_id);

    /**
     * @brief Enable/disable data protection features
     * @param encryption_enabled Enable encryption
     * @param access_control_enabled Enable access control
     * @param audit_enabled Enable audit logging
     */
    static void setProtectionSettings(bool encryption_enabled, bool access_control_enabled, bool audit_enabled);

private:
    static bool initialized_;
    static bool hardware_security_available_;
    static bool encryption_enabled_;
    static bool access_control_enabled_;
    static bool audit_enabled_;
    
    static std::vector<AccessControlEntry> access_control_list_;
    static std::vector<ProtectionPolicy> protection_policies_;
    static std::vector<AuditLogEntry> audit_log_;
    static std::map<String, String> encryption_keys_;
    
    // Crypto contexts
    static mbedtls_entropy_context entropy_;
    static mbedtls_ctr_drbg_context ctr_drbg_;
    static mbedtls_aes_context aes_context_;
    
    // Internal crypto operations
    static bool initializeCrypto();
    static std::vector<uint8_t> generateIV(size_t length);
    static String encryptAES(const String& data, const String& key, const String& iv, EncryptionAlgorithm algorithm);
    static String decryptAES(const String& encrypted_data, const String& key, const String& iv, EncryptionAlgorithm algorithm);
    
    // Access control helpers
    static bool loadAccessControlList();
    static bool saveAccessControlList();
    static bool isAccessExpired(const AccessControlEntry& entry);
    static AccessLevel getEffectiveAccessLevel(const String& user_id, const String& data_type);
    
    // Anonymization helpers
    static DynamicJsonDocument blurLocation(const DynamicJsonDocument& data, float blur_radius_km);
    static DynamicJsonDocument blurTimestamp(const DynamicJsonDocument& data, uint32_t blur_window_seconds);
    static std::vector<uint8_t> redactImageAreas(const uint8_t* image_data, size_t image_size, const std::vector<std::pair<uint16_t, uint16_t>>& redaction_areas);
    
    // Audit helpers
    static void saveAuditLog();
    static void pruneAuditLog();
    static String generateAuditEntryId();
    
    // Utility methods
    static String classificationToString(DataClassification classification);
    static String algorithmToString(EncryptionAlgorithm algorithm);
    static String accessLevelToString(AccessLevel level);
    static bool validateEncryptionContext(const EncryptionContext& context);
    static EncryptionContext parseEncryptionContext(const String& encrypted_data);
};

#endif // DATA_PROTECTOR_H