#include "sfg_trajectory_planner/core/scene.hpp"

#include <uuid.h>

namespace sfg_trajectory_planner::core
{
    template <typename ObjectType, typename... Args>
    ObjectType *Scene::create_object(const std::string &name, SceneObject *parent, Args &&...args)
    {
        auto object = std::make_unique<ObjectType>(SceneObjectKey{}, *this, uuids::uuid_system_generator{}(), std::forward<Args>(args)...);
        auto object_ptr = object.get();

        m_objects[object->get_uuid()] = std::move(object);
        object_ptr->set_name(name);
        object_ptr->set_parent(parent ? parent : m_root.get());
        return object_ptr;
    }

    template <typename ObjectType>
    ObjectType *Scene::find_object_by_uuid(const uuids::uuid &uuid)
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
}
