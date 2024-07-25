#pragma once

#include "engine/common/vec.hpp"

#include "./constraint.hpp"

namespace pbd
{

struct SegmentCollisionConstraint
{
    Vec2 pt_1;
    Vec2 pt_2;
    float radius;
    siv::Ref<Object> obj = {};

    Constraint constraint;

    void create(siv::Ref<Object> object, Vec2 point_1, Vec2 point_2, float radius_)
    {
        pt_1   = point_1;
        pt_2   = point_2;
        radius = radius_;
        obj    = object;
    }

    void solve(float dt)
    {
        Vec2 const closest_point = MathVec2::closestSegmentPoint(obj->position, pt_1, pt_2);

        Vec2 const v = obj->position - closest_point;

        float const dist_to_segment = MathVec2::length(v);

        if (dist_to_segment < radius) {
            Vec2  const normal = v / dist_to_segment;
            Vec2  const r_1     = -normal * radius;
            float const w_1     = obj->getGeneralizedInvMass(r_1, normal);

            float const a            = constraint.compliance / (dt * dt);
            float const delta_lambda = (radius - dist_to_segment)  / (w_1 + a);

            Vec2 const p = delta_lambda * normal;
            obj->applyPositionCorrection(p, r_1);
        }
    }
};

}