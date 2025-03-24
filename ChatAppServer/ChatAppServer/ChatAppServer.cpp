#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <vector>

#include "MessagePackage.h"

#pragma comment (lib,"ws2_32.lib")

using namespace std;

vector<SOCKET> clients;

void Cleanup() { WSACleanup(); }
void CloseSocket(SOCKET socket) { closesocket(socket); }

void CloseSocketAndCleanup(SOCKET socket)
{
    closesocket(socket);
    Cleanup();
}

bool Initialize()
{
    WSADATA wsaDATA;
    if (WSAStartup(MAKEWORD(2, 2), &wsaDATA) != 0)
    {
        cout << "Winsock initialize failed!" << endl;
        return false;
    }
    return true;
}

void BroadcastMessage(const MessagePackage& message, SOCKET sender)
{
    for (SOCKET client : clients)
    {
        //if (client != sender) {
        send(client, reinterpret_cast<const char*>(&message), sizeof(message), 0);
        //}
    }
}

void HandleClient(SOCKET clientSocket)
{
    MessagePackage package;
    int recvSize;

    while ((recvSize = recv(clientSocket, reinterpret_cast<char*>(&package), sizeof(package), 0)) > 0)
    {
        BroadcastMessage(package, clientSocket);
    }
    closesocket(clientSocket);
    clients.erase(remove(clients.begin(), clients.end(), clientSocket), clients.end());
}

int main()
{
    if (!Initialize())
    {
        return -1;
    }
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    const int port = 8817;
    const int listen_count = 10;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        cout << "Socket create failed." << endl;
        //WSACleanup();
        Cleanup();
        return -1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cout << "Bind failed" << endl;
        CloseSocketAndCleanup(serverSocket);
        //closesocket(serverSocket);
        //WSACleanup();
        return -1;
    }

    if (listen(serverSocket, listen_count) == SOCKET_ERROR)
    {
        cout << "Listen error" << endl;
        CloseSocketAndCleanup(serverSocket);
        return -1;
    }
    cout << "Server is running... Waiting Connections..." << endl;

    while (true)
    {
        clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
        if (clientSocket == INVALID_SOCKET)
        {
            cout << "connection is not accepted" << endl;
            continue;
        }
        clients.push_back(clientSocket);

        thread clientThread(HandleClient, clientSocket);
        clientThread.detach();
    }
    cout << "Server has started succesfully" << endl;
    CloseSocketAndCleanup(serverSocket);
    return 1;
}
