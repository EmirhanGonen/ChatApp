#include "chatClient.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

SOCKET clientSocket;
thread receiveThread;

bool chatClient::ConnectToServer() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup hatası: " << WSAGetLastError() << endl;
        return false;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Socket oluşturulamadı: " << WSAGetLastError() << endl;
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8817);
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        cerr << "Geçersiz IP adresi!" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return false;
    }

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bağlantı hatası: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return false;
    }

    connected_to_server = true;
    CreateReceiveChannel();
    return true;
}

void chatClient::DisconnectToServer() {
    connected_to_server = false;
    closesocket(clientSocket);
    WSACleanup();
}

void chatClient::SendMessageToServer(MessageType messageType, std::string message) {
    if (!connected_to_server || message.empty()) return;

    MessagePackage package(messageType, message, user_name, user_name);
    int totalSent = 0;
    int packageSize = sizeof(MessagePackage);

    while (totalSent < packageSize) {
        int sent = send(clientSocket, ((char*)&package) + totalSent, packageSize - totalSent, 0);
        if (sent == SOCKET_ERROR) {
            cerr << "Gönderme hatası: " << WSAGetLastError() << endl;
            DisconnectToServer();
            return;
        }
        totalSent += sent;
    }
}

void chatClient::EraseMessage(const std::string& formattedMessage) {
    size_t colonPos = formattedMessage.find(": ");
    if (colonPos == string::npos) return;

    string sender = formattedMessage.substr(0, colonPos);
    string content = formattedMessage.substr(colonPos + 2);

    if (sender != user_name) {
        cerr << "Sadece kendi mesajlarınızı silebilirsiniz!" << endl;
        return;
    }

    SendMessageToServer(MessageType::EraseMessagePackage, content);
}

void chatClient::CreateReceiveChannel() {
    receiveThread = thread([this]() { this->ReceiveMessages(); });
    receiveThread.detach();
}

void chatClient::ReceiveMessages() {
    MessagePackage package;
    int packageSize = sizeof(MessagePackage);

    while (connected_to_server) {
        int totalReceived = 0;
        while (totalReceived < packageSize) {
            int received = recv(clientSocket, ((char*)&package) + totalReceived, packageSize - totalReceived, 0);
            if (received <= 0) {
                DisconnectToServer();
                return;
            }
            totalReceived += received;
        }

        lock_guard<mutex> lock(messagesMutex);
        switch (package.m_MessageType) {
            case MessageType::SendMessagePackage: {
                string formattedMsg = string(package.m_MessageOwner) + ": " + package.message;
                messages.push_back(formattedMsg);
                receivedMessages.push_back(package);
                break;
            }
            case MessageType::EraseMessagePackage: {
                string targetMsg = string(package.m_PackageOwner) + ": " + package.message;
                messages.erase(remove(messages.begin(), messages.end(), targetMsg), messages.end());
                receivedMessages.erase(
                    remove_if(receivedMessages.begin(), receivedMessages.end(),
                        [&](const MessagePackage& pkg) {
                            return string(pkg.message) == package.message && 
                                   string(pkg.m_PackageOwner) == package.m_PackageOwner;
                        }),
                    receivedMessages.end());
                break;
            }
        }
    }
}

vector<string> chatClient::GetClientMessages() const {
    lock_guard<mutex> lock(messagesMutex);
    return messages;
}