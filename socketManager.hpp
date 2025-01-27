#pragma once
#include <vector>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include "socketValues.hpp"
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <mutex>

struct PlayerData
{
  int id;
  glm::vec3 position;
};

class Application;
class SocketManager
{
public:
  std::unordered_map<int, PlayerData> networkedPlayers;
  std::mutex playersMutex;

  SocketManager(Application *app) : app(app)
  {
  }
  ~SocketManager()
  {
  }

  void init()
  {
    if (WSAStartup(MAKEWORD(2, 2), &winsockData) != 0)
    {
      std::cerr << "Cannot init winsock." << std::endl;
      std::cin.get();
      return;
    }

    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET)
    {
      std::cerr << "Socket is invalid." << std::endl;
      std::cin.get();
      return;
    }

    addr.sin_port = htons(PORT);
    addr.sin_family = AF_INET;

    if (inet_pton(AF_INET, SERVER_IP, &addr.sin_addr) <= 0)
    {
      std::cerr << "Invalid address/Address not supported.\n";
      closesocket(client);
      WSACleanup();
      return;
    }

    std::cerr << "Connecting..." << std::endl;

    if (connect(client, (sockaddr *)&addr, sizeof(addr)) != 0)
    {
      std::cerr << "Cannot connect to server." << std::endl;
      std::cin.get();
      return;
    }
  }
  void cleanup()
  {
    stopReceiving();

    closesocket(client);
    WSACleanup();
  }

  void startReceiving()
  {
    recvThread = std::thread(&SocketManager::receiveMessages, this, client);
  }

  void receiveMessages(SOCKET clientSocket)
  {
    char buffer[BUFFER_SIZE];
    while (true)
    {
      int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
      if (bytesReceived > 0)
      {
        std::cout << "Received position: " << buffer << std::endl;

        deserialize(buffer);
      }
      else if (bytesReceived == 0)
      {
        std::cerr << "\nConnection closed by the server.\n";
        break;
      }
      else
      {
        std::cerr << "\nError receiving data.\n";
        break;
      }
    }
  }

  void broadcast(glm::vec3 pos);

private:
  Application *app;
  WSADATA winsockData;
  SOCKET client;
  std::thread recvThread;
  struct sockaddr_in addr;

  void deserialize(const std::string &data);

  void stopReceiving()
  {
    if (recvThread.joinable())
    {
      recvThread.join();
    }
  }
};