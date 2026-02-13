#include "sfg_trajectory_planner/core/scene.hpp"

#include "sfg_trajectory_planner/core/serialization/abstract_serializer.hpp"

namespace sfg_trajectory_planner::core
{
    Scene::Scene(const SceneObjectFactory &factory)
        : m_factory(factory),
          m_root(std::make_unique<SceneObject>(SceneObject::ConstructionKey{}, *this, uuids::uuid_system_generator{}()))
    {
    }

    void Scene::serialize(core::serialization::AbstractSerializer *serializer) const
    {
        serializer->serialize("object_count", m_root->get_children().size());

        for (size_t index = 0; index < m_root->get_children().size(); index++)
        {
            serializer->push_group("object_" + std::to_string(index));
            serialize_object(m_root->get_children()[index], serializer);
            serializer->pop_group();
        }
    }

    void Scene::deserialize(core::serialization::AbstractSerializer *serializer)
    {
        // Clear existing objects (except root) before deserialization.
        m_objects.clear();

        auto object_count = std::get<size_t>(serializer->deserialize("object_count"));

        for (size_t index = 0; index < object_count; index++)
        {
            serializer->push_group("object_" + std::to_string(index));
            deserialize_object(m_root.get(), serializer);
            serializer->pop_group();
        }
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

    void Scene::serialize_object(core::SceneObject *object, core::serialization::AbstractSerializer *serializer) const
    {
        object->serialize(serializer);
        serializer->serialize("child_count", object->get_children().size());

        for (size_t index = 0; index < object->get_children().size(); index++)
        {
            serializer->push_group("child_" + std::to_string(index));
            serialize_object(object->get_children()[index], serializer);
            serializer->pop_group();
        }
    }

    void Scene::deserialize_object(core::SceneObject *parent, core::serialization::AbstractSerializer *serializer)
    {
        auto type = std::get<std::string>(serializer->deserialize("type"));
        auto uuid = uuids::uuid::from_string(std::get<std::string>(serializer->deserialize("uuid"))).value();
        auto object = create_object(type, "", parent, uuid);
        object->deserialize(serializer);

        auto child_count = std::get<size_t>(serializer->deserialize("child_count"));

        for (size_t index = 0; index < child_count; index++)
        {
            serializer->push_group("child_" + std::to_string(index));
            deserialize_object(object, serializer);
            serializer->pop_group();
        }
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