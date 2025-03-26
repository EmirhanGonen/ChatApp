#pragma once
#include <string>
#include <cstring>

#pragma pack(push, 1)
enum class MessageType : uint8_t {
    SendMessagePackage = 0,
    EraseMessagePackage = 1
};

struct MessagePackage {
    MessageType m_MessageType;
    char m_MessageOwner[64];
    char m_PackageOwner[64];
    char message[1024];

    MessagePackage() {
        memset(this, 0, sizeof(MessagePackage));
        m_MessageType = MessageType::SendMessagePackage;
    }

    MessagePackage(MessageType type, const std::string& msg, 
                   const std::string& messageOwner, const std::string& packageOwner) {
        memset(this, 0, sizeof(MessagePackage));
        m_MessageType = type;
        
        strncpy_s(m_MessageOwner, messageOwner.c_str(), sizeof(m_MessageOwner)-1);
        strncpy_s(m_PackageOwner, packageOwner.c_str(), sizeof(m_PackageOwner)-1);
        strncpy_s(message, msg.c_str(), sizeof(message)-1);
        
        m_MessageOwner[sizeof(m_MessageOwner)-1] = '\0';
        m_PackageOwner[sizeof(m_PackageOwner)-1] = '\0';
        message[sizeof(message)-1] = '\0';
    }
};
#pragma pack(pop)

static_assert(sizeof(MessagePackage) == 1153, "MessagePackage boyutu hatalı!");