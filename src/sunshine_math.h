#ifndef SUNSHINE_MATH_H
#define SUNSHINE_MATH_H

// point2
union point2
{
     struct {
          int x, y;
     };
     int E[2];
};

point2
Point2(int x, int y)
{
    point2 result;
    result.x = x;
    result.y = y;
    return result;
}

point2
Point2_f(float x, float y)
{
    point2 result = {(int)x, (int)y};
    return result;
}

// vec2
union vec2
{
     struct {
          float x, y;
     };
     float E[2];
};

vec2
Vec2_i(int x, int y)
{
    vec2 result = {(float)x, (float)y};
    return result;
}

vec2
Vec2(float x, float y)
{
    vec2 result;
    result.x = x;
    result.y = y;
    return result;
}

// vec3
union vec3
{
     struct {
          float x, y, z;
     };
     struct {
          float r, g, b;
     };
     float E[3];
};


vec3
lerp_vec3(vec3 &a, vec3 &b, float &t)
{
    vec3 Result = {a.x + t * (b.x - a.x),
                   a.y + t * (b.y - a.y),
                   a.z + t * (b.z - a.z)};
    return Result;
}

#endif
