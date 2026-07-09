#include "AdaptiveCompressionEngine.h"
#include "Base138.h"
#include <fstream>
#include <iostream>

namespace qasm {

bool AdaptiveCompressionEngine::write_aht_file(const AhtUnit& unit, const std::string& filepath) {
    std::ofstream out(filepath, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open file for writing: " << filepath << "\n";
        return false;
    }

    // Write the Base-138 encoded Kesh Fingerprint first
    out << unit.kesh_fingerprint.encoded_value << "\n";

    // Write the state as a raw uint8_t mapped to Base-138 (0, 1, 2, 3)
    out << Base138::from_uint64(static_cast<uint64_t>(unit.state)).encoded_value << "\n";

    // Write the Tain (Concept Vector)
    // In a real implementation, we would encode the floating point arrays into Base-138 chunks.
    // Here we encode a string representation of the vector.
    std::string tain_str = "";
    for (float f : unit.tain) {
        tain_str += std::to_string(f) + ",";
    }
    out << Base138::encode_string(tain_str).encoded_value << "\n";

    // Notice we do NOT save the `tin` (raw syntax).
    // The Adaptive Compression Engine discards raw text to maximize compression.
    // The generative model will reconstruct it later from the Tain.

    out.close();
    std::cout << "[ACE] Successfully wrote compressed AHT file to: " << filepath << "\n";
    return true;
}

bool AdaptiveCompressionEngine::read_aht_file(const std::string& filepath, AhtUnit& out_unit) {
    std::ifstream in(filepath, std::ios::binary);
    if (!in) {
        std::cerr << "Failed to open file for reading: " << filepath << "\n";
        return false;
    }

    // Read implementation stub
    std::string line;
    std::getline(in, line);
    out_unit.kesh_fingerprint = Base138(line);

    std::getline(in, line);
    // Parse state from Base-138 back to uint8_t
    // ...

    in.close();
    return true;
}

} // namespace qasm
