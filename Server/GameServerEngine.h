#pragma once

#include "Timer.h"

class GameServerEngine
{
public:
	void Run();
private:
	const float netDelayTime = 0.05f;
	float delayTime = 0.0f;
	float delta = 0.0f;
	Timer timer;
};
