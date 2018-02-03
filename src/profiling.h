#include "global_defines.h"

global_variable LARGE_INTEGER _PCFreq;

LARGE_INTEGER
GetCurrentClockCounter()
{
     LARGE_INTEGER Result;
     QueryPerformanceCounter(&Result);
     return Result;
}

uint64
GetMilisecondsElapsed(LARGE_INTEGER StartTime, LARGE_INTEGER EndTime)
{
    LARGE_INTEGER ticksElapsed;
    ticksElapsed.QuadPart = EndTime.QuadPart - StartTime.QuadPart;
    //ticksElapsed.QuadPart *= 1000000;
    //ticksElapsed.QuadPart /= _PCFreq.QuadPart;
    return ticksElapsed.QuadPart;
}

void PrintTime(uint64 ticksElapsed, char * label)
{
    char msPerFrameBuffer[512];
    float msElapsed = float(ticksElapsed * 1000000) / _PCFreq.QuadPart;
    int FPS = (int)(1 / ((msElapsed) / 1000000.0f));
    sprintf_s(msPerFrameBuffer, "[%s] \t\t\t %i ticks/f \t %i FPS\n",
              label,
              ticksElapsed,
              FPS);
    OutputDebugStringA(msPerFrameBuffer);
}

void PrintTime(float ticksElapsed, char * label)
{
    char msPerFrameBuffer[512];
    float msElapsed = float(ticksElapsed * 1000000) / _PCFreq.QuadPart;
    sprintf_s(msPerFrameBuffer, "[%s] \t\t\t %.4f ticks/pixel/f\n",
              label,
              ticksElapsed);
    OutputDebugStringA(msPerFrameBuffer);
}
