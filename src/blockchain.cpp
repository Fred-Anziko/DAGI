#include "blockchain.hpp"
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iomanip>
#include "utils.hpp"
#include <iostream>

Transaction::Transaction(const std::string& type, const std::string& modelId,
                       const std::string& from, const std::string& to, 
                       double amount, std::time_t rentalDuration)
    : type(type), modelId(modelId), from(from), to(to), amount(amount),
      timestamp(std::time(nullptr)), expiryTime(rentalDuration > 0 ? timestamp + rentalDuration : 0),
      isCollaborative(false), resourceContribution(0.0) {
}

std::string Transaction::calculateHash() const {
    std::stringstream ss;

    // Base transaction data
    ss << type << modelId << from << to 
       << std::fixed << std::setprecision(6) << amount 
       << timestamp << expiryTime 
       << previousHash;  // Include previousHash first

    // Collaborative data if present
    if (isCollaborative) {
        ss << "collaborative";
        std::vector<std::string> sortedContributors = contributors;
        std::sort(sortedContributors.begin(), sortedContributors.end());
        for (const auto& contributor : sortedContributors) {
            ss << contributor;
        }
        ss << std::fixed << std::setprecision(6) << resourceContribution;

        std::vector<std::pair<std::string, double>> sortedShares(
            rewardShares.begin(), rewardShares.end());
        std::sort(sortedShares.begin(), sortedShares.end());
        for (const auto& [userId, share] : sortedShares) {
            ss << userId << std::fixed << std::setprecision(6) << share;
        }
    }

    return utils::hashString(ss.str());
}

bool Transaction::verifySignature() const {
    if (signature.empty()) {
        std::cout << "Empty signature detected\n";
        return false;
    }

    std::string dataToVerify = calculateHash();
    std::string expectedSignature = utils::hashString("mock_private_key" + dataToVerify);

    bool isValid = (signature == expectedSignature);
    if (!isValid) {
        std::cout << "Signature verification failed for transaction: " << type << "\n";
    }
    return isValid;
}

void Transaction::sign(const std::string& privateKey) {
    std::string dataToSign = calculateHash();
    signature = utils::hashString(privateKey + dataToSign);
}

void BlockchainLedger::addTransaction(const std::string& type, const std::string& modelId,
                                    const std::string& from, const std::string& to, 
                                    double amount, std::time_t rentalDuration) {
    Transaction tx(type, modelId, from, to, amount, rentalDuration);

    if (transactions.empty()) {
        tx.previousHash = utils::hashString("genesis_block");
    } else {
        tx.previousHash = transactions.back().calculateHash();
    }

    tx.sign("mock_private_key");

    if (!tx.verifySignature()) {
        throw std::runtime_error("Transaction signature verification failed");
    }

    if (!transactions.empty() && tx.previousHash != transactions.back().calculateHash()) {
        throw std::runtime_error("Invalid chain link in new transaction");
    }

    transactions.push_back(tx);
}

void BlockchainLedger::addCollaborativeTransaction(
    const std::string& modelId,
    const std::vector<std::string>& contributors,
    const std::vector<double>& contributions) {

    if (contributors.size() != contributions.size() || contributors.empty()) {
        throw std::runtime_error("Invalid contributors or contributions");
    }

    Transaction tx("COLLABORATIVE", modelId, "", "", 0.0);
    tx.isCollaborative = true;
    tx.contributors = contributors;

    double totalContribution = std::accumulate(
        contributions.begin(), contributions.end(), 0.0);

    if (totalContribution <= 0.0) {
        throw std::runtime_error("Total contribution must be positive");
    }

    tx.resourceContribution = totalContribution;

    // Set normalized reward shares
    for (size_t i = 0; i < contributors.size(); ++i) {
        tx.rewardShares[contributors[i]] = contributions[i] / totalContribution;
    }

    if (transactions.empty()) {
        tx.previousHash = utils::hashString("genesis_block");
    } else {
        tx.previousHash = transactions.back().calculateHash();
    }

    tx.sign("mock_private_key");

    if (!tx.verifySignature()) {
        throw std::runtime_error("Collaborative transaction signature verification failed");
    }

    transactions.push_back(tx);
}

bool BlockchainLedger::verifyChain() const {
    if (transactions.empty()) return true;

    std::cout << "\nStarting blockchain verification...\n";

    // Verify genesis block
    const Transaction& genesis = transactions[0];
    if (genesis.previousHash != utils::hashString("genesis_block")) {
        std::cout << "Genesis block has invalid previous hash\n";
        return false;
    }

    if (!genesis.verifySignature()) {
        std::cout << "Genesis block signature verification failed\n";
        return false;
    }

    std::string expectedPreviousHash = genesis.calculateHash();
    std::cout << "Genesis block verified successfully\n";

    // Verify transaction chain
    for (size_t i = 1; i < transactions.size(); i++) {
        const Transaction& tx = transactions[i];
        std::cout << "\nVerifying transaction " << i << " (Type: " << tx.type << ")\n";

        // Verify chain linking
        if (tx.previousHash != expectedPreviousHash) {
            std::cout << "Chain link verification failed at transaction " << i << "\n";
            std::cout << "Expected: " << expectedPreviousHash << "\n";
            std::cout << "Got: " << tx.previousHash << "\n";
            return false;
        }

        // Verify transaction signature
        if (!tx.verifySignature()) {
            std::cout << "Signature verification failed at transaction " << i << "\n";
            return false;
        }

        // Additional checks for collaborative transactions
        if (tx.isCollaborative) {
            if (tx.contributors.empty()) {
                std::cout << "Empty contributors list in collaborative transaction\n";
                return false;
            }

            // Verify reward shares
            double totalShares = std::accumulate(
                tx.rewardShares.begin(),
                tx.rewardShares.end(),
                0.0,
                [](double sum, const auto& pair) { return sum + pair.second; }
            );

            if (std::abs(totalShares - 1.0) > 0.000001) {
                std::cout << "Invalid reward shares distribution (total: " 
                         << totalShares << ")\n";
                return false;
            }
        }

        expectedPreviousHash = tx.calculateHash();
        std::cout << "Transaction " << i << " verified successfully\n";
    }

    std::cout << "Blockchain verification completed successfully\n";
    return true;
}

void BlockchainLedger::addVote(const std::string& modelId, const std::string& voterId, 
                              int rating, const std::string& review) {
    if (rating < 1 || rating > 5) {
        throw std::runtime_error("Rating must be between 1 and 5");
    }

    Vote vote{modelId, voterId, rating, review, std::time(nullptr)};
    modelVotes[modelId].push_back(vote);

    // Update model creator's reputation
    for (const auto& tx : transactions) {
        if (tx.modelId == modelId && tx.type == "CREATE") {
            double reputationChange = (rating - 3.0) * 0.1; // Normalize impact
            updateReputationScore(tx.from, reputationChange);
            break;
        }
    }
}

double BlockchainLedger::getModelRating(const std::string& modelId) const {
    const auto& votes = modelVotes.find(modelId);
    if (votes == modelVotes.end() || votes->second.empty()) {
        return 0.0;
    }

    double sum = std::accumulate(votes->second.begin(), votes->second.end(), 0.0,
        [](double acc, const Vote& v) { return acc + v.rating; });
    return sum / votes->second.size();
}

UserReputation BlockchainLedger::getUserReputation(const std::string& userId) const {
    auto it = userReputations.find(userId);
    if (it != userReputations.end()) {
        return it->second;
    }
    return UserReputation{0.0, 0, 0, {}};
}

void BlockchainLedger::updateResourceContribution(const std::string& userId, double hours) {
    // Update user's reputation based on resource contribution
    double reputationChange = std::log10(1 + hours) * 0.1;
    updateReputationScore(userId, reputationChange);

    // Record the contribution in a transaction
    Transaction tx("RESOURCE_CONTRIBUTION", "", userId, "", 0.0);
    tx.resourceContribution = hours;
    addTransaction("RESOURCE", "", userId, "", 0.0, 0); 
}

double BlockchainLedger::calculateFairPrice(const std::string& modelId) const {
    double basePrice = 100.0; 
    double rating = getModelRating(modelId);

    // Find total training resources invested
    double totalResources = 0.0;
    for (const auto& tx : transactions) {
        if (tx.modelId == modelId && 
            (tx.type == "RESOURCE_CONTRIBUTION" || tx.isCollaborative)) {
            totalResources += tx.resourceContribution;
        }
    }

    // Price formula: base * (rating_factor) * log(1 + resources)
    double ratingFactor = 0.5 + (rating / 10.0); 
    return basePrice * ratingFactor * std::log10(1 + totalResources);
}

std::vector<std::string> BlockchainLedger::getTopContributors() const {
    std::vector<std::pair<std::string, double>> contributors;
    for (const auto& [userId, reputation] : userReputations) {
        contributors.push_back({userId, reputation.score});
    }

    std::sort(contributors.begin(), contributors.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    std::vector<std::string> result;
    for (const auto& [userId, _] : contributors) {
        result.push_back(userId);
        if (result.size() >= 10) break; 
    }
    return result;
}

void BlockchainLedger::updateReputationScore(const std::string& userId, double change) {
    auto& rep = userReputations[userId];
    rep.score = std::max(0.0, rep.score + change);
    rep.totalVotes++;
}

std::vector<Transaction> BlockchainLedger::getTransactions() const {
    return transactions;
}

bool BlockchainLedger::isModelAvailableForRent(const std::string& modelId) const {
    auto now = std::time(nullptr);
    for (const auto& tx : transactions) {
        if (tx.modelId == modelId && tx.type == "RENT") {
            if (tx.expiryTime == 0 || tx.expiryTime > now) {
                return false;
            }
        }
    }
    return true;
}

bool BlockchainLedger::isModelRentedBy(const std::string& modelId, const std::string& user) const {
    auto now = std::time(nullptr);
    for (const auto& tx : transactions) {
        if (tx.modelId == modelId && tx.type == "RENT" && tx.to == user) {
            if (tx.expiryTime == 0 || tx.expiryTime > now) {
                return true;
            }
        }
    }
    return false;
}

std::string BlockchainLedger::calculateBlockHash(const Transaction& tx) const {
    return tx.calculateHash();
}


// New: Documentation & Knowledge Sharing
void BlockchainLedger::addDocumentation(const std::string& modelId, const std::string& authorId,
                                      const std::string& content, const std::vector<std::string>& tags) {
    Documentation doc{
        modelId, authorId, content, tags,
        std::time(nullptr), 0, std::vector<std::string>()
    };
    modelDocs[modelId].push_back(doc);

    // Update author's reputation
    updateReputationScore(authorId, 0.2); 
}

void BlockchainLedger::upvoteDocumentation(const std::string& modelId, const std::string& voterId) {
    if (!modelDocs[modelId].empty()) {
        modelDocs[modelId].back().upvotes++;
        updateReputationScore(modelDocs[modelId].back().authorId, 0.05);
    }
}

void BlockchainLedger::addDocComment(const std::string& modelId, const std::string& userId,
                                   const std::string& comment) {
    if (!modelDocs[modelId].empty()) {
        modelDocs[modelId].back().comments.push_back(comment);
        updateReputationScore(userId, 0.02); 
    }
}

std::vector<Documentation> BlockchainLedger::getModelDocs(const std::string& modelId) const {
    auto it = modelDocs.find(modelId);
    return it != modelDocs.end() ? it->second : std::vector<Documentation>();
}

// New: Quality Control & Governance
void BlockchainLedger::updateQualityMetrics(const std::string& modelId, const QualityMetrics& metrics) {
    modelQuality[modelId] = metrics;
}

bool BlockchainLedger::validateModel(const std::string& modelId, const std::string& validatorId) {
    auto& metrics = modelQuality[modelId];
    metrics.validations.push_back(validatorId);
    metrics.lastAudit = std::time(nullptr);

    // Reward validator
    updateReputationScore(validatorId, 0.1);
    return true;
}

QualityMetrics BlockchainLedger::getModelQuality(const std::string& modelId) const {
    auto it = modelQuality.find(modelId);
    return it != modelQuality.end() ? it->second : QualityMetrics{};
}

// New: Advanced Reward System
void BlockchainLedger::distributeRewards(const std::string& modelId, double totalReward) {
    Transaction tx("REWARD", modelId, "system", "", totalReward);

    // Set the proper chain link
    if (transactions.empty()) {
        tx.previousHash = utils::hashString("genesis_block");
    } else {
        tx.previousHash = transactions.back().calculateHash();
    }

    // Calculate shares based on contributions and reputation
    std::map<std::string, double> shares;
    double totalShares = 0.0;

    for (const auto& transaction : transactions) {
        if (transaction.modelId == modelId && transaction.isCollaborative) {
            for (size_t i = 0; i < transaction.contributors.size(); ++i) {
                const auto& contributor = transaction.contributors[i];
                auto rep = getUserReputation(contributor);
                shares[contributor] += rep.score * transaction.resourceContribution;
                totalShares += shares[contributor];
            }
        }
    }

    // Distribute rewards proportionally
    tx.rewardShares = shares;
    for (auto& [userId, share] : shares) {
        share = (share / totalShares) * totalReward;
        updateReputationScore(userId, share * 0.01);
    }

    // Sign and verify before adding
    tx.sign("mock_private_key");
    if (!tx.verifySignature()) {
        throw std::runtime_error("Reward transaction signature verification failed");
    }

    // Final chain link verification
    if (!transactions.empty() && tx.previousHash != transactions.back().calculateHash()) {
        throw std::runtime_error("Invalid chain link in reward transaction");
    }

    transactions.push_back(tx);
}

double BlockchainLedger::calculateUserReward(const std::string& userId, 
                                           const std::string& modelId) const {
    double totalReward = 0.0;
    for (const auto& tx : transactions) {
        if (tx.type == "REWARD" && tx.modelId == modelId) {
            auto it = tx.rewardShares.find(userId);
            if (it != tx.rewardShares.end()) {
                totalReward += it->second;
            }
        }
    }
    return totalReward;
}

void BlockchainLedger::updateRewardShares(const std::string& modelId,
                                        const std::map<std::string, double>& shares) {
    Transaction tx("REWARD_UPDATE", modelId, "", "", 0.0);
    tx.rewardShares = shares;
    transactions.push_back(tx);
}

// New: Resource Optimization
void BlockchainLedger::trackResourceUsage(const std::string& modelId, 
                                        const ResourceUsage& usage) {
    resourceMetrics[modelId] = usage;
}

ResourceUsage BlockchainLedger::getResourceMetrics(const std::string& modelId) const {
    auto it = resourceMetrics.find(modelId);
    return it != resourceMetrics.end() ? it->second : ResourceUsage{};
}

double BlockchainLedger::optimizeResourceAllocation(const std::string& modelId) {
    auto& usage = resourceMetrics[modelId];

    // Calculate efficiency score
    double efficiency = (usage.cpuHours + usage.gpuHours) > 0 ?
        usage.costTokens / (usage.cpuHours + usage.gpuHours) : 0;

    // Update metrics based on optimization
    usage.costTokens *= 0.9; 
    return efficiency;
}

// New: Version Control
void BlockchainLedger::addModelVersion(const std::string& modelId, 
                                     const ModelVersion& version) {
    versionHistory[modelId].push_back(version);
}

bool BlockchainLedger::rollbackVersion(const std::string& modelId, 
                                     unsigned int targetVersion) {
    auto& history = versionHistory[modelId];
    if (history.empty()) return false;

    // Find and validate target version
    auto it = std::find_if(history.begin(), history.end(),
        [targetVersion](const ModelVersion& v) { 
            return v.version == targetVersion && v.canRollback; 
        });

    if (it != history.end()) {
        // Create rollback transaction with proper chain linking
        Transaction tx("ROLLBACK", modelId, "", "", 0.0);

        // Set genesis hash or link to previous transaction
        if (transactions.empty()) {
            tx.previousHash = utils::hashString("genesis_block");
        } else {
            tx.previousHash = transactions.back().calculateHash();
        }

        // Sign and verify the transaction
        tx.sign("mock_private_key");
        if (!tx.verifySignature()) {
            throw std::runtime_error("Rollback transaction signature verification failed");
        }

        // Verify chain link
        if (!transactions.empty() && tx.previousHash != transactions.back().calculateHash()) {
            throw std::runtime_error("Invalid chain link in rollback transaction");
        }

        transactions.push_back(tx);
        return true;
    }
    return false;
}

std::vector<ModelVersion> BlockchainLedger::getVersionHistory(
    const std::string& modelId) const {
    auto it = versionHistory.find(modelId);
    return it != versionHistory.end() ? it->second : std::vector<ModelVersion>();
}