#pragma once
#include <GLFW/glfw3.h>

class clientWindow
{
public:
	clientWindow() {};

	GLFWwindow* Init(class chatClient* chat_client);
	void Shutdown();

	void StartTick();
	void Tick();
	void EndTick();

private:
	chatClient* chat_client;
	char nick_name_buffer[256] = { 0 };

	bool nickname_changed = false;
	bool will_connect = false;
};