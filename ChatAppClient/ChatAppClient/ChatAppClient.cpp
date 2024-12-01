#include "chatClient.h"
#include "clientWindow.h"
#include <GLFW/glfw3.h>

clientWindow* client_window;
chatClient* chat_client;

GLFWwindow* window;

int main()
{
	client_window = new clientWindow();
	chat_client = new chatClient();

	window = client_window->Init(chat_client);

	while (!glfwWindowShouldClose(window))
	{
		client_window->StartTick();
		client_window->Tick();
		client_window->EndTick();
	}

	client_window->Shutdown();
	chat_client->DisconnectToServer();

	delete client_window;
	delete chat_client;

	return 0;
}