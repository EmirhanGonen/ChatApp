#include <string>
#include <cstring>

enum MessageType : uint8_t
{
    SendMessage = 0,
    EraseMessage = 1
};

struct MessagePackage
{
    MessageType m_MessageType;
    char message[1024];

    MessagePackage()
    {
        m_MessageType = MessageType::SendMessage;
        memset(message, 0, sizeof(message));
    }

    MessagePackage(MessageType type, const std::string& msg)
    {
        m_MessageType = type;
        memset(message, 0, sizeof(message));
        strncpy(message, msg.c_str(), sizeof(message) - 1);
    }
};
