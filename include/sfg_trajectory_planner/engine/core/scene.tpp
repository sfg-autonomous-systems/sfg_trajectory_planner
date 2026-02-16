#include "sfg_trajectory_planner/engine/core/scene.hpp"

#include <uuid.h>

namespace sfg_trajectory_planner::engine::core
{
    template <typename ObjectType>
    ObjectType *Scene::create_object(const std::string &name, SceneObject *parent)
    {
        return dynamic_cast<ObjectType *>(create_object(SceneObject::get_type<ObjectType>(), name, parent));
    }

    template <typename ObjectType>
    ObjectType *Scene::find_object_by_uuid(const uuids::uuid &uuid) const
    {
        auto iterator = m_objects.find(uuid);

        if (iterator == m_objects.end())
        {
            return nullptr;
        }

        auto object_ptr = dynamic_cast<ObjectType *>(iterator->second.get());

        if (!object_ptr)
        {
            return nullptr;
        }
        return object_ptr;
    }

    template <typename ObjectType>
    std::vector<ObjectType *> Scene::find_objects_by_type() const
    {
        return find_objects_by_type(SceneObject::get_type<ObjectType>());
    }
}
