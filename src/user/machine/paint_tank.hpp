#pragma once
#include "user/render_common/card.hpp"
#include "cart_wheel.hpp"


struct PaintTank
{
    Vec2 size = {150.0f, 100.0f};
    float radius = 10.0f;

    Card back;

    Vec2 position = {};

    float const space  = 10.0f;
    float const offset = 5.0f;

    CartWheel wheel_1;
    CartWheel wheel_2;
    CartWheel wheel_3;
    CartWheel wheel_4;

    sf::Text text;

    bool active = false;

    PaintTank()
        : back{size, radius, sf::Color::White}
    {
        float const wheel_radius_top = 20.0f;
        float const wheel_radius_bot = 12.0f;
        wheel_1.setWheelRadius(wheel_radius_top);
        wheel_2.setWheelRadius(wheel_radius_bot);
        wheel_3.setWheelRadius(wheel_radius_top);
        wheel_4.setWheelRadius(wheel_radius_bot);

        // Background
        back.shadow_size = 4.0f;
        back.updateCard();

        // Text
        text.setFont(pez::resources::getFont("font"));
        text.setFillColor({100, 100, 100});
        text.setCharacterSize(16);
        text.setString("Paint\ndispenser");
    }

    void render(pez::render::Context& context)
    {
        back.position = position - size * 0.5f - Vec2{0.0f, 10.0f};

        wheel_1.position = Vec2{back.position.x + offset, -conf::sim::world_size.y * 0.5f - wheel_1.wheel_radius - space};
        wheel_1.render(back.position.x, context);

        wheel_2.position = Vec2{back.position.x + offset, -conf::sim::world_size.y * 0.5f + wheel_2.wheel_radius};
        wheel_2.render(-back.position.x, context);

        wheel_3.position = Vec2{back.position.x + size.x - offset, -conf::sim::world_size.y * 0.5f - wheel_3.wheel_radius - space};
        wheel_3.render(back.position.x, context);

        wheel_4.position = Vec2{back.position.x + size.x - offset, -conf::sim::world_size.y * 0.5f + wheel_4.wheel_radius};
        wheel_4.render(-back.position.x, context);

        back.render(context);

        Vec2 const text_offset = {7.0f, 58.0f};
        text.setPosition(back.position + text_offset);
        context.draw(text);

        float const radius_led{4.0f};
        sf::CircleShape led(radius_led);
        led.setOrigin(radius_led, radius_led);

        float const radius_coef = 1.5f;
        Vec2 const led_offset = {back.size.x - back.corner_radius * radius_coef, back.corner_radius * radius_coef};
        led.setPosition(back.position + led_offset);
        led.setFillColor(active ? sf::Color::Green : sf::Color{0, 100, 0});
        led.setOutlineThickness(1.0f);
        led.setOutlineColor(sf::Color{200, 200, 200});
        context.draw(led);
    }
};

