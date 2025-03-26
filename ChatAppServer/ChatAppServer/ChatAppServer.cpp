#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <vector>
#include <mutex>
#include "MessagePackage.h"

#pragma comment (lib,"ws2_32.lib")

using namespace std;

vector<SOCKET> clients;
mutex clientsMutex;

void Cleanup() { WSACleanup(); }
void CloseSocket(SOCKET socket) { closesocket(socket); }

void CloseSocketAndCleanup(SOCKET socket) {
    closesocket(socket);
    Cleanup();
}

bool Initialize() {
    WSADATA wsaDATA;
    return WSAStartup(MAKEWORD(2, 2), &wsaDATA) == 0;
}

void BroadcastMessage(const MessagePackage& message, SOCKET sender) {
    lock_guard<mutex> lock(clientsMutex);
    const char* data = reinterpret_cast<const char*>(&message);
    int dataSize = sizeof(MessagePackage);

    for (SOCKET client : clients) {
        int totalSent = 0;
        while (totalSent < dataSize) {
            int sent = send(client, data + totalSent, dataSize - totalSent, 0);
            if (sent == SOCKET_ERROR) {
                cerr << "Send error: " << WSAGetLastError() << endl;
                {
                    lock_guard<mutex> lock(clientsMutex);
                    clients.erase(remove(clients.begin(), clients.end(), client), clients.end());
                }
                closesocket(client);
                break;
            }
            totalSent += sent;
        }
    }
}

void HandleClient(SOCKET clientSocket) {
    MessagePackage package;
    int packageSize = sizeof(MessagePackage);

    while (true) {
        int totalReceived = 0;
        while (totalReceived < packageSize) {
            int recvSize = recv(clientSocket, reinterpret_cast<char*>(&package) + totalReceived, packageSize - totalReceived, 0);
            if (recvSize <= 0) {
                {
                    lock_guard<mutex> lock(clientsMutex);
                    clients.erase(remove(clients.begin(), clients.end(), clientSocket), clients.end());
                }
                closesocket(clientSocket);
                return;
            }
            totalReceived += recvSize;
        }
        BroadcastMessage(package, clientSocket);
    }
}

int main() {
    if (!Initialize()) {
        cerr << "Failed to initialize Winsock!" << endl;
        return -1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Failed to create socket!" << endl;
        Cleanup();
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8817);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind error: " << WSAGetLastError() << endl;
        CloseSocketAndCleanup(serverSocket);
        return -1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        cerr << "Listen error: " << WSAGetLastError() << endl;
        CloseSocketAndCleanup(serverSocket);
        return -1;
    }

    cout << "Server is running..." << endl;
    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept error: " << WSAGetLastError() << endl;
            continue;
        }

        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        thread(HandleClient, clientSocket).detach();
    }

    CloseSocketAndCleanup(serverSocket);
    return 0;
}