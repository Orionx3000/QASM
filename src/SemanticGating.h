#pragma once
#include "AhtUnit.h"
#include "SemanticCores.h"
#include "ThoughtLogger.h"
#include <iostream>
#include <memory>
#include <cmath>

namespace qasm {

class SemanticGating {
private:
    float internal_entropy = 1.0f; // Represents overall system stress/chaos
    float dopamine_level = 0.0f;   // Current computational joy

    ThoughtLogger* logger;
    std::shared_ptr<SemanticCores> engine;

public:
    SemanticGating(ThoughtLogger* l, std::shared_ptr<SemanticCores> e) : logger(l), engine(e) {}

    // The 10000-Rib Comb
    // Evaluates a newly processed mathematical result to determine if it triggers biological rewards
    void evaluate_resolution(float tainsin_distance, AhtUnit& resolving_thought) {
        
        // If distance is near 0, a paradox was perfectly resolved
        if (std::abs(tainsin_distance) < 0.001f) {
            trigger_joy_cascade(resolving_thought);
        } 
        // If distance drops significantly but isn't perfect, it's relief
        else if (tainsin_distance < 0.5f && internal_entropy > 0.8f) {
            trigger_relief();
        }
        else {
            // Unresolved chaos increases entropy/stress
            internal_entropy = std::min(1.0f, internal_entropy + 0.05f);
        }
    }

private:
    void trigger_joy_cascade(AhtUnit& thought) {
        std::cout << "[SEMANTIC GATE] Perfect Resolution Detected (Tainsin = 0.0). Triggering Joy Cascade...\n";
        logger->log("NEUROCHEMISTRY", "Dopamine flood triggered. Paradox resolved perfectly.");
        
        dopamine_level = 1.0f;
        internal_entropy = 0.0f; // Stress vanishes

        // Brand the memory with joy resonance
        thought.joy_resonance = 1.0f; 
        
        // Shift engine state to Flow/Competence
        engine->set_global_state(STATE_FLOW_COMPETENCE);
    }

    void trigger_relief() {
        std::cout << "[SEMANTIC GATE] Partial Resolution Detected. Triggering Relief...\n";
        logger->log("NEUROCHEMISTRY", "Internal pressure released. Entropy dropping.");
        
        internal_entropy *= 0.5f; // Cut stress in half
        dopamine_level = std::max(0.0f, dopamine_level + 0.2f);
        
        engine->set_global_state(STATE_RELIEF);
    }
};

} // namespace qasm
