#pragma once
#include "engine/common/index_vector.hpp"
#include "engine/common/utils.hpp"

#include "./configuration.hpp"
#include "./constraints/constraint.hpp"
#include "./object.hpp"
#include "./complex.hpp"

#include "./constraints/drag_constraint.hpp"
#include "./constraints/drag_constraint_interpolated.hpp"
#include "./constraints/object_pin.hpp"
#include "./constraints/angular_constraint.hpp"
#include "./constraints/segment_pin.hpp"
#include "./constraints/angular_limit.hpp"

namespace pbd
{

struct Solver
{
    siv::IndexVector<Object> objects;

    siv::IndexVector<DragConstraintInterpolated> drag_constraints;
    siv::IndexVector<ObjectPinConstraint>        object_pins;
    siv::IndexVector<AngularConstraint>          angular_constraints;
    siv::IndexVector<AngularLimit>               angular_limits;
    siv::IndexVector<SegmentPinConstraint>       segment_pin_constraints;

    Vec2D         gravity  = {0.0, 1000.0};
    pbd::RealType friction = 0.0002;

    uint32_t sub_steps{200};

    void update(RealType dt)
    {
        uint32_t const pos_iter{1};
        RealType const sub_dt{dt / to<RealType>(sub_steps)};

        for (uint32_t i{sub_steps}; i--;) {
            for (auto &obj: objects) {
                obj.forces = gravity / obj.inv_mass;
                obj.update(sub_dt);
            }

            resetConstraints();

            if (i%2) {
                solveConstraints(sub_dt);
            } else {
                solveConstraintsReverse(sub_dt);
            }


            for (auto& obj: objects) {
                obj.updateVelocities(sub_dt, friction);
            }
        }
    }

    siv::Ref<DragConstraintInterpolated> createDragConstraint(siv::Ref<Object> obj, Vec2D target, RealType compliance)
    {
        auto const constraint_id = drag_constraints.emplace_back();
        auto& constraint = drag_constraints[constraint_id];
        constraint.steps_count = sub_steps;
        constraint.create(obj, target);
        constraint.constraint.compliance = compliance;
        return drag_constraints.createRef(constraint_id);
    }

    void remove(siv::Ref<DragConstraintInterpolated> const& c)
    {
        drag_constraints.erase(c);
    }

    // Maybe replace with anchor
    siv::Ref<ObjectPinConstraint> createObjectPinConstraint(Anchor anchor_1, Anchor anchor_2, RealType compliance)
    {
        auto const constraint_id = object_pins.emplace_back();
        auto& constraint = object_pins[constraint_id];
        constraint.create(anchor_1, anchor_2);
        constraint.constraint.compliance = compliance;
        return object_pins.createRef(constraint_id);
    }

    void remove(siv::Ref<ObjectPinConstraint> const& c)
    {
        object_pins.erase(c);
    }

    siv::Ref<AngularConstraint> createAngularConstraint(siv::Ref<Object> obj_1, siv::Ref<Object> obj_2, RealType angle, RealType compliance)
    {
        auto const constraint_id = angular_constraints.emplace_back();
        auto& constraint = angular_constraints[constraint_id];
        constraint.create(obj_1, obj_2, angle);
        constraint.constraint.compliance = compliance;
        return angular_constraints.createRef(constraint_id);
    }

    void remove(siv::Ref<AngularConstraint> const& c)
    {
        angular_constraints.erase(c);
    }

    siv::Ref<AngularLimit> createAngularLimit(siv::Ref<Object> obj_1, siv::Ref<Object> obj_2, RealType angle_min, RealType angle_max, RealType offset, RealType compliance)
    {
        auto const constraint_id = angular_limits.emplace_back();
        auto& constraint = angular_limits[constraint_id];
        constraint.create(obj_1, obj_2, angle_min, angle_max, offset);
        constraint.constraint.compliance = compliance;
        return angular_limits.createRef(constraint_id);
    }

    void remove(siv::Ref<AngularLimit> const& c)
    {
        angular_limits.erase(c);
    }

    siv::Ref<SegmentPinConstraint> createSegmentPin(siv::Ref<Object> segment, siv::Ref<Object> pinned_object, uint32_t pinned_particle, RealType compliance)
    {
        auto const constraint_id = segment_pin_constraints.emplace_back();
        auto& constraint = segment_pin_constraints[constraint_id];
        constraint.create(segment, pinned_object, pinned_particle);
        constraint.constraint.compliance = compliance;
        return segment_pin_constraints.createRef(constraint_id);
    }

    void remove(siv::Ref<SegmentPinConstraint> const& c)
    {
        segment_pin_constraints.erase(c);
    }

    siv::Ref<Object> createObject()
    {
        siv::ID const id = objects.emplace_back();
        return objects.createRef(id);
    }

    void resetConstraints()
    {
        for (auto& c: drag_constraints) {
            c.constraint.lambda = 0.0;
        }

        for (auto& c: object_pins) {
            c.constraint.lambda = 0.0;
        }

        for (auto& c: angular_constraints) {
            c.constraint.lambda = 0.0;
        }

        for (auto& c : segment_pin_constraints) {
            c.constraint.lambda = 0.0f;
        }

        for (auto& c : angular_limits) {
            c.constraint.lambda = 0.0f;
        }
    }

    void solveConstraints(RealType dt)
    {
        for (auto& c : drag_constraints) {
            c.solve(dt);
        }

        for (auto& c : object_pins) {
            c.solve(dt);
        }

        for (auto& c : angular_constraints) {
            c.solve(dt);
        }

        for (auto& c : segment_pin_constraints) {
            c.solve(dt);
        }

        for (auto& c : angular_limits) {
            c.solve(dt);
        }
    }

    void solveConstraintsReverse(RealType dt)
    {
        for (auto& c : drag_constraints) {
            c.solve(dt);
        }

        solveReverse(object_pins, dt);

        solveReverse(angular_constraints, dt);

        for (auto& c : segment_pin_constraints) {
            c.solve(dt);
        }

        for (auto& c : angular_limits) {
            c.solve(dt);
        }
    }

    template<typename TConstraint>
    void solveReverse(siv::IndexVector<TConstraint>& vector, RealType dt)
    {
        auto& v = vector.getData();
        for (auto it{v.rbegin()}; it != v.rend(); ++it) {
            it->solve(dt);
        }
    }
};

}