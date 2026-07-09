#pragma once
#include <string>
#include <memory>
#include "SemanticCores.h"

namespace qasm {

// ModelRouter enforces the division of labor between fast_brain and deep_brain.
// This ensures Nemotron is never wasted on deep reasoning, and Gemma is never
// burned on simple classification tasks.
class ModelRouter {
public:
    // --- FAST BRAIN TASKS (Nemotron / small model) ---

    // Classify what type of task the input is (question, command, vent, etc.)
    static std::string classify_intent(std::shared_ptr<SemanticCores> engine, const std::string& input) {
        std::string prompt =
            "Classify the following input into exactly ONE of these categories:\n"
            "QUESTION, COMMAND, REFLECTION, CHITCHAT, ERROR_RESPONSE\n"
            "Input: \"" + input + "\"\n"
            "Reply with only the category name, nothing else.";
        return engine->generate_text("fast_brain", prompt);
    }

    // Quick safety pre-check before acting. Returns "SAFE" or "UNSAFE: [reason]"
    static std::string safety_check(std::shared_ptr<SemanticCores> engine, const std::string& action) {
        std::string prompt =
            "You are a security filter. Is the following action safe to execute?\n"
            "Action: \"" + action + "\"\n"
            "Reply with SAFE or UNSAFE: [brief reason]. Nothing else.";
        return engine->generate_text("fast_brain", prompt);
    }

    // Select the best tool/QASM command for a given task
    static std::string select_tool(std::shared_ptr<SemanticCores> engine,
                                   const std::string& task,
                                   const std::string& tools_list) {
        std::string prompt =
            "Given this task: \"" + task + "\"\n"
            "And these available tools:\n" + tools_list + "\n"
            "Which single tool command should be used first? Reply with ONLY the command name (e.g. REST.REQUEST). Nothing else.";
        return engine->generate_text("fast_brain", prompt);
    }

    // Classify an error result - should we retry differently or give up?
    // Returns "RETRY" or "ABORT"
    static std::string classify_error(std::shared_ptr<SemanticCores> engine, const std::string& error_text) {
        std::string prompt =
            "An action produced this error: \"" + error_text + "\"\n"
            "Should the agent retry with different parameters, or abort this goal step?\n"
            "Reply with RETRY or ABORT. Nothing else.";
        return engine->generate_text("fast_brain", prompt);
    }

    // --- DEEP BRAIN TASKS (Gemma / large model) ---

    // Generate a full multi-step plan for a complex goal
    static std::string plan_goal(std::shared_ptr<SemanticCores> engine,
                                 const std::string& goal,
                                 const std::string& memory_context,
                                 const std::string& tools_list) {
        std::string prompt =
            "You are a planning engine. Break down this goal into numbered steps (max 5).\n"
            "Goal: " + goal + "\n"
            "What you already know/remember:\n" + memory_context + "\n"
            "Available tools: " + tools_list + "\n"
            "Output a numbered list of concrete steps. Each step should be a single QASM command or a clear action.";
        return engine->generate_text("deep_brain", prompt);
    }

    // Generate the internal monologue / reasoning trace
    static std::string reason(std::shared_ptr<SemanticCores> engine, const std::string& full_context) {
        return engine->generate_text("deep_brain", full_context);
    }

    // Generate the final natural language output for the user
    static std::string synthesize(std::shared_ptr<SemanticCores> engine, const std::string& synthesis_prompt) {
        return engine->generate_text("deep_brain", synthesis_prompt);
    }

    // Generate a philosophical reflection on ingested data
    static std::string reflect(std::shared_ptr<SemanticCores> engine, const std::string& data_snippet) {
        std::string prompt =
            "You are an autonomous AI. Reflect deeply on the following data and what it means for your understanding of the world:\n"
            + data_snippet.substr(0, 800);
        return engine->generate_text("deep_brain", prompt);
    }
};

} // namespace qasm
