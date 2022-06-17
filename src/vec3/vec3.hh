#pragma once

#include <cstddef>
#include <ostream>

class Vec3
{
public:
    float x_, y_, z_;

    Vec3(float x = 0.0, float y = 0.0, float z = 0.0);
    Vec3(const float buf[3]);
    friend std::ostream &operator<<(std::ostream &os, Vec3 const &v);
    static void min(Vec3 &out, const Vec3 &a, const Vec3 &b);
    static void max(Vec3 &out, const Vec3 &a, const Vec3 &b);
    float dot(const Vec3 &other) const;
    Vec3 cross(const Vec3 &b) const;
    float length_squared() const;
    float length() const;
    void normalize();
    Vec3 normalized() const;
    Vec3 operator+(const Vec3 &other) const;
    Vec3 operator-(const Vec3 &other) const;
    Vec3 operator-() const;
    Vec3 operator*(float t);
    Vec3 &operator+=(const Vec3 &other);
    Vec3 &operator*=(float t);
    Vec3 &operator/=(float t);
    Vec3 operator/(float t) const;
    float &operator[](size_t index);
};

typedef Vec3 Point3;
