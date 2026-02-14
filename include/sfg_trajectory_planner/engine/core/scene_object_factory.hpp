#pragma once

#include "sfg_trajectory_planner/engine/core/scene_object.hpp"

namespace sfg_trajectory_planner::engine::core
{
    class SceneObjectFactory
    {
    public:
        struct RegisteredTypeInfo
        {
            std::string m_type;
            std::string m_display_name;
        };

        SceneObjectFactory();

        std::vector<RegisteredTypeInfo> get_registered_types() const;

        template <typename ObjectType>
        void register_object_type(std::function<std::unique_ptr<ObjectType>(SceneObject::ConstructionKey, Scene &, uuids::uuid)> creator, const std::string &display_name = "");

        template <typename ObjectType>
        std::unique_ptr<ObjectType> create_object(SceneObject::ConstructionKey key, Scene &scene, uuids::uuid uuid) const;
        std::unique_ptr<SceneObject> create_object(const std::string &type, SceneObject::ConstructionKey key, Scene &scene, uuids::uuid uuid) const;

    private:
        struct RegisteredType
        {
        public:
            RegisteredTypeInfo m_info;
            std::function<std::unique_ptr<SceneObject>(SceneObject::ConstructionKey, Scene &, uuids::uuid)> m_creator;
        };

        std::unordered_map<std::string, RegisteredType> m_registered_types;
    };
}

#include "sfg_trajectory_planner/engine/core/scene_object_factory.tpp"