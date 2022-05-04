#pragma once

#include <cmath>
#include <iostream>
#include <stdexcept>

class Vec3
{
public:
    double x, y, z;

    Vec3(double x = 0.0, double y = 0.0, double z = 0.0) : x(x), y(y), z(z) {}
    Vec3(float buf[3])
    {
        x = buf[0];
        y = buf[1];
        z = buf[2];
    }

    // https://stackoverflow.com/a/66663070/8094047
    friend auto operator<<(std::ostream &os, Vec3 const &m) -> std::ostream &
    {
        return os << "<Vector (" << m.x << ", " << m.y << ", " << m.z << ")>";
    }

    static void min(Vec3 &out, const Vec3 &a, const Vec3 &b)
    {
        out.x = fmin(a.x, b.x);
        out.y = fmin(a.y, b.y);
        out.z = fmin(a.z, b.z);
    }

    static void max(Vec3 &out, const Vec3 &a, const Vec3 &b)
    {
        out.x = fmax(a.x, b.x);
        out.y = fmax(a.y, b.y);
        out.z = fmax(a.z, b.z);
    }

    double dot(const Vec3 &other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cross(const Vec3 &b) const
    {
        Vec3 out;
        out.x = y * b.z - z * b.y;
        out.y = z * b.x - x * b.z;
        out.z = x * b.y - y * b.x;
        return out;
    }

    double length_squared() const { return x * x + y * y + z * z; }

    double length() const { return sqrt(length_squared()); }

    void normalize() { *this /= length(); }

    Vec3 normalized() const
    {
        double l = length();
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

    Vec3 operator*(const double t)
    {
        Vec3 out;
        out *= t;
        return out;
    }

    Vec3 &operator+=(const Vec3 &other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vec3 &operator*=(const double t)
    {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    Vec3 &operator/=(const double t) { return *this *= 1.0 / t; }

    Vec3 operator/(const double t) const { return Vec3(x / t, y / t, z / t); }

    double &operator[](const size_t index)
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
};

typedef Vec3 Point3;