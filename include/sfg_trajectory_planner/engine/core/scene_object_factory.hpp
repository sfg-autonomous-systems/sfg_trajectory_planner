#pragma once

#include "sfg_trajectory_planner/engine/core/generic_factory.hpp"
#include "sfg_trajectory_planner/engine/core/scene_object.hpp"

namespace sfg_trajectory_planner::engine::core
{
    using SceneObjectFactory = GenericFactory<SceneObject, SceneObject::ConstructionKey, Scene &, uuids::uuid>;
}