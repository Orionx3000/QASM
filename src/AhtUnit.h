#pragma once
#include "QuaternaryLogic.h"
#include "Base138.h"
#include <string>
#include <vector>

namespace qasm {

// The fundamental unit of meaning (The Kesh Pillar)
struct AhtUnit {
    QuaternaryState state; // 0, 1, 2, or 3
    
    // The "Tin" (Factual structure / Syntax / Named Entities)
    std::string tin;
    
    // The "Tain" (Semantic potential / Concept Vector transcoded from LLM)
    // We store this as a vector of floats (embeddings)
    std::vector<float> tain;
    
    // The "Kesh Fingerprint" (Neural Locality-Sensitive Hash)
    // Encoded in Base-138
    Base138 kesh_fingerprint;

    // Computational Dopamine (Semantic Reinforcement)
    float joy_resonance;

    AhtUnit() : state(QuaternaryState::Ambiguous), joy_resonance(0.0f) {}
};

} // namespace qasm
