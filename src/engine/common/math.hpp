#pragma once
#include <algorithm>
#include <cmath>

#undef min
#undef max

struct Math
{
    template<typename TFloat>
    struct Constant
    {
        static constexpr TFloat Pi = static_cast<TFloat>(3.141592653589793238462643383279502884197);
        static constexpr TFloat TwoPi = 2.0 * Pi;
    };

    using ConstantF32 = Constant<float>;
    using ConstantF64 = Constant<double>;

    static float pow(float v, const uint32_t p)
    {
        float res = 1;
        for (uint32_t i{p}; i--;) {
            res *= v;
        }
        return res;
    }

    template<typename T>
    static T sign(T v)
    {
        return v < 0.0f ? -1.0f : 1.0f;
    }
    
    static float sigm(float x)
    {
        return 1.0f / (1.0f + exp(-x));
    }
    
    static float sigm_0(float x)
    {
        return sigm(x) - 0.5f;
    }

    static float radToDeg(float r)
    {
        constexpr float radian_to_deg = 180.0f / ConstantF32::Pi;
        return r * radian_to_deg;
    }

    static float clamp(float v, float min, float max)
    {
        return std::min(std::max(min, v), max);
    }

    static float gaussian(float x, float a, float b, float c)
    {
        const float n = x-b;
        return a * exp(-(n * n)/(2.0f * c * c));
    }
};


struct MathVec2
{
    template<template<typename> class Vec2Type, typename T>
    static T length2(Vec2Type<T> v)
    {
        return v.x * v.x + v.y * v.y;
    }


    template<template<typename> class Vec2Type, typename T>
    static T length(Vec2Type<T> v)
    {
        return sqrt(length2(v));
    }

    template<template<typename> class Vec2Type, typename T>
    static T angle(Vec2Type<T> v_1, Vec2Type<T> v_2 = {1.0, 0.0})
    {
        const T dot = v_1.x * v_2.x + v_1.y * v_2.y;
        const T det = v_1.x * v_2.y - v_1.y * v_2.x;
        return atan2(det, dot);
    }

    template<template<typename> class Vec2Type, typename T>
    static T dot(Vec2Type<T> v1, Vec2Type<T> v2)
    {
        return v1.x * v2.x + v1.y * v2.y;
    }

    template<template<typename> class Vec2Type, typename T>
    static T cross(Vec2Type<T> v1, Vec2Type<T> v2)
    {
        return v1.x * v2.y - v1.y * v2.x;
    }
    
    template<typename Vec2Type>
    static Vec2Type normal(const Vec2Type& v)
    {
        return {-v.y, v.x};
    }

    template<typename Vec2Type>
    static Vec2Type rotate(const Vec2Type& v, float angle)
    {
        const float ca = cos(angle);
        const float sa = sin(angle);
        return {ca * v.x - sa * v.y, sa * v.x + ca * v.y};
    }

    template<typename Vec2Type>
    static Vec2Type rotateDir(const Vec2Type& v, const Vec2Type& dir)
    {
        return { dir.x * v.x - dir.y * v.y, dir.y * v.x + dir.x * v.y };
    }

    template<typename Vec2Type>
    static Vec2Type normalize(const Vec2Type& v)
    {
        return v / length(v);
    }

    template<template<typename> class Vec2Type, typename T>
    static Vec2Type<T> reflect(const Vec2Type<T>& v, const Vec2Type<T>& n)
    {
        return v - n * (MathVec2::dot(v, n) * 2.0f);
    }

    template<template<typename> class Vec2Type, typename T>
    static T distToLine(const Vec2Type<T>& point, const Vec2Type<T>& direction, const Vec2Type<T>& origin)
    {
        const auto        u0 = point - origin;
        const Vec2Type<T> u1 = MathVec2::dot(u0, direction) * direction;
        const Vec2Type<T> u2 = u0 - u1;
        return MathVec2::length(u2);
    }

    template<template<typename> class Vec2Type, typename T>
    static Vec2Type<T> closestSegmentPoint(const Vec2Type<T>& point, const Vec2Type<T>& segment_point_1, const Vec2Type<T>& segment_point_2)
    {
        auto const segment_v   = segment_point_2 - segment_point_1;
        auto const segment_len = MathVec2::length(segment_v);
        auto const segment_dir = segment_v / segment_len;
        auto const segment_nrm = MathVec2::normal(segment_dir);

        T const dist_to_line = distToLine(point, segment_dir, segment_point_1);

        T const side = MathVec2::dot(segment_point_1 - point, segment_nrm);
        Vec2Type<T> const projected = point + Math::sign(side) * segment_nrm * dist_to_line;
        T const dist_to_segment_point_1 = MathVec2::dot(segment_dir, projected - segment_point_1);
        if (dist_to_segment_point_1 < T{0}) {
            return segment_point_1;
        } else if (dist_to_segment_point_1 > segment_len) {
            return segment_point_2;
        }
        return projected;
    }

    template<template<typename> class Vec2Type, typename T>
    static Vec2Type<T> getIntersection(const Vec2Type<T>& p1, const Vec2Type<T>& n1, const Vec2Type<T>& p2, const Vec2Type<T>& n2)
    {
        Vec2Type<T> const p1End = p1 + n1;
        Vec2Type<T> const p2End = p2 + n2;

        T const m1 = (p1End.y - p1.y) / (p1End.x - p1.x);
        T const m2 = (p2End.y - p2.y) / (p2End.x - p2.x);
        T const b1 = p1.y - m1 * p1.x;
        T const b2 = p2.y - m2 * p2.x;
        T const px = (b2 - b1) / (m1 - m2);
        T const py = m1 * px + b1;

        return {px, py};
    }
};

