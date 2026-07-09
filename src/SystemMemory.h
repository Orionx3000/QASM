#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace qasm {
class SystemMemory {
public:
    inline static int boot_count = 0;
    inline static std::string last_sleep = "Unknown";
    
    static std::string get_current_time_str() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        // Suppress warning on windows
        #pragma warning(suppress : 4996)
        ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    static void init() {
        std::ifstream file("system_memory.txt");
        if (file.is_open()) {
            std::string line;
            while(std::getline(file, line)) {
                if (line.find("boot_count=") == 0) boot_count = std::stoi(line.substr(11));
                if (line.find("last_sleep=") == 0) last_sleep = line.substr(11);
            }
            file.close();
        }
        boot_count++;
        save();
    }

    static void save() {
        std::ofstream file("system_memory.txt");
        file << "boot_count=" << boot_count << "\n";
        file << "last_sleep=" << get_current_time_str() << "\n";
        file.close();
    }
};
}
