#pragma once
#include <GLFW/glfw3.h>
#include "chatClient.h"

class clientWindow {
public:
	clientWindow() = default;
	GLFWwindow* Init(chatClient* chat_client);
	void Shutdown();
	void StartTick();
	void Tick();
	void EndTick();

private:
	chatClient* chat_client = nullptr;
	char nick_name_buffer[256] = { 0 };
	bool nickname_changed = false;
	bool will_connect = false;
};