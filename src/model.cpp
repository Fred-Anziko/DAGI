#include "model.hpp"
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include "utils.hpp"
#include <unordered_set>
#include <map>


AIModel::AIModel(const std::string& name, const std::vector<MediaType>& types) 
    : name(name), accuracy(0.0), version(1), validated(false) {
    id = generateId();
    supportedTypes.insert(types.begin(), types.end());
    initializeMediaProperties();
}

void AIModel::initializeMediaProperties() {
    for (const auto& type : supportedTypes) {
        MediaProperties props{};
        switch (type) {
            case MediaType::TEXT:
                props.inputSize = 512;
                props.outputSize = 512;
                props.text.maxSequenceLength = 1024;
                props.text.vocabularySize = 50000;
                break;
            case MediaType::IMAGE:
                props.inputSize = 224 * 224 * 3;
                props.outputSize = 1000;
                props.visual.width = 224;
                props.visual.height = 224;
                props.visual.channels = 3;
                break;
            case MediaType::AUDIO:
                props.inputSize = 16000;
                props.outputSize = 512;
                props.audio.sampleRate = 16000;
                props.audio.channels = 1;
                props.audio.bitDepth = 16;
                break;
            case MediaType::VIDEO:
                props.inputSize = 224 * 224 * 3 * 30;
                props.outputSize = 1000;
                props.visual.width = 224;
                props.visual.height = 224;
                props.visual.channels = 3;
                props.visual.frameRate = 30;
                break;
        }
        mediaProps[type] = props;
    }
}

bool AIModel::supportsMediaType(MediaType type) const {
    return supportedTypes.find(type) != supportedTypes.end();
}

std::vector<MediaType> AIModel::getSupportedTypes() const {
    return std::vector<MediaType>(supportedTypes.begin(), supportedTypes.end());
}

void AIModel::validateMediaType(MediaType type) const {
    if (!supportsMediaType(type)) {
        throw std::runtime_error("Model does not support this media type");
    }
}

void AIModel::train() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 0.1);

    accuracy += dis(gen);
    if (accuracy > 1.0) accuracy = 1.0;

    // Calculate total weight size for all supported media types
    size_t totalWeightSize = 0;
    for (const auto& type : supportedTypes) {
        const auto& props = mediaProps[type];
        switch (type) {
            case MediaType::TEXT:
                totalWeightSize += props.text.vocabularySize * props.outputSize;
                std::cout << "Added text weights: " << totalWeightSize << " bytes\n";
                break;
            case MediaType::IMAGE:
                totalWeightSize += props.visual.width * props.visual.height * 
                                 props.visual.channels * props.outputSize;
                std::cout << "Added image weights: " << totalWeightSize << " bytes\n";
                break;
            case MediaType::AUDIO:
                totalWeightSize += props.audio.sampleRate * props.audio.channels * 
                                 props.outputSize;
                std::cout << "Added audio weights: " << totalWeightSize << " bytes\n";
                break;
            case MediaType::VIDEO:
                totalWeightSize += props.visual.width * props.visual.height * 
                                 props.visual.channels * props.visual.frameRate * 
                                 props.outputSize;
                std::cout << "Added video weights: " << totalWeightSize << " bytes\n";
                break;
        }
    }

    std::cout << "Total weight size for all media types: " << totalWeightSize << " bytes\n";
    weights.resize(totalWeightSize);

    // Initialize weights with random values
    for (auto& w : weights) {
        w = static_cast<uint8_t>(dis(gen) * 255);
    }

    saveWeightSnapshot();
    incrementVersion();
    std::cout << "Model version incremented to: " << version << std::endl;
}

void AIModel::trainWithText(const std::string& text) {
    validateMediaType(MediaType::TEXT);
    std::cout << "Training with text of length: " << text.length() << std::endl;
    std::cout << "Sample text content: " << text.substr(0, 50) << "...\n";
    train();
}

void AIModel::trainWithImage(const std::vector<uint8_t>& imageData) {
    validateMediaType(MediaType::IMAGE);
    const auto& props = mediaProps[MediaType::IMAGE];
    std::cout << "Training with image of size: " << imageData.size() << " bytes\n";
    std::cout << "Image dimensions: " << props.visual.width << "x" << props.visual.height 
              << "x" << props.visual.channels << "\n";
    train();
}

void AIModel::trainWithAudio(const std::vector<uint8_t>& audioData) {
    validateMediaType(MediaType::AUDIO);
    const auto& props = mediaProps[MediaType::AUDIO];
    std::cout << "Training with audio of size: " << audioData.size() << " bytes\n";
    std::cout << "Audio properties: " << props.audio.sampleRate << "Hz, " 
              << props.audio.channels << " channels\n";
    train();
}

void AIModel::trainWithVideo(const std::vector<uint8_t>& videoData) {
    validateMediaType(MediaType::VIDEO);
    const auto& props = mediaProps[MediaType::VIDEO];
    std::cout << "Training with video of size: " << videoData.size() << " bytes\n";
    std::cout << "Video properties: " << props.visual.width << "x" << props.visual.height 
              << "@" << props.visual.frameRate << "fps\n";
    train();
}

std::string AIModel::processText(const std::string& input) const {
    validateMediaType(MediaType::TEXT);
    const auto& props = mediaProps.at(MediaType::TEXT);
    return "Processed: " + input.substr(0, std::min(input.length(), 
            static_cast<size_t>(props.text.maxSequenceLength)));
}

std::vector<uint8_t> AIModel::processImage(const std::vector<uint8_t>& input) const {
    validateMediaType(MediaType::IMAGE);
    return input;
}

std::vector<uint8_t> AIModel::processAudio(const std::vector<uint8_t>& input) const {
    validateMediaType(MediaType::AUDIO);
    return input;
}

std::vector<uint8_t> AIModel::processVideo(const std::vector<uint8_t>& input) const {
    validateMediaType(MediaType::VIDEO);
    return input;
}

void AIModel::configureMediaProperties(MediaType type, const MediaProperties& props) {
    validateMediaType(type);
    mediaProps[type] = props;
}

const MediaProperties& AIModel::getMediaProperties(MediaType type) const {
    validateMediaType(type);
    return mediaProps.at(type);
}

void AIModel::save() const {
    std::ofstream file("models/" + id + "_v" + std::to_string(version) + ".model", 
                      std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to save model file");
    }
    file.write(reinterpret_cast<const char*>(weights.data()), weights.size());
}

void AIModel::load(const std::string& modelId) {
    std::ifstream file("models/" + modelId + "_v" + std::to_string(version) + ".model", 
                      std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to load model file");
    }

    size_t totalSize = 0;
    for (const auto& [type, props] : mediaProps) {
        totalSize += props.inputSize * props.outputSize;
    }

    weights.resize(totalSize);
    file.read(reinterpret_cast<char*>(weights.data()), weights.size());
}

bool AIModel::validate() {
    validated = true;
    for (const auto& w : weights) {
        if (w == 0) {
            validated = false;
            break;
        }
    }
    return validated;
}

void AIModel::saveWeightSnapshot() {
    weightHistory[version] = weights;
}

std::string AIModel::exportModel() const {
    std::stringstream ss;
    ss << id << "," << name << ",";
    for (const auto& type : supportedTypes) {
        ss << static_cast<int>(type) << ";";
    }
    ss << accuracy << "," << version;
    return utils::hashString(ss.str());
}

bool AIModel::importModel(const std::string& modelData) {
    try {
        if (modelData.empty()) return false;

        size_t totalSize = 0;
        for (const auto& [type, props] : mediaProps) {
            totalSize += props.inputSize * props.outputSize;
        }

        weights.resize(totalSize);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, 255);

        for (auto& w : weights) {
            w = static_cast<uint8_t>(dis(gen));
        }

        return true;
    } catch (...) {
        return false;
    }
}

std::string AIModel::generateId() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    std::stringstream ss;
    ss << std::hex << millis;
    return ss.str();
}