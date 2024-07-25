#pragma once
#include "engine/engine.hpp"
#include "user/physic/solver.hpp"

/** A physics based tube connecting the paint tank to the cursor.
 *
 */
struct Tube
{
    // Skip first object as it is not part of the tube
    uint32_t const     object_offset = 1;
    // Period coefficient describing how much of a period is represented in the tube
    double const       period_coef = 0.3f;
    // The physics solver, provided by the physics processor
    pbd::Solver&      solver;
    // The number of segments, provided by the solver
    size_t const      segments_count;
    // The vertex array used to draw the tube
    sf::VertexArray   va_segments;

    Tube()
        : solver{pez::core::getProcessor<PhysicSystem>().solver}
        , segments_count(solver.objects.size() - object_offset)
        , va_segments{sf::PrimitiveType::TriangleStrip, 2 * segments_count}
    {

    }

    /** Checks if the segment @p i is filled with paint given the current @p time
     *
     * @param signal The current signal
     * @param time The current time
     * @param i The index of the segement
     * @return A boolean describing the segment's paint state
     */
    [[nodiscard]]
    bool getSegmentPaintStatus(Signal const& signal, float time, uint32_t i) const
    {
        auto const   signal_size = signal.data.size();
        float const  time_ratio  = fmod(time / Math::ConstantF32::TwoPi, 1.0f);
        size_t const tube_idx    = segments_count - i;
        auto const   idx         = to<uint32_t>(to<float>(signal_size) * time_ratio + to<float>(tube_idx) * period_coef) % signal_size;
        return signal.getFlag(idx);
    }

    /** Update the tube's rigidity depending on paint presence
     *
     * @param signal The current signal
     * @param time The current time
     */
    void updateRigidity(Signal const& signal, float time)
    {
        if (!signal.data.empty()) {
            double const compliance = 0.00000001;

            uint32_t i{0};
            for (auto& c : solver.angular_constraints) {
                bool const draw = getSegmentPaintStatus(signal, time, i);
                if (draw) {
                    c.constraint.compliance = compliance * 0.35;
                } else {
                    c.constraint.compliance = compliance;
                }
                ++i;
            }
        }
    }

    /** Renders the tube
     *
     * @param signal The current signal
     * @param time The current time
     * @param context The render context to use
     */
    void render(Signal const& signal, float time, pez::render::Context& context)
    {
        renderTube(8.0, sf::Color::White, context);

        if (!signal.data.empty()) {
            renderTube(6.0, {50, 50, 50}, context);
            // Generate paint
            renderTubeCallback(6.0, [&](uint32_t i) -> sf::Color {
                return getSegmentPaintStatus(signal, time, i) ? sf::Color{231, 111, 81} : sf::Color{231, 111, 81, 0};
            }, context);
            renderTube(6.0, {255, 255, 255, 100}, context);
        }
    }

private:
    /** Renders an path matching the tube's current geometry with customizable color per segment
     *
     * @tparam TCallback Any callable that takes an @p uint32_t and returns a @p sf::Color
     * @param width The width of the path
     * @param color_callback A callback that tells the color of each segment
     * @param context The context to use to draw the path
     */
    template<typename TCallback>
    void renderTubeCallback(pbd::RealType width, TCallback&& color_callback, pez::render::Context& context)
    {
        uint32_t const  offset = 1;

        auto const& objects = solver.objects.getData();

        pbd::Vec2D current;
        pbd::Vec2D d;
        pbd::Vec2D n;
        pbd::Vec2D last = objects[offset].getWorldPosition(0);

        for (uint32_t i{offset}; i < segments_count + offset; ++i) {
            auto const pt = objects[i];
            current = pt.getWorldPosition(1);
            d = current - last;
            n = MathVec2::normalize(MathVec2::normal(d));
            sf::Color const color = color_callback(i);
            va_segments[2 * (i - offset) + 0].position = to<Vec2>(last + width * n);
            va_segments[2 * (i - offset) + 1].position = to<Vec2>(last - width * n);
            va_segments[2 * (i - offset) + 0].color = color;
            va_segments[2 * (i - offset) + 1].color = color;
            last = current;
        }
        context.draw(va_segments);
    }

    /// Helper function that draw a path with one color
    void renderTube(pbd::RealType width, sf::Color color, pez::render::Context& context)
    {
        renderTubeCallback(width, [&](uint32_t) { return  color; }, context);
    }
};