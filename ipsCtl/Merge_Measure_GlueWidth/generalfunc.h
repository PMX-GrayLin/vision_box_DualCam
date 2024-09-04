#pragma once

#include <opencv2/opencv.hpp>
#include "cvip.h"


#ifdef   ELAPSED_TIME_ms_DEBUG

#define  printT(...) COLOR_PRINT("Elapsed Time (ms): " __VA_ARGS__)

#else 

#define  printT(...) 

#endif


class StartStopTimer
{
    int64 startTime;

    /**
     * Value of the timer.
     */
    double value;

public:
    /**
     * Constructor.
     */
    StartStopTimer()
    {
        reset();
    }

    /**
     * Starts the timer.
     */
    void start()
    {
        startTime = cv::getTickCount();
    }

    /**
     * Stops the timer and updates timer value.
     */
    void stop(std::string strInfo = std::string())
    {
        reset();
        int64 stopTime = cv::getTickCount();
        value = 1000 * ((double)stopTime - startTime) / cv::getTickFrequency();
        printT("%s ---> %5.3f\n", strInfo.c_str(), value);
    }

    /**
     * Resets the timer value to 0.
     */
    void reset()
    {
        value = 0.0;
    }

};

