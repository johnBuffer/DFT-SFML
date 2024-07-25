#pragma once
#include "engine/engine.hpp"
#include "engine/common/utils.hpp"

#include "user/configuration.hpp"
#include "user/signal.hpp"

#include "user/dft.hpp"
#include "user/machine/cart_wheel.hpp"
#include "user/machine/paint_tank.hpp"
#include "user/wheel_sum.hpp"

#include "user/render_common/card.hpp"
#include "user/render_common/empty_card.hpp"

#include "user/machine/physic_system.hpp"
#include "user/machine/tube.hpp"


struct Renderer : public pez::core::IRenderer
{
    /// Use 1 or 2 DFT to reconstruct the signal
    enum class Mode
    {
        Mono,
        Dual
    };

    Mode mode = Mode::Dual;

    Signal signal;
    Signal signal_x;
    Signal signal_y;

    bool     focus_on_tip_position = false;
    WheelSum cycloid_x;
    WheelSum cycloid_y;
    WheelSum cycloid_mono;

    sf::VertexArray va_signal;
    sf::VertexArray va_dft;

    DFT dft_x;
    DFT dft_y;
    DFT dft_mono;

    float const slow_motion_coef = 0.01f;
    float const time_speed       = 3.0f;
    float       time             = 0.0f;
    bool        slow_mo          = false;

    Tracer tracer;

    float const axe_padding  = 10.0f;
    float const axe_height   = 10.0f;
    float const cycloid_dist = 70.0f;
    EmptyCard axe_x;
    EmptyCard axe_y;

    Card      background;
    EmptyCard background_outline;

    Vec2 const slider_size = {40.0f, 30.0f};
    Card slider_x;
    Card slider_y;
    CartWheel slider_wheel_1;

    bool      draw_tank = true;
    PaintTank tank;
    Tube      tube;

    float const help_margin = 20.0f;
    bool        draw_help   = true;
    sf::Text    text;

    Renderer()
        : signal{0}
        , signal_x{0}
        , signal_y{0}
        , va_signal{sf::PrimitiveType::LineStrip}
        , va_dft{sf::PrimitiveType::LineStrip}
        , axe_x{{axe_height, conf::sim::world_size.y + cycloid_dist + axe_padding}, axe_height * 0.5f, sf::Color::White}
        , axe_y{{conf::sim::world_size.x + cycloid_dist + axe_padding, axe_height}, axe_height * 0.5f, sf::Color::White}
        , background(conf::sim::world_size, 20.0f, {50, 50, 50})
        , background_outline(conf::sim::world_size, 20.0f, sf::Color::White)
        , slider_x{slider_size, 5.0f, sf::Color::White}
        , slider_y{{slider_size.y, slider_size.x}, 5.0f, sf::Color::White}
    {
        // Signal X
        dft_x.setSignal(signal_x.data);

        // Signal Y
        dft_y.setSignal(signal_y.data);

        // Signal mono
        dft_mono.setSignal(signal.data);

        cycloid_x.position    = {0.0f, -conf::sim::world_size.y * 0.5f - cycloid_dist};
        cycloid_y.position    = {-conf::sim::world_size.x * 0.5f - cycloid_dist, 0.0f};
        cycloid_mono.position = {0.0f, 0.0f};

        // Tracer
        tracer.width_start   = 6.0f;
        tracer.width_end     = 1.5f;
        tracer.width_speed   = 1.0f;
        tracer.interpolation = Interpolation::Linear;
        tracer.setColor({231, 111, 81});

        // Axes
        axe_x.setThickness(4.0f);
        axe_y.setThickness(4.0f);
        axe_x.setColor({200, 200, 200});
        axe_y.setColor({200, 200, 200});
        slider_wheel_1.setWheelRadius(9.0f);

        // Background
        background_outline.position = -conf::sim::world_size * 0.5f;
        background.position         = -conf::sim::world_size * 0.5f;

        // Help text
        text.setFont(pez::resources::getFont("font"));
        text.setFillColor(sf::Color::White);
        text.setCharacterSize(20);
        text.setPosition(help_margin, help_margin);
        text.setString("[H] - Toggle help\n"
                       "[S] - Add coefficient\n"
                       "[M] - Switch dual / mono\n"
                       "[R] - Reset time and tracer\n"
                       "[F] - Toggle focus on tip position\n"
                       "[P] - Toggle paint dispenser rendering\n"
                       "[X] - Toggle slow motion\n"
                       "\n"
                       "[Mouse Right] - Draw\n"
                       "[Mouse Left]  - Move viewport\n"
                       "[Mouse Wheel] - Zoom");
    }

    void render(pez::render::Context& context) override
    {
        auto& solver = pez::core::getProcessor<PhysicSystem>().solver;

        // DFT inverse
        if (mode == Mode::Dual) {
            cycloid_x.render(dft_x, time, context);
            cycloid_y.render(dft_y, time, context);
        }

        background_outline.setThickness(10.0f);
        background_outline.render(context);
        background.render(context);

        // If in mono mode, render wheel sum on top of background
        if (mode == Mode::Mono) {
            cycloid_mono.render(dft_mono, time, context);
        }

        va_signal.resize(signal.data.size());
        // Draw input signal if not in reconstruction mode
        if (dft_x.coefficients.empty())
        {
            uint32_t i{0};
            for (DFT::Complex c : signal.data) {
                Vec2 const position{c.real(), c.imag()};
                va_signal[i].position = position;
                va_signal[i].color = signal.getFlag(i) ? sf::Color::White : sf::Color{255, 255, 255, 0};
                ++i;
            }
            context.draw(va_signal);
        }

        if (draw_tank) {
            // Render physic
            tube.render(signal, time, context);
            // Tube rigidity
            tube.updateRigidity(signal, time);

            // Render paint tank
            auto const &pin = solver.segment_pin_constraints[0];
            tank.position = {to<float>(pin.object_pinned->getWorldPosition(pin.pinned_particle).x), -conf::sim::world_size.y * 0.5f - background_outline.thickness * 0.5f};
            tank.render(context);

            // Render
            renderAxes(context);
        }

        Vec2 marker_position = {};
        float const marker_radius{tracer.width_start};
        sf::CircleShape marker{marker_radius};
        marker.setOrigin(marker_radius, marker_radius);
        marker.setOutlineThickness(12.0f);
        marker.setOutlineColor(sf::Color::White);
        marker.setFillColor({0, 0, 0, 0});

        float const status_radius = marker_radius + 4.0f;
        sf::CircleShape marker_status{status_radius};
        marker_status.setOrigin(status_radius, status_radius);
        marker_status.setOutlineThickness(2.0f);
        marker_status.setFillColor({0, 0, 0, 0});
        marker_status.setOutlineColor({0, 100, 0});

        if (!signal.data.empty()) {
            marker_position = getTipPosition();
            solver.drag_constraints[0].setTarget(to<pbd::Vec2D>(marker_position));
            bool const draw = signal.getDraw(time);
            if (draw) {
                marker.setFillColor({231, 111, 81});
                marker_status.setOutlineColor(sf::Color::Green);
            }

            tracer.addPoint(marker_position, draw);
            tracer.render(context);

            tank.active = tube.getSegmentPaintStatus(signal, time, 0);

            if (slow_mo) {
                time += slow_motion_coef * time_speed / (to<float>(signal.data.size()));
            } else {
                time += time_speed / (to<float>(signal.data.size()));
            }
        }

        marker.setPosition(marker_position);
        marker_status.setPosition(marker_position);

        tracer.render(context);

        if (focus_on_tip_position) {
            pez::render::setFocus(marker_position);
        }

        if (draw_tank) {
            context.draw(marker);
            context.draw(marker_status);
        }

        if (draw_help) {
            context.drawDirect(text);
        }
    }

    /// Updates signals for X and Y, only needed in dual mode
    void updateSignals()
    {
        uint32_t i{0};
        signal_x.clear();
        signal_y.clear();
        for (auto const& coord : signal.data) {
            signal_x.addPoint({coord.real(), 0.0f}, signal.getFlag(i));
            signal_y.addPoint({0.0f, coord.imag()}, signal.getFlag(i));
            ++i;
        }
    }

    [[nodiscard]]
    Vec2 getTipPosition() const
    {
        if (mode == Mode::Dual) {
            return Vec2{cycloid_x.tip_position.x, cycloid_y.tip_position.y};
        }
        return cycloid_mono.tip_position;
    }

    void addCoefficient()
    {
        if (mode == Mode::Dual) {
            updateSignals();
        }

        dft_x.addCoefficientPair();
        dft_y.addCoefficientPair();
        dft_mono.addCoefficient();
    }

    void renderAxes(pez::render::Context& context)
    {
        if (mode == Mode::Dual) {
            axe_x.position = cycloid_x.position + Vec2{cycloid_x.tip_position.x - axe_height * 0.5f, cycloid_x.tip_position.y - axe_padding};
            axe_y.position = cycloid_y.position + Vec2{cycloid_y.tip_position.x - axe_padding, cycloid_y.tip_position.y - axe_height * 0.5f};
            slider_wheel_1.position = Vec2{axe_x.position.x + axe_height * 0.5f, conf::sim::world_size.y * 0.5f + background_outline.thickness + slider_wheel_1.wheel_radius};
            slider_wheel_1.render(-(axe_x.position.x), context);
            slider_wheel_1.position = Vec2{axe_x.position.x + axe_height * 0.5f, conf::sim::world_size.y * 0.5f - slider_wheel_1.wheel_radius};
            slider_wheel_1.render(axe_x.position.x, context);
            slider_wheel_1.position = Vec2{conf::sim::world_size.x * 0.5f + background_outline.thickness + slider_wheel_1.wheel_radius, axe_y.position.y + axe_height * 0.5f};
            slider_wheel_1.render(axe_y.position.y, context);
            slider_wheel_1.position = Vec2{conf::sim::world_size.x * 0.5f - slider_wheel_1.wheel_radius, axe_y.position.y + axe_height * 0.5f};
            slider_wheel_1.render(-(axe_y.position.y), context);
            axe_x.render(context);
            axe_y.render(context);
            slider_x.position = Vec2{axe_x.position.x + axe_height * 0.5f, conf::sim::world_size.y * 0.5f + background_outline.thickness * 0.5f} - slider_size * 0.5f;
            slider_x.render(context);
            slider_y.position = Vec2{conf::sim::world_size.x * 0.5f + background_outline.thickness * 0.5f, axe_y.position.y + axe_height * 0.5f} - Vec2{slider_size.y, slider_size.x} * 0.5f;
            slider_y.render(context);
        }
    }
};
