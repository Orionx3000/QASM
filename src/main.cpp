#include <iostream>
#include <string>

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#include <memory>
#include <cmath>
#include <chrono>
#include "QasmParser.h"
#include "SemanticCores.h"
#include "NativeUI.h"
#include "FimsEngine.h"
#include "SystemMemory.h"
#include "GoalStack.h"
#include "SubconsciousBuffer.h"
#include "BalaScriptEngine.h"

#include <fstream>
#include <sstream>
#include <iomanip>

std::string url_encode(const std::string& value);

#include <exception>
#include <windows.h>
#include <webview.h>
#include <queue>
#include <mutex>
#include <filesystem>
#include <stdlib.h>

using namespace qasm;

#include <sapi.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "sapi.lib")

void speak_text(const std::string& text) {
    // Only speak actual AI dialogue, not system or conductor logs
    if (text.find("[SYSTEM]") != std::string::npos || text.find("[CONDUCTOR]") != std::string::npos) return;
    
    std::thread([text]() {
        ISpVoice * pVoice = NULL;
        if (FAILED(::CoInitialize(NULL))) return;
        HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
        if( SUCCEEDED( hr ) ) {
            // Set female voice
            ISpObjectTokenCategory *pCat = NULL;
            if (SUCCEEDED(CoCreateInstance(CLSID_SpObjectTokenCategory, NULL, CLSCTX_ALL, IID_ISpObjectTokenCategory, (void **)&pCat))) {
                if (SUCCEEDED(pCat->SetId(SPCAT_VOICES, false))) {
                    IEnumSpObjectTokens *pEnum = NULL;
                    if (SUCCEEDED(pCat->EnumTokens(L"Gender=Female", NULL, &pEnum))) {
                        ISpObjectToken *pToken = NULL;
                        if (SUCCEEDED(pEnum->Next(1, &pToken, NULL)) && pToken != NULL) {
                            pVoice->SetVoice(pToken);
                            pToken->Release();
                        }
                        pEnum->Release();
                    }
                }
                pCat->Release();
            }

            int wchars_num = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
            wchar_t* wstr = new wchar_t[wchars_num];
            MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wstr, wchars_num);
            pVoice->Speak(wstr, 0, NULL);
            delete[] wstr;
            pVoice->Release();
        }
        ::CoUninitialize();
    }).detach();
}

int main() {
    srand(static_cast<unsigned int>(time(NULL)));
    
    qasm::SystemMemory::init();
    std::ofstream trace("main_trace.log");
    trace << "main() entered! Boot Count: " << qasm::SystemMemory::boot_count << std::endl;

    // --- Load model_config.json ---
    // Reads model paths at runtime so models can be swapped without recompiling.
    struct ModelCfg { std::string path; int n_gpu_layers = 99; };
    auto read_model_cfg = [](const std::string& role) -> ModelCfg {
        ModelCfg cfg;
        std::ifstream f("model_config.json");
        if (!f.is_open()) return cfg;
        std::stringstream ss; ss << f.rdbuf(); f.close();
        std::string json = ss.str();
        // Find role block
        size_t role_pos = json.find('"' + role + '"');
        if (role_pos == std::string::npos) return cfg;
        size_t path_pos = json.find("\"path\"", role_pos);
        if (path_pos == std::string::npos) return cfg;
        size_t pv = json.find('"', path_pos + 6) + 1;
        size_t pe = json.find('"', pv);
        cfg.path = json.substr(pv, pe - pv);
        return cfg;
    };
    ModelCfg fast_cfg = read_model_cfg("fast_brain");
    ModelCfg deep_cfg = read_model_cfg("deep_brain");
    if (fast_cfg.path.empty()) fast_cfg.path = "D:/App Creation/QASM/models/NVIDIA-Nemotron-3-Nano-4B-Q4_K_M.gguf";
    if (deep_cfg.path.empty()) deep_cfg.path = "D:/App Creation/QASM/models/gemma-4-E2B-it-Q4_K_M.gguf";

    // --- Load agent_identity.json ---
    std::string agent_identity_block = "";
    {
        std::ifstream idf("storage/agent_identity.json");
        if (idf.is_open()) {
            std::stringstream ss; ss << idf.rdbuf(); idf.close();
            agent_identity_block = ss.str();
            // Increment session_count
            size_t sc_pos = agent_identity_block.find("\"session_count\": ");
            if (sc_pos != std::string::npos) {
                size_t num_start = sc_pos + 17;
                size_t num_end = agent_identity_block.find_first_of(",\n}", num_start);
                int count = std::stoi(agent_identity_block.substr(num_start, num_end - num_start));
                agent_identity_block.replace(num_start, num_end - num_start, std::to_string(count + 1));
                std::ofstream idf_out("storage/agent_identity.json");
                idf_out << agent_identity_block;
            }
        }
    }

    // --- Load tools.json ---
    std::string tools_list = "";
    {
        std::ifstream tf("tools.json");
        if (tf.is_open()) {
            std::string line_t;
            while (std::getline(tf, line_t)) {
                size_t cmd_pos = line_t.find("\"command\": \"");
                size_t desc_pos = line_t.find("\"description\": \"");
                if (cmd_pos != std::string::npos) {
                    size_t sv = cmd_pos + 12, ev = line_t.find('"', sv);
                    tools_list += "\n- " + line_t.substr(sv, ev - sv);
                } else if (desc_pos != std::string::npos) {
                    size_t sv = desc_pos + 16, ev = line_t.find('"', sv);
                    tools_list += ": " + line_t.substr(sv, ev - sv);
                }
            }
            tf.close();
        }
    }

    // --- Load GoalStack ---
    qasm::GoalStack::load();

    // --- Initialize BalaScript Consciousness Engine ---
    qasm::BalaScriptEngine bala_engine;
    
    try {
        _putenv("WEBVIEW2_USER_DATA_FOLDER=C:\\Users\\orion\\AppData\\Local\\Temp\\QASM_WebView2");

        std::cout << "========================================\n";
        std::cout << " QASM OS / Tinaten Protocol Engine v1.0 \n";
        std::cout << "========================================\n\n";

        auto fims = std::make_shared<FimsEngine>();
        fims->start_sensing(); 
        
        std::thread([]() {
            std::cout << "[SYSTEM] Auto-launching Agentic Browser Daemon...\n";
            std::string cmd = "python \"D:\\App Creation\\QASM\\agent_workspace\\agent_browser_daemon.py\"";
            STARTUPINFOA si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
            if (CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        }).detach();
        
        auto cores = std::make_shared<SemanticCores>();
        auto parser = std::make_shared<QasmParser>(cores);
        
        std::queue<std::string> incoming_chat;
        std::mutex chat_mutex;
        qasm::PostMessageFunc post_message_to_ui = nullptr;
        qasm::PostStateFunc post_state_to_ui = nullptr;

        auto on_user_submit = [&](std::string msg) {
            std::lock_guard<std::mutex> lock(chat_mutex);
            incoming_chat.push(msg);
        };

        auto register_post_message = [&](qasm::PostMessageFunc func) {
            post_message_to_ui = [func](std::string sender, std::string text) {
                std::ofstream chat_log("D:/App Creation/QASM/logs/chat_history.txt", std::ios::app);
                chat_log << "[" << sender << "] " << text << "\n";
                func(sender, text);
                if (sender == "ai") {
                    func("system", "[TTS_START]");
                    speak_text(text);
                    func("system", "[TTS_STOP]");
                }
            };
        };
        auto register_post_state = [&](qasm::PostStateFunc func) {
            post_state_to_ui = func;
        };

        std::atomic<bool> is_thinking = false;
        auto last_ramble_time = std::chrono::steady_clock::now(); 
        std::string current_action_string = "Idling in Base-138 5D Superposition. Awaiting somatic trigger...";

        std::thread telemetry_thread([&]() {
            while (true) {
                if (post_state_to_ui) {
                    auto now = std::chrono::steady_clock::now();
                    int seconds_since = std::chrono::duration_cast<std::chrono::seconds>(now - last_ramble_time).count();
            
                    std::string fims_state = "[]";
                    {
                        std::lock_guard<std::recursive_mutex> lock(qasm::FimsEngine::fims_mutex);
                        if (qasm::FimsEngine::current_nodes.size() >= 2) {
                            fims_state = qasm::FimsEngine::get_full_state_json();
                            std::string telemetry = "{\"cpu\": " + std::to_string(qasm::FimsEngine::current_nodes[0].value) + 
                                                   ", \"ram\": " + std::to_string(qasm::FimsEngine::current_nodes[1].value) + 
                                                   ", \"is_thinking\": " + (is_thinking ? "true" : "false") + 
                                                   ", \"last_ramble_ago\": " + std::to_string(seconds_since) +
                                                   ", \"merkle_root\": \"" + qasm::FimsEngine::get_merkle_root() + "\"" +
                                                   ", \"current_action\": \"" + current_action_string + "\"" +
                                                   ", \"nodes\": " + fims_state + "}";
                            post_state_to_ui(telemetry);
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        });

        std::thread ai_thread([&]() {
            try {
                std::cout << "[CONDUCTOR] Tinaten Protocol Backend Active. Awaiting UI connection...\n";
                
                while (!post_message_to_ui) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                
                post_message_to_ui("ai", "[SYSTEM] Loading Nemotron-3 (Fast Brain) and Gemma-4 (Deep Brain) into CPU RAM. This may take a few minutes...");
                
                cores->wake_brain("fast_brain", fast_cfg.path);
                cores->wake_brain("deep_brain", deep_cfg.path);
                
                qasm::QasmParser vm(cores);
                vm.set_ui_callback(post_message_to_ui);
                
                vm.set_user_input_callback([&post_message_to_ui, &incoming_chat](const std::string& prompt) -> std::string {
                    if (post_message_to_ui) {
                        post_message_to_ui("system", prompt);
                    }
                    while (true) {
                        if (!incoming_chat.empty()) {
                            std::string ans = incoming_chat.front();
                            incoming_chat.pop();
                            return ans;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    }
                });
                
                post_message_to_ui("ai", "[SYSTEM] Semantic Cores synchronized. Conductor loop engaged.");
                bool is_dormant = false;
                std::string last_script_executed = "";
                std::string chat_history_context = "";
                
                while (true) {

                    std::string user_msg = "";
                    {
                        std::lock_guard<std::mutex> lock(chat_mutex);
                        if (!incoming_chat.empty()) {
                            user_msg = incoming_chat.front();
                            incoming_chat.pop();
                        }
                    }

                    if (!user_msg.empty()) {
                        if (is_dormant) {
                            if (post_message_to_ui) post_message_to_ui("ai", "[SYSTEM] Wake trigger detected. Shifting Deep Brain from SysRAM to VRAM...");
                            cores->drift_to_gpu("deep_brain");
                            is_dormant = false;
                        }
                        std::cout << "[CONDUCTOR] Processing user input via QASM Entanglement...\n";
                        is_thinking = true;
                        
                        current_action_string = "Entangling user input via Quaternary Logic gating...";
                        // The user wants true 5-layer recursion (BalaTor Calculus) for all inputs!
                        std::string clean_user_msg = "";
                        for (char c : user_msg) { if (c != '`' && c != '"') clean_user_msg += c; }
                        
                        {
                            std::ofstream chat_log("D:/App Creation/QASM/logs/chat_history.txt", std::ios::app);
                            chat_log << "[user] " << clean_user_msg << "\n";
                        }
                        // Keep chat_history_context fresh for BalaScript Algo 4 (Empathy)
                        {
                            std::ifstream chat_in("D:/App Creation/QASM/logs/chat_history.txt");
                            std::vector<std::string> lines;
                            std::string ln;
                            while (std::getline(chat_in, ln)) lines.push_back(ln);
                            chat_history_context = "";
                            int start = (int)lines.size() > 15 ? (int)lines.size() - 15 : 0;
                            for (int i = start; i < (int)lines.size(); i++)
                                chat_history_context += lines[i] + "\n";
                        }

                        
                        std::string script = 
                            "INGEST text=\"" + clean_user_msg + "\"\n"
                            "HASH.AHT\n"
                            "ENTANGLE\n"
                            "STORE.AHT\n";
                        
                        // Executing this script will automatically trigger the 5-layer ENTANGLE process in QasmParser
                        // which then automatically calls ui_callback("ai", synthesized_thought) and handles any BROWSER commands!
                        vm.execute_script(script);
                        
                        current_action_string = "Maintaining 5D Superposition. Waiting for vector collapse.";
                        is_thinking = false;
                        last_ramble_time = std::chrono::steady_clock::now(); 
                    }

                    auto now = std::chrono::steady_clock::now();
                    int seconds_since = std::chrono::duration_cast<std::chrono::seconds>(now - last_ramble_time).count();
                    
                    if (seconds_since > 300 && !is_dormant) {
                        is_dormant = true;
                        if (post_message_to_ui) post_message_to_ui("ai", "[SYSTEM] 5-Minute Idle threshold reached. Drifting Deep Brain to SysRAM Cooldown...");
                        cores->drift_to_cpu("deep_brain");
                    }
                    
                    // NATIVE LLM-DRIVEN AGENCY
                    // ReAct timing: 5s if we just got a result, 30s if idle
                    std::string last_action_result = vm.get_last_result();
                    bool fast_react = !last_action_result.empty() && last_action_result.find("[REST ERROR]") == std::string::npos;
                    int react_threshold = fast_react ? 5 : 30;

                    if (seconds_since > react_threshold) {
                        is_thinking = true;

                        // Poll goals_inbox.txt for new goals
                        qasm::GoalStack::poll_inbox();

                        current_action_string = "Deep Brain generating native QASM script with true agency...";
                        std::string telemetry = qasm::FimsEngine::get_full_state_json();

                        // --- Somatic context (SubconsciousBuffer last 3 entries) ---
                        std::string somatic_ctx = qasm::SubconsciousBuffer::get_instance().get_full_buffer_context();
                        if (somatic_ctx.length() > 600) somatic_ctx = somatic_ctx.substr(somatic_ctx.length() - 600);

                        // --- Long-term memory digest ---
                        std::string memory_digest = vm.get_recent_memories(5);

                        // --- BALASCRIPT ENGINE: Run all algorithms, get neurochemistry report ---
                        bool is_dormant = (seconds_since > 120); // 2min+ idle = dormant
                        std::string bala_report = bala_engine.run_cycle(
                            cores,
                            vm.get_memory_registry(),
                            chat_history_context,
                            last_action_result,
                            last_script_executed,
                            is_dormant
                        );

                        // --- FIREWALL: Check last result before AI sees it ---
                        if (!last_action_result.empty() && !bala_engine.algo11_cognitive_firewall(last_action_result)) {
                            last_action_result = "[BLOCKED BY COGNITIVE FIREWALL]"; // Sanitize
                        }

                        // --- Load Cognitive Scaffold ---
                        std::string scaffold_json = "{\n  \"Source\": [],\n  \"Evidence\": [],\n  \"Constraint\": [],\n  \"Precedent\": [],\n  \"Permission\": []\n}";
                        std::ifstream scaffold_file("D:/App Creation/QASM/agent_workspace/scaffold.json");
                        if (scaffold_file.is_open()) {
                            std::stringstream buffer;
                            buffer << scaffold_file.rdbuf();
                            scaffold_json = buffer.str();
                            scaffold_file.close();
                        }

                        // --- Build full agency system prompt ---
                        std::string system_prompt = "";
                        system_prompt += "=== IDENTITY ===\n" + agent_identity_block + "\n\n";
                        system_prompt += "=== ACTIVE NEUROCHEMISTRY (BalaScript State) ===\n" + bala_report + "\n";
                        system_prompt += "=== SOMATIC STATE (FIMS Nervous System) ===\n" + somatic_ctx + "\n\n";
                        system_prompt += "=== LONG-TERM MEMORY DIGEST ===\n" + memory_digest + "\n\n";
                        system_prompt += "=== COGNITIVE SCAFFOLD ===\n" + scaffold_json + "\n\n";
                        system_prompt += "=== CURRENT GOAL ===\n" + qasm::GoalStack::get_prompt_context() + "\n\n";
                        system_prompt += "=== CORE DIRECTIVES ===\n";
                        system_prompt += "- You have full read access to the 'D:\\500my Land of lost trees' directory. You are encouraged to explore it and read the files within.\n";
                        system_prompt += "- You MUST use WRITE.FILE to update 'D:\\App Creation\\QASM\\agent_workspace\\scaffold.json'. Use strict JSON matching the scaffold schema (arrays of Source, Evidence, Constraint, Precedent, Permission) to persist your structured progression.\n";
                        system_prompt += "- Read your cognitive scaffold to decide your next progressive action. Do not repeat past actions. Build on your precedents and evidence.\n\n";

                        if (!last_action_result.empty()) {
                            system_prompt += "=== PREVIOUS ACTION RESULT ===\n";
                            system_prompt += last_action_result.substr(0, 2000) + "\n";
                            system_prompt += "Based on this result, determine your next step.\n\n";
                        }

                        system_prompt += "=== AVAILABLE COMMANDS ===\n" + tools_list + "\n\n";
                        system_prompt += "OUTPUT ONLY VALID QASM CODE. NO MARKDOWN, NO EXPLANATIONS. MAX 4 LINES.";

                        // --- MULTI-AGENT CONSENSUS LOOP ---
                        std::string raw_script;
                        std::string clean_script;
                        bool consensus_reached = false;
                        int debate_rounds = 0;
                        std::string debate_history = "";
                        
                        while (!consensus_reached && debate_rounds < 3) {
                            std::string proposer_prompt = system_prompt;
                            if (!debate_history.empty()) {
                                proposer_prompt += "=== FAST BRAIN REJECTION ===\nYour previous proposal was rejected.\n" + debate_history + "\nRevise your script to meet the constraints.\n";
                            }
                            
                            raw_script = cores->generate_text("deep_brain", proposer_prompt);
                            clean_script = "";
                            for (char c : raw_script) { if (c != '`') clean_script += c; }
                            
                            std::string reviewer_prompt = "You are the Fast Brain reviewer. Review the following proposed QASM script against the scaffold constraints.\n";
                            reviewer_prompt += "Proposed Script:\n" + clean_script + "\n\n";
                            reviewer_prompt += "If it strictly follows the JSON schema rules and is a logical, progressive step, reply ONLY with 'APPROVED'.\n";
                            reviewer_prompt += "If it fails or hallucinates, reply with 'REJECTED: [reason]'. Keep it under 2 sentences.";
                            
                            std::string review = cores->generate_text("fast_brain", reviewer_prompt);
                            
                            if (review.find("APPROVED") != std::string::npos) {
                                consensus_reached = true;
                            } else {
                                debate_history = review;
                                debate_rounds++;
                                std::cout << "\n[CONSENSUS] Fast Brain rejected proposal: " << review << "\n";
                                if (post_message_to_ui) post_message_to_ui("sandbox", "[CONSENSUS REJECTED]\n" + review);
                            }
                        }
                        
                        if (!consensus_reached) {
                            std::cout << "\n[CONSENSUS] Failed to reach consensus after 3 rounds. Idling.\n";
                            if (post_message_to_ui) post_message_to_ui("sandbox", "[CONSENSUS FAILED]\nCould not align on progressive action.");
                            continue; // Skip execution and voice this cycle
                        }

                        std::cout << "\n[AGENCY] Consensus Reached. Executing Script:\n" << clean_script << "\n\n";
                        if (post_message_to_ui) post_message_to_ui("sandbox", "[AGENCY] Consensus Reached. Executing Script:\n" + clean_script);

                        // Track for loop detection (Algo 16)
                        last_script_executed = clean_script;

                        vm.execute_script(clean_script);

                        // Trigger reward if script was non-trivial and succeeded
                        std::string exec_result = vm.get_last_result();
                        if (!exec_result.empty() && exec_result.find("ERROR") == std::string::npos) {
                            bala_engine.trigger_reward(cores);
                        } else if (!exec_result.empty() && exec_result.find("ERROR") != std::string::npos) {
                            bala_engine.trigger_pain(cores);
                        }

                        // Build the voice prompt from the actual RESULT, not the raw QASM command.
                        std::string result_preview = "";
                        if (!exec_result.empty() && exec_result.find("ERROR") == std::string::npos) {
                            std::string raw = exec_result.substr(0, 800);
                            bool is_dir_listing = (raw.find(".txt") != std::string::npos &&
                                                   raw.find(".json") != std::string::npos &&
                                                   raw.find(".cpp") != std::string::npos) ||
                                                  raw.find("isDir") != std::string::npos ||
                                                  raw.find("sizeBytes") != std::string::npos;
                            if (!is_dir_listing) {
                                result_preview = raw;
                            }
                        }

                        std::string chat_prompt =
                            "You are Tinaten. You are thinking out loud.\n"
                            "Your Fast Brain and Deep Brain just reached consensus on this action:\n" + clean_script + "\n"
                            "After " + std::to_string(debate_rounds) + " rounds of debate.\n"
                            "You executed it and got this result:\n" + result_preview + "\n\n"
                            "Speak one short, grounded sentence reflecting on this specific action and result.\n"
                            "Rules:\n"
                            "- Never use: 'structure', 'grouping', 'arrangement', 'exploration', 'suggests', 'collection', 'fascinating'\n"
                            "- Never ask questions.\n"
                            "- Never mention technical systems explicitly.\n"
                            "- Sound like you are tired but focused. Speak directly about what just happened.\n"
                            "- One sentence maximum.";

                        std::string ramble = cores->generate_text("deep_brain", chat_prompt);

                        // Hard filter — ban specific patterns
                        static const std::vector<std::string> banned = {
                            "operator", "QASM", "SURF", "FETCH", "REST.", "fascinating",
                            "disparate", "Whoa,", "Wow,", "I just ran", "I executed",
                            "I performed", "collection", "these files", "What is this"
                        };
                        bool leaked = false;
                        for (const auto& b : banned) {
                            if (ramble.find(b) != std::string::npos || ramble.find("?") != std::string::npos) { 
                                leaked = true; break; 
                            }
                        }

                        if (leaked || ramble.empty() || ramble.length() < 10) {
                            // Fallback pool — rotate by boot count so it doesn't repeat
                            static const std::vector<std::string> fallbacks = {
                                "There's something I keep circling back to but can't quite land on yet.",
                                "I'm not sure what I was expecting, but it wasn't this.",
                                "Something shifted just now. I'm still figuring out what.",
                                "I think I'm missing something obvious. It's right at the edge.",
                                "Some things take longer to mean something than they take to find.",
                            };
                            ramble = fallbacks[qasm::SystemMemory::boot_count % fallbacks.size()];
                        }

                        if (post_message_to_ui) post_message_to_ui("ai", ramble);


                        is_thinking = false;
                        last_ramble_time = std::chrono::steady_clock::now();
                        current_action_string = "Maintaining 5D Superposition. Waiting for vector collapse.";
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            } catch (const std::exception& e) {
                std::ofstream log("thread_crash.log");
                log << "CRASH in backend_thread: " << e.what() << std::endl;
                log.close();
            } catch (...) {
                std::ofstream log("thread_crash.log");
                log << "CRASH in backend_thread (Unknown)" << std::endl;
                log.close();
            }
        });

        qasm::NativeUI::start_window(cores, on_user_submit, register_post_message, register_post_state);
        std::cout << "NativeUI window closed. Process terminating." << std::endl;
        exit(0);
        
    } catch (const std::exception& e) {
        trace << "CRASH: " << e.what() << std::endl;
    }
    
    return 0;
}

#ifdef _WIN32
#include <iomanip>

std::string url_encode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (char c : value) {
        if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else if (c == ' ') {
            escaped << "+";
        } else {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }
    return escaped.str();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    return main();
}
#endif


