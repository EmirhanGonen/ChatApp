#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "MessagePackage.h"

class chatClient {
public:
	chatClient() = default;
	bool ConnectToServer();
	void DisconnectToServer();
	void SendMessageToServer(MessageType messageType, std::string message);
	void EraseMessage(const std::string& formattedMessage);
	void SetUserName(const std::string& new_user_name) { user_name = new_user_name; }
	bool GetClientConnectedToServer() const { return connected_to_server; }
	std::vector<std::string> GetClientMessages() const;

private:
	void ReceiveMessages();
	void CreateReceiveChannel();
	std::vector<MessagePackage> receivedMessages;
	std::string user_name;
	bool connected_to_server = false;
	mutable std::mutex messagesMutex;
	std::vector<std::string> messages;
};