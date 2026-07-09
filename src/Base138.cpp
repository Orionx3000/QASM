#include "Base138.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <stdexcept>

namespace qasm {

// Map 0-137 to strings
std::string get_glyphthem(uint8_t value) {
    if (value == 0) return "𝄻"; // Whole rest
    if (value >= 1 && value <= 100) {
        if (value < 10) return std::to_string(value);
        return "(" + std::to_string(value) + ")";
    }
    if (value >= 101 && value <= 126) {
        return std::string(1, 'a' + (value - 101));
    }
    switch (value) {
        case 127: return "𝄞"; // G-Clef
        case 128: return "𝄢"; // F-Clef
        case 129: return "𝄡"; // C-Clef
        case 130: return "♯"; // Sharp
        case 131: return "♭"; // Flat
        case 132: return "♮"; // Natural
        case 133: return "𝄐"; // Fermata
        case 134: return "𝄌"; // Coda
        case 135: return "𝄋"; // Segno
        case 136: return "𝄪"; // Double Sharp
        case 137: return "𝄫"; // Double Flat
    }
    return "?"; // Error
}

Base138 Base138::from_uint64(uint64_t value) {
    if (value == 0) return Base138("𝄻");
    
    std::vector<uint8_t> remainders;
    while (value > 0) {
        remainders.push_back(value % 138);
        value /= 138;
    }
    
    std::string result = "";
    for (auto it = remainders.rbegin(); it != remainders.rend(); ++it) {
        result += get_glyphthem(*it);
    }
    
    return Base138(result);
}

// Quick encode string by casting chars to uint64 and base-138 converting them
Base138 Base138::encode_string(const std::string& input) {
    std::string encoded = "";
    for (char c : input) {
        encoded += from_uint64(static_cast<uint64_t>(c)).encoded_value + " ";
    }
    return Base138(encoded);
}
// =========================================================
// The Bala Tor Calculus (True High-Dimensional Vector Math)
// =========================================================

float Base138::calculate_tainsin(const std::vector<float>& vecA, const std::vector<float>& vecB) {
    if (vecA.empty() || vecB.empty() || vecA.size() != vecB.size()) {
        return -2.0f; // Invalid/Dissonant state
    }
    
    float dot_product = 0.0f;
    float normA = 0.0f;
    float normB = 0.0f;
    
    for (size_t i = 0; i < vecA.size(); ++i) {
        dot_product += vecA[i] * vecB[i];
        normA += vecA[i] * vecA[i];
        normB += vecB[i] * vecB[i];
    }
    
    if (normA == 0.0f || normB == 0.0f) {
        return 0.0f; // Orthogonal/Zero vector
    }
    
    return dot_product / (std::sqrt(normA) * std::sqrt(normB));
}

std::vector<float> Base138::calculate_kesh(const std::vector<std::vector<float>>& vectors) {
    if (vectors.empty()) return {};
    
    size_t dim = vectors[0].size();
    std::vector<float> centroid(dim, 0.0f);
    
    int valid_count = 0;
    for (const auto& vec : vectors) {
        if (vec.size() != dim) continue;
        
        for (size_t i = 0; i < dim; ++i) {
            centroid[i] += vec[i];
        }
        valid_count++;
    }
    
    if (valid_count > 0) {
        for (size_t i = 0; i < dim; ++i) {
            centroid[i] /= static_cast<float>(valid_count);
        }
    }
    
    return centroid;
}

} // namespace qasm
