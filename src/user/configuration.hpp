#pragma once
#include <SFML/Graphics.hpp>
#include "engine/common/vec.hpp"

namespace conf
{

namespace win
{

UVec2 const window_size = {2560, 1440};

}

namespace sim
{

Vec2 const world_size = {1600.0f, 800.0f};

}

namespace signal
{

float const padding_sampling_dist = 10.0f;

}

}