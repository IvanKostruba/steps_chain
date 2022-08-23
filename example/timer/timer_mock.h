#pragma once

#include <string>
#include <unordered_map>

class TimerMock {
public:
	void setTimer(const std::string& requestId, int delay);

	std::unordered_map<std::string, int> _timers;
};
