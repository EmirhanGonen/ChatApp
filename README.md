# ChatApp

A lightweight, multi-client TCP chat application written in **C++** for Windows, featuring a real-time GUI client built with **Dear ImGui** and a multithreaded broadcast server built on **WinSock2**.

The repository contains two separate Visual Studio solutions:

```
ChatApp/
├── ChatAppServer/   # Console-based TCP broadcast server
└── ChatAppClient/   # GUI client (Dear ImGui + GLFW + OpenGL 3)
```

---

## Features

- **Multi-client support** — the server accepts unlimited concurrent connections, spawning a dedicated thread per client.
- **Real-time message broadcasting** — every message received is relayed to all connected clients.
- **Message deletion** — click any of your own messages in the client UI to delete it for everyone. Ownership is validated on the client, so you can only erase your own messages.
- **Custom binary protocol** — messages are exchanged as fixed-size, tightly packed structs (`#pragma pack(1)`), verified at compile time with `static_assert`.
- **Reliable transmission** — both send and receive paths loop until the full package is transferred, correctly handling partial TCP reads/writes.
- **Thread-safe state** — client lists (server) and message history (client) are guarded with `std::mutex` / `std::lock_guard`.
- **Immediate-mode GUI** — connect/disconnect toggle with live status indicator, nickname entry, scrollable message list, and an Enter-to-send input field.

---

## Architecture

### Wire Protocol — `MessagePackage`

Both sides share an identical `MessagePackage.h` defining the packet layout:

| Field            | Type                | Size       | Purpose                          |
|------------------|---------------------|------------|----------------------------------|
| `m_MessageType`  | `MessageType : uint8_t` | 1 byte  | `SendMessagePackage` / `EraseMessagePackage` |
| `m_MessageOwner` | `char[64]`          | 64 bytes   | Original author of the message   |
| `m_PackageOwner` | `char[64]`          | 64 bytes   | Sender of this packet            |
| `message`        | `char[1024]`        | 1024 bytes | Message content                  |

Total size: **1153 bytes**, enforced at compile time:

```cpp
static_assert(sizeof(MessagePackage) == 1153, "MessagePackage size mismatch!");
```

Fixed-size packets keep framing trivial — no length prefixes or delimiters are needed; each `recv` loop simply reads exactly `sizeof(MessagePackage)` bytes.

### Server (`ChatAppServer`)

A console application that:

1. Initializes WinSock (`WSAStartup`), binds to **port 8817** on `INADDR_ANY`, and listens with a backlog of 10.
2. Accepts clients in the main loop and stores their sockets in a `std::vector<SOCKET>` protected by a mutex.
3. Detaches a `HandleClient` thread per connection, which reads complete packages and calls `BroadcastMessage`.
4. On send/receive failure, the client socket is removed from the list and closed — dead connections are cleaned up automatically.

The server is intentionally stateless regarding message content: it does not interpret `SendMessagePackage` vs `EraseMessagePackage`, it simply relays every package to all clients. All erase logic lives on the client side.

### Client (`ChatAppClient`)

Split into three responsibilities:

- **`chatClient`** — networking layer. Connects to `127.0.0.1:8817`, runs a detached receive thread, and maintains a mutex-guarded message history. Incoming `SendMessagePackage` packets are formatted (`You(name): msg` for own messages, `name: msg` for others); incoming `EraseMessagePackage` packets remove the matching message from both the display list and the raw package history.
- **`clientWindow`** — presentation layer. Owns the GLFW window and ImGui frame lifecycle (`StartTick` / `Tick` / `EndTick` / `Shutdown`). The UI flow is gated: connect first, then choose a nickname, then chat. Clicking a message via `ImGui::Selectable` triggers the erase request.
- **`main`** — wires the two together and runs the render loop until the window closes.

---

## Requirements

- **OS:** Windows (uses WinSock2)
- **IDE:** Visual Studio 2022 (v143 toolset)
- **Client dependencies:**
  - [GLFW](https://www.glfw.org/) (window/context management)
  - OpenGL 3.3
  - [Dear ImGui](https://github.com/ocornut/imgui) — bundled in `ChatAppClient/imgui/` with GLFW + OpenGL3 backends
- **Server dependencies:** none beyond the Windows SDK (`ws2_32.lib` is linked via `#pragma comment`)

---

## Building & Running

1. **Server**
   - Open `ChatAppServer/ChatAppServer.sln` in Visual Studio.
   - Build and run (x64). You should see `Server is running...` in the console.

2. **Client**
   - Open `ChatAppClient/ChatAppClient.sln` in Visual Studio.
   - Make sure GLFW include/lib paths are configured for your machine (ImGui is already vendored in the repo).
   - Build and run (x64). Launch multiple instances to chat between clients.

3. **Usage**
   - Tick the **Connect** checkbox — the status indicator turns green when connected.
   - Enter a nickname and press **Enter**.
   - Type messages and press **Enter** to send.
   - Click one of **your own** messages to delete it for all participants.

> The client connects to `127.0.0.1` by default. To chat across machines, change the IP in `chatClient::ConnectToServer` and rebuild.

---

## Possible Improvements

- Configurable server IP/port from the UI instead of hardcoded values
- Server-side validation of erase requests (currently trust-based on the client)
- Message timestamps and unique message IDs (erase currently matches by content + owner)
- Graceful client disconnect notification broadcast to other users
- Cross-platform sockets (BSD sockets / `asio`) to drop the Windows-only constraint

---

## License

No license specified yet — feel free to open an issue if you'd like to use this project.
