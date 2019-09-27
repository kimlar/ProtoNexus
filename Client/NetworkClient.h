#pragma once

#include <SFML/Network.hpp>
#include "Game.h"

class NetworkClient
{
public:
	NetworkClient(Game& game) : game(game) {}
	~NetworkClient() {}

	bool Start();
	bool Update();
	void CleanUp();

private:
	bool Connect(std::string ipToServer, std::string userID);
	void Disconnect();

	void SendMessageToServer();

	void NetworkTx(std::string data);
	std::string NetworkRx();
		
	bool run;
	const unsigned short port = 2000;
	sf::TcpSocket socket;
	std::string userID;
	sf::SocketSelector selector;
	sf::Time serverTimeout = sf::milliseconds(5000);

	///////////////////////////////////////////////////////////////////////////////////////
	Game& game;
	///////////////////////////////////////////////////////////////////////////////////////
};
