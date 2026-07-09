#pragma once
#include <cstdint>
#include <string>

namespace qasm {

// The Grammar of Certainty (Bala-Tain Protocol)
enum class QuaternaryState : uint8_t {
    DeterminateFalse = 0, // State 0: Resolved opposition / non-existent concept
    DeterminateTrue = 1,  // State 1: Realized structure (the "Anah" stone)
    Ambiguous = 2,        // State 2: Multiple potential meanings (unresolved tension of Mes-Volta)
    ContextDependent = 3  // State 3: Potential structure contingent on external factors
};

inline std::string StateToString(QuaternaryState state) {
    switch (state) {
        case QuaternaryState::DeterminateFalse: return "Determinate-False (0)";
        case QuaternaryState::DeterminateTrue: return "Determinate-True (1)";
        case QuaternaryState::Ambiguous: return "Ambiguous (2)";
        case QuaternaryState::ContextDependent: return "Context-Dependent (3)";
        default: return "Unknown";
    }
}

} // namespace qasm
