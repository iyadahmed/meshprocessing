/* Header only 3 component vector library,
 * the main reason to be header only is to allow inlining,
 * which is critical for this data structure, as it is meant to be used in very
 * tight loops (e.g rendering, processing geometry, etc) */

#pragma once

#include <cmath>
#include <ostream>


template <typename T>
class Vec3 {
  T x, y, z;

  Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
  Vec3(const T buf[3]) {
    x = buf[0];
    y = buf[1];
    z = buf[2];
  }
  Vec3(T value = 0.0f) { x = y = z = value; }

  // https://stackoverflow.com/a/66663070/8094047
  friend std::ostream &operator<<(std::ostream &os, Vec3<T> const &v) {
    return os << "<Vector (" << v.x << ", " << v.y << ", " << v.z << ")>";
  }

  static void min(Vec3<T> &out, const Vec3<T> &a, const Vec3<T> &b) {
    out.x = std::min(a.x, b.x);
    out.y = std::min(a.y, b.y);
    out.z = std::min(a.z, b.z);
  }

  static void max(Vec3<T> &out, const Vec3<T> &a, const Vec3<T> &b) {
    out.x = std::max(a.x, b.x);
    out.y = std::max(a.y, b.y);
    out.z = std::max(a.z, b.z);
  }

  static Vec3<T> max(const Vec3<T> &a, const Vec3<T> &b) {
    Vec3 out;
    max(out, a, b);
    return out;
  }

  static Vec3<T> min(const Vec3 &a, const Vec3 &b) {
    Vec3 out;
    max(out, a, b);
    return out;
  }

  void min(const Vec3<T> &other) {
    x = std::min(x, other.x);
    y = std::min(y, other.y);
    z = std::min(z, other.z);
  }

  void max(const Vec3<T> &other) {
    x = std::max(x, other.x);
    y = std::max(y, other.y);
    z = std::max(z, other.z);
  }

  T dot(const Vec3<T> &other) const {
    return x * other.x + y * other.y + z * other.z;
  }

  Vec3<T> cross(const Vec3<T> &other) const {
    return {y * other.z - z * other.y, z * other.x - x * other.z,
            x * other.y - y * other.x};
  }

  T length_squared() const { return x * x + y * y + z * z; }

  T length() const { return sqrt(length_squared()); }

  void normalize() { *this /= length(); }

  Vec3<T> normalized() const {
    T l = length();
    return {x / l, y / l, z / l};
  }

  Vec3<T> operator+(const Vec3<T> &other) const {
    return {x + other.x, y + other.y, z + other.z};
  }

  Vec3<T> operator-(const Vec3<T> &other) const {
    return {x - other.x, y - other.y, z - other.z};
  }

  Vec3<T> operator-() const { return {-x, -y, -z}; }

  Vec3<T> operator*(T t) const { return {x * t, y * t, z * t}; }

  friend Vec3<T> operator*(T t, const Vec3<T> &v) { return v * t; }

  Vec3<T> &operator+=(const Vec3<T> &other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  Vec3<T> &operator*=(T t) {
    x *= t;
    y *= t;
    z *= t;
    return *this;
  }

  Vec3<T> &operator/=(T t) {
    x /= t;
    y /= t;
    z /= t;
    return *this;
  }

  Vec3<T> operator/(T t) const { return {x / t, y / t, z / t}; }

  bool operator==(const Vec3<T> &other) const {
    return (x == other.x) && (y == other.y) && (z == other.z);
  }

  bool operator!=(const Vec3<T> &other) const {
    return !(this->operator==(other));
  }

  T &operator[](size_t index) {
    return reinterpret_cast<T *>(this)[index];
  }

  T operator[](size_t index) const {
    return reinterpret_cast<const T *>(this)[index];
  }
};
