#include "storage.hpp"
#include <algorithm>

void ModelStorage::storeModel(const AIModel& model) {
    models.push_back(model);
}

void ModelStorage::updateModel(const AIModel& model) {
    auto it = std::find_if(models.begin(), models.end(),
                          [&](const AIModel& m) { return m.getId() == model.getId(); });
    if (it != models.end()) {
        *it = model;
    }
}

std::optional<AIModel> ModelStorage::getModel(const std::string& id) {
    auto it = std::find_if(models.begin(), models.end(),
                          [&](const AIModel& m) { return m.getId() == id; });
    if (it != models.end()) {
        return *it;
    }
    return std::nullopt;
}

std::vector<AIModel> ModelStorage::listModels() {
    return models;
}
