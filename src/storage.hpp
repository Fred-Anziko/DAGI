#pragma once
#include "model.hpp"
#include <vector>
#include <optional>

class ModelStorage {
public:
    void storeModel(const AIModel& model);
    void updateModel(const AIModel& model);
    std::optional<AIModel> getModel(const std::string& id);
    std::vector<AIModel> listModels();
    
private:
    std::vector<AIModel> models;
};
