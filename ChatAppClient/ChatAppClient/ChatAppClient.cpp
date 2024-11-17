#include <vector>
#include <thread>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h> 
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

SOCKET clientSocket;
vector<string> messages;
string user_name;

string ModifyMessage(const string& receivedMessage, const string& userName) {
	size_t colonPos = receivedMessage.find(":"); // ":" karakterinin yerini bul
	if (colonPos != string::npos) {
		string sender = receivedMessage.substr(0, colonPos); // ":" öncesindeki kullanýcý adý
		string content = receivedMessage.substr(colonPos + 1); // ":" sonrasýndaki mesaj

		if (sender == userName) {
			return "You(" + user_name + "): " + content; // Eðer gönderen kullanýcý iseniz "You" olarak deðiþtirin
		}
		else {
			return sender + ":" + content; // Diðer kullanýcýlarýn mesajýný olduðu gibi býrak
		}
	}
	return receivedMessage; // ":" yoksa mesajý deðiþtirmeden döndür
}


void ReceiveMessages() {
	char buffer[1024];
	int recvSize;

	while (true)
	{
		recvSize = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (recvSize > 0) {
			buffer[recvSize] = '\0';
			messages.push_back(ModifyMessage(buffer, user_name));
			//messages.push_back(buffer);
			cout << "Mesaj: " << buffer << endl;
		}
	}
}

int main()
{
	cout << "Enter Your Username: ";
	getline(cin, user_name);

	GLFWwindow* window;
	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(640, 480, "ChatApp", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

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

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	char messageBuffer[256] = { 0 };

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Chat App");
		for (string& message : messages) {
			ImGui::Text(message.c_str());
		}

		if (ImGui::InputText("Message", messageBuffer, sizeof(messageBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			// Kullanýcý "Enter" tuþuna bastýðýnda mesajý gönder
			string message(messageBuffer);
			string messagewithusername(user_name + ": " + message);
			if (!message.empty())
			{
				send(clientSocket, messagewithusername.c_str(), messagewithusername.length(), 0);
				memset(messageBuffer, 0, sizeof(messageBuffer)); // Mesaj buffer'ýný temizle
			}
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		//getline(cin, message);
		//send(clientSocket, message.c_str(), message.length(), 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}