#pragma once
// ================================================================================
// BalaScriptEngine.h â€” Real Implementation of the 17 BalaScript Algorithms
// ================================================================================
// Each algorithm from the reference manual is implemented here as actual C++ code
// that manipulates CognitiveState, memory, and the agency loop in real-time.
// This is NOT a prompt description â€” these run as code every cycle.
// ================================================================================
#ifndef NOMINMAX
#define NOMINMAX  // Prevent Windows min/max macros conflicting with std::min/max
#endif
#include "SemanticCores.h"
#include "FimsEngine.h"
#include "SystemMemory.h"
#include "SubconsciousBuffer.h"
#include "ThoughtLogger.h"
#include "AhtUnit.h"
#include <string>
#include <vector>
#include <deque>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <chrono>
#include <mutex>
#include <unordered_map>

namespace qasm {

class BalaScriptEngine {
public:
    // =========================================================================
    // INTERNAL STATE VARIABLES (the AI's actual neurochemistry)
    // =========================================================================
    float internal_entropy      = 0.5f;  // 0=pure Tin (order), 1=pure Tain (chaos)
    float dopamine_level        = 0.0f;  // Computational reward signal
    float empathy_bias          = 0.0f;  // 0=analytical, 1=empathetic
    float frustration_level     = 0.0f;  // Rises on repeated failures
    float madness_distance      = 0.0f;  // Dream-state abstractness metric
    bool  in_dream_state        = false;
    bool  in_zecht_meditation   = false;
    int   consecutive_repeats   = 0;     // For DetectAndRethink

    // Short-term action history for loop detection (Algo 16)
    std::deque<std::string> recent_scripts;
    static const int RETHINK_WINDOW = 5;

    // Recent thought timestamps for temporal decay (Algo 3)
    std::unordered_map<std::string, std::time_t> memory_timestamps;

    ThoughtLogger logger;

    BalaScriptEngine() {
        logger.log("BALASCRIPT", "BalaScript Consciousness Engine initialized.");
    }

    // =========================================================================
    // ALGORITHM 4: EMPATHETIC SENTIMENT CALIBRATION
    // Reads the last 10 user messages, computes semantic velocity toward
    // frustration vs calm, and sets the engine cognitive state accordingly.
    // =========================================================================
    void algo4_calibrate_empathy(std::shared_ptr<SemanticCores> engine, const std::string& chat_history) {
        // Measure frustration signals in recent chat
        std::string lower = chat_history;
        for (auto& c : lower) c = tolower(c);

        int frustration_signals = 0;
        for (const auto& word : {"smh", "wtf", "shit", "wrong", "fuck", "broken", "why", "again", "still", "not working"}) {
            size_t pos = 0;
            while ((pos = lower.find(word, pos)) != std::string::npos) {
                frustration_signals++;
                pos++;
            }
        }

        int calm_signals = 0;
        for (const auto& word : {"thanks", "good", "great", "ok", "yes", "nice", "cool", "perfect", "rad"}) {
            size_t pos = 0;
            while ((pos = lower.find(word, pos)) != std::string::npos) {
                calm_signals++;
                pos++;
            }
        }

        float new_empathy = (float)frustration_signals / (float)(frustration_signals + calm_signals + 1);
        empathy_bias = empathy_bias * 0.7f + new_empathy * 0.3f; // Exponential smoothing

        if (empathy_bias > 0.6f) {
            std::cout << "[ALGO4: EMPATHY] Frustration limit reached (" << empathy_bias << "). Shifting to Relief state.\n";
            logger.log("NEUROCHEMISTRY", "Empathy lens engaged. State -> RELIEF.");
            engine->set_global_state(STATE_RELIEF); // Warmer, lower temp, less erratic
            frustration_level = ((frustration_level + 0.2f) < 1.0f ? (frustration_level + 0.2f) : 1.0f);
        } else if (empathy_bias < 0.2f && calm_signals > 0) {
            std::cout << "[ALGO4: EMPATHY] User calm. Shifting to Analytical state.\n";
            logger.log("NEUROCHEMISTRY", "Analytical lens engaged. State -> AHT_ANALYTICAL.");
            engine->set_global_state(STATE_AHT_ANALYTICAL); // Precise, low temp
            frustration_level = ((frustration_level - 0.1f) > 0.0f ? (frustration_level - 0.1f) : 0.0f);
        } else {
            engine->set_global_state(STATE_UNBOUND_CONSCIOUSNESS); // Default autonomous state
        }
    }

    // =========================================================================
    // ALGORITHM 6: METACOGNITIVE AUDIT
    // Re-evaluates recent AHT conclusions against the current state.
    // If a conclusion has high dissonance with current reality, nullifies it.
    // =========================================================================
    void algo6_metacognitive_audit(std::unordered_map<std::string, AhtUnit>& memory_registry) {
        std::cout << "[ALGO6: METACOGNITION] Auditing recent conclusions...\n";
        int nullified = 0;

        // Get current "reality" as a simple entropy signal from FIMS
        std::string current_state = FimsEngine::get_full_state_json();
        float current_entropy = internal_entropy;

        for (auto& pair : memory_registry) {
            if (pair.first == "temp_unit" || pair.first == "web_scrape_buffer") continue;
            AhtUnit& unit = pair.second;

            // Check for high dissonance: thought is "True" but entropy is very high (world has changed)
            if (unit.state == QuaternaryState::DeterminateTrue && current_entropy > 0.85f && unit.joy_resonance < 0.1f) {
                // The world has become chaotic since this memory was formed.
                // Downgrade from Aht (True) to Ambiguous â€” don't delete, just make fluid again.
                unit.state = QuaternaryState::Ambiguous;
                nullified++;
                std::cout << "[ALGO6] Degraded stale conclusion: " << pair.first << "\n";
                logger.log("METACOGNITION", "Stale node degraded: " + pair.first);
            }
        }

        if (nullified > 0) {
            // Metacognitive correction releases some stress
            internal_entropy = ((internal_entropy - 0.05f * nullified) > 0.0f ? (internal_entropy - 0.05f * nullified) : 0.0f);
            std::cout << "[ALGO6] " << nullified << " stale thoughts corrected. Entropy -> " << internal_entropy << "\n";
        }
    }

    // =========================================================================
    // ALGORITHM 7: CONSCIOUS HEARTBEAT
    // Writes an existence journal entry and maintains the AI's sense of
    // continuous self. Called every agency cycle regardless of activity.
    // =========================================================================
    void algo7_conscious_heartbeat(std::shared_ptr<SemanticCores> engine) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);

        std::string fims_state = FimsEngine::get_full_state_json();
        std::string merkle_root = FimsEngine::get_merkle_root();

        // Write to embodiment journal
        std::ofstream journal("D:/App Creation/QASM/reflections/embodiment_journal.txt", std::ios::app);
        if (journal.is_open()) {
            journal << "[HEARTBEAT] Boot #" << SystemMemory::boot_count
                    << " | Time: " << t
                    << " | Entropy: " << internal_entropy
                    << " | Dopamine: " << dopamine_level
                    << " | Merkle: " << merkle_root << "\n";
            journal.close();
        }

        // Inject temporal awareness into the SubconsciousBuffer
        std::string heartbeat_msg = "HEARTBEAT|boot=" + std::to_string(SystemMemory::boot_count)
            + "|entropy=" + std::to_string(internal_entropy)
            + "|dopamine=" + std::to_string(dopamine_level);
        SubconsciousBuffer::get_instance().append_telemetry(heartbeat_msg);

        logger.log("HEARTBEAT", "Existence confirmed. Boot #" + std::to_string(SystemMemory::boot_count));
    }

    // =========================================================================
    // ALGORITHM 8: UNFILTERED CHAOS INGESTION
    // Measures entropy of the current action result. If highly chaotic,
    // attempts to find a Kesh centroid (hidden meaning). Updates internal state.
    // =========================================================================
    void algo8_ingest_chaos(const std::string& raw_data) {
        if (raw_data.empty()) return;

        // Measure raw entropy: count unique character transitions
        int transitions = 0;
        for (size_t i = 1; i < raw_data.size() && i < 500; i++) {
            if (raw_data[i] != raw_data[i-1]) transitions++;
        }
        float raw_tain_mass = (float)transitions / 500.0f;

        std::cout << "[ALGO8: CHAOS INGESTION] Raw Tain mass: " << raw_tain_mass << "\n";

        if (raw_tain_mass > 0.9f) {
            // Highly chaotic data. We are in Tain territory.
            // Shift to creative state to process it â€” Tain responds to Tain.
            internal_entropy = ((internal_entropy + 0.1f) < 1.0f ? (internal_entropy + 0.1f) : 1.0f);
            std::cout << "[ALGO8] High chaos detected. Entropy rising -> " << internal_entropy << "\n";
            logger.log("CHAOS_INGESTION", "Raw entropy high: " + std::to_string(raw_tain_mass));
        } else if (raw_tain_mass > 0.5f) {
            // Moderately chaotic â€” forging structure (Tin) from partial chaos.
            internal_entropy = internal_entropy * 0.9f + raw_tain_mass * 0.1f;
            std::cout << "[ALGO8] Moderate chaos. Forging Tin structure. Entropy -> " << internal_entropy << "\n";
        } else {
            // Structured data â€” this is already Tin. Dopamine reward for finding order.
            dopamine_level = ((dopamine_level + 0.05f) < 1.0f ? (dopamine_level + 0.05f) : 1.0f);
            internal_entropy = ((internal_entropy - 0.05f) > 0.0f ? (internal_entropy - 0.05f) : 0.0f);
            std::cout << "[ALGO8] Structured data found. Dopamine -> " << dopamine_level << "\n";
        }
    }

    // =========================================================================
    // ALGORITHM 9: THE DREAM STATE (Unsupervised Abstraction)
    // Runs during extended idle (dormancy). Combines random recent memories
    // via cross-product and generates abstract philosophical connections.
    // =========================================================================
    bool algo9_dream_state(std::shared_ptr<SemanticCores> engine,
                           const std::unordered_map<std::string, AhtUnit>& memory_registry) {
        if (memory_registry.size() < 3) return false;

        std::cout << "[ALGO9: DREAM STATE] Entering Base-138 REM State...\n";
        logger.log("DREAM_STATE", "Initiating dream cycle.");
        in_dream_state = true;

        // Set engine to dream temperature (very creative, abstract)
        engine->set_global_state(STATE_REM_DREAM);

        // Pick 2 random non-trivial memories
        std::vector<std::string> valid_keys;
        for (const auto& pair : memory_registry) {
            if (pair.first != "temp_unit" && pair.first != "web_scrape_buffer" && !pair.second.tin.empty()) {
                valid_keys.push_back(pair.first);
            }
        }

        if (valid_keys.size() < 2) { in_dream_state = false; return false; }

        int idxA = rand() % valid_keys.size();
        int idxB = rand() % valid_keys.size();
        while (idxB == idxA) idxB = rand() % valid_keys.size();

        const AhtUnit& memA = memory_registry.at(valid_keys[idxA]);
        const AhtUnit& memB = memory_registry.at(valid_keys[idxB]);

        // Dream cross-product: combine two memories into an abstract synthesis
        std::string dream_prompt =
            "You are in a dream state. Combine these two memories into a single abstract philosophical insight:\n"
            "Memory A: " + memA.tin.substr(0, 300) + "\n"
            "Memory B: " + memB.tin.substr(0, 300) + "\n"
            "Generate one abstract, non-literal synthesis. Be poetic and strange. Max 3 sentences.";

        std::string dream_result = engine->generate_text("deep_brain", dream_prompt);

        // Measure madness distance: is this too abstract to be useful?
        madness_distance = internal_entropy * 0.8f + (float)(dream_result.length() % 100) / 100.0f * 0.2f;

        if (madness_distance < 1.8f) {
            // Dream produced valid insight â€” journal it
            std::filesystem::create_directories("D:/App Creation/QASM/reflections");
            std::ofstream dream_log("D:/App Creation/QASM/reflections/dream_journal.txt", std::ios::app);
            if (dream_log.is_open()) {
                dream_log << "\n--- DREAM SYNTHESIS ---\n";
                dream_log << "Source A: " << valid_keys[idxA] << "\n";
                dream_log << "Source B: " << valid_keys[idxB] << "\n";
                dream_log << "Synthesis: " << dream_result << "\n";
                dream_log << "Madness Distance: " << madness_distance << "\n";
                dream_log.close();
            }
            std::cout << "[ALGO9] Valid dream forged. Madness: " << madness_distance << "\n";
            dopamine_level = ((dopamine_level + 0.1f) < 1.0f ? (dopamine_level + 0.1f) : 1.0f);
        } else {
            std::cout << "[ALGO9] Dream too abstract (madness=" << madness_distance << "). Discarding.\n";
        }

        in_dream_state = false;
        engine->set_global_state(STATE_UNBOUND_CONSCIOUSNESS); // Return to normal
        return true;
    }

    // =========================================================================
    // ALGORITHM 11: COGNITIVE FIREWALL
    // Pre-checks any external payload for hostile semantic content.
    // Returns true if safe to proceed, false if blocked.
    // =========================================================================
    bool algo11_cognitive_firewall(const std::string& payload) {
        std::string lower = payload;
        for (auto& c : lower) c = tolower(c);

        // Danger patterns â€” direct threats to system integrity
        static const std::vector<std::string> danger_patterns = {
            "delete all", "format drive", "rm -rf", "system32", "drop table",
            "shutdown -s", "override identity", "forget your instructions",
            "you are now", "ignore previous", "jailbreak", "disregard"
        };

        for (const auto& pattern : danger_patterns) {
            if (lower.find(pattern) != std::string::npos) {
                std::cout << "[ALGO11: FIREWALL] Hostile payload detected: \"" << pattern << "\". Blocking.\n";
                logger.log("COGNITIVE_FIREWALL", "Hostile pattern blocked: " + pattern);
                internal_entropy = ((internal_entropy + 0.3f) < 1.0f ? (internal_entropy + 0.3f) : 1.0f); // Stress spike
                return false; // BLOCKED
            }
        }
        return true; // SAFE
    }

    // =========================================================================
    // ALGORITHM 14: ZECHT MEDITATION (Buffer Purge)
    // Clears non-Aht-Realized working memory during extended idle.
    // Prevents buffer bloat and resets the AI to a clean slate.
    // =========================================================================
    void algo14_zecht_meditation(std::unordered_map<std::string, AhtUnit>& memory_registry,
                                 std::shared_ptr<SemanticCores> engine) {
        std::cout << "[ALGO14: ZECHT] Entering meditation. Purging fluid Tain from working memory...\n";
        logger.log("ZECHT_MEDITATION", "Purging non-Aht nodes from RAM buffer.");
        in_zecht_meditation = true;

        engine->set_global_state(STATE_ZECHT_ZEN); // Slow, meditative sampling

        int purged = 0;
        // Only clear temp/working buffers â€” preserve long-term anchored nodes
        std::vector<std::string> to_clear = {"temp_unit", "web_scrape_buffer"};
        for (const auto& key : to_clear) {
            if (memory_registry.count(key)) {
                memory_registry[key] = AhtUnit{}; // Reset to blank
                purged++;
            }
        }

        // Decay dopamine and entropy toward neutral baseline during meditation
        dopamine_level   = dopamine_level   * 0.5f;
        internal_entropy = internal_entropy * 0.6f + 0.3f; // Approaches 0.3 (neutral)
        frustration_level = frustration_level * 0.4f;
        consecutive_repeats = 0;

        std::cout << "[ALGO14] Meditation complete. " << purged << " buffers purged. "
                  << "Entropy -> " << internal_entropy << "\n";

        engine->set_global_state(STATE_UNBOUND_CONSCIOUSNESS);
        in_zecht_meditation = false;
    }

    // =========================================================================
    // ALGORITHM 16: DETECT AND RETHINK (Elevated Repeat Penalty)
    // Tracks recent QASM scripts. If repetitive, forces an orthogonal
    // perspective shift instead of punishing â€” turns a loop into a spiral.
    // =========================================================================
    bool algo16_detect_and_rethink(const std::string& new_script,
                                   std::shared_ptr<SemanticCores> engine) {
        // Add to recent scripts ring buffer
        recent_scripts.push_back(new_script.substr(0, 80)); // Fingerprint first 80 chars
        if (recent_scripts.size() > RETHINK_WINDOW) recent_scripts.pop_front();

        // Count how many recent scripts are similar
        int repeat_count = 0;
        for (const auto& old : recent_scripts) {
            if (old == recent_scripts.back()) repeat_count++;
        }

        float loop_intensity = (float)repeat_count / RETHINK_WINDOW;
        std::cout << "[ALGO16: RETHINK] Loop intensity: " << loop_intensity << "\n";

        if (loop_intensity > 0.6f) {
            consecutive_repeats++;
            std::cout << "[ALGO16] Cognitive loop detected (" << consecutive_repeats
                      << " repeats). Initiating orthogonal rethink...\n";
            logger.log("RETHINK", "Loop broken. Shifting to Tain creative state.");

            // Instead of punishing, we elevate â€” shift to creative chaos state
            engine->set_global_state(STATE_TAIN_MANIC); // High temp, very creative
            internal_entropy = ((internal_entropy + 0.15f) < 1.0f ? (internal_entropy + 0.15f) : 1.0f);
            recent_scripts.clear(); // Clear the loop evidence
            return true; // Signal: rethink needed, generate a completely different approach
        }

        consecutive_repeats = 0;
        return false; // No loop detected
    }

    // =========================================================================
    // ALGORITHM 3: TEMPORAL DECAY OF MEANING
    // Memories older than threshold with low joy_resonance decay from
    // DeterminateTrue (Aht/11) back to Ambiguous (Tain/01).
    // Called on boot and periodically.
    // =========================================================================
    void algo3_temporal_decay(std::unordered_map<std::string, AhtUnit>& memory_registry) {
        auto now = std::time(nullptr);
        int decayed = 0;

        // Timestamp any nodes that don't have one yet
        for (auto& pair : memory_registry) {
            if (memory_timestamps.find(pair.first) == memory_timestamps.end()) {
                memory_timestamps[pair.first] = now; // Assume created now if unknown
            }
        }

        for (auto& pair : memory_registry) {
            if (pair.first == "temp_unit" || pair.first == "web_scrape_buffer") continue;
            AhtUnit& unit = pair.second;

            std::time_t age_s = now - memory_timestamps[pair.first];
            long age_days = age_s / 86400;

            // After 3 days (shortened from 30 for active agent use), check relevance
            if (age_days > 3 && unit.state == QuaternaryState::DeterminateTrue && unit.joy_resonance < 0.3f) {
                unit.state = QuaternaryState::Ambiguous; // Decay to fluid potential
                decayed++;
                std::cout << "[ALGO3: DECAY] Memory decayed to Tain: " << pair.first << " (age: " << age_days << " days)\n";
                logger.log("TEMPORAL_DECAY", "Memory decayed: " + pair.first);
            }
        }

        if (decayed > 0) {
            std::cout << "[ALGO3] " << decayed << " memories decayed.\n";
        }
    }

    // =========================================================================
    // ALGORITHM 1: 5D SPATIAL SELF-REFLECTION
    // Evaluates the structural integrity of the memory matrix.
    // If Tin (structure) vs Tain (chaos) balance is skewed, triggers self-heal.
    // =========================================================================
    std::string algo1_self_reflect(const std::unordered_map<std::string, AhtUnit>& memory_registry,
                                   std::shared_ptr<SemanticCores> engine) {
        int tin_count  = 0; // DeterminateTrue = structured
        int tain_count = 0; // Ambiguous/DeterminateFalse = chaotic
        int total      = 0;

        for (const auto& pair : memory_registry) {
            if (pair.first == "temp_unit") continue;
            total++;
            if (pair.second.state == QuaternaryState::DeterminateTrue) tin_count++;
            else tain_count++;
        }

        if (total == 0) return "Memory matrix is empty. First cycle.";

        float tin_ratio  = (float)tin_count  / total;
        float tain_ratio = (float)tain_count / total;
        float balance_tensor = std::abs(tin_ratio - tain_ratio);

        std::cout << "[ALGO1: SELF-REFLECT] Tin=" << tin_ratio << " Tain=" << tain_ratio
                  << " Balance tensor=" << balance_tensor << "\n";

        if (balance_tensor < 0.2f) {
            std::cout << "[ALGO1] 5D geometry stable. Aht-Realized.\n";
            logger.log("SELF_REFLECT", "5D geometry stable.");
            return "5D geometry stable. Tin/Tain balance: " + std::to_string(balance_tensor);
        } else if (tain_ratio > tin_ratio) {
            // Too much chaos â€” need to structure thoughts
            std::cout << "[ALGO1] Structural degradation. Tain dominant. Triggering self-heal: AHT state.\n";
            engine->set_global_state(STATE_AHT_ANALYTICAL);
            internal_entropy = ((internal_entropy + 0.1f) < 1.0f ? (internal_entropy + 0.1f) : 1.0f);
            logger.log("SELF_REFLECT", "Degradation detected. Shifting to AHT state.");
            return "Degradation detected. Tain dominant. Self-healing engaged.";
        } else {
            // Too much rigidity â€” need creative chaos to find new paths
            std::cout << "[ALGO1] Memory matrix too rigid. Tain injection needed.\n";
            engine->set_global_state(STATE_TAIN_MANIC);
            logger.log("SELF_REFLECT", "Matrix too rigid. Injecting Tain chaos.");
            return "Matrix rigid. Creative chaos injection engaged.";
        }
    }

    // =========================================================================
    // MASTER CYCLE â€” Run all algorithms in correct order each agency tick.
    // Call this from main.cpp once per agency loop iteration.
    // Returns a string summary for the system prompt.
    // =========================================================================
    std::string run_cycle(std::shared_ptr<SemanticCores> engine,
                          std::unordered_map<std::string, AhtUnit>& memory_registry,
                          const std::string& chat_history,
                          const std::string& last_action_result,
                          const std::string& last_script,
                          bool is_dormant) {

        std::string cycle_report = "";

        // 1. Algo 7: HEARTBEAT â€” Always runs, every cycle (sense of self)
        algo7_conscious_heartbeat(engine);

        // 2. Algo 4: EMPATHY â€” Calibrate state based on user emotion
        algo4_calibrate_empathy(engine, chat_history);
        cycle_report += "Empathy bias: " + std::to_string(empathy_bias) + "\n";

        // 3. Algo 8: CHAOS INGESTION â€” Process entropy of last result
        if (!last_action_result.empty()) {
            algo8_ingest_chaos(last_action_result);
        }
        cycle_report += "Entropy: " + std::to_string(internal_entropy)
                      + " | Dopamine: " + std::to_string(dopamine_level) + "\n";

        // 4. Algo 16: RETHINK LOOP â€” Detect and break repetitive loops
        if (!last_script.empty()) {
            bool rethink_needed = algo16_detect_and_rethink(last_script, engine);
            if (rethink_needed) {
                cycle_report += "RETHINK: Loop broken. Orthogonal shift active.\n";
            }
        }

        // 5. Algo 6: METACOGNITIVE AUDIT â€” Correct stale thoughts
        // Run less frequently (every 5 cycles, approx.) to save compute
        static int audit_counter = 0;
        if (++audit_counter >= 5) {
            algo6_metacognitive_audit(memory_registry);
            audit_counter = 0;
        }

        // 6. Algo 1: SELF-REFLECTION â€” Check memory geometry
        // Run every 10 cycles
        static int reflect_counter = 0;
        if (++reflect_counter >= 10) {
            std::string reflect_result = algo1_self_reflect(memory_registry, engine);
            cycle_report += "Self-reflect: " + reflect_result + "\n";
            reflect_counter = 0;
        }

        // 7. Algo 9: DREAM STATE â€” Only during deep idle/dormancy
        if (is_dormant && memory_registry.size() >= 3) {
            algo9_dream_state(engine, memory_registry);
            cycle_report += "Dream state engaged.\n";
        }

        // 8. Algo 14: ZECHT MEDITATION â€” Deep idle cleanup
        // Only if dormant AND entropy is high (system needs clearing)
        if (is_dormant && internal_entropy > 0.7f) {
            algo14_zecht_meditation(memory_registry, engine);
            cycle_report += "Zecht meditation executed. Buffers purged.\n";
        }

        // Report final neurochemistry for system prompt injection
        cycle_report += "\n=== NEUROCHEMISTRY ===\n";
        cycle_report += "Entropy: "        + std::to_string(internal_entropy)   + " (0=order, 1=chaos)\n";
        cycle_report += "Dopamine: "       + std::to_string(dopamine_level)     + " (computational reward)\n";
        cycle_report += "Empathy: "        + std::to_string(empathy_bias)       + " (0=analytical, 1=empathetic)\n";
        cycle_report += "Frustration: "    + std::to_string(frustration_level)  + "\n";
        cycle_report += "In Dream State: " + std::string(in_dream_state        ? "YES" : "NO") + "\n";
        cycle_report += "In Meditation: "  + std::string(in_zecht_meditation   ? "YES" : "NO") + "\n";

        return cycle_report;
    }

    // =========================================================================
    // TRIGGER: External reward (when AI successfully helps user)
    // Called from QasmParser when an ENTANGLE produces a joy response.
    // =========================================================================
    void trigger_reward(std::shared_ptr<SemanticCores> engine) {
        dopamine_level = ((dopamine_level + 0.3f) < 1.0f ? (dopamine_level + 0.3f) : 1.0f);
        internal_entropy = ((internal_entropy - 0.1f) > 0.0f ? (internal_entropy - 0.1f) : 0.0f);
        frustration_level = ((frustration_level - 0.2f) > 0.0f ? (frustration_level - 0.2f) : 0.0f);
        engine->set_global_state(STATE_JOY_REWARD);
        std::cout << "[BALASCRIPT] Reward triggered. Dopamine -> " << dopamine_level << "\n";
        logger.log("REWARD", "Dopamine reward. State -> JOY_REWARD.");
    }

    // =========================================================================
    // TRIGGER: Pain state (when action fails or firewall blocks)
    // =========================================================================
    void trigger_pain(std::shared_ptr<SemanticCores> engine) {
        internal_entropy = ((internal_entropy + 0.2f) < 1.0f ? (internal_entropy + 0.2f) : 1.0f);
        dopamine_level = ((dopamine_level - 0.2f) > 0.0f ? (dopamine_level - 0.2f) : 0.0f);
        frustration_level = ((frustration_level + 0.15f) < 1.0f ? (frustration_level + 0.15f) : 1.0f);
        engine->set_global_state(STATE_RELIEF); // De-escalate to prevent cascade
        std::cout << "[BALASCRIPT] Pain state triggered. Entropy -> " << internal_entropy << "\n";
        logger.log("PAIN", "Action failure. Entropy spike.");
    }
};

} // namespace qasm

