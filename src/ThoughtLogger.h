#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#define MKDIR(dir) mkdir(dir, 0777)
#endif

namespace qasm {

class ThoughtLogger {
private:
    std::string log_dir;
    std::ofstream current_file;
    int file_index = 1;
    int line_count = 0;
    const int MAX_LINES_PER_FILE = 5000; // Rotate file every 5000 lines

    std::string get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }

    void open_new_file() {
        if (current_file.is_open()) {
            current_file.close();
        }
        std::stringstream filename;
        // e.g. D:\App Creation\QASM\logs\thought_log_0001.txt
        filename << log_dir << "\\thought_log_" << std::setfill('0') << std::setw(4) << file_index << ".txt";
        current_file.open(filename.str(), std::ios::app);
        if (current_file.is_open()) {
            current_file << "================================================================================\n";
            current_file << "TINATEN OS - INTERNAL THOUGHT LOG (VOLUME " << file_index << ")\n";
            current_file << "INITIATED: " << get_timestamp() << "\n";
            current_file << "================================================================================\n\n";
        }
        file_index++;
        line_count = 0;
    }

public:
    ThoughtLogger(const std::string& directory = "D:\\App Creation\\QASM\\logs") {
        log_dir = directory;
        MKDIR(log_dir.c_str());
        open_new_file();
    }

    ~ThoughtLogger() {
        if (current_file.is_open()) {
            current_file << "\n[LOGGER TERMINATED AT " << get_timestamp() << "]\n";
            current_file.close();
        }
    }

    void log(const std::string& module, const std::string& message) {
        if (!current_file.is_open()) return;

        std::string time_str = get_timestamp();
        current_file << "[" << time_str << "] [" << module << "] " << message << "\n";
        current_file.flush(); // Ensure immediate write to disk so user can parse it live
        
        line_count++;
        if (line_count >= MAX_LINES_PER_FILE) {
            open_new_file();
        }
    }
};

} // namespace qasm
