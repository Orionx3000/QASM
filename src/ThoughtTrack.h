#pragma once
#include <string>
#include <vector>
#include "AhtUnit.h"
#include "Base138.h"

namespace qasm {

class ThoughtTrack {
public:
    std::string track_id;
    std::string origin_context;
    std::string current_lpn;
    int frustration_counter;
    QuaternaryState state;
    bool is_active;

    ThoughtTrack(const std::string& context) 
        : origin_context(context), frustration_counter(0), state(QuaternaryState::Ambiguous), is_active(true) {
        track_id = Base138::encode_string(context).encoded_value;
    }

    void increment_frustration() {
        frustration_counter++;
        if (frustration_counter >= 3) {
            state = QuaternaryState::DeterminateFalse; // Hits Pain threshold
        }
    }

    void reset_frustration() {
        frustration_counter = 0;
        state = QuaternaryState::DeterminateTrue; // Hits Joy
    }
};

} // namespace qasm
