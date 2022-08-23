#include "timer_mock.h"

void TimerMock::setTimer(const std::string& requestId, int delay) {
	_timers[requestId] = delay;
}
