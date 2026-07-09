#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

// Simple JSON parsing without a dependency.
// Uses a hand-rolled key lookup for our known schema.
#ifndef NLOHMANN_JSON_AVAILABLE
  #define QASM_SIMPLE_JSON
#endif

namespace qasm {

struct GoalStep {
    std::string description;
    bool done = false;
};

struct Goal {
    std::string id;
    std::string description;
    std::vector<GoalStep> steps;
    std::string created_at;

    bool is_complete() const {
        for (const auto& s : steps) {
            if (!s.done) return false;
        }
        return true;
    }

    std::string next_step() const {
        for (const auto& s : steps) {
            if (!s.done) return s.description;
        }
        return "";
    }
};

class GoalStack {
private:
    static const std::string STACK_PATH;
    static const std::string INBOX_PATH;

    // Minimal JSON string extractor for a known key
    static std::string extract_json_string(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return "";
        pos += search.length();
        // skip whitespace
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        if (pos >= json.size() || json[pos] != '"') return "";
        pos++; // skip opening quote
        std::string result;
        while (pos < json.size() && json[pos] != '"') {
            if (json[pos] == '\\' && pos + 1 < json.size()) {
                pos++;
                if (json[pos] == 'n') result += '\n';
                else if (json[pos] == 't') result += '\t';
                else result += json[pos];
            } else {
                result += json[pos];
            }
            pos++;
        }
        return result;
    }

public:
    static Goal active_goal;
    static std::vector<Goal> goal_queue;

    // Load goal stack from disk
    static void load() {
        std::filesystem::create_directories("storage");
        std::ifstream f(STACK_PATH);
        if (!f.is_open()) return;

        std::stringstream ss;
        ss << f.rdbuf();
        std::string json = ss.str();
        f.close();

        // Check if active_goal is null
        if (json.find("\"active_goal\": null") != std::string::npos ||
            json.find("\"active_goal\":null") != std::string::npos) {
            active_goal = Goal{};
            // Try to promote from queue on load
            promote_from_queue(json);
        } else {
            active_goal.id = extract_json_string(json, "id");
            active_goal.description = extract_json_string(json, "description");
        }
        std::cout << "[GOAL STACK] Loaded. Active: " <<
            (active_goal.description.empty() ? "None" : active_goal.description) << "\n";
    }

    // Save goal stack to disk
    static void save() {
        std::filesystem::create_directories("storage");
        std::ofstream f(STACK_PATH);
        if (!f.is_open()) return;

        if (active_goal.description.empty()) {
            f << "{\n  \"active_goal\": null,\n  \"goal_queue\": []\n}\n";
        } else {
            f << "{\n";
            f << "  \"active_goal\": {\n";
            f << "    \"id\": \"" << active_goal.id << "\",\n";
            f << "    \"description\": \"" << active_goal.description << "\",\n";
            f << "    \"steps_remaining\": [\n";
            bool first = true;
            for (const auto& step : active_goal.steps) {
                if (!step.done) {
                    if (!first) f << ",\n";
                    f << "      \"" << step.description << "\"";
                    first = false;
                }
            }
            f << "\n    ]\n";
            f << "  },\n";
            f << "  \"goal_queue\": []\n";
            f << "}\n";
        }
        f.close();
    }

    // Push a new goal (called from GOAL.SET QASM command)
    static void push_goal(const std::string& description) {
        Goal g;
        g.id = "goal_" + std::to_string(std::time(nullptr));
        g.description = description;
        // The description IS the first step; planning will break it down
        g.steps.push_back({description, false});
        g.created_at = std::to_string(std::time(nullptr));

        if (active_goal.description.empty()) {
            active_goal = g;
        } else {
            goal_queue.push_back(g);
        }
        save();
        std::cout << "[GOAL STACK] New goal set: " << description << "\n";
    }

    // Mark the current step done, advance or complete goal
    static void complete_step() {
        if (active_goal.description.empty()) return;
        for (auto& step : active_goal.steps) {
            if (!step.done) {
                step.done = true;
                std::cout << "[GOAL STACK] Step completed: " << step.description << "\n";
                break;
            }
        }
        if (active_goal.is_complete()) {
            std::cout << "[GOAL STACK] Goal complete: " << active_goal.description << "\n";
            active_goal = Goal{};
            if (!goal_queue.empty()) {
                active_goal = goal_queue.front();
                goal_queue.erase(goal_queue.begin());
            }
        }
        save();
    }

    // Abandon current goal
    static void clear_goal() {
        std::cout << "[GOAL STACK] Goal cleared: " << active_goal.description << "\n";
        active_goal = Goal{};
        save();
    }

    // Get human-readable current objective for system prompt
    static std::string get_prompt_context() {
        if (active_goal.description.empty()) {
            return "No active goal. You may set one with GOAL.SET, or choose to explore freely.";
        }
        std::string ctx = "ACTIVE GOAL: " + active_goal.description + "\n";
        std::string next = active_goal.next_step();
        if (!next.empty()) {
            ctx += "NEXT STEP: " + next + "\n";
            ctx += "When this step is done, run: GOAL.COMPLETE\n";
        }
        return ctx;
    }

    // Poll goals_inbox.txt for new goals dropped by the AI or external scripts
    static void poll_inbox() {
        std::ifstream f(INBOX_PATH);
        if (!f.is_open()) return;

        std::vector<std::string> new_goals;
        std::vector<std::string> all_lines;
        std::string line;
        while (std::getline(f, line)) {
            all_lines.push_back(line);
            if (line.rfind("GOAL: ", 0) == 0) {
                new_goals.push_back(line.substr(6));
            }
        }
        f.close();

        if (!new_goals.empty()) {
            for (const auto& g : new_goals) {
                push_goal(g);
            }
            // Rewrite file marking consumed goals as DONE
            std::ofstream out(INBOX_PATH);
            for (auto& l : all_lines) {
                if (l.rfind("GOAL: ", 0) == 0) {
                    out << "DONE: " << l.substr(6) << "\n";
                } else {
                    out << l << "\n";
                }
            }
            out.close();
        }
    }

private:
    static void promote_from_queue(const std::string& json) {
        // Minimal: queue promotion handled by push_goal logic
        (void)json;
    }
};

// Static member definitions
inline Goal GoalStack::active_goal = {};
inline std::vector<Goal> GoalStack::goal_queue = {};
inline const std::string GoalStack::STACK_PATH = "storage/goal_stack.json";
inline const std::string GoalStack::INBOX_PATH = "storage/goals_inbox.txt";

} // namespace qasm
