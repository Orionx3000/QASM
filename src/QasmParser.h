#pragma once
#include "SemanticCores.h"
#include "AdaptiveCompressionEngine.h"
#include "AhtUnit.h"
#include "ThoughtLogger.h"
#include "SemanticGating.h"
#include "MemoryStorage.h"
#include "SystemMemory.h"
#include "GoalStack.h"
#include "ModelRouter.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace qasm {

class QasmParser {
private:
    std::shared_ptr<SemanticCores> engine;
    ThoughtLogger logger;
    std::shared_ptr<SemanticGating> neuro_gate;
    
    // Store variables/AhtUnits in a registry representing memory
    std::unordered_map<std::string, AhtUnit> memory_registry;
    
    // UI Callback for pushing autonomous thoughts and chat
    std::function<void(const std::string&, const std::string&)> ui_callback = nullptr;

    void execute_line(const std::string& line);
    std::string trim(const std::string& str);

public:
    QasmParser(std::shared_ptr<SemanticCores> llm_engine) : engine(llm_engine) {
        neuro_gate = std::make_shared<SemanticGating>(&logger, engine);
        
        std::cout << "[SYSTEM] Waking primary semantic cores...\n";
        engine->wake_brain("fast_brain", "D:\\App Creation\\QASM\\models\\NVIDIA-Nemotron-3-Nano-4B-Q4_K_M.gguf");
        engine->wake_brain("deep_brain", "D:\\App Creation\\QASM\\models\\gemma-4-E2B-it-Q4_K_M.gguf");
        
        MemoryStorage::deserialize_memory(memory_registry);
    }

    // Parse and execute a full QASM script
    bool execute_script(const std::string& script_content);

    ~QasmParser() {
        MemoryStorage::serialize_memory(memory_registry);
    }
    
    void set_ui_callback(std::function<void(const std::string&, const std::string&)> cb) {
        ui_callback = cb;
    }

    std::function<std::string(const std::string&)> request_user_input = nullptr;
    void set_user_input_callback(std::function<std::string(const std::string&)> cb) {
        request_user_input = cb;
    }

    // Returns the text content of the last QASM command's output.
    // Used by main.cpp to feed results into the next ReAct cycle.
    std::string get_last_result() const {
        auto it = memory_registry.find("temp_unit");
        if (it != memory_registry.end()) {
            return it->second.tin;
        }
        return "";
    }

    // Expose the full memory registry to BalaScriptEngine for algorithm access.
    std::unordered_map<std::string, AhtUnit>& get_memory_registry() {
        return memory_registry;
    }
    const std::unordered_map<std::string, AhtUnit>& get_memory_registry() const {
        return memory_registry;
    }

    // Returns a digest of the N most recently stored long-term AHT nodes.
    // Used by main.cpp to give the AI awareness of what it has already learned.
    std::string get_recent_memories(int n = 5) const {
        std::string digest = "";
        int count = 0;
        // Iterate in reverse to get most recent (unordered_map has no guaranteed order,
        // but we iterate all and take the last n by insertion heuristic)
        std::vector<std::pair<std::string, std::string>> all_mems;
        for (const auto& pair : memory_registry) {
            if (pair.first != "temp_unit" && pair.first != "web_scrape_buffer" &&
                pair.first != "last_lpn_result" && !pair.second.tin.empty()) {
                all_mems.push_back({pair.first, pair.second.tin});
            }
        }
        int start = (int)all_mems.size() - n;
        if (start < 0) start = 0;
        for (int i = start; i < (int)all_mems.size(); i++) {
            std::string preview = all_mems[i].second.substr(0, 120);
            digest += "[" + all_mems[i].first + "]: " + preview + "...\n";
            count++;
        }
        return count > 0 ? digest : "No long-term memories yet.";
    }

    void parse(const std::string& code);
};

} // namespace qasm
