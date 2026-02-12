#include "sfg_trajectory_planner/core/scene_object_factory.hpp"

namespace sfg_trajectory_planner::core
{
    SceneObjectFactory::SceneObjectFactory()
    {
        register_object_type<SceneObject>(
            [&](SceneObject::ConstructionKey key, Scene &scene, uuids::uuid uuid)
            {
                return std::make_unique<SceneObject>(key, scene, uuid);
            },
            "Scene Object");
    }

    std::vector<SceneObjectFactory::RegisteredTypeInfo> SceneObjectFactory::get_registered_types() const
    {
        std::vector<RegisteredTypeInfo> types;

        for (const auto &[type, registered_type] : m_registered_types)
        {
            types.push_back(registered_type.m_info);
        }
        return types;
    }

    std::unique_ptr<SceneObject> SceneObjectFactory::create_object(const std::string &type, SceneObject::ConstructionKey key, Scene &scene, uuids::uuid uuid) const
    {
        auto iterator = m_registered_types.find(type);

        if (iterator == m_registered_types.end())
        {
            return nullptr;
        }
        return iterator->second.m_creator(key, scene, uuid);
    }
}