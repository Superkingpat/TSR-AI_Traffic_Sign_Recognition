#include "Timer.h"

Timer::Timer() {
    m_deltaTime = 0.0;
    m_timeCounter = 0.0;
}

void Timer::startTime() {
    m_startTime = std::chrono::high_resolution_clock::now();
}

void Timer::update() {
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> delta_time = endTime - m_startTime;
    m_deltaTime = delta_time.count();
    m_timeCounter += m_deltaTime;
    m_startTime = endTime;
}

double Timer::getDeltaTime() const {
    return m_deltaTime;
}

double Timer::getCounter() const {
    return m_timeCounter;
}

void Timer::resetCounter() {
    m_timeCounter = 0.0;
}

