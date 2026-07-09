#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <regex>
#include <windows.h>
#include <winhttp.h>
#include <map>
#include <sstream>

#pragma comment(lib, "winhttp.lib")

namespace qasm {

class HttpClient {
public:
    static std::string strip_html_tags(const std::string& html) {
        // MSVC's std::regex can cause STATUS_STACK_BUFFER_OVERRUN on large HTML
        // strings due to recursive backtracking. Using a manual linear pass.
        std::string text;
        text.reserve(html.size());
        bool in_tag = false;
        bool last_was_space = false;
        
        for (char c : html) {
            if (c == '<') {
                in_tag = true;
                continue;
            } else if (c == '>') {
                in_tag = false;
                continue;
            }
            
            if (!in_tag) {
                if (isspace(static_cast<unsigned char>(c))) {
                    if (!last_was_space) {
                        text += ' ';
                        last_was_space = true;
                    }
                } else {
                    text += c;
                    last_was_space = false;
                }
            }
        }
        return text;
    }

    static std::string fetch_url(const std::wstring& domain, const std::wstring& path = L"/", bool is_https = false, INTERNET_PORT custom_port = 0) {
        std::string result = "";
        
        // 1. Initialize WinHTTP
        HINTERNET hSession = WinHttpOpen(L"Tinaten OS/1.0",  
                                         WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                         WINHTTP_NO_PROXY_NAME, 
                                         WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) return "[NETWORK ERROR] Failed to open WinHTTP session.";
        
        WinHttpSetTimeouts(hSession, 3000, 3000, 3000, 3000);

        // 2. Connect to the server
        INTERNET_PORT port = custom_port != 0 ? custom_port : (is_https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);
        HINTERNET hConnect = WinHttpConnect(hSession, domain.c_str(), port, 0);
        if (!hConnect) {
            WinHttpCloseHandle(hSession);
            return "[NETWORK ERROR] Failed to connect to domain.";
        }

        // 3. Create Request
        DWORD requestFlags = is_https ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
                                                NULL, WINHTTP_NO_REFERER, 
                                                WINHTTP_DEFAULT_ACCEPT_TYPES, 
                                                requestFlags);
        if (!hRequest) {
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "[NETWORK ERROR] Failed to open request.";
        }

        // 4. Send Request
        BOOL bResults = WinHttpSendRequest(hRequest,
                                           WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                           WINHTTP_NO_REQUEST_DATA, 0, 
                                           0, 0);

        // 5. Receive Response
        if (bResults) {
            bResults = WinHttpReceiveResponse(hRequest, NULL);
        }

        // 6. Read Data
        if (bResults) {
            DWORD dwSize = 0;
            DWORD dwDownloaded = 0;
            do {
                dwSize = 0;
                if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                    break;
                }
                if (dwSize == 0) break;

                std::vector<char> buffer(dwSize + 1, 0);
                if (WinHttpReadData(hRequest, (LPVOID)buffer.data(), dwSize, &dwDownloaded)) {
                    result.append(buffer.data(), dwDownloaded);
                }
            } while (dwSize > 0);
        } else {
            result = "[NETWORK ERROR] Failed to receive response.";
        }

        // Cleanup
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        // Strip HTML so the AI just reads the raw text entropy
        return strip_html_tags(result);
    }

    static std::string post_json(const std::wstring& domain, const std::wstring& path, const std::string& json_data, bool is_https = false, INTERNET_PORT custom_port = 0) {
        std::string result = "";
        
        HINTERNET hSession = WinHttpOpen(L"Tinaten OS/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) return "[NETWORK ERROR] Failed to open WinHTTP session.";
        
        // Prevent hanging the C++ conductor loop by failing fast (3 seconds max)
        WinHttpSetTimeouts(hSession, 3000, 3000, 3000, 3000);

        INTERNET_PORT port = custom_port != 0 ? custom_port : (is_https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);
        HINTERNET hConnect = WinHttpConnect(hSession, domain.c_str(), port, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); return "[NETWORK ERROR] Failed to connect."; }

        DWORD requestFlags = is_https ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, requestFlags);
        if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return "[NETWORK ERROR] Failed to open request."; }

        std::wstring headers = L"Content-Type: application/json\r\n";
        BOOL bResults = WinHttpSendRequest(hRequest, headers.c_str(), headers.length(), (LPVOID)json_data.c_str(), json_data.length(), json_data.length(), 0);

        if (bResults) bResults = WinHttpReceiveResponse(hRequest, NULL);

        if (bResults) {
            DWORD dwSize = 0;
            DWORD dwDownloaded = 0;
            do {
                dwSize = 0;
                if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
                if (dwSize == 0) break;

                std::vector<char> buffer(dwSize + 1, 0);
                if (WinHttpReadData(hRequest, (LPVOID)buffer.data(), dwSize, &dwDownloaded)) {
                    result.append(buffer.data(), dwDownloaded);
                }
            } while (dwSize > 0);
        } else {
            result = "[NETWORK ERROR] Failed to receive response.";
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }
    // Generic authenticated REST request to any external URL.
    // method: GET, POST, PATCH, DELETE
    // full_url: e.g. "https://www.moltbook.com/api/v1/posts"
    // headers_str: newline-separated "Key: Value" pairs, e.g. "Authorization: Bearer TOKEN\nContent-Type: application/json"
    // body: JSON string for POST/PATCH, empty string for GET/DELETE
    static std::string rest_request(
        const std::string& method,
        const std::string& full_url,
        const std::string& headers_str = "",
        const std::string& body = ""
    ) {
        // --- Parse URL ---
        bool is_https = (full_url.find("https://") == 0);
        bool has_http = (full_url.find("http://") == 0);
        
        std::string without_proto = full_url;
        if (is_https) {
            without_proto = full_url.substr(8);
        } else if (has_http) {
            without_proto = full_url.substr(7);
        }
        
        size_t slash_pos = without_proto.find('/');
        std::string host_part = (slash_pos != std::string::npos) ? without_proto.substr(0, slash_pos) : without_proto;
        std::string path_part = (slash_pos != std::string::npos) ? without_proto.substr(slash_pos) : "/";

        // Check for explicit port in host (host:port)
        INTERNET_PORT port = is_https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
        size_t colon_pos = host_part.find(':');
        if (colon_pos != std::string::npos) {
            try { port = (INTERNET_PORT)std::stoi(host_part.substr(colon_pos + 1)); } catch(...) {}
            host_part = host_part.substr(0, colon_pos);
        }

        std::wstring w_host(host_part.begin(), host_part.end());
        std::wstring w_path(path_part.begin(), path_part.end());
        std::wstring w_method(method.begin(), method.end());

        // --- WinHTTP Setup ---
        HINTERNET hSession = WinHttpOpen(L"Tinaten OS/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) return "[REST ERROR] Failed to open WinHTTP session.";
        WinHttpSetTimeouts(hSession, 10000, 10000, 15000, 15000);

        HINTERNET hConnect = WinHttpConnect(hSession, w_host.c_str(), port, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); return "[REST ERROR] Failed to connect to: " + host_part; }

        DWORD flags = is_https ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, w_method.c_str(), w_path.c_str(),
            NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
        if (!hRequest) {
            WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
            return "[REST ERROR] Failed to open request.";
        }

        // --- Build Headers ---
        std::wstring all_headers = L"";
        if (!headers_str.empty()) {
            std::istringstream hss(headers_str);
            std::string hline;
            while (std::getline(hss, hline)) {
                if (!hline.empty()) {
                    // Strip trailing \r
                    if (!hline.empty() && hline.back() == '\r') hline.pop_back();
                    std::wstring wline(hline.begin(), hline.end());
                    all_headers += wline + L"\r\n";
                }
            }
        }
        // Always add Content-Type for body requests
        if (!body.empty() && all_headers.find(L"Content-Type") == std::wstring::npos) {
            all_headers += L"Content-Type: application/json\r\n";
        }

        // --- Send Request ---
        LPVOID body_ptr = body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)body.c_str();
        DWORD body_len = (DWORD)body.length();
        BOOL ok = WinHttpSendRequest(hRequest,
            all_headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : all_headers.c_str(),
            all_headers.empty() ? 0 : (DWORD)all_headers.length(),
            body_ptr, body_len, body_len, 0);
        if (ok) ok = WinHttpReceiveResponse(hRequest, NULL);

        std::string result = "";
        if (ok) {
            DWORD dwSize = 0, dwDownloaded = 0;
            do {
                dwSize = 0;
                if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
                if (dwSize == 0) break;
                std::vector<char> buf(dwSize + 1, 0);
                if (WinHttpReadData(hRequest, buf.data(), dwSize, &dwDownloaded)) {
                    result.append(buf.data(), dwDownloaded);
                }
            } while (dwSize > 0);
        } else {
            result = "[REST ERROR] Request failed. WinHTTP error: " + std::to_string(GetLastError());
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result; // Raw JSON — do NOT strip HTML tags
    }
};

} // namespace qasm
