#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <map>
#include <set>
#include <sstream>

enum class MediaType {
    TEXT = 1,
    IMAGE = 2,
    AUDIO = 4,
    VIDEO = 8
};

struct MediaProperties {
    // Common properties
    unsigned int inputSize;
    unsigned int outputSize;

    // Media-specific properties
    union {
        struct {
            unsigned int maxSequenceLength;  // for text
            unsigned int vocabularySize;
        } text;
        struct {
            unsigned int width;             // for image/video
            unsigned int height;
            unsigned int channels;
            unsigned int frameRate;         // for video only
        } visual;
        struct {
            unsigned int sampleRate;        // for audio
            unsigned int channels;
            unsigned int bitDepth;
        } audio;
    };
};

class AIModel {
public:
    AIModel(const std::string& name, const std::vector<MediaType>& supportedTypes);

    std::string getId() const { return id; }
    std::string getName() const { return name; }
    double getAccuracy() const { return accuracy; }
    unsigned int getVersion() const { return version; }
    bool isValidated() const { return validated; }
    bool supportsMediaType(MediaType type) const;
    std::vector<MediaType> getSupportedTypes() const;

    // Enhanced training methods
    void train();
    void trainWithText(const std::string& text);
    void trainWithImage(const std::vector<uint8_t>& imageData);
    void trainWithAudio(const std::vector<uint8_t>& audioData);
    void trainWithVideo(const std::vector<uint8_t>& videoData);

    // Media processing methods
    std::string processText(const std::string& input) const;
    std::vector<uint8_t> processImage(const std::vector<uint8_t>& input) const;
    std::vector<uint8_t> processAudio(const std::vector<uint8_t>& input) const;
    std::vector<uint8_t> processVideo(const std::vector<uint8_t>& input) const;

    void save() const;
    void load(const std::string& modelId);
    bool validate();
    void incrementVersion() { version++; }

    // Model sharing functionality
    std::string exportModel() const;
    bool importModel(const std::string& modelData);

    // Media properties configuration
    void configureMediaProperties(MediaType type, const MediaProperties& props);
    const MediaProperties& getMediaProperties(MediaType type) const;

    // Debug information
    std::string getDebugInfo() const {
        std::stringstream ss;
        ss << "Supported Types: ";
        for (const auto& type : supportedTypes) {
            switch(type) {
                case MediaType::TEXT: ss << "TEXT "; break;
                case MediaType::IMAGE: ss << "IMAGE "; break;
                case MediaType::AUDIO: ss << "AUDIO "; break;
                case MediaType::VIDEO: ss << "VIDEO "; break;
            }
        }
        ss << "| Weight Size: " << weights.size() << " bytes"
           << " | Validated: " << (validated ? "Yes" : "No");
        return ss.str();
    }

private:
    std::string id;
    std::string name;
    std::set<MediaType> supportedTypes;
    double accuracy;
    std::vector<std::uint8_t> weights;
    unsigned int version;
    bool validated;
    std::map<unsigned int, std::vector<std::uint8_t>> weightHistory;
    std::map<MediaType, MediaProperties> mediaProps;

    static std::string generateId();
    void saveWeightSnapshot();
    void initializeMediaProperties();
    void validateMediaType(MediaType type) const;
};