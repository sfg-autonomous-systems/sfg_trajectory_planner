#include "sfg_trajectory_planner/core/scene_object_factory.hpp"

namespace sfg_trajectory_planner::core
{
    template <typename ObjectType>
    void SceneObjectFactory::register_object_type(std::function<std::unique_ptr<ObjectType>(SceneObject::ConstructionKey, Scene &, uuids::uuid)> creator, const std::string &display_name)
    {
        auto type = SceneObject::get_type<ObjectType>();
        auto iterator = m_registered_types.find(type);

        if (iterator != m_registered_types.end())
        {
            return;
        }
        m_registered_types[type] = {type, !display_name.empty() ? display_name : type, creator};
    }

    template <typename ObjectType>
    std::unique_ptr<ObjectType> SceneObjectFactory::create_object(SceneObject::ConstructionKey key, Scene &scene, uuids::uuid uuid) const
    {
        return create_object(SceneObject::get_type<ObjectType>(), key, scene, uuid);
    }
}