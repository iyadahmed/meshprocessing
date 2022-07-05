/* Header only 3 component vector library,
 * the main reason to be header only is to allow inlining,
 * which is critical for this data structure, as it is meant to be used in very tight loops (e.g rendering, processing geometry, etc) */

// TODO: refactor into header and source files with move constructor and assignment overriden

#pragma once

#include <cstddef>
#include <ostream>
#include <cmath>
#include <stdexcept>

class Vec3
{
public:
    float x, y, z;

    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3(const float buf[3])
    {
        x = buf[0];
        y = buf[1];
        z = buf[2];
    }
    Vec3(float value = 0.0f)
    {
        x = y = z = value;
    }

    // https://stackoverflow.com/a/66663070/8094047
    friend std::ostream &operator<<(std::ostream &os, Vec3 const &v)
    {
        return os << "<Vector (" << v.x << ", " << v.y << ", " << v.z << ")>";
    }

    void min(Vec3 &out, const Vec3 &a, const Vec3 &b)
    {
        out.x = fminf(a.x, b.x);
        out.y = fminf(a.y, b.y);
        out.z = fminf(a.z, b.z);
    }

    void max(Vec3 &out, const Vec3 &a, const Vec3 &b)
    {
        out.x = fmaxf(a.x, b.x);
        out.y = fmaxf(a.y, b.y);
        out.z = fmaxf(a.z, b.z);
    }

    void min(const Vec3 &other)
    {
        x = fminf(x, other.x);
        y = fminf(y, other.y);
        z = fminf(z, other.z);
    }

    void max(const Vec3 &other)
    {
        x = fmaxf(x, other.x);
        y = fmaxf(y, other.y);
        z = fmaxf(z, other.z);
    }

    float dot(const Vec3 &other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cross(const Vec3 &other) const
    {
        Vec3 out;
        out.x = y * other.z - z * other.y;
        out.y = z * other.x - x * other.z;
        out.z = x * other.y - y * other.x;
        return out;
    }

    float length_squared() const { return x * x + y * y + z * z; }

    float length() const { return sqrt(length_squared()); }

    void normalize() { *this /= length(); }

    Vec3 normalized() const
    {
        float l = length();
        return Vec3(x / l, y / l, z / l);
    }

    Vec3 operator+(const Vec3 &other) const
    {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3 &other) const
    {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator-() const { return Vec3(-x, -y, -z); }

    Vec3 operator*(float t) const
    {
        return Vec3(x * t, y * t, z * t);
    }

    friend Vec3 operator*(float t, const Vec3 &v)
    {
        return v * t;
    }

    Vec3 &operator+=(const Vec3 &other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vec3 &operator*=(float t)
    {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    Vec3 &operator/=(float t) { return *this *= 1.0 / t; }

    Vec3 operator/(float t) const { return Vec3(x / t, y / t, z / t); }

    float &operator[](size_t index)
    {
        if (index == 0)
        {
            return x;
        }
        else if (index == 1)
        {
            return y;
        }
        else if (index == 2)
        {
            return z;
        }

        throw std::out_of_range("Index out of range");
    }

    float operator[](size_t index) const
    {
        return (*this)[index];
    }
};

typedef Vec3 Point3;
