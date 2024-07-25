#pragma once
#include <SFML/Graphics.hpp>
#include "engine/engine.hpp"

/**
 * 2D representation of one DFT coefficient
 */
struct Wheel : public sf::Drawable
{
    uint32_t const points_count  = 64;
    float const    outline       = 0.05f;
    float const    cycle_radius  = 1.0f - outline;
    float const    pin_radius    = 0.04f;

    sf::CircleShape cycle;
    sf::CircleShape pin;

    sf::Font& font;
    mutable sf::Text text;

    float div = 1.0f;
    DFT::Coef coef;

    mutable sf::Color text_color;

    Wheel()
        : cycle{cycle_radius}
        , pin{pin_radius}
        , font{pez::resources::getFont("font")}
    {
        // The main wheel background
        cycle.setOrigin(cycle_radius, cycle_radius);
        cycle.setOutlineThickness(outline);
        cycle.setOutlineColor(sf::Color{240, 240, 240});
        cycle.setPointCount(points_count);

        // The center of the wheel
        pin.setOrigin(pin_radius, pin_radius);
        pin.setFillColor({200, 200, 200});
        pin.setPointCount(points_count);
        pin.setOutlineThickness(pin_radius * 0.2f);
        pin.setOutlineColor({240, 240, 240});

        // Text configuration
        float const text_scale{0.001f};
        text_color = {140, 140, 140};
        text.setFont(font);
        text.setFillColor(text_color);
        text.setCharacterSize(120);
        text.setScale(text_scale, text_scale);
    }

    /** Renders the wheel
     *
     * @param target The SFML target to render to
     * @param states The SFML states to apply (transform used here)
     */
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(cycle, states);
        target.draw(pin, states);

        float const norm = coef.getNorm() * div;
        if (norm < 1.0f) {
            return;
        }

        text.setFillColor({140, 140, 140});

        float const space = 0.1f;
        float end_a = drawText(0.0f, 1.4f, cycle_radius, toString(coef.i), target, states.transform);
        drawText(end_a + space, 0.3f, cycle_radius - 0.04f, "amplitude", target, states.transform);
        end_a = drawText(end_a + space, 0.75f, cycle_radius - 0.07f, toString(norm), target, states.transform);
        drawText(end_a + space, 0.3f, cycle_radius - 0.04f, "phase", target, states.transform);
        drawText(end_a + space, 0.75f, cycle_radius - 0.07f, toString(coef.getArg()), target, states.transform);

        text.setFillColor({200, 200, 200});
        drawText(0.0f, 0.25f, cycle_radius - 0.8f, (coef.i > 0) ? "rotates this way >>>" : "<<< rotates this way", target, states.transform);
        drawText(Math::ConstantF32::Pi, 0.25f, cycle_radius - 0.8f, "Mind your fingers", target, states.transform);

        drawText(Math::ConstantF32::Pi, 0.5f, cycle_radius, "WARNING: sensitive electronic device // pezzza Inc. 2024", target, states.transform);
    }

    /** Renders curved text, following an arc
     *
     * @param start_angle The start angle of the arc
     * @param scale The scale
     * @param radius The radius of the arc
     * @param str The string to render
     * @param target SFML to target to draw on
     * @param transform 2D transformation to apply
     * @return The angle at which the text ends, useful to draw another text after this one
     */
    float drawText(float start_angle, float scale, float radius, std::string const& str, sf::RenderTarget& target, sf::Transform const& transform) const
    {
        /* !! VERY UNOPTIMIZED !!
         * requires a draw call per char
         */

        // Magic value that could be removed
        float const base_scale = 0.001f;
        float const s          = base_scale * scale;
        float a = start_angle - Math::ConstantF32::Pi * 0.5f;
        for (char c : str) {
            float const advance = font.getGlyph(c, 120, false).advance * s * 1.1f;
            text.setString(c);
            text.setPosition(radius * Vec2{cos(a), sin(a)});
            text.setRotation(Math::radToDeg(a) + 90.0f);
            text.setScale(s, s);
            target.draw(text, transform);
            a += advance / radius;
        }
        return a + Math::ConstantF32::Pi * 0.5f;
    }
};