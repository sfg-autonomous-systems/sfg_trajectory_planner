#pragma once

#include "sfg_trajectory_planner/scene_object.hpp"

#include <memory>
#include <unordered_map>

namespace sfg_trajectory_planner
{
    class Scene
    {
    public:
        Scene();
        SceneObject *get_root();
        template <typename ObjectType = SceneObject, typename... Args>
        ObjectType *create_object(const std::string &name, SceneObject *parent = nullptr, Args &&...args);
        void destroy_object(SceneObject *object);

    private:
        std::unique_ptr<SceneObject> m_root;
        std::unordered_map<SceneObject *, std::unique_ptr<SceneObject>> m_objects;
    };
}

#include "sfg_trajectory_planner/scene.tpp"