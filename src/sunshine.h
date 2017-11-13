#ifndef RAYKH
#define RAYKH

#include <math.h>
#include "sunshine_math.h"


struct raytracer_memory
{
    bool IsInitialized;
    float offset;
    uint64 PermanentStorageSize;
    void *PermanentStorage;
};

struct sunshine_offscreen_buffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
    int BitmapMemSize;
};

#include "gameOfLife.h"

void
UpdateAndRender(raytracer_memory *Memory,
                sunshine_offscreen_buffer *Buffer,
                void (*WindowUpdateCallback)(void))
{
    // Get application state from Memory
    if (!Memory->IsInitialized)
    {
        Memory->offset = 0.0f;
        Memory->IsInitialized = 1;
    }
    LARGE_INTEGER StartTime = GetCurrentClockCounter();

    uint8 *row = (uint8 *)Buffer->Memory;
    uint8 *nextGenMemCursor = row + Buffer->BitmapMemSize;

    for (int y=0; y < Buffer->Height; ++y)
    {
        uint32 *PixelNextGen = (uint32 *)nextGenMemCursor;
        for (int x=0; x < Buffer->Width; ++x)
        {
            //uint32 *Pixel = (uint32 *)row;
            int active = IsActiveField(Buffer, x, y);
            int sum = AnalizeNeighbours(Buffer, x, y);

            if (active)
            {
                if (sum < 2 || sum > 3)
                {
                    *PixelNextGen = ((0 << 16) | (0 << 8) | 0);
                }
                else
                {
                    *PixelNextGen = ((255 << 16) | (255 << 8) | 255);
                }
            }
            else
            {
                if (sum == 3)
                {
                    *PixelNextGen = ((255 << 16) | (255 << 8) | 255);
                }
            }
            *PixelNextGen++;
        }
        nextGenMemCursor += Buffer->Pitch;
    }

    MoveMemory(Buffer->Memory,
               (void *)((uint8 *)Buffer->Memory + Buffer->BitmapMemSize),
               Buffer->BitmapMemSize);

    WindowUpdateCallback();
}

#endif //RAYKH
