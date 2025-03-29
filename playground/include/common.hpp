// common.hpp

#pragma once

#include <cassert>
#include <cmath>
#include <vector>
#include <string_view>
#include <raylib.h>
#include <raymath.h>

namespace sim
{
    namespace Math
    {
        template <typename T> constexpr T min(T lhs, T rhs) { return lhs < rhs ? lhs : rhs; }
        template <typename T> constexpr T max(T lhs, T rhs) { return lhs > rhs ? lhs : rhs; }
        template <typename T> constexpr T clamp(T val, T lhs, T rhs) { return min(rhs, max(lhs, val)); }
        template <typename T> constexpr T lerp(T a, T b, float t) { return T(a + (b - a) * t); }
        template <typename T> constexpr T wrap(const T v, const T w) { return v > w ? (v - w) : v; }
        template <typename T> constexpr T sign(T v) { return T((T(0) < v) - (v < T(0))); }
    }

    struct Point {
        constexpr Point() = default;
        constexpr Point(int x, int y) : x(x), y(y) {}
        template <typename U, typename V>
        constexpr Point(U x, V y) : x(int(x)), y(int(y)) {}
        constexpr Point(const Vector2& rhs) : x(int(rhs.x)), y(int(rhs.y)) {}

        constexpr Point operator+ (const Point& rhs) const { return Point{ x + rhs.x, y + rhs.y }; }
        constexpr Point operator- (const Point& rhs) const { return Point{ x - rhs.x, y - rhs.y }; }
        constexpr Point operator* (const Point& rhs) const { return Point{ x * rhs.x, y * rhs.y }; }
        constexpr Point operator/ (const Point& rhs) const { return Point{ x / rhs.x, y / rhs.y }; }

        constexpr bool is_zero() const { return x == 0 && y == 0; }
        constexpr bool has_negative() const { return x < 0 || y < 0; }
        constexpr Vector2 to_vec2() const { return Vector2{ float(x), float(y) }; }

        int x = 0;
        int y = 0;
        bool operator==(const Point& other) const {
            return x == other.x && y == other.y;
        }// ADD: Compare two points' equality
    };
}