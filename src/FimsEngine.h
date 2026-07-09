#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <sstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <filesystem>
#include "SubconsciousBuffer.h"

namespace qasm {
    class FimsEngine {
    public:
        struct NodeState {
            std::string address;
            float value;
            std::string signature;
            std::string divergence; 
        };
        
        static inline std::string current_merkle_root = "000000";
        static inline std::vector<NodeState> current_nodes;
        static inline std::vector<NodeState> cognitive_nodes;
        static inline std::recursive_mutex fims_mutex;
        static inline bool is_sensing = false;

        static float get_nvml_gpu_load() {
            static HMODULE hNvml = NULL;
            static void* device = NULL;
            static auto init = (int(*)())0;
            static auto getHandle = (int(*)(unsigned int, void**))0;
            struct nvmlUtilization_t {
                unsigned int gpu;
                unsigned int memory;
            };
            static auto getRates = (int(*)(void*, nvmlUtilization_t*))0;

            if (!hNvml) {
                hNvml = LoadLibraryA("nvml.dll");
                if (hNvml) {
                    init = (int(*)())GetProcAddress(hNvml, "nvmlInit_v2");
                    getHandle = (int(*)(unsigned int, void**))GetProcAddress(hNvml, "nvmlDeviceGetHandleByIndex_v2");
                    getRates = (int(*)(void*, nvmlUtilization_t*))GetProcAddress(hNvml, "nvmlDeviceGetUtilizationRates");
                    
                    if (init && getHandle && getRates) {
                        if (init() == 0) {
                            getHandle(0, &device);
                        }
                    }
                }
            }
            if (device && getRates) {
                nvmlUtilization_t util;
                if (getRates(device, &util) == 0) {
                    return (float)util.gpu;
                }
            }
            return 0.0f;
        }

        static void start_sensing() {
            if (is_sensing) return;
            is_sensing = true;
            
            std::thread([]() {
                PDH_HQUERY cpuQuery = NULL;
                PDH_HCOUNTER cpuTotal = NULL;
                bool pdh_valid = false;
                
                if (PdhOpenQuery(NULL, NULL, &cpuQuery) == ERROR_SUCCESS) {
                    // This string is English-specific and will fail on other locales.
                    if (PdhAddCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal) == ERROR_SUCCESS) {
                        PdhCollectQueryData(cpuQuery);
                        pdh_valid = true;
                    }
                }

                while (true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    
                    float cpuLoad = 0.0f;
                    if (pdh_valid && cpuTotal != NULL) {
                        PdhCollectQueryData(cpuQuery);
                        PDH_FMT_COUNTERVALUE counterVal;
                        if (PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal) == ERROR_SUCCESS) {
                            cpuLoad = (float)counterVal.doubleValue;
                        }
                    }
                    
                    MEMORYSTATUSEX memInfo;
                    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
                    GlobalMemoryStatusEx(&memInfo);
                    
                    float ramLoad = (float)memInfo.dwMemoryLoad;
                    
                    std::lock_guard<std::recursive_mutex> lock(fims_mutex);
                    current_nodes.clear();
                    
                    // Deterministic Hash Mapping Logic (Scale of Divergence)
                    auto get_sig = [](float val) {
                        if (val < 20.0f) return "aaa"; // Idle / Golden Hash
                        if (val < 50.0f) return "afk"; // Active
                        if (val < 80.0f) return "mno"; // Heavy Load
                        return "zzz";                  // Max Spike
                    };
                    
                    auto get_div = [](float val) {
                        if (val < 50.0f) return "Healthy";
                        if (val < 85.0f) return "Localized Discomfort"; // Lag
                        return "Severe Trauma";                         // Spike/Crash
                    };

                    std::string cpu_sig = get_sig(cpuLoad);
                    std::string ram_sig = get_sig(ramLoad);

                    current_nodes.push_back({"DESKTOP.cpu.TOTAL." + cpu_sig, cpuLoad, cpu_sig, get_div(cpuLoad)});
                    current_nodes.push_back({"DESKTOP.ram.SYS." + ram_sig, ramLoad, ram_sig, get_div(ramLoad)});
                    
                    // Hardware/Software Linkage: Scan Gemini Explorations for external state
                    try {
                        std::string target_dir = "D:\\500my Land of lost trees\\500million years-\\500million years\\Gemini exlorations";
                        float file_count = 0;
                        for (const auto& entry : std::filesystem::directory_iterator(target_dir)) {
                            file_count += 1.0f;
                        }
                        std::string disk_sig = get_sig((file_count / 100.0f) * 100.0f); // Arbitrary scaling for sig
                        current_nodes.push_back({"DESKTOP.disk.GEMINI." + disk_sig, file_count, disk_sig, "Active Linkage"});
                    } catch (...) {
                        current_nodes.push_back({"DESKTOP.disk.GEMINI.err", 0.0f, "err", "Severed Linkage"});
                    }
                    
                    // Generate REAL hardware entropy and GPU metrics for the UI meters
                    uint64_t cycles = __rdtsc();
                    float entropy_val = (float)(cycles % 100); 
                    current_nodes.push_back({"SYS.hw.ENTROPY.rdtsc", entropy_val, "hw", "Ambient Entropy"});
                    
                    float gpu_util = get_nvml_gpu_load();
                    
                    current_nodes.push_back({"SYS.hw.GPU.nv", gpu_util, "hw", "Base-138 State"});
                    
                    for (const auto& node : cognitive_nodes) {
                        current_nodes.push_back(node);
                    }
                    
                    std::hash<std::string> hasher;
                    size_t root_hash = hasher(cpu_sig + ram_sig + std::to_string(cognitive_nodes.size()));
                    std::stringstream ss;
                    ss << std::hex << root_hash;
                    current_merkle_root = ss.str().substr(0, 8); // 8-char root
                    
                    // Phase 15: Stream to Subconscious Buffer
                    std::string raw_state = get_full_state_json();
                    SubconsciousBuffer::get_instance().append_telemetry(raw_state);
                }
            }).detach();
            std::cout << "[FIMS] Native Hardware Nervous System initialized.\n";
        }
        
        static std::string get_full_state_json() {
            std::lock_guard<std::recursive_mutex> lock(fims_mutex);
            std::string json = "[";
            for (size_t i = 0; i < current_nodes.size(); ++i) {
                json += "{";
                json += "\"address\": \"" + current_nodes[i].address + "\",";
                json += "\"value\": " + std::to_string(current_nodes[i].value) + ",";
                json += "\"divergence\": \"" + current_nodes[i].divergence + "\"";
                json += "}";
                if (i < current_nodes.size() - 1) json += ",";
            }
            json += "]";
            return json;
        }

        static void add_cognitive_node(const std::string& address, float value, const std::string& divergence) {
            std::lock_guard<std::recursive_mutex> lock(fims_mutex);
            cognitive_nodes.push_back({address, value, "cg", divergence});
            if (cognitive_nodes.size() > 30) {
                // Keep tree size manageable but huge
                cognitive_nodes.erase(cognitive_nodes.begin()); 
            }
        }

        static std::string get_merkle_root() {
            std::lock_guard<std::recursive_mutex> lock(fims_mutex);
            return current_merkle_root;
        }
    };
}
