#pragma once
#include "../object.hpp"
#include "../anchor.hpp"
#include "../configuration.hpp"

#include "./constraint.hpp"

namespace pbd
{

struct AngularConstraint
{
    Constraint constraint;

    siv::Ref<Object> o_1;
    siv::Ref<Object> o_2;

    pbd::RealType angle = 0.0;

    void create(siv::Ref<Object> obj_1, siv::Ref<Object> obj_2, pbd::RealType target_angle)
    {
        o_1 = obj_1;
        o_2 = obj_2;
        angle = target_angle;
    }

    void solve(RealType dt)
    {
        Vec2D n_1 = {cos(o_1->angle), sin(o_1->angle)};
        Vec2D n_2 = {cos(o_2->angle), sin(o_2->angle)};

        pbd::RealType const d_a = angle - MathVec2::angle(n_1, n_2);
        pbd::RealType const w1 = o_1->inv_inertia_tensor;
        pbd::RealType const w2 = o_2->inv_inertia_tensor;
        pbd::RealType const a            = constraint.compliance / (dt * dt);
        pbd::RealType const delta_lambda = (-d_a - a * constraint.lambda) / (w1 + w2 + a);
        constraint.lambda = delta_lambda;

        o_1->applyRotationCorrection( delta_lambda);
        o_2->applyRotationCorrection(-delta_lambda);
    }
};

}
