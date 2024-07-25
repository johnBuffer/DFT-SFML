#pragma once
#include "./card.hpp"


struct CardOutline
{
    float outline_thickness;
    Card  outline;
    Card  background;
    Vec2  position = {};

    CardOutline(Vec2 size, float radius, float outline_thickness_ = 5.0f, sf::Color fill_color = {50, 50, 50}, sf::Color outline_color = sf::Color::White)
        : outline_thickness{outline_thickness_}
        , outline(size + 2.0f * Vec2{outline_thickness, outline_thickness}, radius + outline_thickness, outline_color)
        , background{size, radius, fill_color}
    {

    }

    void setPosition(Vec2 position_)
    {
        position            = position_;
        outline.position    = position - Vec2{outline_thickness, outline_thickness};
        background.position = position;
    }

    void render(pez::render::Context& context)
    {
        outline.render(context);
        background.render(context);
    }
};
