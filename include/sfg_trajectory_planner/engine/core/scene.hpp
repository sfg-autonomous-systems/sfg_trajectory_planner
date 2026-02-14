#pragma once

#include "sfg_trajectory_planner/engine/core/scene_object_factory.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class Scene
    {
    public:
        Scene(const SceneObjectFactory &factory);
        void serialize(serialization::AbstractSerializer *serializer) const;
        void deserialize(serialization::AbstractSerializer *serializer);

        std::vector<SceneObjectFactory::RegisteredTypeInfo> get_possible_types() const;
        SceneObject *get_root();

        template <typename ObjectType = SceneObject>
        ObjectType *create_object(const std::string &name, SceneObject *parent = nullptr);
        SceneObject *create_object(const std::string &type, const std::string &name, SceneObject *parent = nullptr);
        void destroy_object(SceneObject *object);

        template <typename ObjectType = SceneObject>
        ObjectType *find_object_by_uuid(const uuids::uuid &uuid);
        template <typename ObjectType = SceneObject>
        std::vector<ObjectType *> find_objects_by_type();
        std::vector<SceneObject *> find_objects_by_type(const std::string &type);

    private:
        void serialize_object(SceneObject *object, serialization::AbstractSerializer *serializer) const;
        void deserialize_object(SceneObject *parent, serialization::AbstractSerializer *serializer);

        SceneObject *create_object(const std::string &type, const std::string &name, SceneObject *parent, uuids::uuid uuid);

        const SceneObjectFactory &m_factory;
        std::unique_ptr<SceneObject> m_root;
        std::unordered_map<uuids::uuid, std::unique_ptr<SceneObject>> m_objects;
    };
}

#include "sfg_trajectory_planner/engine/core/scene.tpp"