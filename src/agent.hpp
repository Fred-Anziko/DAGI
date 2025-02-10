#pragma once
#include "model.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

enum class AgentState {
    IDLE,
    PROCESSING,
    TRAINING,
    ERROR
};

enum class AgentAction {
    ANALYZE,
    TRAIN,
    PROCESS,
    WAIT
};

struct AgentContext {
    MediaType mediaType;
    std::string input;
    std::vector<uint8_t> binaryData;
    std::map<std::string, std::string> parameters;
};

class ModelAgent {
public:
    ModelAgent(std::shared_ptr<AIModel> model);
    
    // Core agent capabilities
    void processContext(const AgentContext& context);
    AgentAction decideNextAction(const AgentContext& context);
    void executeAction(AgentAction action, const AgentContext& context);
    
    // State management
    AgentState getState() const { return state; }
    void setState(AgentState newState) { state = newState; }
    
    // Decision making
    double evaluateAction(AgentAction action, const AgentContext& context);
    std::string getActionReasoning() const { return lastReasoning; }
    
    // Learning and adaptation
    void updateBehavior(const AgentContext& context, bool success);
    void learn(const AgentContext& context, const std::string& feedback);

private:
    std::shared_ptr<AIModel> model;
    AgentState state;
    std::string lastReasoning;
    std::map<AgentAction, double> actionScores;
    
    // Decision history for learning
    struct Decision {
        AgentAction action;
        AgentContext context;
        bool success;
    };
    std::vector<Decision> decisionHistory;
    
    // Helper methods
    void logDecision(AgentAction action, const AgentContext& context, bool success);
    std::string generateReasoning(AgentAction action, const AgentContext& context);
    bool validateContext(const AgentContext& context) const;
};
