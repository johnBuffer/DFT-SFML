#pragma once
#include "engine/engine.hpp"
#include "user/configuration.hpp"
#include "user/physic/solver.hpp"


struct PhysicSystem : public pez::core::IProcessor
{
    pbd::Solver solver;

    PhysicSystem()
    {
        solver.gravity = {0.0f, 2000.0f};

        createObjects();
    }

    void update(float dt) override
    {
        solver.update(dt);
    }

    void createObjects()
    {
        double const compliance     = 0.000001;
        double const segment_length = 10.0f;

        pbd::Vec2D position = {0.0f, -conf::sim::world_size.y * 0.5f};

        // Top segment
        auto segment = solver.createObject();
        segment->particles.emplace_back(0.0f, 0.0f);
        segment->particles.emplace_back(conf::sim::world_size.x * 0.75, 0.0f);
        segment->computeProperties();
        segment->setPositionInstant({0.0f, -conf::sim::world_size.y * 0.5f + 30.0f});
        segment->moving = false;
        segment->inv_inertia_tensor = 0.0;
        segment->inv_mass = 0.0;

        auto last_obj = solver.createObject();
        last_obj->particles.emplace_back(0.0f, 0.0f);
        last_obj->particles.emplace_back(segment_length, 0.0f);
        last_obj->setPositionInstant(position);
        last_obj->density = 100.0;
        last_obj->computeProperties();

        solver.createSegmentPin(segment, last_obj, 0, compliance);
        solver.createAngularConstraint(segment, last_obj, Math::ConstantF32::Pi * 0.5f, 0.0);

        for (uint32_t i{1}; i < 100; ++i) {
            position.x += segment_length;
            auto ref = solver.createObject();
            ref->particles.emplace_back(0.0f, 0.0f);
            ref->particles.emplace_back(segment_length, 0.0f);
            ref->setPositionInstant(position);
            //ref->density = 0.25 + 0.75 / to<float>(i + 1);
            ref->computeProperties();

            solver.createObjectPinConstraint({last_obj, 1}, {ref, 0}, compliance);
            solver.createAngularConstraint(last_obj, ref, 0.0, compliance * 0.1);

            last_obj = ref;
        }

        auto drag = solver.createDragConstraint(last_obj, {segment_length, 0.0}, compliance);
        drag->target = {0.0f, 0.0f};
    }
};
