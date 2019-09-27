#pragma once

#include <string>
#include <vector>

class Game
{
public:
	Game() {}
	~Game() {}

	void Start();
	void Update(float delta);
	void CleanUp();

	int GetTestValue() { return testValue; }
	std::vector<std::string> GetSendMessages() { return vSendMessages; }
	void SetReadMessages(std::vector<std::string> vReadMessages) { this->vReadMessages = vReadMessages; }
private:
	int testValue = 0;
	std::vector<std::string> vSendMessages;
	std::vector<std::string> vReadMessages;
};
