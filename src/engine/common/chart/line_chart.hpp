#pragma once
#include <SFML/Graphics.hpp>

#include "engine/engine.hpp"
#include "engine/common/math.hpp"
#include "engine/common/racc.hpp"
#include "engine/common/utils.hpp"
#include "engine/render/render_context.hpp"

#include "./utils.hpp"


struct LineChart
{
    // Data
    RAccBase<float> values;
    Vec2            extremes;

    sf::Vector2f    position;
    sf::Vector2f    size;

    sf::VertexArray va_area;
    sf::VertexArray va_line;

    uint32_t values_added = 0;

    sf::Color color;

    float line_thickness = 2.0f;

    bool draw_area = true;
    bool draw_line = true;

    explicit
    LineChart(sf::Vector2f size_)
        : values(100)
        , size{size_}
        , va_area{sf::PrimitiveType::TriangleStrip}
        , va_line{sf::PrimitiveType::TriangleStrip}
        , color{sf::Color::Red}
    {}

    void clear()
    {
        values.clear();
        values_added = 0;
        extremes = {0.0f, 0.0f};
        va_line.clear();
        va_area.clear();
    }

    void addValue(float new_value) {
        ++values_added;

        values.addValueBase(new_value);

        if (new_value < extremes.x) {
            extremes.x = new_value;
        }
        if (new_value > extremes.y) {
            extremes.y = new_value;
        }

        uint32_t const count = values.getCount();
        float const alpha_max_height = 1.0f / std::max(std::abs(extremes.x), std::abs(extremes.y));
        float const dx = size.x / float(count - 1);

        va_area.resize(2 * count);
        values.foreach([&](uint32_t i, float v) {
            float const normalized_x = to<float>(i) * dx + position.x;
            float const scaled_y     = getScaledY(v);
            va_area[2 * i].position = {normalized_x, scaled_y};

            float const alpha_ratio = std::abs(v) * alpha_max_height;
            auto const  alpha       = to<uint8_t>(std::min(1.0f, 2.0f * alpha_ratio) * 150.0f);
            sf::Color const area_color{color.r, color.g, color.b, alpha};
            va_area[2 * i].color = area_color;

            va_area[2 * i + 1].position = {normalized_x, getScaledY(0.0f)};
            va_area[2 * i + 1].color = {color.r, color.g, color.b, 0};
        });

        if (draw_line) {
            generateLineStrip([this](uint32_t i) { return getPoint(i); }, count, va_line, line_thickness, color);
        }
    }

    [[nodiscard]]
    Vec2 getPoint(uint32_t i) const
    {
        return va_area[2 * i].position;
    }

    void render(pez::render::Context& context)
    {
        if (draw_area) {
            context.draw(va_area);
        }

        if (draw_line) {
            context.draw(va_line);
        }
    }

    [[nodiscard]]
    uint32_t getGlobalValueIndex(uint32_t data_buffer_idx) const
    {
        return data_buffer_idx + ((values_added < values.max_values_count) ? 0 : (values_added - values.max_values_count));
    }

    [[nodiscard]]
    float getScaledY(float y) const
    {
        float const width = extremes.y - extremes.x;
        float const normalized_y = (y - extremes.x) / width;
        float const scaled_y = normalized_y * size.y;
        return size.y - scaled_y + position.y;
    }

    void setColor(sf::Color c)
    {
        color = c;
    }
};
