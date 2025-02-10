#include "agent.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>
#include <iostream>

ModelAgent::ModelAgent(std::shared_ptr<AIModel> model)
    : model(model), state(AgentState::IDLE) {
    // Initialize action scores
    actionScores[AgentAction::ANALYZE] = 1.0;
    actionScores[AgentAction::TRAIN] = 1.0;
    actionScores[AgentAction::PROCESS] = 1.0;
    actionScores[AgentAction::WAIT] = 0.5;
}

void ModelAgent::processContext(const AgentContext& context) {
    if (!validateContext(context)) {
        setState(AgentState::ERROR);
        lastReasoning = "Invalid context provided";
        return;
    }

    setState(AgentState::PROCESSING);
    AgentAction nextAction = decideNextAction(context);
    executeAction(nextAction, context);
}

AgentAction ModelAgent::decideNextAction(const AgentContext& context) {
    std::vector<std::pair<AgentAction, double>> scores;

    // Evaluate each possible action
    for (const auto& [action, baseScore] : actionScores) {
        double score = evaluateAction(action, context);
        scores.push_back({action, score});
    }

    // Choose the action with highest score
    auto maxScore = std::max_element(scores.begin(), scores.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    lastReasoning = generateReasoning(maxScore->first, context);
    return maxScore->first;
}

void ModelAgent::executeAction(AgentAction action, const AgentContext& context) {
    bool success = true;
    try {
        switch (action) {
            case AgentAction::ANALYZE:
                std::cout << "Analyzing " << static_cast<int>(context.mediaType) 
                         << " type content...\n";
                // Implement analysis logic based on media type
                break;

            case AgentAction::TRAIN:
                std::cout << "Training model with new " 
                         << static_cast<int>(context.mediaType) << " data...\n";
                switch (context.mediaType) {
                    case MediaType::TEXT:
                        model->trainWithText(context.input);
                        break;
                    case MediaType::IMAGE:
                        model->trainWithImage(context.binaryData);
                        break;
                    case MediaType::AUDIO:
                        model->trainWithAudio(context.binaryData);
                        break;
                    case MediaType::VIDEO:
                        model->trainWithVideo(context.binaryData);
                        break;
                }
                break;

            case AgentAction::PROCESS:
                std::cout << "Processing " << static_cast<int>(context.mediaType) 
                         << " content...\n";
                switch (context.mediaType) {
                    case MediaType::TEXT:
                        model->processText(context.input);
                        break;
                    case MediaType::IMAGE:
                        model->processImage(context.binaryData);
                        break;
                    case MediaType::AUDIO:
                        model->processAudio(context.binaryData);
                        break;
                    case MediaType::VIDEO:
                        model->processVideo(context.binaryData);
                        break;
                }
                break;

            case AgentAction::WAIT:
                std::cout << "Agent waiting for better context...\n";
                break;
        }
    } catch (const std::exception& e) {
        success = false;
        setState(AgentState::ERROR);
        std::cerr << "Error executing action: " << e.what() << std::endl;
    }

    logDecision(action, context, success);
    updateBehavior(context, success);
}

double ModelAgent::evaluateAction(AgentAction action, const AgentContext& context) {
    double score = actionScores[action];

    // Adjust score based on context and media type
    switch (action) {
        case AgentAction::TRAIN:
            // Prefer training when accuracy is low
            score *= (1.0 - model->getAccuracy());
            // Adjust based on media type complexity
            switch (context.mediaType) {
                case MediaType::TEXT: score *= 1.2; break;
                case MediaType::IMAGE: score *= 1.1; break;
                case MediaType::AUDIO: score *= 1.0; break;
                case MediaType::VIDEO: score *= 0.9; break;
            }
            break;

        case AgentAction::PROCESS:
            // Prefer processing when accuracy is high
            score *= model->getAccuracy();
            // Check if we have sufficient data
            if (context.mediaType == MediaType::TEXT && context.input.empty()) {
                score *= 0.5;
            } else if (context.mediaType != MediaType::TEXT && context.binaryData.empty()) {
                score *= 0.5;
            }
            break;

        case AgentAction::ANALYZE:
            // Prefer analysis for new or complex inputs
            score *= 0.8;
            // Adjust based on context parameters
            if (context.parameters.find("mode") != context.parameters.end() &&
                context.parameters.at("mode") == "analysis") {
                score *= 1.5;
            }
            break;

        case AgentAction::WAIT:
            // Low priority action, but higher if context is insufficient
            score *= 0.5;
            if ((context.mediaType == MediaType::TEXT && context.input.empty()) ||
                (context.mediaType != MediaType::TEXT && context.binaryData.empty())) {
                score *= 2.0;
            }
            break;
    }

    // Consider past decisions
    if (!decisionHistory.empty()) {
        double successRate = std::count_if(decisionHistory.begin(), decisionHistory.end(),
            [action](const Decision& d) { 
                return d.action == action && d.success; 
            }) / static_cast<double>(decisionHistory.size());
        score *= (0.5 + successRate);
    }

    return score;
}

void ModelAgent::updateBehavior(const AgentContext& context, bool success) {
    // Update action scores based on success/failure and context
    const double learningRate = 0.1;
    const double penalty = 0.8;
    const double reward = 1.2;

    for (auto& [action, score] : actionScores) {
        // Apply context-specific adjustments
        double contextMultiplier = 1.0;
        if (context.parameters.find("mode") != context.parameters.end()) {
            if (context.parameters.at("mode") == "training" && action == AgentAction::TRAIN) {
                contextMultiplier *= (1.0 + learningRate);
            } else if (context.parameters.at("mode") == "analysis" && action == AgentAction::ANALYZE) {
                contextMultiplier *= (1.0 + learningRate);
            }
        }

        if (success) {
            score *= reward * contextMultiplier;
        } else {
            score *= penalty * contextMultiplier;
        }
        // Normalize scores
        score = std::max(0.1, std::min(score, 2.0));
    }
}

void ModelAgent::learn(const AgentContext& context, const std::string& feedback) {
    // Implement feedback-based learning using context
    std::cout << "Agent received feedback for " << static_cast<int>(context.mediaType) 
              << " type: " << feedback << std::endl;

    // Adjust action scores based on feedback sentiment
    bool isPositive = feedback.find("good") != std::string::npos || 
                     feedback.find("excellent") != std::string::npos;
    bool isNegative = feedback.find("needs improvement") != std::string::npos || 
                     feedback.find("poor") != std::string::npos;

    if (isPositive) {
        actionScores[AgentAction::PROCESS] *= 1.1;
    } else if (isNegative) {
        actionScores[AgentAction::TRAIN] *= 1.1;
    }
}

bool ModelAgent::validateContext(const AgentContext& context) const {
    // Validate that the model supports the media type
    if (!model->supportsMediaType(context.mediaType)) {
        return false;
    }

    // Validate input based on media type
    switch (context.mediaType) {
        case MediaType::TEXT:
            return !context.input.empty();

        case MediaType::IMAGE:
        case MediaType::AUDIO:
        case MediaType::VIDEO:
            return !context.binaryData.empty();
    }

    return true;
}

void ModelAgent::logDecision(AgentAction action, const AgentContext& context, bool success) {
    decisionHistory.push_back({action, context, success});
    if (decisionHistory.size() > 1000) { // Keep history bounded
        decisionHistory.erase(decisionHistory.begin());
    }
}

std::string ModelAgent::generateReasoning(AgentAction action, const AgentContext& context) {
    std::stringstream ss;
    ss << "Selected action: " << static_cast<int>(action) << " for media type "
       << static_cast<int>(context.mediaType) << " because: ";

    switch (action) {
        case AgentAction::ANALYZE:
            ss << "Input requires analysis";
            if (context.parameters.find("mode") != context.parameters.end()) {
                ss << " (mode: " << context.parameters.at("mode") << ")";
            }
            break;
        case AgentAction::TRAIN:
            ss << "Model accuracy can be improved (current: " 
               << model->getAccuracy() << ")";
            break;
        case AgentAction::PROCESS:
            ss << "Model is ready to process input";
            if (context.mediaType == MediaType::TEXT) {
                ss << " of length " << context.input.length();
            } else {
                ss << " of size " << context.binaryData.size() << " bytes";
            }
            break;
        case AgentAction::WAIT:
            ss << "Insufficient context for other actions";
            break;
    }

    return ss.str();
}