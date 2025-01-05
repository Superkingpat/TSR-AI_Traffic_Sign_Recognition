#include "Timer.h"

Timer::Timer() {
	m_deltaTime = 1.f;
	m_timeCounter = 0.f;
}

void Timer::startTime() {
	m_startTime = std::chrono::high_resolution_clock::now();;
}

void Timer::endTime() {
	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> delta_time = endTime - m_startTime;
	m_startTime = std::chrono::high_resolution_clock::now();
	m_deltaTime = delta_time.count();
	m_timeCounter += m_deltaTime;
}

float Timer::getDeltaTime() {
	return m_deltaTime;
}

float Timer::getCounter() {
	return m_timeCounter;
}

void Timer::resetCounter() {
	m_timeCounter = 0.f;
}


