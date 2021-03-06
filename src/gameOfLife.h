#ifndef GAMEOFLIFEH
#define GAMEOFLIFEH
#include "sunshine_rendering.h"

#define ACTIVE_CELL_COLOR 0x00FFFFFF
#define INACTIVE_CELL_COLOR 0x00000000
#define NEW_CELL_COLOR 0x00FF0000

uint32
IsActiveField(sunshine_offscreen_buffer *Buffer, int x, int y)
{
    uint32 color = *(uint32 *)GetCanonicalPixelAddress(Buffer, x, y);
    if (color & NEW_CELL_COLOR & ACTIVE_CELL_COLOR)
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
        int width = Buffer->Width;
        uint32 *leftTopPixel = (uint32 *)GetCanonicalPixelAddress(Buffer, x-1, y-1);
        uint32 *row1 = (leftTopPixel + width);
        uint32 *row2 = (leftTopPixel + 2 * width);

        neighbours[0] = *leftTopPixel;
        neighbours[1] = *(leftTopPixel + 1);
        neighbours[2] = *(leftTopPixel + 2);

        neighbours[3] = *(row1);
        neighbours[4] = *(row1 + 2);

        neighbours[5] = *(row2);
        neighbours[6] = *(row2 + 1);
        neighbours[7] = *(row2 + 2);
    }

    int active = ACTIVE_CELL_COLOR;
    int sum = 0;
    for (int i=0; i < 8; i++)
    {
        sum += (neighbours[i] & NEW_CELL_COLOR);
    }
    return sum / (uint32)NEW_CELL_COLOR;
}

#endif //GAMEOFLIFEH

