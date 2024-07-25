#pragma once

namespace pbd
{

struct NonPenetration
{
    Constraint constraint;
    Object* o_1;
    Object* o_2;
    uint32_t p_1;
    uint32_t p_2;

    void create(Object& obj_1, uint32_t p_1_idx, Object& obj_2, uint32_t p_2_idx)
    {
        constraint.compliance = 0.0f;
        o_1 = &obj_1;
        o_2 = &obj_2;
        p_1 = p_1_idx;
        p_2 = p_2_idx;
    }

    void solve(float dt)
    {
        Vec2 const p_1_world = o_1->getWorldPosition(o_1->particles[p_1]);
        Vec2 const p_2_world = o_2->getWorldPosition(o_2->particles[p_2]);
        Vec2 const v = p_1_world - p_2_world;

        float const d = MathVec2::length(v);
        if (d >= 1.0f) {
            return;
        }
        Vec2  const n = v / d;

        Vec2 const contact = (p_1_world + p_2_world) * 0.5f;
        Vec2 const r1      = contact - o_1->position;
        Vec2 const r2      = contact - o_2->position;

        float const w1 = o_1->getGeneralizedInvMass(r1, n);
        float const w2 = o_2->getGeneralizedInvMass(r2, n);

        float const a            = constraint.compliance / (dt * dt);
        float const delta_lambda = (0.5f * (1.0f - d) - a * constraint.lambda) / (w1 + w2 + a);
        constraint.lambda       += delta_lambda;

        Vec2 const p = delta_lambda * n;
        o_1->applyPositionCorrection(p, r1);
        o_2->applyPositionCorrection(-p, r2);

        constraint.force = constraint.lambda / (dt * dt);
    }
};

}
