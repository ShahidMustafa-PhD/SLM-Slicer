#pragma once
#include <iostream>
#include <string>
#include <filesystem>
#include "MarcAPIManager.hpp" // Make sure this path is correct

namespace Marc {

    inline void CheckAndReport(const std::string& action, MarcErrorCode code) {
        if (code != MARC_S_OK) {
            std::cerr << "[ERROR] " << action << " failed with code: 0x"
                      << std::hex << code << std::dec << std::endl;
        } else {
            std::cout << "[OK] " << action << " succeeded." << std::endl;
        }
    }

    inline bool CheckFileExists(const std::filesystem::path& p, const std::string& label = "") {
        if (!std::filesystem::exists(p)) {
            std::cerr << "[ERROR] " << (label.empty() ? "File" : label)
                      << " does not exist: " << p << std::endl;
            return false;
        }
        return true;
    }

    // Optional macro for quick use
    #define CHECK_MARC_CALL(action, call) \
        do { \
            MarcErrorCode ec = (call); \
            if (ec != MARC_S_OK) { \
                std::cerr << "[ERROR] " << (action) << " failed with code: 0x" \
                          << std::hex << ec << std::dec << std::endl; \
            } else { \
                std::cout << "[OK] " << (action) << " succeeded." << std::endl; \
            } \
        } while(0)

} // namespace Marc
