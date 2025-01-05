#pragma once
#include "ObjectHandler.h"

class Timer {
private:
	float m_deltaTime;
	float m_timeCounter;
	std::chrono::high_resolution_clock::time_point m_startTime;
public:
	Timer();
	void startTime();
	void endTime();
	float getDeltaTime();
	float getCounter();
	void resetCounter();
};

