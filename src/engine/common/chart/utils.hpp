#pragma once
#include <SFML/Graphics.hpp>
#include "engine/common/vec.hpp"
#include "engine/common/math.hpp"


template<typename TAccessCallback>
void generateLineStrip(TAccessCallback&& callback, uint64_t points_count, sf::VertexArray& va, float thickness, sf::Color color)
{
    if (points_count > 1) {
        va.resize(2 * points_count);
        va.setPrimitiveType(sf::PrimitiveType::TriangleStrip);

        Vec2 last_v{MathVec2::normalize(callback(1) - callback(0))};
        Vec2 last_n{MathVec2::normal(last_v)};

        va[0].position = callback(0) + Vec2{0, thickness};
        va[0].color = color;
        va[1].position = callback(0) - Vec2{0, thickness};
        va[1].color = color;

        for (uint32_t i{2}; i < points_count; ++i) {
            Vec2 const v = MathVec2::normalize(callback(i) - callback(i - 1));
            Vec2 const n = MathVec2::normal(v);

            float const dot = MathVec2::dot(last_v, v);

            auto const computeGeometry = [&](uint32_t idx, float normal_direction, float dot_result) {
                constexpr float threshold = 0.9f;
                float const     normal    = thickness * normal_direction;

                if (dot < threshold) {
                    Vec2 const pt_1 = callback(i) + n * normal;
                    Vec2 const pt_2 = callback(i - 1) + last_n * normal;
                    va[idx].position = MathVec2::getIntersection(pt_1, v, pt_2, last_v);
                    va[idx].color = color;
                } else {
                    Vec2 const pt_1 = callback(i - 1) + n * normal;
                    va[idx].position = pt_1;
                    va[idx].color = color;
                }
            };

            computeGeometry(2 * (i - 1)    ,  1.0f, dot);
            computeGeometry(2 * (i - 1) + 1, -1.0f, dot);

            last_v = v;
            last_n = n;
        }

        va[2 * (points_count - 1)].position = callback(points_count - 1) + thickness * Vec2{0, 1};
        va[2 * (points_count - 1)].color = color;
        va[2 * (points_count - 1) + 1].position = callback(points_count - 1) - thickness * Vec2{0, 1};
        va[2 * (points_count - 1) + 1].color = color;
    }
}
