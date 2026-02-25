#pragma once

#include "sfg_trajectory_planner/engine/core/scene_object_factory.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class Scene : public serialization::ISerializable
    {
    public:
        Scene(const SceneObjectFactory &factory);
        void serialize(serialization::AbstractSerializer *serializer) const override;
        void deserialize(serialization::AbstractSerializer *serializer) override;
        void serialize_object(SceneObject *object, serialization::AbstractSerializer *serializer) const;
        SceneObject *deserialize_object(SceneObject *parent, serialization::AbstractSerializer *serializer);

        std::vector<SceneObjectFactory::RegisteredTypeInfo> get_possible_types() const;
        SceneObject *get_root() const;

        template <typename ObjectType = SceneObject>
        ObjectType *create_object(const std::string &name, SceneObject *parent = nullptr);
        SceneObject *create_object(const std::string &type, const std::string &name, SceneObject *parent = nullptr);
        void destroy_object(SceneObject *object);

        template <typename ObjectType = SceneObject>
        ObjectType *find_object_by_uuid(const uuids::uuid &uuid) const;
        template <typename ObjectType = SceneObject>
        std::vector<ObjectType *> find_objects_by_type() const;
        std::vector<SceneObject *> find_objects_by_type(const std::string &type) const;

    private:
        SceneObject *create_object(const std::string &type, const std::string &name, SceneObject *parent, uuids::uuid uuid);

        const SceneObjectFactory &m_factory;
        std::unique_ptr<SceneObject> m_root;
        std::unordered_map<uuids::uuid, std::unique_ptr<SceneObject>> m_objects;
    };
}

#include "sfg_trajectory_planner/engine/core/scene.tpp"