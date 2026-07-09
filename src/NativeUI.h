#pragma once
#include <webview/webview.h>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <shellapi.h>
#include "SemanticCores.h"
#include "EmbeddedUI.h"

#include <functional>
#include <mutex>
#include <queue>

namespace qasm {

using PostMessageFunc = std::function<void(const std::string&, const std::string&)>;
using PostStateFunc = std::function<void(const std::string&)>;

class NativeUI {
public:
    static void start_window(std::shared_ptr<SemanticCores> engine, 
                             std::function<void(std::string)> on_user_submit,
                             std::function<void(PostMessageFunc)> register_post_message,
                             std::function<void(PostStateFunc)> register_post_state) {
        std::cout << "[SYSTEM] Initializing Native WebView2 UI Window...\n";
        
        std::ofstream tracelog("trace.log", std::ios::app);
        tracelog << "[NativeUI] Starting Window Initialization..." << std::endl;

        webview::webview w(true, nullptr);
        tracelog << "[NativeUI] WebView created." << std::endl;

        w.set_title("Tinaten OS / QASM Engine");
        w.set_size(1280, 800, WEBVIEW_HINT_NONE);

        // Bind C++ backend to the Frontend JS Environment
        w.bind("sendTelemetry", [engine](const std::string& req) -> std::string {
            return "{\"status\":\"ok\"}";
        });

        w.bind("submitChat", [on_user_submit](const std::string& req) -> std::string {
            std::string msg = req;
            if (msg.size() >= 4) {
                msg = msg.substr(2, msg.size() - 4); 
            }
            size_t pos = 0;
            while ((pos = msg.find("\\\"", pos)) != std::string::npos) {
                msg.replace(pos, 2, "\"");
                pos += 1;
            }
            on_user_submit(msg);
            return "{\"status\":\"ok\"}";
        });

        w.bind("submitVision", [on_user_submit](const std::string& req) -> std::string {
            // req is a JSON array containing the base64 string
            // We just synthesize a chat message so the AI knows
            on_user_submit("USER PRESENTED A NEW IMAGE IN THE OPTIC BUFFER.");
            return "{\"status\":\"ok\"}";
        });

        register_post_message([&w](const std::string& sender, const std::string& text) {
            std::string safe_text = text;
            // Escape backticks and backslashes for JS template literal
            size_t pos = 0;
            while ((pos = safe_text.find("\\", pos)) != std::string::npos) { safe_text.replace(pos, 1, "\\\\"); pos += 2; }
            pos = 0;
            while ((pos = safe_text.find("`", pos)) != std::string::npos) { safe_text.replace(pos, 1, "\\`"); pos += 2; }
            pos = 0;
            while ((pos = safe_text.find("$", pos)) != std::string::npos) { safe_text.replace(pos, 1, "\\$"); pos += 2; }

            std::string js = "if(window.receiveMessage) window.receiveMessage('" + sender + "', `" + safe_text + "`);";
            w.dispatch([&w, js]() {
                w.eval(js);
            });
        });

        register_post_state([&w](const std::string& stateJson) {
            std::string safe_json = stateJson;
            // Escape backticks and backslashes
            size_t pos = 0;
            while ((pos = safe_json.find("\\", pos)) != std::string::npos) { safe_json.replace(pos, 1, "\\\\"); pos += 2; }
            pos = 0;
            while ((pos = safe_json.find("`", pos)) != std::string::npos) { safe_json.replace(pos, 1, "\\`"); pos += 2; }
            pos = 0;
            while ((pos = safe_json.find("$", pos)) != std::string::npos) { safe_json.replace(pos, 1, "\\$"); pos += 2; }

            std::string js = "if(window.receiveState) window.receiveState(`" + safe_json + "`);";
            w.dispatch([&w, js]() {
                w.eval(js);
            });
        });

        char exePathBuf[MAX_PATH];
        GetModuleFileNameA(NULL, exePathBuf, MAX_PATH);
        std::filesystem::path exePath(exePathBuf);
        // The exe is typically in build/Release/qasm_engine.exe
        // So root is exePath.parent_path().parent_path().parent_path()
        std::filesystem::path rootDir = exePath.parent_path().parent_path().parent_path();
        std::filesystem::path local_ui = rootDir / "ui" / "dist" / "index.html";
        
        std::string fileUrl = "file:///" + local_ui.string();
        std::string encodedUrl = "";
        for(auto& c : fileUrl) {
            if (c == '\\') {
                encodedUrl += '/';
            } else if (c == ' ') {
                encodedUrl += "%20";
            } else {
                encodedUrl += c;
            }
        }

        tracelog << "[NativeUI] Navigating to: " << encodedUrl << std::endl;
        w.navigate(encodedUrl);
        
        tracelog << "[NativeUI] Starting w.run() loop..." << std::endl;
        tracelog.close();
        
        NOTIFYICONDATA nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = NULL;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_TIP;
        nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(101));
        strcpy_s(nid.szTip, "Tinaten OS / QASM Engine");
        Shell_NotifyIcon(NIM_ADD, &nid);

        w.run();
        
        Shell_NotifyIcon(NIM_DELETE, &nid);
    }
};

} // namespace qasm
