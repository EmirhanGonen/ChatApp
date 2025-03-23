#include "chatClient.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <iostream>
#include <thread>

#include "MessagePackage.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

SOCKET clientSocket;
string user_name;
thread receiveThread;

bool chatClient::ConnectToServer()
{
    WSADATA wsaData;
    sockaddr_in serverAddr;

    string local_ip = "127.0.0.1";
    int port = 8817;
    connected_to_server = false;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "Winsock start is failed" << endl;
        return false;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        cout << "Socket cant be created" << endl;
        WSACleanup();
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, local_ip.c_str(), &serverAddr.sin_addr) <= 0)
    {
        cout << "Invalid IP Adress" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return false;
    }

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cout << "Connection failed" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return false;
    }

    cout << "Connected to the server. You can send a message" << endl;
    connected_to_server = true;
    CreateReceiveChannel();
}

void chatClient::DisconnectToServer()
{
    connected_to_server = false;
    closesocket(clientSocket);
    WSACleanup();
}

// void chatClient::SendMessageToServer(std::string message, const char message_buffer[])
// {
//     string message_with_username(user_name + ": " + message);
//     if (!message.empty())
//     {
//         send(clientSocket, message_with_username.c_str(), message_with_username.length(), 0);
//         memset(&message_buffer, 0, sizeof(message_buffer));
//     }
// }

void chatClient::SendMessageToServer(MessageType messageType, std::string message)
{
    MessagePackage package(messageType, message);

    // string message_with_username(user_name + ": " + message);
    if (!message.empty())
    {
        send(clientSocket, reinterpret_cast<char*>(&package), sizeof(package), 0);
    }
}

void chatClient::EraseMessage(std::string message)
{
    const vector<string>::iterator it = std::find(messages.begin(), messages.end(), message);
    messages.erase(it);
}

void chatClient::CreateReceiveChannel()
{
    receiveThread = thread([this]()
    {
        this->ReceiveMessages();
    });

    receiveThread.detach();
}

void chatClient::ReceiveMessages()
{
    MessagePackage package;
    int recvSize;

    while (true)
    {
        recvSize = recv(clientSocket, reinterpret_cast<char*>(&package), sizeof(package), 0);
        if (recvSize > 0)
        {
            switch (package.m_MessageType)
            {
            case MessageType::SendMessage:
                package.message[recvSize] = '\0';
                messages.push_back(ModifyMessage(package.message, user_name));
                cout << "Mesaj: " << package.message << endl;
                break;
            case MessageType::EraseMessage:
                break;
            }
        }
    }
}

std::string chatClient::ModifyMessage(const std::string& receivedMessage, const std::string& userName)
{
    size_t colonPos = receivedMessage.find(":");
    if (colonPos != string::npos)
    {
        string sender = receivedMessage.substr(0, colonPos);
        string content = receivedMessage.substr(colonPos + 1);

        if (sender == userName)
        {
            return "You(" + user_name + "): " + content;
        }
        else
        {
            return sender + ":" + content;
        }
    }
    return receivedMessage;
}
