#pragma once

#include "engine/common/vec.hpp"

#include "./constraint.hpp"
#include "../configuration.hpp"

namespace pbd
{

struct SegmentPinConstraint
{
    siv::Ref<Object> object_segment;
    siv::Ref<Object> object_pinned;
    uint32_t pinned_particle = 0;

    Constraint constraint;

    void create(siv::Ref<Object> segment, siv::Ref<Object> pinned_object, uint32_t pinned_particle_)
    {
        object_segment  = segment;
        object_pinned   = pinned_object;
        pinned_particle = pinned_particle_;
    }

    void solve(RealType dt)
    {
        Vec2D const pt_1 = object_segment->getWorldPosition(0);
        Vec2D const pt_2 = object_segment->getWorldPosition(1);
        Vec2D const pinned_pt_world = object_pinned->getWorldPosition(pinned_particle);
        Vec2D const closest_point   = MathVec2::closestSegmentPoint(pinned_pt_world, pt_1, pt_2);

        Vec2D const    v    = closest_point - pinned_pt_world;
        RealType const dist = MathVec2::length(v);

        if (dist > 0.0) {
            Vec2D const n   = v / dist;
            Vec2D const r_1 = pinned_pt_world - object_pinned->position;
            RealType const w_1 = object_pinned->getGeneralizedInvMass(r_1,  n);
            RealType const a            = constraint.compliance / (dt * dt);
            RealType const delta_lambda = dist / (w_1 + a);
            constraint.lambda = delta_lambda;

            Vec2D const p = delta_lambda * n;
            object_pinned->applyPositionCorrection(p, r_1);
        }
    }
};

}