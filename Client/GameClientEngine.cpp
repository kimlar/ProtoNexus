#include "GameClientEngine.h"
#include "NetworkClient.h"
#include "Game.h"
#include "PrintUtils.h"

//#include <Windows.h>

void GameClientEngine::Run()
{
	timer.Start();

	Game* game = new Game();
	game->Start();

	NetworkClient networkClient(*game);;
	if (!networkClient.Start())
	{
		//std::cout << "ERROR: GameClientEngine::Run(): networkClient.Start() failed" << std::endl;
		ERROR("GameClientEngine::Run(): networkClient.Start() failed");
		return; // Failure!
	}

	//int testValue = 0;

	// Game loop
	bool run = true;
	while (run)
	{
		delta = timer.GetDeltaTime();

		game->Update(delta);

		run = networkClient.Update();		

		// Wait a little -- to relax the network IO
		delayTime = delta;
		while (delayTime < netDelayTime)
		{
			delta = timer.GetDeltaTime();
			delayTime += delta;
		}

	}

	networkClient.CleanUp();

	game->CleanUp();
	delete game;
	game = nullptr;
}
