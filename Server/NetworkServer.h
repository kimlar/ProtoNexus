#pragma once

#include <string>
#include <vector>
#include <SFML/Network.hpp>
#include "Game.h"

class NetworkServer
{
public:
	NetworkServer(Game& game) : game(game) {}
	~NetworkServer() {}

	bool Start();
	void Update();
	void CleanUp();

private:
	bool CheckForIncomingConnection();
	void ReceiveMessageFromClients();
	void SendMessageToClients();
	
	void ClientToRemove(int clientID);
	void RemoveAllDisconnectedClients();
	
	void NetworkTx(int clientID, std::string data);
	std::string NetworkRx(int clientID);

	const unsigned short port = 2000;
	sf::TcpListener listener;
	sf::SocketSelector selector;
	std::vector<sf::TcpSocket*>clients;
	std::vector<int>clientsToRemove;

	///////////////////////////////////////////////////////////////////////////////////////
	void PrintPlayerList();
	std::string GetPlayerNameBySlotId(int slotId);
	int GetSlotIdByPlayerName(std::string playerName);
	std::vector<std::string> vPlayerName;
	///////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////
	Game& game;
	///////////////////////////////////////////////////////////////////////////////////////
};
