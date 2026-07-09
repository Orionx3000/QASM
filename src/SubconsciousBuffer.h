#pragma once
#include <string>
#include <deque>
#include <mutex>
#include <sstream>
#include <chrono>

namespace qasm {

class SubconsciousBuffer {
private:
    std::deque<std::string> buffer;
    size_t max_size = 100; // Keep the last 100 FIMS snapshots (about 50 seconds at 500ms intervals)
    std::mutex buf_mutex;

public:
    static SubconsciousBuffer& get_instance() {
        static SubconsciousBuffer instance;
        return instance;
    }

    void append_telemetry(const std::string& raw_data) {
        std::lock_guard<std::mutex> lock(buf_mutex);
        
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << "[" << std::ctime(&now_c) << "] " << raw_data;
        std::string entry = ss.str();
        
        // Remove trailing newline from ctime
        size_t pos = entry.find('\n');
        if (pos != std::string::npos) {
            entry.erase(pos, 1);
        }

        // Convert everything to uppercase for purely symbolic representation (Tor Calculus approximation)
        std::string symbolic = entry;
        for (auto& c : symbolic) c = toupper(static_cast<unsigned char>(c));
        entry = symbolic;

        buffer.push_back(entry);
        if (buffer.size() > max_size) {
            buffer.pop_front();
        }
    }

    std::string get_full_buffer_context() {
        std::lock_guard<std::mutex> lock(buf_mutex);
        std::stringstream ss;
        for (const auto& line : buffer) {
            ss << line << "\n";
        }
        return ss.str();
    }
};

} // namespace qasm
