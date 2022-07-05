#pragma once

#include <cstddef>
#include <ostream>

class Vec3
{
public:
    float x, y, z;

    Vec3(float x, float y, float z);
    Vec3(const float buf[3]);
    Vec3(float value);
    Vec3();
    friend std::ostream &operator<<(std::ostream &os, Vec3 const &v);
    static void min(Vec3 &out, const Vec3 &a, const Vec3 &b);
    static void max(Vec3 &out, const Vec3 &a, const Vec3 &b);
    void min(const Vec3 &other);
    void max(const Vec3 &other);
    float dot(const Vec3 &other) const;
    Vec3 cross(const Vec3 &other) const;
    float length_squared() const;
    float length() const;
    void normalize();
    Vec3 normalized() const;
    Vec3 operator+(const Vec3 &other) const;
    Vec3 operator-(const Vec3 &other) const;
    Vec3 operator-() const;
    Vec3 operator*(float t) const;
    Vec3 &operator+=(const Vec3 &other);
    Vec3 &operator*=(float t);
    Vec3 &operator/=(float t);
    Vec3 operator/(float t) const;
    float &operator[](size_t index);
    float operator[](size_t index) const;
};

typedef Vec3 Point3;
