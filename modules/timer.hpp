#ifndef MODULES_TIMER_HPP
#define MODULES_TIMER_HPP

#include <iostream>
#include <chrono>

class Timer
{

public:
    void Start()
    {
        start = std::chrono::steady_clock::now();
    }
    
    void Stop()
    {
        stop = std::chrono::steady_clock::now();
    }

    void Report()
    {
        std::chrono::steady_clock::duration d_start = start.time_since_epoch();
        std::chrono::steady_clock::duration d_stop = stop.time_since_epoch();
        std::chrono::steady_clock::duration d = d_stop - d_start;
        std::chrono::duration<double, std::ratio<1, 1>> dsecs(d);
        std::cout << "\n Wall time is " << dsecs.count() << " seconds" << std::endl;
    }

private:
    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock::time_point stop;

};

#endif