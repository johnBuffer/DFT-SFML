#pragma once
#include "engine/engine.hpp"

#include "renderer.hpp"
#include "user/machine/physic_system.hpp"

void initialize()
{
    pez::resources::registerFont("res/font.ttf", "font");
    pez::resources::registerTexture("res/wheel.png", "wheel");

    pez::core::registerProcessor<PhysicSystem>();
    pez::core::registerRenderer<Renderer>();
}