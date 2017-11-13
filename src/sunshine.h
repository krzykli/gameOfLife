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


struct renderStats
{
    float renderTime;
    int aaSamples;
    int resolution[2];
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


#include "sunshine_rendering.h"


bool
IsAddressWithinFrameBufferBounds(sunshine_offscreen_buffer *Buffer, void* Pixel)
{
    if ((uint8 *)Buffer->Memory > (uint8 *)Pixel ||
        (uint8 *)GetPixelAddress(Buffer, Buffer->Width - 1, Buffer->Height - 1) < (uint8 *)Pixel)
    {
        return false;
    }
    else
    {
         return true;
    }
}

uint32
MSAA(sunshine_offscreen_buffer *Buffer, int x, int y)
{
    vec3 Result;

    int x_left = max(x - 1, 0);
    int x_right = min(x + 1, Buffer->Width - 1);

    vec3 colorLeft = uint32_to_vec3_color(*(uint32 *)GetPixelAddress(Buffer, x_left, y));
    vec3 colorMiddle = uint32_to_vec3_color(*(uint32 *)GetPixelAddress(Buffer, x, y));
    vec3 colorRight = uint32_to_vec3_color(*(uint32 *)GetPixelAddress(Buffer, x_right, y));

    Result.r = (colorLeft.r + colorMiddle.r + colorRight.r) / 3.0f;
    Result.g = (colorLeft.g + colorMiddle.g + colorRight.g) / 3.0f;
    Result.b = (colorLeft.b + colorMiddle.b + colorRight.b) / 3.0f;

    return vec3_color_to_uint32(Result);
}

uint32
IsActiveField(sunshine_offscreen_buffer *Buffer, int x, int y)
{
    uint32 color = *(uint32 *)GetCanonicalPixelAddress(Buffer, x, y);
    if (color == ((255 << 16) | (255 << 8) | 255))
    {
         return color;
    }
    else
    {
         return 0;
    }
}

int
AnalizeNeighbours(sunshine_offscreen_buffer *Buffer, int x, int y)
{
    int neighbours[8] = {0};
    if (x == 0 || y == 0 || x == Buffer->Width - 1 || y == Buffer->Height - 1)
    {
        if (IsPixelWithinFrameBufferBounds(Buffer, x - 1, y - 1))
        {
            neighbours[0] = IsActiveField(Buffer, x - 1, y - 1);
        }
        if (IsPixelWithinFrameBufferBounds(Buffer, x - 1, y))
        {
            neighbours[1] = IsActiveField(Buffer, x - 1, y);
        }
        if (IsPixelWithinFrameBufferBounds(Buffer, x - 1, y + 1))
        {
            neighbours[2] = IsActiveField(Buffer, x - 1, y + 1);
        }
        if (IsPixelWithinFrameBufferBounds(Buffer, x, y - 1))
        {
            neighbours[3] = IsActiveField(Buffer, x, y - 1);
        }
        if (IsPixelWithinFrameBufferBounds(Buffer, x, y + 1))
        {
            neighbours[4] = IsActiveField(Buffer, x, y + 1);
        }
        if (IsPixelWithinFrameBufferBounds(Buffer, x + 1, y - 1))
        {
            neighbours[5] = IsActiveField(Buffer, x + 1, y - 1);
        }
        if (IsPixelWithinFrameBufferBounds(Buffer, x + 1, y))
        {
            neighbours[6] = IsActiveField(Buffer, x + 1, y);
        }
        if (IsPixelWithinFrameBufferBounds(Buffer, x + 1, y + 1))
        {
            neighbours[7] = IsActiveField(Buffer, x + 1, y + 1);
        }
    }
    else
    {
        uint32 *leftTopPixel = (uint32 *)GetCanonicalPixelAddress(Buffer, x-1, y-1);
        uint32 *row1 = (leftTopPixel + Buffer->Width);
        uint32 *row2 = (leftTopPixel + 2 * Buffer->Width);

        neighbours[0] = *leftTopPixel;
        neighbours[1] = *(leftTopPixel + 1);
        neighbours[2] = *(leftTopPixel + 2);

        neighbours[3] = *(row1);
        neighbours[4] = *(row1 + 2);

        neighbours[5] = *(row2);
        neighbours[6] = *(row2 + 1);
        neighbours[7] = *(row2 + 2);
    }

    int active = ((255 << 16) | (255 << 8) | 255);
    int sum = 0;
    for (int i=0; i < 8; i++)
    {
        sum += neighbours[i];
    }
    return (sum / active);
}

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

    //float offset = Memory->offset;

    //float cycleOffset = float((sin(5 * offset * 3.14f) + 1.0f)) / 2.0f;

    //vec3 corner1 = {1.0f - cycleOffset, 0.0f + cycleOffset, 0.0f + cycleOffset};
    //vec3 corner2 = {1.0f, 1.0f - cycleOffset, 0.0f};
    //vec3 corner3 = {0.0f + cycleOffset, 0.0f + cycleOffset, 1.0f - cycleOffset};
    //vec3 corner4 = {0.0f, 1.0f - cycleOffset, 1.0f};

    LARGE_INTEGER StartTime = GetCurrentClockCounter();

    //DrawRectangle(Buffer, 0, 0, 31, 31, 0);
    //DrawRectangle(Buffer, 0, 0, Buffer->Width - 1, Buffer->Height - 1, 0);
    //DrawLine(Buffer, 0, 0, Buffer->Width - 1, Buffer->Height - 1);
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
    //DrawRectangle(Buffer, 0, 0, 31, 31, 0);
    //DrawLine(Buffer, 0, 0, 31, 31);

    WindowUpdateCallback();
}

#endif //RAYKH
