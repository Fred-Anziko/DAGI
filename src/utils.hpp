#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace utils {
    std::string hashString(const std::string& input);
    std::vector<std::uint8_t> loadBinaryFile(const std::string& path);
    void saveBinaryFile(const std::string& path, const std::vector<std::uint8_t>& data);
}