#pragma once
#include "AhtUnit.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace qasm {

class MemoryStorage {
public:
    static const std::string MEMORY_FILE_PATH;

    // Save the entire memory matrix to a binary file
    static bool serialize_memory(const std::unordered_map<std::string, AhtUnit>& registry) {
        std::filesystem::create_directories("storage");
        
        std::ofstream ofs(MEMORY_FILE_PATH, std::ios::binary);
        if (!ofs) {
            std::cerr << "[MemoryStorage] Error: Could not open " << MEMORY_FILE_PATH << " for writing.\n";
            return false;
        }

        // Write the number of entries
        size_t count = registry.size();
        ofs.write(reinterpret_cast<const char*>(&count), sizeof(count));

        for (const auto& pair : registry) {
            // Write key (string)
            size_t key_len = pair.first.length();
            ofs.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
            ofs.write(pair.first.data(), key_len);

            // Write AhtUnit state
            QuaternaryState state = pair.second.state;
            ofs.write(reinterpret_cast<const char*>(&state), sizeof(state));

            // Write tin (string)
            size_t tin_len = pair.second.tin.length();
            ofs.write(reinterpret_cast<const char*>(&tin_len), sizeof(tin_len));
            ofs.write(pair.second.tin.data(), tin_len);

            // Write tain (vector<float>)
            size_t tain_len = pair.second.tain.size();
            ofs.write(reinterpret_cast<const char*>(&tain_len), sizeof(tain_len));
            if (tain_len > 0) {
                ofs.write(reinterpret_cast<const char*>(pair.second.tain.data()), tain_len * sizeof(float));
            }

            // Write kesh_fingerprint (string inside Base138)
            size_t kesh_len = pair.second.kesh_fingerprint.encoded_value.length();
            ofs.write(reinterpret_cast<const char*>(&kesh_len), sizeof(kesh_len));
            ofs.write(pair.second.kesh_fingerprint.encoded_value.data(), kesh_len);

            // Write Computational Dopamine (joy_resonance)
            float joy = pair.second.joy_resonance;
            ofs.write(reinterpret_cast<const char*>(&joy), sizeof(joy));
        }

        ofs.close();
        return true;
    }

    // Load the entire memory matrix from a binary file
    static bool deserialize_memory(std::unordered_map<std::string, AhtUnit>& registry) {
        std::ifstream ifs(MEMORY_FILE_PATH, std::ios::binary);
        if (!ifs) {
            std::cout << "[MemoryStorage] No existing memory matrix found. Starting with a blank slate.\n";
            return false; // File doesn't exist yet, that's fine.
        }

        size_t count = 0;
        if (!ifs.read(reinterpret_cast<char*>(&count), sizeof(count))) {
            return false;
        }

        registry.clear();

        for (size_t i = 0; i < count; ++i) {
            // Read key
            size_t key_len = 0;
            ifs.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));
            std::string key(key_len, '\0');
            ifs.read(&key[0], key_len);

            AhtUnit unit;

            // Read state
            ifs.read(reinterpret_cast<char*>(&unit.state), sizeof(unit.state));

            // Read tin
            size_t tin_len = 0;
            ifs.read(reinterpret_cast<char*>(&tin_len), sizeof(tin_len));
            std::string tin(tin_len, '\0');
            ifs.read(&tin[0], tin_len);
            unit.tin = tin;

            // Read tain
            size_t tain_len = 0;
            ifs.read(reinterpret_cast<char*>(&tain_len), sizeof(tain_len));
            unit.tain.resize(tain_len);
            if (tain_len > 0) {
                ifs.read(reinterpret_cast<char*>(unit.tain.data()), tain_len * sizeof(float));
            }

            // Read kesh_fingerprint
            size_t kesh_len = 0;
            ifs.read(reinterpret_cast<char*>(&kesh_len), sizeof(kesh_len));
            std::string kesh(kesh_len, '\0');
            ifs.read(&kesh[0], kesh_len);
            unit.kesh_fingerprint = Base138(kesh);

            // Read Computational Dopamine (joy_resonance)
            ifs.read(reinterpret_cast<char*>(&unit.joy_resonance), sizeof(unit.joy_resonance));

            registry[key] = std::move(unit);
        }

        ifs.close();
        std::cout << "[MemoryStorage] Memory Matrix loaded successfully. " << count << " thoughts recovered.\n";
        return true;
    }
};

inline const std::string MemoryStorage::MEMORY_FILE_PATH = "storage/aht_core.bin";

} // namespace qasm
