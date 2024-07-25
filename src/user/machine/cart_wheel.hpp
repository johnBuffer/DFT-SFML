#pragma once
#include "engine/engine.hpp"
#include "engine/common/math.hpp"
#include <SFML/Graphics.hpp>

struct CartWheel
{
    sf::Sprite      sprite;
    Vec2            position;
    sf::VertexArray shadow;

    float wheel_radius = 30.0f;

    CartWheel()
        : shadow{sf::PrimitiveType::TriangleFan}
    {
        setWheelRadius(wheel_radius);

        uint32_t const points_count     = 32;
        float const    shadow_thickness = 0.25f;
        shadow.resize(points_count + 1);
        shadow[0].position = {0.0f, 0.0f};
        shadow[0].color    = {20, 20, 20, 100};
        for (uint32_t i{0}; i < points_count; ++i) {
            float const da = Math::ConstantF32::TwoPi / to<float>(points_count - 1);
            float const a  = to<float>(i) * da;
            shadow[i + 1].position = (1.0f + shadow_thickness) * Vec2{cos(a), sin(a)};
            shadow[i + 1].color    = sf::Color{0, 0, 0, 0};
        }
    }

    void setWheelRadius(float radius)
    {
        auto const& texture = pez::resources::getTexture("wheel");
        wheel_radius = radius;
        auto const texture_size = to<Vec2>(texture.getSize());
        sprite.setTexture(texture);
        sprite.setOrigin(texture_size * 0.5f);
        float const scale = 2.0f * wheel_radius / texture_size.x;
        sprite.setScale(scale, scale);
    }

    void render(float dist, pez::render::Context& context)
    {
        sf::Transform transform;
        transform.translate(position);
        transform.scale(wheel_radius, wheel_radius);
        context.draw(shadow, transform);

        sprite.setPosition(position);
        sprite.setRotation(Math::radToDeg(dist / wheel_radius));
        context.draw(sprite);
    }
};

