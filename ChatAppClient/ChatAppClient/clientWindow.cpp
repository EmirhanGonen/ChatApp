#include "clientWindow.h"

#include <iostream>
#include <ostream>

#include "chatClient.h"
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"
#include <string>
#include <vector>

using namespace std;

GLFWwindow* client_window;
char client_message_buffer[256] = { 0 };


GLFWwindow* clientWindow::Init(class chatClient* chat_client)
{
	if (!glfwInit())
		return NULL;

	client_window = glfwCreateWindow(640, 480, "ChatApp", NULL, NULL);
	if (!client_window) {
		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(client_window);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(client_window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	this->chat_client = chat_client;

	return client_window;
}

void clientWindow::StartTick()
{
	glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void clientWindow::Tick()
{

	ImGui::Begin("Chat App");

	if (ImGui::Checkbox("Connect", &will_connect))
	{
		if (will_connect && !chat_client->GetClientConnectedToServer()) {
			chat_client->ConnectToServer();
		}
		else if (chat_client->GetClientConnectedToServer())
		{
			chat_client->DisconnectToServer();
		}
	}

	ImGui::SameLine();

	bool client_connected = chat_client->GetClientConnectedToServer();
	ImVec4 color = client_connected ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	const char* text = client_connected ? "Connected" : "Disconnected";
	ImGui::TextColored(color, "%s", text);

	if (!client_connected) {
		ImGui::End();
		return;
	}

	if (!nickname_changed) {
		ImGui::SetNextItemWidth(150.00f);

		if (ImGui::InputText("NickName", nick_name_buffer, sizeof(nick_name_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
			chat_client->SetUserName(string(nick_name_buffer));
			nickname_changed = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Change")) {
			chat_client->SetUserName(string(nick_name_buffer));
			nickname_changed = true;
		}
		ImGui::End();
		return;
	}
	for (std::string& message : chat_client->GetClientMessages()) {
		if (ImGui::Selectable(message.c_str()))
		{
			chat_client->EraseMessage(message);
		}
	}

	if (ImGui::InputText("Message", client_message_buffer, sizeof(client_message_buffer), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		string message(client_message_buffer);
		chat_client->SendMessageToServer(message, client_message_buffer);

		memset(client_message_buffer, 0, sizeof(client_message_buffer));
	}
	ImGui::End();
}

void clientWindow::EndTick()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


	//getline(cin, message);
	//send(clientSocket, message.c_str(), message.length(), 0);

	glfwSwapBuffers(client_window);
	glfwPollEvents();
}

void clientWindow::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}
