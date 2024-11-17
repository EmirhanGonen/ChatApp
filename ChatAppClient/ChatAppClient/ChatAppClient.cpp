#include <thread>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h> 

#pragma comment(lib, "ws2_32.lib")

using namespace std;

SOCKET clientSocket;

void ReceiveMessages() {
	char buffer[1024];
	int recvSize;

	while (true)
	{
		recvSize = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (recvSize > 0) {
			buffer[recvSize] = '\0';
			cout << "Mesaj: " << buffer << endl;
		}
	}
}

int main()
{
	WSADATA wsaData;
	sockaddr_in serverAddr;

	string local_ip = "127.0.0.1";
	int port = 8817;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "Winsock start is failed" << endl;
		return -1;
	}

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "Socket cant be created" << endl;
		WSACleanup();
		return -1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	if (inet_pton(AF_INET, local_ip.c_str(), &serverAddr.sin_addr) <= 0)
	{
		cout << "Invalid IP Adress" << endl;
		closesocket(clientSocket);
		WSACleanup();
		return -1;
	}

	if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "Connection failed" << endl;
		closesocket(clientSocket);
		WSACleanup();
		return -1;
	}

	cout << "Connected to the server. You can send a message" << endl;

	thread receiveThread(ReceiveMessages);
	receiveThread.detach();

	string message;
	while (true)
	{
		getline(cin, message);
		send(clientSocket, message.c_str(), message.length(), 0);
	}

	closesocket(clientSocket);
	WSACleanup();
	return 0;
}