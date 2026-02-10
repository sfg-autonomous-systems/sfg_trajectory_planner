#pragma once

#include "sfg_trajectory_planner/scene_object.hpp"

namespace sfg_trajectory_planner
{
    class Selection
    {
    public:
        static void select_object(SceneObject *object);
        static SceneObject *get_selected_object();

    private:
        static SceneObject *s_selected_object;
    };
}