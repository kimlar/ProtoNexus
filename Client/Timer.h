#pragma once

#include <SFML/System.hpp>

class Timer
{
public:
	void Start();
	float GetDeltaTime();
private:
	sf::Clock clock;
};
