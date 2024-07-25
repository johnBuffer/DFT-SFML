#pragma once
#include <SFML/Graphics.hpp>
#include "engine/engine.hpp"

#include "./dft.hpp"
#include "./render_common/tracer.hpp"
#include "./wheel.hpp"


struct WheelSum
{
    sf::Font font;
    sf::Text text;

    sf::VertexArray shadow;

    Vec2   tip_position;

    Vec2 position = {};

    WheelSum()
    {
        // Font
        font.loadFromFile("res/font.ttf");

        // Initialize shadow's vertex array
        generateShadow();
    }

    /** Renders the inverse of the provided @p DFT at time @p t
     *
     * @param dft The DFT to render
     * @param t The current time
     * @param context Render context to use
     */
    void render(DFT const& dft, float t, pez::render::Context& context)
    {
        // Ensure the signal has been set
        if (dft.signal == nullptr) {
            std::cout << "Uninitialized DFT provided, skipping wheel rendering." << std::endl;
            return;
        }

        size_t const samples_count = dft.signal->size();
        float const  div = 1.0f / to<float>(samples_count);

        // The coefficient representation
        Wheel wheel;
        wheel.div = div;

        DFT::Complex current{};
        std::vector<DFT::Coef> const sorted_coef = getSortedCoefficients(dft);

        for (auto const& c : sorted_coef) {
            wheel.coef = c;

            float const radius{c.getNorm() * div};

            sf::Transform transform;
            transform.translate(current.real() + position.x, current.imag() + position.y);
            transform.scale(radius, radius);
            transform.rotate(Math::radToDeg(c.getArg() + to<float>(c.i) * t));
            context.draw(shadow, transform);
            context.draw(wheel, transform);

            float const x = t * to<float>(c.i);
            current += c.v * div * DFT::Complex{cos(x), sin(x)};
        }

        tip_position = {current.real(), current.imag()};
    }

    /** Sorts the coefficients of the provided DFT by descending norm
     *
     * @param dft The DFT to operate on
     * @return A copy of the coefficients, sorted by descending norm
     */
    [[nodiscard]]
    static std::vector<DFT::Coef> getSortedCoefficients(DFT const& dft)
    {
        std::vector<DFT::Coef> result = dft.coefficients;
        // Sort coefficients by descending norm
        std::sort(result.begin(), result.end(), [](DFT::Coef const& c1, DFT::Coef const& c2) {
            return c1.getNorm() > c2.getNorm();
        });
        return result;
    }

    /// Generates the wheel's shadow
    void generateShadow()
    {
        // Number of points for the shadow circle
        uint32_t const points_count = 64;

        shadow.setPrimitiveType(sf::TriangleFan);
        // Shadow
        float const shadow_thickness = 0.1f;
        shadow.resize(points_count + 1);
        shadow[0].position = {0.0f, 0.0f};
        shadow[0].color    = {20, 20, 20, 255};
        for (uint32_t i{0}; i < points_count; ++i) {
            float const da = Math::ConstantF32::TwoPi / to<float>(points_count - 1);
            float const a  = to<float>(i) * da;
            shadow[i + 1].position = (1.0f + shadow_thickness) * Vec2{cos(a), sin(a)};
            shadow[i + 1].color    = sf::Color{20, 20, 20, 0};
        }
    }
};
