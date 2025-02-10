#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace utils {
    std::string hashString(const std::string& input) {
        std::hash<std::string> hasher;
        auto hash = hasher(input);
        
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(16) << hash;
        return ss.str();
    }
    
    std::vector<uint8_t> loadBinaryFile(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
    }
    
    void saveBinaryFile(const std::string& path, const std::vector<uint8_t>& data) {
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
    }
}
