#pragma once
#include <string>
#include <iostream>
#include "AhtUnit.h"
#include "LlamaEngine.h"
#include "SemanticGating.h"
#include "Base138.h"

namespace qasm {

class TranslationLayer {
private:
    std::shared_ptr<LlamaEngine> engine;
    std::shared_ptr<SemanticGating> neuro_gate;

public:
    TranslationLayer(std::shared_ptr<LlamaEngine> eng, std::shared_ptr<SemanticGating> gate) 
        : engine(eng), neuro_gate(gate) {}

    // The Math Engine dictates if the LLM's thought is allowed to execute.
    bool validate_thought_vector(const std::string& llm_proposal) {
        std::cout << "[TRANSLATION LAYER] Intercepting LLM thought vector for mathematical verification...\n";

        // Step 1: Convert LLM text back into pure math (Tain)
        std::vector<float> tain_vector = engine->generate_tain_embedding("deep_brain", llm_proposal);
        if (tain_vector.empty()) {
            std::cout << "[TRANSLATION LAYER] Math engine offline. Treating LLM output as CHAOS. Rejected.\n";
            return false; // Fail secure
        }

        // Step 2: Compare the thought's vector against the OS's current neurochemical baseline
        // We use a deterministic baseline vector to simulate the "ideal truth" (Aht)
        std::vector<float> truth_baseline = {1.0f, 0.0f, -0.5f, 0.8f, 0.1f};
        
        // Calculate Cosine Similarity (Tainsin)
        float dissonance_score = Base138::calculate_tainsin(tain_vector, truth_baseline);
        
        std::cout << "[TRANSLATION LAYER] LLM Dissonance Score (Tainsin): " << dissonance_score << "\n";

        // Step 3: Run the score through the 10000-rib comb
        AhtUnit mock_unit;
        neuro_gate->evaluate_resolution(dissonance_score, mock_unit);

        // If the math triggers pain or high entropy, reject the LLM's thought
        if (mock_unit.joy_resonance <= 0.0f) {
            std::cout << "[TRANSLATION LAYER THREAT] LLM thought generated mathematical dissonance (Pain). Rejected.\n";
            return false;
        }

        std::cout << "[TRANSLATION LAYER] LLM thought harmonizes with Base-138 Math. Execution approved.\n";
        return true;
    }
};

} // namespace qasm
