
uint32
vec3_color_to_uint32(vec3 &color)
{
     uint8 Red = uint8(color.r * 255);
     uint8 Green = uint8(color.g * 255);
     uint8 Blue = uint8(color.b * 255);
     return ((Red << 16) | (Green << 8) | Blue);
}


vec3
uint32_to_vec3_color(uint32 color)
{
    vec3 Result;

    uint32 maskRed = 0x00FF0000;
    uint32 maskGreen = 0x0000FF00;
    uint32 maskBlue = 0x000000FF;

    unsigned char Red = unsigned char((color & maskRed) >> 16);
    unsigned char Green = unsigned char((color & maskGreen) >> 8);
    unsigned char Blue = unsigned char((color & maskBlue));

    Result.r = float(Red / 255.0);
    Result.g = float(Green / 255.0);
    Result.b = float(Blue / 255.0);

    return Result;
}


void*
GetPixelAddress(sunshine_offscreen_buffer *Buffer, point2 &Pixel)
{
    uint8 *row = (uint8 *)Buffer->Memory;
    int offsetFromOrigin = (Buffer->Height - 1 - Pixel.y) * Buffer->Pitch +
        Buffer->BytesPerPixel * Pixel.x;
    return row += offsetFromOrigin;
}

void*
GetCanonicalPixelAddress(sunshine_offscreen_buffer *Buffer, int x, int y)
{
    uint8 *row = (uint8 *)Buffer->Memory;
    int offsetFromOrigin = y * Buffer->Pitch + Buffer->BytesPerPixel * x;
    return row += offsetFromOrigin;
}


void*
GetPixelAddress(sunshine_offscreen_buffer *Buffer, int x, int y)
{
    uint8 *row = (uint8 *)Buffer->Memory;
    int offsetFromOrigin = (Buffer->Height - 1 - y) * Buffer->Pitch +
        Buffer->BytesPerPixel * x;
    return row += offsetFromOrigin;
}


bool
IsPixelWithinFrameBufferBounds(sunshine_offscreen_buffer *Buffer, int x0, int y0)
{
    if (x0 < 0 || y0 < 0 || x0 > Buffer->Width - 1 || y0 > Buffer->Height - 1)
    {
        return false;
    }
    else
    {
         return true;
    }
}


void
ColorPixel(sunshine_offscreen_buffer *Buffer, uint32 Color, int x, int y)
{
    if (IsPixelWithinFrameBufferBounds(Buffer, x, y))
    {
        uint32 *Pixel = (uint32 *)GetPixelAddress(Buffer, x, y);
        *Pixel = Color;
    }
}

void
DrawLine(sunshine_offscreen_buffer *Buffer, point2 A, point2 B)
{
    LARGE_INTEGER StartTime = GetCurrentClockCounter();

    // Check bounds
    int xA = max(A.x, 0);
    xA = min(xA, Buffer->Width - 1);
    int yA = max(A.y, 0);
    yA = min(yA, Buffer->Height - 1);

    int xB = max(B.x, 0);
    xB = min(xB, Buffer->Width - 1);
    int yB = max(B.y, 0);
    yB = min(yB, Buffer->Height - 1);

    uint8 Red = 0;
    uint8 Green = 100;
    uint8 Blue = 255;

    int dx = xB - xA;
    int dy = yB - yA;

    float a;

    point2 pointA = Point2(xA, yA);
    point2 pointB = Point2(xB, yB);

    if (dx == 0)
    {
        uint8 *row = (uint8 *)GetPixelAddress(Buffer, pointA);
        for (int y = pointA.y; y <= pointB.y; ++y)
        {
            uint32 *Pixel = (uint32 *)row;
            *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
            row -= Buffer->Pitch;
        }
    }

    if (dy == 0)
    {
        int x_min = min(pointA.x, pointB.x);
        int x_max = max(pointA.x, pointB.x);
        // NOTE(kk): yA here is fine because dy == 0
        uint32 *Pixel= (uint32 *)GetPixelAddress(Buffer, x_min, pointA.y);
        for (int x = x_min; x <= x_max; ++x)
        {
            *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
        }
    }

    else
    {
        if (abs(dx) >= abs(dy))
        {
            a = float(dy) / float(dx);
            float b = pointA.y - a * pointA.x;
            int step_dx = (pointA.x > pointB.x) ? -1 : 1;
            if (step_dx == 1)
            {
                for (int x = pointA.x; x <= pointB.x; x += step_dx)
                {
                    float y = a * x + b;
                    uint32 *Pixel = (uint32 *)GetPixelAddress(Buffer, int(x), int(y));
                    *Pixel = ((Red << 16) | (Green << 8) | Blue);
                }
            }
            else
            {
                for (int x = pointA.x; x >= pointB.x; x += step_dx)
                {
                    float y = a * x + b;
                    uint32 *Pixel = (uint32 *)GetPixelAddress(Buffer, int(x), int(y));
                    *Pixel = ((Red << 16) | (Green << 8) | Blue);
                }
            }
        }
        else
        {
            a = float(dx) / float(dy);
            float b = xA - a * yA;
            int step_dy = (yA > yB) ? -1 : 1;
            if (step_dy == 1)
            {
                for (int y = pointA.y; y <= pointB.y; y += step_dy)
                {
                    float x = a * y + b;
                    uint32 *Pixel = (uint32 *)GetPixelAddress(Buffer, int(x), int(y));
                    *Pixel = ((Red << 16) | (Green << 8) | Blue);
                }
            }
            else
            {
                for (int y = pointA.y; y >= pointB.y; y += step_dy)
                {
                    float x = a * y + b;
                    uint32 *Pixel = (uint32 *)GetPixelAddress(Buffer, int(x), int(y));
                    *Pixel = ((Red << 16) | (Green << 8) | Blue);
                }
            }
        }
    }

    LARGE_INTEGER EndTime = GetCurrentClockCounter();
    char msPerFrameBuffer[512];
    sprintf_s(msPerFrameBuffer, "Ticks: %i\n", EndTime.QuadPart - StartTime.QuadPart);
    OutputDebugStringA(msPerFrameBuffer);
}

void
DrawFilledRectangle(sunshine_offscreen_buffer *Buffer, point2 A, point2 B)
{
    // NOTE(kk): assume for now point A has lower coords
    // Check bounds
    int x1 = max(A.x, 0);
    int y1 = max(A.y, 0);

    int x2 = min(B.x, Buffer->Width - 1);
    int y2 = min(B.y, Buffer->Height - 1);

    uint8 Red = 240;
    uint8 Green = 30;
    uint8 Blue = 140;

    uint8 *row = (uint8 *)GetPixelAddress(Buffer, x1, y1);

    for (int y = y1; y <= y2; ++y)
    {
        uint32 *Pixel = (uint32 *)row;
        for (int x = x1; x <= x2; ++x)
        {
            *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
        }
        row -= Buffer->Pitch;
    }
}


void
DrawCircleBase(sunshine_offscreen_buffer *Buffer, point2 center, int radius,
               void (*DrawCircleCallback)(sunshine_offscreen_buffer *buffer, point2 c, int X, int Y))
{

    LARGE_INTEGER StartTime = GetCurrentClockCounter();

    int x0 = center.x;
    int y0 = center.y;

    uint8 Red = 30;
    uint8 Green = 30;
    uint8 Blue = 30;
    uint32 Color = ((Red << 16) | (Green << 8) | Blue);

    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y)
    {
        DrawCircleCallback(Buffer, center, x, y);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy +=2;
        }
        if (err > 0)
        {
            x--;
            dx += 2;
            err += (-radius << 1) + dx;
        }
    }

    LARGE_INTEGER EndTime = GetCurrentClockCounter();
    char msPerFrameBuffer[512];
    sprintf_s(msPerFrameBuffer, "Ticks: %i\n", EndTime.QuadPart - StartTime.QuadPart);
    OutputDebugStringA(msPerFrameBuffer);
}


void
DrawCircleOutlineCallback(sunshine_offscreen_buffer *Buffer, point2 center, int x, int y)
{
    uint8 Red = 30;
    uint8 Green = 30;
    uint8 Blue = 30;
    uint32 Color = ((Red << 16) | (Green << 8) | Blue);

    int x0 = center.x;
    int y0 = center.y;
    ColorPixel(Buffer, Color, x0 + x, y0 + y);
    ColorPixel(Buffer, Color, x0 + y, y0 + x);
    ColorPixel(Buffer, Color, x0 - y, y0 - x);
    ColorPixel(Buffer, Color, x0 - x, y0 - y);
    ColorPixel(Buffer, Color, x0 - x, y0 + y);
    ColorPixel(Buffer, Color, x0 + x, y0 - y);
    ColorPixel(Buffer, Color, x0 - y, y0 + x);
    ColorPixel(Buffer, Color, x0 + y, y0 - x);
}

//
// TRIANGLE
//
void
DrawCircleFilledCallback(sunshine_offscreen_buffer *Buffer, point2 center, int x, int y)
{
    int x0 = center.x;
    int y0 = center.y;

    point2 pointA = {-y + x0, x + y0};
    point2 pointB = {y + x0, x + y0};
    DrawLine(Buffer, pointA, pointB);

    pointA = {-x + x0, y + y0};
    pointB = {x + x0, y + y0};
    DrawLine(Buffer, pointA, pointB);

    pointA = {-x + x0, -y + y0};
    pointB = {x + x0, -y + y0};
    DrawLine(Buffer, pointA, pointB);

    pointA = {-y + x0, -x + y0};
    pointB = {y + x0, -x + y0};
    DrawLine(Buffer, pointA, pointB);
}

void swap(int &a, int &b)
{
    int temp = a;
    a = b;
    b = temp;
}

struct Sort3PointsVerticallyOut
{
     int a, b, c;
};

Sort3PointsVerticallyOut Sort3PointsVertically(point2* Points)
{
    // Returns ascending order
    int indexOrder[3] = {0, 1, 2};
    int a = Points[0].y;
    int b = Points[1].y;
    int c = Points[2].y;

    if(a>b)
    {
        swap(a, b);
        swap(indexOrder[0], indexOrder[1]);
    }
    if(a>c)
    {
        swap(a,c);
        swap(indexOrder[0], indexOrder[2]);
    }
    if(b>c)
    {
        swap(b,c);
        swap(indexOrder[1], indexOrder[2]);
    }

    return {indexOrder[0], indexOrder[1], indexOrder[2]};
}


void
FillBottomFlatTriangle(sunshine_offscreen_buffer *Buffer,
                       point2 v1, point2 v2, point2 v3)
{
    float invslope1 = float(v2.x - v1.x) / float(v2.y - v1.y);
    float invslope2 = float(v3.x - v1.x) / float(v3.y - v1.y);

    float curx1 = float(v1.x);
    float curx2 = float(v1.x);

    for (int scanlineY = v1.y; scanlineY >= v2.y; scanlineY--)
    {
        point2 pointA = {(int)curx1, scanlineY};
        point2 pointB = {(int)curx2, scanlineY};
        DrawLine(Buffer, pointA, pointB);
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

void
FillTopFlatTriangle(sunshine_offscreen_buffer *Buffer,
                    point2 v1, point2 v2, point2 v3)
{
  float invslope1 = float(v3.x - v1.x) / float(v3.y - v1.y);
  float invslope2 = float(v3.x - v2.x) / float(v3.y - v2.y);

  float curx1 = float(v3.x);
  float curx2 = float(v3.x);

  for (int scanlineY = v3.y; scanlineY < v2.y; scanlineY++)
  {
    point2 pointA = {(int)curx1, scanlineY};
    point2 pointB = {(int)curx2, scanlineY};
    DrawLine(Buffer, pointA, pointB);
    curx1 += invslope1;
    curx2 += invslope2;
  }
}

void
DrawTriangle(sunshine_offscreen_buffer *Buffer,
             point2 A,
             point2 B,
             point2 C)
{
    // NOTE(kk): assume for now point A has lower coords
    // Check bounds
    point2 Points[3] = {A, B, C};
    Sort3PointsVerticallyOut order = Sort3PointsVertically(Points);
    point2 minPoint = Points[order.a];
    point2 midPoint = Points[order.b];
    point2 maxPoint = Points[order.c];


    if (midPoint.y == maxPoint.y)
    {
        FillTopFlatTriangle(Buffer, maxPoint, midPoint, minPoint);
    }
    if (midPoint.y == minPoint.y)
    {
        FillBottomFlatTriangle(Buffer, maxPoint, midPoint, minPoint);
    }
    else
    {
        /* general case - split the triangle in a topflat and bottom-flat one */
        point2 p4 = {(int)(maxPoint.x + ((float)(midPoint.y - maxPoint.y) /
                    (float)(minPoint.y - maxPoint.y)) * (minPoint.x - maxPoint.x)),
                    midPoint.y};
        FillBottomFlatTriangle(Buffer, maxPoint, midPoint, p4);
        FillTopFlatTriangle(Buffer, midPoint, p4, minPoint);
    }
}

//
// END TRIANGLE
//
