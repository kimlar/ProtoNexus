#include "GameServerEngine.h"
#include "NetworkServer.h"
#include "Game.h"
#include "PrintUtils.h"

void GameServerEngine::Run()
{
	timer.Start();

	Game* game = new Game();
	game->Start();

	NetworkServer networkServer(*game);
	if (!networkServer.Start())
	{
		ERROR("GameServerEngine::Run(): networkServer.Start() failed");
		return; // Failure!
	}

	// Game loop
	bool run = true;
	while (run)
	{
		delta = timer.GetDeltaTime();

		game->Update(delta);

		networkServer.Update();

		// Wait a little -- to relax the network IO
		delayTime = delta;
		while (delayTime < netDelayTime)
		{
			delta = timer.GetDeltaTime();
			delayTime += delta;
		}
	}

	networkServer.CleanUp();

	game->CleanUp();
	delete game;
	game = nullptr;
}
