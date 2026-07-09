#pragma once
#include <thread>
#include <chrono>
#include <iostream>
#include "SubconsciousBuffer.h"
#include "LlamaEngine.h"
#include "Conductor.h"

namespace qasm {

class AutonomousHeartbeat {
private:
    std::shared_ptr<Conductor> conductor;
    bool is_running;

public:
    AutonomousHeartbeat(std::shared_ptr<Conductor> cond) 
        : conductor(cond), is_running(false) {}

    void start_heartbeat() {
        if (is_running) return;
        is_running = true;

        std::thread([this]() {
            std::cout << "[AUTONOMY] Heartbeat initiated. OS is now proactive.\n";
            while (is_running) {
                // The Heartbeat ticks every 30 seconds
                std::this_thread::sleep_for(std::chrono::seconds(30));

                std::cout << "[HEARTBEAT] Waking up. Scanning Subconscious Buffer...\n";
                std::string context = SubconsciousBuffer::get_instance().get_full_buffer_context();
                
                if (context.find("ERROR") != std::string::npos || context.find("Trauma") != std::string::npos || context.find("anomaly") != std::string::npos) {
                    std::cout << "[HEARTBEAT] Dissonance detected. Spawning new Thought Track...\n";
                    conductor->spawn_track(context.substr(0, 50));
                }
                
                // Evaluate all active trains of thought non-linearly
                conductor->evaluate_tracks();
                
            }
        }).detach();
    }
};

} // namespace qasm
