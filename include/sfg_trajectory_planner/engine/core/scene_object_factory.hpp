#pragma once

#include "sfg_trajectory_planner/engine/core/generic_factory.hpp"
#include "sfg_trajectory_planner/engine/core/scene_object.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class SceneObjectFactory : public GenericFactory<SceneObject, SceneObject::ConstructionKey, Scene &, uuids::uuid>
    {
    };
}