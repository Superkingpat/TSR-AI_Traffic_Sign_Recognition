#include "Timer.h"

Timer::Timer() {
    m_deltaTime = 0.0;
    m_timeCounter1 = 0.0;
    m_timeCounter2 = 0.0;
}

void Timer::startTime() {
    m_startTime = std::chrono::high_resolution_clock::now();
    m_sinceCreation = std::chrono::high_resolution_clock::now();
}

void Timer::update() {
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> delta_time = endTime - m_startTime;
    m_deltaTime = delta_time.count();
    m_timeCounter1 += m_deltaTime;
    m_timeCounter2 += m_deltaTime;
    m_startTime = endTime;
}

double Timer::getDeltaTime() const {
    return m_deltaTime;
}

double Timer::getCounter1() const {
    return m_timeCounter1;
}

void Timer::resetCounter1() {
    m_timeCounter1 = 0.0;
}

double Timer::getCounter2() const {
    return m_timeCounter2;
}

void Timer::resetCounter2() {
    m_timeCounter2 = 0.0;
}

double Timer::getFullTime() {
    auto nTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dur = nTime - m_sinceCreation;
    return dur.count();
}

