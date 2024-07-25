#pragma once
#include "../object.hpp"
#include "../anchor.hpp"

#include "./constraint.hpp"

namespace pbd
{

struct DistanceConstraint
{
    Constraint constraint;

    Anchor anchor_1;
    Anchor anchor_2;

    float target         = 0.0f;
    float current_length = 0.0f;

    void create(Anchor anchor_1_, Anchor anchor_2_, float target_length)
    {
        anchor_1 = anchor_1_;
        anchor_2 = anchor_2_;

        target = target_length;
    }

    void solve(float dt)
    {
        Vec2 const anchor_1_world_position = anchor_1.obj->getWorldPosition(anchor_1.obj_coord);
        Vec2 const anchor_2_world_position = anchor_2.obj->getWorldPosition(anchor_2.obj_coord);

        Vec2 const r1 = anchor_1_world_position - anchor_1.obj->position;
        Vec2 const r2 = anchor_2_world_position - anchor_2.obj->position;

        Vec2  const v  = anchor_1_world_position - anchor_2_world_position;
        current_length = MathVec2::length(v);
        Vec2  const n  = v / current_length;

        float const w1 = anchor_1.obj->getGeneralizedInvMass(r1,  n);
        float const w2 = anchor_2.obj->getGeneralizedInvMass(r2, -n);

        float const a            = constraint.compliance / (dt * dt);
        float const delta_lambda = ((target - current_length) - a * constraint.lambda) / (w1 + w2 + a);
        constraint.lambda       += delta_lambda;

        Vec2 const p = delta_lambda * n;
        anchor_1.obj->applyPositionCorrection( p, r1);
        anchor_2.obj->applyPositionCorrection(-p, r2);

        constraint.force = constraint.lambda / (dt * dt);
    }
};

}
