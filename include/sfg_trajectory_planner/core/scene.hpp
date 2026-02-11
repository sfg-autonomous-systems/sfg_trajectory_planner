#pragma once

#include "sfg_trajectory_planner/core/scene_object.hpp"

namespace sfg_trajectory_planner::core
{
    class Scene
    {
    public:
        Scene();
        SceneObject *get_root();

        template <typename ObjectType = SceneObject, typename... Args>
        ObjectType *create_object(const std::string &name, SceneObject *parent = nullptr, Args &&...args);

        void destroy_object(SceneObject *object);

        template <typename ObjectType = SceneObject>
        ObjectType *find_object_by_uuid(const uuids::uuid &uuid);

    private:
        std::unique_ptr<SceneObject> m_root;
        std::unordered_map<uuids::uuid, std::unique_ptr<SceneObject>> m_objects;
    };
}

#include "sfg_trajectory_planner/core/scene.tpp"