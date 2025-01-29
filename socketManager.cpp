#include "socketManager.hpp"
#include "application.hpp"
#include "json.hpp"
using json = nlohmann::json;

std::string stripAfterJson(std::string &input)
{
  int braceCount = 0;
  size_t lastValidPos = std::string::npos;

  for (size_t i = 0; i < input.size(); ++i)
  {
    if (input[i] == '{')
    {
      braceCount++;
    }
    else if (input[i] == '}')
    {
      braceCount--;
      if (braceCount == 0)
      {
        lastValidPos = i;
        break;
      }
    }
  }

  if (lastValidPos != std::string::npos)
  {
    input = input.substr(0, lastValidPos + 1);
  }

  return input;
}

void SocketManager::deserialize(const std::string &data)
{
  std::lock_guard<std::mutex> lock(playersMutex);

  try
  {
    std::string copiedData = data;
    stripAfterJson(copiedData);
    json receivedData = json::parse(copiedData);
    int broadcastType = receivedData["type"];

    if (broadcastType == 0)
    {
      int id = receivedData["server-id"];
      glm::vec3 position(receivedData["position"][0], receivedData["position"][1], receivedData["position"][2]);

      if (networkedPlayers.find(id) == networkedPlayers.end())
      {
        PlayerData player;
        player.id = app->addPlayer();
        player.position = position;

        networkedPlayers[id] = player;
      }
      else
      {
        networkedPlayers[id].position = position;
        app->editPlayer(networkedPlayers[id]);

        /*std::cout << "Updated player ID: " << networkedPlayers[id].id << " Position: ("
                  << networkedPlayers[id].position.x << ", "
                  << networkedPlayers[id].position.y << ", "
                  << networkedPlayers[id].position.z << ")" << std::endl;*/
      }
    }
    else if (broadcastType == 1)
    {
      int id = receivedData["server-id"];
      if (networkedPlayers.find(id) != networkedPlayers.end())
      {
        app->removePlayer(networkedPlayers[id].id);
        networkedPlayers.erase(id);
      }
      else
      {
        std::cerr << "Error: Attempted to remove non-existent player with ID " << id << std::endl;
      }
    }
    else if (broadcastType == 2)
    {
      int id = receivedData["server-id"];
      if (id == -1)
      {
        app->youAreTagged();
      }
      else
      {
        if (networkedPlayers.find(id) != networkedPlayers.end())
        {
          app->setTagged(networkedPlayers[id].id);
        }
        else
        {
          std::cerr << "Tagged player does not exist: " << id << std::endl;
        }
      }
    }
    else
    {
      std::cerr << "Incorrect Broadcast Type" << std::endl;
    }
  }
  catch (const json::parse_error &e)
  {
    std::cerr << "JSON Parsing Error: " << e.what() << std::endl;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Deserialization Error: " << e.what() << std::endl;
  }
}

void SocketManager::broadcast(glm::vec3 pos)
{
  json message;
  message["position"] = {pos.x, pos.y, pos.z};

  std::string formattedPos = message.dump();

  send(client, formattedPos.c_str(), formattedPos.size(), 0);
}
