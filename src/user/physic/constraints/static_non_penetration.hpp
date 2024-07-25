#pragma once

namespace pbd
{

struct StaticNonPenetration
{
    Constraint constraint;
    Vec2       position;
    float      radius;

    siv::Ref<Object> object;
    float            object_radius;

    void create(Vec2 position_, float radius_, siv::Ref<Object> object_, float object_radius_)
    {
        constraint.compliance = 0.0f;
        position = position_;
        radius = radius_;
        object = object_;
        object_radius = object_radius_;
    }

    void solve(float dt)
    {
        Vec2 const obj_pos = object->position;
        Vec2 const v = obj_pos - position;

        float const d = MathVec2::length(v);
        float const min_dist = radius + object_radius;
        if (d >= min_dist) {
            return;
        }
        Vec2  const n = v / d;

        Vec2 const contact = (obj_pos + position) * 0.5f;
        Vec2 const r_1      = contact - obj_pos;

        float const w_1 = object->getGeneralizedInvMass(r_1, n);

        float const a            = constraint.compliance / (dt * dt);
        float const delta_lambda = ((min_dist - d) - a * constraint.lambda) / (w_1 + a);
        constraint.lambda       += delta_lambda;

        Vec2 const p = delta_lambda * n;
        object->applyPositionCorrection(p, r_1);

        constraint.force = constraint.lambda / (dt * dt);
    }
};

}
