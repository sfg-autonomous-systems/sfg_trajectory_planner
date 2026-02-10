#include "sfg_trajectory_planner/scene.hpp"

namespace sfg_trajectory_planner
{
    template <typename ObjectType, typename... Args>
    ObjectType *Scene::create_object(const std::string &name, SceneObject *parent, Args &&...args)
    {
        auto object = std::make_unique<ObjectType>(SceneObjectKey{}, *this, std::forward<Args>(args)...);
        auto object_ptr = object.get();

        m_objects[object_ptr] = std::move(object);
        object_ptr->set_name(name);
        object_ptr->set_parent(parent ? parent : m_root.get());
        return object_ptr;
    }
}
