#pragma once

#include "sfg_trajectory_planner/engine/core/generic_factory.hpp"
#include "sfg_trajectory_planner/engine/core/scene_object.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class SceneObjectFactory : public GenericFactory<SceneObject, SceneObject::ConstructionKey, const Scene &, uuids::uuid>
    {
    public:
        template <typename ObjectType>
        void register_object(const std::string &display_name = "");
        template <typename ObjectType>
        void register_object(CreatorFunction creator);
        template <typename ObjectType>
        void register_object(const std::string &display_name, CreatorFunction creator);
    };
}

#include "sfg_trajectory_planner/engine/core/scene_object_factory.tpp"