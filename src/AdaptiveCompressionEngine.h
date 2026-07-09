#pragma once
#include "AhtUnit.h"
#include <string>

namespace qasm {

class AdaptiveCompressionEngine {
public:
    // Write an AhtUnit to a highly compressed Base-138 file (discarding Tin text if needed)
    static bool write_aht_file(const AhtUnit& unit, const std::string& filepath);

    // Read an AhtUnit from a compressed Base-138 file
    static bool read_aht_file(const std::string& filepath, AhtUnit& out_unit);
};

} // namespace qasm
