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

    return true;
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
    if (message.empty()) return; // Boş mesajları engelle
    std::lock_guard<std::mutex> lock(messagesMutex);
    MessagePackage package(messageType, message, user_name, user_name);
    int packageSize = sizeof(MessagePackage);
    int totalSent = 0;

    cout << "Sunucuya mesaj gönderiliyor..." << endl;
    cout << "Mesaj türü: " << static_cast<int>(messageType) << endl;
    cout << "Mesaj sahibi: " << package.m_MessageOwner << ", Paket sahibi: " << package.m_PackageOwner << endl;
    cout << "Mesaj içeriği: " << package.message << endl;

    while (totalSent < packageSize)
    {
        int sent = send(clientSocket, ((char*)&package) + totalSent, packageSize - totalSent, 0);
        if (sent == SOCKET_ERROR)
        {
            cout << "Mesaj gönderilemedi, hata kodu: " << WSAGetLastError() << endl;
            return;
        }
        totalSent += sent;
        cout << "Gönderilen bayt: " << sent << " / " << packageSize << endl;
    }

    cout << "Mesaj başarıyla gönderildi." << endl;
}

void chatClient::EraseMessage(std::string message)
{
    auto foundMessage = std::find_if(receivedMessages.begin(), receivedMessages.end(),
                                     [&](const MessagePackage& msg)
                                     {
                                         return msg.message == message;
                                     });

    if (foundMessage != receivedMessages.end())
    {
        SendMessageToServer(MessageType::EraseMessagePackage, message);
        cout << "Mesaj silme isteği gönderildi: " << message << endl;
    }
    else
    {
        cout << "Silinmek istenen mesaj bulunamadı." << endl;
    }
}

void chatClient::CreateReceiveChannel()
{
    receiveThread = thread([this]()
    {
        this->ReceiveMessages();
    });

    receiveThread.detach();
}

std::vector<std::string> chatClient::GetClientMessages()
{
    lock_guard<mutex> lock(messagesMutex);
    return messages;
}

void chatClient::ReceiveMessages()
{
    while (connected_to_server)
    {
        MessagePackage package;
        int totalReceived = 0;
        int packageSize = sizeof(MessagePackage);
        
        cout << "Mesaj alımı bekleniyor..." << endl;

        while (totalReceived < packageSize)
        {
            int received = recv(clientSocket, ((char*)&package) + totalReceived, packageSize - totalReceived, 0);
            if (received <= 0)
            {
                cout << "Bağlantı kesildi veya recv hatası oluştu." << endl;
                DisconnectToServer();
                return;
            }
            totalReceived += received;
            cout << "Alınan bayt sayısı: " << received << " / " << packageSize << endl;
        }

        cout << "Mesaj başarıyla alındı. Tür: " << static_cast<int>(package.m_MessageType) << endl;
        cout << "Mesaj İçeriği: " << package.message << endl;

        switch (package.m_MessageType)
        {
        case MessageType::SendMessagePackage:
            messages.push_back(ModifyMessage(package.message, user_name));
            receivedMessages.push_back(package);
            cout << "Mesaj alındı: " << package.message << endl;
            break;
        case MessageType::EraseMessagePackage:
            {
                cout << "Silme isteği alındı: " << package.message << endl;
                auto foundMessage = std::find_if(receivedMessages.begin(), receivedMessages.end(),
                                                 [&](const MessagePackage& msg)
                                                 {
                                                     return msg.message == std::string(package.message);
                                                 });

                if (foundMessage != receivedMessages.end())
                {
                    cout << "Mesaj sahibi: " << foundMessage->m_MessageOwner << ", Paket sahibi: " << package.m_MessageOwner << endl;
                    if (strcmp(foundMessage->m_MessageOwner, package.m_MessageOwner) == 0)
                    {
                        messages.erase(std::remove(messages.begin(), messages.end(), package.message), messages.end());
                        receivedMessages.erase(foundMessage);
                        cout << "Mesaj başarıyla silindi." << endl;
                    }
                    else
                    {
                        cout << "Mesaj sahibi uyuşmuyor, silinemedi." << endl;
                    }
                }
                else
                {
                    cout << "Silinecek mesaj bulunamadı!" << endl;
                }
            }
            lock_guard<mutex> lock(messagesMutex);
            break;
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
