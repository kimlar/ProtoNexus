#include "NetworkClient.h"

//#include <iostream>
#include "PrintUtils.h"

/////////////////////////////////////////////////////////////////////////////////////////
//#define USE_MSG_NETWORK

#ifdef USE_MSG_NETWORK
#define MSG_NETWORK(x) std::cout << x << std::endl
#else
#define MSG_NETWORK(x)
#endif
/////////////////////////////////////////////////////////////////////////////////////////


bool NetworkClient::Start()
{
	//std::cout << "Servers IP: ";
	std::string ipToServer = "127.0.0.1";

	std::cout << "User: ";
	std::cin >> userID;
	
	return Connect(ipToServer, userID);
}

bool NetworkClient::Update()
{
	std::vector<std::string> vReceived;
	std::string received;

	sf::Time serverWait = sf::milliseconds(1);
	while (selector.wait(serverWait))
	{
		if (selector.isReady(socket))
		{
			received = NetworkRx();
			if (received == "")
				continue;
			//std::cout << "NetworkClient::Update(): NetworkRx(): Received: " << received << std::endl;
			MSG_NETWORK("NetworkClient::Update(): NetworkRx(): Received: " << received);
			vReceived.push_back(received);
		}
	}
	game.SetReadMessages(vReceived);
	SendMessageToServer();

	return run;
}

void NetworkClient::CleanUp()
{
	Disconnect();
}

bool NetworkClient::Connect(std::string ipToServer, std::string userID)
{
	sf::IpAddress ipServer = sf::IpAddress(ipToServer);
	if (socket.connect(ipServer, port) != sf::Socket::Done)
	{
		ERROR("Could not connect to server: " << ipToServer << ":" << std::to_string(port));
		return false; // Failed!
	}

	sf::Packet packet;
	std::string status = "connected";
	packet << userID << status;
	if (socket.send(packet) != sf::Socket::Done)
	{
		ERROR("Could not send userID to server");
		return false; // Failed!
	}
	socket.setBlocking(false);

	selector.add(socket);

	run = true;

	PRINT("Connected to server: " << ipToServer << ":" << std::to_string(port));;

	return true; // Success!
}

void NetworkClient::Disconnect()
{
	//std::cout << "DEBUG: NetworkClient::Disconnect(): Disconnect from server" << std::endl;
	MSG_NETWORK("NetworkClient::Disconnect(): Disconnect from server");
	socket.disconnect();
	run = false;
}

void NetworkClient::SendMessageToServer()
{
	std::vector<std::string> vSendMessages = game.GetSendMessages();

	for (int m = 0; m < vSendMessages.size(); m++)
	{
		if (vSendMessages[m] == "") // Don't have anything to send.
			continue;
		NetworkTx(vSendMessages[m]);
		MSG_NETWORK("NetworkClient::SendMessageToServer(): " << vSendMessages[m]);
	}
}

void NetworkClient::NetworkTx(std::string data)
{
	sf::Packet packet;
	packet << data;
	sf::Socket::Status status = socket.send(packet);
	switch (status)
	{
	case sf::Socket::Done:
		//std::cout << "DEBUG: NetworkClient::NetworkTx(...): sf::Socket::Done" << std::endl;
		break;
	case sf::Socket::NotReady:
		//std::cout << "DEBUG: NetworkClient::NetworkTx(...): sf::Socket::NotReady" << std::endl;
		MSG_NETWORK("NetworkClient::NetworkTx(...): sf::Socket::NotReady");
		break;
	case sf::Socket::Partial:
		//std::cout << "DEBUG: NetworkClient::NetworkTx(...): sf::Socket::Partial" << std::endl;
		MSG_NETWORK("NetworkClient::NetworkTx(...): sf::Socket::Partial");
		while (status != sf::Socket::Done)
		{
			status = socket.send(packet);
			if (status == sf::Socket::Disconnected)
			{
				//std::cout << "NetworkClient::NetworkTx(...): Server has disconnected" << std::endl;
				MSG_NETWORK("NetworkClient::NetworkTx(...): Server has disconnected");
				Disconnect();
				break;
			}
			else if (status == sf::Socket::Error)
			{
				//std::cout << "ERROR: NetworkClient::NetworkTx(...): An unknown error occurred" << std::endl;
				ERROR("NetworkClient::NetworkTx(...): An unknown error occurred");
				break;
			}
		}
		break;
	case sf::Socket::Disconnected:
		//std::cout << "DEBUG: NetworkClient::NetworkTx(...): sf::Socket::Disconnected" << std::endl;
		//std::cout << "NetworkClient::NetworkTx(...): Server has disconnected" << std::endl;
		MSG_NETWORK("NetworkClient::NetworkTx(...): Server has disconnected");
		Disconnect();
		break;
	case sf::Socket::Error:
		//std::cout << "DEBUG: NetworkClient::NetworkTx(...): sf::Socket::Error" << std::endl;
		//std::cout << "ERROR: NetworkClient::NetworkTx(...): An unknown error occurred" << std::endl;
		ERROR("NetworkClient::NetworkTx(...): An unknown error occurred");
		break;
	}

	return;
}

std::string NetworkClient::NetworkRx()
{
	std::string result = "";

	sf::Packet packet;
	sf::Socket::Status status = socket.receive(packet);
	switch (status)
	{
	case sf::Socket::Done:
		//std::cout << "DEBUG: NetworkClient::NetworkRx(...): sf::Socket::Done" << std::endl;
		packet >> result;
		break;
	case sf::Socket::NotReady:
		//std::cout << "DEBUG: NetworkClient::NetworkRx(...): sf::Socket::NotReady" << std::endl;
		MSG_NETWORK("NetworkClient::NetworkRx(...): sf::Socket::NotReady");
		break;
	case sf::Socket::Partial:
		//std::cout << "DEBUG: NetworkClient::NetworkRx(...): sf::Socket::Partial" << std::endl;
		MSG_NETWORK("NetworkClient::NetworkRx(...): sf::Socket::Partial");
		break;
	case sf::Socket::Disconnected:
		//std::cout << "DEBUG: NetworkClient::NetworkRx(...): sf::Socket::Disconnected" << std::endl;
		//std::cout << "NetworkClient::NetworkRx(...): Server has disconnected" << std::endl;
		MSG_NETWORK("NetworkClient::NetworkRx(...): Server has disconnected");
		Disconnect();
		break;
	case sf::Socket::Error:
		//std::cout << "DEBUG: NetworkClient::NetworkRx(...): sf::Socket::Error" << std::endl;
		//std::cout << "ERROR: NetworkClient::NetworkRx(...): An unknown error occurred" << std::endl;
		ERROR("NetworkClient::NetworkRx(...): An unknown error occurred");
		break;
	}

	return result;
}
