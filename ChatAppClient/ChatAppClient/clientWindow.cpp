#include "clientWindow.h"
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

GLFWwindow* clientWindow::Init(chatClient* chat_client) {
    if (!glfwInit()) return nullptr;

    GLFWwindow* window = glfwCreateWindow(640, 480, "ChatApp", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    this->chat_client = chat_client;
    return window;
}

void clientWindow::StartTick() {
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void clientWindow::Tick() {
    ImGui::Begin("Chat App");

    if (ImGui::Checkbox("Connect", &will_connect)) {
        if (will_connect && !chat_client->GetClientConnectedToServer()) {
            chat_client->ConnectToServer();
        } else if (!will_connect && chat_client->GetClientConnectedToServer()) {
            chat_client->DisconnectToServer();
        }
    }

    ImGui::SameLine();
    bool connected = chat_client->GetClientConnectedToServer();
    ImVec4 color = connected ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1);
    ImGui::TextColored(color, connected ? "Connected" : "Disconnected");

    if (!connected) {
        ImGui::End();
        return;
    }

    if (!nickname_changed) {
        ImGui::InputText("User Name", nick_name_buffer, sizeof(nick_name_buffer), ImGuiInputTextFlags_EnterReturnsTrue);
        if (ImGui::Button("Apply") || ImGui::IsItemActive()) {
            chat_client->SetUserName(nick_name_buffer);
            nickname_changed = true;
        }
        ImGui::End();
        return;
    }

    for (const auto& msg : chat_client->GetClientMessages()) {
        if (ImGui::Selectable(msg.c_str())) {
            chat_client->EraseMessage(msg);
        }
    }

    static char messageBuffer[1024] = {0};
    if (ImGui::InputText("Message", messageBuffer, sizeof(messageBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        chat_client->SendMessageToServer(MessageType::SendMessagePackage, messageBuffer);
        memset(messageBuffer, 0, sizeof(messageBuffer));
    }

    ImGui::End();
}

void clientWindow::EndTick() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(glfwGetCurrentContext());
    glfwPollEvents();
}

void clientWindow::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}