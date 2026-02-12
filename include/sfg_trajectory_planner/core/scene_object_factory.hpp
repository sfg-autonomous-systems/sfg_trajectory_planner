#pragma once

#include "sfg_trajectory_planner/core/scene_object.hpp"

namespace sfg_trajectory_planner::core
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
        void register_object_type(std::function<std::unique_ptr<ObjectType>(SceneObjectKey, Scene &, uuids::uuid)> creator, const std::string &display_name = "");

        template <typename ObjectType>
        std::unique_ptr<ObjectType> create_object(SceneObjectKey key, Scene &scene, uuids::uuid uuid) const;
        std::unique_ptr<SceneObject> create_object(const std::string &type, SceneObjectKey key, Scene &scene, uuids::uuid uuid) const;

    private:
        struct RegisteredType
        {
        public:
            RegisteredTypeInfo m_info;
            std::function<std::unique_ptr<SceneObject>(SceneObjectKey, Scene &, uuids::uuid)> m_creator;
        };

        std::unordered_map<std::string, RegisteredType> m_registered_types;
    };
}

#include "sfg_trajectory_planner/core/scene_object_factory.tpp"