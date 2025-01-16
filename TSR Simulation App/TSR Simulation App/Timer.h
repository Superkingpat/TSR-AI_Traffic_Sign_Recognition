#pragma once
#include "ObjectHandler.h"

class Timer {
private:
    double m_deltaTime;
    double m_timeCounter;
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_sinceCreation;

public:
    Timer();
    void startTime();
    void update();
    double getDeltaTime() const;
    double getCounter() const;
    void resetCounter();
    double getFullTime();
};
