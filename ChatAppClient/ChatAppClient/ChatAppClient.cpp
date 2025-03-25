#include "chatClient.h"
#include "clientWindow.h"
#include <GLFW/glfw3.h>

int main() {
	clientWindow window;
	chatClient client;

	GLFWwindow* glfwWindow = window.Init(&client);
	if (!glfwWindow) return -1;

	while (!glfwWindowShouldClose(glfwWindow)) {
		window.StartTick();
		window.Tick();
		window.EndTick();
	}

	window.Shutdown();
	client.DisconnectToServer();
	return 0;
}