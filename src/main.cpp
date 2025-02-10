#include <iostream>
#include <string>
#include <iomanip>
#include "model.hpp"
#include "blockchain.hpp"
#include "storage.hpp"
#include "agent.hpp"
#include "utils.hpp"
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <map>

// Define the training configuration struct first
struct TrainingConfig {
    double learningRate = 0.01;
    int batchSize = 32;
    int iterations = 100;
    bool showProgress = false;
    std::string metricsFile;
};

void printProgress(int current, int total, double accuracy) {
    int barWidth = 50;
    float progress = static_cast<float>(current) / total;
    int pos = static_cast<int>(barWidth * progress);

    std::cout << "\r[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "% "
              << "Accuracy: " << std::fixed << std::setprecision(4) << accuracy
              << std::flush;
}

void exportTrainingMetrics(const std::string& filePath, 
                          double finalAccuracy,
                          int modelVersion,
                          const TrainingConfig& config) {
    std::ofstream metrics(filePath);
    metrics << "Final Accuracy: " << std::fixed << std::setprecision(6) << finalAccuracy << "\n"
            << "Model Version: " << modelVersion << "\n"
            << "Training Iterations: " << config.iterations << "\n"
            << "Learning Rate: " << config.learningRate << "\n"
            << "Batch Size: " << config.batchSize << "\n";
}

void processBatchTraining(const std::string& directory, MediaType mediaType, const TrainingConfig& config) {
    std::vector<MediaType> types = {mediaType};
    auto model = std::make_shared<AIModel>("BatchTrainModel", types);
    ModelAgent agent(model);
    int fileCount = 0;
    int totalFiles = std::distance(std::filesystem::directory_iterator(directory),
                                 std::filesystem::directory_iterator());

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            try {
                std::vector<uint8_t> data = utils::loadBinaryFile(entry.path().string());
                std::string textContent(data.begin(), data.end());

                std::map<std::string, std::string> params;
                params["mode"] = "training";
                params["learning_rate"] = std::to_string(config.learningRate);
                params["batch_size"] = std::to_string(config.batchSize);
                params["iterations"] = std::to_string(config.iterations);

                AgentContext context{
                    .mediaType = mediaType,
                    .input = textContent,
                    .binaryData = data,
                    .parameters = params
                };

                agent.processContext(context);
                fileCount++;

                if (config.showProgress) {
                    printProgress(fileCount, totalFiles, model->getAccuracy());
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing file " << entry.path().filename() 
                         << ": " << e.what() << std::endl;
                continue;
            }
        }
    }

    if (config.showProgress) {
        std::cout << std::endl;
    }

    if (!config.metricsFile.empty()) {
        exportTrainingMetrics(config.metricsFile, model->getAccuracy(), 
                            model->getVersion(), config);
    }

    std::cout << "\nTraining Summary:\n"
              << "Files Processed: " << fileCount << "/" << totalFiles << "\n"
              << "Final Accuracy: " << model->getAccuracy() << "\n"
              << "Model Version: " << model->getVersion() << std::endl;
}

void runTests();
void testMediaModels();
void testAgentCapabilities();

void printUsage() {
    std::cout << "Usage: aimarket [OPTION]... [FILE]\n"
              << "Options:\n"
              << "  --status                    Get agent status\n"
              << "  --process FILE --type TYPE  Process input file\n"
              << "  --train FILE --type TYPE    Train with input file\n"
              << "  --batch-train DIR --type TYPE Train with all files in directory\n"
              << "  --config FILE              Load training configuration\n"
              << "  --learning-rate RATE       Set learning rate (default: 0.01)\n"
              << "  --batch-size SIZE          Set batch size (default: 32)\n"
              << "  --iterations NUM           Set training iterations (default: 100)\n"
              << "  --show-progress            Show training progress\n"
              << "  --export-metrics FILE      Export training metrics to file\n"
              << "  --reasoning                Get agent reasoning\n"
              << "  --test                     Run test suite\n"
              << "  --version                  Print version\n"
              << "  --help                     Print this help\n"
              << "  --crawl URL                Crawl URL and train with content\n";
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int processCommandLine(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];
    TrainingConfig config;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--learning-rate" && i + 1 < argc) {
            config.learningRate = std::stod(argv[++i]);
        } else if (arg == "--batch-size" && i + 1 < argc) {
            config.batchSize = std::stoi(argv[++i]);
        } else if (arg == "--iterations" && i + 1 < argc) {
            config.iterations = std::stoi(argv[++i]);
        } else if (arg == "--show-progress") {
            config.showProgress = true;
        } else if (arg == "--export-metrics" && i + 1 < argc) {
            config.metricsFile = argv[++i];
        }
    }

    if (command == "--batch-train" && argc >= 5) {
        std::string directory = argv[2];
        if (std::string(argv[3]) != "--type") {
            std::cerr << "Error: Missing --type argument\n";
            return 1;
        }
        std::string mediaTypeStr = argv[4];

        MediaType mediaType;
        if (mediaTypeStr == "TEXT") mediaType = MediaType::TEXT;
        else if (mediaTypeStr == "IMAGE") mediaType = MediaType::IMAGE;
        else if (mediaTypeStr == "AUDIO") mediaType = MediaType::AUDIO;
        else if (mediaTypeStr == "VIDEO") mediaType = MediaType::VIDEO;
        else {
            std::cerr << "Error: Invalid media type\n";
            return 1;
        }

        processBatchTraining(directory, mediaType, config);
        return 0;
    }

    if (command == "--status") {
        std::vector<MediaType> types = {MediaType::TEXT};
        auto model = std::make_shared<AIModel>("StatusModel", types);
        ModelAgent agent(model);
        std::cout << static_cast<int>(agent.getState()) << std::endl;
        return 0;
    }

    if (command == "--crawl" && argc >= 3) {
        std::string url = argv[2];
        std::string goal = argc >= 4 ? argv[3] : "";
        std::string mediaTypeStr = "TEXT";  

        std::string cmd = "python3 src/web_crawler.py \"" + url + "\" \"" + goal + "\"";
        std::string result = exec(cmd.c_str());

        if (result.find("\"success\": true") != std::string::npos) {
            std::vector<MediaType> types = {MediaType::TEXT};
            auto model = std::make_shared<AIModel>("WebCrawlerModel", types);
            ModelAgent agent(model);

            std::vector<uint8_t> data = utils::loadBinaryFile("temp/crawled_data.txt");
            std::string textContent(data.begin(), data.end());

            AgentContext context{
                .mediaType = MediaType::TEXT,
                .input = textContent,
                .binaryData = data,
                .parameters = {{"mode", "training"}}
            };

            agent.processContext(context);
            std::cout << "Training complete. Accuracy: " << model->getAccuracy()
                      << "\nFeedback: " << agent.getActionReasoning() << std::endl;
        } else {
            std::cerr << "Error crawling website: " << result << std::endl;
            return 1;
        }
        return 0;
    }

    if (command == "--process" && argc >= 5) {
        std::string filePath = argv[2];
        if (std::string(argv[3]) != "--type") {
            std::cerr << "Error: Missing --type argument\n";
            return 1;
        }
        std::string mediaTypeStr = argv[4];

        MediaType mediaType;
        if (mediaTypeStr == "TEXT") mediaType = MediaType::TEXT;
        else if (mediaTypeStr == "IMAGE") mediaType = MediaType::IMAGE;
        else if (mediaTypeStr == "AUDIO") mediaType = MediaType::AUDIO;
        else if (mediaTypeStr == "VIDEO") mediaType = MediaType::VIDEO;
        else {
            std::cerr << "Error: Invalid media type\n";
            return 1;
        }

        std::vector<MediaType> types = {mediaType};
        auto model = std::make_shared<AIModel>("ProcessModel", types);
        ModelAgent agent(model);

        std::vector<uint8_t> data = utils::loadBinaryFile(filePath);
        std::string textContent(data.begin(), data.end());

        AgentContext context{
            .mediaType = mediaType,
            .input = textContent,
            .binaryData = data,
            .parameters = {{"mode", "process"}}
        };

        agent.processContext(context);
        std::cout << "Processed result: " << agent.getActionReasoning() << std::endl;
        return 0;
    }

    if (command == "--train" && argc >= 5) {
        std::string filePath = argv[2];
        if (std::string(argv[3]) != "--type") {
            std::cerr << "Error: Missing --type argument\n";
            return 1;
        }
        std::string mediaTypeStr = argv[4];

        MediaType mediaType;
        if (mediaTypeStr == "TEXT") mediaType = MediaType::TEXT;
        else if (mediaTypeStr == "IMAGE") mediaType = MediaType::IMAGE;
        else if (mediaTypeStr == "AUDIO") mediaType = MediaType::AUDIO;
        else if (mediaTypeStr == "VIDEO") mediaType = MediaType::VIDEO;
        else {
            std::cerr << "Error: Invalid media type\n";
            return 1;
        }

        std::vector<MediaType> types = {mediaType};
        auto model = std::make_shared<AIModel>("TrainModel", types);
        ModelAgent agent(model);

        std::vector<uint8_t> data = utils::loadBinaryFile(filePath);
        std::string textContent(data.begin(), data.end());

        AgentContext context{
            .mediaType = mediaType,
            .input = textContent,
            .binaryData = data,
            .parameters = {{"mode", "training"}}
        };

        agent.processContext(context);
        std::cout << "Training complete. Accuracy: " << model->getAccuracy()
                  << "\nFeedback: " << agent.getActionReasoning() << std::endl;
        return 0;
    }

    if (command == "--reasoning") {
        std::vector<MediaType> types = {MediaType::TEXT};
        auto model = std::make_shared<AIModel>("ReasoningModel", types);
        ModelAgent agent(model);
        std::cout << agent.getActionReasoning() << std::endl;
        return 0;
    }

    if (command == "--test") {
        runTests();
        return 0;
    }

    if (command == "--version") {
        std::cout << "AIMarket v1.0.0\n";
        return 0;
    }

    if (command == "--help") {
        printUsage();
        return 0;
    }

    std::cerr << "Error: Unknown command\n";
    printUsage();
    return 1;
}

void printSeparator() {
    std::cout << "\n" << std::string(50, '-') << "\n";
}

void testMediaModels() {
    std::cout << "\nTesting Multi-Modal Model Capabilities...\n";
    printSeparator();

    std::vector<MediaType> allTypes = {
        MediaType::TEXT,
        MediaType::IMAGE,
        MediaType::AUDIO,
        MediaType::VIDEO
    };
    AIModel multiModel("MultiModal-Mock", allTypes);

    std::cout << "Testing Text Processing:\n";
    multiModel.trainWithText("Sample training text for natural language processing.");
    std::string processedText = multiModel.processText("Test input text");
    std::cout << "Text processing result: " << processedText << "\n";

    std::cout << "\nTesting Image Processing:\n";
    std::vector<uint8_t> mockImageData(224 * 224 * 3, 128);  
    multiModel.trainWithImage(mockImageData);
    auto processedImage = multiModel.processImage(mockImageData);
    std::cout << "Image processing completed, output size: " << processedImage.size() << " bytes\n";

    std::cout << "\nTesting Audio Processing:\n";
    std::vector<uint8_t> mockAudioData(16000 * 2, 64);  
    multiModel.trainWithAudio(mockAudioData);
    auto processedAudio = multiModel.processAudio(mockAudioData);
    std::cout << "Audio processing completed, output size: " << processedAudio.size() << " bytes\n";

    std::cout << "\nTesting Video Processing:\n";
    std::vector<uint8_t> mockVideoData(224 * 224 * 3 * 30, 192);  
    multiModel.trainWithVideo(mockVideoData);
    auto processedVideo = multiModel.processVideo(mockVideoData);
    std::cout << "Video processing completed, output size: " << processedVideo.size() << " bytes\n";

    std::cout << "\nModel Information:\n";
    std::cout << "Model ID: " << multiModel.getId() << "\n";
    std::cout << "Model Name: " << multiModel.getName() << "\n";
    std::cout << "Supported Media Types:\n";
    for (const auto& type : multiModel.getSupportedTypes()) {
        std::string typeName;
        switch(type) {
            case MediaType::TEXT: typeName = "TEXT"; break;
            case MediaType::IMAGE: typeName = "IMAGE"; break;
            case MediaType::AUDIO: typeName = "AUDIO"; break;
            case MediaType::VIDEO: typeName = "VIDEO"; break;
        }
        std::cout << "- " << typeName << " processing supported\n";
    }
    std::cout << "Model Version: " << multiModel.getVersion() << "\n";
    std::cout << "Model Accuracy: " << multiModel.getAccuracy() << "\n";
}

void testAgentCapabilities() {
    std::cout << "\nTesting Agent Capabilities...\n";
    printSeparator();

    std::vector<MediaType> allTypes = {
        MediaType::TEXT,
        MediaType::IMAGE,
        MediaType::AUDIO,
        MediaType::VIDEO
    };
    auto model = std::make_shared<AIModel>("Agent-Test-Model", allTypes);

    ModelAgent agent(model);

    std::cout << "Test 1: Agent Text Processing\n";
    AgentContext textContext{
        .mediaType = MediaType::TEXT,
        .input = "Sample text for agent processing",
        .binaryData = std::vector<uint8_t>(),
        .parameters = {{"mode", "analysis"}}
    };
    agent.processContext(textContext);
    std::cout << "Agent reasoning: " << agent.getActionReasoning() << "\n";

    std::cout << "\nTest 2: Agent Image Processing\n";
    std::vector<uint8_t> mockImageData(224 * 224 * 3, 128);
    AgentContext imageContext{
        .mediaType = MediaType::IMAGE,
        .input = "",
        .binaryData = mockImageData,
        .parameters = {{"mode", "training"}}
    };
    agent.processContext(imageContext);
    std::cout << "Agent reasoning: " << agent.getActionReasoning() << "\n";

    std::cout << "\nTest 3: Agent Learning\n";
    agent.learn(textContext, "Good performance on text analysis");
    agent.learn(imageContext, "Image processing needs improvement");

    std::cout << "\nTest 4: Agent State Management\n";
    std::cout << "Initial state: " << static_cast<int>(agent.getState()) << "\n";
    agent.setState(AgentState::PROCESSING);
    std::cout << "Processing state: " << static_cast<int>(agent.getState()) << "\n";

    std::cout << "\nTest 5: Agent Decision Making\n";
    AgentAction nextAction = agent.decideNextAction(textContext);
    std::cout << "Next action for text context: " << static_cast<int>(nextAction) << "\n"
              << "Reasoning: " << agent.getActionReasoning() << "\n";
}

void runTests() {
    std::cout << "Running Enhanced AI Model Marketplace Tests...\n";
    BlockchainLedger ledger;
    ModelStorage storage;

    printSeparator();
    std::cout << "Test 1: Creating models with version tracking...\n";

    std::vector<MediaType> allTypes = {
        MediaType::TEXT,
        MediaType::IMAGE,
        MediaType::AUDIO,
        MediaType::VIDEO
    };
    AIModel model("Universal-Mock", allTypes);

    storage.storeModel(model);
    ledger.addTransaction("CREATE", model.getId(), "system", "", 0.0);
    std::cout << "Created multi-modal model successfully (Initial version: "
              << model.getVersion() << ")\n";

    printSeparator();
    std::cout << "Test 2: Training and version updates...\n";
    for (int i = 0; i < 3; i++) {
        model.train();
        storage.updateModel(model);
        std::cout << "Training iteration " << (i + 1)
                  << " completed. Version: " << model.getVersion()
                  << ", Accuracy: " << std::fixed << std::setprecision(4)
                  << model.getAccuracy() << std::endl; 
        std::cout << "Debug: Model internal state after iteration " << i + 1 << ": " << model.getDebugInfo() << std::endl; 
    }

    printSeparator();
    std::cout << "Test 3: Testing collaborative training...\n";
    std::vector<std::string> contributors = {"alice", "bob", "charlie"};
    std::vector<double> contributions = {10.5, 8.3, 15.2}; 
    ledger.addCollaborativeTransaction(model.getId(), contributors, contributions);
    std::cout << "Collaborative training session recorded\n";

    printSeparator();
    std::cout << "Test 4: Testing community voting system...\n";
    ledger.addVote(model.getId(), "user1", 5, "Excellent performance!");
    ledger.addVote(model.getId(), "user2", 4, "Good but could be better");
    ledger.addVote(model.getId(), "user3", 5, "Revolutionary!");

    double rating = ledger.getModelRating(model.getId());
    std::cout << "Model average rating: " << rating << "/5.0\n";

    printSeparator();
    std::cout << "Test 5: Testing resource sharing and reputation...\n";
    ledger.updateResourceContribution("alice", 25.5); 
    ledger.updateResourceContribution("bob", 15.3);

    auto aliceRep = ledger.getUserReputation("alice");
    std::cout << "Alice's reputation score: " << aliceRep.score << "\n";

    printSeparator();
    std::cout << "Test 6: Testing fair pricing mechanism...\n";
    double fairPrice = ledger.calculateFairPrice(model.getId());
    std::cout << "Calculated fair price: " << fairPrice << " tokens\n";

    printSeparator();
    std::cout << "Test 7: Viewing top contributors...\n";
    auto topContributors = ledger.getTopContributors();
    std::cout << "Top contributors:\n";
    for (const auto& contributor : topContributors) {
        auto rep = ledger.getUserReputation(contributor);
        std::cout << "User: " << contributor
                  << ", Reputation: " << rep.score
                  << ", Models Shared: " << rep.modelsShared << "\n";
    }

    printSeparator();
    std::cout << "Test 8: Testing rental with reputation...\n";
    const std::string renter = "dave";
    const std::string owner = "alice";
    const double rentalPrice = ledger.calculateFairPrice(model.getId()) * 0.1; 
    const time_t rentalDuration = 24 * 3600; 

    ledger.addTransaction("RENT", model.getId(), renter, owner,
                         rentalPrice, rentalDuration);

    if (ledger.isModelRentedBy(model.getId(), renter)) {
        std::cout << "Model successfully rented to " << renter
                  << " for 24 hours at " << rentalPrice << " tokens\n";
    }

    printSeparator();
    std::cout << "Test 9: Testing documentation system...\n";
    std::vector<std::string> tags = {"tutorial", "best-practices", "training"};
    ledger.addDocumentation(model.getId(), "expert1",
                            "Comprehensive guide to training this model effectively.", tags);
    ledger.upvoteDocumentation(model.getId(), "user1");
    ledger.addDocComment(model.getId(), "user2", "Very helpful guide!");

    auto docs = ledger.getModelDocs(model.getId());
    std::cout << "Documentation entries: " << docs.size()
              << ", Upvotes: " << docs[0].upvotes
              << ", Comments: " << docs[0].comments.size() << "\n";

    printSeparator();
    std::cout << "Test 10: Testing quality control system...\n";
    QualityMetrics metrics{
        .accuracy = 0.95,
        .reliability = 0.98,
        .userCount = 100,
        .avgResponseTime = 0.05,
        .validations = std::vector<std::string>(),
        .lastAudit = std::time(nullptr)
    };
    ledger.updateQualityMetrics(model.getId(), metrics);
    ledger.validateModel(model.getId(), "validator1");

    auto quality = ledger.getModelQuality(model.getId());
    std::cout << "Model quality metrics - Accuracy: " << quality.accuracy
              << ", Reliability: " << quality.reliability
              << ", Validations: " << quality.validations.size() << "\n";

    printSeparator();
    std::cout << "Test 11: Testing advanced reward distribution...\n";
    ledger.distributeRewards(model.getId(), 1000.0); 

    for (const auto& contributor : {"alice", "bob", "charlie"}) {
        double reward = ledger.calculateUserReward(contributor, model.getId());
        std::cout << "Reward for " << contributor << ": " << reward << " tokens\n";
    }

    printSeparator();
    std::cout << "Test 12: Testing resource optimization...\n";
    ResourceUsage usage{120.5, 48.3, 256.0, 1024.0, 500.0};
    ledger.trackResourceUsage(model.getId(), usage);

    double efficiency = ledger.optimizeResourceAllocation(model.getId());
    auto optimizedUsage = ledger.getResourceMetrics(model.getId());
    std::cout << "Resource efficiency score: " << efficiency
              << "\nOptimized cost: " << optimizedUsage.costTokens << " tokens\n";

    printSeparator();
    std::cout << "Test 13: Testing version control system...\n";
    ModelVersion version{1, utils::hashString("v1"), "", std::time(nullptr),
                         "Initial release", true};
    ledger.addModelVersion(model.getId(), version);

    if (ledger.rollbackVersion(model.getId(), 1)) {
        std::cout << "Successfully rolled back to version 1\n";
    }

    auto history = ledger.getVersionHistory(model.getId());
    std::cout << "Version history entries: " << history.size() << "\n";

    printSeparator();
    std::cout << "Test 14: Verifying blockchain integrity...\n";
    if (ledger.verifyChain()) {
        std::cout << "Blockchain verification successful\n";
    } else {
        std::cout << "Blockchain verification failed\n";
    }

    printSeparator();
    std::cout << "\nTesting Multi-Modal Model Capabilities:\n";
    testMediaModels();
    printSeparator();
    std::cout << "\nTesting Agent Capabilities:\n";
    testAgentCapabilities();
}

int main(int argc, char* argv[]) {
    try {
        return processCommandLine(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}