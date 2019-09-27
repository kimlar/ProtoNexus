#include "NetworkServer.h"

//#include <iostream>
//#include <SFML/Network.hpp>
#include "PrintUtils.h"

/////////////////////////////////////////////////////////////////////////////////////////
//#define USE_MSG_NETWORK

#ifdef USE_MSG_NETWORK
#define MSG_NETWORK(x) std::cout << x << std::endl
#else
#define MSG_NETWORK(x)
#endif
/////////////////////////////////////////////////////////////////////////////////////////

bool NetworkServer::Start()
{
	/////////////////////////////////////////////////////////////////////////////////////////
	if (listener.listen(port) != sf::Socket::Done)
	{
		ERROR("Server could not listen to port " << std::to_string(port));
		return false; // Failed!
	}
	selector.add(listener);

	//std::cout << "Server Running" << std::endl;
	PRINT("Server Running");
	/////////////////////////////////////////////////////////////////////////////////////////
	return true; // Success!
}

void NetworkServer::Update()
{
	if (selector.wait())
	{
		if (CheckForIncomingConnection())
			return;

		ReceiveMessageFromClients();
		SendMessageToClients();
	}
}

void NetworkServer::CleanUp()
{
	/////////////////////////////////////////////////////////////////////////////////////////
	for (std::vector<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); it++)
		delete *it;
	/////////////////////////////////////////////////////////////////////////////////////////
}


bool NetworkServer::CheckForIncomingConnection()
{
	if (selector.isReady(listener))
	{
		sf::TcpSocket *socket = new sf::TcpSocket;

		if (listener.accept(*socket) == sf::Socket::Done)
		{
			sf::Packet packet;
			std::string userID;
			std::string status;
			if (socket->receive(packet) == sf::Socket::Done)
				packet >> userID >> status;
			//std::cout << userID << " has " << status << std::endl;
			MSG_NETWORK("New connection: " << userID << " has " << status);
			
			clients.push_back(socket);
			selector.add(*socket);
			
			vPlayerName.push_back(userID);
			PrintPlayerList();

			//std::cout << "New connection: " << userID << std::endl;
			//PRINT("New connection: " << userID);

			return true; // There was a new incoming connection!
		}
		else
		{
			// Error, we could not create a connection. Deleting the socket
			delete socket;

			//std::cout << "NetworkServer::CheckForIncomingConnection(): Error: We could not create a connection" << std::endl;
			ERROR("NetworkServer::CheckForIncomingConnection(): Error: We could not create a connection");
		}
	}

	return false; // No incoming connection!
}

void NetworkServer::ReceiveMessageFromClients()
{
	std::vector<std::string> vReceived;

	for (int i = 0; i < clients.size(); i++)
	{
		if (selector.isReady(*clients[i]))
		{
			std::string received = NetworkRx(i);
			if (received == "") // Did not actually receive something.
				continue;
			//std::cout << "NetworkServer::ReceiveMessageFromClients(): [" << i << "]: #" << received << "#" << std::endl;
			MSG_NETWORK("NetworkServer::ReceiveMessageFromClients(): [" << i << "]: #" << received << "#");
			received = GetPlayerNameBySlotId(i) + ": " + received;
			vReceived.push_back(received);
		}
	}

	game.SetReadMessages(vReceived);
}

void NetworkServer::SendMessageToClients()
{
	//std::string testData = "new position";
	//std::string testData = std::to_string(game.GetTestValue());

	std::vector<std::string> vSendMessages = game.GetSendMessages();

	for (int m = 0; m < vSendMessages.size(); m++)
	{
		for (int i = 0; i < clients.size(); i++)
		{
			//if (testData == "") // Don't have anything to send.
			//	continue;
			//NetworkTx(i, testData);
			//std::cout << "NetworkServer::SendMessageToClients(): [" << i << "]: " << testData << std::endl;
			//MSG_NETWORK("NetworkServer::SendMessageToClients(): [" << i << "]: " << testData);

			if (vSendMessages[m] == "") // Don't have anything to send.
				continue;
			NetworkTx(i, vSendMessages[m]);
			MSG_NETWORK("NetworkServer::SendMessageToClients(): [" << i << "]: " << vSendMessages[m]);
		}
	}
}

void NetworkServer::ClientToRemove(int clientID)
{
	clientsToRemove.push_back(clientID);
	std::string message;
	message = "Player [";
	message += vPlayerName[clientID];
	message += "] has disconnected";

	// Broadcast to all except the client that has disconnected
	for (int j = 0; j < clients.size(); j++)
	{
		if (clientID != j)
			NetworkTx(j, message);
	}

	RemoveAllDisconnectedClients();
}

void NetworkServer::RemoveAllDisconnectedClients()
{
	if (clientsToRemove.empty())
		return;

	//std::cout << "About to disconnect clients" << std::endl;
	VERBOSE("About to disconnect clients");

	for (int cid = (int)clientsToRemove.size() - 1; cid >= 0; cid--)
	{
		selector.remove(*clients[clientsToRemove[cid]]);

		clients[clientsToRemove[cid]]->disconnect();
		clients.erase(clients.begin() + clientsToRemove[cid]);
		//std::cout << "Client ID " << clientsToRemove[cid] << " has disconnected" << std::endl;
		MSG_NETWORK("Client ID " << clientsToRemove[cid] << " has disconnected");

		// Update player list
		vPlayerName.erase(vPlayerName.begin() + clientsToRemove[cid]);
		PrintPlayerList();
	}
	clientsToRemove.clear();
}

void NetworkServer::NetworkTx(int clientID, std::string data)
{
	// Check
	if (clientID >= clients.size()) // TODO: Check if this is correct!
	{
		//std::cout << "ERROR: NetworkServer::NetworkTx(...): Client ID does not exist" << std::endl;
		ERROR("NetworkServer::NetworkTx(...): Client ID does not exist");
		return; // Failure;
	}

	sf::Packet packet;
	packet << data;
	sf::Socket::Status status = clients[clientID]->send(packet);
	switch (status)
	{
	case sf::Socket::Done:
		//std::cout << "DEBUG: NetworkServer::NetworkTx(...): sf::Socket::Done" << std::endl;
		break;
	case sf::Socket::NotReady:
		//std::cout << "DEBUG: NetworkServer::NetworkTx(...): sf::Socket::NotReady" << std::endl;
		MSG_NETWORK("NetworkServer::NetworkTx(...): sf::Socket::NotReady");
		break;
	case sf::Socket::Partial:
		//std::cout << "DEBUG: NetworkServer::NetworkTx(...): sf::Socket::Partial" << std::endl;
		MSG_NETWORK("NetworkServer::NetworkTx(...): sf::Socket::Partial");
		while (status != sf::Socket::Done)
		{
			status = clients[clientID]->send(packet);
			if (status == sf::Socket::Disconnected)
			{
				//std::cout << "NetworkServer::NetworkTx(...): Client has disconnected" << std::endl;
				MSG_NETWORK("NetworkServer::NetworkTx(...): Client has disconnected");
				ClientToRemove(clientID);
				break;
			}
			else if (status == sf::Socket::Error)
			{
				//std::cout << "ERROR: NetworkServer::NetworkTx(...): An unknown error occurred" << std::endl;
				ERROR("NetworkServer::NetworkTx(...): An unknown error occurred");
				break;
			}
		}
		break;
	case sf::Socket::Disconnected:
		//std::cout << "DEBUG: NetworkServer::NetworkTx(...): sf::Socket::Disconnected" << std::endl;
		//std::cout << "NetworkServer::NetworkTx(...): Client has disconnected" << std::endl;
		MSG_NETWORK("NetworkServer::NetworkTx(...): Client has disconnected");
		ClientToRemove(clientID);
		break;
	case sf::Socket::Error:
		//std::cout << "DEBUG: NetworkServer::NetworkTx(...): sf::Socket::Error" << std::endl;
		//std::cout << "ERROR: NetworkServer::NetworkTx(...): An unknown error occurred" << std::endl;
		ERROR("NetworkServer::NetworkTx(...): An unknown error occurred");
		break;
	}

	return;
}

std::string NetworkServer::NetworkRx(int clientID)
{
	std::string result = "";

	// Check
	if (clientID >= clients.size()) // TODO: Check if this is correct!
	{
		//std::cout << "ERROR: NetworkServer::NetworkRx(...): Client ID does not exist" << std::endl;
		ERROR("NetworkServer::NetworkRx(...): Client ID does not exist");
		return ""; // Failure;
	}

	sf::Packet packet;
	sf::Socket::Status status = clients[clientID]->receive(packet);
	switch (status)
	{
	case sf::Socket::Done:
		//std::cout << "DEBUG: NetworkServer::NetworkRx(...): sf::Socket::Done" << std::endl;
		packet >> result;
		break;
	case sf::Socket::NotReady:
		//std::cout << "DEBUG: NetworkServer::NetworkRx(...): sf::Socket::NotReady" << std::endl;
		MSG_NETWORK("NetworkServer::NetworkRx(...): sf::Socket::NotReady");
		break;
	case sf::Socket::Partial:
		//std::cout << "DEBUG: NetworkServer::NetworkRx(...): sf::Socket::Partial" << std::endl;
		MSG_NETWORK("NetworkServer::NetworkRx(...): sf::Socket::Partial");
		break;
	case sf::Socket::Disconnected:
		//std::cout << "DEBUG: NetworkServer::NetworkRx(...): sf::Socket::Disconnected" << std::endl;
		//std::cout << "NetworkServer::NetworkRx(...): Client has disconnected" << std::endl;
		MSG_NETWORK("NetworkServer::NetworkRx(...): Client has disconnected");
		ClientToRemove(clientID);
		break;
	case sf::Socket::Error:
		//std::cout << "DEBUG: NetworkServer::NetworkRx(...): sf::Socket::Error" << std::endl;
		//std::cout << "ERROR: NetworkServer::NetworkRx(...): An unknown error occurred" << std::endl;
		MSG_NETWORK("NetworkServer::NetworkRx(...): An unknown error occurred");
		break;
	}

	return result;
}

void NetworkServer::PrintPlayerList()
{
	/*
	std::cout << "=============================================" << std::endl;
	std::cout << "Player list:" << std::endl;
	for (int v = 0; v < (int)vPlayerName.size(); v++)
	{
		std::cout << "vPlayerName[" << v << "] = " << vPlayerName[v] << std::endl;
	}
	std::cout << "=============================================" << std::endl;
	*/

	MSG_NETWORK("=============================================");
	MSG_NETWORK("Player list:");
	for (int v = 0; v < (int)vPlayerName.size(); v++)
	{
		MSG_NETWORK("vPlayerName[" << v << "] = " << vPlayerName[v]);
	}
	MSG_NETWORK("=============================================");
}

std::string NetworkServer::GetPlayerNameBySlotId(int slotId)
{
	if (vPlayerName.empty())
		return nullptr; // Did not find any player on that slot id

	return vPlayerName[slotId];
}

int NetworkServer::GetSlotIdByPlayerName(std::string playerName)
{
	for (int i = 0; i < (int)vPlayerName.size() - 1; i++)
	{
		if (vPlayerName[i] == playerName)
			return i;
	}

	return -1; // Did not find player with that name
}
