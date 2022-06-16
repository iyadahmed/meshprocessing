#include <cmath>
#include <ostream>
#include <stdexcept>

#include "vec3.hh"

Vec3::Vec3(float x, float y, float z) : x_(x), y_(y), z_(z) {}
Vec3::Vec3(const float buf[3])
{
    x_ = buf[0];
    y_ = buf[1];
    z_ = buf[2];
}

// https://stackoverflow.com/a/66663070/8094047
std::ostream &operator<<(std::ostream &os, Vec3 const &v)
{
    return os << "<Vector (" << v.x_ << ", " << v.y_ << ", " << v.z_ << ")>";
}

void Vec3::min(Vec3 &out, const Vec3 &a, const Vec3 &b)
{
    out.x_ = fmin(a.x_, b.x_);
    out.y_ = fmin(a.y_, b.y_);
    out.z_ = fmin(a.z_, b.z_);
}

void Vec3::max(Vec3 &out, const Vec3 &a, const Vec3 &b)
{
    out.x_ = fmax(a.x_, b.x_);
    out.y_ = fmax(a.y_, b.y_);
    out.z_ = fmax(a.z_, b.z_);
}

float Vec3::dot(const Vec3 &other) const
{
    return x_ * other.x_ + y_ * other.y_ + z_ * other.z_;
}

Vec3 Vec3::cross(const Vec3 &other) const
{
    Vec3 out;
    out.x_ = y_ * other.z_ - z_ * other.y_;
    out.y_ = z_ * other.x_ - x_ * other.z_;
    out.z_ = x_ * other.y_ - y_ * other.x_;
    return out;
}

float Vec3::length_squared() const { return x_ * x_ + y_ * y_ + z_ * z_; }

float Vec3::length() const { return sqrt(length_squared()); }

void Vec3::normalize() { *this /= length(); }

Vec3 Vec3::normalized() const
{
    float l = length();
    return Vec3(x_ / l, y_ / l, z_ / l);
}

Vec3 Vec3::operator+(const Vec3 &other) const
{
    return Vec3(x_ + other.x_, y_ + other.y_, z_ + other.z_);
}

Vec3 Vec3::operator-(const Vec3 &other) const
{
    return Vec3(x_ - other.x_, y_ - other.y_, z_ - other.z_);
}

Vec3 Vec3::operator-() const { return Vec3(-x_, -y_, -z_); }

Vec3 Vec3::operator*(float t)
{
    Vec3 out;
    out *= t;
    return out;
}

Vec3 &Vec3::operator+=(const Vec3 &other)
{
    x_ += other.x_;
    y_ += other.y_;
    z_ += other.z_;
    return *this;
}

Vec3 &Vec3::operator*=(float t)
{
    x_ *= t;
    y_ *= t;
    z_ *= t;
    return *this;
}

Vec3 &Vec3::operator/=(float t) { return *this *= 1.0 / t; }

Vec3 Vec3::operator/(float t) const { return Vec3(x_ / t, y_ / t, z_ / t); }

float &Vec3::operator[](size_t index)
{
    if (index == 0)
    {
        return x_;
    }
    else if (index == 1)
    {
        return y_;
    }
    else if (index == 2)
    {
        return z_;
    }

    throw std::out_of_range("Index out of range");
}
