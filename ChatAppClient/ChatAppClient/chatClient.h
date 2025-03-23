#pragma once
#include <string>
#include <vector>


class chatClient
{
public:
	chatClient() = default;

	bool ConnectToServer();
	void DisconnectToServer();

	void SendMessageToServer(std::string message, const char message_buffer[]);
	void EraseMessage(std::string message);


	void SetUserName(const std::string new_user_name) { user_name = new_user_name; }
	bool GetClientConnectedToServer() const { return connected_to_server; }
	std::vector<std::string> GetClientMessages() const { return messages; }

protected:
	void ReceiveMessages();

private:
	void CreateReceiveChannel();
	std::string ModifyMessage(const std::string& receivedMessage, const std::string& userName);

	std::string user_name;
	bool connected_to_server = false;
	std::vector<std::string> messages;
};