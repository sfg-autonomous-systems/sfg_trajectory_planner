#include "sfg_trajectory_planner/engine/core/scene_object_factory.hpp"

namespace sfg_trajectory_planner::engine::core
{
    template <typename ObjectType>
    void SceneObjectFactory::register_object(const std::string &display_name)
    {
        register_type<ObjectType, ObjectType>(display_name);
    }

    template <typename ObjectType>
    void SceneObjectFactory::register_object(CreatorFunction creator)
    {
        register_type<ObjectType, ObjectType>(creator);
    }

    template <typename ObjectType>
    void SceneObjectFactory::register_object(const std::string &display_name, CreatorFunction creator)
    {
        register_type<ObjectType, ObjectType>(display_name, creator);
    }
}