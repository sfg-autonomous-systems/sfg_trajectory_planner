#include "sfg_trajectory_planner/core/scene.hpp"

namespace sfg_trajectory_planner::core
{
    Scene::Scene(const SceneObjectFactory &factory)
        : m_factory(factory),
          m_root(std::make_unique<SceneObject>(SceneObjectKey{}, *this, uuids::uuid_system_generator{}()))
    {
    }

    std::vector<SceneObjectFactory::RegisteredTypeInfo> Scene::get_possible_types() const
    {
        return m_factory.get_registered_types();
    }

    SceneObject *Scene::get_root()
    {
        return m_root.get();
    }

    SceneObject *Scene::create_object(const std::string &type, const std::string &name, SceneObject *parent)
    {
        auto object = m_factory.create_object(type, SceneObjectKey{}, *this, uuids::uuid_system_generator{}());
        auto object_ptr = object.get();

        if (!object_ptr)
        {
            return nullptr;
        }

        m_objects[object->get_uuid()] = std::move(object);
        object_ptr->set_name(name);
        object_ptr->set_parent(parent ? parent : m_root.get());
        return object_ptr;
    }

    void Scene::destroy_object(SceneObject *object)
    {
        if (!object || object == m_root.get())
        {
            return;
        }

        auto iterator = m_objects.find(object->get_uuid());

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

    std::vector<SceneObject *> Scene::find_objects_by_type(const std::string &type)
    {
        std::vector<SceneObject *> objects;

        for (auto &[uuid, object] : m_objects)
        {
            if (object->get_type() == type)
            {
                objects.push_back(object.get());
            }
        }
        return objects;
    }
}