#include "Timer.h"

void Timer::Start()
{
	clock.restart();
}

float Timer::GetDeltaTime()
{
	return clock.restart().asSeconds();
}
