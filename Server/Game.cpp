#include "Game.h"

#include "PrintUtils.h"

void Game::Start()
{

}

void Game::Update(float delta)
{
	testValue++;
	if (testValue > 1000000)
		testValue = 0;

	vSendMessages.clear();
	vSendMessages.push_back("First data");
	vSendMessages.push_back("Second data");
	vSendMessages.push_back("Third data");
	vSendMessages.push_back(std::to_string(testValue));
	vSendMessages.push_back("Fifth data");
	for (int i = 0; i < vSendMessages.size(); i++)
		PRINT("Game::Update(..): vSendMessages[" << i << "]: " << vSendMessages[i]);

	for (int i = 0; i < vReadMessages.size(); i++)
		PRINT("Game::Update(..): vReadMessages[" << i << "]: " << vReadMessages[i]);
	vReadMessages.clear();
}

void Game::CleanUp()
{

}
