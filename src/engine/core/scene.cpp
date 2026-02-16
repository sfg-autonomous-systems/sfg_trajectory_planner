#include "sfg_trajectory_planner/engine/core/scene.hpp"

#include "sfg_trajectory_planner/engine/core/serialization/abstract_serializer.hpp"

namespace sfg_trajectory_planner::engine::core
{
    Scene::Scene(const SceneObjectFactory &factory)
        : m_factory(factory),
          m_root(std::make_unique<SceneObject>(SceneObject::ConstructionKey{}, *this, uuids::uuid_system_generator{}()))
    {
    }

    void Scene::serialize(serialization::AbstractSerializer *serializer) const
    {
        serializer->begin_sequence("objects", engine::core::serialization::AbstractSerializer::Mode::Write);

        for (auto child : m_root->get_children())
        {
            serializer->next_item();
            serialize_object(child, serializer);
        }
        serializer->end_sequence();
    }

    void Scene::deserialize(serialization::AbstractSerializer *serializer)
    {
        auto object_count = serializer->begin_sequence("objects", engine::core::serialization::AbstractSerializer::Mode::Read);
        m_objects.clear();
        m_objects.reserve(object_count);

        for (size_t index = 0; index < object_count; index++)
        {
            serializer->next_item();
            deserialize_object(m_root.get(), serializer);
        }
        serializer->end_sequence();
    }

    std::vector<SceneObjectFactory::RegisteredTypeInfo> Scene::get_possible_types() const
    {
        return m_factory.get_registered_types();
    }

    SceneObject *Scene::get_root() const
    {
        return m_root.get();
    }

    SceneObject *Scene::create_object(const std::string &type, const std::string &name, SceneObject *parent)
    {
        return create_object(type, name, parent, uuids::uuid_system_generator{}());
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

        // Create a copy of the children vector since destroying a child will
        //  modify the original vector when setting the parent to nullptr.
        auto children = object->get_children();

        for (auto child : children)
        {
            destroy_object(child);
        }
        m_objects.erase(iterator);
    }

    std::vector<SceneObject *> Scene::find_objects_by_type(const std::string &type) const
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

    void Scene::serialize_object(SceneObject *object, serialization::AbstractSerializer *serializer) const
    {
        object->serialize(serializer);
        serializer->begin_sequence("children", engine::core::serialization::AbstractSerializer::Mode::Write);

        for (auto child : object->get_children())
        {
            serializer->next_item();
            serialize_object(child, serializer);
        }
        serializer->end_sequence();
    }

    void Scene::deserialize_object(SceneObject *parent, serialization::AbstractSerializer *serializer)
    {
        auto object = create_object(serializer->deserialize<std::string>("type"), "", parent, uuids::uuid::from_string(serializer->deserialize<std::string>("uuid")).value());
        object->deserialize(serializer);
        auto child_count = serializer->begin_sequence("children", engine::core::serialization::AbstractSerializer::Mode::Read);

        for (size_t index = 0; index < child_count; index++)
        {
            serializer->next_item();
            deserialize_object(object, serializer);
        }
        serializer->end_sequence();
    }

    SceneObject *Scene::create_object(const std::string &type, const std::string &name, SceneObject *parent, uuids::uuid uuid)
    {
        auto object = m_factory.create_object(type, SceneObject::ConstructionKey{}, *this, uuid);
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
}