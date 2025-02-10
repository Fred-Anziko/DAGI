#pragma once
#include <string>
#include <vector>
#include <ctime>
#include <memory>
#include <map>
#include "utils.hpp"

struct Vote {
    std::string modelId;
    std::string voterId;
    int rating;        // 1-5 stars
    std::string review;
    std::time_t timestamp;
};

struct UserReputation {
    double score;
    int totalVotes;
    int modelsShared;
    std::vector<std::string> reviews;
};

// New: Documentation Entry
struct Documentation {
    std::string modelId;
    std::string authorId;
    std::string content;
    std::vector<std::string> tags;
    std::time_t timestamp;
    int upvotes;
    std::vector<std::string> comments;
};

// New: Quality Metrics
struct QualityMetrics {
    double accuracy;
    double reliability;
    int userCount;
    double avgResponseTime;
    std::vector<std::string> validations;
    std::time_t lastAudit;
};

// New: Resource Usage
struct ResourceUsage {
    double cpuHours;
    double gpuHours;
    double memoryGB;
    double bandwidthGB;
    double costTokens;
};

// New: Version Control
struct ModelVersion {
    unsigned int version;
    std::string commitHash;
    std::string parentHash;
    std::time_t timestamp;
    std::string changes;
    bool canRollback;
};

struct Transaction {
    std::string type;
    std::string modelId;
    std::string from;
    std::string to;
    double amount;
    std::time_t timestamp;
    std::time_t expiryTime;  // For rentals
    std::string signature;
    std::string previousHash;

    // Federation & Resource Sharing
    bool isCollaborative;
    std::vector<std::string> contributors;
    double resourceContribution;  // CPU/GPU hours contributed

    // New: Reward Distribution
    std::map<std::string, double> rewardShares;

    Transaction(const std::string& type, const std::string& modelId,
               const std::string& from, const std::string& to, 
               double amount, std::time_t rentalDuration = 0);

    std::string calculateHash() const;
    bool verifySignature() const;
    void sign(const std::string& privateKey);
};

class BlockchainLedger {
public:
    // Existing transaction methods
    void addTransaction(const std::string& type, const std::string& modelId,
                       const std::string& from, const std::string& to, 
                       double amount, std::time_t rentalDuration = 0);

    std::vector<Transaction> getTransactions() const;
    bool verifyChain() const;
    bool isModelAvailableForRent(const std::string& modelId) const;
    bool isModelRentedBy(const std::string& modelId, const std::string& user) const;

    // Existing democratization features
    void addVote(const std::string& modelId, const std::string& voterId, 
                int rating, const std::string& review);
    double getModelRating(const std::string& modelId) const;
    UserReputation getUserReputation(const std::string& userId) const;
    void updateResourceContribution(const std::string& userId, double hours);

    // Federation & Collaborative features
    void addCollaborativeTransaction(const std::string& modelId,
                                   const std::vector<std::string>& contributors,
                                   const std::vector<double>& contributions);
    double calculateFairPrice(const std::string& modelId) const;
    std::vector<std::string> getTopContributors() const;

    // New: Documentation & Knowledge Sharing
    void addDocumentation(const std::string& modelId, const std::string& authorId,
                         const std::string& content, const std::vector<std::string>& tags);
    void upvoteDocumentation(const std::string& modelId, const std::string& voterId);
    void addDocComment(const std::string& modelId, const std::string& userId,
                      const std::string& comment);
    std::vector<Documentation> getModelDocs(const std::string& modelId) const;

    // New: Quality Control & Governance
    void updateQualityMetrics(const std::string& modelId, const QualityMetrics& metrics);
    bool validateModel(const std::string& modelId, const std::string& validatorId);
    QualityMetrics getModelQuality(const std::string& modelId) const;

    // New: Advanced Reward System
    void distributeRewards(const std::string& modelId, double totalReward);
    double calculateUserReward(const std::string& userId, const std::string& modelId) const;
    void updateRewardShares(const std::string& modelId, 
                          const std::map<std::string, double>& shares);

    // New: Resource Optimization
    void trackResourceUsage(const std::string& modelId, const ResourceUsage& usage);
    ResourceUsage getResourceMetrics(const std::string& modelId) const;
    double optimizeResourceAllocation(const std::string& modelId);

    // New: Version Control
    void addModelVersion(const std::string& modelId, const ModelVersion& version);
    bool rollbackVersion(const std::string& modelId, unsigned int targetVersion);
    std::vector<ModelVersion> getVersionHistory(const std::string& modelId) const;

private:
    std::vector<Transaction> transactions;
    std::map<std::string, std::vector<Vote>> modelVotes;
    std::map<std::string, UserReputation> userReputations;

    // New private maps for additional features
    std::map<std::string, std::vector<Documentation>> modelDocs;
    std::map<std::string, QualityMetrics> modelQuality;
    std::map<std::string, ResourceUsage> resourceMetrics;
    std::map<std::string, std::vector<ModelVersion>> versionHistory;

    std::string calculateBlockHash(const Transaction& tx) const;
    void updateReputationScore(const std::string& userId, double change);
};