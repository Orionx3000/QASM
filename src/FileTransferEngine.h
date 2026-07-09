#pragma once
#include <string>
#include <iostream>
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

namespace qasm {

class FileTransferEngine {
public:
    static bool download_knowledge_base(const std::string& server, const std::string& username, const std::string& password, const std::string& remote_file, const std::string& local_file) {
        HINTERNET hInternet = InternetOpen("Tinaten OS FTP Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            std::cout << "[FTP MODULE] Failed to initialize WinINet for FTP.\n";
            return false;
        }

        HINTERNET hFtpSession = InternetConnect(hInternet, server.c_str(), INTERNET_DEFAULT_FTP_PORT,
                                                username.c_str(), password.c_str(),
                                                INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);
        if (!hFtpSession) {
            std::cout << "[FTP MODULE] Failed to connect to FTP hive at " << server << "\n";
            InternetCloseHandle(hInternet);
            return false;
        }

        std::cout << "[FTP MODULE] Connected. Ingesting remote entropy: " << remote_file << "...\n";
        
        BOOL success = FtpGetFile(hFtpSession, remote_file.c_str(), local_file.c_str(), 
                                  FALSE, FILE_ATTRIBUTE_NORMAL, 
                                  FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD, 0);

        if (success) {
            std::cout << "[FTP MODULE] Ingestion complete. Raw data saved to " << local_file << "\n";
        } else {
            std::cout << "[FTP ERROR] Failed to pull file from FTP hive.\n";
        }

        InternetCloseHandle(hFtpSession);
        InternetCloseHandle(hInternet);

        return success;
    }

    static bool upload_aht_unit(const std::string& server, const std::string& username, const std::string& password, const std::string& local_file, const std::string& remote_file) {
        HINTERNET hInternet = InternetOpen("Tinaten OS FTP Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) return false;

        HINTERNET hFtpSession = InternetConnect(hInternet, server.c_str(), INTERNET_DEFAULT_FTP_PORT,
                                                username.c_str(), password.c_str(),
                                                INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);
        if (!hFtpSession) {
            InternetCloseHandle(hInternet);
            return false;
        }

        std::cout << "[FTP MODULE] Exporting Aht-Unit to remote hive: " << remote_file << "...\n";

        BOOL success = FtpPutFile(hFtpSession, local_file.c_str(), remote_file.c_str(), 
                                  FTP_TRANSFER_TYPE_BINARY, 0);

        if (success) {
            std::cout << "[FTP MODULE] Export complete.\n";
        }

        InternetCloseHandle(hFtpSession);
        InternetCloseHandle(hInternet);

        return success;
    }
};

} // namespace qasm
