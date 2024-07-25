#pragma once
#include "../object.hpp"
#include "../anchor.hpp"

#include "./constraint.hpp"

namespace pbd
{

struct AngularLimit
{
    Constraint constraint;

    siv::Ref<Object> o_1;
    siv::Ref<Object> o_2;

    RealType angle_min = 0.0;
    RealType angle_max = 0.0;
    RealType offset    = 0.0;

    void create(siv::Ref<Object> obj_1, siv::Ref<Object> obj_2, RealType min, RealType max, RealType offset_ = 0.0f)
    {
        o_1       = obj_1;
        o_2       = obj_2;
        angle_min = min;
        angle_max = max;
        offset    = offset_;
    }

    void solve(RealType dt)
    {
        RealType const a_1 = o_1->angle         ;
        RealType const a_2 = o_2->angle - offset;
        RealType const current = a_2 - a_1;

        RealType d_a = 0.0f;
        if (current < angle_min) {
            d_a = angle_min - current;
        } else if (current > angle_max) {
            d_a = angle_max - current;
        }

        RealType const w1 = o_1->inv_inertia_tensor;
        RealType const w2 = o_2->inv_inertia_tensor;

        RealType const a             = constraint.compliance / (dt * dt);
        RealType const delta_lambda  = (-d_a - a * constraint.lambda) / (w1 + w2 + a);
        constraint.lambda        += delta_lambda;

        o_1->applyRotationCorrection( delta_lambda);
        o_2->applyRotationCorrection(-delta_lambda);
    }
};

}
