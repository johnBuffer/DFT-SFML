#pragma once
#include <complex>
#include "engine/common/math.hpp"
#include "engine/common/binary_io.hpp"
#include "user/configuration.hpp"
#include "user/dft.hpp"


/**
 * Represents a sequence of 2D points stored as complex numbers to be used by DFT.
 */
struct Signal
{
    std::vector<DFT::Complex>  data;
    std::vector<uint32_t> flags;

    uint32_t points_count = 0;

    Signal() = default;

    explicit
    Signal(uint32_t size)
        : data(size)
    {
    }

    void addPoint(Vec2 point, bool draw = true)
    {
        // Remove closing loop padding points
        data.resize(points_count);
        flags.resize(points_count);
        // Add the new sample
        data.emplace_back(point.x, point.y);
        flags.push_back(draw);
        ++points_count;
    }

    void addPointFill(Vec2 point, bool draw = true)
    {
        if (points_count) {
            fill(getVec2(points_count - 1), point, conf::signal::padding_sampling_dist, true, draw);
        } else {
            addPoint(point, draw);
        }
    }

    void closeLoop()
    {
        if (points_count > 1) {
            DFT::Complex const pt_1{data.back()};
            DFT::Complex const pt_2{data.front()};
            fill({pt_1.real(), pt_1.imag()}, {pt_2.real(), pt_2.imag()}, conf::signal::padding_sampling_dist, false, false);
        }
    }

    void fill(Vec2 point_1, Vec2 point_2, float sampling_dist, bool is_data, bool draw)
    {
        Vec2 const  v          = point_2 - point_1;
        float const dist       = MathVec2::length(v);
        auto const  step_count = to<uint32_t>(dist / sampling_dist) + 1;

        for (uint32_t i{1}; i < step_count; ++i) {
            Vec2 const next_point = point_1 + MathVec2::normalize(v) * sampling_dist * to<float>(i);

            if (is_data) {
                addPoint(next_point, draw);
            } else {
                data.emplace_back(next_point.x, next_point.y);
                flags.push_back(false);
            }
        }
    }

    [[nodiscard]]
    Vec2 getVec2(uint32_t i) const
    {
        DFT::Complex const& c{data[i]};
        return {c.real(), c.imag()};
    }

    [[nodiscard]]
    Vec2 getLastPoint() const
    {
        DFT::Complex const& c{data.back()};
        return {c.real(), c.imag()};
    }

    void clear()
    {
        data.clear();
        points_count = 0;
    }

    void writeToFile(std::string const& filename) const
    {
        BinaryWriter writer(filename);
        writer.write(data.size());
        for (auto const& p : data) {
            writer.write(p);
        }
    }

    void loadFromFile(std::string const& filename)
    {
        BinaryReader reader(filename);
        auto const count = reader.read<uint64_t>();
        data.resize(count);
        for (uint64_t i{0}; i < count; ++i) {
            reader.readInto(data[i]);
        }
    }

    [[nodiscard]]
    uint32_t getFlag(size_t i) const
    {
        return flags[i];
    }

    [[nodiscard]]
    bool getDraw(float time) const
    {
        float const time_ratio = fmod(time / Math::ConstantF32::TwoPi, 1.0f);
        auto const idx = to<uint32_t>(to<float>(data.size()) * time_ratio);
        return getFlag(idx);
    }
};
