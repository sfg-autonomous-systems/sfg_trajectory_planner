#include "sfg_trajectory_planner/core/scene.hpp"

namespace sfg_trajectory_planner::core
{
    Scene::Scene()
    {
        m_root = std::make_unique<SceneObject>(SceneObjectKey{}, *this);
    }

    SceneObject *Scene::get_root()
    {
        return m_root.get();
    }

    void Scene::destroy_object(SceneObject *object)
    {
        if (!object || object == m_root.get())
        {
            return;
        }

        auto iterator = m_objects.find(object);

        if (iterator == m_objects.end())
        {
            return;
        }

        auto children = object->get_children();

        for (auto child : children)
        {
            destroy_object(child);
        }
        m_objects.erase(iterator);
    }
}