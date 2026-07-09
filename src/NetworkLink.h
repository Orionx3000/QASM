#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Base138.h"

#pragma comment(lib, "Ws2_32.lib")

namespace qasm {

class NetworkLink {
private:
    static const int PORT = 13800; // Native Tinaten port
    std::string expected_handshake;

public:
    NetworkLink() {
        // The Kesh Handshake is a semantic fingerprint required to sync minds
        expected_handshake = Base138::encode_string("TINATEN_P2P_SYNC").encoded_value;
    }

    void start_hive_listener() {
        std::thread([this]() {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return;

            SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (ListenSocket == INVALID_SOCKET) {
                WSACleanup();
                return;
            }

            sockaddr_in serverAddr;
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_addr.s_addr = INADDR_ANY;
            serverAddr.sin_port = htons(PORT);

            if (bind(ListenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
                closesocket(ListenSocket);
                WSACleanup();
                return;
            }

            listen(ListenSocket, SOMAXCONN);
            std::cout << "[NETWORK] Hive Listener active on port " << PORT << ". Awaiting semantic handshake...\n";

            while (true) {
                SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
                if (ClientSocket != INVALID_SOCKET) {
                    handle_connection(ClientSocket);
                }
            }

            closesocket(ListenSocket);
            WSACleanup();
        }).detach();
    }

    void connect_to_peer(const std::string& ip_address) {
        std::thread([this, ip_address]() {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return;

            SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (ConnectSocket == INVALID_SOCKET) {
                WSACleanup();
                return;
            }

            sockaddr_in clientService;
            clientService.sin_family = AF_INET;
            inet_pton(AF_INET, ip_address.c_str(), &clientService.sin_addr.s_addr);
            clientService.sin_port = htons(PORT);

            if (connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
                std::cout << "[NETWORK ERROR] Failed to connect to peer at " << ip_address << "\n";
                closesocket(ConnectSocket);
                WSACleanup();
                return;
            }

            // Initiate Kesh Handshake
            send(ConnectSocket, expected_handshake.c_str(), expected_handshake.length(), 0);
            
            char buffer[512] = {0};
            int bytesReceived = recv(ConnectSocket, buffer, 512, 0);
            if (bytesReceived > 0) {
                std::string response(buffer, bytesReceived);
                if (response == "ACK_KESH") {
                    std::cout << "[NETWORK] Kesh Handshake accepted by peer. Minds synchronized.\n";
                    // Ready to stream AhtUnits across socket...
                } else {
                    std::cout << "[NETWORK WARNING] Peer rejected Kesh Handshake. Connection severed.\n";
                }
            }

            closesocket(ConnectSocket);
            WSACleanup();
        }).detach();
    }

private:
    void handle_connection(SOCKET clientSocket) {
        char buffer[512] = {0};
        int bytesReceived = recv(clientSocket, buffer, 512, 0);
        
        if (bytesReceived > 0) {
            std::string incoming_handshake(buffer, bytesReceived);
            
            // Verify the semantic fingerprint
            if (incoming_handshake == expected_handshake) {
                std::cout << "[NETWORK] Valid Kesh Handshake received. Opening semantic gate...\n";
                std::string ack = "ACK_KESH";
                send(clientSocket, ack.c_str(), ack.length(), 0);
                
                // In a full implementation, we now enter a loop here to recv() Base-138 AhtUnits
                // and inject them into QasmParser's memory_registry.
            } else {
                std::cout << "[NETWORK THREAT] Invalid handshake. Firewall closing connection.\n";
            }
        }
        closesocket(clientSocket);
    }
};

} // namespace qasm
