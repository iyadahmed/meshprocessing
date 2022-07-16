/* Header only 3 component vector library,
 * the main reason to be header only is to allow inlining,
 * which is critical for this data structure, as it is meant to be used in very tight loops (e.g rendering, processing geometry, etc) */

// NOTE: consider refactoring into header and source files with move constructor and assignment overriden to keep some of inlining performance

#pragma once

#include <cmath>
#include <ostream>

struct Vec3
{
    float x, y, z;

    Vec3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}
    Vec3(const float buf[3]) noexcept
    {
        x = buf[0];
        y = buf[1];
        z = buf[2];
    }
    Vec3(float value = 0.0f) noexcept
    {
        x = y = z = value;
    }

    // https://stackoverflow.com/a/66663070/8094047
    friend std::ostream &operator<<(std::ostream &os, Vec3 const &v)
    {
        return os << "<Vector (" << v.x << ", " << v.y << ", " << v.z << ")>";
    }

    static void min(Vec3 &out, const Vec3 &a, const Vec3 &b) noexcept
    {
        out.x = std::min(a.x, b.x);
        out.y = std::min(a.y, b.y);
        out.z = std::min(a.z, b.z);
    }

    static void max(Vec3 &out, const Vec3 &a, const Vec3 &b) noexcept
    {
        out.x = std::max(a.x, b.x);
        out.y = std::max(a.y, b.y);
        out.z = std::max(a.z, b.z);
    }

    void min(const Vec3 &other) noexcept
    {
        x = std::min(x, other.x);
        y = std::min(y, other.y);
        z = std::min(z, other.z);
    }

    void max(const Vec3 &other) noexcept
    {
        x = std::max(x, other.x);
        y = std::max(y, other.y);
        z = std::max(z, other.z);
    }

    float dot(const Vec3 &other) const noexcept
    {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cross(const Vec3 &other) const noexcept
    {
        return {y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x};
    }

    float length_squared() const noexcept
    {
        return x * x + y * y + z * z;
    }

    float length() const noexcept
    {
        return sqrt(length_squared());
    }

    void normalize() noexcept
    {
        *this /= length();
    }

    Vec3 normalized() const noexcept
    {
        float l = length();
        return {x / l, y / l, z / l};
    }

    Vec3 operator+(const Vec3 &other) const noexcept
    {
        return {x + other.x, y + other.y, z + other.z};
    }

    Vec3 operator-(const Vec3 &other) const noexcept
    {
        return {x - other.x, y - other.y, z - other.z};
    }

    Vec3 operator-() const noexcept
    {
        return {-x, -y, -z};
    }

    Vec3 operator*(float t) const noexcept
    {
        return {x * t, y * t, z * t};
    }

    friend Vec3 operator*(float t, const Vec3 &v) noexcept
    {
        return v * t;
    }

    Vec3 &operator+=(const Vec3 &other) noexcept
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vec3 &operator*=(float t) noexcept
    {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    Vec3 &operator/=(float t) noexcept
    {
        x /= t;
        y /= t;
        z /= t;
        return *this;
    }

    Vec3 operator/(float t) const noexcept
    {
        return {x / t, y / t, z / t};
    }

    bool operator==(const Vec3 &other) const noexcept
    {
        return (x == other.x) && (y == other.y) && (z == other.z);
    }

    bool operator!=(const Vec3 &other) const noexcept
    {
        return !(this->operator==(other));
    }

    float &operator[](size_t index) noexcept
    {
        return reinterpret_cast<float *>(this)[index];
    }

    float operator[](size_t index) const noexcept
    {
        return (*this)[index];
    }
};

// Convenience functions
inline Vec3 cross(const Vec3 &a, const Vec3 &b)
{
    return a.cross(b);
}

inline float dot(const Vec3 &a, const Vec3 &b)
{
    return a.dot(b);
}
