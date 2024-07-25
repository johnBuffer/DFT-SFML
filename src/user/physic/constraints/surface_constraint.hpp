#pragma once
#include "./constraint.hpp"

namespace pbd
{

struct SurfaceConstraint
{
    Vec2 origin;
    Vec2 normal;

    Constraint constraint;

    void create(Vec2 origin_, Vec2 normal_)
    {
        origin = origin_;
        normal = normal_;
    }

    void solve(Object& obj, float dt)
    {
        float const offset = getOffset(obj);

        if (offset > 0.0f) {
            Vec2  const r1 = -normal * 0.5f;
            float const w1 = obj.getGeneralizedInvMass(r1, normal);

            float const a            = constraint.compliance / (dt * dt);
            float const delta_lambda = offset / (w1 + a);

            Vec2 const p = delta_lambda * normal;
            obj.applyPositionCorrection(p, r1);
        }
    }

    [[nodiscard]]
    float getOffset(Object const& object) const
    {
        float const dist = MathVec2::distToLine(object.position, MathVec2::normal(normal), origin);
        float const side = MathVec2::dot(object.position - origin, normal);
        if (side < 0.0f) { // The object is inside the surface
            return dist + 0.5f;
        } else { // The object is outside but overlap is still possible
            return 0.5f - dist;
        }
    }
};

}