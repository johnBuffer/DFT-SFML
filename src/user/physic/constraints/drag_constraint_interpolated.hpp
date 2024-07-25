#pragma once
#include "../object.hpp"
#include "../anchor.hpp"
#include "../configuration.hpp"

#include "./constraint.hpp"

namespace pbd
{
struct DragConstraintInterpolated
{
    Constraint constraint;
    Vec2D      target;
    Vec2D      target_current;
    Vec2D      update_move;
    uint32_t   steps_count = 1;
    uint32_t   current_step = 0;
    Anchor     anchor;

    void create(siv::Ref<Object> obj, Vec2D a)
    {
        anchor.obj       = obj;
        anchor.obj_coord = a;
    }

    void setTarget(Vec2D t)
    {
        current_step = 0;
        target = t;
        update_move = (target - target_current) / to<RealType>(steps_count);
    }

    void solve(RealType dt)
    {
        if (current_step < steps_count) {
            ++current_step;
            target_current += update_move;
        }

        Vec2D const pa = anchor.obj->getWorldPosition(anchor.obj_coord);
        Vec2D const r1 = pa - anchor.obj->position;

        Vec2D const    v = (target_current - pa);
        RealType const d = MathVec2::length(v);
        if (d == 0.0) {
            return;
        }
        Vec2D const n = v / d;

        RealType const w1 = anchor.obj->getGeneralizedInvMass(r1, n);
        RealType const w2 = 0.0f;
        RealType const a             = constraint.compliance / (dt * dt);
        RealType const delta_lambda  = (d) / (w1 + w2 + a);

        Vec2D const p = delta_lambda * n;
        anchor.obj->applyPositionCorrection(p, r1);

        constraint.force = constraint.lambda / (dt * dt);
    }
};
}
