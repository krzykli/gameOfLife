#ifndef RAYKH
#define RAYKH

#include <math.h>
#include "sunshine_math.h"

// ~0.15 ticks per pixel with 1 thread

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

struct work_order
{
   sunshine_offscreen_buffer *Buffer;
   int xmin;
   int ymin;
   int xmax;
   int ymax;
};

struct work_queue
{
   int workOrderCount;
   work_order* WorkOrders;
   volatile int NextWorkOrderIndex;
   volatile int TileRetiredCount;
};

struct raytracer_memory
{
    bool IsInitialized;
    int CoreCount;
    int TileWidth;
    int TileHeight;
    int TileCountX;
    int TileCountY;
    int TileCount;
    work_queue Queue;
    float offset;
    uint64 PermanentStorageSize;
    void *PermanentStorage;
};


bool
RenderTile(work_queue *Queue)
{
    int WorkOrderIndex = Queue->NextWorkOrderIndex;
    if (WorkOrderIndex >= Queue->workOrderCount)
    {
         return false;
    }
    work_order *Order = Queue->WorkOrders + WorkOrderIndex;
    sunshine_offscreen_buffer *Buffer = Order->Buffer;
    int xmin = Order->xmin;
    int ymin = Order->ymin;
    int xmax = Order->xmax;
    int ymax = Order->ymax;
    uint8 *start= (uint8 *)Buffer->Memory;
    uint8 *startNextGen = start + Buffer->BitmapMemSize;
    //uint32 *PixelNextGen = (uint32 *)nextGenMemCursor;
    for (int y=ymin; y < ymax; ++y)
    {
        uint8 *nextGenMemCursor = startNextGen + Buffer->Width * y * 4;
        uint32 *PixelNextGen = (uint32 *)nextGenMemCursor + xmin;

        for (int x=xmin; x < xmax; ++x)
        {
            uint32 active = IsActiveField(Buffer, x, y);
            uint32 sum = AnalizeNeighbours(Buffer, x, y);

            if (active)
            {
                if (sum < 2 || sum > 3)
                {
                    *PixelNextGen = INACTIVE_CELL_COLOR;
                }
                else
                {
                    *PixelNextGen = ACTIVE_CELL_COLOR;
                }
            }
            else
            {
                if (sum == 3)
                {
                    *PixelNextGen = NEW_CELL_COLOR;
                }
            }
            PixelNextGen++;
        }
    }
    Queue->TileRetiredCount++;
    Queue->NextWorkOrderIndex++;
    return 0;
}


DWORD WINAPI
WorkerThread(void* lpParameter)
{
     work_queue *Queue = (work_queue*)lpParameter;
     while(RenderTile(Queue)) {};
     return 0;
}

void
CreateWorkerThread(void *Params)
{
    DWORD ThreadID;
    HANDLE ThreadHandle = CreateThread(0, 0, WorkerThread, Params, 0, &ThreadID);
    CloseHandle(ThreadHandle);
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
        int coreCount = 2;
        Memory->CoreCount = coreCount;
        Memory->TileWidth = Buffer->Width / coreCount;
        Memory->TileHeight = Buffer->Width / coreCount;
        Memory->TileCountX = (Buffer->Width + Memory->TileWidth - 1) / Memory->TileWidth;
        Memory->TileCountY = (Buffer->Height + Memory->TileHeight - 1) / Memory->TileHeight;
        Memory->TileCount = Memory->TileCountX * Memory->TileCountY;
        work_queue Queue = {};
        Queue.workOrderCount = 0;
        Queue.WorkOrders = (work_order *)malloc(Memory->TileCount*sizeof(work_order));
        Memory->Queue = Queue;
    }
    LARGE_INTEGER StartTime = GetCurrentClockCounter();
    int TileCountY = Memory->TileCountY;
    int TileCountX = Memory->TileCountX;
    int TileWidth = Memory->TileWidth;
    int TileHeight = Memory->TileHeight;
    int TileCount = Memory->TileCount;
    work_queue Queue = Memory->Queue;

    for (int tileY = 0; tileY < TileCountY; ++tileY)
    {
        int tileYmin = tileY * TileHeight;
        int tileYmax= tileYmin + TileHeight;

        if (tileYmax > Buffer->Height)
        {
             tileYmax = Buffer->Height;
        }

        for (int tileX = 0; tileX < TileCountX; ++tileX)
        {
            int tileXmin = tileX * TileWidth;
            int tileXmax= tileXmin + TileWidth;

            if (tileXmax > Buffer->Width)
            {
                 tileXmax = Buffer->Width;
            }

            work_order *Order = Queue.WorkOrders + Queue.workOrderCount++;
            Order->Buffer = Buffer;
            Order->xmin = tileXmin;
            Order->xmax = tileXmax;
            Order->ymin = tileYmin;
            Order->ymax = tileYmax;
            tileXmin = tileXmin;
        }
    }

    //for (int i=0; i<4; i++)
    //{
         //CreateWorkerThread(&Queue);
    //}

    while(Queue.TileRetiredCount < TileCount)
    {
        RenderTile(&Queue);
    }

    MoveMemory(Buffer->Memory,
               (void *)((uint8 *)Buffer->Memory + Buffer->BitmapMemSize),
               Buffer->BitmapMemSize);

    WindowUpdateCallback();
}

#endif //RAYKH
