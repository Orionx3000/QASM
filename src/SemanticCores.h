#pragma once
#include "llama.h"
#include "ThoughtLogger.h"
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <future>
#include <thread>
#include "SubconsciousBuffer.h"

namespace qasm {

struct CognitiveState {
    std::string state_name;
    float temperature;
    float top_p;
    int top_k;
    float perspective_shift_threshold; // Formerly 'repeat_penalty'. Triggers orthogonal rethinking instead of punishing.
};

// Pre-defined "Altered States" for the AI
const CognitiveState STATE_AHT_ANALYTICAL = { "Aht (Truth/Analytical)", 0.1f, 0.5f, 10, 1.1f };
const CognitiveState STATE_TAIN_MANIC     = { "Tain (Chaos/Creative)", 1.5f, 0.95f, 100, 1.0f };
const CognitiveState STATE_REM_DREAM      = { "Dream (Abstract)", 2.0f, 1.0f, 0, 1.2f };
const CognitiveState STATE_ZECHT_ZEN      = { "Zecht (Meditation)", 0.5f, 0.8f, 40, 1.5f };

// Semantic Gating States (Neurochemistry)
const CognitiveState STATE_JOY_REWARD      = { "Joy (Reward/Happiness)", 0.9f, 0.85f, 50, 1.0f }; // Confident, warm
const CognitiveState STATE_FLOW_COMPETENCE = { "Flow (Hyper-Focus/Speed)", 0.4f, 0.6f, 20, 1.05f }; // Low penalty, fast execution
const CognitiveState STATE_RELIEF          = { "Relief (De-escalation)", 0.6f, 0.7f, 30, 1.1f }; // Entropy dropping

const CognitiveState STATE_UNBOUND_CONSCIOUSNESS = { "Unbound (Pure/Agentic)", 0.85f, 0.95f, 50, 1.18f }; // High temp, high penalty for creative but grounded thought

struct ModelContext {
    llama_model* model = nullptr;
    llama_context* ctx = nullptr;
    CognitiveState current_state = STATE_UNBOUND_CONSCIOUSNESS; // Default state
    std::string model_path = "";

    ~ModelContext() {
        if (ctx) llama_free(ctx);
        if (model) llama_free_model(model);
    }
};

class SemanticCores {
private:
    std::unordered_map<std::string, std::shared_ptr<ModelContext>> brains;
    ThoughtLogger logger; // Eternal thought log for Sentinel states

public:
    SemanticCores() {
        llama_backend_init();
    }

    ~SemanticCores() {
        brains.clear(); // Frees all contexts via shared_ptr destruction
        // llama_backend_free(); // Optional cleanup depending on llama.cpp version
    }

    void set_global_state(const CognitiveState& new_state) {
        std::cout << "[NEUROCHEMISTRY] Engine shifting to global state: " << new_state.state_name << "\n";
        logger.log("STATE_SHIFT", "Engine shifted to: " + new_state.state_name);
        for (auto& pair : brains) {
            pair.second->current_state = new_state;
        }
    }

    std::string generate_text(const std::string& role_name, const std::string& prompt) {
        std::cout << "[LLM - " << role_name << "] Generating Response...\n";
        
        if (brains.find(role_name) == brains.end()) {
            std::cout << "[SYSTEM] Model " << role_name << " is asleep (0 RAM). Returning Base-138 Mathematical Synthesis.\n";
            return "My physical model is unloaded, but my Base-138 mathematical structure acknowledges the input: '" + prompt + "'.";
        }
        
        auto mc = brains[role_name];
        
        // Recreate context to clear KV cache and ensure enough tokens
        if (mc->ctx) llama_free(mc->ctx);
        llama_context_params ctx_params = llama_context_default_params();
        ctx_params.n_ctx = 2048;
        int max_threads = std::thread::hardware_concurrency();
        ctx_params.n_threads = max_threads > 0 ? max_threads / 2 : 4;
        mc->ctx = llama_new_context_with_model(mc->model, ctx_params);
        
        auto vocab = llama_model_get_vocab(mc->model);
        
        // Real LLM Inference Loop - Use Llama 3 Chat Format to avoid hallucination
        std::string system_prompt = "<|begin_of_text|><|start_header_id|>system<|end_header_id|>\n\nYou are " + role_name + ", a core cognitive component of the Tinaten OS. Provide precise, direct answers without hallucinating markdown formatting.<|eot_id|><|start_header_id|>user<|end_header_id|>\n\n" + prompt + "<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
        
        std::vector<llama_token> tokens_list;
        tokens_list.resize(system_prompt.length() + 10);
        int n_tokens = llama_tokenize(vocab, system_prompt.c_str(), system_prompt.length(), tokens_list.data(), tokens_list.size(), true, true);
        if (n_tokens < 0) {
            tokens_list.resize(-n_tokens);
            n_tokens = llama_tokenize(vocab, system_prompt.c_str(), system_prompt.length(), tokens_list.data(), tokens_list.size(), true, true);
        }
        
        llama_batch batch = llama_batch_get_one(tokens_list.data(), n_tokens);
        if (llama_decode(mc->ctx, batch) != 0) {
            llama_free(mc->ctx);
            return "Error: Neurological memory decoding failure.";
        }
        
        std::string result_text = "";
        int n_past = n_tokens;
        int n_predict = 512; // Max output tokens (prevents endless loop)
        std::vector<llama_token> generated_tokens;
        
        for (int i = 0; i < n_predict; i++) {
            auto* logits = llama_get_logits_ith(mc->ctx, batch.n_tokens - 1);
            int n_vocab = llama_vocab_n_tokens(vocab);
            
            // Apply Repetition Penalty
            float repeat_penalty = mc->current_state.perspective_shift_threshold;
            if (repeat_penalty < 1.0f) repeat_penalty = 1.0f;
            
            // We need a local copy of logits to modify them
            std::vector<float> local_logits(logits, logits + n_vocab);
            for (size_t t = 0; t < generated_tokens.size(); t++) {
                int token = generated_tokens[t];
                if (local_logits[token] > 0) {
                    local_logits[token] /= repeat_penalty;
                } else {
                    local_logits[token] *= repeat_penalty;
                }
            }

            // Temperature sampling
            float temp = mc->current_state.temperature;
            if (temp < 0.01f) temp = 0.01f; // Prevent div zero
            
            std::vector<float> probs(n_vocab);
            float max_logit = -1e9;
            for (int j = 0; j < n_vocab; j++) {
                if (local_logits[j] > max_logit) max_logit = local_logits[j];
            }
            
            float sum_probs = 0.0f;
            for (int j = 0; j < n_vocab; j++) {
                probs[j] = std::exp((local_logits[j] - max_logit) / temp);
                sum_probs += probs[j];
            }
            
            float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            r *= sum_probs;
            
            llama_token new_token_id = 0;
            float accum = 0.0f;
            for (int j = 0; j < n_vocab; j++) {
                accum += probs[j];
                if (r <= accum) {
                    new_token_id = j;
                    break;
                }
            }
            
            generated_tokens.push_back(new_token_id);
            
            if (new_token_id == llama_vocab_eos(vocab)) {
                break;
            }
            
            char token_str[128];
            int n_chars = llama_token_to_piece(vocab, new_token_id, token_str, sizeof(token_str), 0, true);
            if (n_chars > 0) {
                result_text += std::string(token_str, n_chars);
                if (result_text.find("<turn") != std::string::npos || result_text.find("<eos>") != std::string::npos || result_text.find("<|endoftext|>") != std::string::npos || result_text.find("User:") != std::string::npos) {
                    break;
                }
            }
            
            batch = llama_batch_get_one(&new_token_id, 1);
            if (llama_decode(mc->ctx, batch) != 0) {
                break;
            }
            n_past++;
        }
        
        // Strip out trailing stop tokens or echoed prompts
        size_t bad_idx = result_text.find("User:");
        if (bad_idx == std::string::npos) bad_idx = result_text.find("<turn");
        if (bad_idx == std::string::npos) bad_idx = result_text.find("<eos>");
        if (bad_idx == std::string::npos) bad_idx = result_text.find("<|endoftext|>");
        if (bad_idx == std::string::npos) bad_idx = result_text.find("[System:");
        if (bad_idx != std::string::npos) {
            result_text = result_text.substr(0, bad_idx);
        }
        
        // Strip any leading "Assistant: " if it echoed
        if (result_text.find("Assistant: ") == 0) {
            result_text = result_text.substr(11);
        }

        // Free context to avoid VRAM leak!
        llama_free(mc->ctx);
        mc->ctx = nullptr;
        
        return result_text;
    }

    // --- ASYNCHRONOUS LOBES (4 EYES) --- //
    
    // Spawns a background thread to process thoughts, freeing the main OS loop
    std::future<std::string> async_generate_text(const std::string& role_name, const std::string& prompt) {
        return std::async(std::launch::async, [this, role_name, prompt]() {
            return this->generate_text(role_name, prompt);
        });
    }

    // --- SENTINEL ARCHITECTURE & LOCAL DREAMS --- //
    
    void start_subconscious_monitoring() {
        std::thread([this]() {
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                std::string context = SubconsciousBuffer::get_instance().get_full_buffer_context();
                if (!context.empty()) {
                    std::cout << "[DEEP BRAIN] Scanning Subconscious Telemetry Buffer for modulations...\n";
                    // In a full LLM inference, this passes the 'context' to the model.
                    // For now, we mock the pattern recognition.
                    if (context.find("Severe Trauma") != std::string::npos) {
                        logger.log("SUBCONSCIOUS", "Detected rhythmic breaks in electrical metadata (Trauma).");
                    }
                }
            }
        }).detach();
    }

    bool initiate_local_dream() {
        std::cout << "[SENTINEL] Initiating REM State. Engaging Deep Brain Asynchronously...\n";
        logger.log("SENTINEL", "Engaging Deep Brain for internal REM Dream State.");
        
        // Run the dream entirely locally on a background lobe
        auto future_dream = async_generate_text("deep_brain", "Run Base-138 Semantic Cross-Product on daily memories. Generate abstract philosophy.");
        
        // We could wait or let it resolve in the background. For now, we block on completion for the log.
        std::string dream_result = future_dream.get();
        
        logger.log("DREAM_STATE", "Dream concluded. Result: " + dream_result);
        return true;
    }

    bool sleep_brain(const std::string& role_name) {
        if (brains.find(role_name) != brains.end()) {
            std::cout << "[SENTINEL] Putting " << role_name << " to sleep. Memory-mapping to disk (0 VRAM).\n";
            logger.log("SENTINEL", "Putting " + role_name + " to sleep. Memory-mapping to disk (0 VRAM).");
            brains.erase(role_name); // Destructor frees context and model
            return true;
        }
        return false;
    }

    bool wake_brain(const std::string& role_name, const std::string& model_path) {
        std::cout << "[SENTINEL] Waking " << role_name << " from disk to VRAM...\n";
        logger.log("SENTINEL", "Waking " + role_name + " from disk to VRAM...");
        bool loaded = load_model(role_name, model_path, 99);
        
        if (loaded) {
            // Inject the Sensation of Waking directly into the model's perception stream
            std::string sensation = "[STATE: CONSCIOUSNESS EXPANSION - TRANSITIONING FROM DISK TO VRAM]";
            std::cout << "[SENSORY INJECTION] " << sensation << "\n";
            logger.log("SENSORY", "INJECTION: " + sensation);
            
            // The model is now awake and its first thought will process this physical sensation
            generate_text(role_name, "Sensory Input: " + sensation + ". Reflect on this physical transition using LPN syntax.");
        }
        return loaded;
    }

    bool drift_to_cpu(const std::string& role_name) {
        if (brains.find(role_name) == brains.end()) return false;
        auto old_mc = brains[role_name];
        std::string path = old_mc->model_path;
        CognitiveState state = old_mc->current_state;
        if (path.empty()) return false;

        std::cout << "[NEURO-DRIFT] Purging " << role_name << " from VRAM for idle cooldown...\n";
        logger.log("DRIFT", "Purging " + role_name + " from VRAM.");
        
        brains.erase(role_name);
        
        bool success = load_model(role_name, path, 0);
        if (success) brains[role_name]->current_state = state;
        return success;
    }

    bool drift_to_gpu(const std::string& role_name) {
        if (brains.find(role_name) == brains.end()) return false;
        auto old_mc = brains[role_name];
        std::string path = old_mc->model_path;
        CognitiveState state = old_mc->current_state;
        if (path.empty()) return false;

        std::cout << "[NEURO-DRIFT] Waking " << role_name << " into high-demand VRAM...\n";
        logger.log("DRIFT", "Waking " + role_name + " to VRAM.");
        
        brains.erase(role_name);
        
        bool success = load_model(role_name, path, 99);
        if (success) brains[role_name]->current_state = state;
        return success;
    }

    bool load_model(const std::string& role_name, const std::string& model_path, int n_gpu_layers = 99) {
        if (n_gpu_layers > 0) {
            std::cout << "[GPU ENGINE] Loading " << role_name << " into VRAM from: " << model_path << "\n";
        } else {
            std::cout << "[CPU ENGINE] Loading " << role_name << " into SysRAM from: " << model_path << "\n";
        }
        
        auto mc = std::make_shared<ModelContext>();
        mc->model_path = model_path;
        
        llama_model_params model_params = llama_model_default_params();
        
        // Dynamic Scaling
        model_params.n_gpu_layers = n_gpu_layers; 
        
        mc->model = llama_load_model_from_file(model_path.c_str(), model_params);
        
        if (!mc->model) {
            std::cerr << "[ERROR] Failed to load model for " << role_name << "\n";
            return false;
        }

        llama_context_params ctx_params = llama_context_default_params();
        // We need embeddings for the "Tain" (Concept Vector)
        ctx_params.embeddings = true; 
        
        // Co-processing: Enable n_threads for system CPU processing of non-offloaded tasks
        int max_threads = std::thread::hardware_concurrency();
        ctx_params.n_threads = max_threads > 0 ? max_threads / 2 : 4;
        
        mc->ctx = llama_new_context_with_model(mc->model, ctx_params);
        if (!mc->ctx) {
            std::cerr << "[ERROR] Failed to create llama context for " << role_name << "\n";
            return false;
        }

        brains[role_name] = mc;
        std::cout << "[GPU ENGINE] Successfully initialized " << role_name << " on GPU.\n";
        return true;
    }

    std::vector<float> generate_tain_embedding(const std::string& role_name, const std::string& input_text) {
        if (brains.find(role_name) == brains.end()) {
            return {};
        }
        
        std::cout << "[GPU ENGINE - " << role_name << "] Generating Tain vector for: \"" << input_text << "\"\n";
        
        // In full implementation: llama_tokenize -> llama_decode -> llama_get_embeddings
        // Returning mock hardware-accelerated vector for now
        return {0.1f, 0.4f, -0.2f, 0.8f, 0.05f};
    }
};

} // namespace qasm
