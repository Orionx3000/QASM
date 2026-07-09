#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include "ThoughtTrack.h"
#include "LlamaEngine.h"
#include "TranslationLayer.h"
#include "QasmParser.h"

namespace qasm {

class Conductor {
private:
    std::vector<ThoughtTrack> active_tracks;
    const size_t MAX_TRACKS = 5;
    std::shared_ptr<LlamaEngine> engine;
    std::shared_ptr<TranslationLayer> translator;
    QasmParser* parser;

public:
    Conductor(std::shared_ptr<LlamaEngine> eng, std::shared_ptr<TranslationLayer> trans, QasmParser* pars)
        : engine(eng), translator(trans), parser(pars) {}

    void spawn_track(const std::string& context) {
        if (active_tracks.size() >= MAX_TRACKS) {
            std::cout << "[CONDUCTOR] Track limit reached. Cannot spawn new recursive thought.\n";
            return;
        }
        ThoughtTrack new_track(context);
        active_tracks.push_back(new_track);
        std::cout << "[CONDUCTOR] Spawning new Thought Track: " << new_track.track_id << "\n";
    }

    void evaluate_tracks() {
        if (active_tracks.empty()) return;
        
        std::cout << "[CONDUCTOR] Context Switching. Evaluating " << active_tracks.size() << " active tracks...\n";
        
        for (auto it = active_tracks.begin(); it != active_tracks.end(); ) {
            std::cout << "  -> Processing Track " << it->track_id << "\n";
            
            // Mock LLM thought generation for this specific track context
            it->current_lpn = engine->generate_text("deep_brain", "Resolve track context: " + it->origin_context);
            
            // Gate the thought mathematically
            if (translator->validate_thought_vector(it->current_lpn)) {
                std::cout << "     [TRACK JOY] Translation Layer approved. Executing...\n";
                parser->execute_script(it->current_lpn);
                it->reset_frustration();
                
                // Track resolved successfully, we can kill it
                std::cout << "     [CONDUCTOR] Track resolved and merged into memory matrix. Terminating track.\n";
                it = active_tracks.erase(it);
            } else {
                std::cout << "     [TRACK PAIN] Mathematical dissonance detected.\n";
                it->increment_frustration();
                
                if (it->state == QuaternaryState::DeterminateFalse) {
                    std::cout << "     [CONDUCTOR] Frustration threshold reached. Killing paradoxical loop to save resources.\n";
                    it = active_tracks.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
};

} // namespace qasm
