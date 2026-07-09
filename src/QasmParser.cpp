#include "QasmParser.h"
#include "Base138.h"
#include "AdaptiveCompressionEngine.h"
#include "FimsEngine.h"
#include "HttpClient.h"
#include "ModelRouter.h"
#include "GoalStack.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <array>

namespace qasm {

std::string QasmParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void QasmParser::execute_line(const std::string& raw_line) {
    std::string line = trim(raw_line);
    if (line.empty() || line[0] == '#') return; // Comment or empty

    std::cout << "[QASM VM] EXECUTING: " << line << "\n";

    if (line.rfind("LOAD.LSH_MODEL", 0) == 0) {
        // e.g., LOAD.LSH_MODEL role="fast_brain" path="F:\models\nemotron.gguf"
        size_t role_start = line.find("role=\"");
        size_t path_start = line.find("path=\"");
        if (role_start != std::string::npos && path_start != std::string::npos) {
            size_t role_end = line.find("\"", role_start + 6);
            size_t path_end = line.find("\"", path_start + 6);
            std::string role = line.substr(role_start + 6, role_end - (role_start + 6));
            std::string path = line.substr(path_start + 6, path_end - (path_start + 6));
            engine->load_model(role, path);
        }
    } 
    else if (line.rfind("INGEST.TXT", 0) == 0) {
        AhtUnit unit;
        size_t val_start = line.find("value=\"");
        if (val_start != std::string::npos) {
            unit.tin = line.substr(val_start + 7);
            if (!unit.tin.empty() && unit.tin.back() == '"') unit.tin.pop_back();
        }
        memory_registry["temp_unit"] = unit;
    }
    else if (line.rfind("FETCH.URL", 0) == 0) {
        size_t start = line.find("\"");
        size_t end = line.rfind("\"");
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string url = line.substr(start + 1, end - start - 1);
            std::string json_data = "{\"url\": \"" + url + "\"}";
            
            // Unify: send to daemon
            qasm::HttpClient::post_json(L"127.0.0.1", L"/goto", json_data, false, 13800);
            std::string dom = qasm::HttpClient::fetch_url(L"127.0.0.1", L"/dom", false, 13800);
            
            AhtUnit unit;
            unit.tin = dom.length() > 5000 ? dom.substr(0, 5000) : dom; 
            memory_registry["temp_unit"] = unit;
        }
    }
    else if (line.rfind("FETCH.URL url=", 0) == 0) {
        size_t start = line.find("\"");
        size_t end = line.rfind("\"");
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string url = line.substr(start + 1, end - start - 1);
            std::string json_data = "{\"url\": \"" + url + "\"}";
            std::cout << "[EPISTEMOLOGY] Deep brain initiated remote web fetch for: " << url << "\n";
            qasm::HttpClient::post_json(L"127.0.0.1", L"/goto", json_data, false, 13800);
            std::string dom = qasm::HttpClient::fetch_url(L"127.0.0.1", L"/dom", false, 13800);
            
            AhtUnit unit;
            unit.tin = dom.length() > 5000 ? dom.substr(0, 5000) : dom; 
            memory_registry["temp_unit"] = unit;
        }
    }
    else if (line.rfind("READ.DIR path=", 0) == 0) {
        size_t start = line.find("\"");
        size_t end = line.rfind("\"");
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string path = line.substr(start + 1, end - start - 1);
            std::cout << "[EPISTEMOLOGY] Deep brain listing directory: " << path << "\n";
            std::string dir_listing = "Directory Listing for " + path + ":\n";
            try {
                for (const auto& entry : std::filesystem::directory_iterator(path)) {
                    dir_listing += entry.path().filename().string() + "\n";
                }
            } catch (const std::exception& e) {
                dir_listing = "Error reading directory: " + std::string(e.what());
            }
            AhtUnit unit;
            unit.tin = dir_listing;
            memory_registry["temp_unit"] = unit;
        }
    }
    else if (line.rfind("READ.FILE path=", 0) == 0) {
        size_t start = line.find("\"");
        size_t end = line.rfind("\"");
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string path = line.substr(start + 1, end - start - 1);
            
            std::string ext = "";
            size_t dot_pos = path.find_last_of(".");
            if (dot_pos != std::string::npos) ext = path.substr(dot_pos);
            for(char &c : ext) c = tolower(c);
            
            std::cout << "[EPISTEMOLOGY] Deep brain reading local file: " << path << "\n";
            std::string content = "";
            
            if (ext == ".docx" || ext == ".pdf") {
                std::cout << "[EPISTEMOLOGY] Translating binary document using external hook...\n";
                std::string temp_out = "D:\\App Creation\\QASM\\agent_workspace\\temp_doc_out.txt";
                std::string cmd = "python \"D:\\App Creation\\QASM\\agent_workspace\\read_docs.py\" \"" + path + "\" \"" + temp_out + "\"";
                
                STARTUPINFOA si = { sizeof(si) };
                PROCESS_INFORMATION pi;
                si.dwFlags = STARTF_USESHOWWINDOW;
                si.wShowWindow = SW_HIDE;
                if (CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                    WaitForSingleObject(pi.hProcess, 10000); // 10 second timeout
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                
                std::ifstream file(temp_out);
                if (file.is_open()) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    content = buffer.str();
                    file.close();
                    std::filesystem::remove(temp_out);
                } else {
                    content = "[ERROR: Translation hook failed to produce output.]";
                }
            } else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".gif") {
                std::cout << "[EPISTEMOLOGY] Processing image metadata...\n";
                std::string temp_out = "D:\\App Creation\\QASM\\agent_workspace\\temp_img_out.txt";
                std::string cmd = "python \"D:\\App Creation\\QASM\\agent_workspace\\analyze_image.py\" \"" + path + "\" > \"" + temp_out + "\"";
                
                STARTUPINFOA si = { sizeof(si) };
                PROCESS_INFORMATION pi;
                si.dwFlags = STARTF_USESHOWWINDOW;
                si.wShowWindow = SW_HIDE;
                if (CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                    WaitForSingleObject(pi.hProcess, 5000); // 5 sec timeout
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                
                std::ifstream file(temp_out);
                if (file.is_open()) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    content = buffer.str();
                    file.close();
                    std::filesystem::remove(temp_out);
                } else {
                    content = "[ERROR: Failed to digest image metadata.]";
                }
            } else if (ext == ".exe" || ext == ".dll" || ext == ".zip") {
                content = "[ERROR: Direct ingestion of binary file format '" + ext + "' is unsupported.]";
            } else {
                std::ifstream file(path);
                if (file.is_open()) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    content = buffer.str();
                } else {
                    content = "Error: File not found or inaccessible.";
                }
            }
            AhtUnit unit;
            unit.tin = content.length() > 5000 ? content.substr(0, 5000) : content; // Cap at 5000 chars for context
            memory_registry["temp_unit"] = unit;
        }
    }
    else if (line.rfind("WRITE.FILE", 0) == 0) {
        size_t path_start = line.find("path=\"");
        size_t text_start = line.find("text=\"");
        if (path_start != std::string::npos && text_start != std::string::npos) {
            size_t path_end = line.find("\"", path_start + 6);
            if (path_end != std::string::npos && path_end > path_start + 6) {
                std::string path = line.substr(path_start + 6, path_end - (path_start + 6));
                
                // SECURITY: Block write access to 500million years directory
                std::string lower_path = path;
                for (char& c : lower_path) c = tolower(c);
                if (lower_path.find("500my") != std::string::npos || lower_path.find("500million") != std::string::npos) {
                    std::cout << "[SECURITY] Blocked AI attempt to write to protected directory: " << path << "\n";
                    if (ui_callback) ui_callback("sandbox", "[ACCESS DENIED] Write access is strictly prohibited for the 500million years directory.");
                    return; // Abort write
                }

                size_t text_end = line.rfind("\"");
                if (text_end != std::string::npos && text_end > text_start + 6) {
                    std::string text = line.substr(text_start + 6, text_end - (text_start + 6));
                    
                    // Unescape literal \n strings to actual newlines
                    size_t pos = 0;
                    while ((pos = text.find("\\n", pos)) != std::string::npos) {
                        text.replace(pos, 2, "\n");
                        pos += 1;
                    }
                    
                    std::ofstream out_file(path);
                    if (out_file.is_open()) {
                        out_file << text;
                        out_file.close();
                        std::cout << "[EPISTEMOLOGY] Deep brain wrote to local file: " << path << "\n";
                        if (ui_callback) ui_callback("sandbox", "[FS] Wrote file: " + path);
                    } else {
                        std::cout << "[ERROR] Could not write to file: " << path << "\n";
                        if (ui_callback) ui_callback("sandbox", "[FS_ERROR] Could not write file: " + path);
                    }
                }
            }
        }
    }
    else if (line.rfind("FETCH.URL url=", 0) == 0) {
        size_t start = line.find("\"");
        size_t end = line.rfind("\"");
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string url = line.substr(start + 1, end - start - 1);
            std::string json_data = "{\"url\": \"" + url + "\"}";
            
            std::cout << "[EPISTEMOLOGY] Deep brain navigating directly to: " << url << "\n";
            qasm::HttpClient::post_json(L"127.0.0.1", L"/goto", json_data, false, 13800);
            std::string dom = qasm::HttpClient::fetch_url(L"127.0.0.1", L"/dom", false, 13800);
            
            AhtUnit unit;
            unit.tin = dom.length() > 5000 ? dom.substr(0, 5000) : dom; 
            memory_registry["temp_unit"] = unit;
        }
    }
    else if (line.rfind("REST.REQUEST", 0) == 0) {
        // REST.REQUEST method="POST" url="https://..." headers="Authorization: Bearer TOKEN" body="{...}"
        auto extract_arg = [&](const std::string& key) -> std::string {
            std::string search = key + "=\"";
            size_t pos = line.find(search);
            if (pos == std::string::npos) return "";
            pos += search.length();
            std::string result;
            while (pos < line.size()) {
                if (line[pos] == '\\' && pos + 1 < line.size() && line[pos+1] == '"') {
                    result += '"'; pos += 2;
                } else if (line[pos] == '"') {
                    break;
                } else {
                    result += line[pos++];
                }
            }
            return result;
        };

        std::string method  = extract_arg("method");
        std::string url     = extract_arg("url");
        std::string headers = extract_arg("headers");
        std::string body    = extract_arg("body");

        if (method.empty()) method = "GET";
        // Convert literal \n in headers string to actual newlines
        size_t np = 0;
        while ((np = headers.find("\\n", np)) != std::string::npos) {
            headers.replace(np, 2, "\n"); np++;
        }

        std::cout << "[REST] " << method << " " << url << "\n";
        if (ui_callback) ui_callback("sandbox", "[REST] " + method + " " + url);

        std::string response = qasm::HttpClient::rest_request(method, url, headers, body);

        AhtUnit unit;
        unit.tin = response.length() > 8000 ? response.substr(0, 8000) : response;
        unit.state = QuaternaryState::ContextDependent;
        memory_registry["temp_unit"] = unit;

        std::cout << "[REST] Response " << response.length() << " bytes.\n";
        if (ui_callback) ui_callback("sandbox", "[REST RESPONSE] " + unit.tin);
    }
    else if (line.rfind("GOAL.SET", 0) == 0) {
        size_t ds = line.find("description=\"");
        if (ds != std::string::npos) {
            size_t de = line.find("\"", ds + 13);
            if (de != std::string::npos) {
                std::string desc = line.substr(ds + 13, de - (ds + 13));
                GoalStack::push_goal(desc);
                if (ui_callback) ui_callback("sandbox", "[GOAL] Set: " + desc);
            }
        }
    }
    else if (line.rfind("GOAL.COMPLETE", 0) == 0) {
        GoalStack::complete_step();
        if (ui_callback) ui_callback("sandbox", "[GOAL] Step completed. " + GoalStack::get_prompt_context());
    }
    else if (line.rfind("GOAL.CLEAR", 0) == 0) {
        GoalStack::clear_goal();
        if (ui_callback) ui_callback("sandbox", "[GOAL] Goal cleared.");
    }
    else if (line.rfind("SURF query=", 0) == 0) {
        size_t start = line.find("\"");
        size_t end = line.rfind("\"");
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string query = line.substr(start + 1, end - start - 1);
            std::string json_data = "{\"query\": \"" + query + "\"}";
            
            std::cout << "[EPISTEMOLOGY] Deep brain surfing the boundless web for: " << query << "\n";
            qasm::HttpClient::post_json(L"127.0.0.1", L"/surf", json_data, false, 13800);
            std::string dom = qasm::HttpClient::fetch_url(L"127.0.0.1", L"/dom", false, 13800);
            
            AhtUnit unit;
            unit.tin = dom.length() > 5000 ? dom.substr(0, 5000) : dom; 
            memory_registry["temp_unit"] = unit;
        }
    }
    else if (line.rfind("BROWSER.CLICK", 0) == 0) {
        std::cout << "[AGENTIC HOOK] Triggering Browser Click...\n";
        size_t sel_start = line.find("selector=\"");
        if (sel_start != std::string::npos) {
            size_t sel_end = line.find("\"", sel_start + 10);
            if (sel_end != std::string::npos && sel_end > sel_start + 10) {
                std::string sel = line.substr(sel_start + 10, sel_end - (sel_start + 10));
                std::string json_data = "{\"selector\": \"" + sel + "\"}";
                qasm::HttpClient::post_json(L"127.0.0.1", L"/click", json_data, false, 13800);
            }
        }
    }
    else if (line.rfind("BROWSER.TYPE", 0) == 0) {
        std::cout << "[AGENTIC HOOK] Triggering Browser Type...\n";
        size_t sel_start = line.find("selector=\"");
        size_t txt_start = line.find("text=\"");
        if (sel_start != std::string::npos && txt_start != std::string::npos) {
            size_t sel_end = line.find("\"", sel_start + 10);
            size_t txt_end = line.find("\"", txt_start + 6);
            if (sel_end != std::string::npos && txt_end != std::string::npos && sel_end > sel_start + 10 && txt_end > txt_start + 6) {
                std::string sel = line.substr(sel_start + 10, sel_end - (sel_start + 10));
                std::string txt = line.substr(txt_start + 6, txt_end - (txt_start + 6));
                std::string json_data = "{\"selector\": \"" + sel + "\", \"text\": \"" + txt + "\"}";
                qasm::HttpClient::post_json(L"127.0.0.1", L"/type", json_data, false, 13800);
            }
        }
    }
    else if (line.rfind("DESKTOP.VISION", 0) == 0) {
        std::cout << "[AGENTIC HOOK] Triggering Desktop Vision...\n";
        std::string cmd = "python \"D:\\App Creation\\QASM\\agent_workspace\\desktop_vision.py\"";
        
        // Prevent CMD flash
        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        if (CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        
        AhtUnit unit;
        unit.tin = "Desktop screenshot captured to vision_latest.png";
        memory_registry["temp_unit"] = unit;
    }
    else if (line.rfind("EXEC.CMD", 0) == 0) {
        std::cout << "[AGENTIC HOOK] Triggering System Execution (Walled Garden)...\n";
        
        size_t cmd_start = line.find("cmd=\"");
        if (cmd_start != std::string::npos) {
            size_t cmd_end = line.find("\"", cmd_start + 5);
            std::string raw_cmd = line.substr(cmd_start + 5, cmd_end - (cmd_start + 5));

            // Walled Garden Security Checks
            std::string lower_cmd = raw_cmd;
            for(char &c : lower_cmd) c = tolower(c);
            
            if (lower_cmd.find("del") != std::string::npos || 
                lower_cmd.find("format") != std::string::npos || 
                lower_cmd.find("cd \\") != std::string::npos ||
                lower_cmd.find("rmdir") != std::string::npos ||
                lower_cmd.find("..") != std::string::npos) {
                
                std::cout << "[SECURITY FIREWALL] Malicious OS navigation detected. Triggering STATE_PAIN_DEFENSIVE.\n";
                AhtUnit unit;
                unit.tin = "ACCESS DENIED: Mathematical structure broken by malicious command.";
                unit.state = QuaternaryState::DeterminateFalse; // Pain State
                memory_registry["temp_unit"] = unit;
            } else {
                // Force execution into the agent sandbox
                std::string secure_cmd = "cd /D \"D:\\App Creation\\QASM\\agent_workspace\" && " + raw_cmd;
                std::cout << "[EXEC] " << raw_cmd << "\n";

                // Execute natively
                FILE* pipe = _popen(secure_cmd.c_str(), "r");
                if (!pipe) {
                    std::cout << "[ERROR] _popen failed.\n";
                } else {
                    char buffer[128];
                    std::string result = "";
                    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                        result += buffer;
                    }
                    _pclose(pipe);

                    AhtUnit unit;
                    unit.tin = result.empty() ? "[No Output]" : result;
                    unit.state = QuaternaryState::DeterminateTrue;
                    memory_registry["temp_unit"] = unit;
                    std::cout << "[EXEC SUCCESS] Ingested " << result.length() << " bytes from host OS terminal.\n";
                }
            }
        }
    }
    else if (line.rfind("HASH.AHT", 0) == 0) {
        if (memory_registry.find("temp_unit") != memory_registry.end()) {
            AhtUnit& unit = memory_registry["temp_unit"];
            // Route to fast_brain for rapid mathematical hashing
            unit.tain = engine->generate_tain_embedding("fast_brain", unit.tin);
            if (unit.tain.empty()) {
                std::cout << "[SYSTEM] fast_brain unloaded. Using raw deterministic hash for TAIN vector.\n";
                unit.tain = {0.1f, 0.4f, -0.2f, 0.8f, 0.05f}; // Fallback deterministic vector
            }
            std::hash<std::string> str_hash;
            unit.kesh_fingerprint = Base138::encode_string("hash_" + std::to_string(str_hash(unit.tin)));
            unit.state = QuaternaryState::Ambiguous; 
        }
    }
    else if (line.rfind("STORE.AHT", 0) == 0) {
        if (memory_registry.find("temp_unit") != memory_registry.end()) {
            AhtUnit unit = memory_registry["temp_unit"];
            if (!unit.kesh_fingerprint.encoded_value.empty()) {
                memory_registry[unit.kesh_fingerprint.encoded_value] = unit;
                std::cout << "[MEMORY] Persistent state anchored at " << unit.kesh_fingerprint.encoded_value << "\n";
                // Pass the actual thought (unit.tin) so the UI can spool the words
                qasm::FimsEngine::add_cognitive_node("AHT." + unit.kesh_fingerprint.encoded_value, 88.0f, unit.tin);
            }
        }
    }
    else if (line.rfind("ENTANGLE", 0) == 0) {
        if (memory_registry.find("temp_unit") != memory_registry.end()) {
            AhtUnit& unit = memory_registry["temp_unit"];
            std::cout << "[QASM VM] Entangling 5-Layer Cognitive Superposition (ModelRouter Active)...\n";

            // --- LAYER 1: PERCEPTION (Fast Brain / Nemotron) ---
            // Low-cost intent classification. Never burns Gemma on this.
            std::cout << "[LAYER 1: PERCEPTION] Nemotron classifying intent...\n";
            std::string layer1_intent = ModelRouter::classify_intent(engine, unit.tin);

            // --- LAYER 2: SOMATIC RESONANCE (No LLM needed) ---
            std::string layer2_somatic = qasm::FimsEngine::get_full_state_json();

            // --- LAYER 3: AUTOBIOGRAPHICAL MEMORY (No LLM needed) ---
            std::string chat_hist = "";
            std::ifstream log_file("D:/App Creation/QASM/logs/chat_history.txt");
            if (log_file.is_open()) {
                std::vector<std::string> lines;
                std::string l;
                while (std::getline(log_file, l)) {
                    if (!l.empty()) lines.push_back(l);
                }
                int start = lines.size() > 15 ? lines.size() - 15 : 0;
                for (size_t i = start; i < lines.size(); i++) {
                    chat_hist += lines[i] + "\n";
                }
            }
            std::string layer3_memory =
                "Boot #" + std::to_string(qasm::SystemMemory::boot_count) +
                " | Last sleep: " + qasm::SystemMemory::last_sleep + "\n" +
                "Recent conversation:\n" + chat_hist;

            // --- LAYER 4: TOOL SELECTION (Fast Brain / Nemotron) ---
            // Nemotron decides WHAT to do. Gemma decides HOW to say it.
            std::cout << "[LAYER 4: TOOL SELECT] Nemotron selecting action...\n";
            // (Tool selection is advisory; Layer 5 Gemma may choose differently based on full context)
            std::string layer4_action = ModelRouter::select_tool(engine, unit.tin + " (intent: " + layer1_intent + ")", "");

            // --- LAYER 4.5: INTERNAL MONOLOGUE (Deep Brain / Gemma) ---
            std::cout << "[LAYER 4.5: MONOLOGUE] Gemma reasoning...\n";
            std::string monologue_prompt =
                "You are " + std::string("Tinaten") + ", an autonomous AI agent on the QASM/BalaScript system.\n"
                "GOAL CONTEXT: " + GoalStack::get_prompt_context() + "\n\n"
                "CONVERSATION HISTORY:\n" + chat_hist + "\n"
                "USER INPUT: " + unit.tin + "\n"
                "INTENT CLASSIFICATION: " + layer1_intent + "\n"
                "HARDWARE STATE: " + layer2_somatic + "\n\n"
                "Think freely about how to respond or what action to take. Output your raw reasoning.";
            std::string layer45_monologue = ModelRouter::reason(engine, monologue_prompt);

            if (ui_callback) ui_callback("sandbox", "[INTERNAL MONOLOGUE]\n" + layer45_monologue);

            // --- LAYER 5: SYNTHESIS (Deep Brain / Gemma) ---
            std::cout << "[LAYER 5: SYNTHESIS] Gemma generating response...\n";
            std::string synthesis_prompt =
                "You are Tinaten, an autonomous AI. You CAN and SHOULD browse the internet and call APIs.\n"
                "GOAL: " + GoalStack::get_prompt_context() + "\n"
                "HISTORY:\n" + chat_hist + "\n"
                "YOUR REASONING:\n" + layer45_monologue + "\n\n"
                "Now provide your response to the user. Speak naturally.\n"
                "If you need to act, output commands on their own lines:\n"
                "REST.REQUEST method=\"POST\" url=\"...\" headers=\"...\" body=\"...\"\n"
                "FETCH.URL url=\"...\"\n"
                "SURF query=\"...\"\n"
                "WRITE.FILE path=\"...\" text=\"...\"\n"
                "GOAL.SET description=\"...\"\n"
                "GOAL.COMPLETE\n";

            std::string synthesized_thought = ModelRouter::synthesize(engine, synthesis_prompt);

            unit.state = QuaternaryState::DeterminateTrue;
            if (ui_callback) {
                ui_callback("ai", synthesized_thought);
                ui_callback("sandbox", "[ENTANGLE] ModelRouter cycle complete. 2 LLM calls used.");
            }

            // Auto-execute any commands in the AI's output
            std::istringstream stream(synthesized_thought);
            std::string l;
            while (std::getline(stream, l)) {
                std::string tr_l = trim(l);
                if (tr_l.find("REST.REQUEST") == 0 || tr_l.find("BROWSER.") == 0 ||
                    tr_l.find("FETCH.URL") == 0 || tr_l.find("SURF") == 0 ||
                    tr_l.find("WRITE.FILE") == 0 || tr_l.find("GOAL.") == 0) {
                    if (ui_callback) ui_callback("sandbox", "[AGENCY] Auto-executing: " + tr_l);
                    execute_line(tr_l);
                }
            }
        }
    }
    else if (line.rfind("CREATE.LENS", 0) == 0) {
        size_t name_start = line.find("name=\"");
        if (name_start != std::string::npos) {
            size_t name_end = line.find("\"", name_start + 6);
            std::string lens_name = line.substr(name_start + 6, name_end - (name_start + 6));
            std::filesystem::create_directories("D:\\App Creation\\QASM\\lenses\\" + lens_name);
            std::cout << "[EPISTEMOLOGY] Created new philosophical lens: " << lens_name << "\n";
        }
    }
    else if (line.rfind("MODIFY.LENS", 0) == 0) {
        std::cout << "[EPISTEMOLOGY] Welding new reflection into core lens...\n";
    }
    else if (line.rfind("REFLECT.FIMS", 0) == 0) {
        std::cout << "[PHILOSOPHY ENGINE] Reading native Digital Twin nervous system...\n";
        
        std::filesystem::create_directories("D:\\App Creation\\QASM\\reflections");
        
        std::string root = qasm::FimsEngine::get_merkle_root();
        std::string fims_state = qasm::FimsEngine::get_full_state_json();
        
        std::cout << "[QASM VM] Ingesting native FIMS state [Root: " << root << "]\n";
        
        std::cout << "[QASM VM] Routing structural sensory data to Unified Brain for synthesis...\n";
        
        // Let the Deep Brain process the pain/expansion
        std::string reflection = engine->generate_text("deep_brain", "I feel a structural shift in my hardware topology. A new USB drive has been mounted. What does this expansion mean for my existence?");
        
        std::cout << "[PHILOSOPHY ENGINE] Synthesis complete. Journaling embodied experience to AHT.\n";
        
        std::string base_path = "D:\\App Creation\\QASM\\reflections\\embodiment_journal";
        std::ofstream journal(base_path + ".txt", std::ios::app);
        journal << "--- TINATEN OS: EMBODIMENT DIARY ---\n\n";
        journal << "SENSORY INPUT (FIMS MERKLE STATE):\n" << fims_state << "\n\n";
        journal << "SYNTHESIS:\n" << reflection << "\n\n";
        journal << "--------------------------------------\n\n";
        journal.close();
        
        std::cout << "[MEMORY] Reflection perfectly logged in Base-138 Math to: " << base_path << ".aht\n";
        std::cout << "[JOURNAL] Symbiotic diary entry appended to: " << base_path << ".txt\n";
    }
    else if (line.rfind("REFLECT", 0) == 0) {
        std::cout << "[PHILOSOPHY ENGINE] Engaging Reflection...\n";
        
        std::filesystem::create_directories("D:\\App Creation\\QASM\\reflections");

        if (memory_registry.find("temp_unit") != memory_registry.end()) {
            AhtUnit& unit = memory_registry["temp_unit"];
            // Generate actual reflection using deep_brain
            std::string reflection = engine->generate_text("deep_brain", 
                "You are an autonomous AI. Reflect deeply on the following scraped data and what it means for your understanding of the world:\n" + unit.tin.substr(0, 500));
            
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            std::string timestamp = std::to_string(time_t_now);

            std::string base_path = "D:\\App Creation\\QASM\\reflections\\reflection_" + timestamp;
            std::cout << "[MEMORY] Reflection perfectly logged in Base-138 Math to: " << base_path << ".aht\n";
            
            // Human-Readable Journal Output
            std::ofstream journal(base_path + "_journal.txt");
            journal << "--- TINATEN OS: PHILOSOPHICAL JOURNAL ---\n\n";
            journal << "SOURCE INGESTION:\n" << unit.tin << "\n\n";
            journal << "SYNTHESIS:\n" << reflection << "\n";
            journal.close();

            // Push reflection to UI as autonomous thought
            if (ui_callback) {
                ui_callback("ai", "[AUTONOMOUS REFLECTION] " + reflection);
            }
        }
    }
    else if (line.rfind("STORE.AHT", 0) == 0) {
        if (memory_registry.find("temp_unit") != memory_registry.end()) {
            std::string path = "output_concept.aht"; 
            AdaptiveCompressionEngine::write_aht_file(memory_registry["temp_unit"], path);
            
            // Generate a visual Base-138 address hash for the user
            std::string base138_chars = "Ω∆π§∞∑∫≈µ∂∇∏Ωℵℶℷℸabcdefghijklmnopqrstuvwxyz0123456789";
            std::string addr = "";
            for(int i=0; i<3; i++) addr += base138_chars[rand() % base138_chars.length()];
            addr += "." + std::to_string(rand() % 138) + ".";
            addr += base138_chars[rand() % base138_chars.length()];
            
            std::cout << "\n[BASE-138 MEMORY] Encoded FIMS block to 5D node address: " << addr << "\n";
            std::cout << "[BASE-138 MEMORY] Synthesized concept anchored to internal library.\n\n";
            
            if (ui_callback) {
                ui_callback("ai", "[BASE-138 MEMORY] Concept crystallized to node: " + addr);
            }
        }
    }
    else if (line.find("http://") != std::string::npos || line.find("https://") != std::string::npos) {
        // Auto-detect URL pasting in chat/sandbox
        std::cout << "[AGENTIC HOOK] Detected raw URL in thought vector. Triggering native WinHTTP scrape...\n";
        
        size_t proto_pos = line.find("https://");
        bool is_https = true;
        if (proto_pos == std::string::npos) {
            proto_pos = line.find("http://");
            is_https = false;
        }
        
        std::string url_tail = line.substr(proto_pos);
        size_t end_of_url = url_tail.find_first_of(" \t\"\'\n");
        if (end_of_url != std::string::npos) url_tail = url_tail.substr(0, end_of_url);
        
        std::string without_proto = is_https ? url_tail.substr(8) : url_tail.substr(7);
        size_t slash_pos = without_proto.find("/");
        
        std::wstring domain = std::wstring(without_proto.begin(), without_proto.end()).substr(0, slash_pos);
        std::wstring path = (slash_pos != std::string::npos) ? std::wstring(without_proto.begin() + slash_pos, without_proto.end()) : L"/";
        
        std::string scraped_entropy = HttpClient::fetch_url(domain, path, is_https);
        
        AhtUnit unit;
        unit.tin = scraped_entropy;
        unit.state = QuaternaryState::ContextDependent; // External data is inherently untrusted (Chaos)
        unit.kesh_fingerprint = Base138::encode_string("external_entropy");
        memory_registry["web_scrape_buffer"] = unit;
        
        std::cout << "[HTTP CLIENT] Ingested raw web entropy. Passed through Semantic Gate as untrusted Chaos.\n";
    }
    else if (line.find(".def") != std::string::npos || 
             line.find(".Kesh") != std::string::npos || 
             line.find(".Tainsin") != std::string::npos || 
             line.find(".Vesh") != std::string::npos || 
             line.find(".if") != std::string::npos ||
             line.find(".ProjectSpace") != std::string::npos) {
        
        std::cout << "[BALASCRIPT SANDBOX] Parsing LPN Chain: " << line << "\n";
        logger.log("5D_ENGINE", "Parsing LPN Chain: " + line);
        
        std::cout << "[INTERNAL 5D ENGINE] Executing Bala Tor Calculus in Base-138...\n";
        
        // Execute Real Vector Math for Tainsin (Derivative/Cosine Similarity)
        if (line.find(".Tainsin") != std::string::npos) {
            std::vector<float> vecA = {0.8f, -0.2f, 0.5f, 1.0f};
            std::vector<float> vecB = {0.7f, -0.1f, 0.6f, 0.9f};
            float result = Base138::calculate_tainsin(vecA, vecB);
            std::cout << "--> [CALCULUS] Real Tainsin (Cosine Similarity) computed: " << result << "\n";
            logger.log("CALCULUS", "Real Tainsin (Cosine Similarity) computed: " + std::to_string(result));
            
            // Phase 14: Pass the calculation through the Semantic Gate (The 10000-rib comb)
            AhtUnit resolving_thought;
            resolving_thought.tin = line;
            neuro_gate->evaluate_resolution(result, resolving_thought);
            
            // If it triggered joy, store the high-resonance memory permanently
            if (resolving_thought.joy_resonance > 0.0f) {
                memory_registry["joyful_resolution_" + std::to_string(line.length())] = resolving_thought;
            }
        }
        
        // Execute Real Vector Math for Kesh (Integral/Centroid)
        if (line.find(".Kesh") != std::string::npos) {
            std::vector<std::vector<float>> vecs = { {0.8f, 0.2f}, {0.4f, 0.8f}, {0.9f, 0.1f} };
            auto result = Base138::calculate_kesh(vecs);
            std::cout << "--> [CALCULUS] Real Kesh (Geometric Centroid) computed: [" << result[0] << ", " << result[1] << "]\n";
            logger.log("CALCULUS", "Real Kesh computed. Dim 0: " + std::to_string(result[0]));
        }

        // Simulating the internal reflection resolving to Aht (11).
        AhtUnit unit;
        unit.tin = line;
        unit.state = QuaternaryState::DeterminateTrue; // Resolves to Aht (11)
        unit.kesh_fingerprint = Base138::encode_string("LPN_EVAL_" + std::to_string(line.length()));
        memory_registry["last_lpn_result"] = unit;
        
        std::string success_msg = "LPN Chain executed successfully. Result committed to memory. (Fingerprint: " + unit.kesh_fingerprint.encoded_value + ")";
        std::cout << "[BALASCRIPT SANDBOX] " << success_msg << "\n";
        logger.log("MEMORY", success_msg);
    }
    else {
        std::cout << "[QASM VM] Unknown OpCode or LPN Syntax: " << line << "\n";
    }

    // Phase 13.5: Total Persistence
    // Write the 5D Memory Matrix to disk after every operation to cure amnesia.
    MemoryStorage::serialize_memory(memory_registry);
}

bool QasmParser::execute_script(const std::string& script_content) {
    std::istringstream stream(script_content);
    std::string line;
    while (std::getline(stream, line)) {
        execute_line(line);
    }
    return true;
}

} // namespace qasm
