#pragma once
#include "engine/engine.hpp"
#include "engine/common/smooth/smooth_value.hpp"


struct Tracer
{
    sf::Color                color = sf::Color::White;
    std::vector<Vec2>        points;
    std::vector<SmoothFloat> width;
    std::vector<uint32_t>    draw;
    sf::VertexArray          va_line;

    float width_start = 4.0f;
    float width_end   = 0.0f;
    float width_speed = 0.0f;
    Interpolation interpolation = Interpolation::EaseInOutQuint;

    explicit
    Tracer()
        : va_line{sf::PrimitiveType::TriangleStrip}
    {}

    void setColor(sf::Color c)
    {
        color = c;
    }

    void clear()
    {
        points.clear();
        width.clear();
    }

    void addPoint(Vec2 pt, bool draw_)
    {
        points.push_back(pt);

        auto& w = width.emplace_back();
        w.setValueInstant(draw_ ? width_start : 0.0f);
        w.setInterpolationFunction(interpolation);
        w.setSpeed(width_speed);
        if (draw_) {
            w = width_end;
        }
    }

    void render(pez::render::Context& context)
    {
        if (points.empty()) {
            return;
        }

        va_line.resize(2 * points.size() - 2);

        if (points.size() > 1) {
            Vec2 last = points[0];
            uint32_t i{0};
            for (auto const& pt: points) {
                if (i > 0) {
                    Vec2 const current = pt;
                    Vec2 const d = current - last;
                    Vec2 const n = MathVec2::normalize(MathVec2::normal(d));
                    float const w = width[i].get();
                    va_line[2 * (i - 1) + 0].position = current + w * n;
                    va_line[2 * (i - 1) + 1].position = current - w * n;
                    va_line[2 * (i - 1) + 0].color = color;
                    va_line[2 * (i - 1) + 1].color = color;
                    last = current;
                }
                ++i;
            }
            context.draw(va_line);
        }

        float const radius{width.back().get()};
        sf::CircleShape tip{radius};
        tip.setOrigin(radius, radius);
        tip.setPosition(points.back());
        tip.setFillColor(color);
        context.draw(tip);
    }
};
