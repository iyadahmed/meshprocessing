#pragma once

#include <cmath>

struct float3
{
    float x, y, z;
    float3(float value = 0.0f)
    {
        x = y = z = value;
    }
    float3(float x, float y, float z) : x(x), y(y), z(z)
    {
    }
    float &operator[](int index)
    {
        return reinterpret_cast<float *>(this)[index];
    }
    void max(const float3 &other)
    {
        x = std::max(x, other.x);
        y = std::max(y, other.y);
        z = std::max(z, other.z);
    }
    void min(const float3 &other)
    {
        x = std::min(x, other.x);
        y = std::min(y, other.y);
        z = std::min(z, other.z);
    }
    float3 operator-(const float3 &other) const
    {
        return {x - other.x, y - other.y, z - other.z};
    }
    float3 operator+(const float3 &other) const
    {
        return {x + other.x, y + other.y, z + other.z};
    }
    float3 operator*(float value)
    {
        return {x * value, y * value, z * value};
    }
    float3 cross(const float3 &other) const
    {
        return {y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x};
    }
    float dot(const float3 &other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }
    void normalize()
    {
        float l = std::sqrt(x * x + y * y + z * z);
        x /= l;
        y /= l;
        z /= l;
    }
};

inline float3 cross(const float3 &a, const float3 &b)
{
    return a.cross(b);
}

inline float dot(const float3 &a, const float3 &b)
{
    return a.dot(b);
}
